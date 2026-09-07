#include <gecode/optimize/relaxation.hpp>
#include <gecode/optimize/constraints.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <string>

using namespace Gecode::Optimize;
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();
bool near(double a, double b) { return std::fabs(a-b) < 1e-6; }
void repaired(const RelaxationResult& result, double violation) {
  assert(result.termination == Termination::Optimal);
  assert(result.has_repair() && result.minimum_violation_established);
  assert(result.guarantee == Guarantee::Numerical);
  assert(result.minimum_weighted_violation && near(*result.minimum_weighted_violation, violation));
  assert(result.weighted_violation && near(*result.weighted_violation, violation));
  assert(result.private_model && result.private_model->model_id != result.source_model_id);
  assert(result.workflow.final_solution.model_id == result.private_model->model_id);
  assert(validate(*result.private_model, result.workflow.final_solution.values).valid);
  double sum = 0;
  for (const auto& item : result.items) {
    assert(item.activity && item.violation && item.weighted_violation && item.slack_value);
    assert(*item.violation >= 0 && *item.slack_value >= -1e-7);
    assert(near(*item.weighted_violation, item.penalty * *item.violation));
    assert(item.source_row.has_value() != item.source_variable.has_value());
    sum += *item.weighted_violation;
  }
  assert(near(sum, violation));
}
}

