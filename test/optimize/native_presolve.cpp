#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>
#include <gecode/optimize/native_preprocess.hpp>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();

// Independent finite-product oracle: no Optimize solve/compiler/validator is
// used to decide feasibility, objective values or the proof status below.
SolveResult oracle(const ModelSnapshot& model) {
  SolveResult result;
  result.model_id=model.model_id; result.revision=model.revision;
  result.guarantee=Guarantee::Exact; result.backend="Gecode native test oracle";
  result.message="Independent finite product exhausted";
  std::vector<double> point(model.variables.size(),std::numeric_limits<double>::quiet_NaN());
  const auto visit=[&](const auto& self,std::size_t slot)->void {
    if (slot<model.variables.size()) {
      const auto& variable=model.variables[slot];
      if (!variable.active) { self(self,slot+1); return; }
      for (int value=static_cast<int>(variable.lower); value<=variable.upper; ++value) {
        point[slot]=value; self(self,slot+1);
      }
      return;
    }
    for (const auto& row:model.rows) if (row.active) {
      std::int64_t sum=0;
      for (const auto& term:row.terms)
        sum+=static_cast<std::int64_t>(term.coefficient)*static_cast<std::int64_t>(point[term.variable.id]);
      if (sum<row.lower || sum>row.upper) return;
    }
    auto value=static_cast<std::int64_t>(model.objective.offset);
    for (const auto& term:model.objective.terms)
      value+=static_cast<std::int64_t>(term.coefficient)*static_cast<std::int64_t>(point[term.variable.id]);
    if (!result.objective || (model.objective.sense==ObjectiveSense::Minimize ? value<*result.objective : value>*result.objective)) {
      result.objective=static_cast<double>(value); result.values=point;
    }
  };
  visit(visit,0);
  if (result.objective) {
    for (const auto& variable:model.variables) result.active_variables.push_back(variable.active);
    result.solution_validated=true; result.termination=Termination::Optimal;
    result.best_bound=result.objective; result.update_gaps(model.objective.sense);
  } else result.termination=Termination::Infeasible;
  return result;
}

Model reduced_fixture(bool maximum=false) {
  Model model;
  const auto dead=model.add_integer(-1,1); model.remove(dead);
  const auto fixed=model.add_integer(2,2), x=model.add_integer(-2,2);
  model.add_row({{fixed,3},{x,-2}},4,8);
  model.add_row({{x,1}},-2,2); // A redundant original row.
  model.set_objective({{fixed,-3},{x,2}},maximum?ObjectiveSense::Maximize:ObjectiveSense::Minimize,11);
  return model;
}

Model auxiliary_fixture(bool maximize,bool bounded,int sign=1) {
  Model model;std::vector<Term> capacity,equality;
  for(int i=0;i<6;++i){const auto x=model.add_binary();capacity.push_back({x,double(i+1)});
    equality.push_back({x,double((maximize?-1:1)*sign*(i*3+1))});}
  model.add_row(capacity,-inf,10);
  const auto auxiliary=model.add_integer(maximize?3:(bounded?-22:-48),maximize?(bounded?28:54):3);
  equality.push_back({auxiliary,double(sign)});model.add_row(equality,3*sign,3*sign);
  model.set_objective({{auxiliary,1}},maximize?ObjectiveSense::Maximize:ObjectiveSense::Minimize,-11);
  return model;
}

