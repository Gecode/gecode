#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
std::uint64_t runs=0;
thread_local std::function<void(const char*)> hook;
// Independent finite product oracle: no native compiler, propagator or common
// validator is used to compute feasible points or objective bounds.
bool feasible(const ModelSnapshot& model,const std::vector<double>& point) {
  const auto row=[&](const std::vector<Term>& terms,double lower,double upper) {
    std::int64_t sum=0;
    for(const auto& term:terms) sum+=static_cast<std::int64_t>(term.coefficient)*static_cast<std::int64_t>(point[term.variable.id]);
    return sum>=lower && sum<=upper;
  };
  for(const auto& variable:model.variables) if(variable.active) {
    const auto value=point.at(variable.variable.id);
    if(value!=std::trunc(value) || !std::isfinite(value)) return false;
    if(!(variable.type==VariableType::SemiInteger && value==0) && (value<variable.lower || value>variable.upper)) return false;
  }
  for(const auto& original:model.rows)
    if(original.active && !original.indicator_origin && !row(original.terms,original.lower,original.upper)) return false;
  for(const auto& indicator:model.indicators) if(indicator.active) {
    const bool enabled=point[indicator.activator.id]==(indicator.active_value?1:0);
    if(enabled && !row(indicator.terms,indicator.lower,indicator.upper)) return false;
    if(indicator.inactive_gate && point[indicator.inactive_gate->id]!=(enabled?0:1)) return false;
  }
  for(const auto& global:model.globals) if(global.active) {
    const bool valid=std::visit([&](const auto& data) {
      using T=std::decay_t<decltype(data)>;
      const auto value=[&](Variable v) {return static_cast<std::int64_t>(point[v.id]);};
      if constexpr(std::is_same_v<T,AllDifferentData>) {
        std::set<std::int64_t> seen;
        for(auto v:data.variables) if(!seen.insert(value(v)).second) return false;
        return true;
      } else if constexpr(std::is_same_v<T,ElementData>) {
        const auto index=value(data.index)-data.index_base;
        return index>=0 && static_cast<std::size_t>(index)<data.elements.size() && value(data.result)==value(data.elements[index]);
      } else if constexpr(std::is_same_v<T,TableData>) {
        std::vector<std::int64_t> tuple;for(auto v:data.variables) tuple.push_back(value(v));
        return std::find(data.tuples.begin(),data.tuples.end(),tuple)!=data.tuples.end();
      } else if constexpr(std::is_same_v<T,CumulativeData>) {
        std::set<std::int64_t> starts;
        for(std::size_t i=0;i<data.starts.size();++i) if(data.durations[i]) starts.insert(value(data.starts[i]));
        for(auto time:starts) {
          std::int64_t demand=0;
          for(std::size_t i=0;i<data.starts.size();++i)
            if(value(data.starts[i])<=time && time<value(data.starts[i])+data.durations[i]) demand+=data.heights[i];
          if(demand>data.capacity) return false;
        }
        return true;
      } else if constexpr(std::is_same_v<T,RegularData>) {
        auto state=data.initial_state;
        for(auto variable:data.variables){
          const auto edge=std::find_if(data.transitions.begin(),data.transitions.end(),[&](const RegularTransition& t){return t.from==state&&t.symbol==value(variable);});
          if(edge==data.transitions.end())return false;
          state=edge->to;
        }
        return std::find(data.final_states.begin(),data.final_states.end(),state)!=data.final_states.end();
      } else {
        if(data.successors.empty()) return false;
        std::set<std::int64_t> seen;std::int64_t next=0;
        for(std::size_t i=0;i<data.successors.size();++i) {
          if(next<0 || static_cast<std::size_t>(next)>=data.successors.size() || !seen.insert(next).second) return false;
          next=value(data.successors[next])-data.index_base;
        }
        return next==0;
      }
    },global.payload);
    if(!valid) return false;
  }
  return true;
}
std::int64_t objective(const ModelSnapshot& model,const std::vector<double>& point) {
  auto value=static_cast<std::int64_t>(model.objective.offset);
  for(const auto& term:model.objective.terms) value+=static_cast<std::int64_t>(term.coefficient)*static_cast<std::int64_t>(point[term.variable.id]);
  return value;
}std::vector<std::vector<double>> points(const ModelSnapshot& model) {
  std::vector<double> point(model.variables.size(),std::numeric_limits<double>::quiet_NaN());
  std::vector<std::vector<double>> all;
  const auto visit=[&](const auto& self,std::size_t slot)->void {
    if(slot==model.variables.size()) {all.push_back(point);return;}
    const auto& v=model.variables[slot];if(!v.active){self(self,slot+1);return;}
    assert(std::isfinite(v.lower) && std::isfinite(v.upper) && v.upper-v.lower<=6);
    if(v.type==VariableType::SemiInteger && v.lower>0){point[slot]=0;self(self,slot+1);}
    for(std::int64_t value=static_cast<std::int64_t>(v.lower);value<=v.upper;++value){point[slot]=static_cast<double>(value);self(self,slot+1);}
  };visit(visit,0);return all;
}
std::optional<std::int64_t> oracle(const ModelSnapshot& source) {
  std::optional<std::int64_t> best;
  for(const auto& p:points(source))if(feasible(source,p)){
    const auto value=objective(source,p);
    if(!best || (source.objective.sense==ObjectiveSense::Minimize?value<*best:value>*best))best=value;
  }return best;
}
std::vector<StartValue> entries(const ModelSnapshot& source,const std::vector<double>& p,bool omit_gates=false) {
  std::set<std::uint64_t> live;
  if(omit_gates)for(const auto& i:source.indicators)if(i.active && i.inactive_gate)live.insert(i.inactive_gate->id);
  std::vector<StartValue> out;
  for(const auto& v:source.variables)if(v.active && !live.count(v.variable.id))out.push_back({v.variable,p[v.variable.id]});
  return out;
}
struct Answer {SolveResult result;NativeFrontierStatistics frontier;NativeBranchingStatistics branching;NativeLpStatistics lp;};
bool is_frontier(int route){return route>=2 && route<=6;}
std::vector<int> routes(){return native_lp_capabilities().available?std::vector<int>{0,1,2,3,4,5,6,7}:std::vector<int>{0,2,3,5};}
Answer solve(const ModelSnapshot& source,const SolveOptions& options,int route,std::size_t resident=100000) {
  Answer out;
  if(route==0)out.result=solve_native(source,options);
  else if(route==1 || route==7){NativeLpOptions lp;lp.solve=options;lp.frequency=NativeLpFrequency::AfterBoundChanges;if(route==7)lp.root_cover_cuts=NativeRootCoverSettings{};
    auto r=solve_native_lp(source,lp);out.result=std::move(r.result);out.lp=r.relaxation;
  }else{NativeSearchOptions search;search.solve=options;search.max_open_nodes=resident;search.order=route==2?NativeSearchOrder::DepthFirst:NativeSearchOrder::BestBound;
    if(route==4 || route==6){search.relaxation=NativeLpSettings{};search.relaxation->frequency=NativeLpFrequency::AfterBoundChanges;if(route==6)search.relaxation->root_cover_cuts=NativeRootCoverSettings{};}
    if(route==5 || route==6)search.branching=NativeBranchingSettings{};
    auto r=solve_native_search(source,search);out.result=std::move(r.result);out.frontier=r.frontier;out.branching=r.branching;out.lp=r.relaxation;
  }return out;
}
SolveOptions options(){SolveOptions out;out.backend=Backend::Native;out.guarantee=Guarantee::Exact;return out;}
void verify(const ModelSnapshot& source,const Answer& answer,int route,const SolveOptions& o) {
  ++runs;const auto best=oracle(source);const auto& r=answer.result;const bool min=source.objective.sense==ObjectiveSense::Minimize;
  assert(r.model_id==source.model_id && r.revision==source.revision && r.guarantee==o.guarantee);
  if(r.has_solution()){
    assert(best && feasible(source,r.values) && r.objective==static_cast<double>(objective(source,r.values)));
    assert(min?*r.objective>=*best:*r.objective<=*best);
    for(const auto& v:source.variables){assert(r.active_variables[v.variable.id]==v.active);if(!v.active)assert(std::isnan(r.values[v.variable.id]));}
  }
  if(r.start_submitted)assert(r.has_solution());
  if(r.best_bound){assert(std::isfinite(*r.best_bound));if(best)assert(min?*r.best_bound<=*best:*r.best_bound>=*best);if(r.has_solution())assert(min?*r.best_bound<=*r.objective:*r.best_bound>=*r.objective);}
  if(r.termination==Termination::Optimal)assert(best && r.has_solution() && r.objective==static_cast<double>(*best) && r.best_bound==r.objective);
  if(r.termination==Termination::Infeasible)assert(!best && !r.has_solution() && !r.start_submitted && !r.best_bound);
  if(!is_frontier(route) && r.termination!=Termination::Optimal)assert(!r.best_bound && !r.absolute_gap && !r.relative_gap);
  if(is_frontier(route)){assert(answer.branching.budget_nodes==answer.frontier.admitted_nodes+answer.branching.probe_status_calls);if(o.node_limit)assert(answer.branching.budget_nodes<=*o.node_limit);}
}
void exhaustive(const Model& model) {
  const auto source=model.snapshot();const auto all=points(source);const auto best=oracle(source);
  for(auto route:routes()){
    auto o=options();const auto cold=solve(source,o,route);verify(source,cold,route,o);assert(!cold.result.start_submitted);
    assert(cold.result.termination==(best?Termination::Optimal:Termination::Infeasible));
    for(const auto& point:all){o.primal_start=entries(source,point);if(o.primal_start.empty())continue;
      const auto r=solve(source,o,route);verify(source,r,route,o);
      if(feasible(source,point)){
        assert(r.result.start_submitted && r.result.termination==Termination::Optimal);
        if(!source.indicators.empty()){o.primal_start=entries(source,point,true);const auto derived=solve(source,o,route);verify(source,derived,route,o);assert(derived.result.start_submitted && derived.result.termination==Termination::Optimal);}
      }else assert(r.result.termination==Termination::InvalidModel && !r.result.has_solution() && !r.result.start_submitted && !r.lp.lp_calls);
    }
  }
  assert(source.revision==model.revision());
}
void fixtures(){
  for(int n=0;n<8;++n){Model m;const auto dead=m.add_binary();m.remove(dead);const auto x=m.add_integer(-1,1),y=m.add_binary();m.add_row({{x,2},{y,1}},-1,1);m.set_objective({{x,double(n%3-1)},{y,double(n%2?2:-3)}},n%2?ObjectiveSense::Maximize:ObjectiveSense::Minimize,n-4);exhaustive(m);}
  {Model m;m.minimize({},-3);exhaustive(m);m.add_row({},1,inf);exhaustive(m);}
  for(bool active:{false,true}){Model m;auto a=m.add_binary(),x=m.add_integer(-1,2);add_indicator(m,a,active,{{x,1}},0,1);m.maximize({{x,2},{a,-1}},-17);exhaustive(m);}
  {Model m;auto x=m.add_variable(VariableType::SemiInteger,2,3),y=m.add_integer(-1,1);m.add_row({{x,1},{y,1}},0,3);m.minimize({{x,1},{y,-2}},9);exhaustive(m);}
  {Model m;auto a=m.add_binary(),b=m.add_binary();add_all_different(m,{a,b});m.minimize({{a,1},{b,-3}});exhaustive(m);}
  {Model m;auto a=m.add_binary();add_all_different(m,{a,a});exhaustive(m);}
  {Model m;auto a=m.add_binary(),b=m.add_binary();add_table(m,{a,b},{{0,1},{1,0}});m.maximize({{a,2}});exhaustive(m);}
  {Model m;auto a=m.add_binary(),b=m.add_binary(),i=m.add_integer(-1,0),r=m.add_binary();add_element(m,i,{a,b},r,-1);m.minimize({{r,2},{a,-1}});exhaustive(m);}
  {Model m;auto a=m.add_binary(),b=m.add_binary();add_cumulative(m,{a,b,a},{1,1,0},{1,1,9},1);m.maximize({{a,1},{b,2}});exhaustive(m);}
  {Model m;std::vector<Variable> s;for(int i=0;i<3;++i)s.push_back(m.add_integer(0,2));add_circuit(m,s);m.minimize({{s[0],2},{s[1],-1}},5);exhaustive(m);}
  for(bool minimize:{false,true}){Model m;const auto x=m.add_binary(),y=m.add_binary(),z=m.add_binary();m.add_row({{x,3},{y,3},{z,1}},-inf,5);m.set_objective({{x,-2},{y,-3},{z,1}},minimize?ObjectiveSense::Minimize:ObjectiveSense::Maximize,-5);exhaustive(m);}
  for(int value:{-INT_MAX+1,INT_MAX-1,-(INT_MAX-1)/2,(INT_MAX-1)/2}){Model m;auto x=m.add_integer(value,value);
    if(std::abs(value)>(INT_MAX-1)/2)m.minimize({},1);else m.minimize({{x,1}},1);
    auto o=options();o.primal_start={{x,double(value)}};
    for(int route:{0,2,3,5}){auto r=solve(m.snapshot(),o,route);verify(m.snapshot(),r,route,o);assert(r.result.termination==Termination::Optimal && r.result.start_submitted);}
  }
}
void limits(){
  Model m;std::vector<Variable> vars;std::vector<Term> terms;for(int i=0;i<4;++i){vars.push_back(m.add_binary());terms.push_back({vars.back(),double(i+1)});}m.maximize(terms,-13);
  const auto source=m.snapshot();std::vector<double> zero(vars.size(),0);
  for(auto route:routes()){
    auto o=options();o.primal_start=entries(source,zero);const auto full=solve(source,o,route);verify(source,full,route,o);assert(full.result.start_submitted);
    const auto cap=is_frontier(route)?full.branching.budget_nodes+1:24;
    for(std::uint64_t n=0;n<=cap;++n){o.node_limit=n;const auto r=solve(source,o,route);verify(source,r,route,o);assert(r.result.termination==Termination::Optimal || r.result.termination==Termination::NodeLimit);assert(r.result.start_submitted==(n>0));}
    o.node_limit.reset();
    for(std::size_t n=0;n<4;++n)if(is_frontier(route)){const auto r=solve(source,o,route,n);verify(source,r,route,o);assert(r.result.start_submitted && r.result.has_solution() && r.frontier.peak_open_nodes<=n);}
    for(int kind=0;kind<3;++kind){auto stop=o;if(kind==0)stop.time_limit_seconds=0;else if(kind==1)stop.node_limit=0;else{stop.cancellation=std::make_shared<CancellationToken>();stop.cancellation->cancel();}auto r=solve(source,stop,route);verify(source,r,route,stop);assert(!r.result.start_submitted && !r.result.has_solution() && !r.lp.lp_calls);}
  }
}
void input_boundaries(){
  Model m;auto dead=m.add_binary();m.remove(dead);auto x=m.add_integer(-1,1),y=m.add_variable(VariableType::SemiInteger,2,3);m.minimize({{x,1},{y,1}},7);const auto source=m.snapshot();Model foreign;auto other=foreign.add_binary();
  for(auto route:routes()){
    auto o=options();o.primal_start={{x,0},{y,0}};
    const auto good=solve(source,o,route);verify(source,good,route,o);assert(good.result.start_submitted);
    for(int kind=0;kind<8;++kind){auto bad=o;
      if(kind==0)bad.primal_start.push_back({x,0});else if(kind==1)bad.primal_start.push_back({other,0});else if(kind==2)bad.primal_start.push_back({dead,0});else if(kind==3)bad.primal_start[0].variable.id=source.variables.size();else if(kind==4)bad.primal_start[0].value=1-1e-12;else if(kind==5)bad.primal_start[1].value=1;else if(kind==6)bad.primal_start[0].value=inf;else bad.primal_start[0].value=std::numeric_limits<double>::quiet_NaN();
      const auto r=solve(source,bad,route);assert(r.result.termination==Termination::InvalidModel && !r.result.start_submitted && !r.result.has_solution());++runs;
    }
    o.primal_start={{x,0}};const auto partial=solve(source,o,route);assert(partial.result.termination==Termination::Unsupported && !partial.result.start_submitted);++runs;
    auto malformed=source;malformed.variables[x.id].variable.model_id++;o.primal_start={{x,0},{y,0}};assert(solve(malformed,o,route).result.termination==Termination::InvalidModel);++runs;
    o.guarantee=Guarantee::Numerical;o.primal_start[0].value=1-1e-12;assert(solve(source,o,route).result.termination==Termination::InvalidModel);++runs;
    o.primal_start[0].value=0;const auto numerical=solve(source,o,route);verify(source,numerical,route,o);assert(numerical.result.start_submitted && numerical.result.termination==Termination::Optimal);
    o=options();o.guarantee=Guarantee::Certified;o.primal_start={{x,0},{y,0}};assert(solve(source,o,route).result.termination==Termination::Unsupported);++runs;
    Model fractional;auto v=fractional.add_binary();fractional.minimize({{v,.5}});o=options();o.primal_start={{v,0}};assert(solve(fractional.snapshot(),o,route).result.termination==Termination::Unsupported);++runs;
    Model overflow;v=overflow.add_integer(0,INT_MAX-1);overflow.minimize({{v,2}});o.primal_start={{v,0}};assert(solve(overflow.snapshot(),o,route).result.termination==Termination::Unsupported);++runs;
  }
  auto o=options();o.primal_start={{x,0},{y,0}};const auto old=solve_native(m,o);assert(old.start_submitted);
  m.maximize({{x,-2},{y,3}},-4);const auto revised=solve_native(m,o);verify(m.snapshot(),Answer{revised,{},{},{}},0,o);assert(revised.start_submitted && old.revision!=revised.revision && old.value(y)==0);
  m.set_bounds(x,1,1);assert(solve_native(m,o).termination==Termination::InvalidModel);
  Model moved(std::move(m));assert(solve_native(m,o).termination==Termination::InvalidModel);assert(solve_native_search(m).result.termination==Termination::InvalidModel);
  NativeLpOptions lp;lp.solve=o;assert(solve_native_lp(m,lp).result.termination==Termination::InvalidModel);
  o.primal_start={{x,1},{y,0}};std::vector<std::future<SolveResult>> tasks;auto snapshot=moved.snapshot();for(int n=0;n<4;++n)tasks.push_back(std::async(std::launch::async,[&]{return solve_native(snapshot,o);}));for(auto& task:tasks){const auto r=task.get();assert(r.start_submitted);verify(snapshot,Answer{r,{},{},{}},0,o);}
}
// Swap logical slots while retaining every structural identity and owned row/gate.
void reverse_indicators(ModelSnapshot& source){
  std::reverse(source.indicators.begin(),source.indicators.end());
  for(std::size_t i=0;i<source.indicators.size();++i){auto& d=source.indicators[i];d.indicator.id=i;
    if(d.inactive_gate)source.variables[d.inactive_gate->id].indicator_origin=d.indicator;
    for(auto row:d.generated_rows)source.rows[row.id].indicator_origin=d.indicator;
  }
}
void gates(){
  Model m;auto a=m.add_binary(),x=m.add_integer(0,3);auto first=add_indicator(m,a,true,{{x,1}},2,inf);assert(first.inactive_gate);auto second=add_indicator(m,*first.inactive_gate,true,{{x,1}},-inf,1);assert(second.inactive_gate);m.minimize({{x,-1}},9);
  auto source=m.snapshot();reverse_indicators(source);
  for(auto route:routes())for(bool active:{false,true}){
    auto o=options();o.primal_start={{a,active?1.:0.},{x,active?2.:1.}};const auto r=solve(source,o,route);verify(source,r,route,o);assert(r.result.start_submitted);
    o.primal_start.push_back({*first.inactive_gate,active?1.:0.});assert(solve(source,o,route).result.termination==Termination::InvalidModel);++runs;
  }
  // A structurally valid cyclic equation set has no guessed orientation.
  source=m.snapshot();auto& d=source.indicators[first.indicator.id];d.activator=*second.inactive_gate;
  for(auto row:d.generated_rows){auto& terms=source.rows[row.id].terms;for(auto& t:terms)if(t.variable==a)t.variable=*second.inactive_gate;std::sort(terms.begin(),terms.end(),[](const Term& l,const Term& r){return l.variable.id<r.variable.id;});}
  for(auto route:routes()){
    auto o=options();o.primal_start={{a,0},{x,1}};const auto partial=solve(source,o,route);assert(partial.result.termination==Termination::Unsupported && !partial.result.has_solution());++runs;
    o.primal_start.push_back({*first.inactive_gate,1});const auto seeded=solve(source,o,route);verify(source,seeded,route,o);assert(seeded.result.start_submitted);
  }
  remove_indicator(m,second.indicator);remove_indicator(m,first.indicator);
  m.add_row({{*first.inactive_gate,1}},1,1);m.minimize({{*second.inactive_gate,-1},{x,1}},-4);source=m.snapshot();
  for(auto route:routes()){
    auto o=options();o.primal_start={{a,1},{x,1}};assert(solve(source,o,route).result.termination==Termination::Unsupported);++runs;
    o.primal_start.push_back({*first.inactive_gate,1});o.primal_start.push_back({*second.inactive_gate,1});auto r=solve(source,o,route);verify(source,r,route,o);assert(r.result.start_submitted);
  }
}
#ifdef GECODE_NATIVE_START_TEST_HOOKS
Model fault_fixture(){Model m;auto a=m.add_binary(),b=m.add_binary(),x=m.add_integer(-1,2);m.add_row({{a,1},{b,1}},0,2);add_all_different(m,{a,b});add_indicator(m,a,true,{{x,1}},0,1);m.minimize({{a,2},{b,1},{x,1}},7);return m;}
void faults(){
  Model m=fault_fixture();const auto source=m.snapshot();auto base=options();base.primal_start={{source.variables[0].variable,0},{source.variables[1].variable,1},{source.variables[2].variable,2}};
  const std::vector<std::string> stages={"before_start","map_entry","gate_dependency","known_slot","derive_gate","start_completeness","before_exact_start","exact_variable","exact_term","exact_row","exact_indicator","exact_global","after_exact_global","exact_objective","after_exact_start","before_numerical_start","after_numerical_start","before_start_publication","after_start_publication","before_start_cutoff","after_start_cutoff","after_start_release"};
  for(auto route:routes())for(const auto& stage:stages)for(int mode=0;mode<3;++mode){
    auto o=base;o.cancellation=std::make_shared<CancellationToken>();bool fired=false,published=false;
    hook=[&](const char* raw){const std::string event=raw;if(event=="after_start_publication")published=true;if(!fired && event==stage){fired=true;if(mode==0)o.cancellation->cancel();else if(mode==1)throw std::bad_alloc();else throw std::runtime_error("injected native start failure");}};
    const auto r=solve(source,o,route);hook={};assert(fired);verify(source,r,route,o);
    assert(r.result.termination==(mode==0?Termination::Cancelled:mode==1?Termination::MemoryLimit:Termination::BackendError));
    assert(r.result.start_submitted==published && r.result.has_solution()==published);
    if(published)assert(*r.result.objective<=10);
    if(is_frontier(route) && stage!="after_start_release")assert(r.result.best_bound==6 && r.frontier.unresolved_regions==1 && !r.frontier.admitted_nodes);
    if(stage=="after_start_publication")assert(r.result.objective==10 && !r.lp.lp_calls);
  }
  // Every injected failure after the incumbent is published retains it.
  for(int route:{0,1,7})if(route==0 || native_lp_capabilities().available){
    for(const std::string stage:{"start_root_alloc","start_search_ready","before_start_search_next","before_start_candidate_validation","before_start_candidate_publication"}){
      auto o=base;o.cancellation=std::make_shared<CancellationToken>();bool fired=false;
      hook=[&](const char* event){if(!fired && stage==event){fired=true;o.cancellation->cancel();}};
      const auto r=solve(source,o,route);hook={};assert(fired);verify(source,r,route,o);assert(r.result.start_submitted && r.result.objective==10 && r.result.termination==Termination::Cancelled);
    }
  }
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
  for(int route:{2,3,5})for(const std::string stage:{"frontier:root_alloc","frontier:before_propagation","frontier:after_propagation","frontier:child_clone","frontier:child_committed","frontier:child_enqueued","frontier:before_validation","frontier:after_validation"})for(int mode=0;mode<3;++mode){
    auto o=base;o.cancellation=std::make_shared<CancellationToken>();bool fired=false;
    hook=[&](const char* event){if(!fired && stage==event){fired=true;if(mode==0)o.cancellation->cancel();else if(mode==1)throw std::bad_alloc();else throw std::runtime_error("injected frontier transfer failure");}};
    const auto r=solve(source,o,route);hook={};assert(fired);verify(source,r,route,o);assert(r.result.start_submitted && r.result.has_solution());assert(r.result.termination==(mode==0?Termination::Cancelled:mode==1?Termination::MemoryLimit:Termination::BackendError));
  }
#endif
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
  if(native_lp_capabilities().available)for(int route:{6,7})for(const std::string stage:{"root_cut:before_loop","root_cut:after_loop","root_cut:prepared"})for(int mode=0;mode<3;++mode){
    auto o=base;o.cancellation=std::make_shared<CancellationToken>();bool fired=false;
    hook=[&](const char* event){if(!fired && stage==event){fired=true;if(mode==0)o.cancellation->cancel();else if(mode==1)throw std::bad_alloc();else throw std::runtime_error("injected start LP setup failure");}};
    const auto r=solve(source,o,route);hook={};assert(fired);verify(source,r,route,o);assert(r.result.start_submitted && r.result.objective==10 && r.result.termination==(mode==0?Termination::Cancelled:mode==1?Termination::MemoryLimit:Termination::BackendError));
  }
#endif
  for(int route:{0,3})for(const std::string stage:{"before_start_publication","after_start_publication"}){
    auto o=base;o.time_limit_seconds=.5;bool fired=false;hook=[&](const char* event){if(!fired && stage==event){fired=true;std::this_thread::sleep_for(std::chrono::milliseconds(550));}};
    const auto r=solve(source,o,route);hook={};assert(fired);verify(source,r,route,o);assert(r.result.termination==Termination::TimeLimit && r.result.start_submitted==(stage=="after_start_publication"));
  }
  // No start invokes no start-specific work/hooks, including explicit empty input.
  for(auto route:routes()){
    auto o=options();std::uint64_t events=0;hook=[&](const char* event){const std::string e=event;if(e.find("frontier:")!=0 && e.find("root_cut:")!=0)++events;};
    const auto first=solve(source,o,route);o.primal_start={};const auto second=solve(source,o,route);hook={};verify(source,first,route,o);verify(source,second,route,o);assert(!events && first.result.objective==second.result.objective && first.frontier.admitted_nodes==second.frontier.admitted_nodes && first.lp.lp_calls==second.lp.lp_calls);
  }
}
#endif
}
namespace Gecode {namespace Optimize {
#ifdef GECODE_NATIVE_START_TEST_HOOKS
void native_start_test_event(const char* event){if(hook)hook(event);}
#endif
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
void native_search_test_event(const char* event){if(hook)hook((std::string("frontier:")+event).c_str());}
#endif
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
void native_root_cut_test_event(const char* event,NativeRootCoverCompletion&){if(hook)hook((std::string("root_cut:")+event).c_str());}
#endif
}}
int main(){
  if(!native_capabilities().available){Model m;auto x=m.add_binary();auto o=options();o.primal_start={{x,0}};assert(solve_native(m,o).termination==Termination::Unsupported);NativeSearchOptions search;search.solve=o;assert(solve_native_search(m,search).result.termination==Termination::Unsupported);NativeLpOptions lp;lp.solve=o;assert(solve_native_lp(m,lp).result.termination==Termination::Unsupported);o.primal_start[0].value=inf;assert(solve_native(m,o).termination==Termination::InvalidModel);std::cout<<"Native starts unavailable: explicit boundaries pass\n";return 0;}
  fixtures();limits();input_boundaries();gates();
#ifdef GECODE_NATIVE_START_TEST_HOOKS
  faults();
#endif
  std::cout<<runs<<" native start original-oracle/budget/fault configurations pass\n";
}
