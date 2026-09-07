/* Optional bridge from the sparse integer model to native Gecode search. */
#ifndef GECODE_OPTIMIZE_NATIVE_HPP
#define GECODE_OPTIMIZE_NATIVE_HPP

#include <gecode/optimize/solve.hpp>

namespace Gecode { namespace Optimize {

BackendCapabilities native_capabilities();

/**
 * Solve the supported finite integer subset with native Gecode propagation/BAB.
 * Original indicators use native reification, not their numerical big-M rows.
 * Exact requests use integer arithmetic checks; Certified is unsupported.
 * Interrupted results have no global bound. See NATIVE.md for numeric limits.
 * This explicit entry point accepts backend Auto or Native. Complete exact
 * starts are supported, including deterministic completion of omitted live
 * indicator inactivity gates. Unresolved partial starts, multiple workers and
 * nonzero random seeds are unsupported. The Model overload includes snapshotting
 * in its deadline and translates moved-from model errors to InvalidModel.
 */
SolveResult solve_native(const ModelSnapshot& model,
                         const SolveOptions& options = {});
SolveResult solve_native(const Model& model, const SolveOptions& options = {});

/**
 * Conservatively select a native algorithm from bounded model structure.
 * Globals, indicators, nonbinary/tiny/large models, unsupported optional-route
 * options and unavailable checked LP retain ordinary native propagation/BAB.
 * Eligible binary knapsack models retain their exact DP strengthening. Other
 * moderate binary linear models may use checked root/updated LP, verified
 * covers and bounded binary reliability branching. No problem-family metadata,
 * reference objective or external start is inferred. Bounded exact presolve,
 * independent-component solving and duplicate-column symmetry are automatic
 * for compatible ordinary integer models without supplied starts. Actual backend identity
 * and a policy explanation are returned in the ordinary SolveResult.
 * Selection, compilation, search, validation and Model snapshotting share the
 * solve deadline. Explicit solve_native/LP/search entry points are unchanged.
 */
SolveResult solve_native_auto(const ModelSnapshot& model,
                             const SolveOptions& options = {});
SolveResult solve_native_auto(const Model& model,
                             const SolveOptions& options = {});

/** Optional automatic transformations. Disabling a feature skips its work;
 * enabling it permits the conservative structural policy to use it when safe.
 * These controls apply throughout reduced models and independent components.
 * They do not change the behavior of explicit solve_native/LP/search APIs.
 */
struct NativeAutoSettings {
  /** Exact reductions, including one safely reconstructable affine objective
   * auxiliary (for example a MiniZinc compiler-generated objective variable).
   */
  bool presolve = true;
  bool components = true;
  bool symmetry = true;
  bool knapsack = true;
};

struct NativeAutoOptions {
  SolveOptions solve;
  NativeAutoSettings settings;
};

/** Run the automatic policy with explicit transformation controls. A separate
 * name preserves unambiguous existing calls such as solve_native_auto(model,{}).
 */
SolveResult solve_native_auto_configured(const ModelSnapshot& model,
                                        const NativeAutoOptions& options = {});
SolveResult solve_native_auto_configured(const Model& model,
                                        const NativeAutoOptions& options = {});

/** Opt-in sequential probe-and-select policy for native Gecode optimization.
 * Racing can INCREASE total CPU work and solve time: probes and restarting the
 * selected strategy repeat work. Several seconds (or longer when configured)
 * can nevertheless discover a substantially better search route. Early progress
 * is a heuristic, not a prediction or a guarantee of eventual speedup.
 * The optional Optimize MiniZinc/FlatZinc frontend exposes this policy separately.
 */
struct NativeRaceOptions {
  SolveOptions solve;
  /** Total nominal exploration time, shared equally by two sequential probes.
   * Finite solve limits reserve at least 75% for the selected restart. Zero
   * disables exploration and runs solve_native_auto directly. Cooperative
   * propagation/LP calls can overrun a probe's local deadline.
   */
  double exploration_seconds = 2.0;
  /** Per-probe node allowance; all probe/restart nodes also count globally. */
  std::uint64_t probe_node_limit = 4096;
  /** Controls only the automatic candidate, including a selected restart or
   * direct-path fallback. The ordinary native comparator retains its existing
   * eligible exact knapsack DP, independently of these settings.
   */
  NativeAutoSettings automatic;
  void validate() const;
};

/** Compare automatic preprocessing/LP policy with ordinary native BAB, preserving
 * eligible exact knapsack DP by default in both. Return immediately on a complete proof;
 * otherwise restart the candidate with the best original validated incumbent,
 * then strongest valid bound, preferring automatic policy on ties. Validated
 * incumbents and original-model bounds from probes are retained. Search trees
 * are not resumed. All phases share one wall/node/cancellation budget.
 * Globals, indicators and incompatible options retain the automatic direct path.
 */
SolveResult solve_native_race(const ModelSnapshot& model,
                             const NativeRaceOptions& options = {});
SolveResult solve_native_race(const Model& model,
                             const NativeRaceOptions& options = {});

}}
#endif
