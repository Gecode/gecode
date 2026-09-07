#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>

using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
std::uint64_t runs=0;

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
void evidence(const ModelSnapshot& source,const NativeSearchOptions& options,const NativeSearchResult& solved) {
  ++runs;const auto expected=oracle(source);const auto& result=solved.result;
  const bool minimize=source.objective.sense==ObjectiveSense::Minimize;
  assert(result.model_id==source.model_id && result.revision==source.revision);
  assert(result.guarantee==options.solve.guarantee && result.elapsed_seconds>=0);
  assert(solved.frontier.peak_open_nodes<=options.max_open_nodes);
  const auto& root=solved.relaxation.root_cover;
  assert(root.requested==bool(options.relaxation && options.relaxation->root_cover_cuts));
  assert(solved.relaxation.lp_calls>=root.lp_calls && solved.relaxation.valid_bounds>=root.valid_bounds);
  assert(solved.relaxation.rejected_bounds>=root.rejected_bounds && solved.relaxation.lp_seconds>=root.lp_seconds);
  if(options.solve.node_limit) assert(solved.frontier.admitted_nodes<=*options.solve.node_limit);
  if(result.has_solution()) {
    assert(expected && feasible(source,result.values));
    assert(result.objective==static_cast<double>(objective(source,result.values)));
    assert(minimize ? *result.objective>=*expected:*result.objective<=*expected);
    for(const auto& variable:source.variables) {
      assert(result.active_variables[variable.variable.id]==variable.active);
      if(!variable.active) assert(std::isnan(result.values[variable.variable.id]));
    }
  }
  if(result.best_bound) {
    assert(std::isfinite(*result.best_bound));
    if(expected) assert(minimize ? *result.best_bound<=*expected:*result.best_bound>=*expected);
    if(result.has_solution()) {
      assert(minimize ? *result.best_bound<=*result.objective:*result.best_bound>=*result.objective);
      assert(result.absolute_gap==std::fabs(*result.objective-*result.best_bound));
    }
  }
  if(result.termination==Termination::Optimal) {
    assert(expected && result.has_solution() && result.objective==static_cast<double>(*expected));
    assert(result.best_bound==result.objective && result.absolute_gap==0 && solved.frontier.unresolved_regions==0);
  } else if(result.termination==Termination::Infeasible) {
    assert(!expected && !result.has_solution() && !result.best_bound && solved.frontier.unresolved_regions==0);
  } else {
    assert(result.termination==Termination::NodeLimit || result.termination==Termination::MemoryLimit ||
           result.termination==Termination::Cancelled || result.termination==Termination::TimeLimit);
  }
}
void all_limits(const Model& model) {
  const auto source=model.snapshot();
  for(auto order:{NativeSearchOrder::DepthFirst,NativeSearchOrder::BestBound}) {
    NativeSearchOptions options;options.order=order;options.solve.guarantee=Guarantee::Exact;
    const auto full=solve_native_search(source,options);evidence(source,options,full);
    assert(full.result.termination==Termination::Optimal || full.result.termination==Termination::Infeasible);
    assert(full.relaxation.lp_calls==0);
    for(std::uint64_t nodes=0;nodes<=full.frontier.admitted_nodes+1;++nodes) {
      options.solve.node_limit=nodes;
      const auto limited=solve_native_search(source,options);evidence(source,options,limited);
      if(nodes<full.frontier.admitted_nodes) assert(limited.result.termination==Termination::NodeLimit);
    }
    options.solve.node_limit.reset();
    for(std::size_t cap=0;cap<=std::min<std::size_t>(5,full.frontier.peak_open_nodes+1);++cap) {
      options.max_open_nodes=cap;evidence(source,options,solve_native_search(source,options));
    }
  }
  assert(model.revision()==source.revision);
}
void ordinary_and_globals() {
  for(int i=0;i<48;++i) {
    Model model;const auto gone=model.add_binary();model.remove(gone);
    const auto x=model.add_integer(-2,2),y=model.add_integer(-1,2),b=model.add_binary();
    model.add_row({{x,double(i%5-2)},{y,double(i%3-1)},{b,2}},-2,3);
    model.add_row({{x,1},{y,-1}},-inf,double(i%4-2));
    model.set_objective({{x,double(i%7-3)},{y,2},{b,-3}},i%2?ObjectiveSense::Minimize:ObjectiveSense::Maximize,i-21);
    all_limits(model);
  }
  Model empty;empty.minimize({},-9);all_limits(empty);empty.add_row({},1,inf);all_limits(empty);
  Model ties;ties.add_integer(-2,2);ties.add_binary();all_limits(ties);
  Model fixed;const auto f=fixed.add_integer(-2,-2);fixed.maximize({{f,-3}},9007199254740900.0);all_limits(fixed);
  for(bool activation:{false,true}) {
    Model model;const auto b=model.add_binary(),x=model.add_integer(-1,2),s=model.add_variable(VariableType::SemiInteger,2,3);
    const auto indicator=add_indicator(model,b,activation,{{x,1},{s,1}},-inf,2);
    add_all_different(model,{x,s});model.minimize({{x,2},{s,-1},{*indicator.inactive_gate,1}},-8);all_limits(model);
  }
  Model element;auto i=element.add_integer(3,4),x=element.add_integer(-1,1),y=element.add_integer(0,2),r=element.add_integer(-1,2);
  add_element(element,i,{x,y},r,3);element.maximize({{r,2},{x,-1}},-2);all_limits(element);
  Model table;x=table.add_integer(-1,1);y=table.add_integer(0,2);
  add_table(table,{x,y},{{-1,2},{0,0},{1,1}});table.minimize({{x,2},{y,1}},5);all_limits(table);
  Model cumulative;x=cumulative.add_integer(-1,1);y=cumulative.add_integer(0,2);
  add_cumulative(cumulative,{x,y},{2,1},{1,1},1);cumulative.minimize({{x,1},{y,1}},-1);all_limits(cumulative);
  Model circuit;std::vector<Variable> successors;
  for(int j=0;j<3;++j) successors.push_back(circuit.add_integer(2,4));
  add_circuit(circuit,successors,2);circuit.minimize({{successors[0],1},{successors[1],-1}});all_limits(circuit);
}
void sibling_bound() {
  for(bool maximum:{false,true}) {
    Model model;const auto x=model.add_binary(),y=model.add_binary();
    model.set_objective({{x,maximum?-2.0:2.0},{y,maximum?-1.0:1.0}},
      maximum?ObjectiveSense::Maximize:ObjectiveSense::Minimize,maximum?17:-17);
    NativeSearchOptions options;options.order=NativeSearchOrder::DepthFirst;options.solve.node_limit=4;
    options.solve.guarantee=Guarantee::Exact;
    const auto solved=solve_native_search(model,options);evidence(model.snapshot(),options,solved);
    assert(solved.result.termination==Termination::NodeLimit && solved.result.has_solution());
    assert(solved.result.objective==(maximum?15:-15));
    assert(solved.result.best_bound==(maximum?17:-17));
    assert(solved.frontier.unresolved_regions>=2);
    auto historical=solved.result;model.set_bounds(x,1,1);
    assert(historical.revision!=model.revision() && historical.value(x)==1);
  }
}
void options_and_boundaries() {
  Model model;const auto x=model.add_binary(),y=model.add_binary();model.minimize({{x,2},{y,1}});
  NativeSearchOptions options;options.order=static_cast<NativeSearchOrder>(-1);
  assert(solve_native_search(model,options).result.termination==Termination::InvalidModel);
  options={};options.relaxation=NativeLpSettings{};options.relaxation->bound_change_interval=0;
  assert(solve_native_search(model,options).result.termination==Termination::InvalidModel);
  options={};options.relaxation=NativeLpSettings{};options.relaxation->root_cover_cuts=NativeRootCoverSettings{};
  options.relaxation->root_cover_cuts->denominator=3;
  assert(solve_native_search(model,options).result.termination==Termination::InvalidModel);
  options={};options.solve.time_limit_seconds=-1;
  assert(solve_native_search(model,options).result.termination==Termination::InvalidModel);
  auto bad=model.snapshot();bad.variables[0].variable.model_id++;
  assert(solve_native_search(bad).result.termination==Termination::InvalidModel);
  if(!native_capabilities().available) {
    options={};options.solve.guarantee=Guarantee::Exact;
    const auto missing=solve_native_search(model,options);
    assert(missing.result.termination==Termination::Unsupported && !missing.result.has_solution() && !missing.result.best_bound);
    assert(missing.result.guarantee==Guarantee::Exact && missing.frontier.admitted_nodes==0);
    assert(solve_native_search(model.snapshot(),options).result.termination==Termination::Unsupported);return;
  }
  options={};options.solve.time_limit_seconds=0;
  auto solved=solve_native_search(model,options);evidence(model.snapshot(),options,solved);
  assert(solved.result.termination==Termination::TimeLimit && !solved.result.has_solution());
  options={};options.solve.cancellation=std::make_shared<CancellationToken>();options.solve.cancellation->cancel();
  solved=solve_native_search(model,options);evidence(model.snapshot(),options,solved);
  assert(solved.result.termination==Termination::Cancelled && !solved.result.has_solution());
  options={};options.solve.guarantee=Guarantee::Certified;
  assert(solve_native_search(model,options).result.termination==Termination::Unsupported);
  options={};options.solve.backend=Backend::Highs;
  assert(solve_native_search(model,options).result.termination==Termination::Unsupported);
  options={};options.solve.threads=2;
  assert(solve_native_search(model,options).result.termination==Termination::Unsupported);
  options={};options.solve.primal_start={{x,1}};
  assert(solve_native_search(model,options).result.termination==Termination::Unsupported);
  Model continuous;continuous.add_continuous(0,1);
  assert(solve_native_search(continuous).result.termination==Termination::Unsupported);
  Model fractional;const auto z=fractional.add_integer(0,1);fractional.minimize({{z,.5}});
  assert(solve_native_search(fractional).result.termination==Termination::Unsupported);
  options={};options.relaxation=NativeLpSettings{};
  if(!native_lp_capabilities().available)
    assert(solve_native_search(model,options).result.termination==Termination::Unsupported);
  // Existing explicit solver behavior and native BAB limit contract are unchanged.
  assert(solve_native(model).backend=="Gecode native");
  SolveOptions old;old.node_limit=1;
  assert(!solve_native(model,old).best_bound);
  NativeLpOptions old_lp;old_lp.frequency=NativeLpFrequency::AfterBoundChanges;
  old_lp.bound_change_interval=3;old_lp.bound_tightening=false;old_lp.validate();
}
void with_lp() {
  if(!native_lp_capabilities().available) return;
  Model model;const auto x=model.add_integer(0,5),y=model.add_integer(-1,5);
  model.add_row({{x,2},{y,3}},7,inf);model.minimize({{x,1},{y,1}},-6);
  std::uint64_t calls=0;
  for(auto frequency:{NativeLpFrequency::Root,NativeLpFrequency::AfterBoundChanges})
    for(auto order:{NativeSearchOrder::DepthFirst,NativeSearchOrder::BestBound}) for(bool covers:{false,true}) {
      NativeSearchOptions options;options.solve.guarantee=Guarantee::Exact;options.order=order;
      options.relaxation=NativeLpSettings{};options.relaxation->frequency=frequency;
      if(covers)options.relaxation->root_cover_cuts=NativeRootCoverSettings{};
      auto result=solve_native_search(model,options);evidence(model.snapshot(),options,result);
      calls+=result.relaxation.lp_calls;
      for(std::uint64_t limit=0;limit<=result.frontier.admitted_nodes+1;++limit) {
        options.solve.node_limit=limit;evidence(model.snapshot(),options,solve_native_search(model,options));
      }
    }
  assert(calls>0);
}

