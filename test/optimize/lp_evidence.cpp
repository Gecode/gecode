#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/lp_evidence.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
namespace O=Gecode::Optimize;
using State=O::LpEvidenceState;using Request=O::LpEvidenceRequest;
constexpr double inf=std::numeric_limits<double>::infinity();
static void near(double a,double b){assert(std::isfinite(a)&&std::abs(a-b)<1e-7);}
template<class F>static void invalid(F fn){bool caught=false;try{fn();}catch(const O::ModelError&){caught=true;}assert(caught);}
static const O::LpEvidence& good(const O::LpEvidenceResult& r){
  if(r.completion!=O::LpEvidenceCompletion::Complete)std::cerr<<r.message<<'\n';
  assert(r.completion==O::LpEvidenceCompletion::Complete&&r.evidence&&!r.stop_reason);
  for(const auto& stage:r.evidence->stages()){
    assert(stage.auxiliary_model&&stage.columns.size()==stage.auxiliary_model->variables.size());
    if(stage.phase!=O::LpEvidencePhase::FeasibleBase){
      std::vector<double> zero(stage.columns.size(),0);
      assert(O::validate(*stage.auxiliary_model,zero,0,0).valid);
    }
    if(stage.attempted){assert(stage.auxiliary_result);
      assert(stage.auxiliary_model->model_id!=r.model_id&&stage.auxiliary_result->model_id==stage.auxiliary_model->model_id);
      assert(stage.auxiliary_result->revision==stage.auxiliary_model->revision&&stage.auxiliary_result->guarantee==O::Guarantee::Numerical);}
  }
  return *r.evidence;
}
int main(){
  O::Model basic;auto x=basic.add_continuous();basic.minimize({{x,-1}},7);
  for(auto request:{Request::Automatic,Request::PrimalRay,Request::Farkas,Request::Both}){
    O::LpEvidenceOptions options;options.request=request;auto out=O::analyze_lp_evidence(basic,options);
    if(!O::capabilities(O::Backend::Highs).available){assert(out.stop_reason==O::Termination::Unsupported&&out.attempted_calls==0&&out.evidence);continue;}
    const auto& e=good(out);assert(out.attempted_calls==(request==Request::Farkas?1:request==Request::Both?3:2));
    if(request!=Request::Farkas){assert(e.primal_ray().state==State::Available);near(e.base_value(x),0);near(e.direction_value(x),1);near(*e.primal_data().normalized_objective_slope,-1);}
    if(request!=Request::PrimalRay)assert(e.farkas().state==State::Unavailable);
  }
  O::LpEvidenceOptions options;
  for(int mode=0;mode<7;++mode){auto wrong=options;
    if(mode==0)wrong.request=static_cast<Request>(99);
    if(mode==1)wrong.checks.recession=-1;
    if(mode==2)wrong.checks.stationarity=inf;
    if(mode==3)wrong.checks.minimum_contradiction=std::numeric_limits<double>::quiet_NaN();
    if(mode==4)wrong.solve.backend=O::Backend::Native;
    if(mode==5)wrong.solve.guarantee=O::Guarantee::Exact;
    if(mode==6)wrong.solve.primal_start={{x,0}};
    auto out=O::analyze_lp_evidence(basic,wrong);assert(out.attempted_calls==0&&out.completion==O::LpEvidenceCompletion::Rejected);
  }
  auto invalid_source=basic.snapshot();invalid_source.variables[0].variable.model_id=0;
  assert(O::analyze_lp_evidence(invalid_source).stop_reason==O::Termination::InvalidModel);
  {O::Model discrete;discrete.add_integer(0,0);assert(O::analyze_lp_evidence(discrete).stop_reason==O::Termination::Unsupported);}
  {O::Model global;O::add_all_different(global,{});assert(O::analyze_lp_evidence(global).stop_reason==O::Termination::Unsupported);}
  for(int stop=0;stop<6;++stop){auto limited=options;
    if(stop==0)limited.solve.time_limit_seconds=0;
    if(stop==1){limited.solve.cancellation=std::make_shared<O::CancellationToken>();limited.solve.cancellation->cancel();}
    if(stop==2)limited.limits.max_work=0;
    if(stop==3)limited.limits.max_auxiliary_variables=0;
    if(stop==4)limited.limits.max_retained_slots=0;
    if(stop==5)limited.solve.node_limit=0;
    auto out=O::analyze_lp_evidence(basic,limited);assert(out.attempted_calls==0&&out.stop_reason);
  }
  if(!O::capabilities(O::Backend::Highs).available){std::cout<<"LP evidence core admission and missing backend passed\n";return 0;}
  for(bool maximize:{false,true})for(bool upper:{false,true}){
    O::Model m;auto v=m.add_continuous(upper?-inf:0,upper?0:inf);
    const double coefficient=(upper?1:-1)*(maximize?-1:1);
    m.set_objective({{v,coefficient}},maximize?O::ObjectiveSense::Maximize:O::ObjectiveSense::Minimize,1e16);
    auto out=O::analyze_lp_evidence(m);const auto& e=good(out);assert(e.primal_ray().state==State::Available);
    near(e.direction_value(v),upper?-1:1);near(*e.primal_data().normalized_objective_slope,-1);
  }
  {
    O::Model m;auto a=m.add_continuous(-inf,inf),b=m.add_continuous(-inf,inf),fixed=m.add_continuous(2,2);
    auto equality=m.add_row({{a,1},{b,-1}},0,0);auto ranged=m.add_row({{fixed,1}},1,3);m.minimize({{a,-1}},100);
    auto out=O::analyze_lp_evidence(m);const auto& e=good(out);near(e.direction_value(a),1);near(e.direction_value(b),1);near(e.direction_value(fixed),0);
    near(e.primal_data().row_direction[equality.id],0);near(e.primal_data().row_direction[ranged.id],0);
  }
  for(bool upper:{false,true})for(bool maximize:{false,true}){
    O::Model m;auto v=m.add_continuous(upper?0:-inf,upper?inf:0);
    auto r=m.add_row({{v,1}},upper?-inf:1,upper?-1:inf);m.set_objective({{v,1}},maximize?O::ObjectiveSense::Maximize:O::ObjectiveSense::Minimize,7);
    auto out=O::analyze_lp_evidence(m);const auto& e=good(out);assert(e.farkas().state==State::Available&&e.primal_ray().state==State::Unavailable);
    near(e.row_multiplier(r).multiplier,upper?-1:1);near(e.column_multiplier(v).multiplier,upper?1:-1);near(*e.farkas_data().contradiction_margin,1);
    assert(e.row_multiplier(r).side==(upper?O::LpEvidenceSide::Upper:O::LpEvidenceSide::Lower));
    assert(out.attempted_calls==2&&!e.stages()[1].attempted);
  }
  {
    O::Model m;auto v=m.add_continuous(-inf,inf);auto lower=m.add_row({{v,1}},1,inf);auto upper=m.add_row({{v,1}},-inf,0);
    O::LpEvidenceOptions o;o.request=Request::Farkas;auto out=O::analyze_lp_evidence(m,o);const auto& e=good(out);
    near(e.row_multiplier(lower).multiplier,1);near(e.row_multiplier(upper).multiplier,-1);near(e.column_multiplier(v).multiplier,0);
    assert(!e.column_multiplier(v).side);near(*e.farkas_data().contradiction_margin,1);
  }
  {
    O::Model m;auto r=m.add_row({},-inf,-2);auto out=O::analyze_lp_evidence(m);const auto& e=good(out);
    assert(e.farkas().state==State::Available&&e.farkas_data().columns.empty());near(e.row_multiplier(r).multiplier,-1);near(*e.farkas_data().contradiction_margin,2);
  }
  {
    O::Model m;auto v=m.add_continuous(0,0);m.add_row({{v,1}},1,1);auto out=O::analyze_lp_evidence(m);const auto& e=good(out);
    assert(e.farkas().state==State::Available);near(e.column_multiplier(v).multiplier,-1);
  }
  for(bool empty:{false,true}){
    O::Model m;if(!empty){auto v=m.add_continuous(0,1);m.minimize({{v,1}});}O::LpEvidenceOptions o;o.request=Request::Both;
    auto out=O::analyze_lp_evidence(m,o);const auto& e=good(out);assert(e.primal_ray().state==State::Unavailable&&e.farkas().state==State::Unavailable&&out.attempted_calls==3);
  }
  {
    O::Model m;auto v=m.add_continuous(0,0);auto u=m.add_continuous(0,inf);m.add_row({{v,1}},1,inf);m.minimize({{u,-1}});
    auto out=O::analyze_lp_evidence(m);const auto& e=good(out);assert(e.primal_ray().state==State::Unavailable&&e.primal_data().direction.empty()&&e.farkas().state==State::Available);
  }
  O::LpEvidenceResult historical;O::Variable live,dead;O::Constraint removed;
  {O::Model m;dead=m.add_continuous();m.remove(dead);live=m.add_continuous();removed=m.add_row({},-1,1);m.remove(removed);m.minimize({{live,-1}});
    historical=O::analyze_lp_evidence(m);m.maximize({{live,1}},9);}
  const auto& h=good(historical);near(h.direction_value(live),1);assert(std::isnan(h.primal_data().direction[dead.id]));
  invalid([&]{h.direction_value(dead);});invalid([&]{h.direction_value({live.model_id+1,live.id});});invalid([&]{h.row_multiplier(removed);});
  options.limits.max_auxiliary_solves=1;auto capped=O::analyze_lp_evidence(basic,options);assert(capped.stop_reason==O::Termination::IterationLimit&&capped.attempted_calls==1&&capped.evidence->primal_ray().state!=State::Available);
  std::cout<<"LP evidence original signs, bounds, rays, Farkas, zero rows, identities and limits passed\n";
}
