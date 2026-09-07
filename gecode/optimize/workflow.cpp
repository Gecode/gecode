#include <gecode/optimize/workflow.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
class UnsupportedWorkflow : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

struct ObjectiveValue {
  long double linear;
  long double total;
};

class ObjectiveSum {
  long double sum_ = 0.0L;
  long double correction_ = 0.0L;
public:
  void add(long double value) {
    const long double next = sum_ + value;
    if (!std::isfinite(value) || !std::isfinite(next))
      throw UnsupportedWorkflow("Objective accumulation exceeds supported precision");
    const long double correction = std::fabs(sum_) >= std::fabs(value)
      ? (sum_ - next) + value : (value - next) + sum_;
    const long double corrected = correction_ + correction;
    if (!std::isfinite(corrected))
      throw UnsupportedWorkflow("Objective accumulation exceeds supported precision");
    sum_ = next;
    correction_ = corrected;
  }
  long double value() const {
    const long double result = sum_ + correction_;
    if (!std::isfinite(result))
      throw UnsupportedWorkflow("Objective accumulation exceeds supported precision");
    return result;
  }
};

ObjectiveValue evaluate(const ObjectiveData& objective,
                        const std::vector<double>& values) {
  ObjectiveSum linear, total;
  // Include the offset before collapsing the compensated terms. A large
  // linear subtotal can otherwise lose a small residual before cancellation
  // against the offset, even when the final objective is representable.
  total.add(static_cast<long double>(objective.offset));
  for (const auto& term : objective.terms) {
    const long double term_value = static_cast<long double>(term.coefficient) *
                                   values[term.variable.id];
    linear.add(term_value);
    total.add(term_value);
  }
  return {linear.value(), total.value()};
}

double finite_double(long double value, const char* what) {
  if (!std::isfinite(value) ||
      std::fabs(value) > static_cast<long double>(std::numeric_limits<double>::max()))
    throw UnsupportedWorkflow(std::string(what) + " exceeds finite double range");
  return static_cast<double>(value);
}

// Round toward a tighter feasible region so conversion never grants additional
// degradation. Candidate validation detects an unrepresentable restrictive lock.
double row_bound(long double value, ObjectiveSense sense) {
  double converted = finite_double(value, "Objective lock");
  if (sense == ObjectiveSense::Minimize && static_cast<long double>(converted) > value)
    converted = std::nextafter(converted, -std::numeric_limits<double>::infinity());
  if (sense == ObjectiveSense::Maximize && static_cast<long double>(converted) < value)
    converted = std::nextafter(converted, std::numeric_limits<double>::infinity());
  if (!std::isfinite(converted))
    throw UnsupportedWorkflow("Objective lock cannot be represented conservatively");
  return converted;
}

struct Retention {
  ObjectiveData objective;
  long double bound;
};

bool retained(const std::vector<Retention>& locks,
              const std::vector<double>& values, double tolerance) {
  for (const auto& lock : locks) {
    const long double actual = evaluate(lock.objective, values).total;
    const long double violation = lock.objective.sense == ObjectiveSense::Minimize
      ? actual - lock.bound : lock.bound - actual;
    if (violation > static_cast<long double>(tolerance)) return false;
  }
  return true;
}

bool bound_agrees(const SolveResult& result, double actual,
                  ObjectiveSense sense, double tolerance) {
  if (!result.objective || !result.best_bound || !std::isfinite(*result.best_bound))
    return false;
  const double scale = std::max({1.0, std::fabs(actual),
                                std::fabs(*result.objective), std::fabs(*result.best_bound)});
  const double allowance = std::max(tolerance,
    64.0 * std::numeric_limits<double>::epsilon() * scale);
  if (std::fabs(static_cast<long double>(actual) - *result.objective) > allowance)
    return false;
  // Recompute from independent activity and the actual bound. A backend's
  // cached zero gap must not establish completion after its values change.
  auto checked = result;
  checked.objective = actual;
  try { checked.update_gaps(sense); }
  catch (const ModelError&) { return false; }
  return checked.absolute_gap && std::isfinite(*checked.absolute_gap) &&
         *checked.absolute_gap <= allowance;
}

void clear_late_solution(SolveResult& result, Termination reason) {
  result.termination = reason;
  result.message = "Stage returned after the shared workflow budget stopped";
  result.solution_validated = false;
  result.values.clear();
  result.objective.reset();
  result.best_bound.reset();
  result.absolute_gap.reset();
  result.relative_gap.reset();
  result.native_backend_gap.reset();
}