#ifdef GECODE_RELAXATION_TEST_FAKE_SOLVER
namespace {
enum class Scenario { Good, IncompleteFirst, IncompleteSecond, CancelFirst, CancelSecond,
  BadPrimal, BadObjective, BadBound, OpenBound, MissingBound, BadSecondBound,
  FalseInfeasible, FirstInfeasible, BadOwner, BadMask, IncompleteBadOwner, IncompleteBadRevision, OffsetHonest, OffsetForged, ResidualHonest, ResidualForged };
Scenario scenario = Scenario::Good;
unsigned calls = 0;
double previous_remaining = inf;
}
namespace Gecode { namespace Optimize {
SolveResult solve(const ModelSnapshot& model, const SolveOptions& options) {
  assert(options.primal_start.empty());
  assert(options.relative_gap == 0 && options.absolute_gap == 0);
  assert(options.time_limit_seconds <= previous_remaining);
  previous_remaining = options.time_limit_seconds;
  const bool residual_case = scenario == Scenario::ResidualHonest || scenario == Scenario::ResidualForged;
  const bool offset_case = scenario == Scenario::OffsetHonest || scenario == Scenario::OffsetForged || residual_case;
  assert(model.variables.size() == (offset_case ? 3u : 2u));
  SolveResult result;
  result.model_id = model.model_id; result.revision = model.revision;
  result.backend = "deterministic repair oracle";
  result.values = offset_case ? std::vector<double>{1, 1e6, 1} : std::vector<double>{1, 1};
  if (scenario == Scenario::ResidualForged) result.values[2] = 0;
  result.active_variables.assign(model.variables.size(), true);
  result.objective = calls == 0 || offset_case ? 1.0 : 8.0;
  if ((scenario == Scenario::OffsetForged && calls == 1) || scenario == Scenario::ResidualForged) result.objective = 0;
  result.best_bound = result.objective;
  result.solution_validated = true;
  result.termination = Termination::Optimal;
  result.update_gaps(model.objective.sense);
  if (scenario == Scenario::BadOwner || scenario == Scenario::IncompleteBadOwner) ++result.model_id;
  if (scenario == Scenario::IncompleteBadRevision) ++result.revision;
  if (scenario == Scenario::BadMask) result.active_variables[0] = false;
  if (scenario == Scenario::BadPrimal) result.values = {2, 0};
  if (scenario == Scenario::BadObjective) {
    result.objective = 0; result.best_bound = 0;
  }
  if (scenario == Scenario::BadBound || (scenario == Scenario::BadSecondBound && calls == 1))
    result.best_bound = *result.objective + 1;
  if (scenario == Scenario::OpenBound) result.best_bound = 0;
  // Keep a forged zero absolute_gap to verify callers recompute bound evidence.
  if (scenario == Scenario::MissingBound) {
    result.best_bound.reset(); result.absolute_gap.reset(); result.relative_gap.reset();
  }
  if (scenario == Scenario::IncompleteBadOwner || scenario == Scenario::IncompleteBadRevision)
    result.termination = Termination::TimeLimit;
  if ((calls == 0 && scenario == Scenario::IncompleteFirst) ||
      (calls == 1 && scenario == Scenario::IncompleteSecond)) result.termination = Termination::TimeLimit;
  if ((calls == 0 && scenario == Scenario::CancelFirst) ||
      (calls == 1 && scenario == Scenario::CancelSecond)) options.cancellation->cancel();
  if (scenario == Scenario::FirstInfeasible || (calls == 1 && scenario == Scenario::FalseInfeasible)) {
    result.termination = Termination::Infeasible;
    result.solution_validated = false; result.values.clear(); result.objective.reset();
  }
  ++calls;
  return result;
}
}}
int main() {
  Model model;
  auto x = model.add_integer(0, 1);
  auto demand = model.add_row({{x, 1}}, 2, inf);
  model.minimize({{x, 1}}, 7);
  for (auto selected : {Scenario::Good, Scenario::IncompleteFirst, Scenario::IncompleteSecond,
       Scenario::CancelFirst, Scenario::CancelSecond, Scenario::BadPrimal,
       Scenario::BadObjective, Scenario::BadBound, Scenario::OpenBound, Scenario::MissingBound,
       Scenario::BadSecondBound, Scenario::FalseInfeasible, Scenario::FirstInfeasible,
       Scenario::BadOwner, Scenario::BadMask, Scenario::IncompleteBadOwner, Scenario::IncompleteBadRevision}) {
    scenario = selected; calls = 0; previous_remaining = inf;
    RelaxationOptions options;
    options.rows = {{demand, RelaxationSide::Lower, 1}};
    options.optimize_original_objective = true;
    options.solve.time_limit_seconds = 60;
    auto result = relax_feasibility(model, options);
    assert(calls >= 1 && calls <= 2);
    assert(model.row(demand).lower == 2);
    if (result.minimum_violation_established) {
      assert(result.violation_lock && result.private_model);
      assert(result.violation_lock->model_id == result.private_model->model_id);
      assert(result.violation_lock->id + 1 == result.private_model->rows.size());
      assert(result.private_model->rows.back().upper == 1);
      if (result.has_repair()) assert(validate(*result.private_model, result.workflow.final_solution.values).valid);
    }
    if (selected == Scenario::Good) {
      repaired(result, 1);
      assert(result.original_objective_optimized && result.violation_lock);
      assert(!result.original_validation.valid);
    } else {
      assert(!result.original_objective_optimized);
      if (selected == Scenario::IncompleteFirst) {
        assert(result.termination == Termination::TimeLimit && calls == 1);
        assert(result.has_repair() && !result.minimum_violation_established);
      } else if (selected == Scenario::IncompleteSecond) {
        assert(result.termination == Termination::TimeLimit && calls == 2);
        assert(result.has_repair() && result.minimum_violation_established && result.violation_lock);
      } else if (selected == Scenario::CancelFirst || selected == Scenario::CancelSecond) {
        assert(result.termination == Termination::Cancelled && !result.has_repair());
        assert(!result.minimum_violation_established);
      } else if (selected == Scenario::FirstInfeasible) {
        assert(result.termination == Termination::Infeasible && !result.has_repair());
        assert(!result.minimum_violation_established);
      } else {
        assert(result.termination == Termination::NumericalFailure);
        // A later contradictory oracle does not revoke earlier valid evidence.
        assert(!result.minimum_violation_established || selected == Scenario::FalseInfeasible ||
               selected == Scenario::BadSecondBound);
      }
    }
  }
  // Exact arithmetic total is 1, even when long double == double and the
  // objective offset cancels a 1e16 linear term. A forged zero bound is invalid.
  for (auto selected : {Scenario::OffsetHonest, Scenario::OffsetForged}) {
    scenario = selected; calls = 0; previous_remaining = inf;
    Model cancellation;
    auto a = cancellation.add_integer(1, 1);
    auto b = cancellation.add_integer(1e6, 1e6);
    auto constant = cancellation.add_row({}, 1, inf);
    cancellation.minimize({{a, 1}, {b, 1e10}}, -1e16);
    RelaxationOptions options;
    options.rows = {{constant, RelaxationSide::Lower, 1}};
    options.optimize_original_objective = true;
    auto result = relax_feasibility(cancellation, options);
    assert(calls == 2);
    if (selected == Scenario::OffsetHonest) {
      repaired(result, 1);
      assert(result.original_objective == 1 && result.original_objective_optimized);
    } else {
      assert(result.termination == Termination::NumericalFailure);
      assert(!result.original_objective_optimized);
      assert(result.minimum_violation_established); // the valid first stage remains established
    }
  }
  for (auto selected : {Scenario::ResidualHonest, Scenario::ResidualForged}) {
    scenario = selected; calls = 0; previous_remaining = inf;
    Model cancellation;
    auto a = cancellation.add_integer(1, 1);
    auto b = cancellation.add_integer(1e6, 1e6);
    auto row = cancellation.add_row({{a, 1}, {b, 1e10}}, -inf, 1e16);
    RelaxationOptions options; options.rows = {{row, RelaxationSide::Upper, 1}};
    auto result = relax_feasibility(cancellation, options);
    assert(calls == 1);
    if (selected == Scenario::ResidualHonest) {
      repaired(result, 1);
      assert(result.items[0].violation == 1);
      assert(!result.original_validation.valid && result.original_validation.max_row_violation == 1);
    } else {
      assert(result.termination == Termination::NumericalFailure);
      assert(!result.has_repair() && !result.minimum_violation_established);
    }
  }
  calls = 0;
  RelaxationOptions options; options.rows = {{demand, RelaxationSide::Lower, 1}};
  options.solve.time_limit_seconds = 0;
  auto result = relax_feasibility(model, options);
  assert(result.termination == Termination::TimeLimit && calls == 0 && !result.has_repair());
  options.solve.time_limit_seconds = inf;
  options.solve.node_limit = 0;
  result = relax_feasibility(model, options);
  assert(result.termination == Termination::NodeLimit && calls == 0);
  options.optimize_original_objective = true;
  options.solve.node_limit = 10;
  result = relax_feasibility(model, options);
  assert(result.termination == Termination::Unsupported && calls == 0);
}
#else
namespace {
void row_and_bound_oracles() {
  for (auto sense : {ObjectiveSense::Minimize, ObjectiveSense::Maximize}) {
    Model model;
    auto x = model.add_integer(0, 4, "x");
    auto y = model.add_integer(1, 2, "y");
    model.add_row({{x, 1}}, -2, inf); // hard finite range for independent enumeration
    model.add_row({{y, 1}}, -inf, 3);
    model.add_row({{x, 1}, {y, 1}}, -inf, 4);
    auto demand = model.add_row({{x, 1}, {y, 1}}, 5, inf, "demand");
    auto balance = model.add_row({{x, 1}, {y, -1}}, -inf, 0, "balance");
    model.set_objective({{x, -2}, {y, 3}}, sense, -17);
    const auto before = model.snapshot();
    RelaxationOptions options;
    options.rows = {{balance, RelaxationSide::Upper, 2}, {demand, RelaxationSide::Lower, 3}};
    options.bounds = {{y, RelaxationSide::Upper, 4}, {x, RelaxationSide::Lower, 1}};
    options.optimize_original_objective = true;
    double best_penalty = inf, best_objective = sense == ObjectiveSense::Minimize ? inf : -inf;
    std::vector<std::vector<double>> best_points;
    for (int a = -2; a <= 4; ++a) for (int b = 1; b <= 3; ++b) if (a+b <= 4) {
      const double penalty = 3*std::max(0, 5-a-b) + 2*std::max(0, a-b) +
                             std::max(0, -a) + 4*std::max(0, b-2);
      const double objective = -2*a + 3*b - 17;
      if (penalty < best_penalty) {
        best_penalty = penalty;
        best_objective = sense == ObjectiveSense::Minimize ? inf : -inf;
        best_points.clear();
      }
      if (penalty != best_penalty) continue;
      if ((sense == ObjectiveSense::Minimize && objective < best_objective) ||
          (sense == ObjectiveSense::Maximize && objective > best_objective)) {
        best_objective = objective; best_points.clear();
      }
      if (objective == best_objective) best_points.push_back({double(a), double(b)});
    }
    auto result = relax_feasibility(model, options);
    repaired(result, best_penalty);
    assert(result.original_objective_optimized && result.violation_lock);
    assert(near(*result.original_objective, best_objective));
    assert(std::find(best_points.begin(), best_points.end(), result.original_values) != best_points.end());
    assert(!result.original_validation.valid && result.original_validation.model_valid);
    assert(result.items[0].source_row->id == demand.id && result.items[1].source_row->id == balance.id);
    assert(result.items[2].source_variable == x && result.items[3].source_variable == y);
    assert(result.private_variables.size() == 2);
    assert(result.private_model->variables[0].type == VariableType::Integer);
    bool foreign = false;
    try { (void)result.workflow.final_solution.value(x); } catch (const ModelError&) { foreign = true; }
    assert(foreign);
    assert(model.id() == before.model_id && model.revision() == before.revision);
    assert(model.row(demand).lower == 5 && model.row(balance).upper == 0);
    assert(model.variable(x).lower == 0 && model.variable(y).upper == 2);
    assert(model.snapshot().rows.size() == before.rows.size());
    assert(model.snapshot().objective.offset == -17 && model.snapshot().objective.sense == sense);
  }
}
void penalty_priorities_and_ties() {
  for (auto sense : {ObjectiveSense::Minimize, ObjectiveSense::Maximize}) {
    Model model; auto x = model.add_integer(0, 4);
    auto lower = model.add_row({{x, 1}}, 3, inf);
    auto upper = model.add_row({{x, 1}}, -inf, 1);
    model.set_objective({{x, 2}}, sense, -11);
    RelaxationOptions options;
    options.rows = {{lower, RelaxationSide::Lower, 1}, {upper, RelaxationSide::Upper, 1}};
    options.optimize_original_objective = true;
    auto result = relax_feasibility(model, options); repaired(result, 2);
    assert(result.original_objective_optimized);
    assert(near(result.original_values[0], sense == ObjectiveSense::Minimize ? 1 : 3));
    assert(near(*result.original_objective, sense == ObjectiveSense::Minimize ? -9 : -5));
    // Positive weights change the optimum before the original objective matters.
    options.rows[0].penalty = 3;
    result = relax_feasibility(model, options); repaired(result, 2);
    assert(near(result.original_values[0], 3));
    options.rows[0].penalty = 1; options.rows[1].penalty = 3;
    result = relax_feasibility(model, options); repaired(result, 2);
    assert(near(result.original_values[0], 1));
  }
}
void continuous_and_constants() {
  Model model;
  auto x = model.add_continuous(0, 10);
  auto row = model.add_row({{x, 1}}, 3, 7);
  auto hard = model.add_row({{x, 1}}, 9, inf);
  RelaxationOptions options; options.rows = {{row, RelaxationSide::Upper, 2}};
  auto result = relax_feasibility(model, options);
  repaired(result, 4); assert(near(result.original_values[0], 9));
  assert(!result.original_objective_optimized && !result.violation_lock);
  model.remove(hard);
  result = relax_feasibility(model, options); repaired(result, 0);
  assert(result.original_validation.valid);

  Model constants;
  auto positive = constants.add_row({}, 2, inf, "constant lower");
  auto negative = constants.add_row({}, -inf, -3, "constant upper");
  constants.minimize({}, 13);
  options.rows = {{negative, RelaxationSide::Upper, 5}, {positive, RelaxationSide::Lower, 2}};
  options.optimize_original_objective = true;
  result = relax_feasibility(constants, options); repaired(result, 19);
  assert(result.original_values.empty() && result.original_objective_optimized);
  assert(result.original_objective == 13 && !result.original_validation.valid);
  assert(result.items[0].violation == 2 && result.items[1].violation == 3);

  Model hard_integer;
  auto integer = hard_integer.add_integer(0.2, 0.8);
  result = relax_feasibility(hard_integer);
  assert(result.termination == Termination::Infeasible && !result.has_repair());
  options = {}; options.bounds = {{integer, RelaxationSide::Lower, 2}};
  result = relax_feasibility(hard_integer, options); repaired(result, 0.4);
  assert(result.original_values[0] == 0 && result.private_model->variables[0].type == VariableType::Integer);

  Model free;
  auto z = free.add_continuous(-inf, inf);
  free.maximize({{z, 1}}, -4);
  options = {}; options.optimize_original_objective = true;
  result = relax_feasibility(free, options);
  assert(result.termination == Termination::Unbounded);
  assert(result.minimum_violation_established && !result.original_objective_optimized);
  assert(result.has_repair() && result.weighted_violation == 0);
}
void binary_semi_indicator() {
  for (bool fixed_one : {false, true}) {
    Model model;
    auto x = model.add_binary();
    model.set_bounds(x, fixed_one ? 1 : 0, fixed_one ? 1 : 0);
    model.add_row({{x, 1}}, fixed_one ? 0 : 1, fixed_one ? 0 : 1);
    RelaxationOptions options;
    options.bounds = {{x, fixed_one ? RelaxationSide::Lower : RelaxationSide::Upper, 3}};
    auto result = relax_feasibility(model, options); repaired(result, 3);
    assert(result.original_values[0] == (fixed_one ? 0 : 1));
    assert(result.private_model->variables[0].type == VariableType::Binary);
    assert(result.private_model->variables[0].lower == 0 && result.private_model->variables[0].upper == 1);
    assert(!result.original_validation.valid);
  }
  {
    Model model;
    auto x = model.add_binary(); model.set_bounds(x, 0.25, 0.75);
    RelaxationOptions options;
    options.bounds = {{x, RelaxationSide::Lower, 2}, {x, RelaxationSide::Upper, 3}};
    auto result = relax_feasibility(model, options); repaired(result, 0.5);
    assert(result.original_values[0] == 0);
    assert(result.private_model->variables[0].lower == 0 && result.private_model->variables[0].upper == 1);
  }
  for (auto type : {VariableType::SemiContinuous, VariableType::SemiInteger}) {
    Model model;
    auto x = model.add_variable(type, 2, 4);
    auto row = model.add_row({{x, 1}}, 1, 1);
    RelaxationOptions options;
    options.rows = {{row, RelaxationSide::Lower, 1}, {row, RelaxationSide::Upper, 3}};
    auto result = relax_feasibility(model, options); repaired(result, 1);
    assert(result.original_values[0] == 0 && !result.original_validation.valid);
    options.bounds = {{x, RelaxationSide::Upper, 1}};
    result = relax_feasibility(model, options);
    assert(result.termination == Termination::Unsupported && !result.has_repair());
  }
  Model model;
  auto active = model.add_binary(); auto x = model.add_integer(0, 4);
  model.set_bounds(active, 1, 1);
  auto indicator = add_indicator(model, active, true, {{x, 1}}, 3, inf);
  auto row = model.add_row({{x, 1}}, -inf, 1);
  model.maximize({{x, 1}}, 31);
  RelaxationOptions options; options.rows = {{row, RelaxationSide::Upper, 1}};
  options.optimize_original_objective = true;
  const auto before = model.snapshot();
  auto result = relax_feasibility(model, options); repaired(result, 2);
  assert(result.original_values[x.id] == 3 && result.original_objective_optimized);
  assert(result.private_model->indicators.size() == 1);
  assert(result.private_model->indicators[0].activator.model_id == result.private_model->model_id);
  assert(model.revision() == before.revision && model.snapshot().indicators[0].active);
  options.bounds = {{x, RelaxationSide::Upper, 1}};
  assert(relax_feasibility(model, options).termination == Termination::Unsupported);
  options.bounds.clear(); options.rows = {{indicator.rows[0], RelaxationSide::Lower, 1}};
  assert(relax_feasibility(model, options).termination == Termination::Unsupported);
}
void rejected_and_stopped() {
  Model model; auto x = model.add_continuous(0, 2);
  auto row = model.add_row({{x, 1}}, 3, inf);
  RelaxationOptions options; options.rows = {{row, RelaxationSide::Lower, 1}};
  for (double penalty : {0.0, -1.0, inf, std::numeric_limits<double>::quiet_NaN()}) {
    options.rows[0].penalty = penalty;
    assert(relax_feasibility(model, options).termination == Termination::InvalidModel);
  }
  options.rows[0].penalty = 1;
  options.rows.push_back(options.rows[0]);
  assert(relax_feasibility(model, options).termination == Termination::InvalidModel);
  options.rows.pop_back(); options.rows[0].side = RelaxationSide::Upper;
  assert(relax_feasibility(model, options).termination == Termination::InvalidModel);
  options.rows[0].side = static_cast<RelaxationSide>(42);
  assert(relax_feasibility(model, options).termination == Termination::InvalidModel);
  options.rows[0].side = RelaxationSide::Lower;
  options.rows[0].row.model_id += 100000;
  assert(relax_feasibility(model, options).termination == Termination::InvalidModel);
  options.rows[0].row = row;
  options.solve.primal_start = {{x, 0}};
  assert(relax_feasibility(model, options).termination == Termination::Unsupported);
  options.solve.primal_start.clear();
  for (auto guarantee : {Guarantee::Exact, Guarantee::Certified}) {
    options.solve.guarantee = guarantee;
    assert(relax_feasibility(model, options).termination == Termination::Unsupported);
  }
  options.solve.guarantee = Guarantee::Numerical;
  options.solve.time_limit_seconds = 0;
  assert(relax_feasibility(model, options).termination == Termination::TimeLimit);
  options.solve.time_limit_seconds = inf;
  options.solve.cancellation = std::make_shared<CancellationToken>(); options.solve.cancellation->cancel();
  assert(relax_feasibility(model, options).termination == Termination::Cancelled);
  options.solve.cancellation.reset(); options.solve.node_limit = 0;
  assert(relax_feasibility(model, options).termination == Termination::NodeLimit);
  options.solve.node_limit = 1; options.optimize_original_objective = true;
  assert(relax_feasibility(model, options).termination == Termination::Unsupported);
  options.solve.node_limit.reset(); options.optimize_original_objective = false;
  auto malformed = model.snapshot(); malformed.rows[0].terms[0].coefficient = inf;
  assert(relax_feasibility(malformed, options).termination == Termination::InvalidModel);
  auto with_global = model.snapshot();
  with_global.globals.push_back({{model.id(), 0}, AllDifferentData{}, "", true});
  assert(relax_feasibility(with_global, options).termination == Termination::Unsupported);
  with_global.globals[0].active = false;
  auto result = relax_feasibility(with_global, options); repaired(result, 1);
  assert(result.private_model->globals.empty());
  with_global.globals[0].global.model_id += 100000;
  assert(relax_feasibility(with_global, options).termination == Termination::InvalidModel);
  model.remove(row);
  assert(relax_feasibility(model, options).termination == Termination::InvalidModel);
  Model tombstones;
  auto deleted = tombstones.add_binary(); tombstones.remove(deleted);
  auto only = tombstones.add_integer(0, 1);
  auto missing = tombstones.add_row({{only, 1}}, 2, inf);
  options = {}; options.rows = {{missing, RelaxationSide::Lower, 1}};
  result = relax_feasibility(tombstones, options); repaired(result, 1);
  assert(result.original_values.size() == 2 && result.private_variables.size() == 2);
  assert(!result.private_model->variables[0].active);
  // Untrusted snapshots can use the allocator's next otherwise valid identity.
  Model future;
  auto snapshot = future.snapshot(); ++snapshot.model_id;
  result = relax_feasibility(snapshot); repaired(result, 0);
  assert(result.private_model->model_id != snapshot.model_id);
}
}
int main() {
  if (!capabilities(Backend::Highs).available) {
    Model model; auto x = model.add_integer(0, 1);
    auto row = model.add_row({{x, 1}}, 2, inf);
    const auto revision = model.revision();
    RelaxationOptions options; options.rows = {{row, RelaxationSide::Lower, 1}};
    auto result = relax_feasibility(model, options);
    assert(result.termination == Termination::Unsupported);
    assert(!result.has_repair() && !result.minimum_violation_established && !result.original_objective_optimized);
    assert(model.revision() == revision && model.row(row).lower == 2);
    options.solve.time_limit_seconds = 0;
    assert(relax_feasibility(model, options).termination == Termination::TimeLimit);
    options.solve.time_limit_seconds = inf;
    options.solve.cancellation = std::make_shared<CancellationToken>(); options.solve.cancellation->cancel();
    assert(relax_feasibility(model, options).termination == Termination::Cancelled);
    return 0;
  }
  row_and_bound_oracles();
  penalty_priorities_and_ties();
  continuous_and_constants();
  binary_semi_indicator();
  rejected_and_stopped();
}
#endif