void root_covers() {
  if(!native_lp_capabilities().available)return;
  for(bool maximum:{false,true})for(bool activation:{false,true}) {
    Model model;const auto gone=model.add_binary();model.remove(gone);
    const auto x=model.add_binary(),y=model.add_binary(),b=model.add_binary();
    const auto s=model.add_variable(VariableType::SemiInteger,2,3);
    model.add_row({{x,3},{y,3}},-inf,5);add_all_different(model,{x,y});
    add_indicator(model,b,activation,{{s,1},{x,1}},2,inf);
    model.set_objective({{x,maximum?2.0:-2.0},{y,maximum?2.0:-2.0},{s,maximum?-1.0:1.0}},
      maximum?ObjectiveSense::Maximize:ObjectiveSense::Minimize,maximum?-17:17);
    const auto source=model.snapshot();
    for(auto order:{NativeSearchOrder::DepthFirst,NativeSearchOrder::BestBound})for(unsigned rounds:{0U,1U,4U}) {
      NativeSearchOptions options;options.solve.guarantee=Guarantee::Exact;options.order=order;
      options.relaxation=NativeLpSettings{};options.relaxation->root_cover_cuts=NativeRootCoverSettings{};
      options.relaxation->root_cover_cuts->max_rounds=rounds;
      const auto full=solve_native_search(model,options);evidence(source,options,full);
      assert(full.result.termination==Termination::Optimal);
      assert(full.relaxation.root_cover.cuts==(rounds?1U:0U));
      for(std::uint64_t nodes=0;nodes<=full.frontier.admitted_nodes+1;++nodes) {
        options.solve.node_limit=nodes;const auto limited=solve_native_search(model,options);evidence(source,options,limited);
        if(!nodes)assert(!limited.relaxation.root_cover.lp_calls);
      }
      options.solve.node_limit.reset();
      for(std::size_t cap=0;cap<=2;++cap) {
        options.max_open_nodes=cap;evidence(source,options,solve_native_search(model,options));
      }
    }
  }
  Model simple;auto x=simple.add_binary(),y=simple.add_binary();
  simple.add_row({{x,3},{y,3}},-inf,5);simple.minimize({{x,-2},{y,-2}},17);
  for(unsigned kind=0;kind<6;++kind) {
    NativeSearchOptions options;options.solve.guarantee=Guarantee::Exact;
    options.relaxation=NativeLpSettings{};options.relaxation->root_cover_cuts=NativeRootCoverSettings{};
    auto& limits=*options.relaxation->root_cover_cuts;
    if(kind==0)limits.max_work=0;
    if(kind==1)limits.max_cuts=0;
    if(kind==2)limits.max_cut_nonzeros=1;
    if(kind==3)limits.max_model_rows=1;
    if(kind==4)limits.max_separation_rows=0;
    if(kind==5)limits.max_terms_per_row=1;
    const auto solved=solve_native_search(simple,options);evidence(simple.snapshot(),options,solved);
    assert(solved.result.termination==Termination::Optimal && solved.result.objective==15);
    assert(!solved.relaxation.root_cover.cuts);
  }
}

