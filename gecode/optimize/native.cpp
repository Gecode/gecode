#include <gecode/optimize/native.hpp>
#include <gecode/optimize/native_lp.hpp>
#include <gecode/optimize/native_search.hpp>
#include <gecode/optimize/native_neighborhoods.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/globals.hpp>
#include <gecode/optimize/native_regular_limits.hpp>
#include <gecode/optimize/native_preprocess.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <type_traits>

#ifdef GECODE_OPTIMIZE_WITH_NATIVE
#include <gecode/int.hh>
#include <gecode/search.hh>
#endif

#if defined(GECODE_OPTIMIZE_WITH_NATIVE) && defined(GECODE_OPTIMIZE_WITH_HIGHS) && \
    defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
#define GECODE_OPTIMIZE_NATIVE_LP_ENABLED 1
#include <gecode/minimodel/lp-relaxation.hpp>
#include <gecode/minimodel/lp-cut-loop.hpp>
#endif

namespace Gecode { namespace Optimize {
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
// Coordinator-only fault injection; no production callback or ABI surface.
void native_search_test_event(const char* event);
#endif
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
void native_root_cut_test_event(const char* event, NativeRootCoverCompletion& completion);
#endif
#ifdef GECODE_NATIVE_START_TEST_HOOKS
void native_start_test_event(const char* event);
#endif
#ifdef GECODE_NATIVE_BRANCHING_TEST_HOOKS
void native_branching_test_event(const char* event, std::size_t slot, double& down, double& up);
#endif
#ifdef GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS
void native_neighborhood_test_event(const char*, std::size_t, int, int, std::vector<double>*);
#endif
namespace {
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
namespace LP = Gecode::Experimental::LpRelaxation;
#endif
class Unsupported : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};
struct Interrupted { Termination reason; };
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
void neighborhood_event(const char* event, std::size_t slot=0, int lower=0, int upper=0,
                        std::vector<double>* values=nullptr) {
#ifdef GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS
  native_neighborhood_test_event(event,slot,lower,upper,values);
#else
  (void)event;(void)slot;(void)lower;(void)upper;(void)values;
#endif
}
void checkpoint(const SolveBudget& budget) {
  if (const auto reason = budget.stop_reason()) throw Interrupted{*reason};
}

using NeighborhoodCompletion = NativeNeighborhoodCompletion;
struct NeighborhoodStopped { NeighborhoodCompletion reason; };
class NeighborhoodBudget {
  const NativeNeighborhoodSettings& settings;
  NativeNeighborhoodStatistics& statistics;
  const SolveBudget& shared;
  const double started;
public:
  NeighborhoodBudget(const NativeNeighborhoodSettings& settings0,
                     NativeNeighborhoodStatistics& statistics0,const SolveBudget& shared0)
    :settings(settings0),statistics(statistics0),shared(shared0),started(shared0.elapsed_seconds()) {}
  void check(const char* event="neighborhood_checkpoint") const {
    neighborhood_event(event);
    if(shared.cancelled()) throw Interrupted{Termination::Cancelled};
    if(shared.time_limit_reached()) throw Interrupted{Termination::TimeLimit};
    if(shared.elapsed_seconds()-started>=settings.time_limit_seconds)
      throw NeighborhoodStopped{NeighborhoodCompletion::LocalTimeLimit};
  }
  void work(std::size_t count=1) {
    check();
    if(count>settings.max_coordinator_work-statistics.coordinator_work)
      throw NeighborhoodStopped{NeighborhoodCompletion::WorkLimit};
    statistics.coordinator_work+=count;
  }
  void source(std::size_t count=1) {
    check();
    if(count>settings.max_source_entries-statistics.source_entries)
      throw NeighborhoodStopped{NeighborhoodCompletion::SourceLimit};
    work(count); statistics.source_entries+=count;
  }
  void finish() noexcept {statistics.elapsed_seconds=shared.elapsed_seconds()-started;}
  double start_seconds() const noexcept {return started;}
};

void construction_checkpoint(const SolveBudget& budget,NeighborhoodBudget* local,
                             const char* event="neighborhood_construction") {
  if(local) local->check(event); else checkpoint(budget);
}

constexpr std::int64_t exact_double_integer = INT64_C(9007199254740992);

int integer(double value, const char* kind) {
  if (!std::isfinite(value) || value != std::trunc(value) ||
      value < Int::Limits::min || value > Int::Limits::max)
    throw Unsupported(std::string(kind) + " must be integral and within native Gecode Int::Limits");
  return static_cast<int>(value);
}
std::optional<int> finite_bound(double value) {
  return std::isfinite(value) ? std::optional<int>(integer(value, "Row bound")) : std::nullopt;
}
struct NativeVariable {
  std::size_t slot;
  int lower;
  int upper;
  bool semi;
};
struct NativeTerm { int column; int coefficient; };
struct NativeRow {
  std::vector<NativeTerm> terms;
  std::optional<int> lower;
  std::optional<int> upper;
};
struct NativeIndicator {
  int activator;
  bool active_value;
  std::optional<int> gate;
  NativeRow row;
};
struct NativeRegular {
  int initial = 0;
  bool accepts_empty = false;
  std::vector<DFA::Transition> transitions; // Includes explicit native sentinel.
  std::vector<int> finals;                  // Deduplicated, with final -1.
};
NativeRegular prepare_regular(const RegularData& data,const SolveBudget& budget) {
  std::map<std::uint64_t,int> states;states.emplace(data.initial_state,0);
  std::set<std::uint64_t> finals;
  std::set<int> symbols;
  for(const auto& edge:data.transitions){
    checkpoint(budget);
    if(edge.symbol<Int::Limits::min||edge.symbol>Int::Limits::max)
      throw Unsupported("Regular alphabet symbol exceeds native Gecode limits");
    symbols.insert(static_cast<int>(edge.symbol));
    states.emplace(edge.from,0);states.emplace(edge.to,0);
  }
  for(auto state:data.final_states){checkpoint(budget);states.emplace(state,0);finals.insert(state);}
  const bool short_symbols=symbols.empty()||(*symbols.begin()>=std::numeric_limits<short>::min()&&*symbols.rbegin()<=std::numeric_limits<short>::max());
  if(const char* error=Detail::native_regular_size_error(data.variables.size(),states.size(),data.transitions.size(),symbols.size(),short_symbols))
    throw Unsupported(error);
  // Original state IDs need not be small or dense. Unreferenced declared states
  // have no transitions or accepting role and never require native storage.
  int next=0;for(auto& entry:states){checkpoint(budget);entry.second=next++;}
  NativeRegular result;result.initial=states.at(data.initial_state);
  result.accepts_empty=finals.count(data.initial_state)!=0;
  result.transitions.reserve(data.transitions.size()+1);result.finals.reserve(finals.size()+1);
  for(const auto& edge:data.transitions){checkpoint(budget);result.transitions.emplace_back(states.at(edge.from),static_cast<int>(edge.symbol),states.at(edge.to));}
  result.transitions.emplace_back(-1,0,0);
  for(auto state:finals){checkpoint(budget);result.finals.push_back(states.at(state));}
  result.finals.push_back(-1);checkpoint(budget);return result;
}
struct Compiled {
  std::vector<NativeVariable> variables;
  std::vector<NativeRow> rows;
  std::vector<NativeIndicator> indicators;
  std::vector<NativeTerm> objective;
  std::vector<int> columns;
  std::vector<const GlobalPayload*> globals;
  std::map<const GlobalPayload*,NativeRegular> regulars;
  int cost_lower = 0;
  int cost_upper = 0;
  std::int64_t offset = 0;
  bool minimize = true;
  bool binary_domains = true;
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  std::shared_ptr<LP::BoundedIntegerBackend> relaxation;
  // Keep exact original-source cut attribution for every live native clone.
  std::shared_ptr<const LP::Cuts::RootLoopResult> root_cover_evidence;
  LP::IntegerOptions relaxation_options;
#endif
};

Compiled compile(const ModelSnapshot& model, const SolveBudget& budget) {
  Compiled result;
  std::vector<int> column(model.variables.size(), -1);
  for (const auto& variable : model.variables) {
    neighborhood_event("compile_variable",static_cast<std::size_t>(variable.variable.id));
    checkpoint(budget);
    if (!variable.active) continue;
    result.binary_domains = result.binary_domains &&
      variable.type == VariableType::Binary && variable.lower == 0 && variable.upper == 1;
    if (variable.type != VariableType::Integer && variable.type != VariableType::Binary &&
        variable.type != VariableType::SemiInteger)
      throw Unsupported("Native bridge supports Integer, Binary and SemiInteger variables only");
    if (result.variables.size() >= static_cast<std::size_t>(Int::Limits::max))
      throw Unsupported("Too many variables for a native IntVarArray");
    column[variable.variable.id] = static_cast<int>(result.variables.size());
    result.variables.push_back({static_cast<std::size_t>(variable.variable.id),
      integer(variable.lower, "Variable lower bound"),
      integer(variable.upper, "Variable upper bound"),
      variable.type == VariableType::SemiInteger});
  }
  const auto terms = [&](const std::vector<Term>& original, std::int64_t limit,
                         int* minimum = nullptr, int* maximum = nullptr) {
    std::vector<NativeTerm> converted;
    std::int64_t magnitude = 0, lower = 0, upper = 0;
    for (const auto& term : original) {
      checkpoint(budget);
      const int coefficient = integer(term.coefficient, "Linear coefficient");
      const int index = column[term.variable.id];
      const auto& variable = result.variables[static_cast<std::size_t>(index)];
      const std::int64_t a = static_cast<std::int64_t>(coefficient) * (variable.semi ? 0 : variable.lower);
      const std::int64_t b = static_cast<std::int64_t>(coefficient) * variable.upper;
      const auto absolute = std::max(std::abs(a), std::abs(b));
      if (absolute > limit - magnitude)
        throw Unsupported("Native linear activity exceeds conservative exact propagation limits; tighten bounds or rescale with exact integers");
      magnitude += absolute;
      lower += std::min(a, b); upper += std::max(a, b);
      converted.push_back({index, coefficient});
    }
    if (minimum) *minimum = static_cast<int>(lower);
    if (maximum) *maximum = static_cast<int>(upper);
    return converted;
  };
  const auto row = [&](const std::vector<Term>& original, double lower, double upper) {
    return NativeRow{terms(original, Int::Limits::max), finite_bound(lower), finite_bound(upper)};
  };
  for (const auto& original : model.rows) {
    checkpoint(budget);
    if (!original.active || original.indicator_origin) continue;
    result.rows.push_back(row(original.terms, original.lower, original.upper));
  }
  for (const auto& original : model.indicators) {
    checkpoint(budget);
    if (!original.active) continue;
    NativeIndicator indicator{column[original.activator.id], original.active_value, {},
                               row(original.terms, original.lower, original.upper)};
    if (original.inactive_gate) indicator.gate = column[original.inactive_gate->id];
    result.indicators.push_back(std::move(indicator));
  }
  // The cost equality includes a cost variable in addition to its expression.
  // Reserving half the native range bounds that complete equality's activity.
  result.objective = terms(model.objective.terms, Int::Limits::max / 2,
                             &result.cost_lower, &result.cost_upper);
  if (model.objective.offset != std::trunc(model.objective.offset) ||
      std::abs(model.objective.offset) > static_cast<double>(exact_double_integer -
        std::max(std::abs(result.cost_lower), std::abs(result.cost_upper))))
    throw Unsupported("Native objective offset and all attainable objective values must be exact double integers (magnitude <= 2^53)");
  result.offset = static_cast<std::int64_t>(model.objective.offset);
  result.minimize = model.objective.sense == ObjectiveSense::Minimize;
  const auto native_constant=[](std::int64_t value) {
    if(value<Int::Limits::min || value>Int::Limits::max)
      throw Unsupported("Global integer constant exceeds native Gecode limits");
  };
  for(const auto& global:model.globals) if(global.active) {
    checkpoint(budget);
    if(Detail::global_variables(global.payload).size()>static_cast<std::size_t>(Int::Limits::max))
      throw Unsupported("Too many global arguments for native arrays");
    std::visit([&](const auto& data) {
      using T=std::decay_t<decltype(data)>;
      if constexpr(std::is_same_v<T,ElementData> || std::is_same_v<T,CircuitData>) {
        const auto count=[&]() {if constexpr(std::is_same_v<T,ElementData>) return data.elements.size();else return data.successors.size();}();
        native_constant(data.index_base);
        if(count) native_constant(data.index_base+static_cast<std::int64_t>(count-1));
      } else if constexpr(std::is_same_v<T,TableData>) {
        if(data.tuples.size()>static_cast<std::size_t>(Int::Limits::max)) throw Unsupported("Too many native table tuples");
        for(const auto& tuple:data.tuples) {checkpoint(budget);for(const auto value:tuple) native_constant(value);}
      } else if constexpr(std::is_same_v<T,CumulativeData>) {
        native_constant(data.capacity);
        constexpr auto limit=std::numeric_limits<std::int64_t>::max();
        std::int64_t widths=0,energy=0,count=0,time_magnitude=0;
        std::int64_t min_height=Int::Limits::max;
        bool overloaded_task=false;
        for(std::size_t i=0;i<data.starts.size();++i) {
          checkpoint(budget);
          if(!data.durations[i] || !data.heights[i]) continue;
          native_constant(data.durations[i]);native_constant(data.heights[i]);
          const auto& variable=result.variables[column[data.starts[i].id]];
          const std::int64_t start=variable.semi?0:variable.lower;
          const std::int64_t end=static_cast<std::int64_t>(variable.upper)+data.durations[i];
          native_constant(end);
          time_magnitude=std::max({time_magnitude,std::abs(start),std::abs(end)});
          min_height=std::min(min_height,data.heights[i]);
          overloaded_task=overloaded_task || data.heights[i]>data.capacity;
          const std::int64_t width=static_cast<std::int64_t>(variable.upper)-start+1;
          const auto task_energy=data.durations[i]*data.heights[i]; // factors <= native int max
          if(width>limit-widths || task_energy>limit-energy)
            throw Unsupported("Native cumulative arithmetic exceeds exact int64 accumulation");
          widths+=width;energy+=task_energy;++count;
        }
        // Zero/singleton task sets and an individually overloaded mandatory
        // task are resolved directly when posting; they need no resource tree.
        if(count<=1 || overloaded_task) return;
        if(data.capacity && (widths>limit/data.capacity ||
             data.capacity*widths>limit/count))
          throw Unsupported("Native cumulative propagation energy range exceeds int64");
        // cumulative/edge-finding.hpp indexes an int update[count*unique_heights]
        // array; count^2 is a conservative bound and also protects task-tree
        // node/event counts and next-power-of-two arithmetic.
        if(count>Int::Limits::max/count)
          throw Unsupported("Native cumulative task-count product exceeds integer array limits");
        // In either time direction all task endpoints have magnitude <= B.
        // Omega/Lambda subtree energies are <= E, and their envelopes are
        // bounded by C*B+E. Edge finding subtracts (C-height)*lct, so its
        // numerator has magnitude <= 2*C*B+E. It then divides by height and
        // casts to int. Require 2*C*B+E <= native_max*min_height, protecting
        // that cast as well as every int64 envelope/difference intermediate.
        // This also implies 2*B+sum(duration)<=native_max, bounding unary
        // tree arithmetic when all task pairs must run disjointly.
        // Products here fit int64: every factor is within native int limits.
        const std::int64_t time_product=data.capacity*time_magnitude;
        const std::int64_t difference_limit=static_cast<std::int64_t>(Int::Limits::max)*min_height;
        if(energy>difference_limit || time_product>(difference_limit-energy)/2)
          throw Unsupported("Native cumulative envelope/difference exceeds exact propagation limits; reduce the time range, durations or resource scale");
      } else if constexpr(std::is_same_v<T,RegularData>) {
        result.regulars.emplace(&global.payload,prepare_regular(data,budget));
      }
    },global.payload);
    result.globals.push_back(&global.payload);
  }
  result.columns=std::move(column);
  return result;
}

#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
LP::BoundedIntegerModel relaxation_model(const Compiled& compiled,
                                        const SolveBudget& budget) {
  LP::BoundedIntegerModel result;
  result.linear.c.resize(compiled.variables.size(), 0);
  for (const auto& variable : compiled.variables) {
    checkpoint(budget);
    result.lower.push_back(variable.semi ? 0 : variable.lower);
    result.upper.push_back(variable.upper);
  }
  for (const auto& term : compiled.objective) {
    checkpoint(budget);
    result.linear.c[term.column] = compiled.minimize ? term.coefficient : -term.coefficient;
  }
  const auto side = [&](const NativeRow& row, int bound, int sign) {
    checkpoint(budget);
    for (const auto& term : row.terms) {
      checkpoint(budget);
      result.linear.column.push_back(static_cast<std::size_t>(term.column));
      result.linear.a.push_back(static_cast<std::int64_t>(sign) * term.coefficient);
    }
    result.linear.b.push_back(static_cast<std::int64_t>(sign) * bound);
    result.linear.row_start.push_back(result.linear.a.size());
  };
  for (const auto& row : compiled.rows) {
    if (row.lower) side(row, *row.lower, 1);
    if (row.upper) side(row, *row.upper, -1);
  }
  try { LP::validate_integer_model(result); }
  catch (const std::invalid_argument& error) {
    throw Unsupported(std::string("Native LP relaxation: ") + error.what());
  }
  return result;
}

struct RelaxationState {
  std::shared_ptr<LP::BoundedIntegerBackend> backend;
  std::shared_ptr<const LP::Cuts::RootLoopResult> root_cover_evidence;
  LP::Stats handoff;
  NativeRootCoverStatistics root;
};

void root_cut_event(const char* event, NativeRootCoverCompletion& completion) {
#ifdef GECODE_NATIVE_ROOT_CUT_TEST_HOOKS
  native_root_cut_test_event(event, completion);
#else
  (void)event; (void)completion;
#endif
}

NativeRootCoverCompletion root_completion(LP::Cuts::RootLoopCompletion value) {
  using From=LP::Cuts::RootLoopCompletion;using To=NativeRootCoverCompletion;
  switch(value) {
  case From::NoNewCuts:return To::NoNewCuts;
  case From::RoundLimit:return To::RoundLimit;
  case From::WorkLimit:return To::WorkLimit;
  case From::StorageLimit:return To::StorageLimit;
  case From::SeparationLimit:return To::SeparationLimit;
  case From::Cancelled:return To::Cancelled;
  case From::TimeLimit:return To::TimeLimit;
  case From::NoPrimalSuggestion:return To::NoPrimalSuggestion;
  case From::InvalidSuggestion:return To::InvalidSuggestion;
  case From::CallbackError:return To::CallbackError;
  case From::BackendError:return To::BackendError;
  case From::AllocationFailure:return To::AllocationFailure;
  }
  throw std::runtime_error("Unknown root cover loop completion");
}

void prepare_relaxation(Compiled& compiled,const NativeLpSettings& settings,
                        const SolveBudget& budget,RelaxationState& state) {
  auto original=relaxation_model(compiled,budget);
  if (settings.root_cover_cuts) {
    const auto& limits=*settings.root_cover_cuts;
    LP::Cuts::RootLoopOptions options;
    options.max_rounds=limits.max_rounds;options.max_work=limits.max_work;
    options.pool.max_cuts=limits.max_cuts;options.pool.max_nonzeros=limits.max_cut_nonzeros;
    options.max_columns=limits.max_model_columns;options.max_rows=limits.max_model_rows;
    options.max_nonzeros=limits.max_model_nonzeros;
    options.separation.max_rows=limits.max_separation_rows;
    options.separation.max_terms_per_row=limits.max_terms_per_row;
    options.denominator=limits.denominator;
    options.stop_requested=[&budget](){return budget.expired();};
    using Clock=std::chrono::steady_clock;
    const auto now=Clock::now();const double remaining=budget.remaining_seconds();
    // Very large finite/infinite solve budgets need no representable absolute
    // deadline: the shared predicate still checks their original budget.
    const auto available=std::chrono::duration<long double>(Clock::time_point::max()-now).count();
    if (std::isfinite(remaining) && static_cast<long double>(remaining)<available)
      options.deadline=now+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(remaining));
    root_cut_event("before_loop",state.root.completion);checkpoint(budget);
    auto loop=LP::Cuts::root_cover_loop(LP::Cuts::SourceModel(std::move(original)),options);
    const auto& stats=loop.stats;
    state.root.completion=root_completion(loop.completion);
    if (state.root.completion==NativeRootCoverCompletion::Cancelled &&
        !budget.cancelled() && budget.time_limit_reached())
      state.root.completion=NativeRootCoverCompletion::TimeLimit;
    state.root.rounds=stats.rounds;state.root.augmentations=stats.augmentations;
    state.root.cuts=loop.cuts.size();state.root.work=stats.work;
    for (const auto& cut:loop.cuts) state.root.nonzeros+=cut.inequality().column.size();
    state.root.projected_coordinates=stats.projected_coordinates;
    state.root.unsupported_rows=stats.unsupported_rows;state.root.oversized_rows=stats.oversized_rows;
    state.root.separated_cuts=stats.separated_cuts;state.root.duplicate_cuts=stats.duplicate_cuts;
    state.root.arithmetic_rejections=stats.arithmetic_rejections;
    state.root.lp_calls=stats.lp_calls;state.root.lp_seconds=stats.lp_seconds;
    state.root.valid_bounds=stats.valid_bounds;state.root.rejected_bounds=stats.rejected_bounds;
    state.root.numerical_infeasibility_reports=stats.floating_infeasible_reports;
    state.backend=loop.backend;
    if (state.backend) state.handoff=state.backend->statistics();
    state.root_cover_evidence=std::make_shared<const LP::Cuts::RootLoopResult>(std::move(loop));
    root_cut_event("after_loop",state.root.completion);
    checkpoint(budget); // Recover the precise outer time/cancel/node reason.
    switch(state.root.completion) {
    case NativeRootCoverCompletion::Cancelled:throw Interrupted{Termination::Cancelled};
    case NativeRootCoverCompletion::TimeLimit:throw Interrupted{Termination::TimeLimit};
    case NativeRootCoverCompletion::AllocationFailure:throw std::bad_alloc();
    case NativeRootCoverCompletion::CallbackError:
    case NativeRootCoverCompletion::BackendError:
      throw std::runtime_error("Native root cover loop failed before native search");
    case NativeRootCoverCompletion::NoNewCuts:
    case NativeRootCoverCompletion::RoundLimit:
    case NativeRootCoverCompletion::WorkLimit:
    case NativeRootCoverCompletion::StorageLimit:
    case NativeRootCoverCompletion::SeparationLimit:
    case NativeRootCoverCompletion::NoPrimalSuggestion:
    case NativeRootCoverCompletion::InvalidSuggestion:break;
    default:throw std::runtime_error("Native root cover loop returned no completion");
    }
    // Root cut limits bound this optional preprocessing only. If no backend was
    // constructed, ordinary checked native LP solving can still proceed.
    if (!state.backend) {
      state.backend=std::make_shared<LP::BoundedIntegerBackend>(state.root_cover_evidence->model.model());
      state.handoff=state.backend->statistics();
    }
  } else {
    state.backend=std::make_shared<LP::BoundedIntegerBackend>(std::move(original));
  }
  compiled.relaxation=state.backend;
  compiled.root_cover_evidence=state.root_cover_evidence;
  compiled.relaxation_options.frequency=settings.frequency==NativeLpFrequency::Root
    ? LP::Frequency::Root:LP::Frequency::EveryNode;
  compiled.relaxation_options.bound_tightening=settings.bound_tightening;
  compiled.relaxation_options.bound_change_interval=settings.bound_change_interval;
  root_cut_event("prepared",state.root.completion);checkpoint(budget);
}

NativeLpStatistics relaxation_statistics(const RelaxationState& state) {
  NativeLpStatistics result;result.root_cover=state.root;
  const auto after=state.backend?state.backend->statistics():LP::Stats{};
  const auto& before=state.handoff;
  result.lp_calls=state.root.lp_calls+(after.lp_calls-before.lp_calls);
  result.lp_seconds=state.root.lp_seconds+(after.lp_ms-before.lp_ms)/1000.0;
  result.valid_bounds=state.root.valid_bounds+(after.valid_bounds-before.valid_bounds);
  result.rejected_bounds=state.root.rejected_bounds+(after.rejected-before.rejected);
  result.numerical_infeasibility_reports=state.root.numerical_infeasibility_reports+
    (after.infeasible_status-before.infeasible_status);
  result.certificate_evaluations=after.certificate_evaluations-before.certificate_evaluations;
  result.conditional_checks=after.conditional_checks-before.conditional_checks;
  result.variable_fixings=after.variable_fixings-before.variable_fixings;
  result.variable_bound_tightenings=after.variable_bound_tightenings-before.variable_bound_tightenings;
  return result;
}
#endif

struct KnapsackBound {
  int objective;
  std::shared_ptr<const std::vector<unsigned char>> witness;
};

struct KnapsackData {
  std::size_t capacity;
  std::vector<std::int64_t> weights, costs;
};

// Shared exact admission for both automatic routing and DP construction.
std::optional<KnapsackData> prepare_knapsack(const Compiled& model,
                                            const SolveBudget& budget) {
  const auto n = model.variables.size();
  if (!model.binary_domains || !n || model.rows.size() != 1 ||
      !model.indicators.empty() || !model.globals.empty()) return {};
  const auto& row = model.rows.front();
  if (row.terms.size() != n) return {};
  const bool positive = row.terms.front().coefficient > 0;
  if (positive ? (!row.upper || (row.lower && *row.lower > 0))
               : (!row.lower || (row.upper && *row.upper < 0))) return {};
  const std::int64_t capacity = positive ? *row.upper : -std::int64_t(*row.lower);
  constexpr std::size_t max_transitions = 32000000;
  constexpr std::size_t max_payload_bytes = 8 * 1024 * 1024;
  if (capacity < 0 || capacity > 65536) return {};
  const auto width = static_cast<std::size_t>(capacity) + 1;
  // Admit work and storage separately. Division protects every later product;
  // packed decisions need one bit per recurrence, not an int64 value per cell.
  if (n > max_transitions / width) return {};
  const auto transitions = n * width;
  const auto decision_words = transitions / 64 + (transitions % 64 != 0);
  std::size_t available = max_payload_bytes;
  const auto account = [&](std::size_t count, std::size_t bytes) {
    if (count > available / bytes) return false;
    available -= count * bytes;
    return true;
  };
  if (!account(width, sizeof(std::int64_t)) ||
      !account(width, sizeof(std::int64_t)) ||
      !account(decision_words, sizeof(std::uint64_t)) ||
      !account(n, sizeof(std::int64_t)) || // weights
      !account(n, sizeof(std::int64_t)) || // normalized costs
      !account(n, sizeof(unsigned char))) return {}; // reconstructed witness
  checkpoint(budget);
  std::vector<std::int64_t> weights(n, 0), costs(n, 0);
  for (const auto& term : row.terms) {
    checkpoint(budget);
    const auto weight = positive ? std::int64_t(term.coefficient)
                                : -std::int64_t(term.coefficient);
    if (weight <= 0 || term.column < 0 || static_cast<std::size_t>(term.column) >= n ||
        weights[static_cast<std::size_t>(term.column)] != 0) return {};
    weights[static_cast<std::size_t>(term.column)] = weight;
  }
  bool improves_zero = false;
  for (const auto& term : model.objective) {
    checkpoint(budget);
    costs[static_cast<std::size_t>(term.column)] = model.minimize
      ? std::int64_t(term.coefficient) : -std::int64_t(term.coefficient);
    improves_zero = improves_zero || costs[static_cast<std::size_t>(term.column)] < 0;
  }
  // Otherwise all-zero is already optimal and the ordinary minimum-first
  // brancher and original objective box provide the same preference and bound.
  if (!improves_zero) return {};
  return KnapsackData{static_cast<std::size_t>(capacity),std::move(weights),std::move(costs)};
}

// Exact root strengthening for one bounded binary capacity row. A missing
// result means the original native search should run without this optional work.
std::optional<KnapsackBound> knapsack_bound(const Compiled& model,
                                           const SolveBudget& budget) {
  const auto started = std::chrono::steady_clock::now();
  const auto within_budget = [&] {
    checkpoint(budget);
    // This optional preprocessing may stop without producing any evidence.
    // Original search keeps the remainder of the unchanged shared solve budget.
    return std::chrono::steady_clock::now() - started < std::chrono::milliseconds(250);
  };
  const auto admitted = prepare_knapsack(model,budget);
  if (!admitted || !within_budget()) return {};
  const auto n = model.variables.size(), capacity = admitted->capacity, width = capacity+1;
  const auto& weights = admitted->weights;
  const auto& costs = admitted->costs;
  // previous[c] minimizes normalized cost using the preceding items and at most
  // c capacity. Empty selection is feasible, so the base row is zero everywhere.
  // Each entry is a feasible subset cost. The compiler's absolute objective
  // activity bound protects every partial sum, including signed costs.
  std::vector<std::int64_t> previous(width, 0), current(width, 0);
  const auto transitions = n * width; // checked by shared admission
  std::vector<std::uint64_t> take(transitions / 64 + (transitions % 64 != 0), 0);
  if (!within_budget()) return {};
  for (std::size_t i = 0; i < n; ++i) {
    const auto row_start = i * width;
    for (std::size_t c = 0; c < width; ++c) {
      if ((c & 255) == 0 && !within_budget()) return {};
      auto value = previous[c];
      if (weights[i] <= static_cast<std::int64_t>(c)) {
        const auto included = costs[i] + previous[c - static_cast<std::size_t>(weights[i])];
        if (included < value) {
          value = included;
          const auto bit = row_start + c;
          take[bit / 64] |= std::uint64_t{1} << (bit % 64);
        }
      }
      current[c] = value;
    }
    previous.swap(current);
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
    native_search_test_event("knapsack_row_completed");
#endif
    if (!within_budget()) return {};
  }
  const auto normalized = previous[capacity];
  auto witness = std::make_shared<std::vector<unsigned char>>(n, 0);
  auto remaining = static_cast<std::size_t>(capacity);
  for (std::size_t i = n; i > 0; --i) {
    if (!within_budget()) return {};
    // Each bit belongs to its original item row, so overwritten rolling values
    // cannot corrupt reconstruction. A tie consistently excludes the item.
    const auto bit = (i - 1) * width + remaining;
    if ((take[bit / 64] >> (bit % 64)) & std::uint64_t{1}) {
      if (weights[i - 1] > static_cast<std::int64_t>(remaining))
        throw std::runtime_error("Native knapsack reconstruction exceeds capacity");
      (*witness)[i - 1] = 1;
      remaining -= static_cast<std::size_t>(weights[i - 1]);
    }
  }
  std::int64_t load = 0, checked_cost = 0;
  for (std::size_t i = 0; i < n; ++i) {
    if (!within_budget()) return {};
    if ((*witness)[i]) { load += weights[i]; checked_cost += costs[i]; }
  }
  const auto raw = model.minimize ? normalized : -normalized;
  if (load > capacity || checked_cost != normalized ||
      raw < model.cost_lower || raw > model.cost_upper)
    throw std::runtime_error("Native knapsack witness failed exact reconstruction checks");
  if (!within_budget()) return {};
  return KnapsackBound{static_cast<int>(raw), std::move(witness)};
}

class NativeSpace final : public Space {
public:
  IntVarArray variables;
  IntVar cost;
  bool minimize;
  std::shared_ptr<const std::vector<unsigned char>> knapsack_witness;

