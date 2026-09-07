#include <gecode/optimize/pool.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
constexpr std::int64_t exact_limit = INT64_C(9007199254740992);
class UnsupportedPool : public std::runtime_error { using std::runtime_error::runtime_error; };
class InvalidPool : public std::runtime_error { using std::runtime_error::runtime_error; };
struct Interrupted { Termination reason; };
void checkpoint(const SolveBudget& budget) {
  if (auto reason = budget.stop_reason()) throw Interrupted{*reason};
}
struct Domain { Variable variable; std::int64_t lower, upper; };

std::vector<Domain> projection(const ModelSnapshot& source, const PoolOptions& options) {
  for (const auto& indicator : source.indicators)
    if (indicator.active) throw UnsupportedPool("Pools with active indicators are not yet supported");
  for (const auto& global : source.globals)
    if (global.active) throw UnsupportedPool("Pools with active globals are not yet supported");
  std::size_t active = 0;
  std::vector<Variable> variables;
  for (const auto& variable : source.variables) if (variable.active) {
    ++active;
    if (variable.type == VariableType::SemiContinuous || variable.type == VariableType::SemiInteger)
      throw UnsupportedPool("Pools with semi-variable domains are not yet supported");
    if (!options.projection && variable.type != VariableType::Continuous)
      variables.push_back(variable.variable);
  }
  if (options.projection) variables = *options.projection;
  if (variables.empty() && active != 0)
    throw UnsupportedPool("A pool requires a nonempty finite discrete projection; continuous-only enumeration is undefined");
  std::set<std::uint64_t> seen;
  std::vector<Domain> domains;
  for (auto handle : variables) {
    if (handle.model_id != source.model_id || handle.id >= source.variables.size() ||
        !source.variables[handle.id].active) throw ModelError("Pool projection contains a foreign, absent, or deleted variable");
    if (!seen.insert(handle.id).second) throw ModelError("Pool projection repeats a variable");
    const auto& data = source.variables[handle.id];
    if (data.type != VariableType::Integer && data.type != VariableType::Binary)
      throw UnsupportedPool("Projected variables must be Integer or Binary");
    if (!std::isfinite(data.lower) || !std::isfinite(data.upper))
      throw UnsupportedPool("Projected variables require finite bounds");
    const double lower = std::ceil(data.lower), upper = std::floor(data.upper);
    if (std::fabs(lower) > exact_limit || std::fabs(upper) > exact_limit)
      throw UnsupportedPool("Projected integer endpoints exceed exact double representation");
    const auto lo = static_cast<std::int64_t>(lower), hi = static_cast<std::int64_t>(upper);
    if (hi >= lo && hi - lo > exact_limit)
      throw UnsupportedPool("Projected domain width exceeds exact no-good coefficient representation");
    domains.push_back({handle, lo, hi});
  }
  return domains;
}

void layout(const SolveResult& result, const ModelSnapshot& model) {
  if (result.model_id != model.model_id || result.revision != model.revision ||
      result.values.size() != model.variables.size() || result.active_variables.size() != model.variables.size())
    throw InvalidPool("Pool oracle returned inconsistent identity or slot layout");
  for (std::size_t i = 0; i < model.variables.size(); ++i)
    if (result.active_variables[i] != model.variables[i].active)
      throw InvalidPool("Pool oracle returned an inconsistent active-variable mask");
}

std::vector<double> canonical(const SolveResult& result, const ModelSnapshot& model,
                              const SolveOptions& options) {
  layout(result, model);
  auto values = result.values;
  if (!validate(model, values, options.feasibility_tolerance, options.integrality_tolerance).valid)
    throw InvalidPool("Pool candidate failed independent remaining-model validation");
  for (const auto& variable : model.variables) if (variable.active && variable.type != VariableType::Continuous) {
    auto& value = values[variable.variable.id];
    if (std::fabs(value) > exact_limit)
      throw UnsupportedPool("Integer pool values exceed exact canonical representation");
    value = std::round(value);
    if (value < variable.lower || value > variable.upper)
      throw InvalidPool("Canonical integer pool value is outside its exact original interval");
  }
  if (!validate(model, values, options.feasibility_tolerance, 0).valid)
    throw InvalidPool("Rounded pool candidate violates hard constraints or no-good exclusions");
  return values;
}