LexicographicResult run(const ModelSnapshot& original,
                       const std::vector<LexicographicObjective>& objectives,
                       const SolveOptions& options, SolveBudget& budget) {
  LexicographicResult output;
  output.model_id = original.model_id;
  output.revision = original.revision;
  auto finish = [&]() {
    output.elapsed_seconds = budget.elapsed_seconds();
    if (output.termination == Termination::Optimal && budget.expired()) {
      output.termination = budget.stop_reason().value_or(Termination::Unknown);
      output.message = "Shared workflow budget stopped before completion";
    }
    return std::move(output);
  };
  try {
    validate_structure(original);
    if (objectives.empty()) throw ModelError("At least one lexicographic objective is required");
    if (options.guarantee != Guarantee::Numerical)
      throw UnsupportedWorkflow("Lexicographic workflow supports Numerical guarantees only");
    if (objectives.size() > 1 && options.node_limit)
      throw UnsupportedWorkflow("Multi-stage node budgets need consumed-node reporting");
    if (budget.expired()) {
      output.termination = budget.stop_reason().value_or(Termination::Unknown);
      return finish();
    }
    ModelSnapshot working = original;
    for (const auto& objective : objectives) {
      if (!std::isfinite(objective.absolute_degradation) || objective.absolute_degradation < 0.0 ||
          !std::isfinite(objective.relative_degradation) || objective.relative_degradation < 0.0)
        throw ModelError("Objective degradation must be nonnegative and finite");
      working.objective = objective.objective;
      validate_structure(working); // reject later malformed objectives before solving
      if (budget.expired()) {
        output.termination = budget.stop_reason().value_or(Termination::Unknown);
        return finish();
      }
    }
    std::vector<Retention> locks;
    for (std::size_t i = 0; i < objectives.size(); ++i) {
      if (budget.expired()) {
        output.termination = budget.stop_reason().value_or(Termination::Unknown);
        return finish();
      }
      working.objective = objectives[i].objective;
      auto stage_options = options;
      stage_options.cancellation = budget.cancellation();
      stage_options.time_limit_seconds = budget.remaining_seconds();
      stage_options.relative_gap = 0.0;
      stage_options.absolute_gap = 0.0;
      if (i != 0) stage_options.primal_start.clear();
      LexicographicStage stage;
      stage.index = i;
      stage.name = objectives[i].name;
      stage.result = solve(working, stage_options);
      if (budget.expired()) {
        const auto reason = budget.stop_reason().value_or(Termination::Unknown);
        clear_late_solution(stage.result, reason);
        output.stages.push_back(std::move(stage));
        output.termination = reason;
        output.message = "Shared workflow budget stopped during stage " + std::to_string(i);
        return finish();
      }
      output.stages.push_back(std::move(stage));
      auto& current = output.stages.back();
      auto& result = current.result;
      std::optional<double> stage_objective;

      // Even an incomplete stage can supply an independently checked incumbent.
      // Never accept a candidate merely because a previous phase accepted it.
      if (result.has_solution()) {
        const auto original_check = validate(original, result.values,
          options.feasibility_tolerance, options.integrality_tolerance);
        const auto lock_check = validate(working, result.values,
          options.feasibility_tolerance, options.integrality_tolerance);
        if (!original_check.valid || !lock_check.valid ||
            !retained(locks, result.values, options.feasibility_tolerance)) {
          output.termination = Termination::NumericalFailure;
          output.message = "Independent original-model or objective-lock validation failed";
          return finish();
        }
        std::vector<double> vector_values;
        vector_values.reserve(objectives.size());
        for (const auto& objective : objectives)
          vector_values.push_back(finite_double(evaluate(objective.objective, result.values).total,
                                               "Reported objective"));
        stage_objective = vector_values[i];
        if (budget.expired()) {
          output.termination = budget.stop_reason().value_or(Termination::Unknown);
          clear_late_solution(result, output.termination);
          output.message = "Shared workflow budget stopped during candidate validation";
          return finish();
        }
        output.final_solution = result;
        output.final_solution.model_id = original.model_id;
        output.final_solution.revision = original.revision;
        output.final_solution.objective = original_check.objective;
        output.final_solution.best_bound.reset();
        output.final_solution.absolute_gap.reset();
        output.final_solution.relative_gap.reset();
        output.final_solution.native_backend_gap.reset();
        output.final_solution.termination = Termination::Unknown;
        output.final_solution.message = "Validated original-model feasible solution; see workflow stages";
        output.objective_values = std::move(vector_values);
      }

      if (result.termination != Termination::Optimal) {
        output.termination = result.termination;
        output.message = "Lexicographic stage " + std::to_string(i) + " stopped: " + result.message;
        if (result.termination == Termination::Infeasible && output.has_solution()) {
          output.termination = Termination::NumericalFailure;
          output.message = "Stage reported infeasible despite a validated retained solution";
        }
        return finish();
      }
      if (!result.has_solution() || result.guarantee != Guarantee::Numerical ||
          !stage_objective || !bound_agrees(result, *stage_objective,
            working.objective.sense, options.feasibility_tolerance)) {
        output.termination = Termination::NumericalFailure;
        output.message = "Stage optimum lacks a validated solution and agreeing numerical global bound";
        return finish();
      }

      if (i + 1 < objectives.size()) {
        const auto value = evaluate(working.objective, result.values);
        const long double degradation = objectives[i].absolute_degradation +
          static_cast<long double>(objectives[i].relative_degradation) * std::fabs(value.total);
        if (!std::isfinite(degradation))
          throw UnsupportedWorkflow("Objective degradation exceeds supported precision");
        const long double change = working.objective.sense == ObjectiveSense::Minimize
          ? degradation : -degradation;
        const long double threshold = value.total + change;
        // Direct linear evaluation avoids cancelling a large objective offset
        // against a rounded reported objective value when forming the row.
        const double bound = row_bound(value.linear + change, working.objective.sense);
        current.retention_bound = finite_double(threshold, "Objective retention threshold");
        RowData row;
        row.constraint = {working.model_id, static_cast<std::uint64_t>(working.rows.size())};
        row.terms = working.objective.terms;
        row.name = "__lexicographic_lock_" + std::to_string(i);
        if (working.objective.sense == ObjectiveSense::Minimize) row.upper = bound;
        else row.lower = bound;
        working.rows.push_back(std::move(row));
        locks.push_back({working.objective, threshold});
        const auto retained_check = validate(working, result.values,
          options.feasibility_tolerance, options.integrality_tolerance);
        if (!retained_check.valid || !retained(locks, result.values, options.feasibility_tolerance))
          throw UnsupportedWorkflow("Objective retention lock loses its incumbent at available precision");
      }
      if (budget.expired()) {
        output.termination = budget.stop_reason().value_or(Termination::Unknown);
        output.message = "Shared workflow budget stopped before stage promotion";
        return finish();
      }
      current.completed = true;
      ++output.completed_stages;
    }
    output.termination = Termination::Optimal;
    output.message = "All ordered stages completed numerically with requested degradation and feasibility tolerances";
  } catch (const UnsupportedWorkflow& e) {
    output.termination = Termination::Unsupported;
    output.message = e.what();
  } catch (const ModelError& e) {
    output.termination = Termination::InvalidModel;
    output.message = e.what();
  } catch (const std::bad_alloc&) {
    output.termination = Termination::MemoryLimit;
    output.message = "Workflow allocation failed";
  } catch (const std::exception& e) {
    output.termination = Termination::BackendError;
    output.message = e.what();
  }
  return finish();
}

LexicographicResult invalid_options(const ModelId id, const Revision revision,
                                    const std::exception& error) {
  LexicographicResult result;
  result.model_id = id;
  result.revision = revision;
  result.termination = Termination::InvalidModel;
  result.message = error.what();
  return result;
}
} // namespace

LexicographicResult solve_lexicographic(
  const ModelSnapshot& model, const std::vector<LexicographicObjective>& objectives,
  const SolveOptions& options) {
  try {
    SolveBudget budget(options);
    return run(model, objectives, options, budget);
  } catch (const ModelError& e) {
    return invalid_options(model.model_id, model.revision, e);
  }
}

LexicographicResult solve_lexicographic(
  const Model& model, const std::vector<LexicographicObjective>& objectives,
  const SolveOptions& options) {
  try {
    SolveBudget budget(options); // includes owning snapshot copy in the deadline
    return run(model.snapshot(), objectives, options, budget);
  } catch (const ModelError& e) {
    return invalid_options(model.id(), model.revision(), e);
  }
}

}}
