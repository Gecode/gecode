// Compile lp_evidence.cpp separately with GECODE_OPTIMIZE_TEST_LP_EVIDENCE=1.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/lp_evidence.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
namespace O=Gecode::Optimize;
using Phase=O::LpEvidencePhase;using State=O::LpEvidenceState;using Request=O::LpEvidenceRequest;
namespace {
enum class Fault {None,Owner,Revision,Mask,Dimension,NaNValue,Objective,NaNBound,WrongBound,Gap,Status,Guarantee,
  FalseInfeasible,Unbounded,NoWitness,NormalizedInfeasible,FiniteLimit,OriginalDirection,InfiniteSide,CancellationSum};
Fault fault=Fault::None;Phase target=Phase::Recession;std::size_t calls=0;
std::string cancel_point,allocation_point;std::size_t cancel_index=0;
std::shared_ptr<O::CancellationToken> cancellation;
double previous=std::numeric_limits<double>::infinity();
O::SolveResult oracle(const O::ModelSnapshot& m,const O::SolveOptions& options){
  O::SolveResult out;out.model_id=m.model_id;out.revision=m.revision;out.guarantee=O::Guarantee::Numerical;
  out.backend="deterministic finite-grid oracle";for(const auto& v:m.variables)out.active_variables.push_back(v.active);
  std::vector<double> values(m.variables.size(),0);const double grid[]={-1,0,.25,.5,1};
  std::function<void(std::size_t)> visit=[&](std::size_t i){
    if(i<values.size()){for(double x:grid)if(x>=m.variables[i].lower&&x<=m.variables[i].upper){values[i]=x;visit(i+1);}return;}
    for(const auto& row:m.rows){long double sum=0;for(const auto& t:row.terms)sum+=t.coefficient*values[t.variable.id];
      if(sum<row.lower-options.feasibility_tolerance||sum>row.upper+options.feasibility_tolerance)return;}
    long double objective=m.objective.offset;for(const auto& t:m.objective.terms)objective+=t.coefficient*values[t.variable.id];
    if(!out.objective||(m.objective.sense==O::ObjectiveSense::Minimize?objective<*out.objective:objective>*out.objective)){out.objective=static_cast<double>(objective);out.values=values;}
  };visit(0);out.solution_validated=bool(out.objective);out.best_bound=out.objective;
  out.termination=out.objective?O::Termination::Optimal:O::Termination::Infeasible;return out;
}
void reset(){fault=Fault::None;target=Phase::Recession;calls=0;cancel_point.clear();allocation_point.clear();cancel_index=0;
  cancellation=std::make_shared<O::CancellationToken>();previous=std::numeric_limits<double>::infinity();}
}
namespace Gecode { namespace Optimize { namespace Detail {
SolveResult lp_evidence_test_solve(LpEvidencePhase phase,const ModelSnapshot& m,const SolveOptions& options){
  assert(options.backend==Backend::Highs&&options.guarantee==Guarantee::Numerical&&options.time_limit_seconds<=previous);
  previous=options.time_limit_seconds;++calls;auto out=oracle(m,options);if(phase!=target)return out;
  const auto discard=[&]{out.values.clear();out.objective.reset();out.best_bound.reset();out.solution_validated=false;};
  switch(fault){
    case Fault::None:break;
    case Fault::Owner:++out.model_id;break;case Fault::Revision:++out.revision;break;
    case Fault::Mask:out.active_variables[0]=false;break;
    case Fault::Dimension:out.values.push_back(0);break;
    case Fault::NaNValue:out.values[0]=std::numeric_limits<double>::quiet_NaN();break;
    case Fault::Objective:*out.objective+=1;break;
    case Fault::NaNBound:out.best_bound=std::numeric_limits<double>::quiet_NaN();break;
    case Fault::WrongBound:out.best_bound=std::numeric_limits<double>::infinity();break;
    case Fault::Gap:out.relative_gap=std::numeric_limits<double>::quiet_NaN();break;
    case Fault::Status:out.termination=static_cast<Termination>(99);break;
    case Fault::Guarantee:out.guarantee=Guarantee::Exact;break;
    case Fault::FalseInfeasible:out.termination=Termination::Infeasible;out.solution_validated=false;break;
    case Fault::Unbounded:discard();out.termination=Termination::Unbounded;break;
    case Fault::NoWitness:discard();break;
    case Fault::NormalizedInfeasible:discard();out.termination=Termination::Infeasible;break;
    case Fault::FiniteLimit:out.termination=Termination::IterationLimit;out.best_bound=-std::numeric_limits<double>::infinity();break;
    case Fault::OriginalDirection:out.values={1e-8};out.objective=-1e-8;out.best_bound=out.objective;break;
    case Fault::InfiniteSide:out.values={1e-8};out.objective=1e-8;out.best_bound=out.objective;break;
    case Fault::CancellationSum:out.values={.25,.25,.25};out.objective=.25;out.best_bound=out.objective;out.solution_validated=true;out.termination=Termination::Optimal;break;
  }
  return out;
}
void lp_evidence_test_checkpoint(const char* point,std::size_t index){
  if(cancel_point==point&&cancel_index==index)cancellation->cancel();
  if(allocation_point==point)throw std::bad_alloc();
}
}}}
int main(){
  O::Model source;auto x=source.add_continuous();source.minimize({{x,-1}},7);
  O::LpEvidenceOptions options;options.solve.time_limit_seconds=30;options.request=Request::Both;
  reset();auto good=O::analyze_lp_evidence(source,options);assert(good.completion==O::LpEvidenceCompletion::Complete&&calls==3&&previous<30);
  assert(good.evidence->primal_ray().state==State::Available&&good.evidence->direction_value(x)==1);
  for(auto f:{Fault::Owner,Fault::Revision,Fault::Mask,Fault::Dimension,Fault::NaNValue,Fault::Objective,
      Fault::NaNBound,Fault::WrongBound,Fault::Gap,Fault::Status,Fault::Guarantee,Fault::FalseInfeasible,Fault::Unbounded,Fault::NoWitness,Fault::NormalizedInfeasible}){
    reset();fault=f;auto out=O::analyze_lp_evidence(source,options);
    assert(out.stop_reason==((f==Fault::Unbounded||f==Fault::NormalizedInfeasible)?O::Termination::BackendError:O::Termination::NumericalFailure)&&out.completion==O::LpEvidenceCompletion::Rejected&&calls==2);
    assert(out.evidence->primal_ray().state==State::Rejected&&out.evidence->farkas().state==State::Rejected);
  }
  reset();target=Phase::FeasibleBase;fault=Fault::FalseInfeasible;
  auto false_infeasible=O::analyze_lp_evidence(source,options);assert(false_infeasible.stop_reason==O::Termination::NumericalFailure&&calls==1);
  reset();fault=Fault::FiniteLimit;auto limited=O::analyze_lp_evidence(source,options);
  assert(limited.completion==O::LpEvidenceCompletion::Interrupted&&limited.stop_reason==O::Termination::IterationLimit&&calls==2);
  assert(limited.evidence->primal_ray().state==State::Available&&!limited.evidence->stages()[2].attempted);
  // An auxiliary direction within its primal tolerance can fail after normalization.
  O::Model bounded;auto fixed=bounded.add_continuous(0,0);bounded.minimize({{fixed,-1}});
  reset();fault=Fault::OriginalDirection;auto normalized=O::analyze_lp_evidence(bounded,options);
  assert(normalized.stop_reason==O::Termination::NumericalFailure&&normalized.evidence->primal_ray().reason==O::LpEvidenceReason::FailedOriginalChecks);
  // Tiny auxiliary stationarity residual does not authorize an infinite bound side.
  O::Model free;auto f=free.add_continuous(-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());free.add_row({{f,1}},1,std::numeric_limits<double>::infinity());
  reset();target=Phase::Farkas;fault=Fault::InfiniteSide;options.request=Request::Farkas;
  auto side=O::analyze_lp_evidence(free,options);assert(side.stop_reason==O::Termination::NumericalFailure&&side.evidence->farkas().reason==O::LpEvidenceReason::FailedOriginalChecks);
  // Original cancellation must retain the small residual across large terms.
  O::Model cancellation_model;auto a=cancellation_model.add_continuous(-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
  auto b=cancellation_model.add_continuous(-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
  cancellation_model.add_row({{a,1}},1,std::numeric_limits<double>::infinity());cancellation_model.add_row({{b,1}},1e16,std::numeric_limits<double>::infinity());
  cancellation_model.add_row({{a,1},{b,1}},-std::numeric_limits<double>::infinity(),1e16);
  reset();target=Phase::Farkas;fault=Fault::CancellationSum;
  auto sum=O::analyze_lp_evidence(cancellation_model,options);assert(sum.completion==O::LpEvidenceCompletion::Complete&&sum.evidence->farkas().state==State::Available);
  assert(sum.evidence->farkas_data().contradiction_margin==1&&sum.evidence->farkas_data().max_stationarity==0);
  // Different valid declared tolerances overlap: reject incompatible conclusions.
  O::Model overlap;auto z=overlap.add_continuous(0,0);overlap.add_row({{z,1}},1e-8,std::numeric_limits<double>::infinity());
  reset();options.request=Request::Both;options.checks.minimum_contradiction=1e-10;
  auto inconsistent=O::analyze_lp_evidence(overlap,options);
  assert(inconsistent.stop_reason==O::Termination::NumericalFailure&&inconsistent.evidence->farkas().reason==O::LpEvidenceReason::InconsistentEvidence);
  assert(inconsistent.evidence->primal_ray().state==State::Rejected&&inconsistent.evidence->farkas().state==State::Rejected);
  assert(inconsistent.evidence->primal_data().base_check.valid&&inconsistent.evidence->farkas_data().contradiction_margin==1e-8);
  for(const auto& item:std::vector<std::pair<std::string,std::size_t>>{{"source_copy",0},{"admission",0},{"before_solve",1},{"after_solve",1},
      {"after_check",1},{"stage_cleanup",1},{"evidence_check",1},{"publication",3},{"source_cleanup",3}}){
    reset();cancel_point=item.first;cancel_index=item.second;options.solve.cancellation=cancellation;
    auto stopped=O::analyze_lp_evidence(source,options);assert(stopped.stop_reason==O::Termination::Cancelled);
    if(stopped.evidence){assert(stopped.evidence->primal_ray().state!=State::Available&&stopped.evidence->farkas().state!=State::Available);}
    assert(stopped.attempted_calls==calls);
  }
  reset();allocation_point="publication";options.solve.cancellation=cancellation;
  auto memory=O::analyze_lp_evidence(source,options);assert(memory.stop_reason==O::Termination::MemoryLimit&&memory.evidence->primal_ray().state!=State::Available);
  reset();options.solve.cancellation=cancellation;options.limits.max_work=good.work-1;
  auto work=O::analyze_lp_evidence(source,options);assert(work.work<=options.limits.max_work&&work.stop_reason==O::Termination::IterationLimit);
  assert(!work.evidence||work.evidence->primal_ray().state!=State::Available);
  std::cout<<"LP evidence adversarial status, original checks, consistency and lifecycle gates passed\n";
}
