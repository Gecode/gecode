#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/validate.hpp>

#include <cassert>
#include <cmath>
#include <limits>
#include <utility>

using namespace Gecode::Optimize;
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();

template<class Action> void rejects(Action action) {
  bool rejected = false;
  try { action(); } catch (const ModelError&) { rejected = true; }
  assert(rejected);
}

bool linear_rows(const ModelSnapshot& model, const std::vector<double>& values,
                 long double tolerance = 0.0L) {
  for (const auto& row : model.rows) {
    if (!row.active) continue;
    long double value = 0;
    for (const auto& term : row.terms)
      value += static_cast<long double>(term.coefficient) * values[term.variable.id];
    if (value < static_cast<long double>(row.lower) - tolerance ||
        value > static_cast<long double>(row.upper) + tolerance) return false;
  }
  return true;
}

void indicator_equivalence() {
  for (bool active : {false, true})
    for (int kind = 0; kind < 3; ++kind) {
      Model model;
      auto b = model.add_binary("b");
      auto x = model.add_integer(-3, 3, "x");
      auto y = model.add_integer(-2, 2, "y");
      const double lower = kind == 1 ? -inf : -2.0;
      const double upper = kind == 0 ? inf : 4.0;
      const auto before = model.revision();
      auto formulation = add_indicator(model, b, active,
                                        {{x, 2}, {y, -3}, {b, 1}}, lower, upper);
      assert(model.revision() == before + 1);
      assert(formulation.inactive_gate);
      const auto snapshot = model.snapshot();
      validate_structure(snapshot);
      assert(formulation.rows.size() == (kind == 2 ? 3U : 2U));
      for (int bv = 0; bv <= 1; ++bv)
        for (int xv = -3; xv <= 3; ++xv)
          for (int yv = -2; yv <= 2; ++yv)
            for (int gv = 0; gv <= 1; ++gv) {
              std::vector<double> values(snapshot.variables.size());
              values[b.id] = bv; values[x.id] = xv; values[y.id] = yv;
              values[formulation.inactive_gate->id] = gv;
              const double original = 2 * xv - 3 * yv + bv;
              const bool enabled = bv == static_cast<int>(active);
              const bool expected = (gv == static_cast<int>(!enabled)) &&
                (!enabled || (original >= lower && original <= upper));
              assert(linear_rows(snapshot, values) == expected);
              assert(validate(snapshot, values, 0.0, 0.0).valid == expected);
            }
    }
}

void semi_domains_and_redundancy() {
  for (auto type : {VariableType::SemiContinuous, VariableType::SemiInteger}) {
    Model model;
    auto b = model.add_binary();
    auto x = model.add_variable(type, 3, 6);
    auto formulation = add_indicator(model, b, true, {{x, 1}}, 2, inf);
    assert(formulation.lower_m && *formulation.lower_m >= 2);
    auto snapshot = model.snapshot();
    assert(snapshot.indicators[0].domains[0].lower == 0.0);
    for (int bv = 0; bv <= 1; ++bv)
      for (int xv : {0, 3, 4, 5, 6}) {
        std::vector<double> values(snapshot.variables.size());
        values[b.id] = bv; values[x.id] = xv;
        values[formulation.inactive_gate->id] = 1 - bv;
        assert(validate(snapshot, values, 0, 0).valid == (bv == 0 || xv >= 2));
      }
  }
  Model model;
  auto b = model.add_binary();
  auto x = model.add_continuous(0, inf);
  // An unbounded upper endpoint is irrelevant to this lower-side relaxation.
  auto redundant = add_indicator(model, b, true, {{x, 1}}, 0, inf);
  assert(redundant.lower_m == 0.0 && !redundant.inactive_gate);
  auto tautology = add_indicator(model, b, false, {{x, 1}}, -inf, inf);
  assert(tautology.rows.empty() && !tautology.inactive_gate);
  auto snapshot = model.snapshot();
  validate_structure(snapshot);
  assert(validate(snapshot, {1, 1e100}, 0, 0).valid);

  Model constants;
  auto c = constants.add_binary();
  auto impossible = add_indicator(constants, c, true, {}, 1, inf);
  auto empty = constants.snapshot();
  assert(validate(empty, {0, 1}, 0, 0).valid);
  assert(!validate(empty, {1, 0}, 0, 0).valid);
  assert(impossible.inactive_gate);
}

void preserve_active_arithmetic_and_check_original_logic() {
  Model model;
  auto b = model.add_binary();
  auto x = model.add_continuous(-1e12, 1e12);
  const double tiny = 1e-9;
  auto formulation = add_indicator(model, b, true, {{b, tiny}, {x, 1}}, tiny, inf);
  const auto snapshot = model.snapshot();
  const auto& generated = model.row(formulation.rows.back());
  assert(generated.lower == tiny);
  assert(generated.terms[0].variable == b && generated.terms[0].coefficient == tiny);
  assert(*formulation.lower_m >= 1e12);
  assert(validate(snapshot, {1, 0, 0}, 0, 0).valid);
  assert(!validate(snapshot, {1, -1e-10, 0}, 0, 0).valid);

  // A near-zero gate can satisfy numerical M rows while violating the logic.
  // Original metadata must reject it for either activation value.
  for (bool active : {false, true}) {
    Model guarded;
    auto enabled = guarded.add_binary();
    auto amount = guarded.add_continuous(0, 1e12);
    auto condition = add_indicator(guarded, enabled, active, {{amount, 1}}, 1e12, inf);
    const auto original = guarded.snapshot();
    std::vector<double> values(original.variables.size());
    values[enabled.id] = active ? 1.0 - 5e-8 : 5e-8;
    values[amount.id] = 1e12 - 10000;
    values[condition.inactive_gate->id] = 5e-8;
    assert(linear_rows(original, values, 1e-7L));
    const auto checked = validate(original, values, 1e-7, 1e-6);
    assert(!checked.valid && checked.model_valid && checked.max_indicator_violation >= 10000);
  }
}

