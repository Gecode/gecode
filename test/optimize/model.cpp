/* Standalone tests: compile with model.cpp; no native Gecode dependencies. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/model.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

using namespace Gecode::Optimize;

namespace {
constexpr double infinity = std::numeric_limits<double>::infinity();

template<class Action>
void rejects(Action action) {
  bool caught = false;
  try {
    action();
  } catch (const ModelError&) {
    caught = true;
  }
  assert(caught);
}

void defaults_and_ownership() {
  static_assert(!std::is_copy_constructible<Model>::value, "No implicit model copy");
  static_assert(!std::is_copy_assignable<Model>::value, "No implicit model assignment");
  static_assert(std::is_nothrow_move_constructible<Model>::value, "Move preserves handles");
  Model a, b;
  assert(a.id() != 0 && b.id() != 0 && a.id() != b.id());
  auto initial = a.snapshot();
  assert(initial.revision == 0 && initial.variables.empty() && initial.rows.empty());
  assert(initial.objective.terms.empty() && initial.objective.offset == 0.0);
  assert(initial.objective.sense == ObjectiveSense::Minimize);
  Variable x = a.add_binary("x");
  Variable y = b.add_binary("y");
  Constraint row = a.add_row({{x, 1.0}}, 0.0, 1.0, "row");
  Constraint foreign = b.add_row({{y, 1.0}}, 0.0, 1.0);
  const auto revision = a.revision();
  rejects([&] { a.add_row({{y, 1.0}}, 0.0, 1.0); });
  rejects([&] { a.minimize({{y, 0.0}}); });
  rejects([&] { a.set_bounds(y, 0.0, 1.0); });
  rejects([&] { a.set_bounds(foreign, 0.0, 1.0); });
  rejects([&] { a.set_coefficient(row, y, 1.0); });
  rejects([&] { a.set_coefficient(foreign, x, 1.0); });
  rejects([&] { a.remove(y); });
  rejects([&] { a.remove(foreign); });
  rejects([&] { a.variable(Variable{}); });
  rejects([&] { a.row(Constraint{a.id(), 1000}); });
  assert(a.revision() == revision);
}

void sparse_terms_and_objective() {
  Model model;
  Variable x = model.add_integer(-2, 2, "x");
  Variable y = model.add_continuous(-infinity, infinity, "y");
  const std::vector<Term> terms{{y, 3}, {x, 4}, {y, -1}, {x, -2},
                               {x, 0}, {y, -2}, {x, -1}};
  Constraint row = model.add_row(terms, -3, 4, "range");
  const auto normalized = model.row(row).terms;
  assert(normalized.size() == 1 && normalized[0].variable == x);
  assert(normalized[0].coefficient == 1.0);
  // Check preservation on a small independent domain, including cancellations.
  for (int xv = -2; xv <= 2; ++xv)
    for (int yv = -2; yv <= 2; ++yv) {
      double original = 0, canonical = 0;
      for (const auto& term : terms)
        original += term.coefficient * (term.variable == x ? xv : yv);
      for (const auto& term : normalized)
        canonical += term.coefficient * (term.variable == x ? xv : yv);
      assert(original == canonical);
    }
  model.maximize({{y, 2}, {x, 3}, {x, -1}}, -7.5);
  auto snapshot = model.snapshot();
  assert(snapshot.objective.sense == ObjectiveSense::Maximize);
  assert(snapshot.objective.offset == -7.5);
  assert(snapshot.objective.terms.size() == 2);
  assert(snapshot.objective.terms[0].variable == x);
  assert(snapshot.objective.terms[0].coefficient == 2);
  assert(snapshot.objective.terms[1].variable == y);
  const Revision before = model.revision();
  model.minimize({}, 3.0);
  assert(model.revision() == before + 1);
  assert(model.snapshot().objective.terms.empty());
  assert(model.snapshot().objective.sense == ObjectiveSense::Minimize);
  assert(snapshot.objective.terms.size() == 2); // The old snapshot owns its data.

  // Cancellation should not depend on the caller's duplicate insertion order.
  std::vector<double> values{-1e16, 1.0, 1e16};
  do {
    model.minimize({{x, values[0]}, {x, values[1]}, {x, values[2]}});
    const auto objective = model.snapshot().objective;
    assert(objective.terms.size() == 1 && objective.terms[0].coefficient == 1.0);
  } while (std::next_permutation(values.begin(), values.end()));
}

void edits_and_tombstones() {
  Model model;
  Variable x = model.add_binary("x"), y = model.add_integer(-4, 7, "y");
  Constraint first = model.add_row({{x, 2}}, 0, 2, "first");
  Constraint second = model.add_row({{y, 1}}, -4, 7, "second");
  model.minimize({{y, 3}}, 1);
  auto old = model.snapshot();
  auto revision = model.revision();
  model.set_coefficient(first, y, 4);
  assert(model.revision() == ++revision);
  assert(model.row(first).terms.size() == 2);
  model.set_coefficient(first, x, -3);
  assert(model.revision() == ++revision);
  assert(model.row(first).terms[0].coefficient == -3);
  model.set_bounds(x, 1, 1);
  assert(model.revision() == ++revision && model.variable(x).lower == 1);
  model.set_bounds(first, -2, 3);
  assert(model.revision() == ++revision && model.row(first).upper == 3);
  model.set_name(x, "renamed");
  assert(model.revision() == ++revision && model.variable(x).name == "renamed");
  model.set_name(first, "renamed row");
  assert(model.revision() == ++revision && model.row(first).name == "renamed row");
  model.set_objective_coefficient(x, 5);
  assert(model.revision() == ++revision);
  model.set_objective_coefficient(y, 0);
  assert(model.revision() == ++revision);
  model.set_objective_offset(-2);
  assert(model.revision() == ++revision && model.snapshot().objective.offset == -2);
  rejects([&] { model.remove(x); }); // Objective still refers to x.
  rejects([&] { model.remove(y); }); // Active rows still refer to y.
  assert(model.revision() == revision);
  model.set_objective_coefficient(x, 0);
  model.set_coefficient(first, x, 0);
  model.remove(x);
  assert(!model.snapshot().variables[0].active);
  rejects([&] { model.variable(x); });
  rejects([&] { model.set_bounds(x, 0, 1); });
  rejects([&] { model.remove(x); });
  rejects([&] { model.add_row({{x, 0}}, 0, 1); });
  Variable z = model.add_binary("new");
  assert(z.id == 2 && z != x);
  model.remove(first);
  model.remove(second);
  model.remove(y);
  rejects([&] { model.row(first); });
  rejects([&] { model.remove(first); });
  Constraint third = model.add_row({{z, 1}}, 0, 1);
  assert(third.id == 2);
  const auto current = model.snapshot();
  assert(current.rows.size() == 3 && !current.rows[0].active && !current.rows[1].active);
  assert(current.rows[0].constraint.model_id == model.id());
  assert(current.rows[0].constraint.id == 0 && current.rows[0].terms.empty());
  assert(old.variables[0].active && old.variables[0].lower == 0);
  assert(old.variables[0].name == "x" && old.rows[0].active);
  assert(old.rows[0].terms.size() == 1 && old.objective.offset == 1);
}

void malformed_data_and_rollback() {
  Model model;
  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double huge = std::numeric_limits<double>::max();
  Variable x = model.add_continuous();
  Constraint row = model.add_row({{x, 2}}, -infinity, 4, "valid");
  model.maximize({{x, 3}}, 7);
  const auto revision = model.revision();
  rejects([&] { model.add_continuous(nan, 1); });
  rejects([&] { model.add_integer(2, 1); });
  rejects([&] { model.add_continuous(infinity, infinity); });
  rejects([&] { model.add_continuous(-infinity, -infinity); });
  rejects([&] { model.add_variable(static_cast<VariableType>(99), 0, 1); });
  rejects([&] { model.add_variable(VariableType::Binary, -1, 1); });
  rejects([&] { model.add_variable(VariableType::Binary, 0, 2); });
  rejects([&] { model.add_variable(VariableType::SemiContinuous, 0, 2); });
  rejects([&] { model.add_variable(VariableType::SemiInteger, -1, 2); });
  rejects([&] { model.add_row({{x, nan}}, 0, 1); });
  rejects([&] { model.add_row({{x, infinity}}, 0, 1); });
  rejects([&] { model.add_row({{x, -infinity}}, 0, 1); });
  rejects([&] { model.add_row({{x, huge}, {x, huge}}, 0, 1); });
  rejects([&] { model.add_row({}, nan, 1); });
  rejects([&] { model.set_bounds(row, 0, nan); });
  rejects([&] { model.set_bounds(x, 1, 0); });
  rejects([&] { model.set_coefficient(row, x, infinity); });
  rejects([&] { model.minimize({{x, huge}, {x, huge}}); });
  rejects([&] { model.minimize({}, nan); });
  rejects([&] { model.set_objective({}, static_cast<ObjectiveSense>(99)); });
  rejects([&] { model.set_objective_coefficient(x, nan); });
  rejects([&] { model.set_objective_offset(infinity); });
  const auto after = model.snapshot();
  assert(after.revision == revision && after.variables.size() == 1 && after.rows.size() == 1);
  assert(after.rows[0].terms.size() == 1 && after.rows[0].terms[0].coefficient == 2);
  assert(after.rows[0].upper == 4 && after.rows[0].lower == -infinity);
  assert(after.objective.sense == ObjectiveSense::Maximize && after.objective.offset == 7);
  assert(after.objective.terms.size() == 1 && after.objective.terms[0].coefficient == 3);
  assert(model.add_continuous().id == 1); // Failed insertion did not consume a slot.
}

void domain_and_empty_models() {
  Model model;
  auto unbounded = model.add_integer(-infinity, infinity);
  assert(model.variable(unbounded).lower == -infinity);
  // No integer lies here: it is a valid but infeasible model, not malformed data.
  model.add_integer(0.2, 0.8);
  auto semi = model.add_variable(VariableType::SemiContinuous, 2, infinity);
  auto semi_integer = model.add_variable(VariableType::SemiInteger, 2.2, 5.8);
  assert(model.variable(semi).lower == 2 && model.variable(semi_integer).upper == 5.8);
  rejects([&] { model.set_bounds(semi, 0, 5); });
  Model empty;
  empty.minimize({}, 42);
  empty.add_row({}, 1, infinity); // Structurally valid contradiction 0 >= 1.
  assert(empty.snapshot().variables.empty() && empty.snapshot().rows.size() == 1);
  assert(empty.snapshot().objective.offset == 42);
}

void move_identity() {
  Model original;
  Variable x = original.add_binary("x");
  Constraint row = original.add_row({{x, 1}}, 0, 1);
  const auto id = original.id();
  const auto revision = original.revision();
  Model moved(std::move(original));
  assert(original.id() == 0 && original.revision() == 0);
  rejects([&] { original.snapshot(); });
  rejects([&] { original.add_binary(); });
  assert(moved.id() == id && moved.revision() == revision);
  assert(moved.variable(x).name == "x" && moved.row(row).terms.size() == 1);
  Model assigned;
  Variable obsolete = assigned.add_binary();
  const auto historical = assigned.snapshot();
  assigned = std::move(moved);
  assert(assigned.id() == id && moved.id() == 0);
  assert(assigned.variable(x).name == "x");
  rejects([&] { assigned.variable(obsolete); });
  assert(historical.variables[0].variable == obsolete);
  Model& same = assigned;
  assigned = std::move(same);
  assert(assigned.id() == id && assigned.variable(x).active);
  original = Model{};
  assert(original.id() != 0 && original.id() != id);
  assert(original.add_binary().id == 0);
}
} // namespace

int main() {
  defaults_and_ownership();
  sparse_terms_and_objective();
  edits_and_tombstones();
  malformed_data_and_rollback();
  domain_and_empty_models();
  move_identity();
}
