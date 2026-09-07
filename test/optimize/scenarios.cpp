#include <gecode/optimize/scenarios.hpp>
#include <gecode/optimize/native.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>

namespace O=Gecode::Optimize;
constexpr double inf=std::numeric_limits<double>::infinity();
static std::size_t cases=0,points=0;
template<class F>static void invalid(F fn){bool caught=false;try{fn();}catch(const O::ModelError&){caught=true;}assert(caught);}
struct Oracle {std::optional<double> objective;std::vector<std::pair<std::vector<double>,bool>> membership;};
// This oracle reads raw base+patch records. It does not materialize the batch or
// call its validator, sparse normalizer, mappings, or solver implementation.
static Oracle enumerate(const O::ModelSnapshot& base,const O::ScenarioDefinition& def) {
  Oracle out;std::vector<double> values(base.variables.size(),std::numeric_limits<double>::quiet_NaN());
  const auto visit=[&] {
    ++points;
    const auto allowed=[&] {
      for(const auto& v:base.variables)if(v.active){
        double lo=v.lower,hi=v.upper;
        for(const auto& p:def.variable_bounds)if(p.variable.id==v.variable.id){if(p.lower)lo=*p.lower;if(p.upper)hi=*p.upper;}
        if(values[v.variable.id]<lo||values[v.variable.id]>hi)return false;
      }
      for(const auto& row:base.rows)if(row.active){
        double lo=row.lower,hi=row.upper;
        for(const auto& p:def.row_bounds)if(p.row.id==row.constraint.id){if(p.lower)lo=*p.lower;if(p.upper)hi=*p.upper;}
        long long sum=0;for(const auto& t:row.terms)sum+=static_cast<long long>(t.coefficient)*static_cast<long long>(values[t.variable.id]);
        if(sum<lo||sum>hi)return false;
      }
      return true;
    };
    const bool accepted=allowed();out.membership.emplace_back(values,accepted);if(!accepted)return;
    long long objective=static_cast<long long>(def.objective_offset.value_or(base.objective.offset));
    for(std::size_t i=0;i<values.size();++i)if(base.variables[i].active){
      double coefficient=0;for(const auto& t:base.objective.terms)if(t.variable.id==i)coefficient=t.coefficient;
      for(const auto& t:def.objective_coefficients)if(t.variable.id==i)coefficient=t.coefficient;
      objective+=static_cast<long long>(coefficient)*static_cast<long long>(values[i]);
    }
    if(!out.objective || (base.objective.sense==O::ObjectiveSense::Minimize?objective<*out.objective:objective>*out.objective))out.objective=static_cast<double>(objective);
  };
  std::function<void(std::size_t)> next=[&](std::size_t slot) {
    if(slot==values.size()){visit();return;}
    const auto& v=base.variables[slot];if(!v.active){next(slot+1);return;}
    double lo=v.lower,hi=v.upper;for(const auto& p:def.variable_bounds)if(p.variable.id==slot){if(p.lower)lo=std::min(lo,*p.lower);if(p.upper)hi=std::max(hi,*p.upper);}
    assert(std::isfinite(lo)&&std::isfinite(hi)&&hi-lo<10);
    for(int x=static_cast<int>(std::ceil(lo));x<=static_cast<int>(std::floor(hi));++x){values[slot]=x;next(slot+1);}
  };
  next(0);return out;
}
static void assert_outcome(const O::ScenarioBatchResult& batch,std::size_t i,const Oracle& oracle) {
  ++cases;const auto& outcome=batch.outcomes[i];assert(outcome.result&&outcome.state==O::ScenarioRunState::Attempted);
  const auto& result=*outcome.result;
  const auto model=batch.batch->materialize(outcome.scenario);
  for(const auto& point:oracle.membership)assert(O::validate(model,point.first).valid==point.second);
  if(!oracle.objective){assert(result.termination==O::Termination::Infeasible&&!result.has_solution());return;}
  if(result.termination!=O::Termination::Optimal)std::cerr<<result.message<<'\n';
  assert(result.termination==O::Termination::Optimal&&result.has_solution());
  assert(result.objective==oracle.objective&&outcome.check&&outcome.check->validation.valid);
  assert(outcome.check->identity_valid&&outcome.check->objective_matches);
  assert(result.model_id==batch.batch->id()&&result.revision==i+1);
  assert(O::validate(model,result.values).valid);
}
static void discrete(O::Backend backend) {
  for(bool maximize:{false,true})for(int a=-2;a<=2;++a)for(int b=-1;b<=1;++b){
    O::Model model;const auto removed=model.add_integer(-1,1);model.remove(removed);
    const auto x=model.add_integer(-2,2),y=model.add_binary();
    const auto deleted=model.add_row({},-1,1);model.remove(deleted);
    const auto row=model.add_row({{x,static_cast<double>(a)},{y,static_cast<double>(b)}},-1,2);
    model.set_objective({{x,2},{y,-1}},maximize?O::ObjectiveSense::Maximize:O::ObjectiveSense::Minimize,7);
    const auto base=model.snapshot();
    std::vector<O::ScenarioDefinition> definitions(6);
    definitions[1].objective_coefficients={{x,0},{y,3}};definitions[1].objective_offset=-9;
    definitions[2].variable_bounds={{x,-1,1},{y,1,1}};
    definitions[3].row_bounds={{row,2,2}};
    definitions[5].variable_bounds={{x,-3,3}};definitions[5].row_bounds={{row,-4,4}};
    definitions[5].objective_coefficients={{x,-3}};definitions[5].objective_offset=8;
    O::ScenarioBatchOptions options;options.solve.backend=backend;
    if(backend==O::Backend::Native)options.solve.guarantee=O::Guarantee::Exact;
    const auto result=O::solve_scenarios(model,definitions,options);
    if(!result.all_resolved())std::cerr<<result.message<<'\n';
    assert(result.all_resolved()&&result.batch&&result.batch->id()!=model.id());
    assert(model.revision()==base.revision&&model.snapshot().objective.offset==7);
    for(std::size_t i=0;i<definitions.size();++i){
      assert_outcome(result,i,enumerate(base,definitions[i]));
      if(result.outcomes[i].result->has_solution()) {
        assert(result.value(result.batch->scenario(i),x)==result.outcomes[i].result->values[x.id]);
        if(backend==O::Backend::Native)assert(result.outcomes[i].check->exact_witness_validated);
      }
    }
    invalid([&]{result.batch->map(removed);});invalid([&]{result.batch->map(deleted);});
    invalid([&]{result.value({model.id(),0},x);});invalid([&]{result.batch->scenario(6);});
    invalid([&]{result.value(result.batch->scenario(0),O::Variable{model.id()+1,x.id});});
  }
}
static void admission(){
  O::Model model;auto x=model.add_integer(0,3);auto row=model.add_row({{x,1}},0,3);model.minimize({{x,1}},1);
  std::vector<O::ScenarioDefinition> definitions(2);O::ScenarioBatchOptions options;
  for(int mode=0;mode<15;++mode){
    auto defs=definitions;
    if(mode==0)defs[1].objective_coefficients={{x,1},{x,2}};
    if(mode==1)defs[1].objective_coefficients={{{999,x.id},1}};
    if(mode==2)defs[1].objective_coefficients={{x,inf}};
    if(mode==3)defs[1].objective_offset=inf;
    if(mode==4)defs[1].variable_bounds={{x,2,1}};
    if(mode==5)defs[1].variable_bounds={{x,{},{} }};
    if(mode==6)defs[1].variable_bounds={{x,0,2},{x,0,1}};
    if(mode==7)defs[1].variable_bounds={{x,inf,{}}};
    if(mode==8)defs[1].row_bounds={{row,0,1},{row,0,2}};
    if(mode==9)defs[1].row_bounds={{row,{},{} }};
    if(mode==10)defs[1].row_bounds={{row,4,2}};
    if(mode==11)defs[1].row_bounds={{{999,row.id},0,1}};
    if(mode==12)defs[1].row_bounds={{row,{},-inf}};
    if(mode==13)defs[1].variable_bounds={{x,std::numeric_limits<double>::quiet_NaN(),{}}};
    if(mode==14)defs[1].objective_coefficients={{{model.id(),99},1}};
    const auto out=O::solve_scenarios(model,defs,options);
    assert(!out.batch&&out.outcomes.empty()&&out.attempted==0&&out.stop_reason==O::Termination::InvalidModel&&out.offending_scenario==1);
  }
  for(int mode=0;mode<4;++mode){auto capped=options;
    if(mode==0)capped.max_scenarios=1;
    if(mode==1){capped.max_patch_entries=0;definitions[1].objective_offset=2;definitions[1].objective_coefficients={{x,1}};}
    if(mode==2)capped.max_saved_value_slots=1;
    if(mode==3)capped.max_work=0;
    const auto out=O::solve_scenarios(model,definitions,capped);
    assert(!out.batch&&!out.attempted&&out.stop_reason==(mode==3?O::Termination::IterationLimit:O::Termination::MemoryLimit));
    definitions=std::vector<O::ScenarioDefinition>(2);
  }
  auto empty=O::solve_scenarios(model,{});assert(empty.all_resolved()&&empty.attempted==0&&empty.outcomes.empty());
  options.solve.node_limit=1;auto nodes=O::solve_scenarios(model,definitions,options);assert(nodes.stop_reason==O::Termination::Unsupported&&!nodes.attempted);
  options.solve.node_limit=0;nodes=O::solve_scenarios(model,definitions,options);assert(nodes.stop_reason==O::Termination::NodeLimit&&!nodes.attempted);
  options={};options.solve.time_limit_seconds=0;assert(O::solve_scenarios(model,definitions,options).stop_reason==O::Termination::TimeLimit);
  options={};options.solve.cancellation=std::make_shared<O::CancellationToken>();options.solve.cancellation->cancel();
  assert(O::solve_scenarios(model,definitions,options).stop_reason==O::Termination::Cancelled);
  options={};options.solve.primal_start={{x,0}};assert(O::solve_scenarios(model,definitions,options).stop_reason==O::Termination::Unsupported);
  options={};options.reuse=static_cast<O::ScenarioReuse>(55);assert(O::solve_scenarios(model,definitions,options).stop_reason==O::Termination::InvalidModel);
  auto malformed=model.snapshot();malformed.rows[0].terms[0].variable.model_id=999;
  assert(O::solve_scenarios(malformed,definitions).stop_reason==O::Termination::InvalidModel);
  O::Model binary;auto b=binary.add_binary();O::ScenarioDefinition bad;bad.variable_bounds={{b,-1,2}};
  assert(O::solve_scenarios(binary,{bad}).stop_reason==O::Termination::InvalidModel);
  O::Model semi;semi.add_variable(O::VariableType::SemiInteger,2,4);
  assert(O::solve_scenarios(semi,{{}}).stop_reason==O::Termination::Unsupported);
  auto g=binary.add_global(O::AllDifferentData{{b}});
  assert(O::solve_scenarios(binary,{{}}).stop_reason==O::Termination::Unsupported);
  binary.remove(g);assert(O::solve_scenarios(binary,{{}}).stop_reason==O::Termination::Unsupported);
}
static void guarantees(){
  O::Model model;auto x=model.add_integer(0,2);model.minimize({{x,1}});
  for(auto backend:{O::Backend::Auto,O::Backend::Highs,O::Backend::Native})
    for(auto guarantee:{O::Guarantee::Numerical,O::Guarantee::Exact,O::Guarantee::Certified})
      for(std::size_t count:{std::size_t(0),std::size_t(1)}){
        O::ScenarioBatchOptions opts;opts.solve.backend=backend;opts.solve.guarantee=guarantee;
        const auto result=O::solve_scenarios(model,std::vector<O::ScenarioDefinition>(count),opts);
        const bool rejected=guarantee==O::Guarantee::Certified || (guarantee==O::Guarantee::Exact&&backend!=O::Backend::Native);
        if(rejected){assert(result.stop_reason==O::Termination::Unsupported&&!result.batch&&!result.attempted);}
        else if(count==0){assert(result.all_resolved()&&result.attempted==0);}
        else if(O::capabilities(backend==O::Backend::Auto?O::Backend::Highs:backend).available){
          assert(result.all_resolved()&&result.outcomes[0].result->guarantee==guarantee);
        }else{assert(result.stop_reason==O::Termination::Unsupported&&result.attempted==1);}
      }
}
static void numerical(){
  O::Model model;auto x=model.add_continuous(0,10),y=model.add_continuous(0,10);
  auto row=model.add_row({{x,1},{y,1}},3,inf);model.minimize({{x,2},{y,3}},7);
  std::vector<O::ScenarioDefinition> defs(5);defs[1].objective_coefficients={{x,4}};
  defs[2].variable_bounds={{x,{},1}};defs[3].row_bounds={{row,4,{}}};
  O::ScenarioBatchOptions options;options.solve.backend=O::Backend::Highs;
  const auto warm=O::solve_scenarios(model,defs,options);assert(warm.all_resolved());
  const double expected[]={13,16,15,15,13};
  for(std::size_t i=0;i<5;++i)assert(std::abs(*warm.outcomes[i].result->objective-expected[i])<1e-7);
  assert(warm.reuse_statistics.solve_calls==5&&warm.reuse_statistics.model_loads==1&&warm.reuse_statistics.incremental_updates==4);
  options.reuse=O::ScenarioReuse::Cold;const auto cold=O::solve_scenarios(model,defs,options);
  assert(cold.all_resolved()&&cold.reuse_statistics.solve_calls==0);
  for(std::size_t i=0;i<5;++i)assert(cold.outcomes[i].result->objective==warm.outcomes[i].result->objective);
  model.set_bounds(x,9,10);defs.clear();
  assert(warm.batch->base().variables[x.id].lower==0&&warm.value(warm.batch->scenario(0),x)==3);
  auto changed=warm.batch->materialize(warm.batch->scenario(0));changed.variables[x.id].lower=8;
  assert(warm.batch->materialize(warm.batch->scenario(0)).variables[x.id].lower==0);
  O::Model unbounded;auto z=unbounded.add_continuous();unbounded.minimize({{z,-1}});
  O::ScenarioDefinition finite;finite.variable_bounds={{z,{},2}};
  auto result=O::solve_scenarios(unbounded,{finite,{}});assert(result.all_resolved());
  assert(result.outcomes[0].result->objective==-2&&result.outcomes[1].result->termination==O::Termination::Unbounded);
  O::Model constant;auto c=constant.add_row({},0,inf);O::ScenarioDefinition impossible;impossible.row_bounds={{c,1,{}}};
  result=O::solve_scenarios(constant,{{},impossible,{}});assert(result.all_resolved());
  assert(result.outcomes[0].result->objective==0&&result.outcomes[1].result->termination==O::Termination::Infeasible);
  O::Model recourse;auto decision=recourse.add_integer(0,2),completion=recourse.add_continuous(0,10);
  recourse.add_row({{decision,1},{completion,1}},2,inf);recourse.minimize({{decision,1},{completion,2}});
  O::ScenarioDefinition fixed;fixed.variable_bounds={{decision,0,0}};
  result=O::solve_scenarios(recourse,{{},fixed});assert(result.all_resolved());
  assert(result.outcomes[0].result->objective==2&&result.outcomes[1].result->objective==4);
  O::Model cancellation;auto a=cancellation.add_continuous(1,1),b=cancellation.add_continuous(1e6,1e6);
  cancellation.minimize({{a,1},{b,1e10}},-1e16);
  result=O::solve_scenarios(cancellation,{{}});assert(result.all_resolved()&&result.outcomes[0].result->objective==1);
}
int main(){
  admission();guarantees();
  if(O::capabilities(O::Backend::Highs).available){discrete(O::Backend::Highs);numerical();}
  else {O::Model m;m.add_integer(0,1);auto out=O::solve_scenarios(m,{{}});assert(out.stop_reason==O::Termination::Unsupported&&!out.all_resolved());}
  if(O::native_capabilities().available)discrete(O::Backend::Native);
  std::cout<<"Scenario admission, histories and cold/reuse: "<<cases<<" outcomes / "<<points<<" exhaustive points passed\n";
}
