#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
using namespace Gecode::Optimize;
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();
Model fixture(int kind) {
  Model m; std::vector<Variable> x; std::vector<Term> sum, cost;
  for (int i=0;i<10;++i) {
    x.push_back(m.add_binary()); sum.push_back({x.back(),kind>=3 ? double(i+1):1.0});
    cost.push_back({x.back(),double((i*7)%13+1)});
  }
  if (kind>=3) {
    if (kind==4) for (auto& t:sum) t.coefficient*=10000;
    m.add_row(sum,-inf,kind==4 ? 70000:17);m.maximize(cost);
  } else {
    m.add_row(sum,3,7);m.minimize(cost);
    if(kind==0) m.add_row({{x[0],1},{x[1],1}},1,inf);
    if(kind==1) for(int i=0;i<9;++i)m.add_row({{x[i],1},{x[i+1],-1}},-inf,0);
    if(kind==2)m.add_row({{x[0],3},{x[1],2},{x[3],4},{x[5],-2}},2,6);
  }
  return m;
}
// Independent exhaustive binary oracle, without solver or common validator.
std::pair<double,std::vector<double>> oracle(const ModelSnapshot& m) {
  double best=m.objective.sense==ObjectiveSense::Minimize ? inf:-inf;
  std::vector<double> witness;
  for(unsigned mask=0;mask<(1U<<m.variables.size());++mask){
    std::vector<double> point(m.variables.size());
    for(unsigned i=0;i<point.size();++i)point[i]=(mask>>i)&1U;
    bool valid=true;
    for(const auto& variable:m.variables)
      valid=valid && point[variable.variable.id]>=variable.lower &&
        point[variable.variable.id]<=variable.upper;
    for(const auto& row:m.rows)if(row.active){
      double value=0;for(auto t:row.terms)value+=t.coefficient*point[t.variable.id];
      valid=valid && value>=row.lower && value<=row.upper;
    }
    if(!valid)continue;
    double value=m.objective.offset;
    for(auto t:m.objective.terms)value+=t.coefficient*point[t.variable.id];
    if(witness.empty() || (m.objective.sense==ObjectiveSense::Minimize ? value<best:value>best)){
      best=value;witness=point;
    }
  }
  return {best,witness};
}
void check(const ModelSnapshot& s,const SolveResult& r,double optimum){
  if(r.termination!=Termination::Optimal)std::cerr<<to_string(r.termination)<<": "<<r.message<<'\n';
  assert(r.termination==Termination::Optimal && r.solution_validated);
  assert(r.model_id==s.model_id && r.revision==s.revision);
  assert(r.objective==optimum && r.best_bound==r.objective);
  assert(r.message.find("Automatic native policy:")==0);
  double value=s.objective.offset;
  for(auto t:s.objective.terms)value+=t.coefficient*r.values[t.variable.id];
  assert(value==optimum);
  for(const auto& v:s.variables){
    assert(r.values[v.variable.id]==0 || r.values[v.variable.id]==1);
    assert(r.values[v.variable.id]>=v.lower && r.values[v.variable.id]<=v.upper);
  }
  for(const auto& row:s.rows){double a=0;for(auto t:row.terms)a+=t.coefficient*r.values[t.variable.id];assert(a>=row.lower && a<=row.upper);}
}