  NativeSpace(const Compiled& model, const SolveBudget& budget,
              NeighborhoodBudget* local=nullptr, bool root_knapsack=true,
              bool depth_first_knapsack=false)
      : variables(*this, static_cast<int>(model.variables.size())),
        cost(*this, model.cost_lower, model.cost_upper), minimize(model.minimize) {
    for (std::size_t i = 0; i < model.variables.size(); ++i) {
      construction_checkpoint(budget,local);
      const auto& variable = model.variables[i];
      if (variable.semi) {
        const int ranges[2][2] = {{0, 0}, {variable.lower, variable.upper}};
        variables[static_cast<int>(i)] = IntVar(*this, IntSet(ranges, 2));
      } else {
        variables[static_cast<int>(i)] = IntVar(*this, variable.lower, variable.upper);
      }
    }
    for (const auto& row : model.rows) {
      construction_checkpoint(budget,local); post(row);
      if(local) local->check("neighborhood_row_posted");
    }
    for (const auto& indicator : model.indicators) {
      construction_checkpoint(budget,local);
      BoolVar enabled(*this, 0, 1);
      rel(*this, variables[indicator.activator], IRT_EQ,
          indicator.active_value ? 1 : 0, Reify(enabled, RM_EQV));
      post(indicator.row, enabled);
      if (indicator.gate) {
        IntArgs coefficients(2); IntVarArgs vars(2);
        coefficients[0] = indicator.active_value ? 1 : -1;
        coefficients[1] = 1;
        vars[0] = variables[indicator.activator]; vars[1] = variables[*indicator.gate];
        linear(*this, coefficients, vars, IRT_EQ, indicator.active_value ? 1 : 0);
      }
    }
    for(const auto* global:model.globals) {
      construction_checkpoint(budget,local);post_global(*global,model,budget,local);
      if(local) local->check("neighborhood_global_posted");
    }
    IntArgs coefficients(static_cast<int>(model.objective.size()));
    IntVarArgs vars(static_cast<int>(model.objective.size()));
    for (std::size_t i = 0; i < model.objective.size(); ++i) {
      if(local) local->check();
      coefficients[static_cast<int>(i)] = model.objective[i].coefficient;
      vars[static_cast<int>(i)] = variables[model.objective[i].column];
    }
    linear(*this, coefficients, vars, IRT_EQ, cost);
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    if (model.relaxation && !local) {
      checkpoint(budget);
      IntVar lp_cost = cost;
      if (!minimize) {
        lp_cost = IntVar(*this, -model.cost_upper, -model.cost_lower);
        IntVarArgs pair(2); pair[0] = cost; pair[1] = lp_cost;
        linear(*this, pair, IRT_EQ, 0);
      }
      LP::integer_linear_minimize(*this, variables, lp_cost,
                                 model.relaxation, model.relaxation_options);
    }
#endif
    bool strengthen = root_knapsack && !local;
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    strengthen = strengthen && !model.relaxation;
#endif
    const auto knapsack = strengthen ? knapsack_bound(model, budget)
                                    : std::optional<KnapsackBound>{};
    if (knapsack) {
      checkpoint(budget);
      knapsack_witness = knapsack->witness;
      // A valid objective-side bound preserves every original feasible point.
      // The witness only orders choices; it never fixes the original model.
      rel(*this, cost, minimize ? IRT_GQ : IRT_LQ, knapsack->objective);
      branch(*this, variables, INT_VAR_SIZE_MIN(),
             INT_VAL([witness=knapsack->witness,depth_first_knapsack]
                     (const Space&, IntVar variable, int column) {
               const int bit = (*witness)[static_cast<std::size_t>(column)];
               const int value = depth_first_knapsack ? 1 - bit : bit;
               return variable.in(value) ? value : variable.min();
             }));
    } else {
      branch(*this, variables, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }
    if(local) local->check("neighborhood_constructed");
  }
  NativeSpace(NativeSpace& other)
      : Space(other), minimize(other.minimize), knapsack_witness(other.knapsack_witness) {
    variables.update(*this, other.variables); cost.update(*this, other.cost);
  }
  Space* copy() override { return new NativeSpace(*this); }
  void constrain(const Space& best) override {
    const auto& incumbent = static_cast<const NativeSpace&>(best);
    rel(*this, cost, minimize ? IRT_LE : IRT_GR, incumbent.cost.val());
  }
private:
  void post_global(const GlobalPayload& payload,const Compiled& model,const SolveBudget& budget,
                   NeighborhoodBudget* local=nullptr) {
    const auto variable=[&](Variable handle) {return variables[model.columns[handle.id]];};
    const auto args=[&](const std::vector<Variable>& handles) {
      IntVarArgs out(static_cast<int>(handles.size()));
      for(std::size_t i=0;i<handles.size();++i) {
        if(local) local->check();out[static_cast<int>(i)]=variable(handles[i]);
      }
      // Some propagators reject repeated variable objects. Equal fresh views
      // preserve aliases without weakening their mathematical meaning.
      unshare(*this,out);
      return out;
    };
    const auto false_constraint=[&]() {rel(*this,IntVar(*this,0,0),IRT_EQ,1);};
    const auto position=[&](Variable handle,std::int64_t base,std::size_t size) {
      IntVar index(*this,0,static_cast<int>(size-1));
      IntVarArgs vars(2);vars[0]=variable(handle);vars[1]=index;
      IntArgs coefficients(2);coefficients[0]=1;coefficients[1]=-1;
      linear(*this,coefficients,vars,IRT_EQ,static_cast<int>(base));
      return index;
    };
    std::visit([&](const auto& data) {
      using T=std::decay_t<decltype(data)>;
      if constexpr(std::is_same_v<T,AllDifferentData>) {
        if(data.variables.size()>1) distinct(*this,args(data.variables),IPL_DOM);
      } else if constexpr(std::is_same_v<T,ElementData>) {
        if(data.elements.empty()) {false_constraint();return;}
        element(*this,args(data.elements),position(data.index,data.index_base,data.elements.size()),variable(data.result),IPL_DOM);
      } else if constexpr(std::is_same_v<T,TableData>) {
        if(data.tuples.empty()) {false_constraint();return;}
        if(data.variables.empty()) return; // At least one empty tuple is true.
        TupleSet tuples(static_cast<int>(data.variables.size()));
        for(const auto& tuple:data.tuples) {
          construction_checkpoint(budget,local);IntArgs entry(static_cast<int>(tuple.size()));
          for(std::size_t i=0;i<tuple.size();++i) entry[static_cast<int>(i)]=static_cast<int>(tuple[i]);
          tuples.add(entry);
        }
        tuples.finalize();extensional(*this,args(data.variables),tuples,IPL_DOM);
      } else if constexpr(std::is_same_v<T,CumulativeData>) {
        std::vector<Variable> starts;std::vector<int> durations,heights;
        for(std::size_t i=0;i<data.starts.size();++i) if(data.durations[i] && data.heights[i]) {
          if(local) local->check();
          starts.push_back(data.starts[i]);durations.push_back(static_cast<int>(data.durations[i]));
          heights.push_back(static_cast<int>(data.heights[i]));
        }
        // Mandatory positive-duration demand above capacity is independently
        // infeasible. A singleton within capacity imposes no restriction on
        // its start. Handle both before native cumulative's min-capacity
        // calculation (whose second minimum uses an INT_MAX sentinel).
        if(std::any_of(heights.begin(),heights.end(),[&](int h){return h>data.capacity;})) {
          false_constraint();return;
        }
        if(starts.size()<=1) return;
        IntArgs p(static_cast<int>(starts.size())),h(static_cast<int>(starts.size()));
        for(std::size_t i=0;i<starts.size();++i) {p[static_cast<int>(i)]=durations[i];h[static_cast<int>(i)]=heights[i];}
        // Select the same disjunctive case with wide addition. Native
        // cumulative's int min_height+second_height can overflow for large
        // resource units, even though both individual heights are supported.
        std::int64_t first=Int::Limits::max,second=Int::Limits::max;
        for(const auto height:heights) {
          if(height<first) {second=first;first=height;}
          else second=std::min(second,static_cast<std::int64_t>(height));
        }
        const auto level=static_cast<IntPropLevel>(IPL_BASIC|IPL_ADVANCED);
        if(first+second>data.capacity) unary(*this,args(starts),p,level);
        else cumulative(*this,static_cast<int>(data.capacity),args(starts),p,h,level);
      } else if constexpr(std::is_same_v<T,RegularData>) {
        const auto& prepared=model.regulars.at(&payload);
        if(data.variables.empty()){if(!prepared.accepts_empty)false_constraint();return;}
        if(data.transitions.empty()||data.final_states.empty()){false_constraint();return;}
        // DFA's constructor may normalize the supplied arrays. Keep each copy
        // private, disable optional minimization, and checkpoint both sides of
        // the non-interruptible native construction/posting calls.
        auto transitions=prepared.transitions;auto finals=prepared.finals;
        construction_checkpoint(budget,local);DFA automaton(prepared.initial,transitions.data(),finals.data(),false);construction_checkpoint(budget,local);
        extensional(*this,args(data.variables),automaton,IPL_DOM);construction_checkpoint(budget,local);
      } else {
        IntVarArgs successors(static_cast<int>(data.successors.size()));
        for(std::size_t i=0;i<data.successors.size();++i) {
          if(local) local->check();
          successors[static_cast<int>(i)]=position(data.successors[i],data.index_base,data.successors.size());
        }
        circuit(*this,successors,IPL_DOM);
      }
    },payload);
  }
  void post(const NativeRow& row, std::optional<BoolVar> enabled = {}) {
    IntArgs coefficients(static_cast<int>(row.terms.size()));
    IntVarArgs vars(static_cast<int>(row.terms.size()));
    for (std::size_t i = 0; i < row.terms.size(); ++i) {
      coefficients[static_cast<int>(i)] = row.terms[i].coefficient;
      vars[static_cast<int>(i)] = variables[row.terms[i].column];
    }
    const auto side = [&](IntRelType relation, int bound) {
      if (enabled) linear(*this, coefficients, vars, relation, bound, Reify(*enabled, RM_IMP));
      else linear(*this, coefficients, vars, relation, bound);
    };
    if (row.lower && row.upper && *row.lower == *row.upper) side(IRT_EQ, *row.lower);
    else {
      if (row.lower) side(IRT_GQ, *row.lower);
      if (row.upper) side(IRT_LQ, *row.upper);
    }
  }
};

// Only assignment infeasibility has this type. Allocation/indexing/internal
// errors must never be relabelled as an invalid user start.
class ExactWitnessFailure : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};
void start_event(const char* event) {
#ifdef GECODE_NATIVE_START_TEST_HOOKS
  native_start_test_event(event);
#else
  (void)event;
#endif
}
void start_checkpoint(const SolveBudget& budget, const char* event) {
  start_event(event); checkpoint(budget);
}
bool exact_domain(const VariableData& variable, double value) {
  return std::isfinite(value) && value == std::trunc(value) &&
    ((variable.type == VariableType::SemiInteger && value == 0) ||
     (value >= variable.lower && value <= variable.upper));
}

// This checker reads the original snapshot and original slots, independently
// of NativeSpace's column mapping, propagation and cost variable. All casts and
// sums are justified by compile's integer and absolute-activity preconditions.
// Start-only checkpoints leave ordinary candidate/no-start scheduling unchanged.
std::int64_t check_exact(const ModelSnapshot& model, const std::vector<double>& values,
                         const SolveBudget* start_budget = nullptr, NeighborhoodBudget* local=nullptr) {
  const auto check = [&](const char* event) {
    if (start_budget) start_checkpoint(*start_budget, event);
    if(local) local->check(event);
  };
  for (const auto& variable : model.variables) {
    check("exact_variable");
    if (!variable.active) continue;
    const double value = values.at(static_cast<std::size_t>(variable.variable.id));
    if (!exact_domain(variable, value))
      throw ExactWitnessFailure("Native witness failed exact original variable check");
  }
  const auto activity = [&](const std::vector<Term>& terms) {
    std::int64_t sum = 0;
    for (const auto& term : terms) {
      check("exact_term");
      sum += static_cast<std::int64_t>(term.coefficient) *
             static_cast<std::int64_t>(values.at(static_cast<std::size_t>(term.variable.id)));
    }
    return sum;
  };
  const auto row = [&](const std::vector<Term>& terms, double lower, double upper) {
    const auto sum = activity(terms);
    if ((std::isfinite(lower) && sum < static_cast<std::int64_t>(lower)) ||
        (std::isfinite(upper) && sum > static_cast<std::int64_t>(upper)))
      throw ExactWitnessFailure("Native witness failed exact original row check");
  };
  for (const auto& original : model.rows) {
    check("exact_row");
    if (original.active && !original.indicator_origin)
      row(original.terms, original.lower, original.upper);
  }
  for (const auto& indicator : model.indicators) {
    check("exact_indicator");
    if (!indicator.active) continue;
    const bool enabled = values[indicator.activator.id] == (indicator.active_value ? 1 : 0);
    if (enabled) row(indicator.terms, indicator.lower, indicator.upper);
    if (indicator.inactive_gate && values[indicator.inactive_gate->id] != (enabled ? 0 : 1))
      throw ExactWitnessFailure("Native witness failed exact indicator gate check");
  }
  for (const auto& global : model.globals) if (global.active) {
    check("exact_global");
    std::string reason;
    if (!Detail::global_satisfied(global.payload, values, 0, reason))
      throw ExactWitnessFailure("Native witness failed exact global check: " + reason);
    check("after_exact_global");
  }
  check("exact_objective");
  return static_cast<std::int64_t>(model.objective.offset) + activity(model.objective.terms);
}

struct PreparedStart {
  std::vector<double> values;
  std::vector<bool> active;
  std::int64_t objective = 0;
  int cost = 0;
};
PreparedStart prepare_start(const ModelSnapshot& model, const Compiled& compiled,
                            const SolveOptions& options, const SolveBudget& budget) {
  start_checkpoint(budget, "before_start");
  PreparedStart result;
  result.values.assign(model.variables.size(), std::numeric_limits<double>::quiet_NaN());
  result.active.reserve(model.variables.size());
  std::vector<std::size_t> known;
  known.reserve(model.variables.size());
  for (const auto& entry : options.primal_start) {
    start_checkpoint(budget, "map_entry");
    const auto handle = entry.variable;
    if (handle.model_id != model.model_id || handle.id >= model.variables.size() ||
        !model.variables[handle.id].active)
      throw ModelError("Native primal start contains a foreign, invalid or deleted variable");
    auto& value = result.values[handle.id];
    if (!std::isnan(value)) throw ModelError("Native primal start contains a duplicate variable");
    if (!exact_domain(model.variables[handle.id], entry.value))
      throw ModelError("Native primal start violates an exact integer variable domain (no rounding is performed)");
    value = entry.value; known.push_back(static_cast<std::size_t>(handle.id));
  }
  // Only live original indicator equations determine omitted helper slots.
  // A slot is queued once, and each dependency is visited once, including
  // chains/reordered metadata. Unseeded cycles and removed gates stay unknown.
  std::vector<std::vector<std::size_t>> dependents(model.variables.size());
  for (std::size_t i = 0; i < model.indicators.size(); ++i) {
    start_checkpoint(budget, "gate_dependency");
    const auto& indicator = model.indicators[i];
    if (indicator.active && indicator.inactive_gate)
      dependents[indicator.activator.id].push_back(i);
  }
  for (std::size_t i = 0; i < known.size(); ++i) {
    start_checkpoint(budget, "known_slot");
    for (auto index : dependents[known[i]]) {
      start_checkpoint(budget, "derive_gate");
      const auto& indicator = model.indicators[index];
      const double gate = result.values[indicator.activator.id] == (indicator.active_value ? 1 : 0) ? 0 : 1;
      auto& value = result.values[indicator.inactive_gate->id];
      if (std::isnan(value)) {
        value = gate; known.push_back(static_cast<std::size_t>(indicator.inactive_gate->id));
      } else if (value != gate) {
        throw ModelError("Native primal start contradicts a live indicator inactivity gate");
      }
    }
  }
  for (const auto& variable : model.variables) {
    start_checkpoint(budget, "start_completeness");
    result.active.push_back(variable.active);
    if (variable.active && std::isnan(result.values[variable.variable.id]))
      throw Unsupported("Native primal start must determine every active variable; only live indicator inactivity gates are completed automatically");
  }
  start_checkpoint(budget, "before_exact_start");
  try { result.objective = check_exact(model, result.values, &budget); }
  catch (const ExactWitnessFailure& error) {
    throw ModelError(std::string("Complete native primal start is infeasible: ") + error.what());
  }
  start_checkpoint(budget, "after_exact_start");
  // compile bounds both quantities by +/-2^53, so this int64 subtraction is
  // exact and cannot overflow. The native cost must fit its preflighted range.
  const auto cost = result.objective - compiled.offset;
  if (cost < compiled.cost_lower || cost > compiled.cost_upper)
    throw std::runtime_error("Native primal start cost disagrees with compiled objective bounds");
  result.cost = static_cast<int>(cost);
  start_checkpoint(budget, "before_numerical_start");
  const auto checked = validate(model, result.values, options.feasibility_tolerance, options.integrality_tolerance);
  if (!checked.valid || !checked.objective || *checked.objective != static_cast<double>(result.objective))
    throw ModelError("Complete native primal start failed independent original-model numerical check: " + checked.message);
  start_checkpoint(budget, "after_numerical_start");
  return result;
}
void publish_start(PreparedStart&& start, SolveResult& result, const SolveBudget& budget) {
  start_checkpoint(budget, "before_start_publication");
  result.values = std::move(start.values); result.active_variables = std::move(start.active);
  result.objective = static_cast<double>(start.objective);
  result.solution_validated = result.start_submitted = true;
  // Callers install their scalar incumbent before the next hook/checkpoint.
}
void start_cutoff(NativeSpace& root, int cost, const SolveBudget& budget) {
  start_checkpoint(budget, "before_start_cutoff");
  rel(root, root.cost, root.minimize ? IRT_LE : IRT_GR, cost);
  start_checkpoint(budget, "after_start_cutoff");
}

// All live search and LP ownership is released before this cooperative gate.
// Return true for a semantic failure requiring a conservative bound fallback.
bool finish_start(SolveResult& result, const SolveBudget& budget) {
  bool failure = false;
  try { start_event("after_start_release"); }
  catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit; result.message = "Native start cleanup allocation failed";
  } catch (const MemoryExhausted& error) {
    result.termination = Termination::MemoryLimit; result.message = error.what();
  } catch (const std::exception& error) {
    result.termination = Termination::BackendError; result.message = error.what(); failure = true;
  }
  if (budget.cancelled() || budget.time_limit_reached()) {
    result.termination = budget.cancelled() ? Termination::Cancelled : Termination::TimeLimit;
    result.message = "Native start solve time/cancellation budget stopped before return";
  }
  return failure;
}

