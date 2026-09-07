/* Owning numerical sensitivity of one specified optimal continuous LP basis. */
#ifndef GECODE_OPTIMIZE_LP_SENSITIVITY_HPP
#define GECODE_OPTIMIZE_LP_SENSITIVITY_HPP
#include <gecode/optimize/lp_basis.hpp>
#include <gecode/optimize/validate.hpp>
#include <variant>

namespace Gecode { namespace Optimize {
namespace Detail { struct LpSensitivityAccess; }

struct LpObjectiveParameter { Variable variable; };
struct LpEqualityRhsParameter { Constraint row; };
using LpSensitivityParameter=std::variant<LpObjectiveParameter,LpEqualityRhsParameter>;
using LpSensitivityEntity=std::variant<Variable,Constraint>;

enum class LpSensitivityState { NotRequested,Available,Unavailable,Rejected };
enum class LpSensitivityReason {
  None,NotRequested,Unsupported,NotOptimal,NoBasis,InvalidSource,InvalidBasis,
  ChangedBasis,FailedReferenceChecks,FailedLinearSolveChecks,FailedIntervalChecks,
  ResourceLimit,Stopped,AllocationFailure,BackendFailure
};
struct LpSensitivityGroup {
  LpSensitivityState state=LpSensitivityState::Unavailable;
  LpSensitivityReason reason=LpSensitivityReason::NotRequested;
  std::string message;
};
struct LpSensitivityTolerances {
  double primal_feasibility=1e-7;
  LpCheckTolerances kkt;
  /** |B*v-rhs| <= absolute + relative*(|B|*|v|+|rhs|), per equation. */
  double system_absolute=1e-9,system_relative=1e-9;
  void validate() const;
};
struct LpSensitivityLimits {
  std::size_t max_rows=4096,max_columns=100000,max_nonzeros=1000000;
  std::size_t max_requests=4096,max_basis_solves=8194;
  /** Checked m*m admission, not an exact backend allocation/RSS bound. */
  std::size_t max_factor_entries=16777216;
  /** Logical retained slots and coordinator visits, not bytes or CPU work. */
  std::size_t max_retained_slots=20000000,max_work=100000000;
};
struct LpSensitivityOptions {
  Backend backend=Backend::Auto;
  double time_limit_seconds=std::numeric_limits<double>::infinity();
  std::shared_ptr<CancellationToken> cancellation;
  /** Nonempty, unique requests; complete preflight precedes factorization. */
  std::vector<LpSensitivityParameter> parameters;
  LpSensitivityTolerances checks;
  LpSensitivityLimits limits;
  void validate() const;
};

enum class LpRangeEndKind { Finite,NegativeInfinity,PositiveInfinity };
struct LpRangeEnd {
  LpRangeEndKind kind=LpRangeEndKind::Finite;
  /** In an available interval, present exactly for Finite. Infinity is a
   * direction, not a parameter value. Default/unavailable data does not
   * describe an interval endpoint. */
  std::optional<double> value;
};
enum class LpSensitivitySide { Lower,Upper,Fixed,Free };
struct LpSensitivityLimiter {
  LpSensitivityEntity entity;
  LpSensitivitySide side=LpSensitivitySide::Lower;
  bool dual_condition=false;
};
struct LpIntervalCheckReport {
  bool accepted=false;
  std::size_t inequalities=0;
  std::optional<double> max_endpoint_violation;
  bool lower_direction_checked=false,upper_direction_checked=false;
  std::string message;
};
struct LpParameterInterval {
  double anchor=0;
  LpRangeEnd lower,upper;
  /** Original objective change = slope*(parameter-anchor), no offset. */
  std::optional<double> objective_slope;
  std::optional<LpSensitivityLimiter> lower_limiter,upper_limiter;
  LpIntervalCheckReport checks;
};
struct LpSensitivityEntry {
  LpSensitivityParameter parameter;
  LpSensitivityGroup group;
  /** Present only for Available; a singleton is distinct from unavailable. */
  std::optional<LpParameterInterval> interval;
};
struct LpSensitivityReferenceChecks {
  ValidationReport primal;
  LpKktReport kkt;
  bool basis_point_matches=false;
  std::optional<double> max_point_difference;
  std::optional<double> max_system_residual,max_scaled_system_residual;
};
struct LpSensitivityWork {
  bool factor_setup_attempted=false;
  /** Attempted private factor-system calls, including calls failing before the
   * backend accessor is entered; not optimization iterations or run telemetry. */
  std::size_t basis_solves=0,coordinator_visits=0,retained_slots=0;
};

/** Immutable historical source/basis/ranges. Accessors perform no backend work.
 * Original-slot masks include tombstones. Lookups reject foreign/tombstone IDs;
 * nullptr means the valid historical entity was not requested. A model/session
 * edit or destruction never changes this object. All intervals are qualified
 * Numerical, for one parameter and this selected basis/status assignment only.
 */
class LpSensitivity {
public:
  LpSensitivity(const LpSensitivity&)=delete;
  LpSensitivity& operator=(const LpSensitivity&)=delete;
  ModelId id() const noexcept {return original_.result.model_id;}
  Revision revision() const noexcept {return original_.result.revision;}
  const LpObservedResult& original() const noexcept {return original_;}
  const std::shared_ptr<const LpBasis>& basis() const noexcept {return basis_;}
  const std::vector<LpSensitivityEntity>& factor_order() const noexcept {return order_;}
  const std::vector<LpSensitivityEntry>& entries() const noexcept {return entries_;}
  const LpSensitivityEntry* objective(Variable) const;
  const LpSensitivityEntry* equality_rhs(Constraint) const;
  const std::vector<bool>& active_columns() const noexcept {return columns_;}
  const std::vector<bool>& active_rows() const noexcept {return rows_;}
  const LpSensitivityReferenceChecks& checks() const noexcept {return checks_;}
  const LpSensitivityTolerances& tolerances() const noexcept {return tolerances_;}
  const std::string& backend_version() const noexcept {return backend_version_;}
private:
  LpSensitivity()=default;
  friend struct Detail::LpSensitivityAccess;
  LpObservedResult original_;
  std::shared_ptr<const LpBasis> basis_;
  std::vector<LpSensitivityEntity> order_;
  std::vector<LpSensitivityEntry> entries_;
  std::vector<bool> columns_,rows_;
  LpSensitivityReferenceChecks checks_;
  LpSensitivityTolerances tolerances_;
  std::string backend_version_;
};
enum class LpSensitivityCompletion { Complete,Partial,Interrupted,Rejected };
struct LpSensitivityResult {
  ModelId model_id=0;
  Revision revision=0;
  LpSensitivityCompletion completion=LpSensitivityCompletion::Rejected;
  LpSensitivityReason reason=LpSensitivityReason::InvalidSource;
  std::optional<Termination> stop_reason;
  std::string message;
  std::shared_ptr<const LpSensitivity> sensitivity;
  LpSensitivityWork work;
  double elapsed_seconds=0;
};

/** Additional private factorization/linear algebra, zero optimization runs.
 * HiGHS/Auto only; a timely optimal solve_lp_observed() result on a Continuous
 * linear source with checked duals and complete basis. Empty row/column systems,
 * active indicators/globals, elided constant rows, automatic relaxation,
 * replacement bases and stale-revision transplants are unsupported. Only
 * objective coefficients and equality common RHS are supported parameter kinds.
 * Factorization/triangular solves are cooperatively budgeted. A final whole-call
 * limit, cancellation, resource or cleanup failure clears every availability;
 * original solve provenance and completed diagnostics remain historical data.
 */
LpSensitivityResult analyze_lp_sensitivity(const LpObservedResult&,
                                          const LpSensitivityOptions&);
}}
#endif
