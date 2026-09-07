/* Explicit root LP / verified global-cover loop. SPDX-License-Identifier: MIT */
#ifndef GECODE_MINIMODEL_LP_CUT_LOOP_HPP
#define GECODE_MINIMODEL_LP_CUT_LOOP_HPP

#include <gecode/minimodel/lp-backend.hpp>
#include <gecode/minimodel/lp-cuts.hpp>
#include <functional>

namespace Gecode { namespace Experimental { namespace LpRelaxation { namespace Cuts {

#ifdef GECODE_LP_CUT_LOOP_TEST_HOOKS
void root_cut_loop_test_event(const char* event);
#endif

enum class RootLoopCompletion {
  NoNewCuts, RoundLimit, WorkLimit, StorageLimit, SeparationLimit,
  Cancelled, TimeLimit, NoPrimalSuggestion, InvalidSuggestion,
  CallbackError, BackendError, AllocationFailure
};

struct RootLoopOptions {
  std::size_t max_rounds=4,max_work=4000000;
  std::size_t max_columns=200000,max_rows=200000,max_nonzeros=2000000;
  PoolLimits pool;
  SeparationOptions separation;
  // A bounded power of two keeps native endpoints * denominator exact in double.
  Integer denominator=1048576;
  std::optional<std::chrono::steady_clock::time_point> deadline;
  std::function<bool()> stop_requested;
  void validate() const {
    if (denominator<=0 || denominator>1048576 || (denominator&(denominator-1)))
      throw std::invalid_argument("root cover denominator must be a power of two in [1,1048576]");
  }
};

struct RootLoopStatistics {
  std::size_t work=0,rounds=0,augmentations=0,projected_coordinates=0;
  std::size_t separated_cuts=0,duplicate_cuts=0,unsupported_rows=0,oversized_rows=0;
  std::size_t arithmetic_rejections=0;
  std::uint64_t lp_calls=0,valid_bounds=0,rejected_bounds=0,floating_infeasible_reports=0;
  double lp_seconds=0.0;
};

/** Exact bound evidence retains its particular matrix and all appended-row
 * attribution. Every cuts[i] is global and belongs to the original source;
 * matrix row original_rows+i is the negation of cuts[i].inequality().
 * It does not certify integer feasibility or floating LP optimality.
 */
struct RootBoundEvidence {
  SourceModel model;
  std::vector<VerifiedCut> cuts;
  BoundResult bound;
};

struct RootLoopResult {
  SourceModel original;
  SourceModel model;
  std::vector<VerifiedCut> cuts;
  // Null until construction has completed. Its model is an immutable copy of model.
  std::shared_ptr<BoundedIntegerBackend> backend;
  std::optional<RootBoundEvidence> best_bound;
  RootLoopCompletion completion=RootLoopCompletion::RoundLimit;
  RootLoopStatistics stats;
  explicit RootLoopResult(SourceModel source):original(source),model(std::move(source)) {}
};

struct RationalSelection {
  FractionalPoint point;
  std::size_t projected_coordinates=0;
};

/** This projects a finite numerical suggestion into the original box, then
 * rounds to the nearest grid point, halfway away from zero. It is solely a cut
 * selection heuristic: neither a feasible witness nor a primal bound.
 */
inline RationalSelection rational_selection_point(const SourceModel& source,
    const std::vector<double>& values,Integer denominator=1048576) {
  RootLoopOptions options;options.denominator=denominator;options.validate();
  const auto& model=source.model();
  if (values.size()!=model.lower.size())
    throw std::invalid_argument("root cover suggestion dimensions");
  RationalSelection result;result.point.denominator=denominator;
  result.point.numerator.reserve(values.size());
  for (std::size_t j=0;j<values.size();++j) {
    if (!std::isfinite(values[j])) throw std::invalid_argument("root cover suggestion must be finite");
    const double projected=std::max(static_cast<double>(model.lower[j]),
      std::min(static_cast<double>(model.upper[j]),values[j]));
    if (projected!=values[j]) ++result.projected_coordinates;
    // Source validation and the denominator cap bound these exact integer
    // endpoints below 2^51. Scaling by a power of two introduces no error.
    const auto lower=Detail::times_positive(model.lower[j],denominator);
    const auto upper=Detail::times_positive(model.upper[j],denominator);
    const double rounded=std::round(projected*static_cast<double>(denominator));
    if (!std::isfinite(rounded) || rounded<static_cast<double>(lower) || rounded>static_cast<double>(upper))
      throw std::invalid_argument("root cover rational projection is outside the original box");
    result.point.numerator.push_back(static_cast<Integer>(rounded));
  }
  return result;
}

namespace LoopDetail {
struct Stop {RootLoopCompletion reason;};
inline void event(const char* name) {
#ifdef GECODE_LP_CUT_LOOP_TEST_HOOKS
  root_cut_loop_test_event(name);
#else
  (void)name;
#endif
}
class Budget {
  const RootLoopOptions& options_;
  RootLoopStatistics& stats_;
public:
  Budget(const RootLoopOptions& options,RootLoopStatistics& stats):options_(options),stats_(stats) {}
  void checkpoint() const {
    bool cancelled=false;
    if (options_.stop_requested) {
      try {cancelled=options_.stop_requested();}
      catch (...) {throw Stop{RootLoopCompletion::CallbackError};}
    }
    if (cancelled) throw Stop{RootLoopCompletion::Cancelled};
    if (options_.deadline && std::chrono::steady_clock::now()>=*options_.deadline)
      throw Stop{RootLoopCompletion::TimeLimit};
  }
  std::size_t remaining() const {return options_.max_work-stats_.work;}
  void charge(std::size_t units) {
    checkpoint();
    if (units>remaining()) throw Stop{RootLoopCompletion::WorkLimit};
    stats_.work+=units;
  }
};
inline void shape(const BoundedIntegerModel& model,const RootLoopOptions& options) {
  const auto limit=static_cast<std::size_t>(std::numeric_limits<HighsInt>::max());
  if (model.linear.c.size()>std::min(options.max_columns,limit) ||
      model.linear.b.size()>std::min(options.max_rows,limit) ||
      model.linear.a.size()>std::min(options.max_nonzeros,limit))
    throw Stop{RootLoopCompletion::StorageLimit};
}
// A structural work reservation for each full validation or model copy. Copies,
// CSR validation and native activity checks are O(columns+rows+nonzeros).
inline void model_work(const BoundedIntegerModel& model,Budget& budget) {
  for (unsigned i=0;i<3;++i) budget.charge(model.linear.c.size());
  for (unsigned i=0;i<2;++i) budget.charge(model.linear.b.size());
  for (unsigned i=0;i<2;++i) budget.charge(model.linear.a.size());
  budget.charge(1);
}
inline void records_work(const std::vector<VerifiedCut>& cuts,Budget& budget) {
  for (const auto& cut:cuts) {budget.charge(1);budget.charge(cut.inequality().column.size());}
}
inline BoundedIntegerModel append(const SourceModel& original,const std::vector<VerifiedCut>& cuts,
    const RootLoopOptions& options,Budget& budget) {
  const auto& base=original.model();
  const auto row_limit=std::min({options.max_rows,
    static_cast<std::size_t>(std::numeric_limits<HighsInt>::max()),
    static_cast<std::size_t>(Int::Limits::max)});
  const auto term_limit=std::min(options.max_nonzeros,
    static_cast<std::size_t>(std::numeric_limits<HighsInt>::max()));
  if (cuts.size()>row_limit-base.linear.b.size()) throw Stop{RootLoopCompletion::StorageLimit};
  std::size_t terms=base.linear.a.size();
  for (const auto& record:cuts) {
    budget.charge(1);
    if (!record.source().same_identity(original) || record.proof().scope.kind!=ScopeKind::Global)
      throw std::invalid_argument("root cover append requires verified global original-source records");
    const auto n=record.inequality().column.size();
    if (n>term_limit-terms) throw Stop{RootLoopCompletion::StorageLimit};
    terms+=n;
  }
  model_work(base,budget);
  BoundedIntegerModel result=base;
  result.linear.a.reserve(terms);result.linear.column.reserve(terms);
  result.linear.b.reserve(base.linear.b.size()+cuts.size());
  result.linear.row_start.reserve(base.linear.b.size()+cuts.size()+1);
  for (const auto& record:cuts) {
    budget.charge(1);const auto& cut=record.inequality();
    for (std::size_t k=0;k<cut.column.size();++k) {
      budget.charge(1);
      result.linear.column.push_back(cut.column[k]);
      result.linear.a.push_back(Detail::subtract(0,cut.coefficient[k]));
    }
    result.linear.b.push_back(Detail::subtract(0,cut.upper));
    result.linear.row_start.push_back(result.linear.a.size());
  }
  // SourceModel and backend construction repeat their own validation below.
  // This explicit preflight rejects unsupported native coefficient/activity
  // ranges before any new model/backend is published.
  model_work(result,budget);validate_integer_model(result);
  return result;
}
inline void observed(const Stats& before,const Stats& after,RootLoopStatistics& stats) {
  stats.lp_calls+=after.lp_calls-before.lp_calls;
  stats.valid_bounds+=after.valid_bounds-before.valid_bounds;
  stats.rejected_bounds+=after.rejected-before.rejected;
  stats.floating_infeasible_reports+=after.infeasible_status-before.infeasible_status;
  stats.lp_seconds+=(after.lp_ms-before.lp_ms)/1000.0;
}
}

/** Root-only bounded heuristic. All source rows, domains and costs survive in
 * their original order. Only independently verified ORIGINAL-source GLOBAL
 * covers are appended. A round is published atomically after native preflight
 * and fresh backend construction. No floating status can establish infeasibility.
 * Deadline/stop checks are cooperative around LP and bounded separation calls;
 * an LP attempt retains the backend's existing 0.2s / 10000-iteration allowance.
 */
inline RootLoopResult root_cover_loop(SourceModel source,const RootLoopOptions& options={}) {
  options.validate(); // Malformed options are a programming error, not partial work.
  (void)source.model();
  RootLoopResult result(source);
  LoopDetail::Budget budget(options,result.stats);
  CutPool pool(source,options.pool);
  try {
    budget.checkpoint();LoopDetail::shape(source.model(),options);
    if (!options.max_rounds) throw LoopDetail::Stop{RootLoopCompletion::RoundLimit};
    if (!options.max_work) throw LoopDetail::Stop{RootLoopCompletion::WorkLimit};
    LoopDetail::model_work(result.model.model(),budget); // Owning backend copy.
    LoopDetail::model_work(result.model.model(),budget); // Backend preflight/import.
    LoopDetail::event("initial_backend");
    auto initial=std::make_shared<BoundedIntegerBackend>(result.model.model());
    budget.checkpoint();result.backend=std::move(initial);
    for (std::size_t round=0;round<options.max_rounds;++round) {
      budget.checkpoint();
      LoopDetail::model_work(result.model.model(),budget); // Bound/certificate scan.
      const auto before=result.backend->statistics();
      ++result.stats.rounds;
      LoopDetail::event("before_bound");
      BoundResult bound;
      try {bound=result.backend->bound(source.model().lower,source.model().upper,true,true);}
      catch (...) {LoopDetail::observed(before,result.backend->statistics(),result.stats);throw;}
      LoopDetail::observed(before,result.backend->statistics(),result.stats);
      LoopDetail::event("after_bound");
      budget.checkpoint();
      if (bound.valid && (!result.best_bound || bound.lower_bound>result.best_bound->bound.lower_bound)) {
        LoopDetail::records_work(result.cuts,budget);
        BoundResult retained;retained.valid=true;retained.lower_bound=bound.lower_bound;
        retained.lp_objective=bound.lp_objective;retained.certificate=bound.certificate;
        RootBoundEvidence evidence{result.model,result.cuts,std::move(retained)};
        LoopDetail::event("bound_publication");
        budget.checkpoint();result.best_bound=std::move(evidence);
      }
      if (!bound.primal_suggestion) throw LoopDetail::Stop{RootLoopCompletion::NoPrimalSuggestion};
      budget.charge(source.model().lower.size());
      RationalSelection selection;
      try {selection=rational_selection_point(source,*bound.primal_suggestion,options.denominator);}
      catch (const std::invalid_argument&) {throw LoopDetail::Stop{RootLoopCompletion::InvalidSuggestion};}
      result.stats.projected_coordinates+=selection.projected_coordinates;
      budget.checkpoint();
      auto separation=options.separation;
      separation.max_work=std::min(separation.max_work,budget.remaining());
      auto separated=separate_covers(source,selection.point,Scope::global(),separation);
      // The separator has already consumed these units, even if a stop occurs
      // immediately afterward. Its own cap guarantees this addition is safe.
      result.stats.work+=separated.stats.work;
      result.stats.separated_cuts+=separated.cuts.size();
      result.stats.duplicate_cuts+=separated.stats.duplicate_cuts;
      result.stats.unsupported_rows+=separated.stats.unsupported_rows;
      result.stats.oversized_rows+=separated.stats.oversized_rows;
      result.stats.arithmetic_rejections+=separated.stats.arithmetic_rejections;
      budget.checkpoint();
      LoopDetail::records_work(pool.records(),budget);
      CutPool staged_pool=pool;
      bool storage=false;
      for (const auto& cut:separated.cuts) {
        // Pool canonical comparisons and copies are linear in its bounded
        // records/terms; reserve that work before each transactional insertion.
        LoopDetail::records_work(staged_pool.records(),budget);
        budget.charge(1);budget.charge(cut.inequality().column.size());
        const auto inserted=staged_pool.insert(cut);
        if (inserted==InsertStatus::Capacity) {storage=true;break;}
        if (inserted==InsertStatus::Duplicate) ++result.stats.duplicate_cuts;
        if (inserted==InsertStatus::ForeignSource || inserted==InsertStatus::Replaced)
          throw std::runtime_error("root cover pool violated original/global append invariants");
      }
      if (staged_pool.records().size()==pool.records().size()) {
        if (storage) throw LoopDetail::Stop{RootLoopCompletion::StorageLimit};
        if (separated.stats.work_limit) throw LoopDetail::Stop{RootLoopCompletion::WorkLimit};
        if (separated.stats.row_limit || separated.stats.cut_limit)
          throw LoopDetail::Stop{RootLoopCompletion::SeparationLimit};
        budget.checkpoint();
        result.completion=RootLoopCompletion::NoNewCuts;return result;
      }
      LoopDetail::event("augmentation_build");
      auto augmented=LoopDetail::append(source,staged_pool.records(),options,budget);
      LoopDetail::model_work(augmented,budget);
      SourceModel staged_model(std::move(augmented));
      LoopDetail::model_work(staged_model.model(),budget);
      LoopDetail::model_work(staged_model.model(),budget);
      LoopDetail::event("augmented_backend");
      auto staged_backend=std::make_shared<BoundedIntegerBackend>(staged_model.model());
      LoopDetail::records_work(staged_pool.records(),budget);
      auto staged_cuts=staged_pool.records();
      LoopDetail::event("augmentation_publication");
      budget.checkpoint();
      // All operations after this publication gate move/swap already owned data.
      result.model=std::move(staged_model);result.backend=std::move(staged_backend);
      result.cuts.swap(staged_cuts);pool=std::move(staged_pool);++result.stats.augmentations;
      LoopDetail::event("after_augmentation_publication");
      if (storage) throw LoopDetail::Stop{RootLoopCompletion::StorageLimit};
      if (separated.stats.work_limit) throw LoopDetail::Stop{RootLoopCompletion::WorkLimit};
    }
    budget.checkpoint();
    result.completion=RootLoopCompletion::RoundLimit;
  } catch (const LoopDetail::Stop& stop) {
    result.completion=stop.reason;
  } catch (const std::bad_alloc&) {
    result.completion=RootLoopCompletion::AllocationFailure;
  } catch (const std::exception&) {
    result.completion=RootLoopCompletion::BackendError;
  } catch (...) {
    result.completion=RootLoopCompletion::BackendError;
  }
  return result;
}

}}}}
#endif