double allowance(double a, double b, const SolveOptions& options) {
  if (options.guarantee == Guarantee::Exact) return 0;
  return std::max(options.feasibility_tolerance, 64 * std::numeric_limits<double>::epsilon() *
                  std::max({1.0, std::fabs(a), std::fabs(b)}));
}
void optimum(const SolveResult& result, double objective, ObjectiveSense sense,
             const SolveOptions& options) {
  if (result.guarantee != options.guarantee || !result.best_bound || !std::isfinite(*result.best_bound))
    throw InvalidPool("Pool rank lacks a matching guarantee and finite global bound");
  if (!result.objective || std::fabs(*result.objective - objective) > allowance(*result.objective, objective, options))
    throw InvalidPool("Pool objective disagrees with the independently recomputed original objective");
  auto checked = result;
  checked.objective = objective;
  try { checked.update_gaps(sense); }
  catch (const ModelError&) { throw InvalidPool("Pool global bound has inconsistent ordering"); }
  if (!checked.absolute_gap || !std::isfinite(*checked.absolute_gap) ||
      *checked.absolute_gap > allowance(objective, *checked.best_bound, options))
    throw InvalidPool("Pool rank lacks a closed independently recomputed gap");
}

void exclude(ModelSnapshot& model, const std::vector<Domain>& domains,
             const std::vector<std::int64_t>& values) {
  std::vector<Term> witnesses;
  auto witness = [&](const Domain& domain, std::int64_t coefficient, bool below) {
    if (coefficient <= 0 || coefficient > exact_limit)
      throw UnsupportedPool("No-good coefficient is not a positive exactly representable integer");
    if (model.variables.size() >= std::numeric_limits<std::uint64_t>::max() ||
        model.rows.size() >= std::numeric_limits<std::uint64_t>::max())
      throw UnsupportedPool("Pool no-good exceeds handle capacity");
    VariableData binary;
    binary.variable = {model.model_id, static_cast<std::uint64_t>(model.variables.size())};
    binary.type = VariableType::Binary; binary.lower = 0; binary.upper = 1;
    binary.name = "__pool_witness_" + std::to_string(binary.variable.id);
    model.variables.push_back(binary);
    RowData row;
    row.constraint = {model.model_id, static_cast<std::uint64_t>(model.rows.size())};
    row.terms = {{domain.variable, 1}, {binary.variable, below ? double(coefficient) : -double(coefficient)}};
    if (below) row.upper = double(domain.upper);
    else row.lower = double(domain.lower);
    row.name = "__pool_implication_" + std::to_string(row.constraint.id);
    model.rows.push_back(std::move(row));
    witnesses.push_back({binary.variable, 1});
  };
  for (std::size_t i = 0; i < domains.size(); ++i) {
    const auto& domain = domains[i]; const auto value = values[i];
    if (value < domain.lower || value > domain.upper)
      throw InvalidPool("Canonical projection value is outside its original integer domain");
    if (value > domain.lower) witness(domain, domain.upper - value + 1, true);
    if (value < domain.upper) witness(domain, value + 1 - domain.lower, false);
  }
  if (model.rows.size() >= std::numeric_limits<std::uint64_t>::max())
    throw UnsupportedPool("Pool no-good exceeds row capacity");
  RowData disjunction;
  disjunction.constraint = {model.model_id, static_cast<std::uint64_t>(model.rows.size())};
  disjunction.terms = std::move(witnesses); disjunction.lower = 1;
  disjunction.name = "__pool_exclusion_" + std::to_string(disjunction.constraint.id);
  model.rows.push_back(std::move(disjunction));
  validate_structure(model);
}

