/* Results and shared solve budgets for the additive optimization API. */
#ifndef GECODE_OPTIMIZE_RESULT_HPP
#define GECODE_OPTIMIZE_RESULT_HPP

#include "types.hpp"

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Gecode { namespace Optimize {

/** The reason a solve stopped, independent of whether it found a solution. */
enum class Termination {
  Unknown,
  Optimal,
  Infeasible,
  Unbounded,
  InfeasibleOrUnbounded,
  TimeLimit,
  NodeLimit,
  MemoryLimit,
  IterationLimit,
  SolutionLimit,
  ObjectiveLimit,
  Cancelled,
  NumericalFailure,
  Unsupported,
  InvalidModel,
  BackendError
};

const char* to_string(Termination termination) noexcept;

/** Monotonic cancellation: a cancelled token cannot be reset or reused. */
class CancellationToken {
public:
  void cancel() noexcept;
  bool cancelled() const noexcept;

private:
  std::atomic<bool> cancelled_{false};
};

/** One original-variable value in a possibly partial primal start. */
struct StartValue {
  Variable variable;
  double value = 0.0;
};

struct SolveOptions {
  Backend backend = Backend::Auto;
  Guarantee guarantee = Guarantee::Numerical;
  double time_limit_seconds = std::numeric_limits<double>::infinity();
  int threads = 1;
  int random_seed = 0;
  double relative_gap = 1e-4;
  double absolute_gap = 1e-6;
  double feasibility_tolerance = 1e-7;
  double integrality_tolerance = 1e-6;
  std::shared_ptr<CancellationToken> cancellation;
  std::optional<std::uint64_t> node_limit;
  /**
   * Optional sparse primal start. Values must be finite, unique, and satisfy
   * their original variable domains. A complete start must satisfy every
   * original constraint, including retained indicators and globals.
   * A partial start is a hint, not a validated incumbent or a fixed assignment.
   * HiGHS accepts partial hints. Native requires a complete exact assignment
   * after any deterministic completion of live indicator inactivity gates;
   * unresolved partial starts are Unsupported. See solve_native().
   */
  std::vector<StartValue> primal_start;

  /**
   * Validate values, throwing ModelError for malformed options.
   * Exact and Certified requests are valid options; a backend that cannot
   * deliver the requested guarantee must return Termination::Unsupported.
   * Gap tolerances may be zero. Feasibility/integrality tolerances must be
   * positive and finite; integrality tolerance must also be less than 0.5.
   */
  void validate() const;
};

/**
 * Copies share a monotonic clock, cancellation token and cumulative node count.
 * All stages of one solve should use this same budget rather than restarting
 * the deadline. Callers explicitly add newly consumed nodes (not a cumulative
 * backend total on every poll). Concurrent node additions saturate at UINT64_MAX.
 */
class SolveBudget {
public:
  explicit SolveBudget(const SolveOptions& options);

  /** A locally capped view sharing the original clock, nodes and cancellation.
   * Exhausting a slice does not cancel its parent. Nested slices cannot relax
   * any parent cap. All nodes consumed through a slice count globally.
   */
  SolveBudget slice(double seconds, std::uint64_t node_allowance) const;

  double elapsed_seconds() const noexcept;
  double remaining_seconds() const noexcept;
  bool cancelled() const noexcept;
  bool time_limit_reached() const noexcept;
  bool node_limit_reached() const noexcept;
  bool expired() const noexcept;
  /** Cancellation takes precedence over time, then node limits. */
  std::optional<Termination> stop_reason() const noexcept;

  void add_nodes(std::uint64_t count = 1) noexcept;
  std::uint64_t nodes() const noexcept;
  std::shared_ptr<CancellationToken> cancellation() const noexcept;

private:
  struct State;
  std::shared_ptr<State> state_;
  double local_deadline_ = std::numeric_limits<double>::infinity();
  std::optional<std::uint64_t> local_node_limit_;
};

/**
 * An owning historical result. Model edits do not invalidate this snapshot.
 * Values and active_variables use original variable slots, including tombstones.
 * Only an independent original-model validator may set solution_validated.
 */
struct SolveResult {
  ModelId model_id = 0;
  Revision revision = 0;
  std::string backend;
  std::string backend_version;
  Termination termination = Termination::Unknown;
  std::string message;
  std::optional<double> objective;
  std::optional<double> best_bound;
  std::optional<double> absolute_gap;
  std::optional<double> relative_gap;
  std::vector<double> values;
  std::vector<bool> active_variables;
  bool solution_validated = false;
  Guarantee guarantee = Guarantee::Numerical;
  double elapsed_seconds = 0.0;
  std::optional<double> native_backend_gap;
  /** Backend accepted the hint; this does not claim it became an incumbent. */
  bool start_submitted = false;

  /** Requires a finite validated objective and every active slot to be finite. */
  bool has_solution() const noexcept;
  /** Reject absent solutions, foreign handles, invalid slots and tombstones. */
  double value(Variable variable) const;

  /**
   * Recompute gaps from the original-sense objective and global best_bound.
   * Missing values or a valid infinite bound leave both gaps unavailable.
   * NaNs, non-finite objectives and inconsistent bound ordering throw ModelError;
   * previous gaps are cleared before validation, never silently clamped to zero.
   * The common normalized relative gap is abs(objective-best_bound) divided by
   * max(1, abs(objective), abs(best_bound)); it is finite for finite inputs,
   * including zero objectives. The absolute gap can overflow double to positive
   * infinity. native_backend_gap preserves the vendor convention separately.
   */
  void update_gaps(ObjectiveSense sense);
};

}}
#endif
