#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/globals.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/native_regular_limits.hpp>
#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/diagnostics.hpp>
#include <gecode/optimize/presolve.hpp>
#include <gecode/optimize/relaxation.hpp>
#include <gecode/optimize/session.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <future>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
using namespace Gecode::Optimize;
using I=std::int64_t;
using Word=std::vector<I>;
namespace {
std::size_t configurations=0,assignments=0,solves=0;
// Generate the finite accepted language from labeled paths. This never looks
// up the next transition by a proposed word and never uses the common checker.
std::set<Word> language(const RegularData& data) {
  std::set<Word> accepted;Word word;
  const auto visit=[&](const auto& self,std::uint64_t state)->void {
    if(word.size()==data.variables.size()){
      if(std::find(data.final_states.begin(),data.final_states.end(),state)!=data.final_states.end())accepted.insert(word);
      return;
    }
    for(const auto& edge:data.transitions)if(edge.from==state){word.push_back(edge.symbol);self(self,edge.to);word.pop_back();}
  };visit(visit,data.initial_state);return accepted;
}
I objective(const ModelSnapshot& model,const std::vector<double>& point) {
  I result=static_cast<I>(model.objective.offset);
  for(const auto& term:model.objective.terms)result+=static_cast<I>(term.coefficient)*static_cast<I>(point[term.variable.id]);
  return result;
}
std::vector<std::vector<double>> points(const ModelSnapshot& model,const RegularData& data) {
  const auto accepted=language(data);std::vector<std::vector<double>> feasible;
  std::vector<double> point(model.variables.size(),std::numeric_limits<double>::quiet_NaN());
  const auto visit=[&](const auto& self,std::size_t index)->void {
    if(index<model.variables.size()){
      const auto& variable=model.variables[index];if(!variable.active){self(self,index+1);return;}
      const I lower=variable.type==VariableType::SemiInteger?0:static_cast<I>(variable.lower),upper=static_cast<I>(variable.upper);
      assert(upper-lower<=5);
      for(I value=lower;value<=upper;++value)if(value>=variable.lower||(variable.type==VariableType::SemiInteger&&value==0)){point[index]=value;self(self,index+1);}
      return;
    }
    Word word;for(auto variable:data.variables)word.push_back(static_cast<I>(point[variable.id]));
    const bool expected=accepted.count(word)!=0;const auto checked=validate(model,point,0,0);
    assert(checked.valid==expected);++assignments;
    if(expected){assert(checked.objective==objective(model,point));feasible.push_back(point);}
  };visit(visit,0);++configurations;return feasible;
}
void checked_result(const ModelSnapshot& model,const RegularData& data,const SolveResult& result,
                    const std::vector<std::vector<double>>& feasible,bool complete) {
  assert(result.model_id==model.model_id&&result.revision==model.revision);
  if(result.has_solution()){
    assert(result.solution_validated&&result.guarantee==Guarantee::Exact);
    assert(result.values.size()==model.variables.size()&&result.active_variables.size()==model.variables.size());
    for(std::size_t i=0;i<model.variables.size();++i)assert(result.active_variables[i]==model.variables[i].active);
    assert(std::any_of(feasible.begin(),feasible.end(),[&](const auto& candidate){
      for(std::size_t i=0;i<model.variables.size();++i)if(model.variables[i].active&&candidate[i]!=result.values[i])return false;return true;
    }));
    Word word;for(auto variable:data.variables)word.push_back(static_cast<I>(result.values[variable.id]));
    assert(language(data).count(word));assert(result.objective==objective(model,result.values));
  }
  std::optional<I> best;
  for(const auto& point:feasible){auto value=objective(model,point);if(!best||(model.objective.sense==ObjectiveSense::Minimize?value<*best:value>*best))best=value;}
  if(result.best_bound&&best)assert(model.objective.sense==ObjectiveSense::Minimize?*result.best_bound<=*best:*result.best_bound>=*best);
  if(complete){
    assert(result.termination==(best?Termination::Optimal:Termination::Infeasible));
    assert(result.has_solution()==bool(best));if(best)assert(result.objective==*best&&result.best_bound==*best);
  }
}
void solve_routes(const ModelSnapshot& model,const RegularData& data,const std::vector<std::vector<double>>& feasible,bool limits=false) {
  SolveOptions options;options.backend=Backend::Native;options.guarantee=Guarantee::Exact;
  if(!native_capabilities().available){auto result=solve_native(model,options);assert(result.termination==Termination::Unsupported&&!result.has_solution());++solves;return;}
  auto result=solve_native(model,options);checked_result(model,data,result,feasible,true);++solves;
  NativeSearchOptions search;search.solve=options;
  for(auto strategy:{NativeSearchOrder::DepthFirst,NativeSearchOrder::BestBound}){
    search.order=strategy;auto solved=solve_native_search(model,search);checked_result(model,data,solved.result,feasible,true);++solves;
    if(limits)for(std::uint64_t cap=0;cap<=solved.frontier.admitted_nodes+1;++cap){search.solve.node_limit=cap;auto stopped=solve_native_search(model,search);checked_result(model,data,stopped.result,feasible,false);search.solve.node_limit.reset();++solves;}
  }
  if(native_lp_capabilities().available){NativeLpOptions lp;lp.solve=options;auto solved=solve_native_lp(model,lp);checked_result(model,data,solved.result,feasible,true);++solves;}
  for(const auto& point:feasible){options.primal_start.clear();for(const auto& variable:model.variables)if(variable.active)options.primal_start.push_back({variable.variable,point[variable.variable.id]});
    auto started=solve_native(model,options);assert(started.start_submitted==!options.primal_start.empty());checked_result(model,data,started,feasible,true);++solves;if(!limits)break;
  }
}
void exhaustive(){
  // Every partial deterministic transition function on two states and signed
  // alphabet {-1,1}, every initial/final set, word lengths zero through three.
  for(unsigned code=0;code<81;++code)for(unsigned initial=0;initial<2;++initial)for(unsigned final=0;final<4;++final)for(unsigned size=0;size<=3;++size){
    Model model;RegularData data;data.state_count=2;data.initial_state=initial;
    for(unsigned i=0;i<size;++i)data.variables.push_back(model.add_integer(-1,1));
    auto choices=code;for(unsigned state=0;state<2;++state)for(I symbol:{I(-1),I(1)}){const auto target=choices%3;choices/=3;if(target)data.transitions.push_back({state,symbol,target-1});}
    for(unsigned state=0;state<2;++state)if(final&(1u<<state))data.final_states.push_back(state);
    model.add_global(data);if(size)model.minimize({{data.variables.front(),-1}},3);
    auto snapshot=model.snapshot();auto feasible=points(snapshot,data);
    if(code%13==0&&initial==1&&final==2)solve_routes(snapshot,data,feasible);
  }
}
void variants_and_history(){
  for(bool maximize:{false,true})for(bool repeat:{false,true}){
    Model model;const auto x=model.add_integer(-1,1),y=model.add_integer(-1,1);
    RegularData data{{x,repeat?x:y},3,0,{{0,-1,1},{0,1,2},{1,1,2},{2,-1,2}},{2,2}};
    const auto global=add_regular(model,data.variables,data.state_count,data.initial_state,data.transitions,data.final_states,"signed");
    model.set_objective({{x,-2},{y,1}},maximize?ObjectiveSense::Maximize:ObjectiveSense::Minimize,-4);
    auto snapshot=model.snapshot();auto feasible=points(snapshot,data);solve_routes(snapshot,data,feasible,true);
    bool failed=false;try{model.remove(x);}catch(const ModelError&){failed=true;}assert(failed);
    const auto revision=model.revision();model.set_name(global,"renamed");assert(model.revision()==revision+1&&model.global(global).name=="renamed");
    model.remove(global);model.minimize({});model.remove(x);validate_structure(model.snapshot());assert(snapshot.globals[0].active&&snapshot.variables[0].active);
    if(!feasible.empty())assert(validate(snapshot,feasible[0],0,0).valid);
  }
  Model semi;const auto x=semi.add_variable(VariableType::SemiInteger,2,3);
  RegularData data{{x},1,0,{{0,0,0},{0,3,0}},{0}};semi.add_global(data);semi.maximize({{x,1}},-2);
  auto snapshot=semi.snapshot();auto feasible=points(snapshot,data);solve_routes(snapshot,data,feasible,true);
  // Native compaction depends only on referenced state IDs, not the declared
  // count; unreachable states/finals do not change the accepted language.
  Model sparse;auto a=sparse.add_integer(-1,1);constexpr auto huge=std::numeric_limits<std::uint64_t>::max();
  RegularData sparse_data{{a},huge,huge-2,{{huge-2,-1,huge-2},{2,1,3}},{huge-2,huge-2,huge-3}};
  sparse.add_global(sparse_data);sparse.minimize({{a,1}});auto s=sparse.snapshot();solve_routes(s,sparse_data,points(s,sparse_data),true);
  // Zero is a real typed state and symbol; missing transitions reject.
  Model zero;auto z=zero.add_binary();RegularData zdata{{z},1,0,{{0,0,0}},{0}};zero.add_global(zdata);
  auto zs=zero.snapshot();solve_routes(zs,zdata,points(zs,zdata),true);
  SolveOptions opts;opts.backend=Backend::Native;opts.guarantee=Guarantee::Exact;opts.primal_start={{z,1}};
  assert(solve_native(zero,opts).termination==(native_capabilities().available?Termination::InvalidModel:Termination::Unsupported));
  for(int stop=0;stop<3;++stop){opts.primal_start={{z,0}};if(stop==0)opts.node_limit=0;if(stop==1){opts.node_limit.reset();opts.time_limit_seconds=0;}
    if(stop==2){opts.time_limit_seconds=std::numeric_limits<double>::infinity();opts.cancellation=std::make_shared<CancellationToken>();opts.cancellation->cancel();}
    const auto result=solve_native(zero,opts);assert(!result.start_submitted&&!result.has_solution());
  }
  const auto zpoints=points(zs,zdata);
  std::vector<std::future<void>> jobs;for(int i=0;i<4;++i)jobs.push_back(std::async(std::launch::async,[zs,zdata,zpoints]{SolveOptions o;o.backend=Backend::Native;o.guarantee=Guarantee::Exact;auto r=solve_native(zs,o);if(native_capabilities().available)checked_result(zs,zdata,r,zpoints,true);}));
  for(auto& job:jobs)job.get();
}
void invalid(){
  Model model;const auto x=model.add_integer(0,1);RegularData valid{{x},2,0,{{0,0,1}},{1}};
  const auto reject=[&](RegularData data){const auto before=model.revision();bool failed=false;try{model.add_global(data);}catch(const ModelError&){failed=true;}assert(failed&&model.revision()==before&&model.snapshot().globals.empty());++configurations;};
  auto data=valid;data.state_count=0;reject(data);data=valid;data.initial_state=2;reject(data);
  data=valid;data.transitions[0].from=2;reject(data);data=valid;data.transitions[0].to=2;reject(data);
  data=valid;data.final_states={2};reject(data);data=valid;data.transitions.push_back(data.transitions[0]);reject(data);
  data.transitions.back().to=0;reject(data);data=valid;data.transitions[0].symbol=9007199254740993LL;reject(data);
  Model other;auto foreign=other.add_binary();data=valid;data.variables={foreign};reject(data);
  auto dead=model.add_integer(0,1);model.remove(dead);data=valid;data.variables={dead};reject(data);
  auto continuous=model.add_continuous(0,1);data=valid;data.variables={continuous};reject(data);
  model.add_global(valid);auto snapshot=model.snapshot();
  for(int failure=0;failure<6;++failure){auto corrupt=snapshot;auto& original=std::get<RegularData>(corrupt.globals[0].payload);
    if(failure==0)original.initial_state=2;if(failure==1)original.transitions.push_back(original.transitions[0]);
    if(failure==2)original.final_states={2};if(failure==3)original.variables={foreign};if(failure==4)original.variables={dead};
    if(failure==5){corrupt.globals[0].active=false;original.variables.clear();original.state_count=0;}
    bool failed=false;try{validate_structure(corrupt);}catch(const ModelError&){failed=true;}assert(failed);++configurations;
    assert(solve_native(corrupt).termination==Termination::InvalidModel);
  }
  // Exact public constants can exceed native symbols even for an empty word;
  // unused transitions do not bypass full native admission.
  for(bool empty:{false,true}){Model large;auto v=large.add_integer(0,0);
    RegularData wide{empty?std::vector<Variable>{}:std::vector<Variable>{v},1,0,{{0,0,0},{0,9007199254740992LL,0}},{0}};
    large.add_global(wide);assert(validate(large.snapshot(),{0},0,0).valid);
    assert(solve_native(large).termination==Termination::Unsupported);
  }
}
void combined_original(){
  Model model;auto x=model.add_binary(),y=model.add_binary(),b=model.add_binary();
  RegularData data{{x,y},3,0,{{0,0,1},{0,1,2},{1,0,1},{1,1,1},{2,0,1}},{1}};
  model.add_global(data);add_all_different(model,{x,y});
  model.add_row({{x,1},{y,1}},1,std::numeric_limits<double>::infinity());
  const auto indicator=add_indicator(model,b,true,{{x,1}},1,std::numeric_limits<double>::infinity());
  assert(indicator.inactive_gate);model.maximize({{x,-2},{y,1},{b,4}},-3);
  const auto snapshot=model.snapshot();std::vector<std::vector<double>> feasible;
  for(int xv=0;xv<=1;++xv)for(int yv=0;yv<=1;++yv)for(int bv=0;bv<=1;++bv)for(int gate=0;gate<=1;++gate){
    std::vector<double> point(snapshot.variables.size());point[x.id]=xv;point[y.id]=yv;point[b.id]=bv;point[indicator.inactive_gate->id]=gate;
    const bool expected=!(xv&&yv)&&xv!=yv&&xv+yv>=1&&(!bv||xv==1)&&gate==1-bv;
    assert(validate(snapshot,point,0,0).valid==expected);++assignments;if(expected)feasible.push_back(point);
  }
  ++configurations;solve_routes(snapshot,data,feasible,true);
  if(native_capabilities().available){
    NativeSearchOptions options;options.solve.backend=Backend::Native;options.solve.guarantee=Guarantee::Exact;
    options.branching=NativeBranchingSettings{};
    if(native_lp_capabilities().available)options.relaxation=NativeLpSettings{};
    const auto searched=solve_native_search(snapshot,options);checked_result(snapshot,data,searched.result,feasible,true);++solves;
  }
}
void cross_boundaries(){
  Model model;auto dead=model.add_binary();model.remove(dead);auto x=model.add_integer(-1,1);
  RegularData data{{x},1,0,{{0,-1,0},{0,1,0}},{0}};
  const auto handle=model.add_global(data);model.minimize({{x,1}},2);
  const auto snapshot=model.snapshot();const auto feasible=points(snapshot,data);solve_routes(snapshot,data,feasible,true);
  auto automatic=solve(snapshot);if(native_capabilities().available){assert(automatic.backend=="Gecode native");assert(automatic.objective==1);}else assert(automatic.termination==Termination::Unsupported);
  SolveOptions highs;highs.backend=Backend::Highs;
  assert(solve(snapshot,highs).termination==Termination::Unsupported);
  assert(analyze_conflict(snapshot).status==ConflictStatus::Unsupported);
  assert(presolve_integer(snapshot).status==PresolveStatus::Unsupported);
  assert(relax_feasibility(snapshot).termination==Termination::Unsupported);
  SolveSession session;assert(session.solve(snapshot).termination==Termination::Unsupported);
  const auto path=std::filesystem::temp_directory_path()/("gecode-regular-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".lp");
  {std::ofstream out(path);out<<"unchanged destination";}
  bool rejected=false;try{write_model(snapshot,path.string());}catch(const ModelError&){rejected=true;}assert(rejected);
  {std::ifstream in(path);std::string contents((std::istreambuf_iterator<char>(in)),{});assert(contents=="unchanged destination");}
  std::filesystem::remove(path);
  // The original word is rechecked after numerical rounding, with no row-only
  // shortcut; exact native start admission never rounds a near-integer hint.
  assert(validate(snapshot,{0,-1.0000001},1e-6,1e-6).valid);
  assert(!validate(snapshot,{0,0},0,0).valid);
  SolveOptions start;start.backend=Backend::Native;start.guarantee=Guarantee::Exact;start.primal_start={{x,-1.0000001}};
  assert(solve_native(snapshot,start).termination==(native_capabilities().available?Termination::InvalidModel:Termination::Unsupported));
  model.remove(handle);model.minimize({});model.remove(x);validate_structure(model.snapshot());
  const auto id=model.id();Model moved(std::move(model));assert(moved.id()==id);validate_structure(moved.snapshot());
  assert(snapshot.globals[0].active&&snapshot.variables[x.id].active);
}
void size_boundaries(){
  const auto okay=[](std::uint64_t w,std::uint64_t q,std::uint64_t t,std::uint64_t a,bool small=false){return Detail::native_regular_size_error(w,q,t,a,small)==nullptr;};
  constexpr auto imax=static_cast<std::uint64_t>(std::numeric_limits<int>::max());
  constexpr auto umax=static_cast<std::uint64_t>(std::numeric_limits<unsigned int>::max());
  assert(okay(imax-1,1,0,0));assert(!okay(imax,1,0,0));assert(okay(0,imax-1,0,0));assert(!okay(0,imax,0,0));
  assert(okay(0,1,imax-1,0));assert(!okay(0,1,imax,0));assert(!okay(0,0,0,0));
  assert(okay(1,imax/2,0,0));assert(!okay(1,imax/2+1,0,0));
  assert(okay(100000,1,umax/100000,1));assert(!okay(100000,1,umax/100000+1,1));
  const auto hash=imax/2+1;assert(okay(0,1,hash-1,hash-1));assert(!okay(0,1,hash,hash));
  const auto short_max=std::numeric_limits<unsigned short>::max();assert(okay(1,1,short_max,short_max,true));assert(!okay(1,1,std::uint64_t(short_max)+1,std::uint64_t(short_max)+1,true));
  assert(!okay(std::numeric_limits<std::uint64_t>::max(),1,0,0));
  // This concrete alphabet fits int16 values but has 65536 possible supports.
  // Do not enter the native graph's ushort support-count overflow.
  Model model;auto x=model.add_integer(0,0);RegularData data{{x},1,0,{},{0}};
  for(I symbol=std::numeric_limits<short>::min();symbol<=std::numeric_limits<short>::max();++symbol)data.transitions.push_back({0,symbol,0});
  model.add_global(data);assert(validate(model.snapshot(),{0},0,0).valid);
  auto result=solve_native(model);assert(result.termination==Termination::Unsupported);
  if(native_capabilities().available)assert(result.message.find("short-symbol")!=std::string::npos);
}
}
int main(){size_boundaries();exhaustive();variants_and_history();invalid();combined_original();cross_boundaries();std::cout<<configurations<<" regular configurations, "<<assignments<<" original assignments, "<<solves<<" native-route solves pass\n";}
