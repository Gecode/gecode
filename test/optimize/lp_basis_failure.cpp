// Link against a separately compiled solve.cpp with
// GECODE_OPTIMIZE_TEST_LP_BASIS_FAILURE=1, and real HiGHS. The seam injects
// rejection/cancellation only AFTER real setBasis has modified backend state.
#include <gecode/optimize/lp_basis.hpp>
#include <gecode/optimize/session.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
namespace O=Gecode::Optimize;
using B=O::LpBasisStatus;
using S=O::LpBasisSubmissionState;
static bool inject_cancellation=false;
namespace Gecode { namespace Optimize { namespace Detail {
bool lp_basis_test_cancel() noexcept { return inject_cancellation; }
}}}
static void value(const O::SolveResult& result,double objective){
  assert(result.termination==O::Termination::Optimal&&result.has_solution());
  assert(std::abs(*result.objective-objective)<1e-7);
}
int main(){
  assert(O::lp_observation_capabilities().available); // Required real numerical backend.
  const double inf=std::numeric_limits<double>::infinity();
  O::Model model;auto x=model.add_continuous(),y=model.add_continuous();
  model.add_row({{x,1},{y,1}},4,inf);model.minimize({{x,2},{y,3}},7);
  O::LpBasisData data;data.source=model.snapshot();data.columns={B::Basic,B::Lower};data.rows={B::Lower};
  O::LpBasisSolveOptions options;options.basis=O::make_lp_basis(data);
  O::SolveSession session;
  auto historical=session.solve_lp_observed(model);value(historical.result,15);
  const auto initial_loads=session.statistics().model_loads;
  auto rejected=session.solve_lp_with_basis(model,options);
  assert(rejected.submission.backend_attempted&&rejected.submission.state==S::Rejected);
  assert(!rejected.submission.statuses_changed&&!rejected.observed.result.has_solution());
  assert(rejected.observed.result.termination==O::Termination::BackendError);
  assert(rejected.observed.observations->basis().state!=O::LpObservationState::Available);
  // No fallback solve occurred. The next call must load a clean backend.
  assert(session.statistics().model_loads==initial_loads);
  value(session.solve(model),15);assert(session.statistics().model_loads==initial_loads+1);
  assert(historical.observations->checks().accepted&&historical.observations->rows()[0].dual==2);
  // Cancellation after actual factor/repair has completed also clears callbacks
  // and disposes the backend before the caller can reuse the session.
  options.observations.solve.cancellation=std::make_shared<O::CancellationToken>();
  inject_cancellation=true;
  const auto before=session.statistics().model_loads;
  auto cancelled=session.solve_lp_with_basis(model,options);
  assert(cancelled.submission.backend_attempted&&cancelled.submission.state==S::Interrupted);
  assert(!cancelled.submission.statuses_changed);
  assert(cancelled.observed.result.termination==O::Termination::Cancelled);
  assert(cancelled.observed.observations->dual_point().reason==O::LpObservationReason::Interrupted);
  value(session.solve(model),15);assert(session.statistics().model_loads==before+1);
  // The zero-row safe submission path has the same cleanup contract.
  O::Model zero;auto z=zero.add_continuous(0,5);zero.minimize({{z,-1}},2);
  O::LpBasisData zd;zd.source=zero.snapshot();zd.columns={B::Upper};
  options={};options.basis=O::make_lp_basis(zd);
  inject_cancellation=false;
  auto zero_failure=session.solve_lp_with_basis(zero,options);
  assert(zero_failure.submission.backend_attempted&&zero_failure.submission.state==S::Rejected);
  assert(zero_failure.observed.result.termination==O::Termination::BackendError);
  const auto zero_loads=session.statistics().model_loads;
  value(session.solve(zero),-3);assert(session.statistics().model_loads==zero_loads+1);
  std::cout<<"LP basis post-real-submission rejection/cancellation cleanup passed\n";
}
