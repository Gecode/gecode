#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize.hh>

#include <cassert>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <optional>

using namespace Gecode::Optimize;
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();

// Deliberately separate integer oracle: no solver/compiler/validator calls.
bool feasible(const ModelSnapshot& model, const std::vector<double>& values) {
  const auto row = [&](const std::vector<Term>& terms, double lo, double hi) {
    std::int64_t sum = 0;
    for (const auto& term : terms)
      sum += static_cast<std::int64_t>(term.coefficient) * static_cast<std::int64_t>(values[term.variable.id]);
    return sum >= lo && sum <= hi;
  };
  for (const auto& original : model.rows)
    if (original.active && !original.indicator_origin && !row(original.terms, original.lower, original.upper)) return false;
  for (const auto& original : model.indicators) if (original.active) {
    const bool enabled = values[original.activator.id] == (original.active_value ? 1 : 0);
    if (enabled && !row(original.terms, original.lower, original.upper)) return false;
    if (original.inactive_gate && values[original.inactive_gate->id] != (enabled ? 0 : 1)) return false;
  }
  // The enumerated global fixture uses all-different only; other native global
  // implementations already have their own full-product conformance suite.
  for (const auto& original : model.globals) if (original.active) {
    const auto& vars = std::get<AllDifferentData>(original.payload).variables;
    for (std::size_t i = 0; i < vars.size(); ++i)
      for (std::size_t j = i + 1; j < vars.size(); ++j)
        if (values[vars[i].id] == values[vars[j].id]) return false;
  }
  return true;
}

std::optional<std::int64_t> oracle(const ModelSnapshot& model) {
  std::vector<double> values(model.variables.size());
  std::optional<std::int64_t> best;
  const auto visit = [&](const auto& self, std::size_t slot) -> void {
    if (slot == model.variables.size()) {
      if (!feasible(model, values)) return;
      auto objective = static_cast<std::int64_t>(model.objective.offset);
      for (const auto& term : model.objective.terms)
        objective += static_cast<std::int64_t>(term.coefficient) * static_cast<std::int64_t>(values[term.variable.id]);
      if (!best || (model.objective.sense == ObjectiveSense::Minimize ? objective < *best : objective > *best)) best = objective;
      return;
    }
    const auto& variable = model.variables[slot];
    if (!variable.active) { self(self, slot + 1); return; }
    if (variable.type == VariableType::SemiInteger) { values[slot] = 0; self(self, slot + 1); }
    for (int value = static_cast<int>(variable.lower); value <= variable.upper; ++value) {
      values[slot] = value; self(self, slot + 1);
    }
  };
  visit(visit, 0); return best;
}

std::uint64_t calls = 0, bounds = 0, tightened = 0;
std::uint64_t configurations = 0;
void check(const Model& model) {
  const auto snapshot = model.snapshot();
  const auto expected = oracle(snapshot);
  for (auto frequency : {NativeLpFrequency::Root, NativeLpFrequency::AfterBoundChanges})
    for (bool filtering : {false, true})
      for (unsigned interval : {1U, 3U}) for (bool covers : {false,true}) {
        ++configurations;
        NativeLpOptions options; options.solve.guarantee = Guarantee::Exact;
        options.frequency = frequency; options.bound_tightening = filtering;
        options.bound_change_interval = interval;
        if(covers) options.root_cover_cuts=NativeRootCoverSettings{};
        const auto solved = solve_native_lp(snapshot, options);
        const auto& result = solved.result;
        if (result.termination != (expected ? Termination::Optimal : Termination::Infeasible)) {
          std::cerr << to_string(result.termination) << ": " << result.message << '\n'; assert(false);
        }
        assert(result.model_id == model.id() && result.revision == model.revision());
        assert(result.guarantee == Guarantee::Exact && result.elapsed_seconds >= 0);
        assert(result.backend == "Gecode native + checked LP");
        assert(result.backend_version.find("HiGHS") != std::string::npos);
        assert(result.has_solution() == expected.has_value());
        if (expected) {
          assert(result.objective == static_cast<double>(*expected));
          assert(result.best_bound == result.objective && result.absolute_gap == 0);
          assert(feasible(snapshot, result.values));
          for (const auto& variable : snapshot.variables) {
            if (!variable.active) { assert(!result.active_variables[variable.variable.id]); continue; }
            const auto value = result.value(variable.variable);
            assert(value == std::trunc(value));
            assert((variable.type == VariableType::SemiInteger && value == 0) ||
                   (value >= variable.lower && value <= variable.upper));
          }
        } else assert(!result.objective && !result.best_bound && !result.solution_validated);
        calls += solved.relaxation.lp_calls; bounds += solved.relaxation.valid_bounds;
        tightened += solved.relaxation.variable_bound_tightenings;
        const auto& root=solved.relaxation.root_cover;
        assert(root.requested==covers && root.rounds<=4 && root.cuts<=64);
        assert(solved.relaxation.lp_calls>=root.lp_calls && solved.relaxation.valid_bounds>=root.valid_bounds);
        assert(solved.relaxation.rejected_bounds>=root.rejected_bounds && solved.relaxation.lp_seconds>=root.lp_seconds);
        assert(root.completion!=(covers?NativeRootCoverCompletion::NotStarted:NativeRootCoverCompletion::NoNewCuts));
        if(!covers) assert(root.completion==NativeRootCoverCompletion::NotRequested && !root.lp_calls);
      }
}

