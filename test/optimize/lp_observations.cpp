#include <gecode/optimize/lp_observations.hpp>
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

namespace O=Gecode::Optimize;
using State=O::LpObservationState;
using Reason=O::LpObservationReason;
constexpr double inf=std::numeric_limits<double>::infinity();
static void near(double a,double b){assert(std::isfinite(a)&&std::abs(a-b)<1e-6);}
static void optimal(const O::LpObservedResult& out,double objective) {
  if(out.result.termination!=O::Termination::Optimal)
    std::cerr<<"unexpected solve status: "<<out.result.message<<'\n';
  assert(out.result.termination==O::Termination::Optimal&&out.result.has_solution());
  near(*out.result.objective,objective);assert(out.observations);
  if(out.observations->dual_point().state!=State::Available)
    std::cerr<<"unexpected dual status: "<<out.observations->dual_point().message<<'\n';
  assert(out.observations->primal_rows().state==State::Available);
  assert(out.observations->dual_point().state==State::Available&&out.observations->checks().accepted);
  near(*out.observations->checks().dual_objective_estimate,objective);
  near(*out.observations->checks().normalized_gap,0);
}
static void no_duals(const O::LpObservedResult& out) {
  assert(out.observations);
  assert(out.observations->dual_point().state!=State::Available);
  assert(out.observations->basis().state!=State::Available);
  for(const auto& c:out.observations->columns())assert(!c.reduced_cost&&!c.basis);
  for(const auto& r:out.observations->rows())assert(!r.dual&&!r.basis);
}
template<class F>static void model_error(F&& fn){bool threw=false;try{fn();}catch(const O::ModelError&){threw=true;}assert(threw);}
int main(){
  O::Model model;auto x=model.add_continuous(),y=model.add_continuous();
  auto demand=model.add_row({{x,1},{y,1}},4,inf);model.minimize({{x,2},{y,3}},7);
  const auto cap=O::lp_observation_capabilities();
  assert(cap.available==O::capabilities(O::Backend::Highs).available);
  for(double bad:{-1.0,inf,std::numeric_limits<double>::quiet_NaN()}) {
    O::LpObservationOptions options;options.checks.complementarity=bad;
    auto out=O::solve_lp_observed(model,options);assert(out.result.termination==O::Termination::InvalidModel&&!out.observations);
  }
  O::LpObservationOptions bad;bad.solve.backend=static_cast<O::Backend>(99);
  assert(O::solve_lp_observed(model,bad).result.termination==O::Termination::InvalidModel);
  O::SolveSession session;
  for(auto guarantee:{O::Guarantee::Exact,O::Guarantee::Certified}){
    O::LpObservationOptions options;options.solve.guarantee=guarantee;
    auto out=session.solve_lp_observed(model,options);assert(out.result.termination==O::Termination::Unsupported);
    assert(out.result.guarantee==guarantee);no_duals(out);assert(session.statistics().solve_calls==0);
  }
  O::LpObservationOptions native;native.solve.backend=O::Backend::Native;
  auto unsupported=session.solve_lp_observed(model,native);
  assert(unsupported.result.termination==O::Termination::Unsupported);no_duals(unsupported);
  for(auto type:{O::VariableType::Integer,O::VariableType::Binary,O::VariableType::SemiContinuous,O::VariableType::SemiInteger}){
    O::Model typed;auto v=typed.add_variable(type,type==O::VariableType::Binary?0:1,1);typed.minimize({{v,1}});
    auto out=session.solve_lp_observed(typed);assert(out.result.termination==O::Termination::Unsupported);no_duals(out);
    assert(out.observations->source().variables[0].type==type);
  }
  // An active empty native constraint must be rejected even without integer columns.
  O::Model global;global.add_global(O::AllDifferentData{});
  auto global_out=session.solve_lp_observed(global);
  assert(global_out.result.termination==O::Termination::Unsupported);no_duals(global_out);
  assert(session.statistics().solve_calls==0);
  if(!cap.available){
    auto out=session.solve_lp_observed(model);assert(out.result.termination==O::Termination::Unsupported);
    no_duals(out);assert(out.observations->source().model_id==model.id());
    assert(out.observations->rows().size()==1&&out.observations->columns().size()==2);
    std::cout<<"LP observations explicit missing-backend/admission contract passed\n";return 0;
  }
  assert(cap.duals&&cap.basis_export&&!cap.backend_version.empty());
  auto first=session.solve_lp_observed(model);optimal(first,15);
  assert(first.observations->basis().state==State::Available);
  near(first.result.value(x),4);near(first.result.value(y),0);
  near(*first.observations->row(demand).dual,2);
  near(*first.observations->column(x).reduced_cost,0);near(*first.observations->column(y).reduced_cost,1);
  near(*first.observations->row(demand).activity,4);near(*first.observations->row(demand).lower_slack,0);
  assert(!first.observations->row(demand).upper_slack);
  assert(first.observations->metadata().backend_primal_tolerance&&first.observations->metadata().backend_dual_tolerance);
  const auto original_revision=model.revision();
  model.minimize({{x,2},{y,1}},7);
  auto second=session.solve_lp_observed(model);optimal(second,11);near(*second.observations->row(demand).dual,1);
  near(*second.observations->column(x).reduced_cost,1);
  assert(first.observations->revision()==original_revision&&first.observations->source().objective.terms[1].coefficient==3);
  near(*first.observations->row(demand).dual,2);
  // Same revision is not sufficient for cache compatibility or historical content identity.
  auto snapshot=model.snapshot();snapshot.objective.terms[1].coefficient=5;
  auto same_revision=session.solve_lp_observed(snapshot);optimal(same_revision,15);
  assert(same_revision.observations->revision()==second.observations->revision());
  assert(same_revision.observations->source().objective.terms[1].coefficient==5);
  snapshot.objective.terms[1].coefficient=100;
  assert(same_revision.observations->source().objective.terms[1].coefficient==5);
  O::LpObservationOptions zero;zero.solve.time_limit_seconds=0;
  auto stopped=session.solve_lp_observed(model,zero);assert(stopped.result.termination==O::Termination::TimeLimit);no_duals(stopped);
  assert(stopped.observations->primal_rows().reason==Reason::Interrupted);
  auto ordinary=session.solve(model);assert(ordinary.termination==O::Termination::Optimal);near(*ordinary.objective,11);
  optimal(session.solve_lp_observed(model),11);
  O::LpObservationOptions cancelled;cancelled.solve.cancellation=std::make_shared<O::CancellationToken>();
  cancelled.solve.cancellation->cancel();auto stop=session.solve_lp_observed(model,cancelled);
  assert(stop.result.termination==O::Termination::Cancelled);no_duals(stop);
  model.set_bounds(demand,8,inf);model.set_bounds(y,0,3);
  optimal(session.solve_lp_observed(model),20);
  model.maximize({{x,-2},{y,-1}},-7);auto edited=session.solve_lp_observed(model);optimal(edited,-20);
  auto cold=O::solve_lp_observed(model);optimal(cold,-20);
  near(*edited.observations->row(demand).dual,*cold.observations->row(demand).dual);
  session.reset();near(*first.observations->row(demand).dual,2);
  O::SolveSession moved(std::move(session));auto invalid=session.solve_lp_observed(model);
  assert(invalid.result.termination==O::Termination::InvalidModel);no_duals(invalid);
  optimal(moved.solve_lp_observed(model),-20);
  // Original min/max dual conventions, free columns and range sides.
  for(int scenario=0;scenario<4;++scenario){
    O::Model m;auto v=m.add_continuous(-inf,inf);
    auto r=m.add_row({{v,1}},scenario<2?-inf:1,scenario<2?4:3);
    if(scenario==0)m.maximize({{v,2}},7);
    else if(scenario==1)m.minimize({{v,-2}},7);
    else m.minimize({{v,scenario==2?1.0:-1.0}});
    const double objective[]={15,-1,1,-3},dual[]={2,-2,1,-1};
    auto out=O::solve_lp_observed(m);optimal(out,objective[scenario]);near(*out.observations->row(r).dual,dual[scenario]);
    assert(out.observations->basis().state==State::Available);
  }
  // Constant rows are independently evaluated; backend rows have their own map.
  std::shared_ptr<const O::LpObservations> retained;
  O::Variable historical;O::Constraint historical_row;
  {
    O::Model m;auto tomb=m.add_continuous();m.remove(tomb);
    auto a=m.add_continuous(0,inf,"duplicate"),b=m.add_continuous(0,inf,"duplicate");historical=a;
    auto gone=m.add_row({},-1,1);m.remove(gone);
    auto r=m.add_row({{a,1}},2,inf);historical_row=r;
    auto constant=m.add_row({},-1,1);auto s=m.add_row({{b,1}},3,inf);
    m.minimize({{a,2},{b,3}},1);auto out=O::solve_lp_observed(m);optimal(out,14);
    retained=out.observations;
    assert(!retained->columns()[tomb.id].active&&!retained->rows()[gone.id].active);
    near(*retained->row(r).dual,2);near(*retained->row(s).dual,3);
    near(*retained->row(constant).activity,0);near(*retained->row(constant).lower_slack,1);
    near(*retained->row(constant).upper_slack,1);near(*retained->row(constant).dual,0);
    assert(retained->row(constant).dual_source==O::LpDualSource::DerivedConstantRow);
    assert(retained->basis().reason==Reason::ElidedConstantRows);
    model_error([&]{retained->column(tomb);});model_error([&]{retained->row(gone);});
    model_error([&]{retained->column(x);});model_error([&]{retained->row(demand);});
    model_error([&]{retained->column({m.id(),999});});
  }
  near(*retained->column(historical).reduced_cost,0);near(*retained->row(historical_row).dual,2);
  O::Model empty;empty.minimize({},7);auto empty_result=O::solve_lp_observed(empty);
  assert(empty_result.result.termination==O::Termination::Optimal);near(*empty_result.result.objective,7);
  assert(empty_result.observations->primal_rows().state==State::Available&&empty_result.observations->rows().empty());
  assert(empty_result.observations->dual_point().reason==Reason::NoBackendSolve);
  O::LpObservationOptions primal_only;primal_only.duals=primal_only.basis=false;
  auto p=O::solve_lp_observed(model,primal_only);
  assert(p.result.has_solution()&&p.observations->primal_rows().state==State::Available);
  assert(p.observations->dual_point().state==State::NotRequested&&p.observations->basis().state==State::NotRequested);
  O::Model infeasible;auto iv=infeasible.add_continuous(0,1);infeasible.add_row({{iv,1}},2,inf);
  auto infeas=O::solve_lp_observed(infeasible);assert(infeas.result.termination==O::Termination::Infeasible);no_duals(infeas);
  O::Model unbounded;auto uv=unbounded.add_continuous();unbounded.minimize({{uv,-1}});
  auto unbound=O::solve_lp_observed(unbounded);assert(unbound.result.termination==O::Termination::Unbounded);no_duals(unbound);
  auto malformed=model.snapshot();malformed.rows[0].terms[0].variable.model_id=0;
  auto malformed_result=O::solve_lp_observed(malformed);assert(malformed_result.result.termination==O::Termination::InvalidModel&&!malformed_result.observations);
  std::cout<<"LP observations real-backend analytic, mapping, ownership and session checks passed\n";
}
