#include <gecode/optimize/relaxation.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
constexpr double infinity = std::numeric_limits<double>::infinity();
class UnsupportedRepair : public std::runtime_error { using std::runtime_error::runtime_error; };
class InvalidRepair : public std::runtime_error { using std::runtime_error::runtime_error; };

class Sum {
  long double sum_ = 0.0L, correction_ = 0.0L;
public:
  void add(long double value) {
    const long double next = sum_ + value;
    if (!std::isfinite(value) || !std::isfinite(next))
      throw UnsupportedRepair("Repair arithmetic exceeds supported precision");
    correction_ += std::fabs(sum_) >= std::fabs(value)
      ? (sum_ - next) + value : (value - next) + sum_;
    sum_ = next;
  }
  long double value() const {
    const auto result = sum_ + correction_;
    if (!std::isfinite(result)) throw UnsupportedRepair("Repair accumulation exceeds supported precision");
    return result;
  }
};

double finite(long double value) {
  if (!std::isfinite(value) || std::fabs(value) > std::numeric_limits<double>::max())
    throw UnsupportedRepair("Repair value cannot be represented as a finite double");
  return static_cast<double>(value);
}

long double evaluate(const std::vector<Term>& terms, const std::vector<double>& values,
                     double offset = 0.0) {
  Sum sum;
  sum.add(offset);
  for (const auto& term : terms) sum.add(static_cast<long double>(term.coefficient) * values.at(term.variable.id));
  return sum.value();
}

void check_penalty(RelaxationSide side, double penalty) {
  if (side != RelaxationSide::Lower && side != RelaxationSide::Upper)
    throw ModelError("Unknown relaxation side");
  if (!std::isfinite(penalty) || penalty <= 0)
    throw ModelError("Relaxation penalties must be positive and finite");
}

bool indicator_variable(const ModelSnapshot& model, Variable variable) {
  for (const auto& indicator : model.indicators) if (indicator.active) {
    if (indicator.activator == variable ||
        (indicator.inactive_gate && *indicator.inactive_gate == variable)) return true;
    for (const auto& term : indicator.terms) if (term.variable == variable) return true;
  }
  return false;
}

struct Construction {
  ModelSnapshot model;
  std::vector<Variable> mapping;
  std::vector<RelaxationItem> items;
  ObjectiveData penalty;
};