void enumerated_models() {
  for (int i = 0; i < 32; ++i) {
    Model model;
    const auto gone = model.add_integer(-1, 1); model.remove(gone);
    const auto x = model.add_integer(-3, 3), y = model.add_integer(-2, 2), b = model.add_binary();
    model.add_row({{x, i % 5 - 2.0}, {y, i % 3 - 1.0}, {b, 2}}, -2, 4);
    model.add_row({{x, -1}, {y, 1}}, -inf, i % 4 - 1.0);
    const auto dead = model.add_row({{x, 1}}, -inf, 0); model.remove(dead);
    model.set_objective({{x, i % 7 - 3.0}, {y, -2}, {b, 3}},
      i % 2 ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, i - 20);
    check(model);
  }
  for (bool activation : {false, true}) for (bool maximum : {false, true}) {
    Model model;
    const auto b = model.add_binary(), x = model.add_integer(-2, 3);
    const auto semi = model.add_variable(VariableType::SemiInteger, 2, 4);
    const auto indicator = add_indicator(model, b, activation, {{x, 2}, {semi, -1}}, -2, 1);
    assert(indicator.inactive_gate);
    model.add_row({{*indicator.inactive_gate, 1}, {semi, 1}}, -inf, 4);
    add_all_different(model, {x, semi});
    model.set_objective({{x, -2}, {semi, 3}, {*indicator.inactive_gate, 1}},
      maximum ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, -7);
    check(model);
  }
  Model empty; empty.minimize({}, -9); check(empty);
  empty.add_row({}, 1, inf); check(empty);
  Model no_cost; no_cost.add_integer(-2, 2); check(no_cost);
  Model coupled;
  const auto x = coupled.add_integer(0, 10), y = coupled.add_integer(0, 10);
  coupled.add_row({{x, 2}, {y, 3}}, 7, inf); coupled.minimize({{x, 1}, {y, 1}}, 8);
  check(coupled);
  for(bool activation:{false,true}) for(bool maximum:{false,true}) {
    Model hybrid;const auto x=hybrid.add_binary(),y=hybrid.add_binary(),b=hybrid.add_binary();
    const auto s=hybrid.add_variable(VariableType::SemiInteger,2,3);
    hybrid.add_row({{x,3},{y,3}},-inf,5);
    add_all_different(hybrid,{x,y});
    add_indicator(hybrid,b,activation,{{s,1},{x,1}},2,inf);
    hybrid.set_objective({{x,maximum?2.0:-2.0},{y,maximum?2.0:-2.0},{s,maximum?-1.0:1.0}},
      maximum?ObjectiveSense::Maximize:ObjectiveSense::Minimize,maximum?-17:17);
    check(hybrid);
  }
  assert(calls > 0 && bounds > 0 && tightened > 0);
}

