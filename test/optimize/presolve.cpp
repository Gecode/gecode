#include <gecode/optimize/presolve.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <set>

using namespace Gecode::Optimize;
namespace {
using Point = std::vector<std::int64_t>;
constexpr double inf = std::numeric_limits<double>::infinity();
bool feasible(const ModelSnapshot& model, const Point& values) {
  for (const auto& row : model.rows) if (row.active) {
    std::int64_t sum=0;
    for (const auto& term : row.terms) sum += static_cast<std::int64_t>(term.coefficient)*values[term.variable.id];
    if (sum < row.lower || sum > row.upper) return false;
  }
  return true;
}
std::int64_t objective(const ModelSnapshot& model, const Point& values) {
  auto sum=static_cast<std::int64_t>(model.objective.offset);
  for (const auto& term : model.objective.terms) sum += static_cast<std::int64_t>(term.coefficient)*values[term.variable.id];
  return sum;
}
std::set<Point> enumerate(const ModelSnapshot& model) {
  std::set<Point> points; Point values(model.variables.size());
  std::function<void(std::size_t)> visit=[&](std::size_t slot) {
    if(slot==values.size()) { if(feasible(model,values)) points.insert(values); return; }
    const auto& variable=model.variables[slot];
    if(!variable.active) { visit(slot+1); return; }
    for(auto x=static_cast<std::int64_t>(variable.lower);x<=variable.upper;++x) { values[slot]=x; visit(slot+1); }
  };
  visit(0); return points;
}
SolveResult candidate(const ModelSnapshot& model, const Point& point) {
  SolveResult result; result.model_id=model.model_id; result.revision=model.revision;
  for(auto value:point) result.values.push_back(static_cast<double>(value));
  for(const auto& variable:model.variables) result.active_variables.push_back(variable.active);
  // Deliberately untrusted objective/proof flags: postsolve must recompute.
  result.objective=-999; result.best_bound=-999; result.termination=Termination::Optimal;
  result.absolute_gap=0; result.relative_gap=0; result.solution_validated=false;
  return result;
}
void equivalent(const ModelSnapshot& source, const PresolveOptions& options={}) {
  const auto before=enumerate(source);
  auto result=presolve_integer(source,options);
  assert(result.model_id==source.model_id && result.revision==source.revision && result.guarantee==Guarantee::Exact);
  if(result.status==PresolveStatus::Infeasible) {
    assert(before.empty() && result.termination==Termination::Infeasible && !result.model && result.infeasible_row);
    return;
  }
  assert(result.model && (result.status==PresolveStatus::Fixpoint || result.status==PresolveStatus::Incomplete));
  const auto& artifact=*result.model;
  assert(artifact.original().model_id==source.model_id && artifact.original().revision==source.revision);
  assert(artifact.reduced().model_id!=source.model_id);
  assert(artifact.reduced().objective.sense==source.objective.sense);
  assert(artifact.variables().size()==source.variables.size() && artifact.rows().size()==source.rows.size());
  std::set<Point> reconstructed;
  for(const auto& point:enumerate(artifact.reduced())) {
    auto post=artifact.postsolve(candidate(artifact.reduced(),point));
    assert(post.exact_witness_validated && post.solution.has_solution());
    assert(post.solution.model_id==source.model_id && post.solution.revision==source.revision);
    assert(post.solution.termination==Termination::Unknown && !post.solution.best_bound);
    assert(!post.solution.absolute_gap && !post.solution.relative_gap && !post.solution.native_backend_gap);
    Point original(source.variables.size());
    for(std::size_t i=0;i<original.size();++i) {
      if(source.variables[i].active) original[i]=static_cast<std::int64_t>(post.solution.values[i]);
      else assert(std::isnan(post.solution.values[i]));
    }
    assert(feasible(source,original) && before.count(original));
    assert(reconstructed.insert(original).second);
    assert(post.solution.objective==objective(source,original));
    assert(objective(source,original)==objective(artifact.reduced(),point));
  }
  assert(reconstructed==before); // full feasible-set preservation, both directions
  for(const auto& point:before) {
    Point projected(artifact.reduced().variables.size());
    for(const auto& mapping:artifact.variables()) if(mapping.active) {
      if(mapping.fixed_value) assert(point[mapping.original.id]==*mapping.fixed_value);
      else projected[mapping.reduced->id]=point[mapping.original.id];
    }
    assert(feasible(artifact.reduced(),projected));
  }
  for(const auto& change:result.changes) {
    assert(change.variable.model_id==source.model_id && change.row.model_id==source.model_id);
    assert(source.rows[change.row.id].active);
    assert(change.side==PresolveBoundSide::Lower ? change.after>change.before : change.after<change.before);
  }
}
void signed_oracles() {
  // Every sign and both inequality senses, including non-divisible numerators.
  for(int a=-3;a<=3;++a) for(int b=-3;b<=3;++b) for(int rhs=-5;rhs<=5;++rhs) {
    Model model; auto x=model.add_integer(-2,2); auto y=model.add_integer(-2,2);
    model.add_row({{x,double(a)},{y,double(b)}},-inf,rhs);
    model.minimize({{x,-2},{y,3}},-4);
    equivalent(model.snapshot());
  }
  std::uint32_t state=9137;
  auto random=[&]() { state=state*1664525u+1013904223u; return state; };
  for(unsigned trial=0;trial<600;++trial) {
    Model model; std::vector<Variable> variables;
    for(unsigned i=0;i<3;++i) {
      if(i==0 && trial%3==0) variables.push_back(model.add_binary());
      else { const int lo=int(random()%3)-2; variables.push_back(model.add_integer(lo,lo+int(random()%3))); }
    }
    for(unsigned i=0,n=1+random()%4;i<n;++i) {
      std::vector<Term> terms;
      for(auto variable:variables) terms.push_back({variable,double(int(random()%7)-3)});
      const int lower=int(random()%13)-6, upper=lower+int(random()%7);
      const unsigned form=random()%3;
      model.add_row(terms,form==0 ? -inf:double(lower),form==1 ? inf:double(upper));
    }
    model.set_objective({{variables[0],-2},{variables[1],1},{variables[2],3}},
                        trial%2 ? ObjectiveSense::Minimize:ObjectiveSense::Maximize,-5);
    equivalent(model.snapshot());
    PresolveOptions partial; partial.max_passes=trial%2;
    equivalent(model.snapshot(),partial);
    partial.max_passes=100; partial.max_row_visits=trial%3;
    equivalent(model.snapshot(),partial);
  }
}
void substitution_and_history() {
  Model model; auto deleted=model.add_binary(); auto old=model.add_row({{deleted,1}},0,1);
  model.remove(old); model.remove(deleted);
  auto x=model.add_integer(2,2,"fixed"); auto y=model.add_integer(-2,2,"kept");
  auto row=model.add_row({{x,3},{y,-2}},4,8,"range");
  model.maximize({{x,-3},{y,2}},11);
  auto before=model.snapshot(); auto result=presolve_integer(model);
  equivalent(before);
  assert(model.revision()==before.revision && model.row(row).lower==4 && model.variable(y).lower==-2);
  assert(result.model && result.model->variables()[x.id].fixed_value==2);
  assert(!result.model->variables()[deleted.id].active && !result.model->rows()[old.id].active);
  assert(result.model->reduced().objective.offset==5 && result.model->reduced().objective.sense==ObjectiveSense::Maximize);
  assert(result.model->reduced().variables.size()==1 && result.model->reduced().variables[0].name=="kept");
  assert(result.model->variables()[y.id].lower==-1 && result.model->variables()[y.id].upper==1);
  auto post=result.model->postsolve(candidate(result.model->reduced(),{0}));
  assert(post.exact_witness_validated && post.solution.value(x)==2 && post.solution.value(y)==0 && post.solution.objective==5);
  model.set_bounds(y,1,1);
  assert(result.model->original().variables[y.id].lower==-2 && post.solution.value(y)==0);

  // Fixed substitution must shift a row that remains a genuine restriction.
  Model retained; auto f=retained.add_integer(-2,-2);
  auto u=retained.add_integer(0,2); auto v=retained.add_integer(0,2);
  auto restricted=retained.add_row({{f,-3},{u,1},{v,1}},7,8,"retained range");
  retained.minimize({{f,-4},{u,2},{v,-1}},-3);
  auto mapped=presolve_integer(retained);
  assert(mapped.model && mapped.model->rows()[restricted.id].reduced);
  assert(mapped.model->rows()[restricted.id].substituted_constant==6);
  const auto& private_row=mapped.model->reduced().rows[mapped.model->rows()[restricted.id].reduced->id];
  assert(private_row.lower==1 && private_row.upper==2 && private_row.name=="retained range");
  assert(mapped.model->reduced().objective.offset==5);
  equivalent(retained.snapshot());
  PresolveOptions no_passes; no_passes.max_passes=0;
  mapped=presolve_integer(retained,no_passes);
  assert(mapped.status==PresolveStatus::Incomplete && mapped.model && mapped.passes==0);
  assert(mapped.model->rows()[restricted.id].substituted_constant==6);
  equivalent(retained.snapshot(),no_passes);

  Model fixed; auto a=fixed.add_integer(1,1); auto b=fixed.add_integer(9007199254740992.0,9007199254740992.0);
  fixed.minimize({{a,1},{b,1}},-9007199254740992.0);
  auto collapsed=presolve_integer(fixed);
  assert(collapsed.model && collapsed.model->reduced().variables.empty());
  assert(collapsed.model->reduced().objective.offset==1);
  post=collapsed.model->postsolve(candidate(collapsed.model->reduced(),{}));
  assert(post.exact_witness_validated && post.solution.objective==1);

  Model equality; auto e=equality.add_integer(-2,2); equality.add_row({{e,-3}},-3,-3);
  equivalent(equality.snapshot());
  auto unique=presolve_integer(equality);
  assert(unique.model && unique.fixed_variables==1 && unique.model->variables()[e.id].fixed_value==1);
  Model empty; empty.minimize({},-9); equivalent(empty.snapshot());
  empty.add_row({},1,inf); equivalent(empty.snapshot());

  Model parity; auto p=parity.add_binary(); auto q=parity.add_binary(); auto r=parity.add_binary();
  parity.add_row({{p,2},{q,2},{r,2}},3,3);
  auto incomplete_knowledge=presolve_integer(parity);
  assert(incomplete_knowledge.status==PresolveStatus::Fixpoint && incomplete_knowledge.model);
  assert(enumerate(parity.snapshot()).empty()); // fixpoint is not a feasibility/optimality decision
  equivalent(parity.snapshot());
}
void limits() {
  Model model; auto x=model.add_integer(0,10); auto y=model.add_integer(0,10); auto z=model.add_integer(0,10);
  model.add_row({{x,1},{y,-1}},-inf,0); model.add_row({{y,1},{z,-1}},-inf,0); model.add_row({{z,1}},-inf,2);
  PresolveOptions options; options.max_passes=1;
  auto result=presolve_integer(model,options);
  assert(result.status==PresolveStatus::Incomplete && result.termination==Termination::IterationLimit && result.model);
  assert(result.passes==1 && result.model->variables()[z.id].upper==2 && result.model->variables()[x.id].upper==10);
  equivalent(model.snapshot(),options);
  options.max_passes=100; options.max_row_visits=1;
  result=presolve_integer(model,options);
  assert(result.model && result.row_visits==1 && result.status==PresolveStatus::Incomplete);
  equivalent(model.snapshot(),options);
  options.max_row_visits.reset(); result=presolve_integer(model,options);
  assert(result.status==PresolveStatus::Fixpoint && result.passes==4 && result.model->variables()[x.id].upper==2);
  options.time_limit_seconds=0; result=presolve_integer(model,options);
  assert(result.status==PresolveStatus::Incomplete && result.termination==Termination::TimeLimit && !result.model && !result.infeasible_row);
  options.time_limit_seconds=-1;
  assert(presolve_integer(model,options).termination==Termination::InvalidModel);
  options.time_limit_seconds=inf; options.cancellation=std::make_shared<CancellationToken>(); options.cancellation->cancel();
  result=presolve_integer(model,options);
  assert(result.termination==Termination::Cancelled && !result.model && !result.infeasible_row);
}
void unsupported_and_overflow() {
  auto unsupported=[](const Model& model) {
    auto result=presolve_integer(model);
    assert(result.status==PresolveStatus::Unsupported && result.termination==Termination::Unsupported);
    assert(!result.model && !result.infeasible_row);
  };
  Model continuous; continuous.add_continuous(0,1); unsupported(continuous);
  Model semi; semi.add_variable(VariableType::SemiInteger,2,4); unsupported(semi);
  Model unbounded; unbounded.add_integer(0,inf); unsupported(unbounded);
  Model fractional; fractional.add_integer(0.25,1); unsupported(fractional);
  Model coefficient; auto c=coefficient.add_binary(); coefficient.add_row({{c,0.5}},0,1); unsupported(coefficient);
  Model sides; auto s=sides.add_binary(); sides.add_row({{s,1}},0.5,1); unsupported(sides);
  Model offset; offset.minimize({},0.5); unsupported(offset);
  Model logical; auto b=logical.add_binary(); auto x=logical.add_integer(0,2);
  auto indicator=add_indicator(logical,b,true,{{x,1}},1,inf); unsupported(logical);
  remove_indicator(logical,indicator.indicator); equivalent(logical.snapshot());
  auto global=add_all_different(logical,{b,x}); unsupported(logical);
  logical.remove(global); equivalent(logical.snapshot());
  auto malformed=logical.snapshot(); malformed.variables[0].variable.model_id++;
  assert(presolve_integer(malformed).status==PresolveStatus::InvalidModel);
  Model product; auto large=product.add_integer(4294967296.0,4294967296.0);
  product.add_row({{large,4294967296.0}},-inf,0); unsupported(product);
  Model sum; std::vector<Term> terms;
  for(int i=0;i<4;++i) terms.push_back({sum.add_integer(2147483648.0,2147483648.0),1073741824.0});
  sum.add_row(terms,-inf,0); unsupported(sum);
  Model quotient; auto a=quotient.add_integer(0,1); auto y=quotient.add_integer(-2147483647.0,2147483647.0);
  auto constant=quotient.add_integer(4294967295.0,4294967295.0);
  quotient.add_row({{a,-1},{y,4294967296.0},{constant,1}},-1,inf);
  auto division=presolve_integer(quotient);
  assert(division.status==PresolveStatus::Unsupported && division.message.find("division")!=std::string::npos && !division.model);
  Model export_range; auto small=export_range.add_integer(1,1);
  auto big=export_range.add_integer(9007199254740992.0,9007199254740992.0);
  export_range.minimize({{small,1},{big,1}}); unsupported(export_range);
}
void postsolve_guards() {
  Model model; auto x=model.add_integer(0,2); model.minimize({{x,3}},4);
  auto result=presolve_integer(model); const auto& artifact=*result.model;
  auto raw=candidate(artifact.reduced(),{1}); raw.values[0]+=4e-7;
  auto good=artifact.postsolve(raw); assert(good.exact_witness_validated && good.solution.objective==7);
  assert(!good.solution.best_bound && good.solution.termination==Termination::Unknown);
  assert(artifact.postsolve(raw,0).solution.termination==Termination::NumericalFailure);
  raw=candidate(artifact.reduced(),{1}); raw.model_id++;
  assert(artifact.postsolve(raw).solution.termination==Termination::InvalidModel);
  raw=candidate(artifact.reduced(),{1}); raw.revision++;
  assert(artifact.postsolve(raw).solution.termination==Termination::InvalidModel);
  raw=candidate(artifact.reduced(),{1}); raw.active_variables[0]=false;
  assert(artifact.postsolve(raw).solution.termination==Termination::InvalidModel);
  raw=candidate(artifact.reduced(),{1}); raw.values[0]=std::numeric_limits<double>::quiet_NaN();
  assert(artifact.postsolve(raw).solution.termination==Termination::NumericalFailure);
  raw=candidate(artifact.reduced(),{3});
  assert(artifact.postsolve(raw).solution.termination==Termination::NumericalFailure);
  raw=candidate(artifact.reduced(),{1}); raw.guarantee=Guarantee::Certified;
  assert(artifact.postsolve(raw).solution.termination==Termination::Unsupported);
  assert(artifact.postsolve(raw,0.5).solution.termination==Termination::InvalidModel);
  Model row_model; auto a=row_model.add_integer(0,2); auto b=row_model.add_integer(0,2);
  row_model.add_row({{a,1},{b,1}},-inf,2);
  auto rows=presolve_integer(row_model);
  auto invalid=rows.model->postsolve(candidate(rows.model->reduced(),{2,2}));
  assert(invalid.solution.termination==Termination::NumericalFailure && !invalid.exact_witness_validated && !invalid.solution.has_solution());
  Model future; auto snapshot=future.snapshot(); ++snapshot.model_id;
  auto distinct=presolve_integer(snapshot);
  assert(distinct.model && distinct.model->reduced().model_id!=snapshot.model_id);
}
}
int main() {
  signed_oracles(); substitution_and_history(); limits(); unsupported_and_overflow(); postsolve_guards();
}
