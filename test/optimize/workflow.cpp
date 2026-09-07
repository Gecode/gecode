#include <gecode/optimize/workflow.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

using namespace Gecode::Optimize;

#ifdef GECODE_WORKFLOW_TEST_FAKE_SOLVER

namespace {
enum class Scenario { Incomplete, CancelDuringSolve, BadLock, MissingBound, FalseInfeasible,
  CancellationResidual, ForgedCancellationObjective, ForgedCancellationGap };
Scenario scenario = Scenario::Incomplete;
unsigned calls = 0;
double previous_remaining = std::numeric_limits<double>::infinity();
}

// Compile this test mode with model/result/validate/workflow.cpp, without the
// real solve.cpp. Deterministic stops exercise the coordinator without sleeping
// or relying on solver/runtime speed. It also distrusts a malformed candidate.
namespace Gecode { namespace Optimize {
SolveResult solve(const ModelSnapshot& model, const SolveOptions& options) {
  assert(options.relative_gap == 0 && options.absolute_gap == 0);
  assert(options.time_limit_seconds <= previous_remaining);
  previous_remaining = options.time_limit_seconds;
  if (calls != 0) {
    assert(options.primal_start.empty());
    assert(model.rows.size() == calls); // one independently checked prior lock
  }
  if (scenario == Scenario::CancellationResidual ||
      scenario == Scenario::ForgedCancellationObjective ||
      scenario == Scenario::ForgedCancellationGap) {
    assert(calls == 0 && model.variables.size() == 2);
    SolveResult result;
    result.model_id = model.model_id; result.revision = model.revision;
    result.values = {1, 1}; result.active_variables = {true, true};
    const auto checked = validate(model, result.values);
    // This fixture's exact objective is 1 + big - big = 1. Its
    // coefficient/offset scale exceeds even an x87 long-double significand.
    assert(checked.valid && checked.objective == 1);
    result.solution_validated = true;
    result.objective = 1; result.best_bound = 1;
    result.update_gaps(model.objective.sense);
    result.termination = Termination::Optimal;
    if (scenario == Scenario::ForgedCancellationObjective) result.objective = 0;
    if (scenario != Scenario::CancellationResidual) result.best_bound = 0;
    // The forged cases deliberately retain the prior cached zero gap.
    ++calls;
    return result;
  }
  SolveResult result;
  result.model_id = model.model_id;
  result.revision = model.revision;
  result.backend = "deterministic test backend";
  result.values = {calls == 0 ? 0.0 : 1.0};
  if (scenario != Scenario::BadLock) result.values[0] = 0.0;
  for (const auto& variable : model.variables) result.active_variables.push_back(variable.active);
  const auto report = validate(model, result.values);
  result.solution_validated = scenario == Scenario::BadLock || report.valid;
  result.objective = calls == 0 ? 0.0 : result.values[0];
  result.best_bound = result.objective;
  result.update_gaps(model.objective.sense);
  result.termination = Termination::Optimal;
  if (scenario == Scenario::MissingBound) {
    result.best_bound.reset();
    result.absolute_gap.reset();
    result.relative_gap.reset();
  }
  if (scenario == Scenario::CancelDuringSolve) options.cancellation->cancel();
  if (calls == 1 && scenario == Scenario::Incomplete)
    result.termination = Termination::TimeLimit;
  if (calls == 1 && scenario == Scenario::FalseInfeasible) {
    result.termination = Termination::Infeasible;
    result.solution_validated = false;
    result.objective.reset();
    result.values.clear();
  }
  ++calls;
  return result;
}
}}