void limits_and_rejections() {
  Model model; const auto x = model.add_integer(0, 100), y = model.add_integer(0, 100);
  model.add_row({{x, 2}, {y, 3}}, 7, inf); model.minimize({{x, 1}, {y, 1}});
  NativeLpOptions options;
  options.solve.time_limit_seconds = 0;
  auto result = solve_native_lp(model, options);
  assert(result.result.termination == Termination::TimeLimit && result.relaxation.lp_calls == 0);
  options = {}; options.solve.cancellation = std::make_shared<CancellationToken>(); options.solve.cancellation->cancel();
  result = solve_native_lp(model.snapshot(), options);
  assert(result.result.termination == Termination::Cancelled && result.relaxation.lp_calls == 0);
  options = {}; options.solve.node_limit = 0;
  result = solve_native_lp(model, options);
  assert(result.result.termination == Termination::NodeLimit && !result.result.best_bound);
  options = {}; options.solve.node_limit = 1;
  result = solve_native_lp(model, options);
  assert(result.result.termination == Termination::NodeLimit && !result.result.best_bound);
  options = {}; options.solve.guarantee = Guarantee::Certified;
  assert(solve_native_lp(model, options).result.termination == Termination::Unsupported);
  options = {}; options.solve.backend = Backend::Highs;
  assert(solve_native_lp(model, options).result.termination == Termination::Unsupported);
  options = {}; options.solve.threads = 2;
  assert(solve_native_lp(model, options).result.termination == Termination::Unsupported);
  options = {}; options.solve.primal_start = {{x, 3}};
  assert(solve_native_lp(model, options).result.termination == Termination::Unsupported);
  options = {}; options.bound_change_interval = 0;
  assert(solve_native_lp(model, options).result.termination == Termination::InvalidModel);
  options = {}; options.frequency = static_cast<NativeLpFrequency>(-1);
  assert(solve_native_lp(model, options).result.termination == Termination::InvalidModel);
  auto malformed = model.snapshot(); malformed.rows[0].terms[0].variable.model_id++;
  assert(solve_native_lp(malformed).result.termination == Termination::InvalidModel);
  Model continuous; continuous.add_continuous(0, 1);
  assert(solve_native_lp(continuous).result.termination == Termination::Unsupported);
  Model fractional; auto variable = fractional.add_integer(0, 1); fractional.minimize({{variable, .5}});
  assert(solve_native_lp(fractional).result.termination == Termination::Unsupported);
  Model scale; variable = scale.add_integer(0, 0); scale.add_row({{variable, 1000000001}}, -inf, 0);
  assert(solve_native(scale).termination == Termination::Optimal);
  assert(solve_native_lp(scale).result.termination == Termination::Unsupported);
  auto old = solve_native_lp(model); const auto revision = old.result.revision;
  model.set_bounds(x, 5, 10);
  auto changed = solve_native_lp(model);
  assert(old.result.revision == revision && changed.result.revision != revision);
  assert(changed.result.objective == 5 && old.result.objective == 3);
  assert(solve_native(model).backend == "Gecode native"); // unchanged native route
}

