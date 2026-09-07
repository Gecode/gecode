/* Explicit bounded primal neighborhoods alongside the native proof frontier. */
#ifndef GECODE_OPTIMIZE_NATIVE_NEIGHBORHOODS_HPP
#define GECODE_OPTIMIZE_NATIVE_NEIGHBORHOODS_HPP

#include <gecode/optimize/native_search.hpp>

namespace Gecode { namespace Optimize {

enum class NativeNeighborhoodPolicy { BinaryHamming };

/** Limits apply to one optional attempt, sharing the ordinary solve budget.
 * Finite provisional settings for this explicit experimental API. Zero caps
 * skip optional work; no existing solve route enables neighborhoods implicitly.
 */
struct NativeNeighborhoodSettings {
  NativeNeighborhoodPolicy policy = NativeNeighborhoodPolicy::BinaryHamming;
  std::size_t radius = 1;
  std::uint64_t max_status_calls = 128;
  std::size_t max_distance_variables = 4096;
  std::size_t max_source_entries = 100000;
  std::size_t max_coordinator_work = 100000;
  std::size_t max_local_spaces = 64;
  double time_limit_seconds = 0.05;
  void validate() const;
};

enum class NativeNeighborhoodCompletion {
  NotStarted, NoIncumbent, ProofCompletedBeforeAttempt, NoEligibleBinary,
  NonrestrictingRadius, FormulationLimit, SourceLimit, WorkLimit, StatusLimit,
  SharedNodeReserve, LocalStorageLimit, LocalTimeLimit, NoImprovement, Improved,
  GlobalStop, Error
};

struct NativeNeighborhoodStatistics {
  bool requested = false;
  NativeNeighborhoodCompletion completion = NativeNeighborhoodCompletion::NotStarted;
  std::optional<Termination> stop_reason;
  std::uint64_t attempts = 0;
  std::size_t eligible_variables = 0, source_entries = 0, coordinator_work = 0;
  // An admitted status attempt can be stopped before propagation actually runs.
  std::uint64_t status_attempts = 0, completed_status_calls = 0;
  std::uint64_t failed_nodes = 0, feasible_candidates = 0, accepted_improvements = 0;
  std::size_t peak_local_spaces = 0, peak_total_spaces = 0;
  // Shared total, not another component to add to frontier/probe/local counts.
  std::uint64_t budget_nodes = 0;
  // Includes eligibility, construction, checking, and release; no local LP work.
  double elapsed_seconds = 0;
};

struct NativeNeighborhoodOptions {
  NativeSearchOptions search;
  NativeNeighborhoodSettings neighborhood;
  void validate() const;
};

struct NativeNeighborhoodResult {
  NativeSearchResult search;
  NativeNeighborhoodStatistics neighborhood;
};

/** Run at most one global BinaryHamming improvement attempt after a checked
 * incumbent, at a stable main-parent boundary. Distance counts active nonfixed
 * original Binary slots without indicator_origin; all other original variables
 * and native constraints remain in the isolated subproblem. There is no partial
 * start repair, current-node LP suggestion, local proof sharing or recursive
 * heuristic. Only a timely exact original-model-validated improvement is shared.
 *
 * Shared node attempts = ordinary frontier admissions + reliability probe
 * attempts + neighborhood status attempts. Optional admissions reserve two ordinary
 * child slots. Local caps end only the heuristic; global limits stop the solve.
 * Main queued/active regions continue to define any interrupted global bound.
 * Native propagation/posting/checking is cooperative, not hard-preemptible.
 * Existing NativeSearch option/result layouts and existing APIs are unchanged.
 */
NativeNeighborhoodResult solve_native_neighborhoods(
    const ModelSnapshot&, const NativeNeighborhoodOptions& = {});
NativeNeighborhoodResult solve_native_neighborhoods(
    const Model&, const NativeNeighborhoodOptions& = {});

}}
#endif
