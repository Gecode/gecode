/* Owning numerical observations of ordinary continuous linear programs. */
#ifndef GECODE_OPTIMIZE_LP_OBSERVATIONS_HPP
#define GECODE_OPTIMIZE_LP_OBSERVATIONS_HPP

#include <gecode/optimize/solve.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Gecode { namespace Optimize {
namespace Detail { struct LpObservationAccess; }

enum class LpObservationState { NotRequested, Available, Unavailable, Rejected };
enum class LpObservationReason {
  None, NotRequested, Unsupported, NoBackendSolve, NoPrimalPoint, NotOptimal,
  NoDualPoint, NoBasis, ElidedConstantRows, Interrupted, InvalidBackendData,
  FailedChecks, AllocationFailure, InvalidModel
};
enum class LpBasisStatus { Lower, Basic, Upper, Zero, NonbasicUnspecified };
enum class LpDualSource { None, Backend, DerivedConstantRow };

struct LpObservationGroup {
  LpObservationState state = LpObservationState::Unavailable;
  LpObservationReason reason = LpObservationReason::NoBackendSolve;
  std::string message;
};

/** Absolute tolerances in original units; each is finite and nonnegative. */
struct LpCheckTolerances {
  double dual_feasibility = 1e-7;
  double stationarity = 1e-7;
  double complementarity = 1e-6;
  double objective_gap = 1e-6;
  void validate() const;
};
struct LpObservationOptions {
  SolveOptions solve;
  bool duals = true;
  bool basis = true;
  LpCheckTolerances checks;
  void validate() const;
};

/** Optional metrics are absent unless their complete computation succeeded. */
struct LpKktReport {
  bool primal_valid = false;
  bool dual_signs_valid = false;
  bool stationarity_valid = false;
  bool complementarity_valid = false;
  bool gap_valid = false;
  bool accepted = false;
  std::optional<double> max_dual_sign_violation;
  std::optional<double> max_stationarity;
  std::optional<double> max_complementarity;
  /** Original objective sense, numerical estimate; never an exact bound. */
  std::optional<double> dual_objective_estimate;
  /** Normalized primal-minus-dual, with the common offset cancelled first. */
  std::optional<double> normalized_gap;
  std::string message;
};

struct LpRowObservation {
  bool active = false;
  std::optional<double> activity;
  /** Absent for an infinite side or unavailable primal observation. */
  std::optional<double> lower_slack, upper_slack;
  std::optional<double> dual;
  LpDualSource dual_source = LpDualSource::None;
  std::optional<LpBasisStatus> basis;
};
struct LpColumnObservation {
  bool active = false;
  std::optional<double> reduced_cost;
  std::optional<LpBasisStatus> basis;
};
struct LpObservationMetadata {
  std::string backend, backend_version;
  LpCheckTolerances checks;
  double primal_check_tolerance = 1e-7;
  std::optional<double> backend_primal_tolerance, backend_dual_tolerance;
};

/**
 * Immutable original-coordinate data; owns the exact historical source model.
 * Groups distinguish unavailable data from zero and from empty available arrays.
 * Row/column lookups reject foreign handles and original tombstones. Accessors
 * perform no solver work and remain usable after model/session destruction.
 * Dual values use A^T*pi + reduced_cost = c in the original objective sense.
 * Basis status is numerical backend data, not a proof of feasibility/optimality.
 */
class LpObservations {
public:
  LpObservations(const LpObservations&) = delete;
  LpObservations& operator=(const LpObservations&) = delete;
  ModelId id() const noexcept { return source_.model_id; }
  Revision revision() const noexcept { return source_.revision; }
  const ModelSnapshot& source() const noexcept { return source_; }
  const LpObservationMetadata& metadata() const noexcept { return metadata_; }
  const LpObservationGroup& primal_rows() const noexcept { return primal_rows_; }
  const LpObservationGroup& dual_point() const noexcept { return dual_point_; }
  const LpObservationGroup& basis() const noexcept { return basis_; }
  const LpKktReport& checks() const noexcept { return checks_; }
  const std::vector<LpRowObservation>& rows() const noexcept { return rows_; }
  const std::vector<LpColumnObservation>& columns() const noexcept { return columns_; }
  const LpRowObservation& row(Constraint constraint) const;
  const LpColumnObservation& column(Variable variable) const;

private:
  LpObservations() = default;
  friend struct Detail::LpObservationAccess;
  ModelSnapshot source_;
  LpObservationMetadata metadata_;
  LpObservationGroup primal_rows_, dual_point_, basis_;
  LpKktReport checks_;
  std::vector<LpRowObservation> rows_;
  std::vector<LpColumnObservation> columns_;
};

struct LpObservedResult {
  SolveResult result;
  /** May be null when invalid input or allocation failure prevents capture. */
  std::shared_ptr<const LpObservations> observations;
};
struct LpObservationCapabilities {
  bool available = false;
  bool duals = false;
  bool basis_export = false;
  std::string backend, backend_version;
  std::vector<std::string> limitations;
};
LpObservationCapabilities lp_observation_capabilities();

/**
 * HiGHS Numerical only, on original Continuous linear models without active
 * indicators or globals. No automatic relaxation or fallback. The whole-call
 * cooperative budget includes snapshot copying, conversion and checking.
 * Duals require a timely optimal primal/dual point; basis absence
 * does not invalidate a validated primal solution. No ray/ranging work occurs.
 */
LpObservedResult solve_lp_observed(const ModelSnapshot&,
                                  const LpObservationOptions& = {});
LpObservedResult solve_lp_observed(const Model&,
                                  const LpObservationOptions& = {});

}}
#endif
