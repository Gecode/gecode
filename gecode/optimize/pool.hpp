/* Ranked solutions over a finite discrete projection. */
#ifndef GECODE_OPTIMIZE_POOL_HPP
#define GECODE_OPTIMIZE_POOL_HPP
#include <gecode/optimize/solve.hpp>

namespace Gecode { namespace Optimize {

enum class PoolCompletion { Incomplete, RequestedLimit, Exhausted };
struct PoolOptions {
  SolveOptions solve;
  std::size_t max_solutions = 10;
  // Default: every active Integer/Binary variable. Explicit projections retain
  // caller order, contain no duplicate handles, and must be finite and nonempty
  // unless the original model has no active variables.
  std::optional<std::vector<Variable>> projection;
};
struct PoolEntry {
  // Original historical identity/slots, independently validated. Its scalar
  // objective is original; bounds/status are cleared because later pool solves
  // optimize a restricted feasible set, not the entire original model.
  SolveResult solution;
  std::vector<std::int64_t> projection_values;
  bool rank_established = false;
};
struct PoolAttempt {
  Termination termination = Termination::Unknown;
  Guarantee guarantee = Guarantee::Numerical;
  std::optional<double> objective;
  std::optional<double> remaining_bound;
  bool candidate_accepted = false;
  bool rank_established = false;
};
struct PoolResult {
  ModelId model_id = 0;
  Revision revision = 0;
  Termination termination = Termination::Unknown;
  PoolCompletion completion = PoolCompletion::Incomplete;
  Guarantee guarantee = Guarantee::Numerical;
  std::string message;
  std::vector<Variable> projection;
  std::vector<PoolEntry> entries;
  std::vector<PoolAttempt> attempts;
  std::size_t ranked_prefix = 0;
  double elapsed_seconds = 0;
  bool exhausted() const noexcept { return completion == PoolCompletion::Exhausted; }
};

/**
 * Repeated optimization plus safe no-good exclusions. Returns one representative
 * per finite Integer/Binary projection assignment, ranked by its best original
 * objective when each remaining-model solve completes. Continuous and unbounded
 * non-projected Integer recourse are allowed by the HiGHS backend. Real-valued
 * completions of a projection are not enumerated. Ties have unspecified order.
 * An interrupted timely candidate may be a final unranked entry. Only definite
 * infeasibility of the remaining model establishes projection exhaustion.
 * Exhaustion is a workflow Optimal status; the last oracle attempt is Infeasible.
 * RequestedLimit uses SolutionLimit and never asserts exhaustion. Numerical
 * completion is tolerance-qualified; Exact requires explicit Native support.
 * Semis, active indicators/globals, Certified, and multistage node budgets are
 * explicitly Unsupported. No original model edits occur. See POOLS.md.
 */
PoolResult solve_pool(const ModelSnapshot& model, const PoolOptions& options = {});
PoolResult solve_pool(const Model& model, const PoolOptions& options = {});
}}
#endif
