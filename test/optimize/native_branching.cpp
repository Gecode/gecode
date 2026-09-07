#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <set>
#include <string>
#include <type_traits>
#include <thread>
#include <vector>
using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
std::uint64_t runs=0;
thread_local std::function<void(const char*,std::size_t,double&,double&)> hook;
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
}
std::optional<std::int64_t> oracle(const ModelSnapshot& model) {
  std::vector<double> point(model.variables.size());std::optional<std::int64_t> best;
  const auto visit=[&](const auto& self,std::size_t slot)->void {
    if(slot==model.variables.size()) {
      if(!feasible(model,point)) return;
      const auto value=objective(model,point);
      if(!best || (model.objective.sense==ObjectiveSense::Minimize ? value<*best:value>*best)) best=value;
      return;
    }
    const auto& variable=model.variables[slot];
    if(!variable.active) {self(self,slot+1);return;}
    if(variable.type==VariableType::SemiInteger) {point[slot]=0;self(self,slot+1);}
    for(int value=static_cast<int>(variable.lower);value<=variable.upper;++value) {point[slot]=value;self(self,slot+1);}
  };
  visit(visit,0);return best;
}

void verify(const ModelSnapshot& source,const NativeSearchOptions& options,const NativeSearchResult& answer,bool operational=false) {
  ++runs;const auto best=oracle(source);const auto& r=answer.result;const auto& b=answer.branching;
  const bool min=source.objective.sense==ObjectiveSense::Minimize;
  assert(r.model_id==source.model_id && r.revision==source.revision && r.guarantee==options.solve.guarantee);
  assert(b.requested==bool(options.branching));
  assert(b.budget_nodes==answer.frontier.admitted_nodes+b.probe_status_calls);
  assert(b.manual_splits+b.fallback_decisions<=b.decisions);
  assert(b.finite_samples==2*b.published_pairs && b.zero_gain_samples<=b.finite_samples);
  assert(b.published_pairs<=b.completed_pairs && b.completed_pairs*2<=b.probe_status_calls);
  assert(b.failed_directions<=b.completed_pairs*2 && b.probe_lp_calls<=answer.relaxation.lp_calls);
  assert(b.probe_lp_seconds<=answer.relaxation.lp_seconds+1e-9);
  assert(answer.frontier.peak_open_nodes<=options.max_open_nodes);
  if(options.solve.node_limit)assert(b.budget_nodes<=*options.solve.node_limit);
  if(options.branching){assert(b.work<=options.branching->max_branching_work);assert(b.history_entries<=options.branching->max_history_entries);assert(b.probe_status_calls<=options.branching->max_probe_status_calls);}
  else assert(!b.decisions && !b.probe_status_calls && !b.history_entries && !b.work);
  if(r.has_solution()) {
    assert(best && feasible(source,r.values) && r.objective==static_cast<double>(objective(source,r.values)));
    assert(min ? *r.objective>=*best:*r.objective<=*best);
    for(const auto& v:source.variables) {assert(r.active_variables[v.variable.id]==v.active);if(!v.active)assert(std::isnan(r.values[v.variable.id]));}
  }
  if(r.best_bound) {
    assert(std::isfinite(*r.best_bound));
    if(best)assert(min ? *r.best_bound<=*best:*r.best_bound>=*best);
    if(r.has_solution())assert(min ? *r.best_bound<=*r.objective:*r.best_bound>=*r.objective);
  }
  if(r.termination==Termination::Optimal)assert(best && r.has_solution() && *r.objective==*best && r.best_bound==r.objective && !answer.frontier.unresolved_regions);
  else if(r.termination==Termination::Infeasible)assert(!best && !r.has_solution() && !r.best_bound && !answer.frontier.unresolved_regions);
  else assert(r.termination==Termination::NodeLimit || r.termination==Termination::MemoryLimit || r.termination==Termination::Cancelled || r.termination==Termination::TimeLimit || (operational && r.termination==Termination::BackendError));
}
NativeSearchOptions settings(){NativeSearchOptions o;o.solve.guarantee=Guarantee::Exact;o.branching=NativeBranchingSettings{};return o;}
void limits(const Model& model,bool lp=false) {
  const auto source=model.snapshot();
  for(auto order:{NativeSearchOrder::DepthFirst,NativeSearchOrder::BestBound}) {
    auto o=settings();o.order=order;
    if(lp){o.relaxation=NativeLpSettings{};o.relaxation->frequency=NativeLpFrequency::AfterBoundChanges;o.relaxation->root_cover_cuts=NativeRootCoverSettings{};}
    const auto full=solve_native_search(source,o);verify(source,o,full);
    assert(full.result.termination==Termination::Optimal || full.result.termination==Termination::Infeasible);
    for(std::uint64_t n=0;n<=full.branching.budget_nodes+1;++n){o.solve.node_limit=n;verify(source,o,solve_native_search(source,o));}
    o.solve.node_limit.reset();
    for(std::size_t n=0;n<=3;++n){o.max_open_nodes=n;verify(source,o,solve_native_search(source,o));}
    o.max_open_nodes=100000;
    for(unsigned n=0;n<11;++n){o.branching=NativeBranchingSettings{};auto& s=*o.branching;
      switch(n){case 0:s.max_candidates_per_decision=0;break;case 1:s.max_probe_status_calls=0;break;case 2:s.max_probe_status_calls=1;break;case 3:s.max_probe_status_calls=2;break;case 4:s.max_probe_status_calls_per_decision=1;break;case 5:s.max_branching_work=0;break;case 6:s.max_history_entries=0;break;case 7:s.max_history_entries=1;break;case 8:s.max_nonimproving_pairs=0;break;case 9:s.max_candidates_per_decision=1;break;case 10:s.reliability_samples=1;break;}
      const auto result=solve_native_search(source,o);verify(source,o,result);
      assert(result.result.termination==Termination::Optimal || result.result.termination==Termination::Infeasible);
    }
  }
  assert(model.revision()==source.revision);
}
void fixtures() {
  for(int i=0;i<24;++i){Model m;auto dead=m.add_binary();m.remove(dead);const auto a=m.add_binary(),b=m.add_binary(),c=m.add_binary();
    m.add_row({{a,double(i%5-2)},{b,2},{c,-1}},-1,double(i%3));
    m.set_objective({{a,double(i%4-2)},{b,-3},{c,2}},i%2?ObjectiveSense::Minimize:ObjectiveSense::Maximize,i-11);limits(m);}
  Model empty;empty.minimize({},-7);limits(empty);empty.add_row({},1,inf);limits(empty);
  for(bool min:{false,true})for(bool activation:{false,true}){
    Model m;const auto a=m.add_binary(),b=m.add_binary(),c=m.add_binary(),s=m.add_variable(VariableType::SemiInteger,2,3);
    m.add_row({{a,3},{b,3},{c,1}},-inf,5);add_all_different(m,{a,b});add_indicator(m,c,activation,{{s,1},{a,-1}},2,3);
    m.set_objective({{a,-2},{b,-1},{c,1},{s,1}},min?ObjectiveSense::Minimize:ObjectiveSense::Maximize,17);limits(m,native_lp_capabilities().available);
  }
  {Model m;const auto a=m.add_binary(),b=m.add_binary();add_table(m,{a,b},{{0,0},{1,1}});m.minimize({{a,1},{b,-2}});limits(m);}
  {Model m;const auto a=m.add_binary(),b=m.add_binary(),i=m.add_integer(-1,0),r=m.add_binary();add_element(m,i,{a,b},r,-1);m.maximize({{r,2},{a,-1}});limits(m);}
  {Model m;const auto a=m.add_binary(),b=m.add_binary();add_cumulative(m,{a,b},{1,1},{1,1},1);m.minimize({{a,2},{b,-1}});limits(m);}
  {Model m;const auto a=m.add_integer(0,2),b=m.add_integer(0,2),c=m.add_integer(0,2),z=m.add_binary();add_circuit(m,{a,b,c},0);m.add_row({{a,1},{z,-1}},0,1);m.minimize({{a,1},{z,-2}});limits(m);}
  {Model m;const auto x=m.add_variable(VariableType::SemiInteger,3,4),y=m.add_integer(-2,2);m.add_row({{x,1},{y,1}},1,3);m.minimize({{x,1},{y,-2}});limits(m);}
}
Model rank_fixture(){Model m;const auto a=m.add_binary(),b=m.add_binary(),c=m.add_binary();m.minimize({{a,1},{b,4},{c,2}},17);return m;}
void boundaries() {
  Model m=rank_fixture();const auto source=m.snapshot();auto o=settings();
  auto bad=o;bad.branching->reliability_samples=0;assert(solve_native_search(source,bad).result.termination==Termination::InvalidModel);
  bad=o;bad.branching->policy=static_cast<NativeBranchingPolicy>(99);assert(solve_native_search(source,bad).result.termination==Termination::InvalidModel);
  for(int kind=0;kind<3;++kind){auto z=o;if(kind==0)z.solve.node_limit=0;else if(kind==1)z.solve.time_limit_seconds=0;else{z.solve.cancellation=std::make_shared<CancellationToken>();z.solve.cancellation->cancel();}auto result=solve_native_search(source,z);verify(source,z,result);assert(!result.branching.decisions && !result.branching.probe_status_calls && !result.result.has_solution());}
  auto full=solve_native_search(source,o);verify(source,o,full);assert(full.branching.probe_status_calls && full.branching.manual_splits && full.branching.published_pairs && full.branching.reliable_candidates);
  for(std::size_t work=0;work<110;++work){auto limited=o;limited.branching->max_branching_work=work;auto r=solve_native_search(source,limited);verify(source,limited,r);assert(r.result.objective==17);}
  for(std::uint64_t n=1;n<5;++n){auto limited=o;limited.solve.node_limit=n;auto r=solve_native_search(source,limited);verify(source,limited,r);assert(!r.branching.probe_status_calls);}
  auto baseline=o;baseline.branching.reset();const auto base=solve_native_search(source,baseline);verify(source,baseline,base);
  auto zero=o;zero.branching->max_branching_work=0;const auto skipped=solve_native_search(source,zero);verify(source,zero,skipped);
  assert(base.frontier.admitted_nodes==skipped.frontier.admitted_nodes && base.frontier.expanded_nodes==skipped.frontier.expanded_nodes && !skipped.branching.manual_splits);
  std::vector<std::future<NativeSearchResult>> tasks;for(int i=0;i<4;++i)tasks.push_back(std::async(std::launch::async,[&]{return solve_native_search(source,o);}));
  for(auto& task:tasks){auto r=task.get();verify(source,o,r);assert(r.branching.probe_status_calls==full.branching.probe_status_calls && r.branching.manual_splits==full.branching.manual_splits);}
  m.set_bounds(m.snapshot().variables[0].variable,1,1);auto edited=m.snapshot();verify(edited,o,solve_native_search(edited,o));verify(source,o,full);
  if(native_lp_capabilities().available){Model lp;auto a=lp.add_binary(),b=lp.add_binary(),c=lp.add_binary();lp.add_row({{a,3},{b,3}},-inf,5);lp.minimize({{a,-2},{b,-2},{c,1}},17);
    for(bool covers:{false,true}){auto opts=settings();opts.relaxation=NativeLpSettings{};opts.relaxation->frequency=NativeLpFrequency::AfterBoundChanges;if(covers)opts.relaxation->root_cover_cuts=NativeRootCoverSettings{};const auto snapshot=lp.snapshot();const auto r=solve_native_search(snapshot,opts);verify(snapshot,opts,r);assert(r.result.objective==15 && r.branching.probe_lp_calls>0);}
  }
  // Two contradictory clause pairs only fail after probing/branching.
  {Model impossible;auto x=impossible.add_binary(),a=impossible.add_binary(),b=impossible.add_binary();
    impossible.add_row({{x,1},{a,1}},1,inf);impossible.add_row({{x,1},{a,-1}},0,inf);
    impossible.add_row({{x,-1},{b,1}},0,inf);impossible.add_row({{x,-1},{b,-1}},-1,inf);
    auto options=settings();options.branching->max_candidates_per_decision=1;const auto snapshot=impossible.snapshot();const auto r=solve_native_search(snapshot,options);verify(snapshot,options,r);
    assert(r.result.termination==Termination::Infeasible && r.branching.failed_directions==2 && r.branching.completed_pairs==1 && !r.branching.published_pairs && !r.branching.finite_samples);
  }
}
#ifdef GECODE_NATIVE_BRANCHING_TEST_HOOKS
void faults_and_traces() {
  Model m=rank_fixture();const auto source=m.snapshot();
  const std::vector<std::string> events={"candidate_scan","pair_begin","probe_clone","probe_cloned","probe_posted","probe_before_status","probe_after_status","probe_destroyed","pair_before_publication","pair_published","candidate_score","branch_selected"};
  for(const auto& event:events)for(int fault=0;fault<3;++fault){
    auto o=settings();o.solve.cancellation=std::make_shared<CancellationToken>();bool fired=false;
    hook=[&](const char* name,std::size_t,double&,double&){if(!fired && name==event){fired=true;if(fault==0)o.solve.cancellation->cancel();else if(fault==1)throw std::bad_alloc();else throw std::runtime_error("probe injected backend error");}};
    const auto r=solve_native_search(source,o);hook={};assert(fired);verify(source,o,r,true);
    assert(r.result.termination==(fault==0?Termination::Cancelled:fault==1?Termination::MemoryLimit:Termination::BackendError));
    assert(!r.result.has_solution() && r.result.best_bound==17 && r.frontier.unresolved_regions==1);
    if(event=="pair_before_publication")assert(!r.branching.published_pairs && !r.branching.finite_samples && !r.branching.history_entries);
  }
  // Interrupt in the second direction: no half-pair history, no proof reuse.
  {auto o=settings();o.solve.cancellation=std::make_shared<CancellationToken>();int statuses=0;hook=[&](const char* e,std::size_t,double&,double&){if(std::string(e)=="probe_after_status" && ++statuses==2)o.solve.cancellation->cancel();};auto r=solve_native_search(source,o);hook={};verify(source,o,r);assert(r.branching.probe_status_calls==2 && !r.branching.published_pairs && r.result.best_bound==17 && !r.result.has_solution());}
  {auto o=settings();o.solve.time_limit_seconds=1;bool fired=false;hook=[&](const char* e,std::size_t,double&,double&){if(!fired && std::string(e)=="pair_before_publication"){fired=true;std::this_thread::sleep_for(std::chrono::milliseconds(1050));}};auto r=solve_native_search(source,o);hook={};verify(source,o,r);assert(fired && r.result.termination==Termination::TimeLimit && !r.branching.published_pairs && r.result.best_bound==17 && !r.result.has_solution());}
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
  const std::vector<std::string> transfers={"child_clone","child_committed","before_propagation","after_propagation","child_enqueued","parent_discharged","before_validation","after_validation"};
  for(const auto& event:transfers)for(int fault=0;fault<3;++fault){
    auto o=settings();o.solve.cancellation=std::make_shared<CancellationToken>();bool armed=false,fired=false;
    hook=[&](const char* name,std::size_t,double&,double&){if(std::string(name)=="branch_selected")armed=true;if(armed && !fired && name==event){fired=true;if(fault==0)o.solve.cancellation->cancel();else if(fault==1)throw std::bad_alloc();else throw std::runtime_error("manual transfer injected failure");}};
    const auto r=solve_native_search(source,o);hook={};assert(armed && fired);verify(source,o,r,true);
    assert(r.result.termination==(fault==0?Termination::Cancelled:fault==1?Termination::MemoryLimit:Termination::BackendError));
    assert(!r.result.has_solution() && r.result.best_bound==17);
  }
  {auto o=settings();o.solve.cancellation=std::make_shared<CancellationToken>();int children=0;hook=[&](const char* e,std::size_t,double&,double&){if(std::string(e)=="child_clone" && ++children==2)o.solve.cancellation->cancel();};auto r=solve_native_search(source,o);hook={};verify(source,o,r);assert(children==2 && r.frontier.unresolved_regions==2 && r.result.best_bound==17 && !r.result.has_solution());}
#endif
  // Scores can change traversal, never feasibility/bounds. Nonfinite scores fall back.
  for(int mode=0;mode<4;++mode){auto o=settings();std::vector<std::size_t> selected;
    hook=[&](const char* e,std::size_t slot,double& down,double& up){if(std::string(e)=="candidate_score"){if(mode==0)down=up=0;else if(mode==1)down=up=std::numeric_limits<double>::quiet_NaN();else if(mode==2)down=up=slot==0?1e300:1;else down=up=-1;}if(std::string(e)=="branch_selected")selected.push_back(slot);};
    auto r=solve_native_search(source,o);hook={};verify(source,o,r);assert(r.result.objective==17);if(mode==2)assert(!selected.empty() && selected.front()==0);else assert(selected.empty());
  }
  // Repeated clones followed by built-in fallback respect Choice lifecycle.
  {auto o=settings();std::size_t cloned=0,fallback=0;hook=[&](const char* e,std::size_t,double& d,double& u){const std::string name=e;if(name=="probe_cloned")++cloned;if(name=="candidate_score")d=u=0;if(name=="branch_fallback")++fallback;};auto r=solve_native_search(source,o);hook={};verify(source,o,r);assert(cloned && fallback && !r.branching.manual_splits);}
  // Gate columns never enter the candidate panel, even when declared Binary.
  {Model g;auto a=g.add_binary(),x=g.add_integer(-1,2);auto formulated=add_indicator(g,a,true,{{x,1}},0,1);(void)formulated;auto snapshot=g.snapshot();assert(snapshot.indicators[0].inactive_gate);const auto gate=snapshot.indicators[0].inactive_gate->id;
    hook=[&](const char* e,std::size_t slot,double&,double&){if(std::string(e)=="pair_begin" || std::string(e)=="branch_selected")assert(slot!=gate);};auto o=settings();auto r=solve_native_search(snapshot,o);verify(snapshot,o,r);
    remove_indicator(g,formulated.indicator);g.minimize({{*formulated.inactive_gate,-9},{a,1}});snapshot=g.snapshot();assert(snapshot.variables[gate].indicator_origin);r=solve_native_search(snapshot,o);hook={};verify(snapshot,o,r);}
}
#endif
}
namespace Gecode {namespace Optimize {
#ifdef GECODE_NATIVE_BRANCHING_TEST_HOOKS
void native_branching_test_event(const char* e,std::size_t slot,double& down,double& up){if(hook)hook(e,slot,down,up);}
#endif
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
void native_search_test_event(const char* e) {double down=0,up=0;if(hook)hook(e,std::numeric_limits<std::size_t>::max(),down,up);}
#endif
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
void native_root_cut_test_event(const char*,NativeRootCoverCompletion&) {}
#endif
}}
int main(){
  if(!native_capabilities().available){Model m;auto o=settings();auto r=solve_native_search(m,o);assert(r.result.termination==Termination::Unsupported && r.branching.requested);o.branching->reliability_samples=0;assert(solve_native_search(m,o).result.termination==Termination::InvalidModel);std::cout<<"Native branching unavailable: explicit boundaries pass\n";return 0;}
  fixtures();boundaries();
#ifdef GECODE_NATIVE_BRANCHING_TEST_HOOKS
  faults_and_traces();
#endif
  std::cout<<runs<<" binary reliability original-oracle/budget/fault configurations pass\n";
}
