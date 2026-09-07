/* Explicit bounded native frontier search with conservative global bounds. */
#ifndef GECODE_OPTIMIZE_NATIVE_SEARCH_HPP
#define GECODE_OPTIMIZE_NATIVE_SEARCH_HPP

#include <gecode/optimize/native_lp.hpp>

namespace Gecode { namespace Optimize {

enum class NativeSearchOrder { DepthFirst, BestBound };

enum class NativeBranchingPolicy { BinaryReliability };

/** Experimental binary directional propagation gains; not LP pseudocosts.
 * Resource caps may be zero to skip optional work. The sample threshold must
 * be positive. Only completed finite probe pairs update solve-local history.
 */
struct NativeBranchingSettings {
  NativeBranchingPolicy policy = NativeBranchingPolicy::BinaryReliability;
  std::size_t max_candidates_per_decision = 8;
  std::uint64_t max_probe_status_calls = 128;
  std::uint64_t max_probe_status_calls_per_decision = 8;
  std::size_t max_branching_work = 100000;
  std::uint64_t reliability_samples = 2;
  std::size_t max_nonimproving_pairs = 2;
  std::size_t max_history_entries = 4096;
  void validate() const;
};

struct NativeBranchingStatistics {
  bool requested = false;
  std::uint64_t decisions = 0, manual_splits = 0, fallback_decisions = 0;
  // Admitted attempts: a final pre-status stop can consume a slot without
  // executing propagation, just as for ordinary frontier admissions.
  std::uint64_t probe_status_calls = 0, completed_pairs = 0;
  std::uint64_t published_pairs = 0, finite_samples = 0, zero_gain_samples = 0;
  std::uint64_t failed_directions = 0, reliable_candidates = 0;
  std::uint64_t probe_propagations = 0;
  // Shared total: ordinary admissions plus probe status attempts. The explicit
  // neighborhood wrapper additionally charges its local status attempts here.
  // This repeats SolveBudget::nodes(), not another component to add to it.
  std::uint64_t budget_nodes = 0;
  std::size_t work = 0, history_entries = 0;
  // Subsets of the solve-wide relaxation totals, not additional calls to add.
  std::uint64_t probe_lp_calls = 0;
  double probe_lp_seconds = 0;
};

struct NativeSearchOptions {
  SolveOptions solve;
  NativeSearchOrder order = NativeSearchOrder::BestBound;
  // Counts queued spaces + expanding parent + in-flight child, not bytes.
  std::size_t max_open_nodes = 100000;
  // Absent means native propagation only, with no LP dependency or attempt.
  std::optional<NativeLpSettings> relaxation;
  // Absent preserves the existing native brancher and admission accounting.
  std::optional<NativeBranchingSettings> branching;
  void validate() const;
};

struct NativeFrontierStatistics {
  std::uint64_t admitted_nodes = 0;
  std::uint64_t expanded_nodes = 0;
  std::uint64_t failed_nodes = 0;
  std::uint64_t bound_pruned_nodes = 0;
  std::uint64_t feasible_leaves = 0;
  std::size_t peak_open_nodes = 0;
  // Queued nodes plus any parent/root whose region remains unresolved.
  std::size_t unresolved_regions = 0;
};

struct NativeSearchResult {
  SolveResult result;
  NativeLpStatistics relaxation;
  NativeFrontierStatistics frontier;
  NativeBranchingStatistics branching;
};

/**
 * Explicit opt-in; solve(), solve_native() and solve_native_lp() are unchanged.
 * Existing finite native integer/global/indicator semantics and limits apply.
 * Stable propagated nodes use normalized minimization bounds. Every unfinished
 * region remains represented, including a parent during partial expansion.
 * Interrupted global bounds aggregate all unresolved regions and an incumbent;
 * they are absent if compilation did not establish the initial objective box.
 * max_open_nodes exhaustion returns MemoryLimit; this is not a byte guarantee.
 * Node quotas restrict admission, allowing the final admitted node to finish
 * when time/cancellation permits. Complete exact starts are supported; no
 * partial starts, parallel workers or gap stopping.
 * An optional relaxation requires native checked LP capability explicitly.
 * BinaryReliability charges probe status attempts to the shared node quota,
 * separately reported from admitted frontier nodes. Probes select a split;
 * they never publish incumbents or pruning evidence. Limits are cooperative
 * around native propagation, which itself runs to fixpoint or failure.
 */
NativeSearchResult solve_native_search(const ModelSnapshot& model,
                                      const NativeSearchOptions& options = {});
NativeSearchResult solve_native_search(const Model& model,
                                      const NativeSearchOptions& options = {});

}}
#endif
