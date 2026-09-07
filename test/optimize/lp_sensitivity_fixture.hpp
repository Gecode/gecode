/* Explicit immutable O1 data for testing without an optimization backend. */
#ifndef TEST_OPTIMIZE_LP_SENSITIVITY_FIXTURE_HPP
#define TEST_OPTIMIZE_LP_SENSITIVITY_FIXTURE_HPP
#include <gecode/optimize/lp_observations_detail.hpp>
#include <cassert>
namespace SensitivityFixture {
namespace O=Gecode::Optimize;
inline O::LpObservedResult two_column_observed(O::ModelSnapshot source) {
  O::LpObservationOptions options;
  O::LpObservedResult out;out.result.model_id=source.model_id;out.result.revision=source.revision;
  out.result.termination=O::Termination::Optimal;out.result.guarantee=O::Guarantee::Numerical;
  out.result.backend="Test explicit original O1 data";out.result.objective=10;out.result.best_bound=10;
  out.result.absolute_gap=0;out.result.relative_gap=0;out.result.values={0,3};out.result.active_variables={true,true};out.result.solution_validated=true;
  auto data=O::Detail::LpObservationAccess::create(source,options);out.observations=data;
  O::Detail::LpBackendObservations raw;raw.attempted=true;raw.timely=true;raw.complete=true;
  raw.info_valid=raw.value_valid=raw.primal_feasible=raw.dual_valid=raw.dual_feasible=raw.basis_valid=raw.info_basis_valid=true;
  raw.model_id=source.model_id;raw.revision=source.revision;raw.column_slots={0,1};raw.row_slots={0};
  raw.column_duals={1,0};raw.row_duals={1};raw.column_basis={O::LpBasisStatus::Lower,O::LpBasisStatus::Basic};raw.row_basis={O::LpBasisStatus::Lower};
  O::SolveBudget budget(options.solve);O::Detail::LpObservationAccess::finish(*data,out.result,raw,budget);
  assert(data->basis().state==O::LpObservationState::Available&&data->checks().accepted);return out;
}
inline O::LpObservedResult two_column_observed(const O::Model& model) {return two_column_observed(model.snapshot());}
}
#endif