class BudgetStop final : public Search::Stop {
  SolveBudget budget_;
  std::uint64_t observed_ = 0;
public:
  explicit BudgetStop(SolveBudget budget) : budget_(std::move(budget)) {}
  void observe(const Search::Statistics& statistics) {
    const auto nodes = static_cast<std::uint64_t>(statistics.node);
    if (nodes > observed_) budget_.add_nodes(nodes - observed_);
    observed_ = std::max(observed_, nodes);
  }
  bool stop(const Search::Statistics& statistics, const Search::Options&) override {
    observe(statistics); return budget_.expired();
  }
};
#endif
} // namespace

BackendCapabilities native_capabilities() {
  BackendCapabilities result;
  result.name = "Gecode native";
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  result.available = result.mixed_integer_linear = result.exact_solving = true;
  result.version = GECODE_VERSION;
#else
  result.limitations.push_back("Built without the native Gecode bridge");
#endif
  result.limitations.push_back("Finite Integer/Binary/SemiInteger linear subset; integral bounds, coefficients and objective offset");
  result.limitations.push_back("Conservative native integer activity limits and exact double objective range; no continuous variables");
  result.limitations.push_back("Native reified indicators and all-different/element/table/cumulative/circuit/regular globals; scoped FlatZinc compilation is available separately");
  result.limitations.push_back("One deterministic worker; complete exact starts with live indicator gate completion; no partial starts, certificates or interrupted global bounds");
  result.limitations.push_back("Time/cancellation is cooperative; native propagation and recomputation cannot be interrupted");
  return result;
}

