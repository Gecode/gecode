/* Owning continuous-LP basis starts; no checkpoint or certificate semantics. */
#ifndef GECODE_OPTIMIZE_LP_BASIS_HPP
#define GECODE_OPTIMIZE_LP_BASIS_HPP
#include <gecode/optimize/lp_observations.hpp>

namespace Gecode { namespace Optimize {
namespace Detail { struct LpBasisAccess; }

/** Untrusted caller data. Factory copies, validates, and freezes all records. */
struct LpBasisData {
  ModelSnapshot source;
  /** Exactly one entry per original slot; inactive slots must be absent. */
  std::vector<std::optional<LpBasisStatus>> rows, columns;
};
enum class LpBasisOrigin { Caller, Observations };
class LpBasis {
public:
  LpBasis(const LpBasis&) = delete;
  LpBasis& operator=(const LpBasis&) = delete;
  ModelId id() const noexcept { return data_.source.model_id; }
  Revision revision() const noexcept { return data_.source.revision; }
  const ModelSnapshot& source() const noexcept { return data_.source; }
  const std::vector<std::optional<LpBasisStatus>>& rows() const noexcept { return data_.rows; }
  const std::vector<std::optional<LpBasisStatus>>& columns() const noexcept { return data_.columns; }
  LpBasisOrigin origin() const noexcept { return origin_; }
private:
  LpBasis() = default;
  friend struct Detail::LpBasisAccess;
  LpBasisData data_;
  LpBasisOrigin origin_ = LpBasisOrigin::Caller;
};
/** Throw ModelError for malformed/unsupported source or status data. */
std::shared_ptr<const LpBasis> make_lp_basis(const LpBasisData&);
/** Requires an Available complete original basis; copies historical source. */
std::shared_ptr<const LpBasis> make_lp_basis(const LpObservations&);

enum class LpBasisSubmissionState {
  NotAttempted, Accepted, Repaired, Rejected, Interrupted
};
struct LpBasisSubmission {
  LpBasisSubmissionState state = LpBasisSubmissionState::NotAttempted;
  bool backend_attempted = false;
  /** Absent until timely accepted backend statuses were completely checked. */
  std::optional<bool> statuses_changed;
  std::string message;
};
struct LpBasisSolveOptions {
  LpObservationOptions observations;
  /** Required. Same owner/revision, original IDs/masks and active source content;
   * inactive payloads are nonsemantic. Never a relaxation. */
  std::shared_ptr<const LpBasis> basis;
  /** Simultaneous primal_start is rejected; no implicit precedence rule. */
  void validate() const;
};
struct LpBasisSolveResult {
  LpObservedResult observed;
  /** Retains submitted source/status lifetime independently of caller/session. */
  std::shared_ptr<const LpBasis> requested_basis;
  LpBasisSubmission submission;
};
/**
 * Numerical HiGHS ordinary continuous LP only; active constant rows unsupported.
 * Accepted/Repaired describe setBasis before optimization, not final status.
 * Repaired means statuses changed during submission. No faster-solve, checkpoint,
 * independent nonsingularity, or certificate claim. A rejected backend submission
 * returns without cold fallback, invalidating the session for a clean next load.
 * Factorization/repair is cooperatively budgeted and may be uninterruptible.
 */
LpBasisSolveResult solve_lp_with_basis(const ModelSnapshot&, const LpBasisSolveOptions&);
LpBasisSolveResult solve_lp_with_basis(const Model&, const LpBasisSolveOptions&);
}}
#endif