Construction construct(const ModelSnapshot& source, const RelaxationOptions& options) {
  Construction built;
  std::set<std::tuple<bool, std::uint64_t, RelaxationSide>> seen;
  for (const auto& selected : options.rows) {
    check_penalty(selected.side, selected.penalty);
    if (selected.row.model_id != source.model_id || selected.row.id >= source.rows.size() ||
        !source.rows[selected.row.id].active) throw ModelError("Relaxed row is foreign, absent, or deleted");
    if (!seen.emplace(false, selected.row.id, selected.side).second)
      throw ModelError("A row side was selected more than once");
    const auto& row = source.rows[selected.row.id];
    if (row.indicator_origin) throw UnsupportedRepair("Generated indicator rows cannot be relaxed; original indicators remain hard");
    RelaxationItem item;
    item.source_row = selected.row; item.side = selected.side; item.penalty = selected.penalty;
    item.original_bound = selected.side == RelaxationSide::Lower ? row.lower : row.upper;
    item.name = row.name.empty() ? "row " + std::to_string(selected.row.id) : row.name;
    if (!std::isfinite(item.original_bound)) throw ModelError("Selected row side has no finite bound");
    built.items.push_back(std::move(item));
  }
  for (const auto& selected : options.bounds) {
    check_penalty(selected.side, selected.penalty);
    if (selected.variable.model_id != source.model_id || selected.variable.id >= source.variables.size() ||
        !source.variables[selected.variable.id].active) throw ModelError("Relaxed variable is foreign, absent, or deleted");
    if (!seen.emplace(true, selected.variable.id, selected.side).second)
      throw ModelError("A variable bound was selected more than once");
    const auto& variable = source.variables[selected.variable.id];
    if (variable.type == VariableType::SemiContinuous || variable.type == VariableType::SemiInteger)
      throw UnsupportedRepair("Selected semi-variable bounds need a typed disjunctive relaxation; their zero domain remains hard");
    if (indicator_variable(source, selected.variable))
      throw UnsupportedRepair("Indicator-participating variable bounds cannot be relaxed without rebuilding the guarded formulation");
    RelaxationItem item;
    item.source_variable = selected.variable; item.side = selected.side; item.penalty = selected.penalty;
    item.original_bound = selected.side == RelaxationSide::Lower ? variable.lower : variable.upper;
    item.name = variable.name.empty() ? "variable " + std::to_string(selected.variable.id) : variable.name;
    if (!std::isfinite(item.original_bound)) throw ModelError("Selected variable side has no finite bound");
    built.items.push_back(std::move(item));
  }
  std::sort(built.items.begin(), built.items.end(), [](const RelaxationItem& a, const RelaxationItem& b) {
    return std::make_tuple(a.source_variable.has_value(), a.source_row ? a.source_row->id : a.source_variable->id, a.side) <
           std::make_tuple(b.source_variable.has_value(), b.source_row ? b.source_row->id : b.source_variable->id, b.side);
  });
  Model identity;
  // Public snapshots can carry an arbitrary otherwise well-formed owner.
  if (identity.id() == source.model_id) identity = Model{};
  built.model = source;
  auto& model = built.model;
  model.model_id = identity.id();
  model.revision = 1;
  // Active globals were rejected before construction. Inactive records have no
  // semantic effect and no live constraints refer to their global handles.
  model.globals.clear();
  auto variable = [&](Variable& handle) { handle.model_id = model.model_id; };
  auto row = [&](Constraint& handle) { handle.model_id = model.model_id; };
  auto indicator = [&](Indicator& handle) { handle.model_id = model.model_id; };
  auto terms = [&](std::vector<Term>& expression) { for (auto& term : expression) variable(term.variable); };
  for (auto& data : model.variables) {
    variable(data.variable);
    if (data.indicator_origin) indicator(*data.indicator_origin);
    built.mapping.push_back(data.variable);
  }
  for (auto& data : model.rows) {
    row(data.constraint); terms(data.terms);
    if (data.indicator_origin) indicator(*data.indicator_origin);
  }
  terms(model.objective.terms);
  for (auto& data : model.indicators) {
    indicator(data.indicator); variable(data.activator); terms(data.terms);
    if (data.inactive_gate) variable(*data.inactive_gate);
    for (auto& handle : data.generated_rows) row(handle);
    for (auto& domain : data.domains) variable(domain.variable);
  }
  for (auto& item : built.items) {
    if (model.variables.size() >= std::numeric_limits<std::uint64_t>::max() ||
        model.rows.size() >= std::numeric_limits<std::uint64_t>::max())
      throw UnsupportedRepair("Private repair model exceeds handle capacity");
    item.slack = {model.model_id, static_cast<std::uint64_t>(model.variables.size())};
    item.penalty_row = {model.model_id, static_cast<std::uint64_t>(model.rows.size())};
    VariableData slack;
    slack.variable = item.slack; slack.lower = 0; slack.upper = infinity;
    slack.name = "__repair_slack_" + std::to_string(item.slack.id);
    model.variables.push_back(std::move(slack));
    RowData penalty_row;
    penalty_row.constraint = item.penalty_row;
    penalty_row.name = "__repair_side_" + std::to_string(item.penalty_row.id);
    if (item.source_row) {
      auto& original_row = model.rows[item.source_row->id];
      penalty_row.terms = original_row.terms;
      if (item.side == RelaxationSide::Lower) original_row.lower = -infinity;
      else original_row.upper = infinity;
    } else {
      auto& original_variable = model.variables[item.source_variable->id];
      penalty_row.terms.push_back({original_variable.variable, 1.0});
      if (item.side == RelaxationSide::Lower)
        original_variable.lower = original_variable.type == VariableType::Binary ? 0.0 : -infinity;
      else original_variable.upper = original_variable.type == VariableType::Binary ? 1.0 : infinity;
    }
    penalty_row.terms.push_back({item.slack, item.side == RelaxationSide::Lower ? 1.0 : -1.0});
    if (item.side == RelaxationSide::Lower) penalty_row.lower = item.original_bound;
    else penalty_row.upper = item.original_bound;
    model.rows.push_back(std::move(penalty_row));
    built.penalty.terms.push_back({item.slack, item.penalty});
  }
  validate_structure(model);
  return built;
}

