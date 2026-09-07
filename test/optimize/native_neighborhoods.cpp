#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native_neighborhoods.hpp>
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
thread_local std::function<void(const char*,std::size_t,int,int,std::vector<double>*)> hook;
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

NativeNeighborhoodOptions options() {
  NativeNeighborhoodOptions out;
  out.search.solve.backend=Backend::Native;out.search.solve.guarantee=Guarantee::Exact;
  out.neighborhood.time_limit_seconds=30;
  return out;
}
void verify(const ModelSnapshot& source,const NativeNeighborhoodOptions& options,
            const NativeNeighborhoodResult& answer) {
  ++runs;const auto best=oracle(source);const auto& r=answer.search.result;
  const bool minimize=source.objective.sense==ObjectiveSense::Minimize;
  assert(r.model_id==source.model_id && r.revision==source.revision);
  assert(r.guarantee==options.search.solve.guarantee);
  if(r.has_solution()) {
    assert(best && feasible(source,r.values));assert(r.objective==static_cast<double>(objective(source,r.values)));
    assert(minimize?*r.objective>=*best:*r.objective<=*best);
  }
  if(r.best_bound && best) assert(minimize?*r.best_bound<=*best:*r.best_bound>=*best);
  if(r.termination==Termination::Optimal) {
    assert(best && r.has_solution() && r.objective==static_cast<double>(*best) && r.best_bound==r.objective);
  }
  if(r.termination==Termination::Infeasible) assert(!best && !r.has_solution());
  const auto& n=answer.neighborhood;
  assert(n.requested && n.attempts<=1 && n.accepted_improvements<=1);
  assert(n.completed_status_calls<=n.status_attempts && n.status_attempts<=options.neighborhood.max_status_calls);
  assert(n.source_entries<=options.neighborhood.max_source_entries);
  assert(n.coordinator_work<=options.neighborhood.max_coordinator_work);
  assert(n.peak_local_spaces<=options.neighborhood.max_local_spaces);
  assert(n.peak_total_spaces<=options.search.max_open_nodes);
  assert(n.budget_nodes==answer.search.frontier.admitted_nodes+answer.search.branching.probe_status_calls+n.status_attempts);
  assert(n.budget_nodes==answer.search.branching.budget_nodes);
  if(options.search.solve.node_limit) assert(n.budget_nodes<=*options.search.solve.node_limit);
  if(n.accepted_improvements) assert(r.has_solution());
}
NativeNeighborhoodResult run(const ModelSnapshot& model,const NativeNeighborhoodOptions& o) {
  auto result=solve_native_neighborhoods(model,o);verify(model,o,result);return result;
}
std::vector<double> worst(const ModelSnapshot& model) {
  std::vector<double> out;
  for(auto p:points(model)) if(feasible(model,p) && (out.empty() ||
      (model.objective.sense==ObjectiveSense::Minimize?objective(model,p)>objective(model,out):objective(model,p)<objective(model,out)))) out=std::move(p);
  return out;
}
void exhaustive(const Model& model) {
  const auto source=model.snapshot();const auto bad=worst(source);
  for(int order=0;order<2;++order)for(int radius=0;radius<4;++radius)for(int start=0;start<2;++start) {
    auto o=options();o.search.order=order?NativeSearchOrder::BestBound:NativeSearchOrder::DepthFirst;
    o.neighborhood.radius=static_cast<std::size_t>(radius);
    if(start && !bad.empty()) o.search.solve.primal_start=entries(source,bad,true);
    auto result=run(source,o);assert(result.search.result.termination==Termination::Optimal || result.search.result.termination==Termination::Infeasible);
    for(std::uint64_t quota=0;quota<14;++quota) {
      o.search.solve.node_limit=quota;run(source,o);
    }
  }
}
Model outside(bool table=false,bool maximize=false) {
  Model model;auto x=model.add_binary(),y=model.add_binary(),z=model.add_binary();
  if(table) add_table(model,{y,z},{{0,0},{1,0},{1,1}});
  else model.add_row({{z,1},{y,-1}},-inf,0);
  if(maximize) model.maximize({{x,-100},{y,20},{z,-1}},17);
  else model.minimize({{x,100},{y,-20},{z,1}},-9);
  return model;
}
void models() {
  auto m=outside();exhaustive(m);
  auto t=outside(true,true);exhaustive(t);
  {Model m;auto x=m.add_binary(),y=m.add_binary(),a=m.add_integer(-2,2);
    auto s=m.add_variable(VariableType::SemiInteger,2,3);m.add_row({{x,1},{y,1},{a,1},{s,1}},0,3);
    m.minimize({{a,2},{s,-1},{x,3},{y,-2}},11);exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_binary();m.add_row({{x,1},{y,-1}},0,0);
    m.maximize({{x,2},{y,1},{z,-1}});exhaustive(m);}
  for(bool activation:{false,true}) {
    Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_integer(-1,2);
    add_indicator(m,x,activation,{{z,1}},0,1);m.maximize({{x,2},{y,3},{z,-1}},-4);exhaustive(m);
  }
  {Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_binary();
    auto indicator=add_indicator(m,x,true,{{z,1}},-inf,0);auto snap=m.snapshot();
    const auto gate=snap.indicators[indicator.indicator.id].inactive_gate;assert(gate);remove_indicator(m,indicator.indicator);
    m.minimize({{x,2},{y,-1},{z,-1},{*gate,-1}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_integer(0,2);
    add_all_different(m,{x,z});m.minimize({{x,3},{y,1},{z,-1}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_binary(),a=m.add_integer(0,1);
    add_element(m,a,{x,y},z);m.minimize({{x,2},{y,-2},{z,3}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),a=m.add_integer(3,5),b=m.add_integer(3,5),c=m.add_integer(3,5);
    add_circuit(m,{a,b,c},3);m.minimize({{x,5},{y,-4},{a,1},{b,-1}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),a=m.add_integer(-1,1),b=m.add_integer(-1,1);
    add_cumulative(m,{a,b,a},{1,1,0},{1,1,8},1);m.minimize({{x,3},{y,-2},{a,2},{b,-1}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_binary();
    add_regular(m,{x,y,x},2,0,{{0,0,0},{0,1,1},{1,0,1},{1,1,0}},{1});
    add_regular(m,{},100000000000ULL,7,{}, {7});m.maximize({{x,3},{y,4},{z,-1}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),y=m.add_binary(),z=m.add_binary();
    add_regular(m,{x,y},2,0,{{0,1,1}},{1});m.maximize({{z,1}});exhaustive(m);}
  {Model m;auto x=m.add_binary(),dead=m.add_binary(),y=m.add_binary();m.remove(dead);
    m.minimize({{x,2},{y,-3}},-9007199254740980.0);exhaustive(m);}
}
void limits_and_skips() {
  auto m=outside();const auto source=m.snapshot();auto o=options();o.search.order=NativeSearchOrder::DepthFirst;
  const auto full=run(source,o);assert(full.neighborhood.accepted_improvements==1);
  for(std::size_t limit:{0U,1U,2U,3U,5U,10U,40U}) {
    auto a=o;a.neighborhood.max_coordinator_work=limit;run(source,a);
    a=o;a.neighborhood.max_source_entries=limit;run(source,a);
    a=o;a.neighborhood.max_local_spaces=limit;run(source,a);
    a=o;a.neighborhood.max_status_calls=limit;run(source,a);
    a=o;a.search.max_open_nodes=limit;run(source,a);
  }
  {auto a=o;a.neighborhood.time_limit_seconds=0;auto r=run(source,a);assert(!r.neighborhood.attempts && r.neighborhood.completion==NativeNeighborhoodCompletion::LocalTimeLimit);}
  {auto a=o;a.neighborhood.max_distance_variables=2;auto r=run(source,a);assert(!r.neighborhood.attempts && r.neighborhood.completion==NativeNeighborhoodCompletion::FormulationLimit);}
  {auto a=o;a.neighborhood.radius=std::numeric_limits<std::size_t>::max();auto r=run(source,a);assert(!r.neighborhood.attempts && r.neighborhood.completion==NativeNeighborhoodCompletion::NonrestrictingRadius);}
  {Model pure;auto x=pure.add_integer(-2,2),y=pure.add_integer(-2,2);pure.add_row({{x,1},{y,1}},0,3);pure.minimize({{x,2},{y,-1}});
    auto r=run(pure.snapshot(),o);assert(!r.neighborhood.attempts);}
  {Model fixed;auto x=fixed.add_binary(),y=fixed.add_binary();fixed.set_bounds(x,0,0);fixed.minimize({{y,1}});run(fixed.snapshot(),o);}
  {Model constant;constant.add_binary();constant.add_binary();constant.minimize({},17);auto r=run(constant.snapshot(),o);assert(!r.neighborhood.attempts);}
  {auto a=o;a.search.solve.node_limit=0;auto r=run(source,a);assert(r.search.result.termination==Termination::NodeLimit && !r.neighborhood.attempts);}
  {auto a=o;a.search.solve.time_limit_seconds=0;auto r=run(source,a);assert(r.search.result.termination==Termination::TimeLimit && !r.neighborhood.attempts);}
  for(int lp=0;lp<2;++lp) {
    if(lp && !native_lp_capabilities().available) continue;
    for(std::uint64_t quota=0;quota<50;++quota) {
      auto a=o;a.search.solve.node_limit=quota;a.search.branching=NativeBranchingSettings{};
      if(lp) {a.search.relaxation=NativeLpSettings{};a.search.relaxation->root_cover_cuts=NativeRootCoverSettings{};}
      run(source,a);
    }
  }
}
void bad_inputs() {
  Model m;auto x=m.add_binary(),y=m.add_binary();m.minimize({{x,1},{y,-1}});
  auto o=options();o.neighborhood.policy=static_cast<NativeNeighborhoodPolicy>(73);
  assert(solve_native_neighborhoods(m,o).search.result.termination==Termination::InvalidModel);
  o=options();o.neighborhood.time_limit_seconds=inf;
  assert(solve_native_neighborhoods(m,o).search.result.termination==Termination::InvalidModel);
  auto source=m.snapshot();source.variables[0].variable.model_id++;
  assert(solve_native_neighborhoods(source,options()).search.result.termination==Termination::InvalidModel);
  source=m.snapshot();source.objective.terms[0].coefficient=0.5;
  assert(solve_native_neighborhoods(source,options()).search.result.termination==Termination::Unsupported);
  source=m.snapshot();source.variables[0].upper=inf;
  assert(solve_native_neighborhoods(source,options()).search.result.termination!=Termination::Infeasible);
  Model moved=std::move(m);assert(solve_native_neighborhoods(m,options()).search.result.termination==Termination::InvalidModel);
  o=options();o.search.solve.guarantee=Guarantee::Certified;
  assert(solve_native_neighborhoods(moved,o).search.result.termination==Termination::Unsupported);
  o=options();o.search.solve.backend=Backend::Highs;
  assert(solve_native_neighborhoods(moved,o).search.result.termination==Termination::Unsupported);
}
#ifdef GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS
void faults() {
  auto m=outside(true);auto source=m.snapshot();auto o=options();o.search.order=NativeSearchOrder::DepthFirst;
  std::vector<std::string> events;
  hook=[&](const char* event,std::size_t,int,int,std::vector<double>*) {events.emplace_back(event);};
  auto full=run(source,o);hook={};assert(full.neighborhood.accepted_improvements==1);
  bool outside_parent=false,publication=false;int parent_x=-1;
  hook=[&](const char* event,std::size_t slot,int lower,int,std::vector<double>* values) {
    if(std::string(event)=="neighborhood_parent_variable" && slot==0) parent_x=lower;
    if(std::string(event)=="before_neighborhood_validation") outside_parent=parent_x==1 && (*values)[0]==0;
    if(std::string(event)=="after_neighborhood_publication") publication=true;
  };
  run(source,o);hook={};assert(outside_parent && publication);
  // Every actual hook position is a cancellation boundary. Counts need not
  // represent executed propagation; the independent full optimum checks proof.
  for(std::size_t stop=0;stop<events.size();++stop) {
    auto a=o;a.search.solve.cancellation=std::make_shared<CancellationToken>();std::size_t count=0;
    hook=[&](const char*,std::size_t,int,int,std::vector<double>*) {if(count++==stop)a.search.solve.cancellation->cancel();};
    auto r=run(source,a);hook={};assert(r.search.result.termination==Termination::Cancelled);
  }
  for(const std::string event:{"neighborhood_root_alloc","neighborhood_constructed","neighborhood_choice",
      "neighborhood_child_clone","before_neighborhood_validation","after_neighborhood_release",
      "before_neighborhood_publication","after_neighborhood_publication"}) {
    bool fired=false;
    hook=[&](const char* at,std::size_t,int,int,std::vector<double>*) {if(!fired && event==at){fired=true;throw std::bad_alloc();}};
    auto r=run(source,o);hook={};assert(fired && r.search.result.termination==Termination::MemoryLimit);
    if(event=="after_neighborhood_publication") assert(r.neighborhood.accepted_improvements==1);
    if(event=="before_neighborhood_publication") assert(!r.neighborhood.accepted_improvements);
  }
  {bool fired=false;hook=[&](const char* event,std::size_t,int,int,std::vector<double>* values) {
    if(!fired && std::string(event)=="before_neighborhood_validation") {fired=true;(*values)[0]=1-1e-12;}
   };auto r=run(source,o);hook={};assert(fired && r.search.result.termination==Termination::BackendError && !r.neighborhood.accepted_improvements);}
  for(const std::string event:{"neighborhood_construction","before_neighborhood_status","after_neighborhood_validation",
      "after_neighborhood_release","before_neighborhood_publication"}) {
    auto a=o;a.neighborhood.time_limit_seconds=0.02;bool fired=false;
    hook=[&](const char* at,std::size_t,int,int,std::vector<double>*) {
      if(!fired && event==at){fired=true;std::this_thread::sleep_for(std::chrono::milliseconds(30));}
    };
    auto r=run(source,a);hook={};assert(fired && r.neighborhood.completion==NativeNeighborhoodCompletion::LocalTimeLimit && !r.neighborhood.accepted_improvements);
    assert(r.search.result.termination==Termination::Optimal);
  }
  for(const std::string event:{"before_neighborhood_publication","after_neighborhood_publication"}) {
    auto a=o;a.search.solve.time_limit_seconds=0.02;bool fired=false;
    hook=[&](const char* at,std::size_t,int,int,std::vector<double>*) {
      if(!fired && event==at){fired=true;std::this_thread::sleep_for(std::chrono::milliseconds(30));}
    };
    auto r=run(source,a);hook={};assert(fired && r.search.result.termination==Termination::TimeLimit);
    assert(r.neighborhood.accepted_improvements==(event=="after_neighborhood_publication"?1U:0U));
    assert(r.neighborhood.elapsed_seconds>=0.03);
  }
  // Radius zero fixes just Binary slots. Its first local status fixes the
  // improving general integer through the strict objective cutoff.
  {Model z;auto x=z.add_binary(),y=z.add_binary(),a=z.add_integer(0,1);z.maximize({{a,1}});
    auto p=options();p.search.solve.primal_start={{x,0},{y,0},{a,0}};p.neighborhood.radius=0;p.neighborhood.max_status_calls=1;
    auto r=run(z.snapshot(),p);assert(r.neighborhood.status_attempts==1 && r.neighborhood.accepted_improvements==1);}
}
#endif
} // namespace
namespace Gecode { namespace Optimize {
#ifdef GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS
void native_neighborhood_test_event(const char* event,std::size_t slot,int lower,int upper,std::vector<double>* values) {
  if(hook) hook(event,slot,lower,upper,values);
}
#endif
}}
int main() {
  if(!native_capabilities().available) {
    Model m;m.add_binary();auto r=solve_native_neighborhoods(m,options());
    assert(r.search.result.termination==Termination::Unsupported && r.neighborhood.requested);
    std::cout<<"Native neighborhoods disabled-backend contract passed\n";return 0;
  }
  models();limits_and_skips();bad_inputs();
#ifdef GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS
  faults();
#endif
  const auto model=outside().snapshot();
  auto a=std::async(std::launch::async,[&]{return solve_native_neighborhoods(model,options());});
  auto b=std::async(std::launch::async,[&]{return solve_native_neighborhoods(model,options());});
  verify(model,options(),a.get());verify(model,options(),b.get());
  std::cout<<"Native neighborhoods passed "<<runs<<" exhaustive/quota/fault configurations\n";
}
