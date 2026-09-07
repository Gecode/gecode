#include <gecode/optimize/lp_basis.hpp>
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/validate.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>
namespace O=Gecode::Optimize;
using B=O::LpBasisStatus;
using S=O::LpBasisSubmissionState;
constexpr double inf=std::numeric_limits<double>::infinity();
static void near(double a,double b){assert(std::isfinite(a)&&std::abs(a-b)<1e-7);}
template<class F>static void invalid(F fn){bool caught=false;try{fn();}catch(const O::ModelError&){caught=true;}assert(caught);}
static void optimal(const O::LpBasisSolveResult& out,double objective){
  if(out.observed.result.termination!=O::Termination::Optimal)
    std::cerr<<out.observed.result.message<<"; "<<out.submission.message<<'\n';
  assert(out.observed.result.termination==O::Termination::Optimal&&out.observed.result.has_solution());
  near(*out.observed.result.objective,objective);
  assert(out.submission.backend_attempted&&out.submission.statuses_changed);
  assert(out.submission.state==S::Accepted||out.submission.state==S::Repaired);
  assert(*out.submission.statuses_changed==(out.submission.state==S::Repaired));
  assert(out.observed.observations&&out.observed.observations->checks().accepted);
}
int main(){
  O::Model model;auto x=model.add_continuous(),y=model.add_continuous();
  auto row=model.add_row({{x,1},{y,1}},4,inf);model.minimize({{x,2},{y,3}},7);
  O::LpBasisData data;data.source=model.snapshot();data.columns={B::Basic,B::Lower};data.rows={B::Lower};
  auto basis=O::make_lp_basis(data);assert(basis->origin()==O::LpBasisOrigin::Caller);
  assert(basis->id()==model.id()&&basis->revision()==model.revision());
  data.source.objective.offset=100;data.columns[0]=B::Upper;
  assert(basis->source().objective.offset==7&&*basis->columns()[0]==B::Basic);
  for(int mode=0;mode<9;++mode){
    O::LpBasisData bad;bad.source=basis->source();bad.rows=basis->rows();bad.columns=basis->columns();
    if(mode==0)bad.rows.clear();
    if(mode==1)bad.columns.push_back(B::Lower);
    if(mode==2)bad.columns[0].reset();
    if(mode==3)bad.columns[0]=B::Lower;
    if(mode==4)bad.columns[1]=B::Basic;
    if(mode==5)bad.columns[1]=static_cast<B>(99);
    if(mode==6)bad.columns[1]=B::Upper;
    if(mode==7)bad.columns[1]=B::Zero;
    if(mode==8)bad.source.rows[0].terms[0].variable.model_id=0;
    invalid([&]{O::make_lp_basis(bad);});
  }
  O::Model constant;constant.add_row({},-1,1);O::LpBasisData cd;cd.source=constant.snapshot();cd.rows={B::Basic};
  invalid([&]{O::make_lp_basis(cd);});
  O::Model integer;integer.add_integer(0,1);O::LpBasisData id;id.source=integer.snapshot();id.columns={B::Lower};
  invalid([&]{O::make_lp_basis(id);});
  O::LpBasisSolveOptions options;options.basis=basis;
  O::SolveSession session;
  auto missing=session.solve_lp_with_basis(model,{});assert(missing.observed.result.termination==O::Termination::InvalidModel);
  assert(missing.submission.state==S::NotAttempted&&!missing.submission.backend_attempted);
  auto starts=options;starts.observations.solve.primal_start={{x,4},{y,0}};
  assert(session.solve_lp_with_basis(model,starts).observed.result.termination==O::Termination::InvalidModel);
  assert(session.statistics().solve_calls==0);
  // Every compatible-content gate is checked even if an untrusted snapshot reuses revision.
  for(int mode=0;mode<9;++mode){
    auto changed=model.snapshot();
    if(mode==0)++changed.revision;
    if(mode==1)changed.variables[0].upper=10;
    if(mode==2)changed.variables[0].name="new label";
    if(mode==3)changed.rows[0].lower=3;
    if(mode==4)changed.rows[0].name="new row label";
    if(mode==5)changed.rows[0].terms[0].coefficient=2;
    if(mode==6)changed.objective.offset=8;
    if(mode==7)changed.objective.sense=O::ObjectiveSense::Maximize;
    if(mode==8)changed.objective.terms[0].coefficient=1;
    auto out=session.solve_lp_with_basis(changed,options);
    assert(out.observed.result.termination==O::Termination::InvalidModel&&!out.submission.backend_attempted);
    assert(out.requested_basis==basis&&session.statistics().solve_calls==0);
  }
  O::Model foreign;auto fx=foreign.add_continuous(),fy=foreign.add_continuous();
  foreign.add_row({{fx,1},{fy,1}},4,inf);foreign.minimize({{fx,2},{fy,3}},7);
  assert(session.solve_lp_with_basis(foreign,options).observed.result.termination==O::Termination::InvalidModel);
  for(auto backend:{O::Backend::Native}){
    auto unsupported=options;unsupported.observations.solve.backend=backend;
    auto out=session.solve_lp_with_basis(model,unsupported);assert(out.observed.result.termination==O::Termination::Unsupported&&!out.submission.backend_attempted);
  }
  auto exact=options;exact.observations.solve.guarantee=O::Guarantee::Exact;
  assert(session.solve_lp_with_basis(model,exact).observed.result.termination==O::Termination::Unsupported);
  if(!O::lp_observation_capabilities().available){
    auto out=session.solve_lp_with_basis(model,options);assert(out.observed.result.termination==O::Termination::Unsupported);
    assert(out.submission.state==S::NotAttempted&&!out.submission.backend_attempted&&out.requested_basis==basis);
    std::cout<<"LP basis factory, source admission and missing-backend checks passed\n";return 0;
  }
  auto first=session.solve_lp_with_basis(model,options);optimal(first,15);assert(first.submission.state==S::Accepted);
  assert(session.statistics().model_loads==1&&session.statistics().basis_warm_starts==0);
  auto exported=O::make_lp_basis(*first.observed.observations);assert(exported->origin()==O::LpBasisOrigin::Observations);
  options.basis=exported;optimal(session.solve_lp_with_basis(model,options),15);
  optimal(O::solve_lp_with_basis(model,options),15);
  // Round-trip original row statuses for max sense and ranged/free entities.
  for(int scenario=0;scenario<3;++scenario){
    O::Model ranged;auto a=ranged.add_continuous(-inf,inf),fixed=ranged.add_continuous(2,2);
    ranged.add_row({{a,1}},1,4);
    if(scenario==0)ranged.maximize({{a,2},{fixed,1}},7);
    else ranged.minimize({{a,scenario==1?1.0:-1.0},{fixed,1}},7);
    const double objective[]={17,10,5};
    auto observed=O::solve_lp_observed(ranged);
    assert(observed.observations->basis().state==O::LpObservationState::Available);
    O::LpBasisSolveOptions ro;ro.basis=O::make_lp_basis(*observed.observations);
    optimal(O::solve_lp_with_basis(ranged,ro),objective[scenario]);
  }
  // A legitimate nonoptimal all-logical basis is accepted before later pivots.
  data.source=model.snapshot();data.columns={B::Lower,B::Lower};data.rows={B::Basic};
  options.basis=O::make_lp_basis(data);auto logical=session.solve_lp_with_basis(model,options);optimal(logical,15);
  assert(logical.submission.state==S::Accepted&&!*logical.submission.statuses_changed);
  assert(logical.observed.observations->columns()[0].basis==B::Basic);
  // Count-correct singular structural basis: HiGHS repairs before optimization.
  {
    O::Model singular;auto a=singular.add_continuous(),b=singular.add_continuous();
    singular.add_row({{a,1},{b,1}},1,1);singular.add_row({{a,2},{b,2}},2,2);
    singular.minimize({{a,1},{b,2}});
    O::LpBasisData sd;sd.source=singular.snapshot();sd.columns={B::Basic,B::Basic};sd.rows={B::Lower,B::Lower};
    O::LpBasisSolveOptions so;so.basis=O::make_lp_basis(sd);
    auto repaired=session.solve_lp_with_basis(singular,so);optimal(repaired,1);
    assert(repaired.submission.state==S::Repaired&&*repaired.submission.statuses_changed);
    near(*session.solve(singular).objective,1);
    optimal(session.solve_lp_with_basis(singular,so),1);
  }
  // Zero-row submitted statuses must never enter the pinned unsafe alien branch.
  {
    O::Model zero;auto a=zero.add_continuous(0,5),b=zero.add_continuous(-inf,inf),c=zero.add_continuous(-2,3);
    zero.minimize({{a,1},{c,-1}},5);
    O::LpBasisData zd;zd.source=zero.snapshot();zd.columns={B::Lower,B::Zero,B::Upper};
    O::LpBasisSolveOptions zo;zo.basis=O::make_lp_basis(zd);
    auto out=O::solve_lp_with_basis(zero,zo);optimal(out,2);assert(out.submission.state==S::Accepted);
    near(out.observed.result.value(b),0);
  }
  // Tombstones retain their slot but never carry a submitted status.
  {
    O::Model holes;auto removed=holes.add_continuous();holes.remove(removed);auto a=holes.add_continuous();
    auto deleted=holes.add_row({},-1,1);holes.remove(deleted);holes.add_row({{a,1}},2,inf);holes.minimize({{a,1}});
    O::LpBasisData hd;hd.source=holes.snapshot();hd.columns={std::nullopt,B::Basic};hd.rows={std::nullopt,B::Lower};
    O::LpBasisSolveOptions ho;ho.basis=O::make_lp_basis(hd);optimal(O::solve_lp_with_basis(holes,ho),2);
    hd.columns[0]=B::Lower;invalid([&]{O::make_lp_basis(hd);});
    hd.columns[0].reset();hd.rows[0]=B::Basic;invalid([&]{O::make_lp_basis(hd);});
  }
  options.basis=exported;auto zero=options;zero.observations.solve.time_limit_seconds=0;
  auto limited=session.solve_lp_with_basis(model,zero);assert(limited.observed.result.termination==O::Termination::TimeLimit&&!limited.submission.backend_attempted);
  auto cancel=options;cancel.observations.solve.cancellation=std::make_shared<O::CancellationToken>();cancel.observations.solve.cancellation->cancel();
  assert(session.solve_lp_with_basis(model,cancel).observed.result.termination==O::Termination::Cancelled);
  optimal(session.solve_lp_with_basis(model,options),15);
  auto p=options;p.observations.duals=p.observations.basis=false;auto primal=session.solve_lp_with_basis(model,p);
  assert(primal.observed.result.has_solution()&&primal.observed.observations->basis().state==O::LpObservationState::NotRequested);
  invalid([&]{O::make_lp_basis(*primal.observed.observations);});
  model.set_bounds(row,5,inf);
  assert(session.solve_lp_with_basis(model,options).observed.result.termination==O::Termination::InvalidModel);
  near(*session.solve(model).objective,17);
  session.reset();assert(first.requested_basis->source().objective.offset==7);
  assert(first.observed.observations->row(row).dual==2);
  O::SolveSession moved(std::move(session));
  assert(session.solve_lp_with_basis(model,options).observed.result.termination==O::Termination::InvalidModel);
  O::Model empty;empty.minimize({},7);O::LpBasisData ed;ed.source=empty.snapshot();
  O::LpBasisSolveOptions eo;eo.basis=O::make_lp_basis(ed);
  auto empty_result=O::solve_lp_with_basis(empty,eo);
  assert(empty_result.observed.result.termination==O::Termination::Optimal);
  near(*empty_result.observed.result.objective,7);
  assert(empty_result.requested_basis&&empty_result.submission.state==S::NotAttempted&&!empty_result.submission.backend_attempted);
  std::shared_ptr<const O::LpBasis> detached;
  {
    O::Model temporary;auto a=temporary.add_continuous();temporary.add_row({{a,1}},2,inf);
    temporary.minimize({{a,1}},1);O::LpBasisData td;td.source=temporary.snapshot();
    td.columns={B::Basic};td.rows={B::Lower};detached=O::make_lp_basis(td);
  }
  O::LpBasisSolveOptions detached_options;detached_options.basis=detached;detached.reset();
  auto retained=O::solve_lp_with_basis(detached_options.basis->source(),detached_options);optimal(retained,3);
  detached_options.basis.reset();assert(retained.requested_basis->source().objective.offset==1);
  std::cout<<"LP basis accepted/repaired, singular/zero-row, source and session conformance passed\n";
}
