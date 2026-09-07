#include <gecode/optimize/diagnostics.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
constexpr double infinity = std::numeric_limits<double>::infinity();

std::vector<ConflictGroup> partition(const ModelSnapshot& model) {
  std::vector<std::size_t> parent(model.variables.size());
  std::iota(parent.begin(), parent.end(), 0);
  auto root = [&](std::size_t i) {
    while (parent[i] != i) { parent[i] = parent[parent[i]]; i = parent[i]; }
    return i;
  };
  auto join = [&](Variable a, Variable b) {
    const auto x = root(a.id), y = root(b.id);
    parent[std::max(x, y)] = std::min(x, y);
  };
  std::vector<bool> guarded(model.variables.size(), false);
  std::vector<bool> generated(model.rows.size(), false);
  for (const auto& indicator : model.indicators) if (indicator.active) {
    guarded[indicator.activator.id] = true;
    auto include = [&](Variable variable) {
      guarded[variable.id] = true;
      join(indicator.activator, variable);
    };
    if (indicator.inactive_gate) include(*indicator.inactive_gate);
    for (const auto& term : indicator.terms) include(term.variable);
    for (auto row : indicator.generated_rows) generated[row.id] = true;
  }
  std::vector<ConflictGroup> groups;
  for (const auto& row : model.rows) if (row.active && !generated[row.constraint.id]) {
    ConflictGroup group;
    group.kind = ConflictGroupKind::Row;
    group.row = row.constraint;
    group.name = row.name.empty() ? "row " + std::to_string(row.constraint.id) : row.name;
    groups.push_back(std::move(group));
  }
  for (const auto& variable : model.variables) {
    if (!variable.active || guarded[variable.variable.id]) continue;
    auto append = [&](ConflictGroupKind kind, const char* suffix) {
      ConflictGroup group;
      group.kind = kind;
      group.variable = variable.variable;
      group.name = (variable.name.empty() ? "variable " + std::to_string(variable.variable.id)
                                          : variable.name) + suffix;
      groups.push_back(std::move(group));
    };
    if (variable.type == VariableType::Binary || variable.type == VariableType::SemiContinuous ||
        variable.type == VariableType::SemiInteger) {
      append(ConflictGroupKind::VariableDomain, " domain");
    } else {
      if (std::isfinite(variable.lower)) append(ConflictGroupKind::LowerBound, " lower bound");
      if (std::isfinite(variable.upper)) append(ConflictGroupKind::UpperBound, " upper bound");
      if (variable.type == VariableType::Integer) append(ConflictGroupKind::Integrality, " integrality");
    }
  }
  std::map<std::size_t, ConflictGroup> components;
  for (const auto& indicator : model.indicators) if (indicator.active) {
    auto& group = components[root(indicator.activator.id)];
    group.kind = ConflictGroupKind::IndicatorComponent;
    group.indicators.push_back(indicator.indicator);
    group.generated_rows.insert(group.generated_rows.end(), indicator.generated_rows.begin(),
                                 indicator.generated_rows.end());
  }
  for (const auto& variable : model.variables) if (guarded[variable.variable.id])
    components.at(root(variable.variable.id)).grouped_variables.push_back(variable.variable);
  for (auto& entry : components) {
    auto& group = entry.second;
    group.name = "indicator component " + std::to_string(group.indicators.front().id);
    groups.push_back(std::move(group));
  }
  return groups;
}

void relax(ModelSnapshot& model, const ConflictGroup& group) {
  auto free_domain = [&](Variable handle) {
    auto& variable = model.variables[handle.id];
    variable.type = VariableType::Continuous;
    variable.lower = -infinity;
    variable.upper = infinity;
  };
  auto erase_row = [&](Constraint handle) {
    auto& row = model.rows[handle.id];
    row.active = false;
    row.terms.clear();
  };
  switch (group.kind) {
  case ConflictGroupKind::Row: erase_row(*group.row); break;
  case ConflictGroupKind::LowerBound: model.variables[group.variable->id].lower = -infinity; break;
  case ConflictGroupKind::UpperBound: model.variables[group.variable->id].upper = infinity; break;
  case ConflictGroupKind::Integrality: model.variables[group.variable->id].type = VariableType::Continuous; break;
  case ConflictGroupKind::VariableDomain: free_domain(*group.variable); break;
  case ConflictGroupKind::IndicatorComponent:
    for (auto indicator : group.indicators) model.indicators[indicator.id].active = false;
    for (auto row : group.generated_rows) erase_row(row);
    for (auto variable : group.grouped_variables) free_domain(variable);
    break;
  }
}