Model cover_model() {
  Model model;const auto x=model.add_binary(),y=model.add_binary();
  model.add_row({{x,3},{y,3}},-inf,5);model.minimize({{x,-2},{y,-2}},17);return model;
}
void root_cover_limits() {
  auto model=cover_model();const auto snapshot=model.snapshot();
  NativeLpOptions options;options.solve.guarantee=Guarantee::Exact;
  options.root_cover_cuts=NativeRootCoverSettings{};
  const auto full=solve_native_lp(model,options);
  assert(full.result.termination==Termination::Optimal && full.result.objective==15);
  const auto& root=full.relaxation.root_cover;
  assert(root.requested && root.cuts==1 && root.nonzeros==2 && root.augmentations==1);
  assert(root.lp_calls==2 && root.rounds==2 && root.completion==NativeRootCoverCompletion::NoNewCuts);
  assert(full.relaxation.lp_calls>=root.lp_calls && full.relaxation.lp_calls<=root.lp_calls+1);
  assert(full.relaxation.valid_bounds>=root.valid_bounds);
  for(unsigned kind=0;kind<11;++kind) {
    options.root_cover_cuts=NativeRootCoverSettings{};auto& limits=*options.root_cover_cuts;
    auto expected=NativeRootCoverCompletion::StorageLimit;
    if(kind==0){limits.max_rounds=0;expected=NativeRootCoverCompletion::RoundLimit;}
    if(kind==1){limits.max_rounds=1;expected=NativeRootCoverCompletion::RoundLimit;}
    if(kind==2){limits.max_work=0;expected=NativeRootCoverCompletion::WorkLimit;}
    if(kind==3)limits.max_cuts=0;
    if(kind==4)limits.max_cut_nonzeros=1;
    if(kind==5)limits.max_model_columns=1;
    if(kind==6)limits.max_model_rows=1;
    if(kind==7)limits.max_model_nonzeros=3;
    if(kind==8){limits.max_separation_rows=0;expected=NativeRootCoverCompletion::SeparationLimit;}
    if(kind==9){limits.max_terms_per_row=1;expected=NativeRootCoverCompletion::NoNewCuts;}
    if(kind==10){limits.max_work=1;expected=NativeRootCoverCompletion::WorkLimit;}
    const auto partial=solve_native_lp(model,options);++configurations;
    assert(partial.result.termination==Termination::Optimal && partial.result.objective==15);
    assert(partial.relaxation.root_cover.completion==expected);
    assert(partial.relaxation.root_cover.work<=limits.max_work);
    assert(partial.relaxation.lp_calls>=partial.relaxation.root_cover.lp_calls);
    if(kind==1)assert(partial.relaxation.root_cover.cuts==1 && partial.relaxation.root_cover.lp_calls==1);
    else assert(partial.relaxation.root_cover.cuts==0);
    assert(partial.result.model_id==snapshot.model_id && partial.result.revision==snapshot.revision);
    assert(feasible(snapshot,partial.result.values));
  }
  for(std::size_t work=10;work<400;work+=13) {
    options.root_cover_cuts=NativeRootCoverSettings{};options.root_cover_cuts->max_work=work;
    auto limited=solve_native_lp(model,options);++configurations;
    assert(limited.result.termination==Termination::Optimal && limited.result.objective==15);
    assert(limited.relaxation.root_cover.work<=work);
  }
  for(unsigned reason=0;reason<3;++reason) {
    options={};options.solve.guarantee=Guarantee::Exact;options.root_cover_cuts=NativeRootCoverSettings{};
    if(reason==0)options.solve.time_limit_seconds=0;
    if(reason==1)options.solve.node_limit=0;
    if(reason==2){options.solve.cancellation=std::make_shared<CancellationToken>();options.solve.cancellation->cancel();}
    auto stopped=solve_native_lp(model,options);
    assert(stopped.result.guarantee==Guarantee::Exact && !stopped.result.has_solution() && !stopped.result.best_bound);
    assert(stopped.result.termination==(reason==0?Termination::TimeLimit:reason==1?Termination::NodeLimit:Termination::Cancelled));
    assert(stopped.relaxation.root_cover.requested && stopped.relaxation.root_cover.completion==NativeRootCoverCompletion::NotStarted);
    assert(!stopped.relaxation.lp_calls);
  }
  Model empty;empty.minimize({},-9);options={};options.root_cover_cuts=NativeRootCoverSettings{};
  auto no_hint=solve_native_lp(empty,options);
  assert(no_hint.result.termination==Termination::Optimal && no_hint.result.objective==-9);
  assert(no_hint.relaxation.root_cover.completion==NativeRootCoverCompletion::NoPrimalSuggestion);
  empty.add_row({},1,inf);no_hint=solve_native_lp(empty,options);
  assert(no_hint.result.termination==Termination::Infeasible && !no_hint.result.has_solution());
  assert(no_hint.relaxation.root_cover.completion==NativeRootCoverCompletion::NoPrimalSuggestion);
  assert(model.revision()==snapshot.revision);
}