void mutation_guards_and_snapshot_validation() {
  Model model;
  auto b = model.add_binary();
  auto x = model.add_continuous(-5, 5);
  auto condition = add_indicator(model, b, true, {{x, 1}}, 2, 3);
  const auto original = model.snapshot();
  model.set_bounds(x, -3, 4);
  model.set_bounds(x, -5, 5);
  const auto revision = model.revision();
  rejects([&] { model.set_bounds(x, -6, 5); });
  rejects([&] { model.set_bounds(x, -5, 6); });
  rejects([&] { model.set_bounds(*condition.inactive_gate, 0, 0); });
  rejects([&] { model.set_bounds(condition.rows.back(), 0, 9); });
  rejects([&] { model.set_coefficient(condition.rows.front(), b, 2); });
  rejects([&] { model.remove(condition.rows.front()); });
  rejects([&] { model.remove(b); });
  assert(model.revision() == revision);
  auto bad = original;
  bad.variables[x.id].lower = -6;
  rejects([&] { validate_structure(bad); });
  bad = original;
  bad.rows[condition.rows.back().id].upper = 4;
  rejects([&] { validate_structure(bad); });
  bad = original;
  bad.indicators[0].lower_m = 0;
  rejects([&] { validate_structure(bad); });
  bad = original;
  bad.indicators[0].domains[0].lower = -100;
  rejects([&] { validate_structure(bad); });
  bad = original;
  bad.variables[b.id].type = VariableType::Integer;
  rejects([&] { validate_structure(bad); });
  bad = original;
  bad.indicators[0].active = false;
  rejects([&] { validate_structure(bad); });
  bad = original;
  bad.indicators.clear();
  rejects([&] { validate_structure(bad); });
  assert(validate(original, {1, 2, 0}, 0, 0).valid);
  remove_indicator(model, condition.indicator);
  assert(model.revision() == revision + 1);
  assert(!model.snapshot().indicators[0].active);
  model.set_bounds(x, -20, 20);
  model.remove(*condition.inactive_gate);
  model.remove(x);
  validate_structure(model.snapshot());
  rejects([&] { remove_indicator(model, condition.indicator); });
  Model moved(std::move(model));
  validate_structure(moved.snapshot());
  assert(moved.snapshot().indicators.size() == 1);
}

void errors_are_atomic() {
  Model model, other;
  auto b = model.add_binary();
  auto x = model.add_continuous(-inf, inf);
  auto integer = model.add_integer(0, 1);
  auto foreign = other.add_binary();
  const auto before = model.snapshot();
  rejects([&] { add_indicator(model, b, true, {{x, 1}}, 0, inf); });
  rejects([&] { add_indicator(model, b, true, {{x, -1}}, -inf, 0); });
  rejects([&] { add_indicator(model, integer, true, {}, 0, 1); });
  rejects([&] { add_indicator(model, b, true, {{foreign, 0}}, 0, 1); });
  rejects([&] { add_indicator(model, foreign, true, {}, 0, 1); });
  rejects([&] { add_indicator(model, b, true, {}, 2, 1); });
  rejects([&] { add_indicator(model, b, true, {}, std::nan(""), 1); });
  rejects([&] { add_boolean_and(model, b, {foreign}); });
  rejects([&] { add_boolean_or(model, integer, {}); });
  rejects([&] { remove_indicator(model, Indicator{other.id(), 0}); });
  assert(model.revision() == before.revision);
  auto after = model.snapshot();
  assert(after.variables.size() == before.variables.size() && after.rows.empty() && after.indicators.empty());
  model.set_bounds(x, -2, 2);
  const auto revision = model.revision();
  rejects([&] {
    add_indicator(model, b, true, {{x, std::numeric_limits<double>::max()}}, -1, 1);
  });
  assert(model.revision() == revision && model.snapshot().rows.empty());
}

void boolean_truth_tables() {
  for (bool conjunction : {false, true})
    for (int count = 0; count <= 4; ++count)
      for (bool alias_result : {false, true}) {
        Model model;
        auto result = model.add_binary("result");
        std::vector<Variable> inputs;
        for (int i = 0; i < count; ++i)
          inputs.push_back(alias_result && i == 0 ? result : model.add_binary());
        if (count > 1) inputs.push_back(inputs.back()); // Duplicate is idempotent.
        const auto revision = model.revision();
        const auto rows = conjunction ? add_boolean_and(model, result, inputs)
                                      : add_boolean_or(model, result, inputs);
        assert(!rows.empty() && model.revision() == revision + 1);
        const auto snapshot = model.snapshot();
        validate_structure(snapshot);
        for (unsigned mask = 0; mask < (1U << snapshot.variables.size()); ++mask) {
          std::vector<double> values(snapshot.variables.size());
          for (std::size_t i = 0; i < values.size(); ++i) values[i] = (mask >> i) & 1U;
          bool expected = conjunction;
          for (auto input : inputs)
            expected = conjunction ? expected && values[input.id] != 0.0
                                   : expected || values[input.id] != 0.0;
          const bool correct = values[result.id] == static_cast<int>(expected);
          assert(linear_rows(snapshot, values) == correct);
          assert(validate(snapshot, values, 0, 0).valid == correct);
        }
      }
}
}

int main() {
  indicator_equivalence();
  semi_domains_and_redundancy();
  preserve_active_arithmetic_and_check_original_logic();
  mutation_guards_and_snapshot_validation();
  errors_are_atomic();
  boolean_truth_tables();
}
