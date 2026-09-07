/* Explicit additional-work numerical LP evidence; never an exact certificate. */
#ifndef GECODE_OPTIMIZE_LP_EVIDENCE_HPP
#define GECODE_OPTIMIZE_LP_EVIDENCE_HPP
#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/validate.hpp>

namespace Gecode { namespace Optimize {
enum class LpEvidenceRequest { Automatic, PrimalRay, Farkas, Both };
struct LpEvidenceTolerances {
  double recession = 1e-7;
  double stationarity = 1e-7;
  double minimum_improvement = 1e-7;
  double minimum_contradiction = 1e-7;
  void validate() const;
};
struct LpEvidenceLimits {
  std::size_t max_auxiliary_variables = 1000000;
  std::size_t max_auxiliary_rows = 1000000;
  std::size_t max_auxiliary_nonzeros = 10000000;
  /** Aggregate source/private snapshots, values, maps and evidence vector slots. */
  std::size_t max_retained_slots = 50000000;
  /** Deterministic coordinator element visits, excluding solver internals. */
  std::size_t max_work = 100000000;
  /** Public solve invocations, including local empty/constant decisions. */
  std::size_t max_auxiliary_solves = 3;
};
struct LpEvidenceOptions {
  SolveOptions solve;
  LpEvidenceRequest request = LpEvidenceRequest::Automatic;
  LpEvidenceTolerances checks;
  LpEvidenceLimits limits;
  void validate() const;
};
enum class LpEvidenceState { NotRequested, Available, Unavailable, Rejected };
enum class LpEvidenceReason {
  None, NotRequested, Unsupported, NoFeasibleBase, NoImprovingDirection,
  NoContradiction, Stopped, InvalidBackendData, FailedOriginalChecks,
  InconsistentEvidence, InvalidModel, ResourceLimit, AllocationFailure
};
struct LpEvidenceGroup {
  LpEvidenceState state = LpEvidenceState::NotRequested;
  LpEvidenceReason reason = LpEvidenceReason::NotRequested;
  std::string message;
};
enum class LpEvidencePhase { FeasibleBase, Recession, Farkas };
enum class LpEvidenceSide { Lower, Upper };
enum class LpEvidenceColumnKind { SourceVariable, RowSide, VariableSide };
struct LpEvidenceColumn {
  LpEvidenceColumnKind kind = LpEvidenceColumnKind::SourceVariable;
  std::size_t original_slot = 0;
  std::optional<LpEvidenceSide> side;
};
struct LpEvidenceStage {
  LpEvidencePhase phase = LpEvidencePhase::FeasibleBase;
  /** Immutable private coordinates; never an original-source SolveResult. */
  std::shared_ptr<const ModelSnapshot> auxiliary_model;
  std::vector<LpEvidenceColumn> columns;
  std::size_t nonzeros = 0;
  /** The public auxiliary solve primitive was invoked; no vendor-run telemetry. */
  bool attempted = false;
  std::optional<SolveResult> auxiliary_result;
  bool candidate_examined = false;
  ValidationReport auxiliary_check;
};
struct LpPrimalEvidence {
  /** Full original-slot vectors; inactive slots are NaN. Empty until checked. */
  std::vector<double> base_point, direction, row_direction;
  ValidationReport base_check;
  std::optional<double> direction_scale, normalized_objective_slope;
  std::optional<double> max_variable_recession_violation, max_row_recession_violation;
};
struct LpFarkasEntry {
  bool active = false;
  double multiplier = 0.0;
  /** Absent for exactly zero multiplier; contribution is then exactly zero. */
  std::optional<LpEvidenceSide> side;
  double contribution = 0.0;
};
struct LpFarkasEvidence {
  /** Signed y for rows and independently derived z=-A^T*y for columns.
   * Positive selects the lower side, negative selects the upper; accepted
   * evidence has positive total finite-side contribution. Numerical only. */
  std::vector<LpFarkasEntry> rows, columns;
  std::optional<double> multiplier_scale, contradiction_margin, max_stationarity;
};
enum class LpEvidenceCompletion { Complete, Interrupted, Rejected };
namespace Detail { struct LpEvidenceAccess; }
class LpEvidence {
public:
  LpEvidence(const LpEvidence&) = delete;
  LpEvidence& operator=(const LpEvidence&) = delete;
  const ModelSnapshot& source() const noexcept;
  ModelId id() const noexcept;
  Revision revision() const noexcept;
  const LpEvidenceTolerances& tolerances() const noexcept;
  double primal_tolerance() const noexcept;
  const LpEvidenceGroup& primal_ray() const noexcept;
  const LpEvidenceGroup& farkas() const noexcept;
  /** Diagnostics can remain on rejection; Available alone denotes acceptance. */
  const LpPrimalEvidence& primal_data() const noexcept;
  const LpFarkasEvidence& farkas_data() const noexcept;
  const std::vector<LpEvidenceStage>& stages() const noexcept;
  /** Check original owner/index/active status; no solve is triggered. */
  double base_value(Variable) const;
  double direction_value(Variable) const;
  const LpFarkasEntry& row_multiplier(Constraint) const;
  const LpFarkasEntry& column_multiplier(Variable) const;
private:
  friend struct Detail::LpEvidenceAccess;
  LpEvidence() = default;
  ModelSnapshot source_;
  LpEvidenceTolerances tolerances_;
  double primal_tolerance_ = 0;
  LpEvidenceGroup primal_, farkas_;
  LpPrimalEvidence primal_data_;
  LpFarkasEvidence farkas_data_;
  std::vector<LpEvidenceStage> stages_;
};
struct LpEvidenceResult {
  ModelId model_id = 0;
  Revision revision = 0;
  LpEvidenceCompletion completion = LpEvidenceCompletion::Rejected;
  std::optional<Termination> stop_reason;
  std::string message;
  std::shared_ptr<const LpEvidence> evidence;
  /** Public auxiliary solve invocations, same unit as max_auxiliary_solves. */
  std::size_t attempted_calls = 0;
  std::size_t work = 0;
  double elapsed_seconds = 0.0;
};
/**
 * Fresh private auxiliary solves; no input model, session or historical result
 * is changed. Numerical continuous LPs only, Auto/HiGHS. Unsupported source is
 * never relaxed. Explicit Farkas needs one call; Automatic at most two; Both at
 * most three. All share one time/cancel allowance and deterministic work caps.
 * Complete describes requested analysis, not source-model optimality/status.
 * A checked source base point is required with a primal direction. A feasible
 * point plus accepted contradiction rejects both as inconsistent evidence.
 * Cleanup/factorization is cooperative; a final expired budget clears evidence
 * availability, retaining diagnostics and private-stage provenance only.
 */
LpEvidenceResult analyze_lp_evidence(const ModelSnapshot&, const LpEvidenceOptions& = {});
LpEvidenceResult analyze_lp_evidence(const Model&, const LpEvidenceOptions& = {});
}}
#endif