#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
std::string root_event;
std::optional<NativeRootCoverCompletion> injected_completion;
std::shared_ptr<CancellationToken> root_cancellation;
int root_exception=0;
void root_failures() {
  auto model=cover_model();NativeLpOptions options;options.solve.guarantee=Guarantee::Exact;
  options.root_cover_cuts=NativeRootCoverSettings{};
  for(auto completion:{NativeRootCoverCompletion::NoPrimalSuggestion,NativeRootCoverCompletion::InvalidSuggestion,
      NativeRootCoverCompletion::CallbackError,NativeRootCoverCompletion::BackendError,NativeRootCoverCompletion::AllocationFailure}) {
    root_event="after_loop";injected_completion=completion;
    auto result=solve_native_lp(model,options);++configurations;
    const bool hint=completion==NativeRootCoverCompletion::NoPrimalSuggestion || completion==NativeRootCoverCompletion::InvalidSuggestion;
    assert(result.result.termination==(hint?Termination::Optimal:
      completion==NativeRootCoverCompletion::AllocationFailure?Termination::MemoryLimit:Termination::BackendError));
    assert(result.result.has_solution()==hint && result.result.guarantee==Guarantee::Exact);
    if(hint)assert(result.result.objective==15);
    else assert(!result.result.best_bound && !result.result.objective);
    assert(result.relaxation.root_cover.completion==completion && result.relaxation.root_cover.cuts==1);
    assert(result.relaxation.lp_calls>=result.relaxation.root_cover.lp_calls);
  }
  injected_completion.reset();
  for(const auto* event:{"before_loop","after_loop","prepared"})for(int mode=0;mode<3;++mode) {
    root_event=event;root_exception=mode;root_cancellation=std::make_shared<CancellationToken>();
    options.solve.cancellation=root_cancellation;
    const auto result=solve_native_lp(model,options);++configurations;
    assert(result.result.termination==(mode==0?Termination::Cancelled:mode==1?Termination::MemoryLimit:Termination::BackendError));
    assert(!result.result.has_solution() && !result.result.best_bound && result.result.guarantee==Guarantee::Exact);
    assert(result.relaxation.root_cover.requested);
  }
  root_event.clear();root_cancellation.reset();root_exception=0;
}
#endif
}

#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
namespace Gecode { namespace Optimize {
void native_root_cut_test_event(const char* event,NativeRootCoverCompletion& completion) {
  if(root_event!=event)return;
  if(injected_completion){completion=*injected_completion;return;}
  if(root_exception==1)throw std::bad_alloc();
  if(root_exception==2)throw std::runtime_error("injected root preparation failure");
  root_cancellation->cancel();
}
}}
#endif

int main() {
  if (!native_lp_capabilities().available) {
    Model model; model.add_integer(0, 1);
    assert(solve_native_lp(model).result.termination == Termination::Unsupported);
    assert(solve_native_lp(model.snapshot()).result.termination == Termination::Unsupported);
    NativeLpOptions invalid; invalid.bound_change_interval = 0;
    assert(solve_native_lp(model, invalid).result.termination == Termination::InvalidModel);
    invalid={};invalid.root_cover_cuts=NativeRootCoverSettings{};invalid.root_cover_cuts->denominator=3;
    assert(solve_native_lp(model,invalid).result.termination==Termination::InvalidModel);
    invalid.root_cover_cuts->denominator=1048576;
    const auto missing=solve_native_lp(model,invalid);
    assert(missing.result.termination==Termination::Unsupported && missing.relaxation.root_cover.requested);
    std::cout << "Native checked LP unavailable: explicit boundary verified\n"; return 0;
  }
  enumerated_models(); limits_and_rejections();root_cover_limits();
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
  root_failures();
#endif
  // Independent solves own LP workspaces; historical results outlive them.
  const auto solve_one = [] {
    Model model; auto x = model.add_integer(-2, 5), y = model.add_integer(0, 5);
    model.add_row({{x, 2}, {y, 3}}, 7, inf); model.minimize({{x, 1}, {y, 1}});
    NativeLpOptions options;options.root_cover_cuts=NativeRootCoverSettings{};
    return solve_native_lp(model,options);
  };
  auto a = std::async(std::launch::async, solve_one), b = std::async(std::launch::async, solve_one);
  const auto first = a.get(), second = b.get();
  assert(first.result.termination == Termination::Optimal && first.result.objective == 2);
  assert(second.result.objective == first.result.objective && second.result.model_id != first.result.model_id);
  std::cout << configurations << " native LP oracle/root-cover configurations pass; " << calls << " LP calls, "
            << bounds << " checked bounds, " << tightened << " interval tightenings\n";
}
