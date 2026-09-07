#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>

using namespace Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
std::uint64_t runs=0;

// Independent finite product oracle: no native compiler, propagator or common
// validator is used to compute feasible points or objective bounds.
bool feasible(const ModelSnapshot& model,const std::vector<double>& point) {
  const auto row=[&](const std::vector<Term>& terms,double lower,double upper) {
    std::int64_t sum=0;
    for(const auto& term:terms) sum+=static_cast<std::int64_t>(term.coefficient)*static_cast<std::int64_t>(point[term.variable.id]);
    return sum>=lower && sum<=upper;
  };
  for(const auto& variable:model.variables) if(variable.active) {
    const auto value=point.at(variable.variable.id);
    if(value!=std::trunc(value) || !std::isfinite(value)) return false;
    if(!(variable.type==VariableType::SemiInteger && value==0) && (value<variable.lower || value>variable.upper)) return false;
  }
  for(const auto& original:model.rows)
    if(original.active && !original.indicator_origin && !row(original.terms,original.lower,original.upper)) return false;
  for(const auto& indicator:model.indicators) if(indicator.active) {
    const bool enabled=point[indicator.activator.id]==(indicator.active_value?1:0);
    if(enabled && !row(indicator.terms,indicator.lower,indicator.upper)) return false;
    if(indicator.inactive_gate && point[indicator.inactive_gate->id]!=(enabled?0:1)) return false;
  }
  for(const auto& global:model.globals) if(global.active) {
    const bool valid=std::visit([&](const auto& data) {
      using T=std::decay_t<decltype(data)>;
      const auto value=[&](Variable v) {return static_cast<std::int64_t>(point[v.id]);};
      if constexpr(std::is_same_v<T,AllDifferentData>) {
        std::set<std::int64_t> seen;
        for(auto v:data.variables) if(!seen.insert(value(v)).second) return false;
        return true;
      } else if constexpr(std::is_same_v<T,ElementData>) {
        const auto index=value(data.index)-data.index_base;
        return index>=0 && static_cast<std::size_t>(index)<data.elements.size() && value(data.result)==value(data.elements[index]);
      } else if constexpr(std::is_same_v<T,TableData>) {
        std::vector<std::int64_t> tuple;for(auto v:data.variables) tuple.push_back(value(v));
        return std::find(data.tuples.begin(),data.tuples.end(),tuple)!=data.tuples.end();
      } else if constexpr(std::is_same_v<T,CumulativeData>) {
        std::set<std::int64_t> starts;
        for(std::size_t i=0;i<data.starts.size();++i) if(data.durations[i]) starts.insert(value(data.starts[i]));
        for(auto time:starts) {
          std::int64_t demand=0;
          for(std::size_t i=0;i<data.starts.size();++i)
            if(value(data.starts[i])<=time && time<value(data.starts[i])+data.durations[i]) demand+=data.heights[i];
          if(demand>data.capacity) return false;
        }
        return true;
      } else if constexpr(std::is_same_v<T,RegularData>) {
        auto state=data.initial_state;
        for(auto variable:data.variables){
          const auto edge=std::find_if(data.transitions.begin(),data.transitions.end(),[&](const RegularTransition& t){return t.from==state&&t.symbol==value(variable);});
          if(edge==data.transitions.end())return false;
          state=edge->to;
        }
        return std::find(data.final_states.begin(),data.final_states.end(),state)!=data.final_states.end();
      } else {
        if(data.successors.empty()) return false;
        std::set<std::int64_t> seen;std::int64_t next=0;
        for(std::size_t i=0;i<data.successors.size();++i) {
          if(next<0 || static_cast<std::size_t>(next)>=data.successors.size() || !seen.insert(next).second) return false;
          next=value(data.successors[next])-data.index_base;
        }
        return next==0;
      }
    },global.payload);
    if(!valid) return false;
  }
  return true;
}
std::int64_t objective(const ModelSnapshot& model,const std::vector<double>& point) {
  auto value=static_cast<std::int64_t>(model.objective.offset);
  for(const auto& term:model.objective.terms) value+=static_cast<std::int64_t>(term.coefficient)*static_cast<std::int64_t>(point[term.variable.id]);
  return value;
}
std::optional<std::int64_t> oracle(const ModelSnapshot& model) {
  std::vector<double> point(model.variables.size());std::optional<std::int64_t> best;
  const auto visit=[&](const auto& self,std::size_t slot)->void {
    if(slot==model.variables.size()) {
      if(!feasible(model,point)) return;
      const auto value=objective(model,point);
      if(!best || (model.objective.sense==ObjectiveSense::Minimize ? value<*best:value>*best)) best=value;
      return;
    }
    const auto& variable=model.variables[slot];
    if(!variable.active) {self(self,slot+1);return;}
    if(variable.type==VariableType::SemiInteger) {point[slot]=0;self(self,slot+1);}
    for(int value=static_cast<int>(variable.lower);value<=variable.upper;++value) {point[slot]=value;self(self,slot+1);}
  };
  visit(visit,0);return best;
}
SolveOptions exact() {
  SolveOptions options; options.backend = Backend::Native; options.guarantee = Guarantee::Exact;
  return options;
}
void check(const ModelSnapshot& source, const SolveResult& result,
           const std::optional<std::int64_t>& best, bool completed = false) {
  ++runs;
  const bool minimize = source.objective.sense == ObjectiveSense::Minimize;
  assert(result.model_id == source.model_id && result.revision == source.revision);
  assert(result.guarantee == Guarantee::Exact);
  if (result.has_solution()) {
    assert(best && feasible(source, result.values));
    assert(result.objective == static_cast<double>(objective(source, result.values)));
    assert(minimize ? *result.objective >= *best : *result.objective <= *best);
    for (const auto& v : source.variables) {
      assert(result.active_variables.at(v.variable.id) == v.active);
      if (!v.active) assert(std::isnan(result.values.at(v.variable.id)));
    }
  }
  if (result.best_bound && best)
    assert(minimize ? *result.best_bound <= *best : *result.best_bound >= *best);
  if (result.termination == Termination::Optimal) {
    assert(best && result.objective == *best && result.best_bound == result.objective);
  } else if (result.termination == Termination::Infeasible) {
    assert(!best && !result.has_solution() && !result.best_bound);
  } else {
    assert(!completed && result.termination == Termination::NodeLimit);
  }
}
void exercise(const Model& model) {
  const auto source = model.snapshot(); const auto best = oracle(source);
  auto options = exact();
  check(source, solve_native(source, options), best, true);
  for (std::uint64_t limit : {0, 1, 2, 3, 5, 8}) {
    options.node_limit = limit; const auto result = solve_native(source, options);
    check(source, result, best);
    if (result.termination == Termination::NodeLimit) assert(!result.best_bound);
  }
  for (auto order : {NativeSearchOrder::DepthFirst, NativeSearchOrder::BestBound}) {
    NativeSearchOptions search; search.solve = exact(); search.order = order;
    check(source, solve_native_search(source, search).result, best, true);
    for (std::uint64_t limit = 0; limit <= 6; ++limit) {
      search.solve.node_limit = limit; const auto result = solve_native_search(source, search);
      check(source, result.result, best);
      assert(result.frontier.admitted_nodes <= limit);
    }
  }
}
Model capacity_model(bool negative = false, bool maximum = false,
                     int capacity = 3, bool redundant_side = false) {
  Model m; const auto dead = m.add_binary(); m.remove(dead);
  const auto a = m.add_binary(), b = m.add_binary();
  const double sign = negative ? -1 : 1;
  m.add_row({{a, 2 * sign}, {b, 2 * sign}},
    negative ? -capacity : (redundant_side ? -1 : -inf),
    negative ? (redundant_side ? 1 : inf) : capacity);
  m.set_objective({{a, maximum ? 3.0 : -3.0}, {b, maximum ? 2.0 : -2.0}},
    maximum ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, maximum ? -11 : 11);
  return m;
}
void pure_models() {
  for (bool negative : {false, true}) for (bool maximum : {false, true})
    for (bool redundant : {false, true}) for (int capacity : {0, 1, 3, 4, 7}) {
      auto m = capacity_model(negative, maximum, capacity, redundant); exercise(m);
    }
  for (int pattern = 0; pattern < 12; ++pattern) {
    Model m; std::vector<Term> row, cost;
    for (int i = 0; i < 5; ++i) {
      const auto x = m.add_binary(); row.push_back({x, double(i + 1)});
      cost.push_back({x, double((pattern + 2 * i) % 7 - 3)});
    }
    m.add_row(row, -inf, pattern % 9);
    m.set_objective(cost, pattern % 2 ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, pattern - 7);
    exercise(m);
  }
  Model ties; auto a = ties.add_binary(), b = ties.add_binary();
  ties.add_row({{a, 2}, {b, 2}}, -inf, 2); ties.minimize({{a, -1}, {b, -1}}); exercise(ties);
  ties.minimize({}); exercise(ties); // No-benefit objective must keep the old path.
}
void exact_bound_and_work() {
  for (bool negative : {false, true}) for (bool maximum : {false, true}) {
    auto m = capacity_model(negative, maximum); const auto source = m.snapshot();
    const auto best = oracle(source);
    for (auto order : {NativeSearchOrder::DepthFirst, NativeSearchOrder::BestBound}) {
      NativeSearchOptions search; search.solve = exact(); search.order = order;
      search.solve.node_limit = 1;
      const auto root = solve_native_search(source, search);
      check(source, root.result, best);
      assert(!root.result.has_solution() && root.result.best_bound == *best);
      search.solve.node_limit = 3;
      const auto full = solve_native_search(source, search);
      check(source, full.result, best, true);
      assert(full.frontier.admitted_nodes == 3 && full.frontier.feasible_leaves == 1);
    }
    auto options = exact(); options.node_limit = 8;
    check(source, solve_native(source, options), best, true);
    // Both an already optimal and a poor feasible start remain ordinary starts.
    for (int selected : {0, 1}) {
      options = exact(); options.primal_start = {
        {source.variables[1].variable, double(selected)}, {source.variables[2].variable, 0}};
      const auto answer = solve_native(source, options);
      check(source, answer, best, true); assert(answer.start_submitted);
    }
  }
}
void zero_cost_ties() {
  // A stable FIFO tie-break alone explores many equal-bound regions before
  // reaching a leaf. The unique region retaining the DP witness must remain
  // reachable within a linear admission quota despite these zero-cost bits.
  constexpr int count = 12;
  for (bool maximum : {false, true}) {
    Model m; std::vector<Term> weights;
    for (int i = 0; i < count; ++i) weights.push_back({m.add_binary(), 1});
    m.add_row(weights, -inf, 8);
    m.set_objective({{weights.front().variable, maximum ? 1.0 : -1.0}},
      maximum ? ObjectiveSense::Maximize : ObjectiveSense::Minimize, maximum ? -3 : 3);
    const auto source = m.snapshot(); const auto best = oracle(source);
    auto options = exact(); options.node_limit = 2 * count + 3;
    check(source, solve_native(source, options), best, true);
    for (auto order : {NativeSearchOrder::DepthFirst, NativeSearchOrder::BestBound}) {
      NativeSearchOptions search; search.solve = options; search.order = order;
      const auto result = solve_native_search(source, search);
      check(source, result.result, best, true);
      assert(result.frontier.admitted_nodes <= *options.node_limit);
    }
  }
}
void packed_reconstruction() {
  // Enumerated optima cover decision bits spanning word/row boundaries, signed
  // costs, both row orientations and objective senses. Reconstruction must use
  // decisions from the original row, never a subsequently overwritten value.
  const int weights[] = {3,4,5,7,9,12,13,17};
  const int costs[] = {-7,-8,-11,3,-12,-16,0,-22};
  for (bool negative : {false,true}) for (bool maximum : {false,true})
    for (int capacity : {17,31,63,64,65}) {
      Model m; std::vector<Term> row, objective_terms;
      for (int i=0;i<8;++i) {
        const auto x=m.add_binary();
        row.push_back({x,double(negative ? -weights[i] : weights[i])});
        objective_terms.push_back({x,double(maximum ? -costs[i] : costs[i])});
      }
      m.add_row(row,negative ? -capacity : -inf,negative ? inf : capacity);
      m.set_objective(objective_terms,maximum ? ObjectiveSense::Maximize : ObjectiveSense::Minimize,
                      maximum ? -9 : 9);
      const auto source=m.snapshot(); const auto best=oracle(source);
      check(source,solve_native(source,exact()),best,true);
      for (auto order : {NativeSearchOrder::DepthFirst,NativeSearchOrder::BestBound}) {
        NativeSearchOptions options; options.solve=exact(); options.order=order;
        check(source,solve_native_search(source,options).result,best,true);
      }
    }
  // Equal-profit choices across multiple packed words retain the first item:
  // every later equal-cost recurrence skips its item, including zero-cost bits.
  Model ties; std::vector<Term> row, costs_with_ties;
  std::vector<Variable> variables;
  for (int i=0;i<70;++i) {
    variables.push_back(ties.add_binary()); row.push_back({variables.back(),1});
    if (i<69) costs_with_ties.push_back({variables.back(),-1});
  }
  ties.add_row(row,-inf,1); ties.minimize(costs_with_ties);
  const auto source=ties.snapshot();
  for (int mode=0;mode<3;++mode) {
    NativeSearchOptions options; options.solve=exact();
    options.order=mode==1 ? NativeSearchOrder::DepthFirst : NativeSearchOrder::BestBound;
    const auto result=mode==0 ? solve_native(source,options.solve)
                             : solve_native_search(source,options).result;
    check(source,result,-1,true);
    for (std::size_t i=0;i<variables.size();++i)
      assert(result.value(variables[i])==(i==0 ? 1 : 0));
  }
}
void fallback_models() {
  for (int kind = 0; kind < 8; ++kind) {
    auto m = capacity_model(); const auto source = m.snapshot();
    const auto a = source.variables[1].variable, b = source.variables[2].variable;
    switch (kind) {
    case 0: m.add_row({{a, 1}}, -inf, 1); break; // Extra row, even if redundant.
    case 1: m.add_binary(); break; // Row does not cover every active column.
    case 2: m.set_bounds(a, 0, 0); break;
    case 3: add_table(m, {a, b}, {{0, 0}, {0, 1}}); break;
    case 4: add_indicator(m, a, true, {{b, 1}}, 1, 1); break;
    case 5: m.set_bounds(source.rows[0].constraint, 1, 3); break;
    case 6: m.set_coefficient(source.rows[0].constraint, b, -2); break;
    case 7: m.add_variable(VariableType::SemiInteger, 2, 3); break;
    }
    exercise(m);
  }
  Model integers; auto a = integers.add_integer(0, 1), b = integers.add_integer(0, 1);
  integers.add_row({{a, 2}, {b, 2}}, -inf, 3); integers.minimize({{a, -3}, {b, -2}});
  exercise(integers); // Binary-valued Integer is deliberately outside admission.
  auto unsupported = capacity_model(); auto malformed = unsupported.snapshot();
  malformed.rows[0].terms[0].coefficient = std::numeric_limits<double>::infinity();
  assert(solve_native(malformed, exact()).termination == Termination::InvalidModel);
}
void cap_boundaries() {
  // Independent optimum: at most one profitable item fits; all other items cost
  // zero and can be excluded. These checks need no exponential large-box oracle.
  for (int capacity : {65536, 65537}) {
    Model m; auto a = m.add_binary(), b = m.add_binary();
    m.add_row({{a, 40000}, {b, 40000}}, -inf, capacity); m.minimize({{a, -3}, {b, -2}});
    NativeSearchOptions o; o.solve = exact(); o.solve.node_limit = 1;
    const auto r = solve_native_search(m, o); ++runs;
    assert(r.result.termination == Termination::NodeLimit && !r.result.has_solution());
    assert(r.result.best_bound == (capacity == 65536 ? -3 : -5));
  }
  // This shape exceeded the old million-int64-cell table, but now passes the
  // shared automatic admission. The route assertion does not depend on whether
  // optional DP finishes before its local time cap on a slow/contended host.
  for (bool negative : {false,true}) for (bool maximum : {false,true}) {
    Model m; std::vector<Term> weights, costs;
    for (int i=0;i<17;++i) {
      const auto x=m.add_binary(); weights.push_back({x,negative ? -40000.0 : 40000.0});
      if (i<2) costs.push_back({x,(maximum ? 1.0 : -1.0)*(i ? 2 : 3)});
    }
    m.add_row(weights,negative ? -65536 : -inf,negative ? inf : 65536);
    m.set_objective(costs,maximum ? ObjectiveSense::Maximize : ObjectiveSense::Minimize,
                    maximum ? -11 : 11);
    auto options=exact(); options.node_limit=1;
    const auto selected=solve_native_auto(m,options); ++runs;
    if (native_lp_capabilities().available)
      assert(selected.message.find("eligible exact knapsack DP")!=std::string::npos);
    else assert(selected.backend=="Gecode native"); // Native-only route still runs DP.
    assert(selected.termination==Termination::NodeLimit && !selected.has_solution());
    const auto source=m.snapshot();
    check(source,solve_native(source,exact()),maximum ? -8 : 8,true);
    // Pre-cancelled large admission cannot publish any partial witness/bound.
    // The existing native-search fault gate also cancels after one completed
    // recurrence row, so retaining that hook tests interruption after allocation.
    options=exact(); options.cancellation=std::make_shared<CancellationToken>();
    options.cancellation->cancel();
    const auto cancelled=solve_native(source,options); ++runs;
    assert(cancelled.termination==Termination::Cancelled);
    assert(!cancelled.has_solution() && !cancelled.best_bound);
  }
  // The new work cap is independent of packed storage: 489*65536 transitions
  // exceed 32 million although the compact arrays would fit the byte cap.
  Model oversized; std::vector<Term> weights, costs;
  for (int i=0;i<489;++i) {
    const auto x=oversized.add_binary(); weights.push_back({x,40000});
    if (i<2) costs.push_back({x,i ? -2.0 : -3.0});
  }
  oversized.add_row(weights,-inf,65535); oversized.minimize(costs);
  NativeSearchOptions options; options.solve=exact(); options.solve.node_limit=1;
  const auto fallback=solve_native_search(oversized,options); ++runs;
  assert(fallback.result.termination==Termination::NodeLimit && !fallback.result.has_solution());
  assert(fallback.result.best_bound==-5);
}
void explicit_lp_and_stops() {
  auto m = capacity_model(); const auto source = m.snapshot();
  if (native_lp_capabilities().available) {
    NativeLpOptions lp; lp.solve = exact();
    check(source, solve_native_lp(source, lp).result, oracle(source), true);
    NativeSearchOptions search; search.solve = exact(); search.solve.node_limit = 1;
    search.relaxation = NativeLpSettings{};
    const auto r = solve_native_search(source, search); ++runs;
    assert(r.result.termination == Termination::NodeLimit && !r.result.has_solution());
    assert(r.result.best_bound == 7); // Exact LP ceil(-4)+offset11; DP would give8.
  }
  for (int mode = 0; mode < 3; ++mode) {
    auto options = exact();
    if (mode == 0) options.node_limit = 0;
    if (mode == 1) options.time_limit_seconds = 0;
    if (mode == 2) { options.cancellation = std::make_shared<CancellationToken>(); options.cancellation->cancel(); }
    const auto r = solve_native(source, options); ++runs;
    assert(r.termination == (mode == 0 ? Termination::NodeLimit : mode == 1 ? Termination::TimeLimit : Termination::Cancelled));
    assert(!r.has_solution() && !r.best_bound);
  }
  std::vector<std::future<SolveResult>> pending;
  for (bool maximum : {false, true}) {
    pending.push_back(std::async(std::launch::async, [maximum] {
      auto local = capacity_model(true, maximum);
      return solve_native(local.snapshot(), exact()); // Compiler/root die before result use.
    }));
  }
  assert(pending[0].get().objective == 8); assert(pending[1].get().objective == -8); runs += 2;
}
} // namespace
int main() {
  if (!native_capabilities().available) {
    auto m = capacity_model(); assert(solve_native(m, exact()).termination == Termination::Unsupported);
    NativeSearchOptions o; o.solve = exact();
    assert(solve_native_search(m, o).result.termination == Termination::Unsupported);
    std::cout << "Native knapsack unavailable: explicit boundaries pass\n"; return 0;
  }
  pure_models(); exact_bound_and_work(); zero_cost_ties(); packed_reconstruction(); fallback_models();
  cap_boundaries(); explicit_lp_and_stops();
  std::cout << runs << " native knapsack original-oracle and budget configurations pass\n";
}
