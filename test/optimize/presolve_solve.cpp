#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace Gecode::Optimize;
int main() {
  std::size_t solved = 0;
  for (bool maximize : {false, true}) for (std::size_t passes : {0U, 100U}) {
    Model source;
    auto removed = source.add_integer(-3, 3); source.remove(removed);
    auto fixed = source.add_integer(2, 2), variable = source.add_integer(-2, 2);
    source.add_row({{fixed, 3}, {variable, -2}}, 4, 8, "range");
    source.set_objective({{fixed, -3}, {variable, 2}},
      maximize ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, 11);
    PresolveOptions options; options.max_passes = passes;
    const auto prepared = presolve_integer(source, options);
    assert(prepared.model && prepared.fixed_variables == 1);
    assert(prepared.status == (passes ? PresolveStatus::Fixpoint : PresolveStatus::Incomplete));
    const auto saved_revision = source.revision();
    source.set_bounds(variable, 0, 0); // transformation remains historical
    for (int backend = 0; backend < 3; ++backend) {
      const bool available = backend == 0 ? capabilities(Backend::Highs).available :
        backend == 1 ? native_capabilities().available : native_lp_capabilities().available;
      if (!available) continue;
      SolveOptions solve_options;
      solve_options.backend = backend == 0 ? Backend::Highs : Backend::Native;
      solve_options.guarantee = backend == 0 ? Guarantee::Numerical : Guarantee::Exact;
      SolveResult reduced;
      if (backend == 2) {
        NativeLpOptions hybrid; hybrid.solve = solve_options;
        reduced = solve_native_lp(prepared.model->reduced(), hybrid).result;
      } else reduced = solve(prepared.model->reduced(), solve_options);
      assert(reduced.termination == Termination::Optimal && reduced.has_solution());
      const auto restored = prepared.model->postsolve(reduced);
      assert(restored.exact_witness_validated && restored.solution.has_solution());
      assert(restored.solution.objective == (maximize ? 7 : 3));
      assert(restored.solution.value(fixed) == 2 && restored.solution.value(variable) == (maximize ? 1 : -1));
      assert(restored.solution.model_id == source.id() && restored.solution.revision == saved_revision);
      assert(restored.solution.revision != source.revision());
      assert(!restored.solution.active_variables[removed.id] && std::isnan(restored.solution.values[removed.id]));
      assert(restored.solution.termination == Termination::Unknown && !restored.solution.best_bound && !restored.solution.absolute_gap);
      assert(validate(prepared.model->original(), restored.solution.values, 0, 0).valid);
      assert(!validate(source.snapshot(), restored.solution.values, 0, 0).valid);
      ++solved;
    }
  }
  // A propagation fixpoint is not a feasibility or optimization result.
  Model parity;
  const auto x = parity.add_binary(), y = parity.add_binary(), z = parity.add_binary();
  parity.add_row({{x, 2}, {y, 2}, {z, 2}}, 3, 3);
  const auto prepared = presolve_integer(parity);
  assert(prepared.status == PresolveStatus::Fixpoint && prepared.model);
  for (const auto backend : {Backend::Highs, Backend::Native}) if (capabilities(backend).available) {
    SolveOptions options; options.backend = backend;
    const auto result = solve(prepared.model->reduced(), options);
    assert(result.termination == Termination::Infeasible && !result.has_solution());
  }
  std::cout << solved << " actual backend solves with exact historical reconstruction pass\n";
}