namespace {
SolveResult solve_native_impl(const ModelSnapshot& model, const SolveOptions& options,
                              const NativeLpOptions* lp_options,
                              NativeLpStatistics* lp_statistics,
                              const SolveBudget* inherited_budget = nullptr,
                              [[maybe_unused]] bool root_knapsack = true) {
  const auto started = std::chrono::steady_clock::now();
  SolveResult result;
  result.model_id = model.model_id; result.revision = model.revision;
  result.backend = lp_options ? "Gecode native + checked LP" : "Gecode native";
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  RelaxationState relaxation;
#else
  (void) lp_statistics;
#endif
  if (lp_statistics && lp_options && lp_options->root_cover_cuts) {
    lp_statistics->root_cover.requested=true;
    lp_statistics->root_cover.completion=NativeRootCoverCompletion::NotStarted;
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    relaxation.root=lp_statistics->root_cover;
#endif
  }
  std::optional<SolveBudget> shared_budget;
  try {
    if (inherited_budget) shared_budget.emplace(*inherited_budget);
    else shared_budget.emplace(options);
    [[maybe_unused]] const auto& budget = *shared_budget;
    result.guarantee=options.guarantee;
    if (lp_options) lp_options->validate();
    validate_structure(model);
    if (options.backend != Backend::Auto && options.backend != Backend::Native)
      throw Unsupported("Native solve requires backend Auto or Native");
    if (options.guarantee == Guarantee::Certified)
      throw Unsupported("Native bridge does not produce independently checkable proof certificates");
    if (options.threads != 1 || options.random_seed != 0)
      throw Unsupported("Native bridge currently uses one deterministic worker and seed 0");
#ifndef GECODE_OPTIMIZE_WITH_NATIVE
    throw Unsupported("Built without the native Gecode bridge");
#else
    result.backend_version = GECODE_VERSION;
#ifndef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    if (lp_options)
      throw Unsupported("Native checked LP requires HiGHS and compiler support for checked 128-bit integer arithmetic");
#endif
    checkpoint(budget);
    auto compiled = compile(model, budget);
    std::optional<int> start_cost;
    if (!options.primal_start.empty()) {
      auto start = prepare_start(model, compiled, options, budget);
      start_cost = start.cost;
      publish_start(std::move(start), result, budget);
      start_checkpoint(budget, "after_start_publication");
    }
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    if (lp_options) {
      prepare_relaxation(compiled,*lp_options,budget,relaxation);
      result.backend_version += std::string("; HiGHS ") + Highs().version();
      checkpoint(budget);
    }
#endif
    result.guarantee = options.guarantee;
    std::vector<bool> active;
    for (const auto& variable : model.variables) active.push_back(variable.active);
    BudgetStop stop(budget);
    Search::Options search_options;
    search_options.threads = 1; search_options.clone = true; search_options.stop = &stop;
    if (start_cost) start_checkpoint(budget, "start_root_alloc");
    auto root = std::make_unique<NativeSpace>(compiled, budget, nullptr,
                                             root_knapsack && !lp_options);
    if (start_cost) start_cutoff(*root, *start_cost, budget);
    checkpoint(budget);
    BAB<NativeSpace> search(root.get(), search_options);
    root.reset();
    if (start_cost) start_checkpoint(budget, "start_search_ready");
    for (;;) {
      checkpoint(budget);
      if (start_cost) start_checkpoint(budget, "before_start_search_next");
      std::unique_ptr<NativeSpace> candidate(search.next());
      stop.observe(search.statistics());
      checkpoint(budget);
      if (!candidate) {
        if (search.stopped()) throw std::runtime_error("Native search stopped without a shared-budget reason");
        result.termination = result.has_solution() ? Termination::Optimal : Termination::Infeasible;
        if (result.has_solution()) {
          result.best_bound = result.objective; result.update_gaps(model.objective.sense);
        }
        result.message = "Native finite integer search exhausted; original integer semantics preserved";
        break;
      }
      std::vector<double> values(model.variables.size(), std::numeric_limits<double>::quiet_NaN());
      for (std::size_t i = 0; i < compiled.variables.size(); ++i)
        values[compiled.variables[i].slot] = candidate->variables[static_cast<int>(i)].val();
      if (start_cost) start_checkpoint(budget, "before_start_candidate_validation");
      const auto exact_objective = check_exact(model, values);
      if (exact_objective != compiled.offset + candidate->cost.val())
        throw std::runtime_error("Native cost does not match exact original objective");
      const auto checked = validate(model, values, options.feasibility_tolerance, options.integrality_tolerance);
      if (!checked.valid || !checked.objective || *checked.objective != static_cast<double>(exact_objective))
        throw std::runtime_error("Native witness failed independent original-model numerical check: " + checked.message);
      if (result.objective && (compiled.minimize ? exact_objective >= *result.objective : exact_objective <= *result.objective))
        throw std::runtime_error("Native search returned a non-improving incumbent");
      auto mask = active; // Allocate before replacing an earlier validated start.
      if (start_cost) start_checkpoint(budget, "before_start_candidate_publication");
      checkpoint(budget);
      result.values = std::move(values); result.active_variables = std::move(mask);
      result.objective = static_cast<double>(exact_objective); result.solution_validated = true;
    }
#endif
  } catch (const Unsupported& e) {
    result.termination = Termination::Unsupported; result.message = e.what();
  } catch (const Interrupted& e) {
    result.termination = e.reason; result.message = "Native shared solve budget exhausted";
  } catch (const ModelError& e) {
    result.termination = Termination::InvalidModel; result.message = e.what();
  } catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit; result.message = "Native bridge allocation failed";
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  } catch (const MemoryExhausted& e) {
    result.termination = Termination::MemoryLimit; result.message = e.what();
#endif
  } catch (const std::exception& e) {
    result.termination = Termination::BackendError; result.message = e.what();
  }
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  if (lp_statistics) *lp_statistics=relaxation_statistics(relaxation);
#endif
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  if (result.start_submitted && shared_budget) {
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    relaxation = RelaxationState{};
#endif
    (void) finish_start(result, *shared_budget);
    if (result.termination != Termination::Optimal) {
      result.best_bound.reset(); result.absolute_gap.reset(); result.relative_gap.reset();
    }
  }
#endif
  result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
  return result;
}
} // namespace

SolveResult solve_native(const ModelSnapshot& model, const SolveOptions& options) {
  return solve_native_impl(model, options, nullptr, nullptr);
}

void NativeRootCoverSettings::validate() const {
  if (!denominator || denominator>1048576 || (denominator&(denominator-1)))
    throw ModelError("Native root cover denominator must be a power of two in [1,1048576]");
}

void NativeLpSettings::validate() const {
  if (!bound_change_interval ||
      (frequency != NativeLpFrequency::Root && frequency != NativeLpFrequency::AfterBoundChanges))
    throw ModelError("Invalid native LP frequency or bound-change interval");
  if (root_cover_cuts) root_cover_cuts->validate();
}

void NativeLpOptions::validate() const {
  solve.validate(); NativeLpSettings::validate();
}

BackendCapabilities native_lp_capabilities() {
  auto result = native_capabilities();
  result.name = "Gecode native + checked LP";
#ifndef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  result.available = result.mixed_integer_linear = result.exact_solving = false;
  result.limitations.push_back("Requires native Gecode, HiGHS and checked 128-bit compiler arithmetic");
#else
  result.version += std::string("; HiGHS ") + Highs().version();
#endif
  result.limitations.push_back("Explicit LP API or conservative automatic native selection; sparse ordinary linear rows with coefficients and bounds of magnitude <= 1e9");
  result.limitations.push_back("Global constraints and indicators are enforced natively and omitted from the LP relaxation");
  result.limitations.push_back("LP attempt limits are cooperative (0.2 seconds / 10000 simplex iterations); no interrupted frontier bound");
  result.limitations.push_back("Root covers are verified only against original ordinary rows and original LP boxes; explicit LP defaults omit them, automatic native selection may enable them");
  return result;
}

NativeLpResult solve_native_lp(const ModelSnapshot& model, const NativeLpOptions& options) {
  NativeLpResult result;
  result.result = solve_native_impl(model, options.solve, &options, &result.relaxation);
  return result;
}

NativeLpResult solve_native_lp(const Model& model, const NativeLpOptions& options) {
  const auto started = std::chrono::steady_clock::now();
  NativeLpResult result;
  result.result.model_id = model.id(); result.result.revision = model.revision();
  result.result.backend = "Gecode native + checked LP";
  result.result.guarantee=options.solve.guarantee;
  if(options.root_cover_cuts) {
    result.relaxation.root_cover.requested=true;
    result.relaxation.root_cover.completion=NativeRootCoverCompletion::NotStarted;
  }
  try {
    options.validate();
    auto snapshot = model.snapshot();
    auto remaining = options;
    if (std::isfinite(remaining.solve.time_limit_seconds))
      remaining.solve.time_limit_seconds = std::max(0.0, remaining.solve.time_limit_seconds -
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count());
    result = solve_native_lp(snapshot, remaining);
  } catch (const ModelError& error) {
    result.result.termination = Termination::InvalidModel; result.result.message = error.what();
  } catch (const std::bad_alloc&) {
    result.result.termination = Termination::MemoryLimit;
    result.result.message = "Native LP model snapshot allocation failed";
  }
  result.result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
  return result;
}

