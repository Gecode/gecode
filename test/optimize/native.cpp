#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native.hpp>
#include <gecode/optimize/constraints.hpp>

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <utility>

using namespace Gecode::Optimize;
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();

bool feasible(const ModelSnapshot& model, const std::vector<double>& values) {
  const auto row = [&](const std::vector<Term>& terms, double lower, double upper) {
    long long activity = 0;
    for (const auto& term : terms)
      activity += static_cast<long long>(term.coefficient) * static_cast<long long>(values[term.variable.id]);
    return activity >= lower && activity <= upper;
  };
  for (const auto& constraint : model.rows)
    if (constraint.active && !constraint.indicator_origin &&
        !row(constraint.terms, constraint.lower, constraint.upper)) return false;
  for (const auto& indicator : model.indicators) {
    if (!indicator.active) continue;
    const bool enabled = values[indicator.activator.id] == (indicator.active_value ? 1 : 0);
    if (enabled && !row(indicator.terms, indicator.lower, indicator.upper)) return false;
    if (indicator.inactive_gate && values[indicator.inactive_gate->id] != (enabled ? 0 : 1)) return false;
  }
  return true;
}

std::optional<long long> oracle(const ModelSnapshot& model) {
  std::optional<long long> optimum;
  std::vector<double> values(model.variables.size());
  std::function<void(std::size_t)> visit = [&](std::size_t slot) {
    if (slot == model.variables.size()) {
      if (!feasible(model, values)) return;
      long long objective = static_cast<long long>(model.objective.offset);
      for (const auto& term : model.objective.terms)
        objective += static_cast<long long>(term.coefficient) * static_cast<long long>(values[term.variable.id]);
      if (!optimum || (model.objective.sense == ObjectiveSense::Minimize ? objective < *optimum : objective > *optimum))
        optimum = objective;
      return;
    }
    const auto& variable = model.variables[slot];
    if (!variable.active) { visit(slot + 1); return; }
    if (variable.type == VariableType::SemiInteger) { values[slot] = 0; visit(slot + 1); }
    for (int value = static_cast<int>(variable.lower); value <= variable.upper; ++value) {
      values[slot] = value; visit(slot + 1);
    }
  };
  visit(0); return optimum;
}

void matches_oracle(const Model& model) {
  const auto snapshot = model.snapshot();
  const auto expected = oracle(snapshot);
  for (auto guarantee : {Guarantee::Numerical, Guarantee::Exact}) {
    SolveOptions options; options.backend = Backend::Native; options.guarantee = guarantee;
    auto result = solve_native(snapshot, options);
    if (result.termination != (expected ? Termination::Optimal : Termination::Infeasible)) {
      std::cerr << "Native oracle mismatch: " << to_string(result.termination) << " " << result.message << '\n';
      assert(false);
    }
    assert(result.model_id == model.id() && result.revision == model.revision());
    assert(result.guarantee == guarantee && result.elapsed_seconds >= 0);
    assert(result.has_solution() == expected.has_value());
    if (expected) {
      assert(*result.objective == static_cast<double>(*expected));
      assert(result.best_bound == result.objective && result.absolute_gap == 0 && result.relative_gap == 0);
      assert(feasible(snapshot, result.values));
      for (const auto& variable : snapshot.variables) {
        if (!variable.active) continue;
        const auto value = result.value(variable.variable);
        assert(value == std::trunc(value));
        assert((variable.type == VariableType::SemiInteger && value == 0) ||
          (value >= variable.lower && value <= variable.upper));
      }
    } else {
      assert(!result.objective && !result.best_bound && !result.solution_validated);
    }
  }
}

void exhaustive_models() {
  for (int scenario = 0; scenario < 36; ++scenario) {
    Model model;
    const auto x = model.add_integer(-3, 3);
    const auto y = model.add_integer(-2, 2);
    const auto b = model.add_binary();
    model.add_row({{x, scenario % 5 - 2.0}, {y, scenario % 3 - 1.0}, {b, 2}}, -2, 4);
    model.add_row({{x, -1}, {y, 1}}, -inf, scenario % 4 - 1.0);
    const std::vector<Term> objective{{x, scenario % 7 - 3.0}, {y, -2}, {b, 3}};
    model.set_objective(objective, scenario % 2 ? ObjectiveSense::Maximize : ObjectiveSense::Minimize,
                         scenario - 20.0);
    matches_oracle(model);
  }
  Model empty; empty.minimize({}, -9); matches_oracle(empty);
  empty.add_row({}, 1, inf); matches_oracle(empty);
  Model no_cost; no_cost.add_integer(-2, 2); matches_oracle(no_cost);
}

