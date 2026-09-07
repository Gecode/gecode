#include <gecode/optimize/pool.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <set>

using namespace Gecode::Optimize;
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();
[[maybe_unused]] bool near(double a, double b) { return std::fabs(a-b) <= 1e-6; }
void historical(const ModelSnapshot& model, const PoolResult& result) {
  assert(result.model_id == model.model_id && result.revision == model.revision);
  std::set<std::vector<std::int64_t>> keys;
  for (const auto& entry : result.entries) {
    assert(keys.insert(entry.projection_values).second);
    assert(entry.solution.has_solution());
    assert(entry.solution.model_id == model.model_id && entry.solution.revision == model.revision);
    assert(entry.solution.values.size() == model.variables.size());
    assert(entry.solution.termination == Termination::Unknown);
    assert(!entry.solution.best_bound && !entry.solution.absolute_gap && !entry.solution.relative_gap);
    auto checked = validate(model, entry.solution.values);
    assert(checked.valid && checked.objective == entry.solution.objective);
    for (std::size_t i=0; i<result.projection.size(); ++i)
      assert(entry.solution.value(result.projection[i]) == entry.projection_values[i]);
  }
}
}

#ifdef GECODE_POOL_TEST_FAKE_SOLVER
namespace {
enum class Scenario { Good, FirstLimit, SecondLimit, FirstCancel, SecondCancel,
  BadOwner, BadRevision, BadMask, BadPrimal, MissingBound, BadBound, OpenBound,
  BadObjective, Duplicate, Ambiguous, BadInfeasibleOwner, WrongGuarantee,
  ReversedRank, ReversedLimitedRank, UnvalidatedFeasible, NanInfeasible, OffsetHonest, OffsetForged, FirstMemory, ReversedInfeasible };
Scenario scenario = Scenario::Good;
unsigned calls = 0;
double remaining = inf;
std::vector<double> first;
// Independent small exact arithmetic oracle, including every private binary
// witness. Does not call the implementation's validator or no-good generator.
std::vector<double> enumerate(const ModelSnapshot& model, bool reverse) {
  std::vector<double> values(model.variables.size()), best;
  double optimum = model.objective.sense == ObjectiveSense::Minimize ? inf : -inf;
  std::function<void(std::size_t)> visit = [&](std::size_t slot) {
    if (slot != model.variables.size()) {
      const auto& variable = model.variables[slot];
      if (!variable.active) { values[slot] = 0; visit(slot+1); return; }
      assert(variable.type == VariableType::Integer || variable.type == VariableType::Binary);
      assert(std::isfinite(variable.lower) && std::isfinite(variable.upper));
      for (int x=static_cast<int>(std::ceil(variable.lower)); x<=std::floor(variable.upper); ++x) {
        values[slot]=x; visit(slot+1);
      }
      return;
    }
    for (const auto& row : model.rows) if (row.active) {
      double activity=0; for (const auto& term : row.terms) activity += term.coefficient*values[term.variable.id];
      if (activity < row.lower || activity > row.upper) return;
    }
    double objective=model.objective.offset;
    for (const auto& term : model.objective.terms) objective += term.coefficient*values[term.variable.id];
    const bool minimize = (model.objective.sense == ObjectiveSense::Minimize) != reverse;
    if (best.empty() || (minimize ? objective<optimum : objective>optimum)) {
      best=values; optimum=objective;
    }
  };
  visit(0); return best;
}
}
namespace Gecode { namespace Optimize {
SolveResult solve(const ModelSnapshot& model, const SolveOptions& options) {
  assert(options.relative_gap == 0 && options.absolute_gap == 0);
  assert(options.time_limit_seconds <= remaining); remaining=options.time_limit_seconds;
  if (calls != 0) assert(options.primal_start.empty());
  SolveResult result;
  result.model_id=model.model_id; result.revision=model.revision;
  result.guarantee=options.guarantee;
  result.values=enumerate(model, calls==0 && (scenario==Scenario::ReversedRank || scenario==Scenario::ReversedLimitedRank || scenario==Scenario::ReversedInfeasible));
  for (const auto& variable : model.variables) result.active_variables.push_back(variable.active);
  result.termination = result.values.empty() ? Termination::Infeasible : Termination::Optimal;
  if (!result.values.empty()) {
    result.solution_validated=true; result.objective=model.objective.offset;
    for (const auto& term : model.objective.terms) *result.objective += term.coefficient*result.values[term.variable.id];
    if (scenario==Scenario::OffsetHonest) result.objective=1;
    if (scenario==Scenario::OffsetForged) result.objective=0;
    result.best_bound=result.objective; result.update_gaps(model.objective.sense);
    if (calls==0) first=result.values;
  }
  if (scenario==Scenario::BadOwner || scenario==Scenario::BadInfeasibleOwner) ++result.model_id;
  if (scenario==Scenario::BadRevision) ++result.revision;
  if (scenario==Scenario::BadMask) result.active_variables[0]=false;
  if (scenario==Scenario::BadPrimal) result.values[0]=99;
  if (scenario==Scenario::MissingBound) { result.best_bound.reset(); result.absolute_gap.reset(); }
  if (scenario==Scenario::BadBound) result.best_bound=*result.objective+1;
  if (scenario==Scenario::OpenBound) result.best_bound=*result.objective-1;
  if (scenario==Scenario::BadObjective) { result.objective=0; result.best_bound=0; }
  if (scenario==Scenario::WrongGuarantee) result.guarantee=Guarantee::Certified;
  if (scenario==Scenario::Duplicate && calls==1) {
    result.values.assign(model.variables.size(), 0);
    std::copy(first.begin(), first.end(), result.values.begin());
  }
  if ((scenario==Scenario::FirstLimit && calls==0) || (scenario==Scenario::SecondLimit && calls==1) ||
      (scenario==Scenario::ReversedLimitedRank && calls==1)) result.termination=Termination::TimeLimit;
  if (scenario==Scenario::FirstMemory) result.termination=Termination::MemoryLimit;
  if ((scenario==Scenario::FirstCancel && calls==0) || (scenario==Scenario::SecondCancel && calls==1)) options.cancellation->cancel();
  if (scenario==Scenario::Ambiguous || scenario==Scenario::BadInfeasibleOwner) {
    result.solution_validated=false; result.values.clear(); result.objective.reset(); result.best_bound.reset();
    result.termination=scenario==Scenario::Ambiguous ? Termination::InfeasibleOrUnbounded : Termination::Infeasible;
  }
  if (scenario==Scenario::UnvalidatedFeasible || (scenario==Scenario::ReversedInfeasible && calls==1)) {
    result.solution_validated=false; result.termination=Termination::Infeasible;
  }
  if (scenario==Scenario::NanInfeasible) {
    result.solution_validated=false; result.values.clear(); result.termination=Termination::Infeasible;
    result.objective=std::numeric_limits<double>::quiet_NaN();
  }
  ++calls; return result;
}
}}
int main() {
  Model model; auto x=model.add_integer(0,2); auto y=model.add_binary();
  model.minimize({{x,1},{y,3}},7);
  PoolOptions options; options.projection=std::vector<Variable>{x}; options.solve.time_limit_seconds=60;
  options.solve.primal_start={{x,2},{y,1}};
  for (auto selected : {Scenario::Good,Scenario::FirstLimit,Scenario::SecondLimit,Scenario::FirstCancel,
       Scenario::SecondCancel,Scenario::BadOwner,Scenario::BadRevision,Scenario::BadMask,Scenario::BadPrimal,
       Scenario::MissingBound,Scenario::BadBound,Scenario::OpenBound,Scenario::BadObjective,Scenario::Duplicate,
       Scenario::Ambiguous,Scenario::BadInfeasibleOwner,Scenario::WrongGuarantee,Scenario::ReversedRank,
       Scenario::ReversedLimitedRank,Scenario::UnvalidatedFeasible,Scenario::NanInfeasible,Scenario::FirstMemory,Scenario::ReversedInfeasible}) {
    scenario=selected; calls=0; remaining=inf;
    auto result=solve_pool(model,options);
    historical(model.snapshot(),result);
    if (selected==Scenario::Good) {
      assert(result.exhausted() && result.entries.size()==3 && result.ranked_prefix==3 && calls==4);
      for (unsigned i=0;i<3;++i) {
        assert(result.entries[i].projection_values==std::vector<std::int64_t>{i});
        assert(result.entries[i].solution.objective==7+i);
      }
    } else {
      assert(!result.exhausted() && result.completion==PoolCompletion::Incomplete);
      if (selected==Scenario::FirstLimit || selected==Scenario::SecondLimit || selected==Scenario::FirstMemory) {
        assert(result.termination==(selected==Scenario::FirstMemory ? Termination::MemoryLimit:Termination::TimeLimit));
        assert(result.entries.size()==(selected==Scenario::SecondLimit ? 2u:1u));
        assert(result.ranked_prefix+1==result.entries.size() && !result.entries.back().rank_established);
      } else if (selected==Scenario::FirstCancel || selected==Scenario::SecondCancel) {
        assert(result.termination==Termination::Cancelled);
        assert(result.entries.size()==(selected==Scenario::FirstCancel ? 0u : 1u));
        assert(result.ranked_prefix==result.entries.size());
      } else if (selected==Scenario::Ambiguous) {
        assert(result.termination==Termination::InfeasibleOrUnbounded && result.entries.empty());
      } else {
        assert(result.termination==Termination::NumericalFailure);
        if (selected==Scenario::ReversedRank || selected==Scenario::ReversedLimitedRank || selected==Scenario::ReversedInfeasible) {
          assert(result.entries.size()==1 && result.ranked_prefix==0 && !result.entries[0].rank_established);
        }
      }
    }
  }
  for (auto selected : {Scenario::OffsetHonest,Scenario::OffsetForged}) {
    scenario=selected; calls=0; remaining=inf;
    Model cancellation; auto a=cancellation.add_integer(1,1); auto b=cancellation.add_integer(1e6,1e6);
    auto choice=cancellation.add_binary(); cancellation.minimize({{a,1},{b,1e10}},-1e16);
    PoolOptions selected_options; selected_options.projection=std::vector<Variable>{choice};
    auto result=solve_pool(cancellation,selected_options);
    if (selected==Scenario::OffsetHonest) {
      assert(result.exhausted() && result.ranked_prefix==2);
      for (const auto& entry:result.entries) assert(entry.solution.objective==1);
    } else {
      assert(result.termination==Termination::NumericalFailure && result.entries.empty() && !result.exhausted());
    }
  }
  scenario=Scenario::Good; calls=0; remaining=inf; options.max_solutions=2;
  auto result=solve_pool(model,options);
  assert(result.completion==PoolCompletion::RequestedLimit && result.termination==Termination::SolutionLimit);
  assert(result.ranked_prefix==2 && calls==2 && !result.exhausted());
  calls=0; options.solve.time_limit_seconds=0;
  assert(solve_pool(model,options).termination==Termination::TimeLimit && calls==0);
  options.solve.time_limit_seconds=60; options.solve.node_limit=1;
  assert(solve_pool(model,options).termination==Termination::Unsupported && calls==0);
}
#else
namespace {
void discrete_oracles(Backend backend, Guarantee guarantee) {
  for (auto sense : {ObjectiveSense::Minimize,ObjectiveSense::Maximize}) {
    Model model; auto x=model.add_integer(-2,2); auto y=model.add_integer(0,2);
    model.add_row({{x,1},{y,1}},0,inf); model.set_objective({{x,2},{y,-1}},sense,-9);
    const auto before=model.snapshot();
    std::map<std::vector<std::int64_t>,double> expected;
    for (int a=-2;a<=2;++a) for(int b=0;b<=2;++b) if(a+b>=0) expected[{a,b}]=2*a-b-9;
    PoolOptions options; options.max_solutions=20; options.solve.backend=backend; options.solve.guarantee=guarantee;
    auto result=solve_pool(model,options); historical(before,result);
    assert(result.exhausted() && result.termination==Termination::Optimal && result.guarantee==guarantee);
    assert(result.entries.size()==expected.size() && result.ranked_prefix==expected.size());
    double previous=sense==ObjectiveSense::Minimize ? -inf:inf;
    for (const auto& entry:result.entries) {
      assert(entry.rank_established && entry.solution.objective==expected.at(entry.projection_values));
      assert(sense==ObjectiveSense::Minimize ? *entry.solution.objective>=previous:*entry.solution.objective<=previous);
      previous=*entry.solution.objective;
    }
    // Excluding x removes all y completions, so there are exactly five classes.
    options.projection=std::vector<Variable>{x};
    result=solve_pool(model,options); historical(before,result);
    assert(result.exhausted() && result.entries.size()==5 && result.ranked_prefix==5);
    for (const auto& entry:result.entries) {
      auto a=entry.projection_values[0]; double best=sense==ObjectiveSense::Minimize ? inf:-inf;
      for(int b=0;b<=2;++b) if(a+b>=0) best=sense==ObjectiveSense::Minimize ? std::min(best,double(2*a-b-9)):std::max(best,double(2*a-b-9));
      assert(entry.solution.objective==best);
    }
    assert(model.revision()==before.revision && model.snapshot().variables.size()==before.variables.size());
    assert(model.snapshot().rows.size()==before.rows.size() && model.snapshot().objective.offset==-9);
    auto historical_value=result.entries[0].solution.value(x);
    model.set_bounds(x,-1,1);
    assert(result.entries[0].solution.value(x)==historical_value && result.revision==before.revision);
  }
}
void recourse() {
  for (auto sense : {ObjectiveSense::Minimize,ObjectiveSense::Maximize}) {
    Model model; auto x=model.add_integer(0,3); auto y=model.add_continuous(-inf,inf);
    model.add_row({{x,-1},{y,1}},-inf,2.25); // y <= x+2.25
    model.add_row({{x,1},{y,1}},3.5,inf); // y >= 3.5-x: x=0 infeasible
    model.set_objective({{x,2},{y,1}},sense,-5);
    PoolOptions options; options.max_solutions=10; options.projection=std::vector<Variable>{x};
    auto result=solve_pool(model,options); historical(model.snapshot(),result);
    assert(result.exhausted() && result.entries.size()==3);
    for(const auto& entry:result.entries) {
      const double a=entry.projection_values[0];
      const double completion=sense==ObjectiveSense::Minimize ? 3.5-a:a+2.25;
      assert(near(entry.solution.value(y),completion));
      assert(near(*entry.solution.objective,2*a+completion-5));
    }
  }
  {
    Model model; auto x=model.add_binary(); auto y=model.add_integer(-inf,inf);
    model.add_row({{x,1},{y,1}},1,inf); model.add_row({{x,-1},{y,1}},-inf,2);
    model.minimize({{x,3},{y,1}});
    PoolOptions options; options.projection=std::vector<Variable>{x};
    auto result=solve_pool(model,options); historical(model.snapshot(),result);
    assert(result.exhausted() && result.ranked_prefix==2);
    assert(result.entries[0].solution.objective==1 && result.entries[1].solution.objective==3);
  }
  Model unbounded; auto choice=unbounded.add_binary(); auto free=unbounded.add_continuous(-inf,inf);
  unbounded.maximize({{free,1}});
  PoolOptions options; options.projection=std::vector<Variable>{choice};
  auto result=solve_pool(unbounded,options);
  assert((result.termination==Termination::Unbounded || result.termination==Termination::InfeasibleOrUnbounded) &&
         result.entries.empty() && !result.exhausted());
}
void boundaries() {
  Model binary; auto x=binary.add_binary(); auto y=binary.add_binary();
  binary.add_row({{x,1},{y,1}},1,inf); // three tied representatives
  PoolOptions options; options.max_solutions=3; options.solve.primal_start={{x,1},{y,0}};
  auto result=solve_pool(binary,options); historical(binary.snapshot(),result);
  assert(result.completion==PoolCompletion::RequestedLimit && result.ranked_prefix==3 && !result.exhausted());
  assert(result.entries[0].solution.start_submitted && !result.entries[1].solution.start_submitted);
  options.max_solutions=4; result=solve_pool(binary,options);
  assert(result.exhausted() && result.entries.size()==3);
  options.projection=std::vector<Variable>{y,x}; result=solve_pool(binary,options);
  assert(result.exhausted() && result.projection[0]==y && result.projection[1]==x);
  options.projection=std::vector<Variable>{x,x};
  assert(solve_pool(binary,options).termination==Termination::InvalidModel);
  options.projection=std::vector<Variable>{Variable{binary.id()+1,x.id}};
  assert(solve_pool(binary,options).termination==Termination::InvalidModel);
  options.projection=std::vector<Variable>{};
  assert(solve_pool(binary,options).termination==Termination::Unsupported);
  options={}; options.max_solutions=0;
  assert(solve_pool(binary,options).termination==Termination::InvalidModel);
  options.max_solutions=2; options.solve.node_limit=0;
  assert(solve_pool(binary,options).termination==Termination::NodeLimit);
  options.solve.node_limit=10;
  assert(solve_pool(binary,options).termination==Termination::Unsupported);
  options.solve.node_limit.reset(); options.solve.time_limit_seconds=0;
  assert(solve_pool(binary,options).termination==Termination::TimeLimit);
  options.solve.time_limit_seconds=inf; options.solve.cancellation=std::make_shared<CancellationToken>(); options.solve.cancellation->cancel();
  assert(solve_pool(binary,options).termination==Termination::Cancelled);
  options={}; options.solve.guarantee=Guarantee::Certified;
  assert(solve_pool(binary,options).termination==Termination::Unsupported);
  options.solve.guarantee=Guarantee::Exact;
  assert(solve_pool(binary,options).termination==Termination::Unsupported);
  options={};
  Model empty; empty.minimize({},7);
  result=solve_pool(empty,options); historical(empty.snapshot(),result);
  assert(result.exhausted() && result.entries.size()==1 && result.entries[0].solution.objective==7);
  empty.add_row({},1,inf); result=solve_pool(empty,options);
  assert(result.exhausted() && result.entries.empty());
  Model fractional; fractional.add_integer(0.2,0.8);
  result=solve_pool(fractional,options); assert(result.exhausted() && result.entries.empty());
  Model tombstones; auto deleted=tombstones.add_binary(); tombstones.remove(deleted); tombstones.add_binary();
  result=solve_pool(tombstones,options); historical(tombstones.snapshot(),result);
  assert(result.exhausted() && result.entries.size()==2 && result.entries[0].solution.values.size()==2);
  options.projection=std::vector<Variable>{deleted};
  assert(solve_pool(tombstones,options).termination==Termination::InvalidModel);
  options={};
  Model continuous; auto c=continuous.add_continuous(0,1);
  assert(solve_pool(continuous,options).termination==Termination::Unsupported);
  options.projection=std::vector<Variable>{c};
  assert(solve_pool(continuous,options).termination==Termination::Unsupported);
  options={};
  Model wide; wide.add_integer(-inf,inf);
  assert(solve_pool(wide,options).termination==Termination::Unsupported);
  Model huge; huge.add_integer(-9007199254740992.0,9007199254740992.0);
  assert(solve_pool(huge,options).termination==Termination::Unsupported);
  Model semi; semi.add_variable(VariableType::SemiInteger,2,4);
  assert(solve_pool(semi,options).termination==Termination::Unsupported);
  options.max_solutions=20;
  Model logical; auto b=logical.add_binary(); auto v=logical.add_integer(0,2);
  auto indicator=add_indicator(logical,b,true,{{v,1}},1,inf);
  assert(solve_pool(logical,options).termination==Termination::Unsupported);
  remove_indicator(logical,indicator.indicator);
  assert(solve_pool(logical,options).completion==PoolCompletion::Exhausted);
  auto global=add_all_different(logical,{b,v});
  assert(solve_pool(logical,options).termination==Termination::Unsupported);
  logical.remove(global);
  assert(solve_pool(logical,options).completion==PoolCompletion::Exhausted);
}
}
int main() {
  if(capabilities(Backend::Native).available) discrete_oracles(Backend::Native,Guarantee::Exact);
  if(!capabilities(Backend::Highs).available) {
    Model model; model.add_binary();
    auto result=solve_pool(model);
    assert(result.termination==Termination::Unsupported && result.entries.empty() && !result.exhausted());
    return 0;
  }
  discrete_oracles(Backend::Highs,Guarantee::Numerical);
  recourse(); boundaries();
}
#endif