SolveResult solve_native(const Model& model, const SolveOptions& options) {
  const auto started = std::chrono::steady_clock::now();
  SolveResult result;
  result.model_id = model.id(); result.revision = model.revision(); result.backend = "Gecode native";
  try {
    options.validate();
    auto snapshot = model.snapshot();
    auto remaining = options;
    if (std::isfinite(remaining.time_limit_seconds))
      remaining.time_limit_seconds = std::max(0.0, remaining.time_limit_seconds -
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count());
    result = solve_native(snapshot, remaining);
  } catch (const ModelError& e) {
    result.termination = Termination::InvalidModel; result.message = e.what();
  } catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit; result.message = "Native model snapshot allocation failed";
  }
  result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
  return result;
}

void NativeBranchingSettings::validate() const {
  if (policy != NativeBranchingPolicy::BinaryReliability)
    throw ModelError("Unknown native branching policy");
  if (!reliability_samples) throw ModelError("Native branching reliability_samples must be positive");
}

void NativeSearchOptions::validate() const {
  solve.validate();
  if (order != NativeSearchOrder::DepthFirst && order != NativeSearchOrder::BestBound)
    throw ModelError("Unknown native frontier search order");
  if (relaxation) relaxation->validate();
  if (branching) branching->validate();
}

namespace {
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
void frontier_event(const char* event) {
#ifdef GECODE_NATIVE_SEARCH_TEST_HOOKS
  native_search_test_event(event);
#else
  (void)event;
#endif
}
// An admitted node may finish at its node quota. Only time/cancellation can
// interrupt that work; the complete budget is checked before the next admission.
void frontier_checkpoint(const SolveBudget& budget) {
  if (budget.cancelled()) throw Interrupted{Termination::Cancelled};
  if (budget.time_limit_reached()) throw Interrupted{Termination::TimeLimit};
}
struct FrontierNode {
  std::unique_ptr<NativeSpace> space;
  int bound;
  std::uint64_t serial;
  bool contains_knapsack_witness = false;
};
struct FrontierCompare {
  bool operator()(const FrontierNode& a, const FrontierNode& b) const noexcept {
    if (a.bound != b.bound) return a.bound > b.bound;
    if (a.contains_knapsack_witness != b.contains_knapsack_witness)
      return !a.contains_knapsack_witness;
    return a.serial > b.serial;
  }
};

void branching_event(const char* event, std::size_t slot, double down=0, double up=0) {
#ifdef GECODE_NATIVE_BRANCHING_TEST_HOOKS
  native_branching_test_event(event,slot,down,up);
#else
  (void)event; (void)slot; (void)down; (void)up;
#endif
}
void count_branching(std::uint64_t& value, std::uint64_t amount=1) {
  if (amount > std::numeric_limits<std::uint64_t>::max()-value)
    throw Interrupted{Termination::MemoryLimit};
  value+=amount;
}
// Ranking state belongs to the coordinator, never a Space/Choice/certificate.
class BinaryReliability {
  struct History {
    int column;
    std::uint64_t pairs=0;
    double down=0,up=0;
  };
  struct Candidate {
    int column;
    std::size_t slot;
    std::optional<std::size_t> history;
  };
  struct Observation { bool failed=false; double gain=0; };
  const NativeSearchOptions& options;
  const ModelSnapshot& source;
  const Compiled& compiled;
  SolveBudget& budget;
  NativeBranchingStatistics& stats;
  std::vector<History> history;

  bool charge(std::size_t amount=1) {
    frontier_checkpoint(budget);
    if (amount>options.branching->max_branching_work-stats.work) return false;
    stats.work+=amount; return true;
  }
  static void split(NativeSpace& space,int column,unsigned int direction) {
    // Both arms partition the actual domain; no relaxation hull is posted.
    const auto variable=space.variables[column];
    if (variable.min()!=0 || variable.max()!=1 || variable.size()!=2 || direction>1)
      throw std::runtime_error("Invalid native binary branching split");
    rel(space,variable,direction ? IRT_GQ:IRT_LQ,direction ? 1:0);
  }
  Observation probe(const NativeSpace& parent,const Candidate& candidate,
      unsigned int direction,int baseline,NativeFrontierStatistics& frontier,
      std::size_t resident) {
    checkpoint(budget);
    frontier.peak_open_nodes=std::max(frontier.peak_open_nodes,resident+1);
    branching_event("probe_clone",candidate.slot);
    frontier_checkpoint(budget);
    std::unique_ptr<NativeSpace> child(static_cast<NativeSpace*>(parent.clone()));
    branching_event("probe_cloned",candidate.slot);
    frontier_checkpoint(budget);
    split(*child,candidate.column,direction);
    branching_event("probe_posted",candidate.slot);
    checkpoint(budget);
    if (budget.nodes()==std::numeric_limits<std::uint64_t>::max())
      throw Interrupted{Termination::MemoryLimit};
    count_branching(stats.probe_status_calls); budget.add_nodes();
    StatusStatistics observed;
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    const auto before=compiled.relaxation ? compiled.relaxation->statistics():LP::Stats{};
    const auto observe_lp=[&] {
      const auto after=compiled.relaxation ? compiled.relaxation->statistics():LP::Stats{};
      count_branching(stats.probe_lp_calls,after.lp_calls-before.lp_calls);
      stats.probe_lp_seconds+=(after.lp_ms-before.lp_ms)/1000.0;
    };
#endif
    SpaceStatus status;
    try {
      branching_event("probe_before_status",candidate.slot);
      frontier_checkpoint(budget);
      status=child->status(observed);
    } catch (...) {
      count_branching(stats.probe_propagations,observed.propagate);
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
      observe_lp();
#endif
      throw;
    }
    count_branching(stats.probe_propagations,observed.propagate);
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    observe_lp();
#endif
    branching_event("probe_after_status",candidate.slot);
    frontier_checkpoint(budget);
    Observation result;
    if (status==SS_FAILED) result.failed=true;
    else {
      if (status!=SS_BRANCH && status!=SS_SOLVED)
        throw std::runtime_error("Unexpected native branching probe status");
      const std::int64_t bound=compiled.minimize ? child->cost.min():-child->cost.max();
      result.gain=static_cast<double>(std::max<std::int64_t>(0,bound-baseline));
    }
    child.reset();
    branching_event("probe_destroyed",candidate.slot);
    frontier_checkpoint(budget);
    return result;
  }
public:
  BinaryReliability(const NativeSearchOptions& o,const ModelSnapshot& m,
      const Compiled& c,SolveBudget& b,NativeBranchingStatistics& s)
      :options(o),source(m),compiled(c),budget(b),stats(s) {}

  static void commit(NativeSpace& child,int column,unsigned int direction) {
    split(child,column,direction);
  }

