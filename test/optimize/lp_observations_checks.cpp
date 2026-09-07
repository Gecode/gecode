#include <gecode/optimize/lp_observations_detail.hpp>
#include <gecode/optimize/validate.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

namespace O=Gecode::Optimize;
namespace D=Gecode::Optimize::Detail;
using State=O::LpObservationState;
using Reason=O::LpObservationReason;
using Basis=O::LpBasisStatus;
constexpr double inf=std::numeric_limits<double>::infinity();
static void near(double a,double b) { assert(std::isfinite(a)&&std::abs(a-b)<1e-8); }
struct Fixture {
  O::ModelSnapshot model;
  O::SolveResult result;
  D::LpBackendObservations raw;
  O::LpObservationOptions options;
};
static Fixture fixture() {
  O::Model model;
  auto x=model.add_continuous(0,10),y=model.add_continuous(0,10);
  model.add_row({{x,1},{y,1}},4,inf);
  model.minimize({{x,2},{y,3}},7);
  Fixture f;f.model=model.snapshot();
  f.result.model_id=model.id();f.result.revision=model.revision();
  f.result.backend="test-only owned data";f.result.backend_version="1";
  f.result.termination=O::Termination::Optimal;f.result.values={4,0};
  f.result.active_variables={true,true};f.result.objective=15;f.result.solution_validated=true;
  f.raw.attempted=f.raw.timely=f.raw.complete=true;
  f.raw.info_valid=f.raw.value_valid=f.raw.primal_feasible=true;
  f.raw.dual_valid=f.raw.dual_feasible=f.raw.basis_valid=f.raw.info_basis_valid=true;
  f.raw.model_id=model.id();f.raw.revision=model.revision();
  f.raw.column_slots={0,1};f.raw.row_slots={0};
  f.raw.column_duals={0,1};f.raw.row_duals={2};
  f.raw.column_basis={Basis::Basic,Basis::Lower};f.raw.row_basis={Basis::Lower};
  f.raw.primal_tolerance=1e-7;f.raw.dual_tolerance=1e-7;
  return f;
}
static std::shared_ptr<O::LpObservations> inspect(const Fixture& f) {
  auto out=D::LpObservationAccess::create(f.model,f.options);
  O::SolveBudget budget(f.options.solve);
  D::LpObservationAccess::finish(*out,f.result,f.raw,budget);
  return out;
}
static void rejected_dual(const Fixture& f,Reason reason=Reason::InvalidBackendData) {
  auto out=inspect(f);
  assert(out->primal_rows().state==State::Available);
  assert(out->dual_point().state==State::Rejected&&out->dual_point().reason==reason);
  for(const auto& c:out->columns()) assert(!c.reduced_cost);
  for(const auto& r:out->rows()) assert(!r.dual&&r.dual_source==O::LpDualSource::None);
}
int main() {
  auto f=fixture();auto good=inspect(f);
  assert(good->primal_rows().state==State::Available);
  assert(good->dual_point().state==State::Available&&good->basis().state==State::Available);
  assert(good->checks().accepted);near(*good->checks().normalized_gap,0);
  near(*good->checks().dual_objective_estimate,15);
  near(*good->rows()[0].activity,4);near(*good->rows()[0].lower_slack,0);
  near(*good->columns()[1].reduced_cost,1);
  // Each validity flag is independently necessary; unavailable is not a zero dual.
  for(int flag=0;flag<7;++flag) {
    f=fixture();
    bool* flags[]={&f.raw.info_valid,&f.raw.value_valid,&f.raw.primal_feasible,
      &f.raw.dual_valid,&f.raw.dual_feasible,&f.raw.basis_valid,&f.raw.info_basis_valid};
    *flags[flag]=false;auto out=inspect(f);
    if(flag<3) {assert(out->dual_point().reason==Reason::NoPrimalPoint);assert(out->basis().reason==Reason::NoPrimalPoint);}
    else if(flag<5) {assert(out->dual_point().reason==Reason::NoDualPoint);assert(out->basis().state==State::Available);}
    else {assert(out->dual_point().state==State::Available);assert(out->basis().reason==Reason::NoBasis);}
  }
  for(double bad:{inf,-inf,std::numeric_limits<double>::quiet_NaN()}) {
    f=fixture();f.raw.row_duals[0]=bad;rejected_dual(f);
    f=fixture();f.raw.column_duals[1]=bad;rejected_dual(f);
  }
  for(int mode=0;mode<4;++mode) {
    f=fixture();
    if(mode==0)f.raw.row_duals.clear();
    if(mode==1)f.raw.row_duals.push_back(0);
    if(mode==2)f.raw.column_duals.pop_back();
    if(mode==3)f.raw.column_duals.push_back(0);
    rejected_dual(f);assert(inspect(f)->basis().state==State::Available);
  }
  for(int mode=0;mode<8;++mode) {
    f=fixture();
    if(mode==0)std::swap(f.raw.column_slots[0],f.raw.column_slots[1]);
    if(mode==1)f.raw.row_slots.clear();
    if(mode==2)f.raw.column_slots.push_back(2);
    if(mode==3)++f.raw.model_id;
    if(mode==4)++f.raw.revision;
    if(mode==5)f.raw.complete=false;
    if(mode==6)f.raw.primal_tolerance=-1;
    if(mode==7)f.raw.dual_tolerance=std::numeric_limits<double>::quiet_NaN();
    rejected_dual(f);assert(inspect(f)->basis().state==State::Rejected);
  }
  for(int mode=0;mode<7;++mode) {
    f=fixture();
    if(mode==0)++f.result.model_id;
    if(mode==1)++f.result.revision;
    if(mode==2)f.result.active_variables.pop_back();
    if(mode==3)f.result.active_variables[1]=false;
    if(mode==4)f.result.objective=14;
    if(mode==5)f.result.values[0]=3;
    if(mode==6)f.result.values[0]=11;
    auto out=inspect(f);assert(out->primal_rows().state==State::Rejected);
    assert(!out->rows()[0].activity&&out->dual_point().state==State::Rejected);
  }
  for(int mode=0;mode<5;++mode) {
    f=fixture();
    if(mode==0)f.raw.column_basis[0]=Basis::Lower;
    if(mode==1)f.raw.column_basis[0]=static_cast<Basis>(99);
    if(mode==2)f.raw.row_basis[0]=Basis::Basic;
    if(mode==3)f.raw.column_basis[1]=Basis::Zero;
    if(mode==4)f.raw.row_basis.clear();
    auto out=inspect(f);assert(out->basis().state==State::Rejected);
    assert(out->dual_point().state==State::Available);
    assert(!out->columns()[0].basis&&!out->rows()[0].basis);
  }
  f=fixture();f.raw.column_duals[1]=0;rejected_dual(f,Reason::FailedChecks);
  auto out=inspect(f);assert(!out->checks().stationarity_valid);near(*out->checks().normalized_gap,0);
  f=fixture();f.raw.row_duals[0]=1;f.raw.column_duals={1,2};
  out=inspect(f);assert(out->checks().stationarity_valid&&!out->checks().dual_signs_valid);
  assert(!out->checks().complementarity_valid&&!out->checks().gap_valid);
  f=fixture();f.raw.row_duals[0]=3;f.raw.column_duals={0,0};
  out=inspect(f);near(*out->checks().normalized_gap,-4);assert(!out->checks().accepted);
  // A tiny multiplier still requires a finite endpoint; never replace inf*epsilon by zero.
  f=fixture();f.model.variables[0].upper=inf;f.raw.column_duals[0]=-1e-12;
  out=inspect(f);assert(!out->checks().dual_objective_estimate&&!out->checks().normalized_gap);
  assert(out->dual_point().state==State::Rejected);
  // Large common offsets cannot hide a nonzero normalized primal/dual gap.
  f=fixture();f.model.objective.offset=1e16;f.result.objective=1e16+8;
  f.raw.row_duals[0]=2.0001;f.raw.column_duals={-0.0001,0.9999};
  out=inspect(f);assert(out->checks().stationarity_valid&&!out->checks().gap_valid);
  assert(*out->checks().normalized_gap>0.0005);
  assert(*out->checks().dual_objective_estimate==*f.result.objective);
  // A^T*pi = 1e16 + 1 - 1e16 must retain the unit residual.
  {
    O::Model model;auto x=model.add_continuous(1,1);
    model.add_row({{x,1e16}},1e16,1e16);model.add_row({{x,1}},1,1);
    model.add_row({{x,-1e16}},-1e16,-1e16);model.minimize({{x,1}});
    f=fixture();f.model=model.snapshot();f.result.model_id=model.id();f.result.revision=model.revision();
    f.result.values={1};f.result.active_variables={true};f.result.objective=1;
    f.raw.model_id=model.id();f.raw.revision=model.revision();f.raw.column_slots={0};f.raw.row_slots={0,1,2};
    f.raw.column_duals={0};f.raw.row_duals={1,1,1};f.raw.basis_valid=false;
    out=inspect(f);assert(out->checks().accepted);near(*out->checks().max_stationarity,0);
  }
  // Retain a row's residual for slack, normal-cone, and basis checks before narrowing.
  {
    O::Model model;auto x=model.add_continuous(1,1),y=model.add_continuous(1,1);
    model.add_row({{x,1e16},{y,1}},1e16,inf);model.minimize({});
    f=fixture();f.model=model.snapshot();f.result.model_id=model.id();f.result.revision=model.revision();
    f.result.values={1,1};f.result.objective=0;
    f.raw.model_id=model.id();f.raw.revision=model.revision();f.raw.row_duals={0};f.raw.column_duals={0,0};
    f.raw.column_basis={Basis::Lower,Basis::Lower};f.raw.row_basis={Basis::Basic};
    out=inspect(f);assert(out->checks().accepted&&out->basis().state==State::Available);
    near(*out->rows()[0].lower_slack,1);
    f.raw.row_basis={Basis::Lower};f.raw.column_basis[0]=Basis::Basic;
    assert(inspect(f)->basis().state==State::Rejected);
    f.raw.row_duals={1};f.raw.column_duals={-1e16,-1};
    out=inspect(f);assert(!out->checks().dual_signs_valid);near(*out->checks().max_complementarity,1);
  }
  f=fixture();f.result.termination=O::Termination::TimeLimit;
  out=inspect(f);assert(out->primal_rows().state==State::Available&&out->dual_point().reason==Reason::NotOptimal);
  f=fixture();f.raw.timely=false;out=inspect(f);assert(out->dual_point().reason==Reason::Interrupted);
  f=fixture();f.raw.attempted=false;out=inspect(f);assert(out->dual_point().reason==Reason::NoBackendSolve);
  f=fixture();f.raw.failure=D::LpCaptureFailure::Allocation;
  rejected_dual(f,Reason::AllocationFailure);
  f=fixture();f.options.duals=f.options.basis=false;out=inspect(f);
  assert(out->primal_rows().state==State::Available&&out->dual_point().state==State::NotRequested&&out->basis().state==State::NotRequested);
  for(double bad:{-1.0,inf,std::numeric_limits<double>::quiet_NaN()}) {
    f=fixture();f.options.checks.stationarity=bad;bool threw=false;
    try{inspect(f);}catch(const O::ModelError&){threw=true;}assert(threw);
  }
  // A deterministic post-collection cancellation models cleanup crossing the final gate.
  // Production uses this same gate after raw vectors have been destroyed; no sleeps/hooks.
  f=fixture();f.options.solve.cancellation=std::make_shared<O::CancellationToken>();
  O::SolveBudget budget(f.options.solve);auto mutable_observation=D::LpObservationAccess::create(f.model,f.options);
  {auto raw=f.raw;D::LpObservationAccess::finish(*mutable_observation,f.result,raw,budget);}
  assert(mutable_observation->dual_point().state==State::Available);
  O::LpObservedResult observed{f.result,mutable_observation};
  f.options.solve.cancellation->cancel();
  D::LpObservationAccess::final_budget(observed,mutable_observation,budget);
  assert(observed.result.termination==O::Termination::Cancelled&&observed.result.has_solution());
  assert(observed.observations->primal_rows().reason==Reason::Interrupted);
  assert(observed.observations->dual_point().reason==Reason::Interrupted);
  assert(!observed.observations->checks().accepted&&!observed.observations->rows()[0].activity);
  f=fixture();f.options.solve.time_limit_seconds=0;out=inspect(f);
  assert(out->primal_rows().reason==Reason::Interrupted&&!out->rows()[0].activity);
  std::cout<<"LP observation independent analytic/corruption checks passed\n";
}