long double residuals(const ModelSnapshot& source, const std::vector<double>& values,
                      std::vector<RelaxationItem>& items) {
  Sum total;
  for (auto& item : items) {
    const long double activity = item.source_row ? evaluate(source.rows[item.source_row->id].terms, values)
                                                : values.at(item.source_variable->id);
    // Include the bound before reducing the compensated sum: activity itself
    // can round to the bound and erase a small but material original residual.
    Sum residual;
    const long double sign = item.side == RelaxationSide::Lower ? -1.0L : 1.0L;
    residual.add(-sign * item.original_bound);
    if (item.source_row) {
      for (const auto& term : source.rows[item.source_row->id].terms)
        residual.add(sign * term.coefficient * values.at(term.variable.id));
    } else {
      residual.add(sign * activity);
    }
    const long double violation = std::max(0.0L, residual.value());
    const long double weighted = static_cast<long double>(item.penalty) * violation;
    item.activity = finite(activity); item.violation = finite(violation);
    item.weighted_violation = finite(weighted); item.slack_value = values.at(item.slack.id);
    total.add(weighted);
  }
  return total.value();
}

long double allowance(long double a, long double b, double feasibility, long double multiplier = 1) {
  const auto roundoff = 64.0L * std::numeric_limits<double>::epsilon() *
                        std::max({1.0L, std::fabs(a), std::fabs(b)});
  return std::max(static_cast<long double>(feasibility) * multiplier, roundoff);
}

void verify_layout(const SolveResult& result, const ModelSnapshot& model) {
  if (result.model_id != model.model_id || result.revision != model.revision ||
      result.values.size() != model.variables.size() || result.active_variables.size() != model.variables.size())
    throw InvalidRepair("Repair candidate has inconsistent model identity or slot layout");
  for (std::size_t i = 0; i < model.variables.size(); ++i)
    if (result.active_variables[i] != model.variables[i].active)
      throw InvalidRepair("Repair candidate active mask does not match its private model");
}

double verify_optimum(const SolveResult& result, const ObjectiveData& objective,
                      const ModelSnapshot& model, double feasibility) {
  verify_layout(result, model);
  if (!result.has_solution() || result.termination != Termination::Optimal ||
      result.guarantee != Guarantee::Numerical || !result.best_bound || !std::isfinite(*result.best_bound))
    throw InvalidRepair("Completed repair stage lacks a validated candidate and finite numerical bound");
  const double actual = finite(evaluate(objective.terms, result.values, objective.offset));
  if (std::fabs(static_cast<long double>(actual) - *result.objective) >
      allowance(actual, *result.objective, feasibility))
    throw InvalidRepair("Repair stage objective disagrees with independently recomputed activity");
  auto checked = result;
  checked.objective = actual;
  try { checked.update_gaps(objective.sense); }
  catch (const ModelError&) { throw InvalidRepair("Repair stage global bound has inconsistent ordering"); }
  if (!checked.absolute_gap || !std::isfinite(*checked.absolute_gap) ||
      *checked.absolute_gap > allowance(actual, *checked.best_bound, feasibility))
    throw InvalidRepair("Repair stage has not closed its independently recomputed numerical gap");
  return actual;
}