void objective_auxiliaries() {
  SolveOptions options;options.backend=Backend::Native;options.guarantee=Guarantee::Exact;
  options.relative_gap=options.absolute_gap=0;options.time_limit_seconds=5;
  for(bool maximize:{false,true})for(bool bounded:{false,true})for(int sign:{-1,1}) {
    const auto model=auxiliary_fixture(maximize,bounded,sign);const auto source=model.snapshot();
    const auto expected=oracle(source);
    for(bool interrupted:{false,true}) {
      auto limited=options;limited.node_limit=2;SolveBudget budget(limited);bool called=false;
      const auto actual=Detail::native_objective_auxiliary(source,limited,budget,
        [&](const ModelSnapshot& reduced,const SolveOptions& exact,SolveBudget& shared) {
          called=true;assert(&shared==&budget && exact.guarantee==Guarantee::Exact);
          assert(!reduced.variables.back().active && reduced.objective.terms.size()==6);
          assert(reduced.rows[1].active==bounded); // Preserve nonredundant auxiliary domain bounds.
          auto result=oracle(reduced);shared.add_nodes(2);
          if(interrupted){result.termination=Termination::NodeLimit;
            result.best_bound=*result.objective+(maximize?3:-3);}
          return result;
        });
      assert(called && actual && actual->model_id==source.model_id && actual->revision==source.revision);
      assert(actual->has_solution() && actual->objective==expected.objective && budget.nodes()==2);
      assert(actual->active_variables.back() && actual->values.back()==*actual->objective+11);
      assert(validate(source,actual->values,0,0).valid);
      assert(actual->termination==(interrupted?Termination::NodeLimit:Termination::Optimal));
      assert(actual->best_bound==*expected.objective+(interrupted?(maximize?3:-3):0));
    }
    if(native_capabilities().available)for(bool enabled:{false,true}) {
      NativeAutoOptions configured;configured.solve=options;configured.settings={enabled,false,false,true};
      auto actual=solve_native_auto_configured(source,configured);
      if(actual.termination!=Termination::Optimal)std::cerr<<actual.message<<'\n';
      assert(actual.termination==Termination::Optimal && actual.objective==expected.objective);
      assert(actual.model_id==source.model_id && actual.revision==source.revision && validate(source,actual.values,0,0).valid);
      assert((actual.message.find("Exact objective auxiliary substitution")!=std::string::npos)==enabled);
      if(enabled && !bounded)assert(actual.message.find("eligible exact knapsack DP")!=std::string::npos);
      if(enabled && !bounded){configured.settings.knapsack=false;
        const auto no_dp=solve_native_auto_configured(source,configured);
        assert(no_dp.termination==Termination::Optimal && no_dp.objective==expected.objective);
        assert(no_dp.message.find("eligible exact knapsack DP")==std::string::npos);
        if(native_lp_capabilities().available)assert(no_dp.backend.find("checked LP")!=std::string::npos);
      }
    }
  }
  auto model=auxiliary_fixture(true,false);const auto original=model.snapshot();
  const auto never=[](const ModelSnapshot&,const SolveOptions&,SolveBudget&) -> SolveResult {assert(false);return {};};
  // A second use of the auxiliary or a non-unit equality coefficient must keep
  // the original model. No unsupported affine substitution is approximated.
  model.add_row({{original.variables.back().variable,1}},3,54);
  SolveBudget repeated(options);assert(!Detail::native_objective_auxiliary(model.snapshot(),options,repeated,never));
  auto nonunit=original;for(auto& term:nonunit.rows[1].terms)term.coefficient*=2;
  nonunit.rows[1].lower*=2;nonunit.rows[1].upper*=2;
  SolveBudget nonunit_budget(options);assert(!Detail::native_objective_auxiliary(nonunit,options,nonunit_budget,never));
  auto started=options;const auto witness=oracle(original);
  for(const auto& v:original.variables)started.primal_start.push_back({v.variable,witness.values[v.variable.id]});
  SolveBudget start_budget(started);assert(!Detail::native_objective_auxiliary(original,started,start_budget,never));
  auto cancelled=options;cancelled.cancellation=std::make_shared<CancellationToken>();
  SolveBudget cancel_budget(cancelled);
  const auto stopped=Detail::native_objective_auxiliary(original,cancelled,cancel_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&){
      auto result=oracle(reduced);cancelled.cancellation->cancel();return result;
    });
  assert(stopped && stopped->termination==Termination::Cancelled && !stopped->has_solution() && !stopped->best_bound);
  SolveBudget foreign_budget(options);
  const auto foreign=Detail::native_objective_auxiliary(original,options,foreign_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&){auto result=oracle(reduced);++result.model_id;return result;});
  assert(foreign && foreign->termination==Termination::BackendError && !foreign->has_solution());
  SolveBudget mask_budget(options);
  const auto mask=Detail::native_objective_auxiliary(original,options,mask_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&){
      auto result=oracle(reduced);result.active_variables[0]=false;return result;
    });
  assert(mask && mask->termination==Termination::BackendError && !mask->has_solution());
  SolveBudget unsupported_budget(options);
  const auto unsupported=Detail::native_objective_auxiliary(original,options,unsupported_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&){
      SolveResult result;result.model_id=reduced.model_id;result.revision=reduced.revision;
      result.guarantee=Guarantee::Exact;result.termination=Termination::Unsupported;return result;
    });
  assert(!unsupported); // The caller retains the originally admitted native route.
}

void fixed_offsets_and_bounds() {
  for (bool maximum:{false,true}) for (auto guarantee:{Guarantee::Exact,Guarantee::Numerical}) {
    const auto model=reduced_fixture(maximum); const auto source=model.snapshot();
    const auto expected=oracle(source);
    for (bool interrupted:{false,true}) {
      SolveOptions options; options.backend=Backend::Native; options.guarantee=guarantee;
      options.node_limit=2;
      SolveBudget budget(options); std::size_t calls=0;
      auto answer=Detail::native_presolve(source,options,budget,
        [&](const ModelSnapshot& reduced,const SolveOptions& exact,SolveBudget& shared) {
          ++calls; assert(&shared==&budget && exact.guarantee==Guarantee::Exact);
          assert(reduced.model_id!=source.model_id && reduced.variables.size()<source.variables.size());
          auto solved=oracle(reduced); shared.add_nodes(2);
          if (interrupted) {
            solved.termination=Termination::NodeLimit;
            solved.best_bound=*solved.objective+(maximum?3:-3);
          }
          return solved;
        });
      assert(answer && calls==1 && answer->guarantee==guarantee);
      assert(answer->model_id==source.model_id && answer->revision==source.revision);
      assert(answer->has_solution() && answer->objective==expected.objective);
      assert(answer->values.size()==source.variables.size());
      assert(!answer->active_variables[0] && std::isnan(answer->values[0]));
      assert(answer->values[1]==2 && answer->values[2]==(maximum?1:-1));
      assert(!answer->start_submitted && budget.nodes()==2);
      assert(answer->termination==(interrupted?Termination::NodeLimit:Termination::Optimal));
      assert(answer->best_bound==*expected.objective+(interrupted?(maximum?3:-3):0));
      assert(answer->absolute_gap==(interrupted?3:0));
    }
    if (native_capabilities().available) {
      SolveOptions options; options.backend=Backend::Native; options.guarantee=guarantee;
      const auto actual=solve_native_auto(source,options);
      assert(actual.termination==Termination::Optimal && actual.objective==expected.objective);
      assert(actual.model_id==source.model_id && actual.revision==source.revision);
      assert(actual.guarantee==guarantee && actual.has_solution() && actual.best_bound==actual.objective);
    }
  }
}

