#include <gecode/optimize.hh>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>

namespace O=Gecode::Optimize;
constexpr double inf=std::numeric_limits<double>::infinity();
using Assignment=std::vector<int>;

// Enumerate the complete finite product using predicates independent of the
// library's global validator and native compiler. Check both acceptance and
// optimal objective, then compare the native search result with that oracle.
static void oracle(O::Model& model,const std::vector<std::pair<int,int>>& domains,
                   const std::function<bool(const Assignment&)>& predicate) {
  auto snapshot=model.snapshot();assert(domains.size()==snapshot.variables.size());
  double best=snapshot.objective.sense==O::ObjectiveSense::Minimize?inf:-inf;
  std::size_t feasible=0;
  Assignment assignment(domains.size());
  const auto visit=[&](auto&& self,std::size_t index)->void {
    if(index<domains.size()) {
      for(int value=domains[index].first;value<=domains[index].second;++value) {
        assignment[index]=value;self(self,index+1);
      }
      return;
    }
    const bool expected=predicate(assignment);
    const std::vector<double> values(assignment.begin(),assignment.end());
    const auto checked=O::validate(snapshot,values,0,0);
    assert(checked.model_valid && checked.valid==expected);
    if(!expected) return;
    ++feasible;
    double objective=snapshot.objective.offset;
    for(const auto& term:snapshot.objective.terms) objective+=term.coefficient*values[term.variable.id];
    if(snapshot.objective.sense==O::ObjectiveSense::Minimize) best=std::min(best,objective);
    else best=std::max(best,objective);
  };
  visit(visit,0);
  O::SolveOptions options;options.backend=O::Backend::Native;options.guarantee=O::Guarantee::Exact;
  const auto result=O::solve(snapshot,options);
  if(!O::capabilities(O::Backend::Native).available) {
    assert(result.termination==O::Termination::Unsupported);return;
  }
  if(feasible) {
    if(result.termination!=O::Termination::Optimal) std::cerr<<result.message<<'\n';
    assert(result.termination==O::Termination::Optimal && result.has_solution());
    assert(result.guarantee==O::Guarantee::Exact && *result.objective==best && *result.best_bound==best);
    assert(O::validate(snapshot,result.values,0,0).valid);
  } else assert(result.termination==O::Termination::Infeasible && !result.has_solution());
}

template<class F> static void invalid(F operation) {bool caught=false;try{operation();}catch(const O::ModelError&){caught=true;}assert(caught);}