RelaxationResult run(const ModelSnapshot& source, const RelaxationOptions& options, SolveBudget& budget) {
  RelaxationResult output;
  output.source_model_id = source.model_id; output.source_revision = source.revision;
  auto finish = [&]() {
    if (budget.expired()) {
      output.termination = budget.stop_reason().value_or(Termination::Unknown);
      output.message = "Shared repair budget stopped; no newly checked repair is promoted";
      output.repair_validated = output.minimum_violation_established = output.original_objective_optimized = false;
      output.minimum_weighted_violation.reset();
    }
    output.elapsed_seconds = budget.elapsed_seconds();
    return std::move(output);
  };
  try {
    validate_structure(source);
    for (std::size_t i = 0; i < source.globals.size(); ++i) {
      if (source.globals[i].global.model_id != source.model_id || source.globals[i].global.id != i)
        throw ModelError("Global metadata identity does not match its source model");
      if (source.globals[i].active) throw UnsupportedRepair("Feasibility relaxation of models with active globals is not supported");
    }
    if (options.solve.guarantee != Guarantee::Numerical)
      throw UnsupportedRepair("Weighted feasibility relaxation provides Numerical evidence only");
    if (!options.solve.primal_start.empty())
      throw UnsupportedRepair("Repair primal starts require explicit private slack mapping and are not yet supported");
    if (options.optimize_original_objective && options.solve.node_limit)
      throw UnsupportedRepair("Lexicographic repair node budgets need cumulative consumed-node reporting");
    if (budget.expired()) return finish();
    auto built = construct(source, options);
    const auto penalty = std::move(built.penalty);
    output.private_model = std::move(built.model);
    output.private_variables = std::move(built.mapping);
    output.items = std::move(built.items);
    std::vector<LexicographicObjective> objectives;
    objectives.push_back({penalty, 0.0, 0.0, "minimum weighted violation"});
    if (options.optimize_original_objective)
      objectives.push_back({output.private_model->objective, 0.0, 0.0, "original objective among minimum-violation repairs"});
    auto solve_options = options.solve;
    // Source-model starts lack the private owner/slacks and may be infeasible.
    solve_options.primal_start.clear();
    solve_options.cancellation = budget.cancellation();
    solve_options.time_limit_seconds = budget.remaining_seconds();
    output.workflow = solve_lexicographic(*output.private_model, objectives, solve_options);
    output.termination = output.workflow.termination; output.message = output.workflow.message;
    if (budget.expired()) return finish();
    for (const auto& stage : output.workflow.stages)
      if (stage.result.has_solution()) verify_layout(stage.result, *output.private_model);
    if (output.workflow.completed_stages >= 1) {
      const auto& stage = output.workflow.stages.at(0);
      if (!stage.completed || !validate(*output.private_model, stage.result.values,
          options.solve.feasibility_tolerance, options.solve.integrality_tolerance).valid)
        throw InvalidRepair("Minimum-violation stage failed independent private-model validation");
      const double optimum = verify_optimum(stage.result, penalty, *output.private_model, options.solve.feasibility_tolerance);
      if (optimum < 0) throw InvalidRepair("Nonnegative weighted violation has a negative reported optimum");
      auto first_items = output.items;
      const auto first_residual = residuals(source, stage.result.values, first_items);
      Sum weights; for (const auto& item : output.items) weights.add(item.penalty);
      const auto multiplier = std::max(1.0L, weights.value());
      if (std::fabs(first_residual - optimum) > allowance(first_residual, optimum,
          options.solve.feasibility_tolerance, multiplier))
        throw InvalidRepair("Minimum penalty does not agree with original-unit violations");
      output.minimum_weighted_violation = optimum;
      output.minimum_violation_established = true;
      if (options.optimize_original_objective) {
        const auto exact_activity = evaluate(penalty.terms, stage.result.values);
        double lock = finite(exact_activity);
        if (static_cast<long double>(lock) > exact_activity) lock = std::nextafter(lock, -infinity);
        RowData row;
        row.constraint = {output.private_model->model_id, static_cast<std::uint64_t>(output.private_model->rows.size())};
        row.terms = penalty.terms; row.upper = lock; row.name = "__repair_minimum_violation";
        output.violation_lock = row.constraint;
        output.private_model->rows.push_back(std::move(row));
      }
    }
    if (output.workflow.has_solution()) {
      const auto& result = output.workflow.final_solution;
      verify_layout(result, *output.private_model);
      const auto checked = validate(*output.private_model, result.values,
        options.solve.feasibility_tolerance, options.solve.integrality_tolerance);
      if (!checked.valid || result.model_id != output.private_model->model_id ||
          result.revision != output.private_model->revision)
        throw InvalidRepair("Candidate failed independent validation of hard constraints, penalty rows, or the minimum lock");
      auto items = output.items;
      const auto weighted = residuals(source, result.values, items);
      if (output.minimum_violation_established) {
        Sum weights; for (const auto& item : items) weights.add(item.penalty);
        if (std::fabs(weighted - *output.minimum_weighted_violation) > allowance(weighted,
            *output.minimum_weighted_violation, options.solve.feasibility_tolerance, std::max(1.0L, weights.value())))
          throw InvalidRepair("Final repair does not retain the established minimum violation");
      }
      std::vector<double> original(result.values.begin(), result.values.begin() + source.variables.size());
      auto original_check = validate(source, original, options.solve.feasibility_tolerance,
                                      options.solve.integrality_tolerance);
      for (const auto& item : items) {
        auto& maximum = item.source_row ? original_check.max_row_violation : original_check.max_bound_violation;
        maximum = std::max(maximum, *item.violation);
        if (*item.violation > options.solve.feasibility_tolerance) {
          original_check.valid = false;
          if (original_check.message.empty()) original_check.message = "Selected original sides remain violated";
        }
      }
      output.weighted_violation = finite(weighted);
      output.original_objective = original_check.objective;
      output.original_values = std::move(original);
      output.original_validation = std::move(original_check);
      output.items = std::move(items);
      output.repair_validated = true;
    }
    if (options.optimize_original_objective && output.workflow.completed_numerically() &&
        output.workflow.completed_stages == 2) {
      if (!output.minimum_violation_established || !output.has_repair())
        throw InvalidRepair("Original-objective completion lacks a validated minimum-violation repair");
      verify_optimum(output.workflow.stages.at(1).result, output.private_model->objective,
                      *output.private_model, options.solve.feasibility_tolerance);
      output.original_objective_optimized = true;
    }
    if (output.termination == Termination::Optimal && output.minimum_violation_established)
      output.message = output.original_objective_optimized
        ? "Minimum weighted violation and the original objective within that repair region were established numerically"
        : "Minimum weighted violation established numerically; original feasibility is reported separately";
  } catch (const UnsupportedRepair& error) {
    output.termination = Termination::Unsupported; output.message = error.what();
  } catch (const InvalidRepair& error) {
    output.termination = Termination::NumericalFailure; output.message = error.what();
    output.minimum_violation_established = output.original_objective_optimized = false;
    output.minimum_weighted_violation.reset();
  } catch (const ModelError& error) {
    output.termination = Termination::InvalidModel; output.message = error.what();
  } catch (const std::bad_alloc&) {
    output.termination = Termination::MemoryLimit; output.message = "Repair allocation failed";
  } catch (const std::exception& error) {
    output.termination = Termination::BackendError; output.message = error.what();
  }
  return finish();
}

RelaxationResult failure(ModelId id, Revision revision, Termination reason, const std::string& message) {
  RelaxationResult result;
  result.source_model_id = id; result.source_revision = revision;
  result.termination = reason; result.message = message;
  return result;
}
}

RelaxationResult relax_feasibility(const ModelSnapshot& model, const RelaxationOptions& options) {
  try { SolveBudget budget(options.solve); return run(model, options, budget); }
  catch (const ModelError& error) { return failure(model.model_id, model.revision, Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return failure(model.model_id, model.revision, Termination::MemoryLimit, "Repair allocation failed"); }
}
RelaxationResult relax_feasibility(const Model& model, const RelaxationOptions& options) {
  try { SolveBudget budget(options.solve); return run(model.snapshot(), options, budget); }
  catch (const ModelError& error) { return failure(model.id(), model.revision(), Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return failure(model.id(), model.revision(), Termination::MemoryLimit, "Repair allocation failed"); }
}

}}