void semi_indicators_and_aliases() {
  for (bool activation : {false, true})
    for (bool maximize : {false, true}) {
      Model model;
      auto b = model.add_binary();
      auto x = model.add_integer(-3, 3);
      auto semi = model.add_variable(VariableType::SemiInteger, 2, 4);
      const auto indicator = add_indicator(model, b, activation,
                                            {{x, 2}, {semi, -1}, {b, 1}}, -2, 1);
      assert(indicator.inactive_gate);
      model.add_row({{*indicator.inactive_gate, 1}, {semi, 1}}, -inf, 4);
      model.set_objective({{x, -2}, {semi, 3}, {*indicator.inactive_gate, 1}},
                            maximize ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, -7);
      matches_oracle(model);
      // Mutated public snapshots must not bypass indicator domain guards.
      auto changed = model.snapshot(); changed.variables[x.id].upper = 4;
      assert(solve_native(changed).termination == Termination::InvalidModel);
      changed = model.snapshot(); changed.indicators.clear();
      assert(solve_native(changed).termination == Termination::InvalidModel);
    }
  Model redundant;
  auto b = redundant.add_binary();
  auto x = redundant.add_integer(0, 2);
  auto condition = add_indicator(redundant, b, true, {{x, 1}}, 0, inf);
  assert(!condition.inactive_gate);
  add_indicator(redundant, b, false, {}, 1, inf);
  add_indicator(redundant, b, true, {{x, 1}}, -inf, inf);
  matches_oracle(redundant);

  // Large derived M is nonintegral after outward rounding. Native reification
  // solves original integral rows, so these generated M coefficients are not
  // incorrectly interpreted as an unsupported original fractional model.
  Model big;
  b = big.add_binary(); x = big.add_integer(-1000000, 1000000);
  auto large = add_indicator(big, b, true, {{x, 1}}, 1000000, inf);
  big.set_bounds(b, 1, 1); big.minimize({{x, 1}});
  SolveOptions exact; exact.guarantee = Guarantee::Exact;
  const auto solution = solve_native(big, exact);
  assert(solution.termination == Termination::Optimal && solution.value(x) == 1000000);
  assert(solution.value(*large.inactive_gate) == 0 && solution.guarantee == Guarantee::Exact);
}

void identities_and_history() {
  Model model;
  auto removed = model.add_integer(-1, 1); model.remove(removed);
  auto x = model.add_integer(-2, 2);
  auto row = model.add_row({{x, 1}}, -1, inf); model.remove(row);
  model.maximize({{x, -3}}, 4);
  const auto old = solve_native(model);
  assert(old.termination == Termination::Optimal && old.value(x) == -2 && *old.objective == 10);
  assert(!old.active_variables[removed.id] && std::isnan(old.values[removed.id]));
  model.set_bounds(x, 1, 2);
  assert(solve_native(model).value(x) == 1 && old.value(x) == -2);
  Model other; auto foreign = other.add_binary();
  bool rejected = false; try { (void) old.value(foreign); } catch (const ModelError&) { rejected = true; }
  assert(rejected);
  rejected = false; try { (void) old.value(removed); } catch (const ModelError&) { rejected = true; }
  assert(rejected);
  auto bad = model.snapshot(); bad.rows[0].constraint.model_id = other.id();
  assert(solve_native(bad).termination == Termination::InvalidModel);
  Model moved(std::move(model));
  assert(solve_native(model).termination == Termination::InvalidModel);
  assert(solve_native(moved).termination == Termination::Optimal);
}