#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
std::string cut_event;
std::optional<NativeRootCoverCompletion> cut_completion;
std::shared_ptr<CancellationToken> cut_cancellation;
int cut_exception=0;
void root_cut_failures() {
  if(!native_lp_capabilities().available)return;
  Model model;const auto x=model.add_binary(),y=model.add_binary();
  model.add_row({{x,3},{y,3}},-inf,5);model.minimize({{x,-2},{y,-2}},17);
  NativeSearchOptions options;options.solve.guarantee=Guarantee::Exact;
  options.relaxation=NativeLpSettings{};options.relaxation->root_cover_cuts=NativeRootCoverSettings{};
  for(auto completion:{NativeRootCoverCompletion::NoPrimalSuggestion,NativeRootCoverCompletion::InvalidSuggestion,
      NativeRootCoverCompletion::CallbackError,NativeRootCoverCompletion::BackendError,NativeRootCoverCompletion::AllocationFailure}) {
    cut_event="after_loop";cut_completion=completion;
    const auto solved=solve_native_search(model,options);++runs;
    const bool hint=completion==NativeRootCoverCompletion::NoPrimalSuggestion || completion==NativeRootCoverCompletion::InvalidSuggestion;
    assert(solved.result.termination==(hint?Termination::Optimal:
      completion==NativeRootCoverCompletion::AllocationFailure?Termination::MemoryLimit:Termination::BackendError));
    assert(solved.result.guarantee==Guarantee::Exact && solved.result.has_solution()==hint);
    assert(solved.result.best_bound && *solved.result.best_bound<=15);
    if(hint)assert(solved.result.objective==15 && feasible(model.snapshot(),solved.result.values));
    else assert(!solved.result.objective && solved.frontier.admitted_nodes==0 && solved.result.best_bound==13);
    assert(solved.relaxation.root_cover.completion==completion);
    assert(solved.relaxation.lp_calls>=solved.relaxation.root_cover.lp_calls);
  }
  cut_completion.reset();
  for(const auto* event:{"before_loop","after_loop","prepared"})for(int mode=0;mode<3;++mode) {
    cut_event=event;cut_exception=mode;cut_cancellation=std::make_shared<CancellationToken>();
    options.solve.cancellation=cut_cancellation;
    const auto solved=solve_native_search(model,options);++runs;
    assert(solved.result.termination==(mode==0?Termination::Cancelled:mode==1?Termination::MemoryLimit:Termination::BackendError));
    assert(!solved.result.has_solution() && solved.result.best_bound==13);
    assert(solved.frontier.admitted_nodes==0 && solved.result.guarantee==Guarantee::Exact);
    assert(solved.relaxation.root_cover.requested);
  }
  cut_event.clear();cut_cancellation.reset();cut_exception=0;
}
#endif
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
std::string selected_event;
unsigned selected_occurrence=0,event_occurrences=0;
bool allocation_failure=false;
std::shared_ptr<CancellationToken> cancellation;
std::map<std::string,unsigned> event_counts;
void knapsack_interruptions() {
  Model model;
  const auto x = model.add_binary(), y = model.add_binary();
  model.add_row({{x,2},{y,2}},-inf,3);
  model.minimize({{x,-3},{y,-2}},11);
  NativeSearchOptions options;
  options.solve.guarantee = Guarantee::Exact;
  assert(oracle(model.snapshot()) == 8);
  for (bool fail : {false,true}) {
    selected_event = "knapsack_row_completed";
    selected_occurrence = 1;
    event_occurrences = 0;
    allocation_failure = fail;
    cancellation = std::make_shared<CancellationToken>();
    options.solve.cancellation = cancellation;
    const auto solved = solve_native_search(model,options);
    assert(event_occurrences == 1);
    assert(solved.result.termination ==
           (fail ? Termination::MemoryLimit : Termination::Cancelled));
    assert(!solved.result.has_solution());
    // The first table row cannot publish a partial DP bound. The original
    // objective box still represents the whole unconstructed root region.
    assert(solved.result.best_bound == 6);
    assert(solved.frontier.admitted_nodes == 0);
    assert(solved.frontier.unresolved_regions == 1);
    evidence(model.snapshot(),options,solved);
  }
  selected_event.clear();
  cancellation.reset();
}
void fault_injections() {
  Model model;const auto x=model.add_binary(),y=model.add_binary();model.minimize({{x,2},{y,1}},-17);
  NativeSearchOptions options;options.order=NativeSearchOrder::DepthFirst;options.solve.guarantee=Guarantee::Exact;
  event_counts.clear();
  solve_native_search(model,options);
  const auto counts=event_counts;
  for(const auto& event:counts) for(unsigned occurrence=1;occurrence<=std::min(2U,event.second);++occurrence)
    for(bool fail:{false,true}) {
      selected_event=event.first;selected_occurrence=occurrence;event_occurrences=0;allocation_failure=fail;
      cancellation=std::make_shared<CancellationToken>();options.solve.cancellation=cancellation;
      const auto result=solve_native_search(model,options);
      assert(event_occurrences==occurrence);
      assert(result.result.termination==(fail?Termination::MemoryLimit:Termination::Cancelled));
      evidence(model.snapshot(),options,result);
      if(event.first=="after_validation" && occurrence==1) assert(!result.result.has_solution());
      if(event.first=="after_validation" && occurrence==2) assert(result.result.objective==-15);
    }
  selected_event.clear();cancellation.reset();
}
#endif
}
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
namespace Gecode { namespace Optimize {
void native_root_cut_test_event(const char* event,NativeRootCoverCompletion& completion) {
  if(cut_event!=event)return;
  if(cut_completion){completion=*cut_completion;return;}
  if(cut_exception==1)throw std::bad_alloc();
  if(cut_exception==2)throw std::runtime_error("injected frontier root preparation failure");
  cut_cancellation->cancel();
}
}}
#endif
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
namespace Gecode { namespace Optimize {
void native_search_test_event(const char* event) {
  ++event_counts[event];
  if(selected_event!=event || ++event_occurrences!=selected_occurrence) return;
  if(allocation_failure) throw std::bad_alloc();
  cancellation->cancel();
}
}}
#endif
int main() {
  options_and_boundaries();
  if(!native_capabilities().available) {std::cout<<"Native frontier unavailable: explicit boundaries pass\n";return 0;}
  ordinary_and_globals();sibling_bound();with_lp();root_covers();
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
  root_cut_failures();
#endif
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
  knapsack_interruptions();fault_injections();
#else
  // Production builds have no shared coordinator hook or search state.
  const auto isolated=[] {
    Model model;const auto x=model.add_integer(-1,3),y=model.add_integer(0,3);
    model.add_row({{x,2},{y,3}},5,inf);model.minimize({{x,1},{y,1}},-3);
    NativeSearchOptions options;options.solve.guarantee=Guarantee::Exact;
    if(native_lp_capabilities().available) options.relaxation=NativeLpSettings{};
    return solve_native_search(model,options);
  };
  auto left=std::async(std::launch::async,isolated),right=std::async(std::launch::async,isolated);
  const auto a=left.get(),b=right.get();
  assert(a.result.termination==Termination::Optimal && a.result.objective==-1);
  assert(b.result.objective==a.result.objective && a.result.model_id!=b.result.model_id);
#endif
  std::cout<<runs<<" native frontier solve/budget/failure configurations pass\n";
}