  std::optional<int> select(const NativeSpace& parent,int baseline,
      NativeFrontierStatistics& frontier,std::size_t resident) {
    count_branching(stats.decisions);
    const auto& settings=*options.branching;
    const auto fallback=[&]() -> std::optional<int> {
      branching_event("branch_fallback",std::numeric_limits<std::size_t>::max());
      frontier_checkpoint(budget); count_branching(stats.fallback_decisions); return {};
    };
    if (!settings.max_candidates_per_decision || !settings.max_history_entries ||
        !settings.max_nonimproving_pairs || !settings.max_branching_work)
      return fallback();
    branching_event("candidate_scan",std::numeric_limits<std::size_t>::max());
    std::vector<Candidate> candidates;
    const auto capacity=std::min(settings.max_candidates_per_decision,compiled.variables.size());
    if (!charge(capacity)) return fallback();
    candidates.reserve(capacity);
    for (std::size_t column=0;column<compiled.variables.size() && candidates.size()<capacity;++column) {
      if (!charge()) break;
      const auto slot=compiled.variables[column].slot;
      const auto variable=parent.variables[static_cast<int>(column)];
      if (source.variables[slot].type!=VariableType::Binary || source.variables[slot].indicator_origin || variable.min()!=0 ||
          variable.max()!=1 || variable.size()!=2) continue;
      bool exhausted=false;
      Candidate candidate{static_cast<int>(column),slot,{}};
      for (std::size_t i=0;i<history.size();++i) {
        if (!charge()) {exhausted=true;break;}
        if (history[i].column==candidate.column) {candidate.history=i;break;}
      }
      if (exhausted) break;
      candidates.push_back(candidate);
    }
    std::optional<int> best;
    std::size_t best_slot=std::numeric_limits<std::size_t>::max(),nonimproving=0;
    unsigned int best_failed=0;
    double best_min=0,best_max=0;
    std::uint64_t decision_calls=0;
    for (const auto& candidate:candidates) {
      if (!charge()) break;
      double down=0,up=0; unsigned int failed=0;
      bool completed=false;
      if (candidate.history && history[*candidate.history].pairs>=settings.reliability_samples) {
        const auto& old=history[*candidate.history];down=old.down;up=old.up;
        count_branching(stats.reliable_candidates);
      } else {
        // Preserve capacity for a complete pair and two ordinary child admissions.
        const auto nodes=budget.nodes();
        const bool room=!options.solve.node_limit || (nodes<=*options.solve.node_limit && *options.solve.node_limit-nodes>=4);
        if (!room || resident>=options.max_open_nodes ||
            settings.max_probe_status_calls-stats.probe_status_calls<2 ||
            settings.max_probe_status_calls_per_decision-decision_calls<2 ||
            (!candidate.history && history.size()>=settings.max_history_entries) || !charge(16)) continue;
        branching_event("pair_begin",candidate.slot);
        frontier_checkpoint(budget);
        const auto first=probe(parent,candidate,0,baseline,frontier,resident);
        const auto second=probe(parent,candidate,1,baseline,frontier,resident);
        decision_calls+=2;
        count_branching(stats.completed_pairs);
        down=first.gain;up=second.gain;failed=unsigned(first.failed)+unsigned(second.failed);
        branching_event("pair_before_publication",candidate.slot,down,up);
        frontier_checkpoint(budget);
        if (!failed) {
          const auto previous=candidate.history ? history[*candidate.history]:History{candidate.column};
          if (previous.pairs==std::numeric_limits<std::uint64_t>::max())
            throw Interrupted{Termination::MemoryLimit};
          auto next=previous;++next.pairs;
          next.down+=(down-next.down)/static_cast<double>(next.pairs);
          next.up+=(up-next.up)/static_cast<double>(next.pairs);
          if (!std::isfinite(next.down) || !std::isfinite(next.up))
            throw std::runtime_error("Nonfinite native branching history");
          if (candidate.history) history[*candidate.history]=next;
          else history.push_back(next);
          stats.history_entries=history.size();
          count_branching(stats.published_pairs);count_branching(stats.finite_samples,2);
          count_branching(stats.zero_gain_samples,unsigned(down==0)+unsigned(up==0));
        }
        count_branching(stats.failed_directions,failed);
        branching_event("pair_published",candidate.slot,down,up);
        frontier_checkpoint(budget); completed=true;
      }
#ifdef GECODE_NATIVE_BRANCHING_TEST_HOOKS
      native_branching_test_event("candidate_score",candidate.slot,down,up);
      frontier_checkpoint(budget);
#endif
      // Scores never carry evidence. Invalid experimental scores lose priority.
      if (!std::isfinite(down) || !std::isfinite(up) || down<0 || up<0) down=up=0;
      const double low=std::min(down,up),high=std::max(down,up);
      const bool better=(failed || high>0) && (!best || failed>best_failed ||
        (failed==best_failed && (low>best_min || (low==best_min &&
        (high>best_max || (high==best_max && candidate.slot<best_slot))))));
      if (better) {best=candidate.column;best_slot=candidate.slot;best_failed=failed;best_min=low;best_max=high;nonimproving=0;}
      else if (completed && ++nonimproving>=settings.max_nonimproving_pairs) break;
    }
    if (!best) return fallback();
    branching_event("branch_selected",best_slot,best_min,best_max);
    frontier_checkpoint(budget);count_branching(stats.manual_splits);return best;
  }
};
#endif

#ifdef GECODE_OPTIMIZE_WITH_NATIVE
// Source-size accounting counts logical fields, not bytes or propagation work.
// The preflight never allocates a flattened global argument/payload copy.
void neighborhood_source(const ModelSnapshot& model,NeighborhoodBudget& local) {
  local.source(3); // Objective marker, offset, sense.
  for(const auto& term:model.objective.terms) {(void)term;local.source();}
  for(const auto& row:model.rows) {
    local.work(); if(!row.active) continue;
    local.source(3); // Row marker and bounds.
    for(const auto& term:row.terms) {(void)term;local.source();}
  }
  for(const auto& indicator:model.indicators) {
    local.work();if(!indicator.active) continue;
    local.source(8); // Identity, activation, bounds, gate, and M fields.
    for(const auto& term:indicator.terms) {(void)term;local.source();}
    for(const auto& row:indicator.generated_rows) {(void)row;local.source();}
    for(const auto& domain:indicator.domains) {(void)domain;local.source(3);}
  }
  for(const auto& global:model.globals) {
    local.work();if(!global.active) continue;local.source();
    std::visit([&](const auto& data) {
      using T=std::decay_t<decltype(data)>;
      if constexpr(std::is_same_v<T,AllDifferentData>) {
        for(auto v:data.variables) {(void)v;local.source();}
      } else if constexpr(std::is_same_v<T,ElementData>) {
        local.source(3);for(auto v:data.elements) {(void)v;local.source();}
      } else if constexpr(std::is_same_v<T,TableData>) {
        for(auto v:data.variables) {(void)v;local.source();}
        for(const auto& row:data.tuples) {local.source();for(auto v:row) {(void)v;local.source();}}
      } else if constexpr(std::is_same_v<T,CumulativeData>) {
        local.source();for(auto v:data.starts) {(void)v;local.source(3);}
      } else if constexpr(std::is_same_v<T,CircuitData>) {
        local.source();for(auto v:data.successors) {(void)v;local.source();}
      } else {
        static_assert(std::is_same_v<T,RegularData>,"Count every original global payload");
        local.source(2);
        for(auto v:data.variables) {(void)v;local.source();}
        for(const auto& edge:data.transitions) {(void)edge;local.source(3);}
        for(auto state:data.final_states) {(void)state;local.source();}
      }
    },global.payload);
  }
}

struct NeighborhoodWitness {
  std::vector<double> values;
  std::vector<bool> active;
  std::int64_t objective=0;
  int raw_cost=0;
  double started_seconds=0;
};
struct NeighborhoodPublicationTiming {
  const SolveBudget& budget;
  NativeNeighborhoodStatistics& statistics;
  double started;
  ~NeighborhoodPublicationTiming() {statistics.elapsed_seconds=budget.elapsed_seconds()-started;}
};
struct NeighborhoodFrame {
  std::unique_ptr<NativeSpace> space;
  std::unique_ptr<const Choice> choice;
  unsigned int next=0;
};

std::optional<NeighborhoodWitness> binary_neighborhood(const ModelSnapshot& source,
    const Compiled& compiled,const NativeSearchOptions& search,
    const NativeNeighborhoodSettings& settings,NativeNeighborhoodStatistics& statistics,
    SolveBudget& shared,const std::vector<double>& reference,int reference_normalized,
    std::size_t main_resident) {
  NeighborhoodBudget local(settings,statistics,shared);
  std::vector<NeighborhoodFrame> stack;
  std::unique_ptr<NativeSpace> current;
  std::optional<NeighborhoodWitness> candidate;
  const auto release=[&] {current.reset();stack.clear();};
  try {
    local.check("before_neighborhood");
    if(!settings.max_status_calls) throw NeighborhoodStopped{NeighborhoodCompletion::StatusLimit};
    const auto room=[&] {
      local.check();checkpoint(shared);
      if(shared.nodes()==std::numeric_limits<std::uint64_t>::max())
        throw Interrupted{Termination::MemoryLimit};
      if(search.solve.node_limit) {
        const auto limit=*search.solve.node_limit;
        if(shared.nodes()>limit || limit-shared.nodes()<3)
          throw NeighborhoodStopped{NeighborhoodCompletion::SharedNodeReserve};
      }
    };
    const auto reserve=[&] {
      local.check();
      const auto resident=stack.size()+(current?1U:0U);
      if(resident>=settings.max_local_spaces || main_resident>=search.max_open_nodes ||
         resident>=search.max_open_nodes-main_resident)
        throw NeighborhoodStopped{NeighborhoodCompletion::LocalStorageLimit};
      statistics.peak_local_spaces=std::max(statistics.peak_local_spaces,resident+1);
      statistics.peak_total_spaces=std::max(statistics.peak_total_spaces,main_resident+resident+1);
    };
    room();
    std::vector<int> columns;
    for(const auto& variable:source.variables) {
      local.work();if(!variable.active) continue;local.source();
      if(variable.type!=VariableType::Binary || variable.indicator_origin ||
         variable.lower!=0 || variable.upper!=1) continue;
      if(statistics.eligible_variables==std::numeric_limits<std::size_t>::max())
        throw NeighborhoodStopped{NeighborhoodCompletion::FormulationLimit};
      ++statistics.eligible_variables;
      if(statistics.eligible_variables>settings.max_distance_variables ||
         statistics.eligible_variables>static_cast<std::size_t>(Int::Limits::max))
        throw NeighborhoodStopped{NeighborhoodCompletion::FormulationLimit};
      columns.push_back(compiled.columns[variable.variable.id]);
    }
    if(columns.empty()) throw NeighborhoodStopped{NeighborhoodCompletion::NoEligibleBinary};
    if(settings.radius>=columns.size()) throw NeighborhoodStopped{NeighborhoodCompletion::NonrestrictingRadius};
    neighborhood_source(source,local);
    // radius < size <= native limit; both RHS subtraction and total absolute
    // activity are therefore within the conservative ordinary linear envelope.
    IntArgs coefficients(static_cast<int>(columns.size()));
    int rhs=static_cast<int>(settings.radius);
    for(std::size_t i=0;i<columns.size();++i) {
      local.work();const auto slot=compiled.variables[columns[i]].slot;
      const double value=reference.at(slot);
      if(value!=0 && value!=1) throw std::runtime_error("Neighborhood reference binary is not exact");
      coefficients[static_cast<int>(i)]=value==1?-1:1;
      if(value==1) --rhs;
    }
    reserve();room();
    statistics.attempts=1;
    local.check("neighborhood_root_alloc");
    // The local budget parameter also suppresses LP attachment, without editing
    // compiled data or the main search's backend/evidence/scheduling state.
    current=std::make_unique<NativeSpace>(compiled,shared,&local);
    IntVarArgs variables(static_cast<int>(columns.size()));
    for(std::size_t i=0;i<columns.size();++i) {
      local.work();variables[static_cast<int>(i)]=current->variables[columns[i]];
    }
    local.check("before_neighborhood_distance");
    linear(*current,coefficients,variables,IRT_LQ,rhs);
    local.check("before_neighborhood_cutoff");
    const int reference_raw=compiled.minimize?reference_normalized:-reference_normalized;
    rel(*current,current->cost,compiled.minimize?IRT_LE:IRT_GR,reference_raw);
    local.check("after_neighborhood_cutoff");
    for(;;) {
      local.work();room();
      if(statistics.status_attempts>=settings.max_status_calls)
        throw NeighborhoodStopped{NeighborhoodCompletion::StatusLimit};
      shared.add_nodes();++statistics.status_attempts;
      local.check("before_neighborhood_status");
      const auto status=current->status();
      ++statistics.completed_status_calls;
      local.check("after_neighborhood_status");
      if(status==SS_SOLVED) {
        ++statistics.feasible_candidates;
        NeighborhoodWitness staged;
        staged.started_seconds=local.start_seconds();
        staged.values.assign(source.variables.size(),std::numeric_limits<double>::quiet_NaN());
        staged.active.reserve(source.variables.size());
        for(const auto& variable:source.variables) {local.work();staged.active.push_back(variable.active);}
        for(std::size_t i=0;i<compiled.variables.size();++i) {
          local.work();staged.values[compiled.variables[i].slot]=current->variables[static_cast<int>(i)].val();
        }
        neighborhood_event("before_neighborhood_validation",0,0,0,&staged.values);
        local.check();
        staged.objective=check_exact(source,staged.values,nullptr,&local);
        staged.raw_cost=current->cost.val();
        if(staged.objective!=compiled.offset+staged.raw_cost)
          throw std::runtime_error("Neighborhood cost disagrees with exact original objective");
        const auto checked=validate(source,staged.values,search.solve.feasibility_tolerance,
                                    search.solve.integrality_tolerance);
        local.check("after_neighborhood_validation");
        if(!checked.valid || !checked.objective || *checked.objective!=static_cast<double>(staged.objective))
          throw std::runtime_error("Neighborhood original-model numerical validation failed: "+checked.message);
        std::size_t distance=0;
        for(int column:columns) {
          local.work();const auto slot=compiled.variables[column].slot;
          if(staged.values[slot]!=reference[slot]) ++distance;
        }
        const int normalized=compiled.minimize?staged.raw_cost:-staged.raw_cost;
        if(distance>settings.radius || normalized>=reference_normalized)
          throw std::runtime_error("Neighborhood witness violates its distance or strict improvement restriction");
        candidate=std::move(staged);statistics.completion=NeighborhoodCompletion::Improved;break;
      }
      if(status==SS_FAILED) {++statistics.failed_nodes;current.reset();}
      else if(status==SS_BRANCH) {
        local.work();local.check("neighborhood_choice");
        std::unique_ptr<const Choice> choice(current->choice());
        if(!choice || choice->alternatives()!=2)
          throw std::runtime_error("Neighborhood requires the ordinary two-alternative brancher");
        stack.push_back({std::move(current),std::move(choice),0});
      } else throw std::runtime_error("Unexpected neighborhood Space status");
      while(!stack.empty() && stack.back().next==stack.back().choice->alternatives()) {
        local.work();stack.pop_back();
      }
      if(stack.empty()) {statistics.completion=NeighborhoodCompletion::NoImprovement;break;}
      local.work();room();reserve();
      auto& frame=stack.back();
      local.check("neighborhood_child_clone");
      current.reset(static_cast<NativeSpace*>(frame.space->clone()));
      current->commit(*frame.choice,frame.next++);
      local.check("neighborhood_child_committed");
    }
  } catch(const NeighborhoodStopped& stop) {
    statistics.completion=stop.reason;candidate.reset();
  } catch(const Interrupted& stop) {
    statistics.completion=NeighborhoodCompletion::GlobalStop;statistics.stop_reason=stop.reason;
    release();local.finish();throw;
  } catch(...) {
    statistics.completion=NeighborhoodCompletion::Error;
    release();local.finish();throw;
  }
  release();
  try {local.check("after_neighborhood_release");}
  catch(const NeighborhoodStopped& stop) {statistics.completion=stop.reason;candidate.reset();}
  catch(const Interrupted& stop) {
    statistics.completion=NeighborhoodCompletion::GlobalStop;statistics.stop_reason=stop.reason;
    local.finish();throw;
  } catch(...) {statistics.completion=NeighborhoodCompletion::Error;local.finish();throw;}
  local.finish();return candidate;
}
#endif

NativeSearchResult native_frontier_impl(const ModelSnapshot& model,
    const NativeSearchOptions& options, SolveBudget& budget,
    const NativeNeighborhoodSettings* neighborhoods=nullptr,
    NativeNeighborhoodStatistics* neighborhood_statistics=nullptr) {
  NativeSearchResult output;
  auto& result = output.result;
  auto& statistics = output.frontier;
  result.model_id = model.model_id; result.revision = model.revision;
  result.backend = options.relaxation ? "Gecode native frontier + checked LP" : "Gecode native frontier";
  if(neighborhoods) result.backend += " + BinaryHamming";
  output.branching.requested=bool(options.branching);
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  std::vector<FrontierNode> frontier;
  std::unique_ptr<NativeSpace> parent;
  std::optional<int> active_bound, initial_bound, incumbent;
  std::int64_t offset = 0;
  bool minimize = true, fallback_bound = false;
  std::uint64_t serial = 0;
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  RelaxationState relaxation;
#endif
#else
  (void)statistics;
#endif
  if (options.relaxation && options.relaxation->root_cover_cuts) {
    output.relaxation.root_cover.requested=true;
    output.relaxation.root_cover.completion=NativeRootCoverCompletion::NotStarted;
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    relaxation.root=output.relaxation.root_cover;
#endif
  }
  try {
    options.validate();if(neighborhoods) neighborhoods->validate();result.guarantee = options.solve.guarantee;
    validate_structure(model);
    if (options.solve.backend != Backend::Auto && options.solve.backend != Backend::Native)
      throw Unsupported("Native frontier requires backend Auto or Native");
    if (options.solve.guarantee == Guarantee::Certified)
      throw Unsupported("Native frontier does not export independently checkable solve certificates");
    if (options.solve.threads != 1 || options.solve.random_seed != 0)
      throw Unsupported("Native frontier supports one worker and seed zero");
#ifndef GECODE_OPTIMIZE_WITH_NATIVE
    throw Unsupported("Built without the native Gecode bridge");
#else
    result.backend_version = GECODE_VERSION;
#ifndef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    if (options.relaxation)
      throw Unsupported("Native frontier checked LP requires HiGHS and checked 128-bit compiler arithmetic");
#endif
    checkpoint(budget);
    auto compiled = compile(model, budget);
    minimize = compiled.minimize; offset = compiled.offset;
    initial_bound = minimize ? compiled.cost_lower : -compiled.cost_upper;
    active_bound = initial_bound; // The entire original region is represented.
    if (!options.solve.primal_start.empty()) {
      auto start = prepare_start(model, compiled, options.solve, budget);
      const int normalized = minimize ? start.cost : -start.cost;
      publish_start(std::move(start), result, budget);
      incumbent = normalized;
      start_checkpoint(budget, "after_start_publication");
    }
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    if (options.relaxation) {
      prepare_relaxation(compiled,*options.relaxation,budget,relaxation);
      result.backend_version += std::string("; HiGHS ") + Highs().version();
      checkpoint(budget);
    }
#endif
    const auto reserve_space = [&] {
      const auto resident = frontier.size() + (active_bound ? 1U : 0U);
      // The root's reservation is made separately, before construction.
      if (resident >= options.max_open_nodes) throw Interrupted{Termination::MemoryLimit};
      statistics.peak_open_nodes = std::max(statistics.peak_open_nodes, resident + 1);
    };
    const auto enqueue = [&](std::unique_ptr<NativeSpace>& space, int bound) {
      if (serial == std::numeric_limits<std::uint64_t>::max())
        throw Interrupted{Termination::MemoryLimit};
      bool contains_witness = options.order == NativeSearchOrder::BestBound &&
        bool(space->knapsack_witness);
      for (std::size_t i = 0; contains_witness && i < space->knapsack_witness->size(); ++i) {
        frontier_checkpoint(budget);
        contains_witness = space->variables[static_cast<int>(i)].in((*space->knapsack_witness)[i]);
      }
      // Compute preference once on the stable space. The comparator never
      // scans domains, and the exact objective bound remains the primary key.
      frontier.push_back({std::move(space), bound, serial++, contains_witness});
      if (options.order == NativeSearchOrder::BestBound)
        std::push_heap(frontier.begin(), frontier.end(), FrontierCompare{});
    };
    const auto evaluate = [&](NativeSpace& space, int& inherited) {
      checkpoint(budget);
      if (budget.nodes()==std::numeric_limits<std::uint64_t>::max())
        throw Interrupted{Termination::MemoryLimit};
      budget.add_nodes(); ++statistics.admitted_nodes;
      frontier_event("before_propagation");
      frontier_checkpoint(budget);
      const auto status = space.status();
      frontier_event("after_propagation");
      frontier_checkpoint(budget);
      if (status == SS_FAILED) { ++statistics.failed_nodes; return false; }
      inherited = std::max(inherited, minimize ? space.cost.min() : -space.cost.max());
      if (incumbent && inherited >= *incumbent) {
        ++statistics.bound_pruned_nodes; return false;
      }
      if (status == SS_SOLVED) {
        std::vector<double> values(model.variables.size(), std::numeric_limits<double>::quiet_NaN());
        std::vector<bool> mask;
        for (const auto& variable : model.variables) mask.push_back(variable.active);
        for (std::size_t i = 0; i < compiled.variables.size(); ++i) {
          frontier_checkpoint(budget);
          values[compiled.variables[i].slot] = space.variables[static_cast<int>(i)].val();
        }
        frontier_event("before_validation");
        const auto objective = check_exact(model, values);
        if (objective != offset + space.cost.val())
          throw std::runtime_error("Native frontier cost disagrees with the exact original objective");
        const auto checked = validate(model, values, options.solve.feasibility_tolerance,
                                      options.solve.integrality_tolerance);
        if (!checked.valid || !checked.objective || *checked.objective != static_cast<double>(objective))
          throw std::runtime_error("Native frontier original-model witness validation failed: " + checked.message);
        frontier_event("after_validation");
        frontier_checkpoint(budget);
        const int value = minimize ? space.cost.val() : -space.cost.val();
        // All allocating validation/copy work precedes this publication gate.
        result.values = std::move(values); result.active_variables = std::move(mask);
        result.objective = static_cast<double>(objective); result.solution_validated = true;
        incumbent = value; ++statistics.feasible_leaves;
        return false;
      }
      if (status != SS_BRANCH) throw std::runtime_error("Unexpected native frontier space status");
      return true;
    };
    if (!options.max_open_nodes) throw Interrupted{Termination::MemoryLimit};
    statistics.peak_open_nodes = 1;
    frontier_event("root_alloc");
    parent = std::make_unique<NativeSpace>(compiled, budget, nullptr,
      !options.relaxation, options.order == NativeSearchOrder::DepthFirst);
    if (incumbent) start_cutoff(*parent, minimize ? *incumbent : -*incumbent, budget);
    if (evaluate(*parent, *active_bound)) enqueue(parent, *active_bound);
    parent.reset(); active_bound.reset();
    BinaryReliability branching(options,model,compiled,budget,output.branching);
    while (!frontier.empty()) {
      frontier_checkpoint(budget);
      if (options.order == NativeSearchOrder::BestBound)
        std::pop_heap(frontier.begin(), frontier.end(), FrontierCompare{});
      active_bound = frontier.back().bound;
      parent = std::move(frontier.back().space); frontier.pop_back();
      if (incumbent && *active_bound >= *incumbent) {
        ++statistics.bound_pruned_nodes; parent.reset(); active_bound.reset(); continue;
      }
      if(neighborhoods && incumbent && neighborhood_statistics->completion==NeighborhoodCompletion::NotStarted) {
#ifdef GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS
        for(std::size_t i=0;i<compiled.variables.size();++i)
          neighborhood_event("neighborhood_parent_variable",compiled.variables[i].slot,
                             parent->variables[static_cast<int>(i)].min(),parent->variables[static_cast<int>(i)].max());
#endif
        auto improvement=binary_neighborhood(model,compiled,options,*neighborhoods,*neighborhood_statistics,
                                              budget,result.values,*incumbent,frontier.size()+1);
        if(improvement) {
          NeighborhoodPublicationTiming timing{budget,*neighborhood_statistics,improvement->started_seconds};
          // Local owners are already released. Complete nonthrowing publication
          // precedes the next hook/checkpoint, including scalar cost installation.
          frontier_checkpoint(budget);
          neighborhood_event("before_neighborhood_publication");
          frontier_checkpoint(budget);
          neighborhood_statistics->elapsed_seconds=budget.elapsed_seconds()-improvement->started_seconds;
          if(neighborhood_statistics->elapsed_seconds>=neighborhoods->time_limit_seconds) {
            neighborhood_statistics->completion=NeighborhoodCompletion::LocalTimeLimit;
            improvement.reset();
          } else {
            const int normalized=minimize?improvement->raw_cost:-improvement->raw_cost;
            result.values=std::move(improvement->values);result.active_variables=std::move(improvement->active);
            result.objective=static_cast<double>(improvement->objective);result.solution_validated=true;
            incumbent=normalized;++neighborhood_statistics->accepted_improvements;
            neighborhood_event("after_neighborhood_publication");
            frontier_checkpoint(budget);
          }
        }
        if(incumbent && *active_bound>=*incumbent) {
          ++statistics.bound_pruned_nodes;parent.reset();active_bound.reset();continue;
        }
      }
      // The stable parent remains represented until every alternative is
      // evaluated and safely enqueued or discharged. Child failure cannot
      // erase an unvisited sibling from the global-bound calculation.
      const auto manual=options.branching ? branching.select(*parent,*active_bound,statistics,frontier.size()+1):std::optional<int>{};
      std::unique_ptr<const Choice> choice;
      if (!manual) choice.reset(parent->choice());
      if (!manual && (!choice || !choice->alternatives())) throw std::runtime_error("Native branch has no alternatives");
      const auto alternatives=manual ? 2U:choice->alternatives();
      ++statistics.expanded_nodes;
      for (unsigned int alternative = 0; alternative < alternatives; ++alternative) {
        checkpoint(budget);
        reserve_space();
        frontier_event("child_clone");
        std::unique_ptr<NativeSpace> child(static_cast<NativeSpace*>(parent->clone()));
        if (manual) BinaryReliability::commit(*child,*manual,alternative);
        else child->commit(*choice, alternative);
        frontier_event("child_committed");
        if (incumbent)
          rel(*child, child->cost, minimize ? IRT_LE : IRT_GR, minimize ? *incumbent : -*incumbent);
        int child_bound = *active_bound;
        if (evaluate(*child, child_bound)) {
          enqueue(child, child_bound);
          frontier_event("child_enqueued");
          frontier_checkpoint(budget);
        }
      }
      parent.reset(); active_bound.reset();
      frontier_event("parent_discharged");
    }
    frontier_checkpoint(budget);
    result.termination = result.has_solution() ? Termination::Optimal : Termination::Infeasible;
    result.message = "All native frontier regions exhausted or pruned by exact incumbent bounds";
    if(neighborhood_statistics && neighborhood_statistics->completion==NeighborhoodCompletion::NotStarted)
      neighborhood_statistics->completion=incumbent?NeighborhoodCompletion::ProofCompletedBeforeAttempt:NeighborhoodCompletion::NoIncumbent;
#endif
  } catch (const Unsupported& error) {
    result.termination = Termination::Unsupported; result.message = error.what();
  } catch (const Interrupted& error) {
    result.termination = error.reason;
    result.message = error.reason == Termination::MemoryLimit ? "Native frontier open-node storage limit reached"
                                                            : "Native frontier shared solve budget stopped";
  } catch (const ModelError& error) {
    result.termination = Termination::InvalidModel; result.message = error.what();
  } catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit; result.message = "Native frontier allocation failed";
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  } catch (const MemoryExhausted& error) {
    result.termination = Termination::MemoryLimit; result.message = error.what();
#endif
  } catch (const std::exception& error) {
    result.termination = Termination::BackendError; result.message = error.what();
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
    fallback_bound = true;
#endif
  }
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  output.branching.budget_nodes=budget.nodes();
  statistics.unresolved_regions = frontier.size() + (active_bound ? 1U : 0U);
  std::optional<int> global = incumbent;
  const auto include = [&](int value) { global = global ? std::min(*global, value) : value; };
  for (const auto& node : frontier) include(node.bound);
  if (active_bound) include(*active_bound);
  // A semantic implementation failure cannot authorize propagated evidence.
  if (fallback_bound) global = initial_bound;
  if (result.termination != Termination::Infeasible && global) {
    result.best_bound = static_cast<double>(offset + (minimize ? *global : -*global));
    result.update_gaps(model.objective.sense);
  }
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  output.relaxation=relaxation_statistics(relaxation);
#endif
  // Charge release of the owning frontier to the same end-to-end deadline.
  frontier.clear(); parent.reset();
  if (result.start_submitted) {
#ifdef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
    relaxation = RelaxationState{};
#endif
    if (finish_start(result, budget)) {
      result.best_bound = initial_bound ? std::optional<double>(static_cast<double>(offset + (minimize ? *initial_bound : -*initial_bound))) : std::nullopt;
      result.update_gaps(model.objective.sense);
    }
  }
  if (budget.cancelled() || budget.time_limit_reached()) {
    result.termination = budget.cancelled() ? Termination::Cancelled : Termination::TimeLimit;
    result.message = "Native frontier time/cancellation budget stopped before return";
  }
#endif
  result.elapsed_seconds = budget.elapsed_seconds();
  if(neighborhood_statistics) {
    neighborhood_statistics->budget_nodes=budget.nodes();
    neighborhood_statistics->peak_total_spaces=std::max(neighborhood_statistics->peak_total_spaces,
                                                        statistics.peak_open_nodes);
    if(neighborhood_statistics->completion==NativeNeighborhoodCompletion::Error)
      neighborhood_statistics->stop_reason=result.termination;
    else if(neighborhood_statistics->completion==NativeNeighborhoodCompletion::Improved &&
            !neighborhood_statistics->accepted_improvements) {
      neighborhood_statistics->completion=result.termination==Termination::BackendError
        ?NativeNeighborhoodCompletion::Error:NativeNeighborhoodCompletion::GlobalStop;
      neighborhood_statistics->stop_reason=result.termination;
    }
    else if(neighborhood_statistics->completion==NativeNeighborhoodCompletion::NotStarted &&
            result.termination!=Termination::Optimal && result.termination!=Termination::Infeasible) {
      neighborhood_statistics->completion=NativeNeighborhoodCompletion::GlobalStop;
      neighborhood_statistics->stop_reason=result.termination;
    }
  }
  return output;
}
NativeSearchResult frontier_failure(ModelId id, Revision revision, const NativeSearchOptions& options,
                                    Termination termination, const std::string& message) {
  NativeSearchResult output; auto& result = output.result;
  result.model_id = id; result.revision = revision; result.guarantee = options.solve.guarantee;
  result.backend = options.relaxation ? "Gecode native frontier + checked LP" : "Gecode native frontier";
  output.branching.requested=bool(options.branching);
  if(options.relaxation && options.relaxation->root_cover_cuts) {
    output.relaxation.root_cover.requested=true;
    output.relaxation.root_cover.completion=NativeRootCoverCompletion::NotStarted;
  }
  result.termination = termination; result.message = message; return output;
}
} // namespace

