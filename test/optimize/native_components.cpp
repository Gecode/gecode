#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native_preprocess.hpp>
#include <gecode/optimize/validate.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>

using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
// A tiny independent exhaustive continuation; every enumeration consumes the
// supplied budget. No solver implementation or common validator chooses values.
SolveResult oracle(const ModelSnapshot& m,const SolveOptions& o,SolveBudget& budget) {
  SolveResult r;r.model_id=m.model_id;r.revision=m.revision;r.guarantee=o.guarantee;
  r.backend="test exhaustive oracle";r.backend_version="1";
  std::vector<double> values(m.variables.size());bool found=false,stopped=false;
  double best=m.objective.sense==ObjectiveSense::Minimize?inf:-inf;
  std::function<void(std::size_t)> visit=[&](std::size_t index) {
    if(stopped)return;
    if(auto reason=budget.stop_reason()){r.termination=*reason;stopped=true;return;}
    if(index<values.size()) {
      for(double v=m.variables[index].lower;v<=m.variables[index].upper;++v){values[index]=v;visit(index+1);}
      return;
    }
    budget.add_nodes();
    for(const auto& row:m.rows)if(row.active) {
      double lhs=0;for(auto t:row.terms)lhs+=t.coefficient*values[t.variable.id];
      if(lhs<row.lower || lhs>row.upper)return;
    }
    double cost=m.objective.offset;for(auto t:m.objective.terms)cost+=t.coefficient*values[t.variable.id];
    if(!found || (m.objective.sense==ObjectiveSense::Minimize?cost<best:cost>best)) {
      found=true;best=cost;r.values=values;
    }
  };
  visit(0);
  if(stopped){r.values.clear();return r;}
  r.termination=found?Termination::Optimal:Termination::Infeasible;
  if(found){r.objective=best;r.best_bound=best;r.solution_validated=true;r.active_variables.assign(values.size(),true);r.update_gaps(m.objective.sense);}
  return r;
}

SolveOptions options() {
  SolveOptions o;o.backend=Backend::Native;o.guarantee=Guarantee::Exact;
  o.relative_gap=o.absolute_gap=0;return o;
}
Model fixture(ObjectiveSense sense) {
  Model m;auto gone=m.add_binary();m.remove(gone);
  auto a=m.add_integer(-2,3),b=m.add_binary(),c=m.add_integer(-3,2),d=m.add_binary();
  m.add_row({{a,1},{b,2}},1,3);
  m.add_row({{c,1}},-2,1);
  auto removed=m.add_row({{a,1},{c,1}},-inf,100);m.remove(removed);
  m.add_row({},-1,1);
  m.set_objective({{a,-3},{b,2},{c,4},{d,-7}},sense,19);
  return m;
}
}

int main() {
  auto o=options();
  for(auto sense:{ObjectiveSense::Minimize,ObjectiveSense::Maximize}) {
    auto m=fixture(sense);auto s=m.snapshot();SolveBudget budget(o);unsigned calls=0;
    auto r=Detail::native_components(s,o,budget,[&](const ModelSnapshot& part,const SolveOptions& same,SolveBudget& shared){
      assert(&shared==&budget && &same==&o);assert(part.objective.offset==0);++calls;
      return oracle(part,same,shared);
    });
    assert(r && calls==3 && r->termination==Termination::Optimal && r->has_solution());
    assert(r->model_id==s.model_id && r->revision==s.revision && r->values.size()==5);
    assert(!r->active_variables[0] && std::isnan(r->values[0]));
    // min: (a,b)=(3,0), c=-2,d=1; max: (a,b)=(-1,1), c=1,d=0.
    assert(r->objective==(sense==ObjectiveSense::Minimize?-5:28));
    assert(r->best_bound==r->objective && r->absolute_gap==0 && r->relative_gap==0);
    assert(validate(s,r->values,0,0).valid);
  }
  Model coupled;auto a=coupled.add_binary(),b=coupled.add_binary();coupled.add_row({{a,1},{b,-1}},0,0);
  SolveBudget coupled_budget(o);unsigned calls=0;
  auto no_call=[&](const ModelSnapshot&,const SolveOptions&,SolveBudget&)->SolveResult{++calls;assert(false);return {};};
  assert(!Detail::native_components(coupled.snapshot(),o,coupled_budget,no_call) && calls==0);
  auto m=fixture(ObjectiveSense::Minimize);auto s=m.snapshot();
  auto start=o;start.primal_start.push_back({s.variables[1].variable,1});SolveBudget start_budget(start);
  assert(!Detail::native_components(s,start,start_budget,no_call));
  auto fractional=s;fractional.objective.offset=0.5;SolveBudget fractional_budget(o);
  assert(!Detail::native_components(fractional,o,fractional_budget,no_call));
  Model many;for(unsigned i=0;i<65;++i)many.add_binary();SolveBudget many_budget(o);
  assert(!Detail::native_components(many.snapshot(),o,many_budget,no_call));
  Model impossible;impossible.add_binary();impossible.add_binary();impossible.add_row({},1,inf);
  SolveBudget impossible_budget(o);
  auto bad=Detail::native_components(impossible.snapshot(),o,impossible_budget,oracle);
  assert(bad && bad->termination==Termination::Infeasible && !bad->has_solution() && !bad->best_bound);
  auto cancelled=o;cancelled.cancellation=std::make_shared<CancellationToken>();cancelled.cancellation->cancel();SolveBudget cancel_budget(cancelled);
  auto stop=Detail::native_components(s,cancelled,cancel_budget,no_call);
  assert(stop && stop->termination==Termination::Cancelled && !stop->has_solution());
  auto limited=o;limited.node_limit=13;SolveBudget node_budget(limited);calls=0;
  auto partial=Detail::native_components(s,limited,node_budget,[&](const ModelSnapshot& part,const SolveOptions& same,SolveBudget& shared){
    ++calls;return oracle(part,same,shared);
  });
  assert(partial && calls==2 && partial->termination==Termination::NodeLimit);
  assert(node_budget.nodes()==13 && !partial->has_solution() && partial->values.empty() && !partial->objective && !partial->best_bound);
  // A second component must not turn its local witness into an original point.
  SolveBudget partial_budget(o);calls=0;
  auto interrupted=Detail::native_components(s,o,partial_budget,[&](const ModelSnapshot& part,const SolveOptions& same,SolveBudget& shared){
    auto local=oracle(part,same,shared);if(++calls==2)local.termination=Termination::TimeLimit;return local;
  });
  assert(interrupted && interrupted->termination==Termination::TimeLimit && !interrupted->has_solution() && !interrupted->best_bound);
  // Independently reject a claimed optimum carrying an infeasible local point.
  SolveBudget dishonest_budget(o);
  auto dishonest=Detail::native_components(s,o,dishonest_budget,[&](const ModelSnapshot& part,const SolveOptions& same,SolveBudget& shared){
    auto local=oracle(part,same,shared);local.values[0]=100;return local;
  });
  assert(dishonest && dishonest->termination==Termination::BackendError && !dishonest->has_solution());
  std::cout<<"Independent component optima, offsets, mapping, infeasibility and shared budgets passed\n";
}
