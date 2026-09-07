/* Private collection/checking seam. Not installed or part of the public API. */
#ifndef GECODE_OPTIMIZE_LP_OBSERVATIONS_DETAIL_HPP
#define GECODE_OPTIMIZE_LP_OBSERVATIONS_DETAIL_HPP
#include <gecode/optimize/lp_observations.hpp>

namespace Gecode { namespace Optimize { namespace Detail {

enum class LpCaptureFailure { None, Allocation, InvalidData };
struct LpBackendObservations {
  bool requested_duals = true, requested_basis = true;
  bool attempted = false, timely = false, complete = false;
  bool info_valid = false, value_valid = false, primal_feasible = false;
  bool dual_valid = false, dual_feasible = false;
  bool basis_valid = false, info_basis_valid = false;
  LpCaptureFailure failure = LpCaptureFailure::None;
  ModelId model_id = 0;
  Revision revision = 0;
  std::vector<std::size_t> column_slots, row_slots;
  std::vector<double> column_duals, row_duals;
  std::vector<LpBasisStatus> column_basis, row_basis;
  std::optional<double> primal_tolerance, dual_tolerance;
};

struct LpObservationAccess {
  static std::shared_ptr<LpObservations> create(ModelSnapshot model,
                                               const LpObservationOptions&);
  static std::string unsupported(const ModelSnapshot&, const SolveOptions&);
  static void unavailable(LpObservations&, LpObservationReason,
                          const std::string& message);
  /** Pure original-model check of owned raw data; never calls a solver. */
  static void finish(LpObservations&, const SolveResult&,
                     const LpBackendObservations&, const SolveBudget&);
  /** Final gate, called after raw capture destruction; also independently testable. */
  static void final_budget(LpObservedResult&, std::shared_ptr<LpObservations>&,
                           const SolveBudget&) noexcept;
};

}}}
#endif