NativeSearchResult solve_native_search(const ModelSnapshot& model, const NativeSearchOptions& options) {
  try {
    SolveBudget budget(options.solve);
    return native_frontier_impl(model, options, budget);
  } catch (const ModelError& error) {
    return frontier_failure(model.model_id, model.revision, options, Termination::InvalidModel, error.what());
  } catch (const std::bad_alloc&) {
    return frontier_failure(model.model_id, model.revision, options, Termination::MemoryLimit, "Native frontier allocation failed");
  }
}
NativeSearchResult solve_native_search(const Model& model, const NativeSearchOptions& options) {
  try {
    SolveBudget budget(options.solve);
    return native_frontier_impl(model.snapshot(), options, budget);
  } catch (const ModelError& error) {
    return frontier_failure(model.id(), model.revision(), options, Termination::InvalidModel, error.what());
  } catch (const std::bad_alloc&) {
    return frontier_failure(model.id(), model.revision(), options, Termination::MemoryLimit, "Native frontier snapshot allocation failed");
  }
}

void NativeNeighborhoodSettings::validate() const {
  if(policy!=NativeNeighborhoodPolicy::BinaryHamming)
    throw ModelError("Unknown native neighborhood policy");
  if(!std::isfinite(time_limit_seconds) || time_limit_seconds<0)
    throw ModelError("Native neighborhood time limit must be finite and nonnegative");
}
void NativeNeighborhoodOptions::validate() const {
  search.validate();neighborhood.validate();
}
namespace {
NativeNeighborhoodResult neighborhood_failure(ModelId id,Revision revision,
    const NativeNeighborhoodOptions& options,Termination reason,const std::string& message) {
  NativeNeighborhoodResult out;
  out.search=frontier_failure(id,revision,options.search,reason,message);
  out.search.result.backend+=" + BinaryHamming";
  out.neighborhood.requested=true;
  out.neighborhood.completion=NativeNeighborhoodCompletion::GlobalStop;
  out.neighborhood.stop_reason=reason;
  return out;
}
}
NativeNeighborhoodResult solve_native_neighborhoods(const ModelSnapshot& model,
                                                   const NativeNeighborhoodOptions& options) {
  try {
    NativeNeighborhoodResult out;out.neighborhood.requested=true;
    SolveBudget budget(options.search.solve);
    out.search=native_frontier_impl(model,options.search,budget,&options.neighborhood,&out.neighborhood);
    return out;
  } catch(const ModelError& error) {
    return neighborhood_failure(model.model_id,model.revision,options,Termination::InvalidModel,error.what());
  } catch(const std::bad_alloc&) {
    return neighborhood_failure(model.model_id,model.revision,options,Termination::MemoryLimit,"Native neighborhood allocation failed");
  }
}
NativeNeighborhoodResult solve_native_neighborhoods(const Model& model,
                                                   const NativeNeighborhoodOptions& options) {
  try {
    NativeNeighborhoodResult out;out.neighborhood.requested=true;
    SolveBudget budget(options.search.solve);
    out.search=native_frontier_impl(model.snapshot(),options.search,budget,&options.neighborhood,&out.neighborhood);
    return out;
  } catch(const ModelError& error) {
    return neighborhood_failure(model.id(),model.revision(),options,Termination::InvalidModel,error.what());
  } catch(const std::bad_alloc&) {
    return neighborhood_failure(model.id(),model.revision(),options,Termination::MemoryLimit,"Native neighborhood snapshot allocation failed");
  }
}