ModelSnapshot subset(const ModelSnapshot& original, const std::vector<ConflictGroup>& groups,
                     const std::vector<bool>& kept) {
  ModelSnapshot model = original;
  model.objective = {};
  for (std::size_t i = 0; i < groups.size(); ++i) if (!kept[i]) relax(model, groups[i]);
  return model;
}

enum class Decision { Unknown, Feasible, Infeasible };

ConflictStatus stopped_status(const ConflictResult& result, Termination reason) {
  if (result.infeasibility_established) return ConflictStatus::Incomplete;
  if (reason == Termination::Unsupported) return ConflictStatus::Unsupported;
  if (reason == Termination::InvalidModel) return ConflictStatus::InvalidModel;
  if (reason == Termination::BackendError || reason == Termination::NumericalFailure ||
      reason == Termination::MemoryLimit) return ConflictStatus::Error;
  return ConflictStatus::Unknown;
}

ConflictResult run(const ModelSnapshot& original, const ConflictOptions& options, SolveBudget& budget) {
  ConflictResult output;
  output.model_id = original.model_id;
  output.revision = original.revision;
  std::vector<ConflictGroup> groups;
  std::vector<bool> kept;
  auto stop = [&](Termination reason, std::string message) {
    output.termination = reason;
    output.status = stopped_status(output, reason);
    output.message = std::move(message);
  };
  auto finish = [&]() {
    if (output.infeasibility_established) {
      // Compact the existing allocation: reporting an interrupted conflict must
      // not need another allocation proportional to its number of groups.
      std::size_t retained = 0;
      for (std::size_t i = 0; i < groups.size(); ++i) if (kept[i]) {
        if (retained != i) groups[retained] = std::move(groups[i]);
        ++retained;
      }
      groups.resize(retained);
      output.groups = std::move(groups);
    }
    if (budget.expired() && (output.status == ConflictStatus::Irreducible ||
                             output.status == ConflictStatus::Feasible)) {
      stop(budget.stop_reason().value_or(Termination::Unknown),
           "Shared conflict budget stopped before completion");
      output.feasible_witness.clear();
    }
    output.elapsed_seconds = budget.elapsed_seconds();
    return std::move(output);
  };
  try {
    validate_structure(original);
    if (std::any_of(original.globals.begin(),original.globals.end(),[](const auto& record){return record.active;})) {
      stop(Termination::Unsupported,"Conflict analysis of native globals requires a global-aware feasibility oracle");
      return finish();
    }
    if (options.solve.guarantee != Guarantee::Numerical || options.solve.node_limit) {
      stop(Termination::Unsupported,
           "Conflict analysis supports Numerical evidence and time/cancellation budgets; node budgets need consumed-node reporting");
      return finish();
    }
    if (budget.expired()) {
      stop(budget.stop_reason().value_or(Termination::Unknown), "Conflict budget stopped before analysis");
      return finish();
    }
    groups = partition(original);
    kept.assign(groups.size(), true);
    auto oracle = [&](const ModelSnapshot& model, std::vector<double>& witness) {
      if (budget.expired()) {
        stop(budget.stop_reason().value_or(Termination::Unknown), "Shared conflict budget stopped");
        return Decision::Unknown;
      }
      auto solve_options = options.solve;
      solve_options.primal_start.clear();
      solve_options.relative_gap = solve_options.absolute_gap = 0.0;
      solve_options.cancellation = budget.cancellation();
      solve_options.time_limit_seconds = budget.remaining_seconds();
      ++output.oracle_calls;
      auto result = solve(model, solve_options);
      output.backend = result.backend;
      output.backend_version = result.backend_version;
      if (budget.expired()) {
        stop(budget.stop_reason().value_or(Termination::Unknown), "Feasibility oracle returned after the shared budget stopped");
        return Decision::Unknown;
      }
      if (result.model_id != model.model_id || result.revision != model.revision ||
          result.guarantee != Guarantee::Numerical) {
        stop(Termination::NumericalFailure, "Feasibility oracle returned incompatible identity or evidence");
        return Decision::Unknown;
      }
      // Interrupted/ambiguous oracles are never advanced, even with an incumbent.
      if (result.termination != Termination::Optimal && result.termination != Termination::Infeasible) {
        stop(result.termination, "Feasibility oracle did not decide the subset: " + result.message);
        return Decision::Unknown;
      }
      if (result.termination == Termination::Infeasible) {
        if (result.has_solution()) {
          stop(Termination::NumericalFailure, "Infeasible oracle result also carries a validated solution");
          return Decision::Unknown;
        }
        return Decision::Infeasible;
      }
      const auto check = validate(model, result.values, options.solve.feasibility_tolerance,
                                  options.solve.integrality_tolerance);
      if (!result.has_solution() || !check.valid) {
        stop(Termination::NumericalFailure, "Feasible oracle candidate failed independent subset validation");
        return Decision::Unknown;
      }
      if (budget.expired()) {
        stop(budget.stop_reason().value_or(Termination::Unknown), "Shared conflict budget stopped during candidate validation");
        return Decision::Unknown;
      }
      witness = std::move(result.values);
      return Decision::Feasible;
    };
    std::vector<double> witness;
    const auto initial = oracle(subset(original, groups, kept), witness);
    if (initial == Decision::Unknown) return finish();
    if (initial == Decision::Feasible) {
      output.status = ConflictStatus::Feasible;
      output.termination = Termination::Optimal;
      output.message = "Original constraints have an independently validated numerical feasible assignment";
      output.feasible_witness = std::move(witness);
      return finish();
    }
    output.infeasibility_established = true;
    for (std::size_t i = 0; i < groups.size(); ++i) {
      kept[i] = false;
      Decision decision;
      try { decision = oracle(subset(original, groups, kept), witness); }
      catch (...) { kept[i] = true; throw; }
      if (decision != Decision::Infeasible) kept[i] = true;
      if (decision == Decision::Unknown) return finish();
      if (decision == Decision::Feasible) {
        groups[i].necessity_verified = true;
        if (options.retain_deletion_witnesses) groups[i].deletion_witness = std::move(witness);
      }
    }
    // Feasible deletion witnesses remain feasible as other groups are relaxed.
    // Recheck retained witnesses against the final conflict, independently.
    for (std::size_t i = 0; i < groups.size(); ++i) if (kept[i]) {
      if (budget.expired()) {
        stop(budget.stop_reason().value_or(Termination::Unknown), "Shared conflict budget stopped during final checks");
        return finish();
      }
      if (options.retain_deletion_witnesses) {
        auto removed = kept;
        removed[i] = false;
        const auto check = validate(subset(original, groups, removed), groups[i].deletion_witness,
                                    options.solve.feasibility_tolerance, options.solve.integrality_tolerance);
        if (!check.valid) {
          stop(Termination::NumericalFailure, "Final deletion witness no longer validates; conflict is not promoted");
          return finish();
        }
      }
    }
    if (std::none_of(kept.begin(), kept.end(), [](bool keep) { return keep; })) {
      stop(Termination::NumericalFailure, "Oracle reported an unconstrained continuous system infeasible");
      return finish();
    }
    output.status = ConflictStatus::Irreducible;
    output.termination = Termination::Infeasible;
    output.message = "Numerically infeasible and deletion-minimal at the reported group granularity";
  } catch (const ModelError& error) {
    stop(Termination::InvalidModel, error.what());
  } catch (const std::bad_alloc&) {
    stop(Termination::MemoryLimit, "Conflict analysis allocation failed");
  } catch (const std::exception& error) {
    stop(Termination::BackendError, error.what());
  }
  return finish();
}

ConflictResult invalid(ModelId id, Revision revision, Termination reason, const std::string& message) {
  ConflictResult result;
  result.model_id = id;
  result.revision = revision;
  result.termination = reason;
  result.status = stopped_status(result, reason);
  result.message = message;
  return result;
}
}

ConflictResult analyze_conflict(const ModelSnapshot& model, const ConflictOptions& options) {
  try { SolveBudget budget(options.solve); return run(model, options, budget); }
  catch (const ModelError& error) { return invalid(model.model_id, model.revision, Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return invalid(model.model_id, model.revision, Termination::MemoryLimit, "Conflict allocation failed"); }
}

ConflictResult analyze_conflict(const Model& model, const ConflictOptions& options) {
  try { SolveBudget budget(options.solve); return run(model.snapshot(), options, budget); }
  catch (const ModelError& error) { return invalid(model.id(), model.revision(), Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return invalid(model.id(), model.revision(), Termination::MemoryLimit, "Conflict allocation failed"); }
}

}}
