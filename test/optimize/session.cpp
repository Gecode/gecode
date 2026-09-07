#include <gecode/optimize/session.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/validate.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>

namespace O = Gecode::Optimize;
constexpr double inf = std::numeric_limits<double>::infinity();
static void close(double a, double b) { assert(std::abs(a-b) < 1e-6); }
static O::SolveResult compare(O::SolveSession& session, const O::ModelSnapshot& model,
                             const O::SolveOptions& options = {}) {
  const auto warm = session.solve(model, options);
  const auto cold = O::solve(model, options);
  assert(warm.termination == cold.termination);
  assert(warm.has_solution() == cold.has_solution());
  if (warm.has_solution()) {
    assert(O::validate(model, warm.values).valid);
    close(*warm.objective, *cold.objective);
  }
  return warm;
}

int main() {
  O::Model model;
  const auto x = model.add_continuous(0, 100);
  const auto y = model.add_continuous(0, 100);
  const auto demand = model.add_row({{x,1},{y,1}}, 4, inf);
  model.minimize({{x,2},{y,3}}, -3);
  O::SolveSession session;
  O::SolveOptions options;
  options.time_limit_seconds = 0;
  assert(session.solve(model, options).termination == O::Termination::TimeLimit);
  assert(session.statistics().model_loads == 0);
  options = {};
  if (!O::capabilities().available) {
    assert(session.solve(model).termination == O::Termination::Unsupported);
    assert(session.statistics().model_loads == 0);
    std::cout << "session backend-unavailable contract passed\n";
    return 0;
  }
  auto first = compare(session, model.snapshot());
  close(*first.objective, 5);
  assert(session.statistics().model_loads == 1);
  model.set_bounds(demand, 8, inf);
  close(*compare(session, model.snapshot()).objective, 13);
  assert(session.statistics().incremental_updates == 1);
  assert(session.statistics().basis_warm_starts == 1);
  model.set_bounds(x, 0, 3);
  close(*compare(session, model.snapshot()).objective, 18);
  model.maximize({{x,-4},{y,-2}}, 10);
  close(*compare(session, model.snapshot()).objective, -6);
  assert(session.statistics().model_loads == 1);
  close(first.value(x), 4); // Historical result survives every mutation.

  // Public snapshots can change without a revision increment: content decides.
  auto snapshot = model.snapshot();
  snapshot.rows[demand.id].lower = 9;
  close(*compare(session, snapshot).objective, -8);
  snapshot.rows[demand.id].terms[0].coefficient = 2;
  compare(session, snapshot);
  assert(session.statistics().model_loads == 2);
  snapshot.variables[x.id].type = O::VariableType::Integer;
  compare(session, snapshot);
  assert(session.statistics().model_loads == 3);

  O::Model typed;
  const auto bit = typed.add_integer(0, 1);
  typed.minimize({{bit,1}});
  compare(session, typed.snapshot());
  auto typed_snapshot = typed.snapshot();
  typed_snapshot.variables[bit.id].type = O::VariableType::Binary;
  const auto type_loads = session.statistics().model_loads;
  compare(session, typed_snapshot);
  assert(session.statistics().model_loads == type_loads+1);

  // Row replacement with the same shape cannot reuse stale row identity.
  compare(session, model.snapshot());
  auto loads = session.statistics().model_loads;
  model.remove(demand);
  model.add_row({{x,1},{y,1}}, 8, inf);
  compare(session, model.snapshot());
  assert(session.statistics().model_loads == loads+1);
  model.set_name(x, "renamed");
  auto unchanged = session.statistics().unchanged_models;
  compare(session, model.snapshot());
  assert(session.statistics().unchanged_models == unchanged+1);
  auto bad = model.snapshot();
  bad.rows.back().terms[0].variable.model_id = 0;
  assert(session.solve(bad).termination == O::Termination::InvalidModel);
  compare(session, model.snapshot());

  // MIP starts are revalidated after each edit and user hints have precedence.
  O::Model mip;
  const auto z = mip.add_integer(0, 5);
  mip.minimize({{z,1}}, 2);
  close(*compare(session, mip.snapshot()).objective, 2);
  mip.set_objective_offset(3);
  auto warm = compare(session, mip.snapshot());
  assert(warm.start_submitted);
  assert(session.statistics().incumbent_starts == 1);
  mip.set_bounds(z, 2, 5);
  warm = compare(session, mip.snapshot());
  assert(!warm.start_submitted);
  close(*warm.objective, 5);
  options.primal_start = {{z,4}};
  auto before = session.statistics().incumbent_starts;
  warm = compare(session, mip.snapshot(), options);
  assert(warm.start_submitted);
  assert(session.statistics().incumbent_starts == before);
  options = {};
  options.node_limit = 1;
  compare(session, mip.snapshot(), options);
  options = {};
  compare(session, mip.snapshot(), options); // Resets persistent node option.
  options.cancellation = std::make_shared<O::CancellationToken>();
  options.cancellation->cancel();
  assert(session.solve(mip, options).termination == O::Termination::Cancelled);
  compare(session, mip.snapshot()); // Fresh token and no dangling callback.

  O::Model semi;
  auto sc = semi.add_variable(O::VariableType::SemiContinuous, 3, 8);
  auto si = semi.add_variable(O::VariableType::SemiInteger, 2.5, 7);
  auto sc_row = semi.add_row({{sc,1}}, 0, inf);
  auto si_row = semi.add_row({{si,1}}, 0, inf);
  semi.minimize({{sc,2},{si,1}}, -1);
  close(*compare(session, semi.snapshot()).objective, -1);
  semi.set_bounds(sc_row, 1, inf); semi.set_bounds(si_row, 1, inf);
  close(*compare(session, semi.snapshot()).objective, 8);
  semi.set_bounds(sc_row, 0, inf); semi.set_bounds(si_row, 0, inf);
  close(*compare(session, semi.snapshot()).objective, -1);

  O::Model logic;
  auto on = logic.add_binary(); auto quantity = logic.add_continuous(0, 10);
  O::add_indicator(logic, on, true, {{quantity,1}}, 7, inf);
  logic.minimize({{on,-20},{quantity,1}});
  close(*compare(session, logic.snapshot()).objective, -13);
  logic.set_bounds(quantity, 8, 10);
  close(*compare(session, logic.snapshot()).objective, -12);
  logic.minimize({{on,20},{quantity,1}});
  close(*compare(session, logic.snapshot()).objective, 8);

  // Diagnostic reductions can leave only constant rows and integer columns.
  // These must never enter HiGHS MIP presolve with an empty nonzero matrix.
  O::Model constants;
  const auto unused_bit = constants.add_binary();
  const auto constant_row = constants.add_row({}, -1, 1);
  constants.minimize({{unused_bit,2}}, -4);
  close(*compare(session, constants.snapshot()).objective, -4);
  constants.set_bounds(constant_row, 1, inf);
  assert(compare(session, constants.snapshot()).termination == O::Termination::Infeasible);
  constants.set_bounds(constant_row, 0, inf);
  close(*compare(session, constants.snapshot()).objective, -4);
  constants.set_bounds(constant_row, 1e-8, inf);
  close(*compare(session, constants.snapshot()).objective, -4);
  O::SolveOptions tighter;
  tighter.feasibility_tolerance = 1e-9;
  assert(compare(session, constants.snapshot(), tighter).termination == O::Termination::Infeasible);

  // Seeded finite integer edits checked by a separate exhaustive oracle.
  O::Model finite;
  const auto a = finite.add_integer(-3, 5);
  const auto b = finite.add_integer(-3, 5);
  const auto row = finite.add_row({{a,2},{b,-1}}, 1, inf);
  std::mt19937 rng(219);
  for (int iteration=0; iteration<80; ++iteration) {
    const int cost_a = int(rng()%9)-4, cost_b = int(rng()%9)-4;
    const int lower = int(rng()%14)-4;
    finite.set_bounds(row, lower, inf);
    finite.minimize({{a,double(cost_a)},{b,double(cost_b)}}, -7);
    double expected = inf;
    for (int av=-3; av<=5; ++av)
      for (int bv=-3; bv<=5; ++bv)
        if (2*av-bv >= lower)
          expected = std::min(expected, double(cost_a*av+cost_b*bv-7));
    const auto answer = session.solve(finite);
    assert(answer.termination == O::Termination::Optimal && answer.has_solution());
    close(*answer.objective, expected);
  }

  // LP infeasibility/unboundedness must not poison a later feasible revision.
  O::Model lp;
  const auto free = lp.add_continuous(-inf, inf);
  lp.minimize({{free,1}});
  assert(compare(session, lp.snapshot()).termination == O::Termination::Unbounded);
  lp.set_bounds(free, 2, 5);
  close(*compare(session, lp.snapshot()).objective, 2);
  const auto impossible = lp.add_row({{free,1}}, 6, inf);
  assert(compare(session, lp.snapshot()).termination == O::Termination::Infeasible);
  lp.set_bounds(impossible, 3, inf);
  close(*compare(session, lp.snapshot()).objective, 3);
  options = {};
  options.time_limit_seconds = 0.5;
  for (int i=0; i<30; ++i) {
    lp.set_objective_offset(i);
    close(*compare(session, lp.snapshot(), options).objective, i+3);
  }

  auto transferred = std::move(session);
  assert(session.solve(lp).termination == O::Termination::InvalidModel);
  compare(transferred, lp.snapshot());
  session.reset();
  assert(session.statistics().solve_calls == 0);
  compare(session, lp.snapshot());
  assert(session.statistics().model_loads == 1);
  O::Model empty;
  empty.minimize({}, 11);
  close(*session.solve(empty).objective, 11);
  compare(session, lp.snapshot());
  assert(session.statistics().model_loads == 2);
  O::Model moved;
  O::Model owner(std::move(moved));
  assert(session.solve(moved).termination == O::Termination::InvalidModel);
  std::cout << "persistent session and exhaustive edit conformance passed\n";
}
