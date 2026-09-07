#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
std::optional<double> oracle(const ModelSnapshot& m){
  std::optional<double> best;std::vector<double> v(m.variables.size());
  const auto visit=[&](const auto& self,std::size_t i)->void{
    if(i<v.size()){
      for(int value=int(m.variables[i].lower);value<=m.variables[i].upper;++value){v[i]=value;self(self,i+1);}return;
    }
    for(const auto& row:m.rows){double a=0;for(auto t:row.terms)a+=t.coefficient*v[t.variable.id];if(a<row.lower || a>row.upper)return;}
    double a=m.objective.offset;for(auto t:m.objective.terms)a+=t.coefficient*v[t.variable.id];
    if(!best || (m.objective.sense==ObjectiveSense::Minimize ? a<*best:a>*best))best=a;
  };visit(visit,0);return best;
}
Model fixture(bool binary,bool maximize,bool identical){
  Model m;std::vector<Term> sum,twice,cost;
  for(int i=0;i<5;++i){auto x=binary?m.add_binary():m.add_integer(-1,2);
    sum.push_back({x,1});twice.push_back({x,2});cost.push_back({x,identical?1.0:double(i+1)});}
  m.add_row(sum,2,inf);m.add_row(twice,-inf,8);
  m.set_objective(cost,maximize?ObjectiveSense::Maximize:ObjectiveSense::Minimize,-13);
  return m;
}
}
int main(){
  if(!native_capabilities().available){std::cout<<"Symmetry native integration unavailable\n";return 0;}
  SolveOptions o;o.backend=Backend::Native;o.guarantee=Guarantee::Exact;o.relative_gap=o.absolute_gap=0;o.time_limit_seconds=5;
  for(bool binary:{false,true})for(bool maximize:{false,true})for(bool identical:{false,true}){
    auto m=fixture(binary,maximize,identical);auto s=m.snapshot();auto expected=oracle(s);
    auto result=solve(m,o);
    if(result.termination!=Termination::Optimal)std::cerr<<result.message<<'\n';
    assert(result.termination==Termination::Optimal && result.objective==expected);
    assert(result.best_bound==expected && result.model_id==s.model_id && result.revision==s.revision);
    assert(validate(s,result.values,0,0).valid);
    assert((result.message.find("duplicate-column symmetry")!=std::string::npos)==identical);
    if(identical)for(unsigned i=1;i<result.values.size();++i)assert(result.values[i-1]<=result.values[i]);
  }
  auto m=fixture(true,false,true);auto s=m.snapshot();auto started=o;
  for(unsigned i=0;i<5;++i)started.primal_start.push_back({s.variables[i].variable,i<2?1.0:0.0});
  auto warm=solve(m,started);assert(warm.termination==Termination::Optimal && warm.start_submitted);
  assert(warm.message.find("duplicate-column symmetry")==std::string::npos);
  auto stopped=o;stopped.cancellation=std::make_shared<CancellationToken>();stopped.cancellation->cancel();
  auto cancelled=solve(m,stopped);assert(cancelled.termination==Termination::Cancelled && !cancelled.has_solution());
  // Two coupled rows prohibit swapping the differently occurring variables.
  Model asymmetric;auto x=asymmetric.add_binary(),y=asymmetric.add_binary();
  asymmetric.add_row({{x,1},{y,2}},1,1);asymmetric.minimize({{x,1},{y,1}});
  auto a=solve(asymmetric,o);assert(a.termination==Termination::Optimal && a.value(x)==1 && a.value(y)==0);
  // Public automatic path assembles two separate components and the offset once.
  Model disconnected;auto p=disconnected.add_integer(0,3),q=disconnected.add_integer(0,3);
  disconnected.add_row({{p,1}},1,3);disconnected.add_row({{q,1}},1,3);
  disconnected.minimize({{p,2},{q,-3}},7);auto d=solve(disconnected,o);
  assert(d.termination==Termination::Optimal && d.objective==0 && validate(disconnected.snapshot(),d.values,0,0).valid);
  assert(d.message.find("independent components")!=std::string::npos);
  std::cout<<"Symmetry and public preprocessing integration checks passed\n";
}