void infeasibility_and_fixpoint() {
  SolveOptions options; options.backend=Backend::Native;
  for (bool interval:{false,true}) {
    Model model; const auto fixed=model.add_integer(2,2);
    const auto x=model.add_binary(),y=model.add_binary(),z=model.add_binary();
    if (interval) model.add_row({{x,1}},2,inf);
    else model.add_row({{x,2},{y,2},{z,2}},3,3);
    model.minimize({{fixed,3},{x,-2}},-11);
    const auto source=model.snapshot(); assert(!oracle(source).has_solution());
    SolveBudget budget(options); std::size_t calls=0;
    const auto result=Detail::native_presolve(source,options,budget,
      [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&) { ++calls; return oracle(reduced); });
    assert(result && result->termination==Termination::Infeasible && !result->has_solution());
    assert(!result->best_bound && !result->objective && result->model_id==source.model_id);
    assert(calls==(interval?0U:1U));
  }
  Model unchanged; const auto x=unchanged.add_binary(),y=unchanged.add_binary();
  unchanged.add_row({{x,1},{y,1}},1,inf);
  SolveBudget budget(options); bool called=false;
  const auto result=Detail::native_presolve(unchanged.snapshot(),options,budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&) { called=true; return oracle(reduced); });
  assert(!result && !called); // A presolve fixpoint is not a solved optimization.
}

void incomplete_artifact() {
  Model model; std::vector<Variable> x;
  for (int i=0;i<8;++i) x.push_back(model.add_binary());
  for (int i=7;i>0;--i) model.add_row({{x[i],1},{x[i-1],-1}},0,0);
  model.add_row({{x[0],1}},1,1); model.minimize({{x.back(),-7}},13);
  SolveOptions options; SolveBudget budget(options);
  const auto result=Detail::native_presolve(model.snapshot(),options,budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&) { return oracle(reduced); });
  assert(result && result->termination==Termination::Optimal && result->objective==6);
  assert(result->message.find("partial reduction")!=std::string::npos);
  for (double value:result->values) assert(value==1);
}

void starts_and_stops() {
  const auto model=reduced_fixture(); const auto source=model.snapshot();
  SolveOptions options; options.primal_start={{source.variables[1].variable,2},{source.variables[2].variable,-1}};
  SolveBudget start_budget(options); bool called=false;
  const auto skipped=Detail::native_presolve(source,options,start_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&) { called=true; return oracle(reduced); });
  assert(!skipped && !called);
  options={}; options.time_limit_seconds=0; SolveBudget zero_budget(options);
  auto stopped=Detail::native_presolve(source,options,zero_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&) { called=true; return oracle(reduced); });
  assert(stopped && stopped->termination==Termination::TimeLimit && !called && !stopped->has_solution());
  options={}; options.cancellation=std::make_shared<CancellationToken>(); SolveBudget cancelled(options);
  stopped=Detail::native_presolve(source,options,cancelled,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget& shared) {
      called=true; auto solved=oracle(reduced); shared.cancellation()->cancel(); return solved;
    });
  assert(called && stopped && stopped->termination==Termination::Cancelled);
  assert(!stopped->has_solution() && !stopped->best_bound && !stopped->objective);
  assert(stopped->model_id==source.model_id && stopped->revision==source.revision);
  options={}; SolveBudget foreign_budget(options);
  const auto foreign=Detail::native_presolve(source,options,foreign_budget,
    [&](const ModelSnapshot& reduced,const SolveOptions&,SolveBudget&) {
      auto solved=oracle(reduced); ++solved.revision; return solved;
    });
  assert(foreign && foreign->termination==Termination::BackendError && !foreign->has_solution());
}
}

int main() {
  objective_auxiliaries(); fixed_offsets_and_bounds(); infeasibility_and_fixpoint(); incomplete_artifact(); starts_and_stops();
  std::cout<<"Exact native presolve composition: offsets, proof transfer, partial reductions, starts and stops pass\n";
}