namespace {
enum class AutomaticNativeRoute { Native, RootLp, UpdatedLp, ReliabilityLp };
struct AutomaticNativeSelection {
  AutomaticNativeRoute route = AutomaticNativeRoute::Native;
  const char* reason = "ordinary native search";
  bool covers = false;
};

AutomaticNativeSelection select_native_route(const ModelSnapshot& model,
    const SolveOptions& options, const SolveBudget& budget,
    const NativeAutoSettings& settings) {
  // Optional routes must never change acceptance of an explicit native option.
  if ((options.backend != Backend::Auto && options.backend != Backend::Native) ||
      options.guarantee == Guarantee::Certified || options.threads != 1 || options.random_seed != 0)
    return {AutomaticNativeRoute::Native,"native option compatibility"};
#ifndef GECODE_OPTIMIZE_WITH_NATIVE
  (void)model; (void)budget; (void)settings;
  return {AutomaticNativeRoute::Native,"checked LP unavailable"};
#else
  // Bound selection's extra work before calling the full native validator or
  // compiler. The ordinary route still validates every uninspected input.
  constexpr std::size_t max_columns = 4096, max_rows = 4096, max_nonzeros = 65536;
  if (model.variables.size() > max_columns || model.rows.size() > max_rows ||
      model.objective.terms.size() > max_nonzeros)
    return {AutomaticNativeRoute::Native,"structural inspection cap"};
  if (!model.globals.empty() || !model.indicators.empty())
    return {AutomaticNativeRoute::Native,"global or indicator structure"};
  std::size_t nonzeros = model.objective.terms.size(), active_variables = 0;
  for (const auto& variable : model.variables) {
    checkpoint(budget);
    if (!variable.active) continue;
    ++active_variables;
    if (variable.type != VariableType::Binary)
      return {AutomaticNativeRoute::Native,"nonbinary variables"};
  }
  if (active_variables <= 4)
    return {AutomaticNativeRoute::Native,"tiny model"};
  for (const auto& row : model.rows) {
    checkpoint(budget);
    if (row.terms.size() > max_nonzeros-nonzeros)
      return {AutomaticNativeRoute::Native,"structural inspection cap"};
    nonzeros += row.terms.size();
  }
  validate_structure(model);
  checkpoint(budget);
  const auto compiled = compile(model,budget);
  if (!compiled.binary_domains || compiled.rows.empty())
    return {AutomaticNativeRoute::Native,"binary domain or row structure"};
  if (settings.knapsack && prepare_knapsack(compiled,budget))
    return {AutomaticNativeRoute::Native,"eligible exact knapsack DP"};
#ifndef GECODE_OPTIMIZE_NATIVE_LP_ENABLED
  return {AutomaticNativeRoute::Native,"checked LP unavailable"};
#else
  // Use the same checked-LP numeric admission as the explicit LP API, without
  // constructing an LP backend or solving a relaxation during selection.
  try { (void)relaxation_model(compiled,budget); }
  catch (const Unsupported&) {
    return {AutomaticNativeRoute::Native,"checked LP numeric admission"};
  }
  bool unit = true, nonnegative = true;
  for (const auto& row : compiled.rows) for (const auto& term : row.terms) {
    checkpoint(budget);
    unit = unit && (term.coefficient == 1 || term.coefficient == -1);
    nonnegative = nonnegative && term.coefficient >= 0;
  }
  if (unit && nonnegative)
    return {AutomaticNativeRoute::RootLp,"root checked LP for nonnegative unit rows"};
  if (unit)
    return {AutomaticNativeRoute::UpdatedLp,"updated checked LP for signed unit rows"};
  if (options.relative_gap != 0 || options.absolute_gap != 0)
    return {AutomaticNativeRoute::UpdatedLp,"updated checked LP with covers; gap-compatible BAB",true};
  return {AutomaticNativeRoute::ReliabilityLp,"updated checked LP, covers and binary reliability",true};
#endif
#endif
}

SolveResult native_auto_impl(const ModelSnapshot& model, const SolveOptions& options,
                             SolveBudget& budget, const NativeAutoSettings& settings) {
  SolveResult result;
  result.model_id=model.model_id; result.revision=model.revision;
  result.backend="Gecode native"; result.guarantee=options.guarantee;
  AutomaticNativeSelection selected;
  try {
    selected=select_native_route(model,options,budget,settings);
    if (selected.route == AutomaticNativeRoute::Native) {
      result=solve_native_impl(model,options,nullptr,nullptr,&budget,settings.knapsack);
    } else {
      NativeLpOptions lp; lp.solve=options;
      if (selected.route != AutomaticNativeRoute::RootLp) {
        lp.frequency=NativeLpFrequency::AfterBoundChanges;
        lp.bound_change_interval=4;
      }
      if (selected.covers) lp.root_cover_cuts=NativeRootCoverSettings{};
      if (selected.route == AutomaticNativeRoute::ReliabilityLp) {
        NativeSearchOptions search; search.solve=options;
        search.order=NativeSearchOrder::DepthFirst;
        search.relaxation=static_cast<const NativeLpSettings&>(lp);
        search.branching=NativeBranchingSettings{};
        search.branching->max_probe_status_calls=128;
        // Keep the public search default's frontier storage allowance.
        result=native_frontier_impl(model,search,budget).result;
      } else {
        NativeLpStatistics statistics;
        result=solve_native_impl(model,options,&lp,&statistics,&budget);
      }
    }
  } catch (const Interrupted& error) {
    // Preserve native structural-error precedence even if bounded inspection
    // observes a stop before reaching validate_structure. No search can start
    // because the same exhausted/cancelled budget reaches the native bridge.
    (void)error;
    result=solve_native_impl(model,options,nullptr,nullptr,&budget,settings.knapsack);
  } catch (const Unsupported& error) {
    result.termination=Termination::Unsupported; result.message=error.what();
  } catch (const ModelError& error) {
    result.termination=Termination::InvalidModel; result.message=error.what();
  } catch (const std::bad_alloc&) {
    result.termination=Termination::MemoryLimit; result.message="Native automatic selection allocation failed";
#ifdef GECODE_OPTIMIZE_WITH_NATIVE
  } catch (const MemoryExhausted& error) {
    result.termination=Termination::MemoryLimit; result.message=error.what();
#endif
  } catch (const std::exception& error) {
    result.termination=Termination::BackendError; result.message=error.what();
  }
  result.message=std::string("Automatic native policy: ")+selected.reason+"; "+result.message;
  result.elapsed_seconds=budget.elapsed_seconds();
  return result;
}

// Transformations are internal and single-pass. Explicit native APIs retain
// their original behavior; all callbacks keep the same global budget.
SolveResult native_auto_pipeline(const ModelSnapshot& model, const SolveOptions& options,
                                 SolveBudget& budget, const NativeAutoSettings& settings) {
#ifndef GECODE_OPTIMIZE_WITH_NATIVE
  return native_auto_impl(model,options,budget,settings);
#else
  const auto fallback=[&]{return native_auto_impl(model,options,budget,settings);};
  if ((!settings.presolve && !settings.components && !settings.symmetry) ||
      !options.primal_start.empty() || options.guarantee==Guarantee::Certified ||
      (options.backend!=Backend::Auto && options.backend!=Backend::Native) ||
      options.threads!=1 || options.random_seed!=0 ||
      !model.globals.empty() || !model.indicators.empty() ||
      model.variables.size()>4096 || model.rows.size()>4096 ||
      model.objective.terms.size()>65536) return fallback();
  std::size_t nonzeros=model.objective.terms.size();
  for(const auto& v:model.variables) if(v.active &&
      v.type!=VariableType::Integer && v.type!=VariableType::Binary)return fallback();
  for(const auto& row:model.rows){
    if(row.terms.size()>65536-nonzeros)return fallback();
    nonzeros+=row.terms.size();
  }
  try {
    // Preserve original native numeric/structural admission before reductions
    // could erase an unsupported term or contradiction.
    validate_structure(model);checkpoint(budget);
    const auto compiled=compile(model,budget);
    if(settings.knapsack && prepare_knapsack(compiled,budget))return fallback();
    const Detail::NativeSolveContinuation leaf=[&](const ModelSnapshot& m,
        const SolveOptions& o,SolveBudget& b){return native_auto_impl(m,o,b,settings);};
    const Detail::NativeSolveContinuation symmetric=[&](const ModelSnapshot& m,
        const SolveOptions& o,SolveBudget& b){
      if(!settings.symmetry)return leaf(m,o,b);
      // A reduced/component model can itself become a DP candidate.
      try {
        if(settings.knapsack && prepare_knapsack(compile(m,b),b))return leaf(m,o,b);
      } catch(const Unsupported&) {
        // Let the ordinary leaf report admission rather than throwing through
        // the presolve coordinator, which can then retain the original model.
        return leaf(m,o,b);
      }
      if(auto result=Detail::native_symmetry(m,o,b,leaf))return *result;
      return leaf(m,o,b);
    };
    const Detail::NativeSolveContinuation components=[&](const ModelSnapshot& m,
        const SolveOptions& o,SolveBudget& b){
      if(settings.components)
        if(auto result=Detail::native_components(m,o,b,symmetric))return *result;
      return symmetric(m,o,b);
    };
    const Detail::NativeSolveContinuation presolved=[&](const ModelSnapshot& m,
        const SolveOptions& o,SolveBudget& b){
      if(settings.presolve)
        if(auto result=Detail::native_presolve(m,o,b,components))return *result;
      return components(m,o,b);
    };
    auto normalized=settings.presolve ? Detail::native_objective_auxiliary(model,options,budget,presolved):
      std::optional<SolveResult>{};
    auto solved=normalized ? std::move(*normalized):presolved(model,options,budget);
    solved.message="Automatic native policy: preprocessing; "+solved.message;
    solved.elapsed_seconds=budget.elapsed_seconds();
    return solved;
  } catch(const Interrupted&) {return fallback();}
    catch(const Unsupported&) {return fallback();}
    catch(const ModelError&) {return fallback();}
    catch(const std::bad_alloc&) {
      SolveResult result;result.model_id=model.model_id;result.revision=model.revision;
      result.backend="Gecode native";result.guarantee=options.guarantee;
      result.termination=Termination::MemoryLimit;
      result.message="Automatic native policy: preprocessing allocation failed";
      result.elapsed_seconds=budget.elapsed_seconds();return result;
    }
#endif
}

template<class Source>
SolveResult native_auto_entry(const Source& source, const SolveOptions& options,
                              const NativeAutoSettings& settings, ModelId id, Revision revision) {
  const auto started=std::chrono::steady_clock::now();
  SolveResult result; result.model_id=id; result.revision=revision;
  result.backend="Gecode native"; result.guarantee=options.guarantee;
  try {
    SolveBudget budget(options);
    if constexpr (std::is_same_v<Source,Model>)
      result=native_auto_pipeline(source.snapshot(),options,budget,settings);
    else result=native_auto_pipeline(source,options,budget,settings);
    // Include transformation/artifact cleanup in the publication deadline.
    // Ordinary timed incumbents and the explicit-start path retain their existing
    // capture semantics; a newly completed preprocessing proof must be timely.
    if (result.message.find("Automatic native policy: preprocessing;")==0 &&
        (result.termination==Termination::Optimal || result.termination==Termination::Infeasible) &&
        (budget.cancelled() || budget.time_limit_reached())) {
      result=SolveResult{};result.model_id=id;result.revision=revision;
      result.backend="Gecode native";result.guarantee=options.guarantee;
      result.termination=budget.cancelled()?Termination::Cancelled:Termination::TimeLimit;
      result.message="Automatic native policy: preprocessing cleanup exhausted deadline";
    }
  } catch (const ModelError& error) {
    result.termination=Termination::InvalidModel;
    result.message=std::string("Automatic native policy: native validation; ")+error.what();
  } catch (const std::bad_alloc&) {
    result.termination=Termination::MemoryLimit;
    result.message="Automatic native policy: native preparation allocation failed";
  }
  result.elapsed_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
  return result;
}
}

SolveResult solve_native_auto(const ModelSnapshot& model, const SolveOptions& options) {
  return native_auto_entry(model,options,{},model.model_id,model.revision);
}
SolveResult solve_native_auto(const Model& model, const SolveOptions& options) {
  return native_auto_entry(model,options,{},model.id(),model.revision());
}

SolveResult solve_native_auto_configured(const ModelSnapshot& model, const NativeAutoOptions& options) {
  return native_auto_entry(model,options.solve,options.settings,model.model_id,model.revision);
}
SolveResult solve_native_auto_configured(const Model& model, const NativeAutoOptions& options) {
  return native_auto_entry(model,options.solve,options.settings,model.id(),model.revision());
}

void NativeRaceOptions::validate() const {
  solve.validate();
  if (!std::isfinite(exploration_seconds) || exploration_seconds<0.0)
    throw ModelError("native race exploration time must be nonnegative and finite");
  if (!probe_node_limit)
    throw ModelError("native race probe node allowance must be positive");
}

namespace {
SolveResult native_race_impl(const ModelSnapshot& model,const NativeRaceOptions& options,
                            SolveBudget& budget) {
  const auto& solve=options.solve;
  // Never reinterpret incompatible options or disturb supplied-start semantics.
  if (options.exploration_seconds==0 || !solve.primal_start.empty() ||
      !model.globals.empty() || !model.indicators.empty() ||
      solve.guarantee==Guarantee::Certified || solve.threads!=1 || solve.random_seed!=0 ||
      (solve.backend!=Backend::Auto && solve.backend!=Backend::Native) ||
      !native_capabilities().available) {
    auto result=native_auto_pipeline(model,solve,budget,options.automatic);
    result.message="Native race skipped (disabled or direct-path compatibility); "+result.message;
    return result;
  }
  const bool minimize=model.objective.sense==ObjectiveSense::Minimize;
  const auto better_value=[&](double a,double b){return minimize ? a<b:a>b;};
  const auto better_bound=[&](double a,double b){return minimize ? a>b:a<b;};
  std::optional<SolveResult> incumbent;
  std::optional<double> bound;
  unsigned probes=0;
  auto observe=[&](const SolveResult& result){
    // Each candidate solves this same original model; preprocessing restores and
    // checks original coordinates before returning. Never compare partial points.
    if(result.has_solution() && (!incumbent || better_value(*result.objective,*incumbent->objective)))
      incumbent=result;
    if(result.best_bound && !std::isnan(*result.best_bound) &&
        (!bound || better_bound(*result.best_bound,*bound)))bound=result.best_bound;
  };
  const auto run=[&](bool automatic,SolveBudget& allowance){
    auto result=automatic ? native_auto_pipeline(model,solve,allowance,options.automatic):
      solve_native_impl(model,solve,nullptr,nullptr,&allowance);
    // A completed proof is only published after destruction of candidate-local
    // artifacts. Reaching a node cap on the final admitted node is permitted.
    if((result.termination==Termination::Optimal || result.termination==Termination::Infeasible) &&
        (allowance.cancelled() || allowance.time_limit_reached())) {
      result=SolveResult{};result.model_id=model.model_id;result.revision=model.revision;
      result.backend="Gecode native";result.guarantee=solve.guarantee;
      result.termination=allowance.cancelled()?Termination::Cancelled:Termination::TimeLimit;
      result.message="Native race candidate cleanup exhausted deadline";
    }
    return result;
  };
  const auto complete=[](const SolveResult& r){return r.termination==Termination::Optimal ||
    r.termination==Termination::Infeasible;};
  const auto failed=[](const SolveResult& r){return r.termination==Termination::InvalidModel ||
    r.termination==Termination::Unsupported || r.termination==Termination::BackendError ||
    r.termination==Termination::MemoryLimit || r.termination==Termination::NumericalFailure;};
  const auto finish=[&](SolveResult result,const char* selected){
    if(!complete(result)) {
      if(incumbent && (!result.has_solution() || better_value(*incumbent->objective,*result.objective))) {
        result.values=std::move(incumbent->values);
        result.active_variables=std::move(incumbent->active_variables);
        result.objective=incumbent->objective;result.solution_validated=true;
      }
      if(bound && (!result.best_bound || better_bound(*bound,*result.best_bound)))result.best_bound=bound;
      if(auto reason=budget.stop_reason())result.termination=*reason;
      result.update_gaps(model.objective.sense);
    }
    result.message="Native sequential race: "+std::to_string(probes)+" probes; selected "+selected+
      "; cumulative nodes="+std::to_string(budget.nodes())+
      "; exploration/restart may increase CPU work and solve time; "+result.message;
    result.elapsed_seconds=budget.elapsed_seconds();return result;
  };
  // Budget slices share the original clock/cancellation/node counter. Reserve
  // most of a finite deadline for exploitation; no hidden fresh solve budget.
  const auto exploration=std::min(options.exploration_seconds,budget.remaining_seconds()*0.25);
  auto automatic_budget=budget.slice(exploration/2,options.probe_node_limit);
  auto automatic=run(true,automatic_budget);++probes;observe(automatic);
  if(complete(automatic) || failed(automatic) || budget.expired())return finish(std::move(automatic),"automatic");
  auto plain_budget=budget.slice(exploration/2,options.probe_node_limit);
  auto plain=run(false,plain_budget);++probes;observe(plain);
  if(complete(plain) || budget.expired())return finish(std::move(plain),"ordinary native");
  bool use_automatic=true;
  if(!failed(plain)) {
    if(plain.has_solution() != automatic.has_solution())use_automatic=automatic.has_solution();
    else if(plain.has_solution() && plain.objective!=automatic.objective)
      use_automatic=!better_value(*plain.objective,*automatic.objective);
    else if(plain.best_bound && (!automatic.best_bound || better_bound(*plain.best_bound,*automatic.best_bound)))
      use_automatic=false;
  }
  // Intentionally restart: these engines do not export resumable search state.
  // Keep probe incumbents separately so an unproductive restart cannot lose them.
  auto result=run(use_automatic,budget);observe(result);
  return finish(std::move(result),use_automatic?"automatic":"ordinary native");
}

template<class Source>
SolveResult native_race_entry(const Source& source,const NativeRaceOptions& options,
                             ModelId id,Revision revision) {
  const auto started=std::chrono::steady_clock::now();
  SolveResult result;result.model_id=id;result.revision=revision;
  result.backend="Gecode native";result.guarantee=options.solve.guarantee;
  try {
    SolveBudget budget(options.solve);options.validate();
    if constexpr(std::is_same_v<Source,Model>)result=native_race_impl(source.snapshot(),options,budget);
    else result=native_race_impl(source,options,budget);
    if((result.termination==Termination::Optimal || result.termination==Termination::Infeasible) &&
        (budget.cancelled() || budget.time_limit_reached())) {
      result=SolveResult{};result.model_id=id;result.revision=revision;
      result.backend="Gecode native";result.guarantee=options.solve.guarantee;
      result.termination=budget.cancelled()?Termination::Cancelled:Termination::TimeLimit;
      result.message="Native race publication cleanup exhausted deadline";
    }
  } catch(const ModelError& error) {
    result.termination=Termination::InvalidModel;result.message=error.what();
  } catch(const std::bad_alloc&) {
    result.termination=Termination::MemoryLimit;result.message="Native race allocation failed";
  } catch(const std::exception& error) {
    result.termination=Termination::BackendError;result.message=error.what();
  }
  result.elapsed_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
  return result;
}
}

SolveResult solve_native_race(const ModelSnapshot& model,const NativeRaceOptions& options) {
  return native_race_entry(model,options,model.model_id,model.revision);
}
SolveResult solve_native_race(const Model& model,const NativeRaceOptions& options) {
  return native_race_entry(model,options,model.id(),model.revision());
}

}}