void options_and_limits() {
  Model model; auto x = model.add_integer(-3, 3); model.minimize({{x, 1}});
  for (auto reason : {Termination::TimeLimit, Termination::NodeLimit, Termination::Cancelled}) {
    SolveOptions options;
    if (reason == Termination::TimeLimit) options.time_limit_seconds = 0;
    if (reason == Termination::NodeLimit) options.node_limit = 0;
    if (reason == Termination::Cancelled) {
      options.cancellation = std::make_shared<CancellationToken>(); options.cancellation->cancel();
    }
    const auto stopped = solve_native(model, options);
    assert(stopped.termination == reason && !stopped.has_solution() && !stopped.best_bound);
  }
  SolveOptions cancelled; cancelled.node_limit = 0; cancelled.time_limit_seconds = 0;
  cancelled.cancellation = std::make_shared<CancellationToken>(); cancelled.cancellation->cancel();
  assert(solve_native(model, cancelled).termination == Termination::Cancelled);
  for (std::uint64_t limit : {1, 2, 3, 4, 14, 20}) {
    Model hard; std::vector<Term> objective;
    for (int i = 0; i < 12; ++i) objective.push_back({hard.add_binary(), 1});
    hard.maximize(objective);
    SolveOptions limited; limited.node_limit = limit; limited.guarantee = Guarantee::Exact;
    const auto stopped = solve_native(hard, limited);
    assert(stopped.termination == Termination::NodeLimit && !stopped.best_bound);
    if (limit >= 14) assert(stopped.has_solution());
    if (stopped.has_solution()) assert(feasible(hard.snapshot(), stopped.values));
  }
  SolveOptions options; options.guarantee = Guarantee::Certified;
  assert(solve_native(model, options).termination == Termination::Unsupported);
  options = {}; options.threads = 2;
  assert(solve_native(model, options).termination == Termination::Unsupported);
  options = {}; options.random_seed = 1;
  assert(solve_native(model, options).termination == Termination::Unsupported);
  options = {}; options.backend = Backend::Highs;
  assert(solve_native(model, options).termination == Termination::Unsupported);
  options = {}; options.primal_start = {{x, 1}};
  const auto seeded = solve_native(model, options);
  assert(seeded.termination == Termination::Optimal && seeded.start_submitted && seeded.objective == -3);
  options = {}; options.feasibility_tolerance = -1;
  assert(solve_native(model, options).termination == Termination::InvalidModel);
}

void unsupported_models() {
  const auto unsupported = [](Model& model) {
    const auto result = solve_native(model);
    assert(result.termination == Termination::Unsupported && !result.has_solution() && !result.best_bound);
  };
  Model continuous; continuous.add_continuous(0, 0); unsupported(continuous);
  Model unbounded; unbounded.add_integer(); unsupported(unbounded);
  Model fractional_domain; fractional_domain.add_integer(0.5, 3); unsupported(fractional_domain);
  Model fractional_row; auto x = fractional_row.add_integer(0, 2);
  fractional_row.add_row({{x, 0.5}}, 0, 1); unsupported(fractional_row);
  Model fractional_side; x = fractional_side.add_integer(0, 2);
  fractional_side.add_row({{x, 1}}, 0.5, 1); unsupported(fractional_side);
  Model fractional_objective; x = fractional_objective.add_integer(0, 2);
  fractional_objective.minimize({{x, 0.5}}); unsupported(fractional_objective);
  Model offset; offset.minimize({}, 0.5); unsupported(offset);
  Model too_big; x = too_big.add_integer(-2000000000, 2000000000);
  too_big.add_row({{x, 2}}, 0, 10); unsupported(too_big);
  Model objective_overflow; x = objective_overflow.add_integer(0, 2000000000);
  objective_overflow.minimize({{x, 1}}); unsupported(objective_overflow);
  Model double_limit; x = double_limit.add_integer(0, 2);
  double_limit.minimize({{x, 1}}, 9007199254740992.0); unsupported(double_limit);
  Model exact_offset; exact_offset.minimize({}, -9007199254740992.0);
  SolveOptions exact; exact.guarantee = Guarantee::Exact;
  const auto result = solve_native(exact_offset, exact);
  assert(result.termination == Termination::Optimal && *result.objective == -9007199254740992.0);
}
}
#endif

int main() {
  const auto capabilities = native_capabilities();
  assert(Gecode::Optimize::capabilities(Backend::Native).available == capabilities.available);
  Model routed;
  auto route_x = routed.add_integer(-2, 5);
  routed.maximize({{route_x,-3}}, 7);
  SolveOptions route_options;
  route_options.backend = Backend::Native;
  route_options.guarantee = Guarantee::Exact;
  auto route_result = solve(routed, route_options);
  if (capabilities.available) {
    assert(route_result.termination == Termination::Optimal && route_result.has_solution());
    assert(route_result.guarantee == Guarantee::Exact && *route_result.objective == 13);
  } else assert(route_result.termination == Termination::Unsupported);
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  assert(capabilities.available && capabilities.exact_solving && !capabilities.linear_programming);
  exhaustive_models(); semi_indicators_and_aliases(); identities_and_history();
  options_and_limits(); unsupported_models();
  std::cout << "PASS native bridge and exhaustive original integer oracle\n";
#else
  assert(!capabilities.available && !capabilities.exact_solving);
  Model model; model.add_integer(0, 1);
  assert(solve_native(model).termination == Termination::Unsupported);
  auto malformed = model.snapshot(); malformed.model_id = 0;
  assert(solve_native(malformed).termination == Termination::InvalidModel);
  Model moved(std::move(model));
  assert(solve_native(model).termination == Termination::InvalidModel);
  std::cout << "PASS disabled native bridge contract\n";
#endif
}