int main() {
  Model model;
  auto x = model.add_binary();
  LexicographicObjective first;
  first.objective.terms = {{x, 1}};
  auto second = first;
  second.objective.sense = ObjectiveSense::Maximize;
  const std::vector<LexicographicObjective> objectives{first, second, first};
  for (Scenario selected : {Scenario::Incomplete, Scenario::CancelDuringSolve,
                            Scenario::BadLock, Scenario::MissingBound, Scenario::FalseInfeasible}) {
    scenario = selected;
    calls = 0;
    previous_remaining = std::numeric_limits<double>::infinity();
    SolveOptions options;
    options.time_limit_seconds = 60;
    options.primal_start = {{x, 1}};
    auto result = solve_lexicographic(model, objectives, options);
    assert(!result.completed_numerically());
    if (selected == Scenario::CancelDuringSolve) {
      assert(calls == 1 && result.termination == Termination::Cancelled);
      assert(!result.has_solution() && result.completed_stages == 0);
      assert(!result.stages[0].result.has_solution());
    } else if (selected == Scenario::MissingBound) {
      assert(calls == 1 && result.termination == Termination::NumericalFailure);
      assert(result.has_solution() && result.completed_stages == 0);
    } else {
      assert(calls == 2 && result.completed_stages == 1);
      assert(result.stages.size() == 2 && !result.stages.back().completed);
      assert(result.has_solution() && result.final_solution.value(x) == 0);
      assert(result.termination == (selected == Scenario::Incomplete
        ? Termination::TimeLimit : Termination::NumericalFailure));
    }
  }
  for (auto selected : {Scenario::CancellationResidual,
                        Scenario::ForgedCancellationObjective, Scenario::ForgedCancellationGap}) {
    scenario = selected; calls = 0;
    previous_remaining = std::numeric_limits<double>::infinity();
    Model cancellation;
    auto residual = cancellation.add_continuous(1, 1);
    auto fixed = cancellation.add_continuous(1, 1);
    const double big = std::ldexp(1.0, std::numeric_limits<long double>::digits + 2);
    assert(std::isfinite(big));
    cancellation.minimize({{residual, 1}, {fixed, big}}, -big);
    LexicographicObjective objective;
    objective.objective = cancellation.snapshot().objective;
    auto result = solve_lexicographic(cancellation, {objective});
    assert(calls == 1 && result.has_solution());
    assert(result.final_solution.objective == 1);
    assert(result.objective_values == std::vector<double>{1});
    if (selected == Scenario::CancellationResidual) {
      assert(result.completed_numerically() && result.completed_stages == 1);
    } else {
      assert(result.termination == Termination::NumericalFailure);
      assert(!result.completed_numerically() && result.completed_stages == 0);
    }
  }
}

#else

