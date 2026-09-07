/* Persistent numerical reoptimization with explicit state reuse. */
#ifndef GECODE_OPTIMIZE_SESSION_HPP
#define GECODE_OPTIMIZE_SESSION_HPP

#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/lp_observations.hpp>
#include <gecode/optimize/lp_basis.hpp>
#include <memory>

namespace Gecode { namespace Optimize {

struct SessionStatistics {
  std::uint64_t solve_calls = 0;
  std::uint64_t model_loads = 0;
  std::uint64_t incremental_updates = 0;
  std::uint64_t unchanged_models = 0;
  /** Runs entered with a retained valid LP basis; no speedup is implied. */
  std::uint64_t basis_warm_starts = 0;
  /** Previously validated MIP witnesses, rechecked and submitted as hints. */
  std::uint64_t incumbent_starts = 0;
};

/**
 * Owns one HiGHS instance. Bounds, costs, row sides, sense and offset edits
 * reuse its model; matrix/type/slot/owner changes reload it. Every snapshot is
 * validated and compared by content, including when its revision is unchanged.
 * LP bases can survive compatible edits. MIP trees/cuts are not retained;
 * previous MIP solutions are reused only after original-model revalidation.
 * Explicit primal starts take precedence. Each call has a fresh solve budget.
 *
 * Auto selects HiGHS; Native sessions are currently Unsupported. Separate
 * sessions may be used independently. Do not call one session concurrently.
 * Moving transfers all state; solving a moved-from session returns InvalidModel.
 * reset() discards model, basis, witnesses and counters (and revives a moved
 * session). Results remain owning historical snapshots after edits or reset.
 */
class SolveSession {
public:
  SolveSession();
  ~SolveSession();
  SolveSession(SolveSession&&) noexcept;
  SolveSession& operator=(SolveSession&&) noexcept;
  SolveSession(const SolveSession&) = delete;
  SolveSession& operator=(const SolveSession&) = delete;

  SolveResult solve(const ModelSnapshot& model, const SolveOptions& options = {});
  SolveResult solve(const Model& model, const SolveOptions& options = {});
  /** Owning continuous-LP observations; same session reuse/lifetime contract. */
  LpObservedResult solve_lp_observed(const ModelSnapshot& model,
                                   const LpObservationOptions& options = {});
  LpObservedResult solve_lp_observed(const Model& model,
                                   const LpObservationOptions& options = {});
  LpBasisSolveResult solve_lp_with_basis(const ModelSnapshot& model,
                                        const LpBasisSolveOptions& options);
  LpBasisSolveResult solve_lp_with_basis(const Model& model,
                                        const LpBasisSolveOptions& options);
  SessionStatistics statistics() const noexcept;
  void reset();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}}
#endif