PoolResult run(const ModelSnapshot& original, const PoolOptions& options, SolveBudget& budget) {
  PoolResult output;
  output.model_id = original.model_id; output.revision = original.revision;
  output.guarantee = options.solve.guarantee;
  auto finish = [&]() {
    if (auto reason = budget.stop_reason()) {
      output.termination = *reason; output.completion = PoolCompletion::Incomplete;
      output.message = "Shared pool budget stopped; previously accepted entries remain historical results";
    }
    output.elapsed_seconds = budget.elapsed_seconds(); return std::move(output);
  };
  try {
    validate_structure(original);
    if (options.max_solutions == 0) throw ModelError("Pool max_solutions must be positive");
    if (options.solve.guarantee == Guarantee::Certified)
      throw UnsupportedPool("Certified pool enumeration is not implemented");
    if (options.solve.guarantee == Guarantee::Exact && options.solve.backend != Backend::Native)
      throw UnsupportedPool("Exact pool enumeration requires the explicit Native backend");
    if (options.max_solutions > 1 && options.solve.node_limit)
      throw UnsupportedPool("Multi-solve pool node budgets require cumulative consumed-node reporting");
    const auto domains = projection(original, options);
    for (const auto& domain : domains) output.projection.push_back(domain.variable);
    checkpoint(budget);
    auto working = original;
    std::set<std::vector<std::int64_t>> seen;
    for (;;) {
      checkpoint(budget);
      auto solve_options = options.solve;
      solve_options.cancellation = budget.cancellation();
      solve_options.time_limit_seconds = budget.remaining_seconds();
      solve_options.relative_gap = solve_options.absolute_gap = 0;
      if (!output.attempts.empty()) solve_options.primal_start.clear();
      auto result = solve(working, solve_options);
      output.attempts.push_back({result.termination, result.guarantee, result.objective, result.best_bound, false, false});
      checkpoint(budget);
      auto& attempt = output.attempts.back();
      if (result.model_id != working.model_id || result.revision != working.revision)
        throw InvalidPool("Pool oracle result belongs to a different model or revision");
      if ((result.objective && !std::isfinite(*result.objective)) ||
          (result.best_bound && std::isnan(*result.best_bound)))
        throw InvalidPool("Pool oracle returned non-finite objective or NaN bound evidence");
      if (result.solution_validated && !result.has_solution())
        throw InvalidPool("Pool oracle claimed a malformed validated solution");
      ValidationReport raw_check;
      if ((output.ranked_prefix != 0 || result.termination == Termination::Infeasible) &&
          result.values.size() == working.variables.size())
        raw_check = validate(working, result.values, options.solve.feasibility_tolerance,
                             options.solve.integrality_tolerance);
      if (raw_check.valid && raw_check.objective && output.ranked_prefix != 0) {
        const auto previous = *output.entries.back().solution.objective;
        const auto improvement = original.objective.sense == ObjectiveSense::Minimize
          ? previous - *raw_check.objective : *raw_check.objective - previous;
        if (improvement > allowance(previous, *raw_check.objective, options.solve)) {
          for (auto& entry : output.entries) entry.rank_established = false;
          for (auto& prior : output.attempts) prior.rank_established = false;
          output.ranked_prefix = 0;
          throw InvalidPool("An independently feasible raw witness disproves earlier pool ranks");
        }
      }
      if (result.termination == Termination::Infeasible) {
        if (result.has_solution() || raw_check.valid || result.guarantee != options.solve.guarantee)
          throw InvalidPool("Pool infeasibility conflicts with an independently feasible raw candidate or requested evidence guarantee");
        output.termination = Termination::Optimal;
        output.completion = PoolCompletion::Exhausted;
        output.message = "All feasible discrete projection classes exhausted under the requested evidence guarantee";
        return finish();
      }
      if (result.has_solution()) {
        if (result.guarantee != options.solve.guarantee)
          throw InvalidPool("Pool candidate does not provide the requested evidence guarantee");
        auto values = canonical(result, working, options.solve);
        std::vector<double> original_values(values.begin(), values.begin() + original.variables.size());
        auto checked = validate(original, original_values, options.solve.feasibility_tolerance, 0);
        if (!checked.valid || !checked.objective || !std::isfinite(*checked.objective))
          throw InvalidPool("Pool candidate failed independent original-model validation");
        std::vector<std::int64_t> key;
        for (const auto& domain : domains) key.push_back(static_cast<std::int64_t>(values[domain.variable.id]));
        if (seen.count(key)) throw InvalidPool("Pool oracle repeated an already excluded projection");
        const bool ranked = result.termination == Termination::Optimal;
        if (output.ranked_prefix != 0) {
          const auto previous = *output.entries.back().solution.objective;
          const auto tolerance = allowance(previous, *checked.objective, options.solve);
          const auto improvement = original.objective.sense == ObjectiveSense::Minimize
            ? previous - *checked.objective : *checked.objective - previous;
          if (improvement > tolerance) {
            // A feasible better remaining point disproves prior ranking even
            // when this new solve is interrupted. Retain feasible values only.
            for (auto& entry : output.entries) entry.rank_established = false;
            for (auto& prior : output.attempts) prior.rank_established = false;
            output.ranked_prefix = 0;
            throw InvalidPool("A later feasible pool candidate disproves earlier rank evidence");
          }
        }
        if (ranked) {
          optimum(result, *checked.objective, original.objective.sense, options.solve);
        } else if (result.termination != Termination::TimeLimit && result.termination != Termination::NodeLimit &&
                   result.termination != Termination::MemoryLimit && result.termination != Termination::IterationLimit && result.termination != Termination::SolutionLimit &&
                   result.termination != Termination::ObjectiveLimit && result.termination != Termination::Cancelled) {
          output.termination = result.termination; output.message = result.message;
          return finish();
        }
        SolveResult historical = result;
        historical.model_id = original.model_id; historical.revision = original.revision;
        historical.values = std::move(original_values);
        historical.active_variables.resize(original.variables.size());
        historical.objective = checked.objective; historical.solution_validated = true;
        historical.best_bound.reset(); historical.absolute_gap.reset(); historical.relative_gap.reset();
        historical.native_backend_gap.reset(); historical.termination = Termination::Unknown;
        historical.message = "Validated original-model pool representative; see its pool rank evidence";
        checkpoint(budget);
        seen.insert(key);
        output.entries.push_back({std::move(historical), key, ranked});
        if (budget.expired()) { output.entries.pop_back(); checkpoint(budget); }
        attempt.candidate_accepted = true; attempt.rank_established = ranked;
        if (ranked) ++output.ranked_prefix;
        if (!ranked) {
          output.termination = result.termination;
          output.message = "A timely validated final candidate is unranked because its solve did not establish optimality";
          return finish();
        }
        if (output.entries.size() == options.max_solutions) {
          output.termination = Termination::SolutionLimit; output.completion = PoolCompletion::RequestedLimit;
          output.message = "Requested number of ranked projection representatives reached; exhaustion was not tested";
          return finish();
        }
        exclude(working, domains, key);
      } else {
        if (result.termination == Termination::Optimal)
          throw InvalidPool("Pool optimum lacks a validated candidate");
        output.termination = result.termination; output.message = result.message;
        return finish();
      }
    }
  } catch (const Interrupted& stopped) {
    output.termination = stopped.reason;
  } catch (const UnsupportedPool& error) {
    output.termination = Termination::Unsupported; output.message = error.what();
  } catch (const InvalidPool& error) {
    output.termination = Termination::NumericalFailure; output.message = error.what();
  } catch (const ModelError& error) {
    output.termination = Termination::InvalidModel; output.message = error.what();
  } catch (const std::bad_alloc&) {
    output.termination = Termination::MemoryLimit; output.message = "Pool allocation failed";
  } catch (const std::exception& error) {
    output.termination = Termination::BackendError; output.message = error.what();
  }
  return finish();
}
PoolResult failure(ModelId id, Revision revision, Guarantee guarantee, Termination reason, const std::string& message) {
  PoolResult output; output.model_id = id; output.revision = revision;
  output.guarantee = guarantee; output.termination = reason; output.message = message; return output;
}
}
PoolResult solve_pool(const ModelSnapshot& model, const PoolOptions& options) {
  try { SolveBudget budget(options.solve); return run(model, options, budget); }
  catch (const ModelError& error) { return failure(model.model_id, model.revision, options.solve.guarantee, Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return failure(model.model_id, model.revision, options.solve.guarantee, Termination::MemoryLimit, "Allocation failed"); }
}
PoolResult solve_pool(const Model& model, const PoolOptions& options) {
  try { SolveBudget budget(options.solve); return run(model.snapshot(), options, budget); }
  catch (const ModelError& error) { return failure(model.id(), model.revision(), options.solve.guarantee, Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return failure(model.id(), model.revision(), options.solve.guarantee, Termination::MemoryLimit, "Allocation failed"); }
}
}}