namespace {
const double infinity = std::numeric_limits<double>::infinity();

LexicographicObjective objective(Variable variable, ObjectiveSense sense,
                                  double offset = 0.0) {
  LexicographicObjective objective;
  objective.objective = {{{variable, 1.0}}, offset, sense};
  return objective;
}

double value(const ObjectiveData& objective, const std::vector<double>& point) {
  double total = objective.offset;
  for (const auto& term : objective.terms)
    total += term.coefficient * point[term.variable.id];
  return total;
}

struct Oracle {
  std::vector<double> stage_optima;
  std::vector<std::vector<double>> final_points;
};

// Independent exhaustive arithmetic oracle: input points are generated by
// explicit test-case inequalities, not by the solver or its model validator.
Oracle enumerate(std::vector<std::vector<double>> points,
                 const std::vector<LexicographicObjective>& objectives) {
  Oracle oracle;
  for (const auto& descriptor : objectives) {
    const auto& objective = descriptor.objective;
    double best = objective.sense == ObjectiveSense::Minimize ? infinity : -infinity;
    for (const auto& point : points) {
      const auto candidate = value(objective, point);
      best = objective.sense == ObjectiveSense::Minimize
        ? std::min(best, candidate) : std::max(best, candidate);
    }
    assert(std::isfinite(best));
    oracle.stage_optima.push_back(best);
    const auto degradation = descriptor.absolute_degradation +
                             descriptor.relative_degradation * std::fabs(best);
    points.erase(std::remove_if(points.begin(), points.end(), [&](const auto& point) {
      const auto candidate = value(objective, point);
      return objective.sense == ObjectiveSense::Minimize
        ? candidate > best + degradation : candidate < best - degradation;
    }), points.end());
  }
  oracle.final_points = std::move(points);
  return oracle;
}

void verify(const LexicographicResult& result, const Oracle& oracle,
            const std::vector<LexicographicObjective>& objectives) {
  assert(result.completed_numerically());
  assert(result.termination == Termination::Optimal);
  assert(result.guarantee == Guarantee::Numerical);
  assert(result.has_solution());
  assert(result.completed_stages == objectives.size());
  assert(result.stages.size() == objectives.size());
  for (std::size_t i = 0; i < objectives.size(); ++i) {
    assert(result.stages[i].completed);
    assert(result.stages[i].index == i);
    assert(result.stages[i].result.termination == Termination::Optimal);
    assert(result.stages[i].result.objective == oracle.stage_optima[i]);
    assert(result.objective_values[i] == value(objectives[i].objective,
                                              result.final_solution.values));
  }
  assert(std::find(oracle.final_points.begin(), oracle.final_points.end(),
                   result.final_solution.values) != oracle.final_points.end());
  assert(result.final_solution.termination == Termination::Unknown);
  assert(!result.final_solution.best_bound && !result.final_solution.absolute_gap &&
         !result.final_solution.relative_gap && !result.final_solution.native_backend_gap);
}

void discrete_oracle_tests() {
  Model model;
  auto x = model.add_integer(0, 4, "x");
  auto y = model.add_integer(0, 4, "y");
  auto demand = model.add_row({{x, 1}, {y, 1}}, 2, infinity, "demand");
  model.maximize({{x, 3}, {y, 2}}, 7);
  const auto before = model.snapshot();
  std::vector<std::vector<double>> points;
  for (int a = 0; a <= 4; ++a)
    for (int b = 0; b <= 4; ++b)
      if (a + b >= 2) points.push_back({static_cast<double>(a), static_cast<double>(b)});
  std::vector<LexicographicObjective> objectives{
    objective(x, ObjectiveSense::Minimize), objective(y, ObjectiveSense::Minimize)};
  objectives[0].name = "highest priority";
  auto result = solve_lexicographic(model, objectives);
  verify(result, enumerate(points, objectives), objectives);
  assert(result.final_solution.value(x) == 0 && result.final_solution.value(y) == 2);
  assert(result.final_solution.objective == 11); // ORIGINAL scalar objective
  assert(result.stages[0].name == "highest priority");
  assert(result.stages[0].retention_bound == 0);
  assert(!result.stages[1].retention_bound);
  assert(result.model_id == before.model_id && result.revision == before.revision);
  assert(model.revision() == before.revision);
  assert(model.snapshot().rows.size() == before.rows.size());
  assert(model.row(demand).lower == 2 && model.row(demand).upper == infinity);
  assert(model.snapshot().objective.offset == 7);
  assert(model.snapshot().objective.sense == ObjectiveSense::Maximize);
  assert(model.snapshot().objective.terms[0].coefficient == 3);
  assert(model.snapshot().objective.terms[1].coefficient == 2);

  SolveOptions started;
  started.primal_start = {{x, 4}, {y, 4}}; // feasible initially, violates first x<=0 lock
  auto with_start = solve_lexicographic(model, objectives, started);
  verify(with_start, enumerate(points, objectives), objectives);
  assert(with_start.stages[0].result.start_submitted);
  assert(!with_start.stages[1].result.start_submitted);

  std::reverse(objectives.begin(), objectives.end()); // priority is input order
  result = solve_lexicographic(before, objectives);
  verify(result, enumerate(points, objectives), objectives);
  assert(result.final_solution.value(x) == 2 && result.final_solution.value(y) == 0);

  objectives = {objective(x, ObjectiveSense::Minimize), objective(y, ObjectiveSense::Minimize)};
  objectives[0].absolute_degradation = 1;
  result = solve_lexicographic(model, objectives);
  verify(result, enumerate(points, objectives), objectives);
  assert(result.final_solution.value(x) == 1 && result.final_solution.value(y) == 1);

  objectives[0] = objective(x, ObjectiveSense::Minimize, 2);
  objectives[0].relative_degradation = 0.5; // f*=2, allowance=1, hence x<=1
  result = solve_lexicographic(model, objectives);
  verify(result, enumerate(points, objectives), objectives);
  assert(result.stages[0].retention_bound == 3);
  assert(result.final_solution.value(x) == 1 && result.final_solution.value(y) == 1);

  model.set_bounds(demand, -infinity, 5);
  points.clear();
  for (int a = 0; a <= 4; ++a)
    for (int b = 0; b <= 4; ++b)
      if (a + b <= 5) points.push_back({static_cast<double>(a), static_cast<double>(b)});
  objectives = {objective(x, ObjectiveSense::Maximize, -10),
                objective(y, ObjectiveSense::Maximize, 3)};
  objectives[0].absolute_degradation = 1;
  result = solve_lexicographic(model, objectives);
  verify(result, enumerate(points, objectives), objectives);
  assert(result.stages[0].retention_bound == -7);
  assert(result.final_solution.value(x) == 3 && result.final_solution.value(y) == 2);

  objectives[1].objective.sense = ObjectiveSense::Minimize;
  result = solve_lexicographic(model, objectives);
  verify(result, enumerate(points, objectives), objectives);
}

void numerical_and_empty_tests() {
  Model offset_model;
  auto x = offset_model.add_integer(1, 4);
  auto result = solve_lexicographic(offset_model,
    {objective(x, ObjectiveSense::Minimize, 1e16), objective(x, ObjectiveSense::Maximize)});
  assert(result.completed_numerically());
  assert(result.final_solution.value(x) == 1); // do not lock x <= round(1e16+1)-1e16

  // A large linear subtotal and offset cancel to an exact small residual.
  // An extreme-scale backend bound can remain unresolved; the workflow must
  // still report the independently computed objective vector faithfully.
  Model cancellation;
  auto residual = cancellation.add_continuous(1, 1);
  auto fixed = cancellation.add_continuous(1e6, 1e6);
  cancellation.minimize({{residual, 1}, {fixed, 1e10}}, -1e16);
  LexicographicObjective cancellation_objective;
  cancellation_objective.objective = cancellation.snapshot().objective;
  result = solve_lexicographic(cancellation, {cancellation_objective});
  assert(result.has_solution() && result.final_solution.objective == 1);
  assert(result.objective_values == std::vector<double>{1});
  assert(result.termination == Termination::Optimal ||
         result.termination == Termination::NumericalFailure);
  if (result.completed_numerically()) {
    assert(result.stages[0].result.objective == 1);
    assert(result.stages[0].result.best_bound &&
           std::fabs(*result.stages[0].result.best_bound - 1) <= 1e-7);
  }

  Model continuous;
  auto a = continuous.add_continuous(0, 4);
  auto b = continuous.add_continuous(0, 4);
  continuous.add_row({{a, 1}, {b, 1}}, 2, infinity);
  result = solve_lexicographic(continuous,
    {objective(a, ObjectiveSense::Minimize), objective(b, ObjectiveSense::Maximize)});
  assert(result.completed_numerically());
  assert(result.final_solution.value(a) == 0 && result.final_solution.value(b) == 4);

  Model empty;
  LexicographicObjective constant;
  constant.objective.offset = 3;
  LexicographicObjective other;
  other.objective.offset = -2;
  result = solve_lexicographic(empty, {constant, other});
  assert(result.completed_numerically() && result.has_solution());
  assert(result.objective_values == std::vector<double>({3, -2}));
  assert(result.final_solution.values.empty());

  Model tombstones;
  auto removed = tombstones.add_binary();
  tombstones.remove(removed);
  result = solve_lexicographic(tombstones, {constant, other});
  assert(result.completed_numerically() && result.has_solution());
  assert(result.final_solution.active_variables == std::vector<bool>({false}));
}

void status_tests() {
  Model model;
  auto x = model.add_integer(0, 4);
  std::vector<LexicographicObjective> objectives{
    objective(x, ObjectiveSense::Minimize), objective(x, ObjectiveSense::Maximize)};
  SolveOptions options;
  options.time_limit_seconds = 0;
  auto result = solve_lexicographic(model, objectives, options);
  assert(result.termination == Termination::TimeLimit && !result.has_solution());
  assert(result.stages.empty() && result.completed_stages == 0);
  options = {};
  options.cancellation = std::make_shared<CancellationToken>();
  options.cancellation->cancel();
  result = solve_lexicographic(model, objectives, options);
  assert(result.termination == Termination::Cancelled && !result.completed_numerically());
  options = {};
  options.node_limit = 0;
  result = solve_lexicographic(model, objectives, options);
  assert(result.termination == Termination::Unsupported && result.stages.empty());
  result = solve_lexicographic(model, {objectives[0]}, options);
  assert(result.termination == Termination::NodeLimit && result.completed_stages == 0);
  options = {};
  options.guarantee = Guarantee::Exact;
  assert(solve_lexicographic(model, objectives, options).termination == Termination::Unsupported);
  options.guarantee = Guarantee::Certified;
  assert(solve_lexicographic(model, objectives, options).termination == Termination::Unsupported);

  options = {};
  options.relative_gap = -1;
  assert(solve_lexicographic(model, objectives, options).termination == Termination::InvalidModel);
  assert(solve_lexicographic(model, {}).termination == Termination::InvalidModel);
  auto invalid = objectives;
  invalid[1].absolute_degradation = -1;
  result = solve_lexicographic(model, invalid);
  assert(result.termination == Termination::InvalidModel && result.stages.empty());
  invalid = objectives;
  invalid[1].relative_degradation = infinity;
  assert(solve_lexicographic(model, invalid).termination == Termination::InvalidModel);
  invalid = objectives;
  invalid[0].objective.terms[0].variable.model_id += 100;
  assert(solve_lexicographic(model, invalid).termination == Termination::InvalidModel);

  options = {};
  options.relative_gap = 1.0;
  options.absolute_gap = 100.0; // workflow requests zero gaps for each phase
  if (capabilities().available)
    assert(solve_lexicographic(model, objectives, options).completed_numerically());

  if (!capabilities().available) {
    assert(solve_lexicographic(model, objectives).termination == Termination::Unsupported);
    return;
  }
  Model infeasible;
  auto impossible = infeasible.add_binary();
  infeasible.add_row({{impossible, 1}}, 2, infinity);
  result = solve_lexicographic(infeasible,
    {objective(impossible, ObjectiveSense::Minimize), objective(impossible, ObjectiveSense::Maximize)});
  assert(result.termination == Termination::Infeasible);
  assert(result.stages.size() == 1 && result.completed_stages == 0 && !result.has_solution());

  Model unbounded;
  auto bounded = unbounded.add_continuous(0, 1);
  auto free = unbounded.add_continuous(-infinity, infinity);
  result = solve_lexicographic(unbounded,
    {objective(bounded, ObjectiveSense::Minimize), objective(free, ObjectiveSense::Maximize)});
  assert(result.termination == Termination::Unbounded);
  assert(result.stages.size() == 2 && result.completed_stages == 1);
  assert(!result.completed_numerically());

  auto overflow = objectives;
  overflow[0].objective.offset = 2;
  overflow[0].relative_degradation = std::numeric_limits<double>::max();
  result = solve_lexicographic(model, overflow);
  assert(result.termination == Termination::Unsupported && result.stages.size() == 1);
  assert(!result.completed_numerically());
}
}

int main() {
  status_tests();
  if (capabilities().available) {
    discrete_oracle_tests();
    numerical_and_empty_tests();
  }
}

#endif