int main() {
  O::Model different;std::vector<O::Variable> v;
  for(int i=0;i<4;++i) v.push_back(different.add_integer(-1,2));
  auto all=O::add_all_different(different,v,"permutation");
  different.minimize({{v[0],1},{v[1],2},{v[2],3},{v[3],4}},-7);
  oracle(different,{{-1,2},{-1,2},{-1,2},{-1,2}},[](const auto& a){auto b=a;std::sort(b.begin(),b.end());return std::adjacent_find(b.begin(),b.end())==b.end();});
  O::Model aliases;auto x=aliases.add_integer(0,2);
  O::add_all_different(aliases,{x,x});oracle(aliases,{{0,2}},[](const auto&){return false;});

  O::Model element;auto index=element.add_integer(-2,2),a=element.add_integer(0,2),b=element.add_integer(-1,1);
  O::add_element(element,index,{a,a,b},index,-1);
  element.maximize({{index,3},{a,-2},{b,1}},4);
  oracle(element,{{-2,2},{0,2},{-1,1}},[](const auto& q){return q[0]>=-1 && q[0]<=1 && q[0]==(q[0]<1?q[1]:q[2]);});
  O::Model empty_element;auto e=empty_element.add_integer(0,1);
  O::add_element(empty_element,e,{},e);oracle(empty_element,{{0,1}},[](const auto&){return false;});

  O::Model table;auto tx=table.add_integer(0,2),ty=table.add_integer(0,2);
  O::add_table(table,{tx,ty,tx},{{0,0,1},{1,2,1},{2,0,2},{1,2,1}});
  table.minimize({{tx,3},{ty,-1}},-9);
  oracle(table,{{0,2},{0,2}},[](const auto& q){return (q[0]==1 && q[1]==2)||(q[0]==2 && q[1]==0);});
  O::Model false_table;O::add_table(false_table,{},{});oracle(false_table,{},[](const auto&){return false;});
  O::Model true_table;O::add_table(true_table,{},{{}});oracle(true_table,{},[](const auto&){return true;});

  O::Model schedule;auto s0=schedule.add_integer(0,4),s1=schedule.add_integer(0,4),s2=schedule.add_integer(0,4);
  O::add_cumulative(schedule,{s0,s1,s2},{2,2,0},{1,1,100},1);
  schedule.minimize({{s0,2},{s1,1},{s2,10}});
  oracle(schedule,{{0,4},{0,4},{0,4}},[](const auto& q){
    for(int time=0;time<7;++time) if((q[0]<=time && time<q[0]+2)+(q[1]<=time && time<q[1]+2)>1) return false;
    return true;
  });
  for(int capacity=1;capacity<=2;++capacity) {
    O::Model same_start;auto s=same_start.add_integer(-2,2);
    O::add_cumulative(same_start,{s,s},{2,2},{1,1},capacity);
    oracle(same_start,{{-2,2}},[capacity](const auto&){return capacity==2;});
  }
  O::Model no_resource;auto start=no_resource.add_integer(-2,2);
  O::add_cumulative(no_resource,{start,start},{0,2},{99,0},0);
  oracle(no_resource,{{-2,2}},[](const auto&){return true;});

  O::Model circuit;std::vector<O::Variable> successors;
  for(int i=0;i<4;++i) successors.push_back(circuit.add_integer(-2,1));
  O::add_circuit(circuit,successors,-2);
  circuit.maximize({{successors[0],1},{successors[1],2},{successors[2],3},{successors[3],4}},8);
  oracle(circuit,{{-2,1},{-2,1},{-2,1},{-2,1}},[](const auto& q){
    bool seen[4]={};int node=0;
    for(int step=0;step<4;++step){if(seen[node])return false;seen[node]=true;node=q[node]+2;}
    return node==0;
  });
  O::Model singleton;auto successor=singleton.add_integer(5,9);
  O::add_circuit(singleton,{successor},7);oracle(singleton,{{5,9}},[](const auto& q){return q[0]==7;});
  O::Model same_successor;auto s=same_successor.add_integer(0,1);
  O::add_circuit(same_successor,{s,s});oracle(same_successor,{{0,1}},[](const auto&){return false;});

  // Old numerical workflows must reject globals rather than solve a relaxation
  // as if it represented the original model.
  const auto automatic=O::solve(different);
  if(O::capabilities(O::Backend::Native).available) {
    assert(automatic.has_solution() && automatic.backend=="Gecode native");
    const O::ObjectiveData total{{{v[0],1},{v[1],1},{v[2],1},{v[3],1}},0,O::ObjectiveSense::Minimize};
    const O::ObjectiveData prefer{{{v[0],1}},0,O::ObjectiveSense::Maximize};
    const auto ordered=O::solve_lexicographic(different,{{total,0,0,"total"},{prefer,0,0,"first"}});
    assert(ordered.completed_numerically() && ordered.objective_values==std::vector<double>({2,2}));
    assert(O::validate(different.snapshot(),ordered.final_solution.values,0,0).valid);
  } else assert(automatic.termination==O::Termination::Unsupported);
  O::SolveOptions highs;highs.backend=O::Backend::Highs;
  assert(O::solve(different,highs).termination==O::Termination::Unsupported);
  O::SolveSession session;assert(session.solve(different).termination==O::Termination::Unsupported);
  assert(O::analyze_conflict(different).status==O::ConflictStatus::Unsupported);
  invalid([&]{O::write_model(different,"unsupported-global.lp");});

  const auto revision=different.revision();const auto snapshot=different.snapshot();
  invalid([&]{O::add_table(different,{v[0]},{{1,2}});});
  invalid([&]{O::add_cumulative(different,{v[0]},{-1},{1},1);});
  invalid([&]{O::add_circuit(different,{});});
  invalid([&]{O::add_all_different(different,{x});});
  assert(different.revision()==revision && different.snapshot().globals.size()==snapshot.globals.size());
  invalid([&]{different.remove(v[0]);});
  auto malformed=snapshot;malformed.globals[0].global.model_id=0;
  assert(!O::validate(malformed,{-1,0,1,2}).model_valid);
  malformed=snapshot;std::get<O::AllDifferentData>(malformed.globals[0].payload).variables[0]=x;
  assert(!O::validate(malformed,{-1,0,1,2}).model_valid);
  different.set_name(all,"renamed");assert(different.global(all).name=="renamed");
  different.remove(all);invalid([&]{different.remove(all);});
  different.minimize({});different.remove(v[0]);
  O::validate_structure(different.snapshot()); // Inactive global can refer to tombstones.
  O::Model moved(std::move(different));O::validate_structure(moved.snapshot());
  invalid([&]{different.add_global(O::AllDifferentData{});});

  O::Model huge;auto h0=huge.add_integer(0,0),h1=huge.add_integer(0,0);
  O::add_cumulative(huge,{h0,h1},{1,1},{INT64_C(9007199254740992),INT64_C(9007199254740992)},INT64_C(9007199254740992));
  const auto failed=O::validate(huge.snapshot(),{0,0},0,0);
  assert(failed.model_valid && !failed.valid && failed.violated_globals==1);
  O::SolveOptions exact;exact.backend=O::Backend::Native;exact.guarantee=O::Guarantee::Exact;
  assert(O::solve(huge,exact).termination==O::Termination::Unsupported);

  // The previous int64 energy/width guards admitted these six tasks, but
  // C*est+sum(energy) overflows Omega/Lambda envelopes. Reject before posting.
  O::Model envelope;std::vector<O::Variable> tasks;
  for(int i=0;i<6;++i) tasks.push_back(envelope.add_integer(715827880,715827880));
  O::add_cumulative(envelope,tasks,std::vector<std::int64_t>(6,1431655760),
                    std::vector<std::int64_t>(6,1073741823),2147483646);
  const auto unsafe_envelope=O::solve(envelope,exact);
  assert(unsafe_envelope.termination==O::Termination::Unsupported && !unsafe_envelope.has_solution());

  // Exactly at the conservative edge-finding int-cast guard and just outside.
  // C=2, h_min=1, E=2 gives 4*B+2 <= native_max=2147483646.
  for(int sign:{-1,1}) for(int outside:{0,1}) {
    const int magnitude=536870911+outside;
    const int begin=sign>0?magnitude-1:-magnitude;
    O::Model boundary;auto p=boundary.add_integer(begin,begin),q=boundary.add_integer(begin,begin);
    O::add_cumulative(boundary,{p,q},{1,1},{1,1},2);
    if(outside) assert(O::solve(boundary,exact).termination==O::Termination::Unsupported);
    else oracle(boundary,{{begin,begin},{begin,begin}},[](const auto&){return true;});
  }
  // Singleton and zero-resource constraints need no envelope arithmetic.
  O::Model lone;auto late=lone.add_integer(2147483645,2147483645);
  O::add_cumulative(lone,{late},{1},{1},10);
  oracle(lone,{{2147483645,2147483645}},[](const auto&){return true;});
  O::Model no_energy;auto late_zero=no_energy.add_integer(2147483646,2147483646);
  O::add_cumulative(no_energy,{late_zero,late_zero},{2147483646,0},{0,2147483646},0);
  oracle(no_energy,{{2147483646,2147483646}},[](const auto&){return true;});
  O::Model overloaded;auto overload_start=overloaded.add_integer(-2,2);
  O::add_cumulative(overloaded,{overload_start},{2},{2},1);
  oracle(overloaded,{{-2,2}},[](const auto&){return false;});

  // Native int addition of the two minimum heights overflowed here. The wide
  // disjunctive test must preserve exact non-overlap semantics instead.
  O::Model resource_units;auto r0=resource_units.add_integer(0,2),r1=resource_units.add_integer(0,2);
  O::add_cumulative(resource_units,{r0,r1},{1,1},{1073741823,2147483646},2147483646);
  resource_units.minimize({{r0,1},{r1,1}});
  oracle(resource_units,{{0,2},{0,2}},[](const auto& q){return q[0]!=q[1];});

  // The advanced propagator indexes count*distinct_heights using int.
  O::Model array_product;auto repeated=array_product.add_integer(0,0);
  std::vector<std::int64_t> distinct_heights(46341);
  for(std::size_t i=0;i<distinct_heights.size();++i) distinct_heights[i]=static_cast<std::int64_t>(i+1);
  O::add_cumulative(array_product,std::vector<O::Variable>(46341,repeated),
                    std::vector<std::int64_t>(46341,1),distinct_heights,46341);
  assert(O::solve(array_product,exact).termination==O::Termination::Unsupported);
  std::cout<<"native globals and independent finite-product oracles passed\n";
}
