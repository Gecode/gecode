// Compile scenarios.cpp separately with GECODE_OPTIMIZE_TEST_SCENARIOS=1.
// The oracle below enumerates the original finite box; no backend calls occur.
#include <gecode/optimize/scenarios.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <string>

namespace O=Gecode::Optimize;
namespace {
enum class Fault {None,Owner,Revision,Mask,Objective,NaNObjective,NaNBound,Bound,
  FalseInfeasible,FalseUnbounded,NoWitness,Guarantee,Incomplete,InfiniteBound,
  Infeasible,Exception,ExactFraction,UnvalidatedObjective,InvalidUnvalidatedPoint,VendorGap,
  UnknownStatus,LooseExactBound,MissingExactBound};
Fault fault=Fault::None;
std::size_t calls=0,fault_call=1;
std::string cancel_point;
std::size_t cancel_index=1;
std::shared_ptr<O::CancellationToken> token;
double last_allowance=std::numeric_limits<double>::infinity();
std::optional<std::uint64_t> observed_nodes;
O::SolveResult oracle(const O::ModelSnapshot& model) {
  O::SolveResult out;out.model_id=model.model_id;out.revision=model.revision;
  for(const auto& v:model.variables)out.active_variables.push_back(v.active);
  std::vector<double> values(model.variables.size(),std::numeric_limits<double>::quiet_NaN());
  std::function<void(std::size_t)> visit=[&](std::size_t slot) {
    if(slot<values.size()) {
      const auto& v=model.variables[slot];if(!v.active){visit(slot+1);return;}
      assert(std::isfinite(v.lower)&&std::isfinite(v.upper)&&v.upper-v.lower<10);
      for(int x=static_cast<int>(std::ceil(v.lower));x<=static_cast<int>(std::floor(v.upper));++x){values[slot]=x;visit(slot+1);}return;
    }
    for(const auto& r:model.rows)if(r.active){long long sum=0;
      for(const auto& t:r.terms)sum+=static_cast<long long>(t.coefficient)*static_cast<long long>(values[t.variable.id]);
      if(sum<r.lower||sum>r.upper)return;}
    long long objective=static_cast<long long>(model.objective.offset);
    for(const auto& t:model.objective.terms)objective+=static_cast<long long>(t.coefficient)*static_cast<long long>(values[t.variable.id]);
    if(!out.objective || (model.objective.sense==O::ObjectiveSense::Minimize?objective<*out.objective:objective>*out.objective)){
      out.objective=static_cast<double>(objective);out.values=values;}
  };
  visit(0);out.solution_validated=out.objective.has_value();out.best_bound=out.objective;
  out.termination=out.objective?O::Termination::Optimal:O::Termination::Infeasible;return out;
}
void reset(){fault=Fault::None;calls=0;cancel_point.clear();token=std::make_shared<O::CancellationToken>();last_allowance=std::numeric_limits<double>::infinity();observed_nodes.reset();}
}
namespace Gecode { namespace Optimize { namespace Detail {
SolveResult scenario_test_solve(const ModelSnapshot& model,const SolveOptions& options,SolveSession*) {
  assert(options.time_limit_seconds<=last_allowance);last_allowance=options.time_limit_seconds;observed_nodes=options.node_limit;
  auto out=oracle(model);out.guarantee=options.guarantee;const auto number=calls++;
  if(number!=fault_call)return out;
  switch(fault){
  case Fault::None:break;
  case Fault::Owner:++out.model_id;break;
  case Fault::Revision:++out.revision;break;
  case Fault::Mask:out.active_variables[0]=true;break;
  case Fault::Objective:*out.objective+=1;break;
  case Fault::NaNObjective:out.objective=std::numeric_limits<double>::quiet_NaN();break;
  case Fault::NaNBound:out.best_bound=std::numeric_limits<double>::quiet_NaN();break;
  case Fault::Bound:*out.best_bound=*out.objective+1;break;
  case Fault::FalseInfeasible:out.termination=Termination::Infeasible;out.solution_validated=false;break;
  case Fault::FalseUnbounded:out.termination=Termination::Unbounded;break;
  case Fault::NoWitness:out.values.clear();out.objective.reset();out.best_bound.reset();out.solution_validated=false;break;
  case Fault::Guarantee:out.guarantee=Guarantee::Certified;break;
  case Fault::Incomplete:out.termination=Termination::NodeLimit;break;
  case Fault::InfiniteBound:out.termination=Termination::NodeLimit;out.best_bound=-std::numeric_limits<double>::infinity();break;
  case Fault::Infeasible:out.termination=Termination::Infeasible;out.values.clear();out.objective.reset();out.best_bound.reset();out.solution_validated=false;break;
  case Fault::Exception:throw std::bad_alloc();
  case Fault::ExactFraction:out.values[1]+=.0000001;out.objective=*out.objective+.0000001;out.best_bound=out.objective;break;
  case Fault::UnvalidatedObjective:out.solution_validated=false;out.termination=Termination::NodeLimit;*out.objective+=1;break;
  case Fault::InvalidUnvalidatedPoint:out.solution_validated=false;out.termination=Termination::NodeLimit;out.values[1]=-100;break;
  case Fault::VendorGap:out.native_backend_gap=-1;break;
  case Fault::UnknownStatus:out.termination=static_cast<Termination>(99);break;
  case Fault::LooseExactBound:*out.best_bound=*out.objective-1;break;
  case Fault::MissingExactBound:out.best_bound.reset();break;
  }
  return out;
}
void scenario_test_checkpoint(const char* point,std::size_t index) {
  if(cancel_point==point&&index==cancel_index)token->cancel();
}
}}}
int main(){
  O::Model model;auto removed=model.add_integer(0,1);model.remove(removed);
  auto x=model.add_integer(0,4);model.minimize({{x,1}},3);
  std::vector<O::ScenarioDefinition> defs(3);defs[1].variable_bounds={{x,2,{}}};defs[2].objective_offset=-7;
  O::ScenarioBatchOptions options;options.solve.time_limit_seconds=30;
  reset();auto good=O::solve_scenarios(model,defs,options);assert(good.all_resolved()&&calls==3&&last_allowance<30);
  assert(good.outcomes[0].result->objective==3&&good.outcomes[1].result->objective==5&&good.outcomes[2].result->objective==-7);
  for(auto f:{Fault::Owner,Fault::Revision,Fault::Mask,Fault::Objective,Fault::NaNObjective,Fault::NaNBound,
      Fault::Bound,Fault::FalseInfeasible,Fault::FalseUnbounded,Fault::NoWitness,Fault::Guarantee,
      Fault::UnvalidatedObjective,Fault::InvalidUnvalidatedPoint,Fault::VendorGap,Fault::UnknownStatus}) {
    reset();fault=f;const auto out=O::solve_scenarios(model,defs,options);
    assert(out.stop_reason==O::Termination::NumericalFailure&&out.resolved==1&&out.attempted==2&&calls==2);
    assert(out.outcomes[0].result->has_solution()&&out.outcomes[0].result->objective==3);
    assert(out.outcomes[1].result&&out.outcomes[1].result->termination==O::Termination::NumericalFailure&&!out.outcomes[1].result->has_solution());
    assert(out.outcomes[2].state==O::ScenarioRunState::NotStarted&&!out.outcomes[2].result);
  }
  for(auto f:{Fault::Incomplete,Fault::InfiniteBound}){
    reset();fault=f;const auto out=O::solve_scenarios(model,defs,options);
    assert(out.stop_reason==O::Termination::NodeLimit&&out.resolved==1&&calls==2);
    assert(out.outcomes[1].result->has_solution()&&out.outcomes[1].check->validation.valid);
    if(f==Fault::InfiniteBound)assert(out.outcomes[1].result->best_bound&&!std::isfinite(*out.outcomes[1].result->best_bound)&&!out.outcomes[1].result->absolute_gap);
  }
  reset();fault=Fault::Infeasible;auto infeasible=O::solve_scenarios(model,defs,options);assert(infeasible.all_resolved()&&calls==3);
  reset();fault=Fault::Exception;auto allocation=O::solve_scenarios(model,defs,options);
  assert(allocation.stop_reason==O::Termination::MemoryLimit&&allocation.resolved==1&&allocation.outcomes[1].result->termination==O::Termination::MemoryLimit);
  for(const auto* point: {"admission","before_solve","after_solve","after_check","after_cleanup"}) {
    reset();cancel_point=point;options.solve.cancellation=token;
    const auto out=O::solve_scenarios(model,defs,options);assert(out.stop_reason==O::Termination::Cancelled);
    if(cancel_point=="admission")assert(!out.batch&&calls==0);
    else {
      assert(out.batch&&out.resolved==1&&out.outcomes[0].result->has_solution());
      assert(!out.outcomes[1].result||!out.outcomes[1].result->has_solution());
      assert(calls==(cancel_point=="before_solve"?1:2));
    }
  }
  reset();cancel_point="batch_cleanup";cancel_index=3;options.solve.cancellation=token;
  auto late=O::solve_scenarios(model,defs,options);assert(late.stop_reason==O::Termination::Cancelled&&!late.all_resolved()&&late.resolved==3);
  for(const auto& stage:late.outcomes)assert(stage.result->has_solution());
  options={};reset();options.solve.backend=O::Backend::Native;options.solve.guarantee=O::Guarantee::Exact;
  auto native=O::solve_scenarios(model,defs,options);assert(native.all_resolved());for(const auto& stage:native.outcomes)assert(stage.check->exact_witness_validated);
  reset();fault=Fault::ExactFraction;auto fraction=O::solve_scenarios(model,defs,options);assert(fraction.stop_reason==O::Termination::NumericalFailure&&fraction.resolved==1);
  for(auto f:{Fault::LooseExactBound,Fault::MissingExactBound}) {
    reset();fault=f;auto bad=O::solve_scenarios(model,defs,options);
    assert(bad.stop_reason==O::Termination::NumericalFailure&&bad.resolved==1&&!bad.all_resolved());
  }
  reset();options.solve.node_limit=7;auto one=O::solve_scenarios(model,{{}},options);assert(one.all_resolved()&&calls==1&&observed_nodes==7);
  reset();options.solve.node_limit=0;auto zero=O::solve_scenarios(model,{{}},options);assert(zero.stop_reason==O::Termination::NodeLimit&&calls==0);
  options={};reset();auto measured=O::solve_scenarios(model,defs,options);assert(measured.all_resolved());
  bool partial=false;
  for(std::size_t cap=0;cap<=measured.work;++cap){reset();options.max_work=cap;auto capped=O::solve_scenarios(model,defs,options);
    assert(capped.work<=cap);if(capped.resolved>0&&capped.resolved<3)partial=true;
    if(!capped.all_resolved())assert(capped.stop_reason==O::Termination::IterationLimit);
    if(capped.resolved)assert(capped.outcomes[0].result->has_solution());}
  assert(partial);
  // The private historical snapshot survives original edits/destruction.
  O::ScenarioBatchResult historical;
  {O::Model local;auto v=local.add_integer(1,2);local.minimize({{v,1}},5);reset();options={};historical=O::solve_scenarios(local,{{}},options);}
  assert(historical.all_resolved()&&historical.batch->materialize(historical.batch->scenario(0)).objective.offset==5);
  // A public snapshot can guess the next process-local owner number; the
  // artifact must still keep its private identity distinct from that source.
  O::Model predicted;auto p=predicted.add_integer(0,1);predicted.minimize({{p,1}});
  auto forged=predicted.snapshot();++forged.model_id;
  forged.variables[0].variable.model_id=forged.model_id;forged.objective.terms[0].variable.model_id=forged.model_id;
  reset();auto distinct=O::solve_scenarios(forged,{{}},options);
  assert(distinct.all_resolved()&&distinct.batch->id()!=forged.model_id);
  std::cout<<"Scenario coordinator evidence, every work boundary and interruption gates passed\n";
}
