/* Explicit native search with independently checked integer LP deductions. */
#ifndef GECODE_OPTIMIZE_NATIVE_LP_HPP
#define GECODE_OPTIMIZE_NATIVE_LP_HPP

#include <gecode/optimize/native.hpp>

namespace Gecode { namespace Optimize {

enum class NativeLpFrequency { Root, AfterBoundChanges };

/** Optional root strengthening limits, independent of the native search limits.
 * Zero work/round/storage caps are valid and preserve the original solve.
 */
struct NativeRootCoverSettings {
  std::size_t max_rounds = 4, max_work = 4000000;
  std::size_t max_cuts = 64, max_cut_nonzeros = 4096;
  std::size_t max_model_columns = 200000, max_model_rows = 200000;
  std::size_t max_model_nonzeros = 2000000;
  std::size_t max_separation_rows = 256, max_terms_per_row = 512;
  unsigned int denominator = 1048576;
  void validate() const;
};

enum class NativeRootCoverCompletion {
  NotRequested, NotStarted, NoNewCuts, RoundLimit, WorkLimit, StorageLimit,
  SeparationLimit, Cancelled, TimeLimit, NoPrimalSuggestion, InvalidSuggestion,
  CallbackError, BackendError, AllocationFailure
};

struct NativeRootCoverStatistics {
  bool requested = false;
  NativeRootCoverCompletion completion = NativeRootCoverCompletion::NotRequested;
  std::size_t rounds = 0, augmentations = 0, cuts = 0, nonzeros = 0, work = 0;
  std::size_t projected_coordinates = 0, unsupported_rows = 0, oversized_rows = 0;
  std::size_t separated_cuts = 0, duplicate_cuts = 0, arithmetic_rejections = 0;
  // Root-loop totals, including intermediate backend instances. Already included
  // in the enclosing NativeLpStatistics totals, not additional calls to add.
  std::uint64_t lp_calls = 0, valid_bounds = 0, rejected_bounds = 0;
  std::uint64_t numerical_infeasibility_reports = 0;
  double lp_seconds = 0;
};

struct NativeLpSettings {
  NativeLpFrequency frequency = NativeLpFrequency::Root;
  bool bound_tightening = true;
  /** Reoptimize after this many observed interval changes; always try at root. */
  unsigned int bound_change_interval = 1;
  /** Absent by default. Adds verified global covers of original ordinary rows. */
  std::optional<NativeRootCoverSettings> root_cover_cuts;
  void validate() const;
};

struct NativeLpOptions : NativeLpSettings {
  SolveOptions solve;
  void validate() const;
};

struct NativeLpStatistics {
  std::uint64_t lp_calls = 0;
  double lp_seconds = 0;
  std::uint64_t valid_bounds = 0;
  std::uint64_t rejected_bounds = 0;
  /** Diagnostic LP reports, never sufficient to prune native search. */
  std::uint64_t numerical_infeasibility_reports = 0;
  std::uint64_t certificate_evaluations = 0;
  std::uint64_t conditional_checks = 0;
  std::uint64_t variable_fixings = 0;
  std::uint64_t variable_bound_tightenings = 0;
  NativeRootCoverStatistics root_cover;
};

struct NativeLpResult {
  SolveResult result;
  NativeLpStatistics relaxation;
};

BackendCapabilities native_lp_capabilities();

/**
 * Explicit opt-in: ordinary solve()/Auto/Native defaults are unchanged.
 * Native propagation enforces every original constraint. Only ordinary linear
 * rows enter the sparse relaxation; semivariables use their convex-hull box.
 * LP bounds and interval cuts require checked integer certificates. This is
 * exact discrete solving, not a complete independently checkable solve proof.
 * Missing native/HiGHS/checked-wide-integer support is Unsupported, never a
 * fallback. Original identities, limits and native arithmetic guards apply.
 * Deadlines are cooperative: an LP attempt has a 0.2-second/10,000-iteration
 * backend limit, but neither that limit nor native propagation is preemptive.
 * See NATIVE-LP.md for scope, scheduling and interrupted-bound semantics.
 */
NativeLpResult solve_native_lp(const ModelSnapshot& model,
                              const NativeLpOptions& options = {});
NativeLpResult solve_native_lp(const Model& model,
                              const NativeLpOptions& options = {});

}}
#endif