void configuration(const SolveOptions& solve,bool lp) {
  NativeAutoOptions options;options.solve=solve;
  // Isolate the transformations so a disabled feature cannot be masked by a
  // different coordinator solving the fixture first.
  for(int feature=0;feature<3;++feature){
    Model m;std::vector<Variable> x;std::vector<Term> sum,cost;
    for(int i=0;i<6;++i){
      x.push_back(feature==0 && i==0 ? m.add_variable(VariableType::Binary,1,1):m.add_binary());
      sum.push_back({x.back(),1});cost.push_back({x.back(),feature==2?1.0:double(i+1)});
    }
    if(feature==1){
      m.add_row({{x[0],1},{x[1],1},{x[2],1}},1,2);
      m.add_row({{x[3],1},{x[4],1},{x[5],1}},1,2);
    } else m.add_row(sum,2,4);
    m.minimize(cost,-9);
    const auto source=m.snapshot();const auto expected=oracle(source).first;
    const char* markers[]={"Exact integer presolve (","independent components","duplicate-column symmetry"};
    for(bool enabled:{false,true}){
      options.settings={false,false,false,false};
      if(feature==0)options.settings.presolve=enabled;
      if(feature==1)options.settings.components=enabled;
      if(feature==2)options.settings.symmetry=enabled;
      const auto result=solve_native_auto_configured(source,options);
      check(source,result,expected);
      assert((result.message.find(markers[feature])!=std::string::npos)==enabled);
    }
  }
  auto knapsack=fixture(3);const auto source=knapsack.snapshot();
  const auto expected=oracle(source).first;
  options.settings={false,false,false,true};
  const auto enabled=solve_native_auto_configured(knapsack,options);check(source,enabled,expected);
  options.settings.knapsack=false;
  const auto disabled=solve_native_auto_configured(source,options);check(source,disabled,expected);
  // Turning off DP must also remove its selection priority: checked LP and
  // reliability become available for this eligible weighted binary knapsack.
  if(lp){
    assert(enabled.backend=="Gecode native");
    assert(enabled.message.find("eligible exact knapsack DP")!=std::string::npos);
    assert(disabled.backend.find("checked LP")!=std::string::npos);
    assert(disabled.message.find("eligible exact knapsack DP")==std::string::npos);
  }
  // A fixed column makes presolve produce a DP-eligible reduced model. Keep
  // components disabled so the selected reduced leaf remains observable.
  auto reduced=fixture(3);const auto fixed=reduced.add_variable(VariableType::Binary,1,1);
  auto cost=reduced.snapshot().objective.terms;cost.push_back({fixed,13});reduced.maximize(cost,-7);
  const auto reduced_source=reduced.snapshot();
  options.settings={true,false,true,false};
  const auto reduced_result=solve_native_auto_configured(reduced,options);
  check(reduced_source,reduced_result,oracle(reduced_source).first);
  assert(reduced_result.message.find("Exact integer presolve (")!=std::string::npos);
  if(lp)assert(reduced_result.backend.find("checked LP")!=std::string::npos);

  // All combinations keep the same original feasible set/objective, including
  // a disconnected model whose component leaves are eligible for knapsack DP.
  Model split;std::vector<Term> split_cost;
  for(int group=0;group<2;++group){std::vector<Term> row;
    for(int i=0;i<5;++i){const auto x=split.add_binary();row.push_back({x,double(i+1)});
      split_cost.push_back({x,double(i*3+group+1)});}
    split.add_row(row,-inf,7);
  }
  split.maximize(split_cost,11);const auto split_source=split.snapshot();
  const auto split_expected=oracle(split_source).first;
  for(unsigned mask=0;mask<16;++mask){
    options.settings={bool(mask&1),bool(mask&2),bool(mask&4),bool(mask&8)};
    check(split_source,solve_native_auto_configured(split,options),split_expected);
    auto stopped=options;stopped.solve.time_limit_seconds=0;
    assert(solve_native_auto_configured(split_source,stopped).termination==Termination::TimeLimit);
    stopped=options;stopped.solve.cancellation=std::make_shared<CancellationToken>();
    stopped.solve.cancellation->cancel();
    assert(solve_native_auto_configured(split_source,stopped).termination==Termination::Cancelled);
  }
}
}
int main(){
  SolveOptions o;o.backend=Backend::Native;o.guarantee=Guarantee::Exact;
  o.relative_gap=o.absolute_gap=0;o.time_limit_seconds=5;
  if(!native_capabilities().available){
    auto m=fixture(0);
    assert(solve_native_auto(m,o).termination==Termination::Unsupported);
    NativeAutoOptions configured;configured.solve=o;configured.settings={false,false,false,false};
    assert(solve_native_auto_configured(m,configured).termination==Termination::Unsupported);
    assert(solve_native_auto_configured(m.snapshot(),configured).termination==Termination::Unsupported);
    configured.solve.threads=0;
    assert(solve_native_auto_configured(m,configured).termination==Termination::InvalidModel);
    assert(solve(m,o).termination==Termination::Unsupported);
    std::cout<<"Automatic native disabled-backend checks passed\n";return 0;
  }
  const bool lp=native_lp_capabilities().available;
  configuration(o,lp);
  for(int kind=0;kind<5;++kind){
    auto m=fixture(kind);auto s=m.snapshot();auto expected=oracle(s);
    for(bool snapshot:{false,true}){
      auto r=snapshot ? solve(s,o):solve(m,o);check(s,r,expected.first);
      if(lp && kind!=3)assert(r.backend.find("checked LP")!=std::string::npos);
      else assert(r.backend==solve_native(s,o).backend);
      if(lp && (kind==2 || kind==4))assert(r.backend.find("frontier")!=std::string::npos);
    }
    auto started=o;for(unsigned i=0;i<s.variables.size();++i)started.primal_start.push_back({s.variables[i].variable,expected.second[i]});
    check(s,solve_native_auto(s,started),expected.first);
    auto defaults=o;defaults.relative_gap=1e-4;defaults.absolute_gap=1e-6;
    check(s,solve_native_auto(s,defaults),expected.first);
    auto stopped=o;stopped.time_limit_seconds=0;
    assert(solve_native_auto(s,stopped).termination==Termination::TimeLimit);
    stopped=o;stopped.cancellation=std::make_shared<CancellationToken>();stopped.cancellation->cancel();
    assert(solve_native_auto(s,stopped).termination==Termination::Cancelled);
    auto invalid=o;invalid.threads=0;
    assert(solve_native_auto(s,invalid).termination==Termination::InvalidModel);
    auto certified=o;certified.guarantee=Guarantee::Certified;
    assert(solve_native_auto(s,certified).termination==Termination::Unsupported);
    auto limited=o;limited.node_limit=1;auto r=solve_native_auto(s,limited);
    assert(r.termination==Termination::Optimal || r.termination==Termination::NodeLimit);
    if(r.best_bound)assert(s.objective.sense==ObjectiveSense::Minimize ? *r.best_bound<=expected.first:*r.best_bound>=expected.first);
    s.rows[0].terms[0].variable.id+=100;
    assert(solve_native_auto(s,o).termination==Termination::InvalidModel);
  }
  Model global;auto a=global.add_integer(0,2),b=global.add_integer(0,2);
  add_all_different(global,{a,b});global.minimize({{a,1},{b,1}});
  auto automatic=o;automatic.backend=Backend::Auto;auto g=solve(global,automatic);
  assert(g.termination==Termination::Optimal && g.objective==1 && g.backend=="Gecode native");
  Model large;for(int i=0;i<4100;++i)large.add_variable(VariableType::Binary,0,0);
  auto l=solve_native_auto(large,o);assert(l.termination==Termination::Optimal && l.objective==0);
  auto numeric=fixture(0);auto ns=numeric.snapshot();
  numeric.add_row({{ns.variables[0].variable,1000000001}},-inf,1000000001);
  auto nr=solve_native_auto(numeric,o);check(numeric.snapshot(),nr,oracle(numeric.snapshot()).first);
  // Presolve can now remove the redundant oversized row. A supplied start
  // deliberately skips transformations, still exercising numeric LP fallback.
  auto numeric_start=o;auto numeric_snapshot=numeric.snapshot();
  auto numeric_witness=oracle(numeric_snapshot).second;
  for(unsigned i=0;i<numeric_witness.size();++i)
    numeric_start.primal_start.push_back({numeric_snapshot.variables[i].variable,numeric_witness[i]});
  auto fallback=solve_native_auto(numeric,numeric_start);
  check(numeric_snapshot,fallback,oracle(numeric_snapshot).first);
  assert(fallback.backend=="Gecode native");
  if(capabilities(Backend::Highs).available){
    SolveOptions numerical;
    auto n=solve(numeric,numerical);
    assert(n.termination==Termination::Optimal && n.backend=="HiGHS");
  }
  Model moved;auto saved=std::move(moved);
  assert(solve_native_auto(moved,o).termination==Termination::InvalidModel);
  std::cout<<"Automatic native routing, exhaustive optima, starts and interruption checks passed\n";
}
