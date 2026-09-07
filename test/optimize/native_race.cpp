#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using namespace Gecode::Optimize;
namespace {
const double inf=std::numeric_limits<double>::infinity();
Model fixture(bool maximize,int seed) {
  Model m;std::vector<Variable> x;std::vector<Term> cost;
  for(int i=0;i<12;++i){x.push_back(m.add_binary());cost.push_back({x.back(),double((i*7+seed)%17+1)});}
  for(int j=0;j<5;++j){std::vector<Term> terms;
    for(int i=0;i<12;++i)terms.push_back({x[i],double((i*13+j*7+seed)%11+1)});
    m.add_row(terms,20,42+j);
  }
  if(maximize)m.maximize(cost,-7);else m.minimize(cost,11);
  return m;
}
double oracle(const ModelSnapshot& m) {
  const bool minimize=m.objective.sense==ObjectiveSense::Minimize;
  double best=minimize?inf:-inf;
  for(unsigned mask=0;mask<(1U<<m.variables.size());++mask){
    bool valid=true;
    for(const auto& row:m.rows){double a=0;for(auto t:row.terms)a+=t.coefficient*((mask>>t.variable.id)&1U);
      valid=valid && a>=row.lower && a<=row.upper;}
    if(!valid)continue;
    double value=m.objective.offset;for(auto t:m.objective.terms)value+=t.coefficient*((mask>>t.variable.id)&1U);
    best=minimize?std::min(best,value):std::max(best,value);
  }
  return best;
}
void check(const ModelSnapshot& m,const SolveResult& r,double expected) {
  assert(r.model_id==m.model_id && r.revision==m.revision);
  if(r.has_solution()) {
    double value=m.objective.offset;
    for(auto t:m.objective.terms)value+=t.coefficient*r.values[t.variable.id];
    assert(r.objective==value);
    for(double x:r.values)assert(x==0 || x==1);
    for(const auto& row:m.rows){double a=0;for(auto t:row.terms)a+=t.coefficient*r.values[t.variable.id];
      assert(a>=row.lower && a<=row.upper);}
  }
  if(r.best_bound)assert(m.objective.sense==ObjectiveSense::Minimize ? *r.best_bound<=expected:*r.best_bound>=expected);
  if(r.termination==Termination::Optimal)assert(r.has_solution() && r.objective==expected && r.best_bound==expected);
}
}
int main(){
  NativeRaceOptions o;o.solve.backend=Backend::Native;o.solve.guarantee=Guarantee::Exact;
  o.solve.relative_gap=o.solve.absolute_gap=0;o.solve.time_limit_seconds=5;
  o.exploration_seconds=1;o.probe_node_limit=1;
  if(!native_capabilities().available){auto m=fixture(false,0);
    o.automatic={false,false,false,false};
    assert(solve_native_race(m,o).termination==Termination::Unsupported);
    o.probe_node_limit=0;
    assert(solve_native_race(m,o).termination==Termination::InvalidModel);return 0;}
  bool saw_two=false;
  for(bool maximize:{false,true})for(int seed=0;seed<4;++seed){
    auto m=fixture(maximize,seed);auto s=m.snapshot();auto optimum=oracle(s);
    auto result=solve_native_race(m,o);check(s,result,optimum);
    if(result.termination!=Termination::Optimal)std::cerr<<result.message<<'\n';
    assert(result.termination==Termination::Optimal);
    saw_two=saw_two || result.message.find("2 probes")!=std::string::npos;
    for(unsigned nodes:{0,1,2,5,20}){
      auto limited=o;limited.solve.node_limit=nodes;
      auto r=solve_native_race(s,limited);check(s,r,optimum);
      assert(r.termination==Termination::Optimal || r.termination==Termination::NodeLimit);
      auto p=r.message.find("cumulative nodes=");assert(p!=std::string::npos);
      assert(std::stoull(r.message.substr(p+17))<=nodes);
    }
    auto disabled=o;disabled.exploration_seconds=0;
    auto d=solve_native_race(s,disabled);check(s,d,optimum);assert(d.termination==Termination::Optimal);
    // Configured automatic candidates retain the same original semantics and
    // obey the global cap across both probes and the selected restart.
    auto configured=o;configured.automatic={false,false,false,false};
    auto configured_result=solve_native_race(s,configured);check(s,configured_result,optimum);
    assert(configured_result.termination==Termination::Optimal);
    configured.solve.node_limit=2;
    configured_result=solve_native_race(s,configured);check(s,configured_result,optimum);
    assert(configured_result.termination==Termination::Optimal || configured_result.termination==Termination::NodeLimit);
    const auto nodes=configured_result.message.find("cumulative nodes=");assert(nodes!=std::string::npos);
    assert(std::stoull(configured_result.message.substr(nodes+17))<=2);
    auto start=o;for(auto v:s.variables)start.solve.primal_start.push_back({v.variable,result.values[v.variable.id]});
    auto started=solve_native_race(s,start);check(s,started,optimum);
    assert(started.message.find("Native race skipped")==0);
    auto stopped=o;stopped.solve.time_limit_seconds=0;
    assert(solve_native_race(s,stopped).termination==Termination::TimeLimit);
    stopped=o;stopped.solve.cancellation=std::make_shared<CancellationToken>();stopped.solve.cancellation->cancel();
    assert(solve_native_race(s,stopped).termination==Termination::Cancelled);
  }
  assert(saw_two);
  // Disabled exploration still forwards settings, rather than falling back to
  // an unconfigured automatic solve. Isolate symmetry to observe its effect.
  Model symmetric;std::vector<Term> terms;
  for(int i=0;i<6;++i)terms.push_back({symmetric.add_binary(),1});
  symmetric.add_row(terms,2,4);symmetric.minimize(terms,-3);
  for(bool enabled:{false,true}){
    auto configured=o;configured.exploration_seconds=0;
    configured.automatic={false,false,enabled,false};
    auto result=solve_native_race(symmetric,configured);check(symmetric.snapshot(),result,-1);
    assert(result.termination==Termination::Optimal && result.message.find("Native race skipped")==0);
    assert((result.message.find("duplicate-column symmetry")!=std::string::npos)==enabled);
  }
  auto m=fixture(false,0);auto invalid=o;invalid.exploration_seconds=-1;
  assert(solve_native_race(m,invalid).termination==Termination::InvalidModel);
  invalid=o;invalid.probe_node_limit=0;
  assert(solve_native_race(m,invalid).termination==Termination::InvalidModel);
  auto s=m.snapshot();s.rows[0].terms[0].variable.id+=100;
  assert(solve_native_race(s,o).termination==Termination::InvalidModel);
  auto saved=std::move(m);assert(solve_native_race(m,o).termination==Termination::InvalidModel);
  Model impossible;auto x=impossible.add_binary();impossible.add_row({{x,2}},1,1);
  assert(solve_native_race(impossible,o).termination==Termination::Infeasible);
  std::cout<<"Native sequential race: exhaustive min/max optima, shared nodes, starts and interruption passed\n";
}
