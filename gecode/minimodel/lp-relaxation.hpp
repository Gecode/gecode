/* Experimental HiGHS-backed LP bound propagator. SPDX-License-Identifier: MIT */
#ifndef GECODE_MINIMODEL_LP_RELAXATION_HPP
#define GECODE_MINIMODEL_LP_RELAXATION_HPP

#include <gecode/minimodel/lp-backend.hpp>
#include <memory>

namespace Gecode { namespace Experimental { namespace LpRelaxation {

enum class Frequency { Root, EveryNode };

struct Options {
  Frequency frequency=Frequency::EveryNode;
  bool reduced_cost_fixing=false;
  /// Reoptimize after this many additional binary assignments (root always).
  unsigned int assignment_interval=1;
};

/**
 * Redundant LP lower-bound propagator for binary linear optimization.
 *
 * The shared backend serializes access to a warm-started continuous LP. Each
 * call resets every column bound to the current space. Only an exact checked
 * dual certificate may tighten the objective. Infeasible floating-point LP
 * statuses alone never fail a space. The native constraints remain responsible
 * for feasibility and the equality between the objective and c*x.
 */
class BoundPropagator : public Propagator {
  ViewArray<Int::IntView> x;
  Int::IntView objective;
  std::shared_ptr<SparseBackend> backend;
  std::shared_ptr<const LpCertificate::Certificate> certificate;
  Options options;
  int last_assigned;
  bool has_bound;
  std::int64_t bound_value;

public:
  BoundPropagator(Home home,const IntVarArgs& variables,IntVar cost,
                  const std::shared_ptr<SparseBackend>& engine,const Options& policy)
      : Propagator(home),x(home,variables),objective(cost),backend(engine),
        options(policy),last_assigned(-1),has_bound(false),bound_value(0) {
    home.notice(*this,AP_DISPOSE);
    x.subscribe(home,*this,Int::PC_INT_BND);
    objective.subscribe(home,*this,Int::PC_INT_BND);
  }
  BoundPropagator(Home home,const IntVarArgs& variables,IntVar cost,
                  const std::shared_ptr<SparseBackend>& engine,Frequency f)
      : BoundPropagator(home,variables,cost,engine,Options{f,false,1}) {}
  BoundPropagator(Space& home,BoundPropagator& other)
      : Propagator(home,other),backend(other.backend),certificate(other.certificate),
        options(other.options),
        last_assigned(other.last_assigned),has_bound(other.has_bound),bound_value(other.bound_value) {
    x.update(home,other.x);
    objective.update(home,other.objective);
  }
  Actor* copy(Space& home) override { return new(home) BoundPropagator(home,*this); }
  PropCost cost(const Space&,const ModEventDelta&) const override {
    // Finish cheaper native deductions before invoking numerical optimization.
    return PropCost::crazy(PropCost::HI,x.size());
  }
  void reschedule(Space& home) override {
    x.reschedule(home,*this,Int::PC_INT_BND);
    objective.reschedule(home,*this,Int::PC_INT_BND);
  }
  size_t dispose(Space& home) override {
    home.ignore(*this,AP_DISPOSE);
    if (!home.failed()) {
      x.cancel(home,*this,Int::PC_INT_BND);
      objective.cancel(home,*this,Int::PC_INT_BND);
    }
    certificate.~shared_ptr();
    backend.~shared_ptr();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
  ExecStatus propagate(Space& home,const ModEventDelta&) override {
    int assigned=0;
    for (int j=0;j<x.size();++j) assigned+=x[j].assigned();
    // Native rows and objective equality completely check fully assigned x.
    if (assigned==x.size()) return home.ES_SUBSUMED(*this);
    const bool solve_lp=last_assigned<0 ||
      (options.frequency==Frequency::EveryNode &&
       static_cast<unsigned int>(assigned-last_assigned)>=options.assignment_interval);
    std::vector<std::int64_t> lower,upper;
    if (solve_lp || certificate) {
      lower.resize(x.size()); upper.resize(x.size());
      for (int j=0;j<x.size();++j) { lower[j]=x[j].min(); upper[j]=x[j].max(); }
    }
    if (solve_lp) {
      const bool retain=options.reduced_cost_fixing || options.assignment_interval>1;
      BoundResult result=backend->bound(lower,upper,retain);
      last_assigned=assigned;
      if (result.certificate)
        certificate=std::move(result.certificate);
      if (result.valid && (!has_bound || result.lower_bound>bound_value)) {
        bound_value=result.lower_bound;
        has_bound=true;
      }
    }

    bool fixed=false;
    if (certificate) {
      std::int64_t evaluated;
      if (options.reduced_cost_fixing) {
        std::vector<unsigned char> forbidden;
        if (certificate->filter(lower,upper,objective.max(),evaluated,forbidden)) {
          const std::uint64_t checks=2*static_cast<std::uint64_t>(x.size()-assigned);
          std::uint64_t fixings=0;
          if (!has_bound || evaluated>bound_value) {
            has_bound=true; bound_value=evaluated;
          }
          if (bound_value>objective.max()) {
            backend->record_filtering(checks,fixings);
            return ES_FAILED;
          }
          // Every forbidden bit was proved against the original box, so
          // all these deductions are valid together. Native propagators
          // run again before the next LP call after any successful fixing.
          for (int j=0; j<x.size(); ++j) {
            if (!forbidden[j]) continue;
            if (forbidden[j]==3) {
              backend->record_filtering(checks,fixings);
              return ES_FAILED;
            }
            const ModEvent event=x[j].eq(home,forbidden[j]==1 ? 1 : 0);
            if (me_failed(event)) {
              backend->record_filtering(checks,fixings);
              return ES_FAILED;
            }
            if (me_modified(event)) { ++fixings; fixed=true; }
          }
          backend->record_filtering(checks,fixings);
        }
      } else if (certificate->lower_bound(lower,upper,evaluated) &&
                 (!has_bound || evaluated>bound_value)) {
        // Throttling LP calls does not discard cheap deductions from the
        // last exact certificate as this space's box becomes smaller.
        has_bound=true; bound_value=evaluated;
      }
    }
    // In a binary space every domain change increases the assigned count.
    // An objective-only event can reuse its cached certified ancestor bound.
    if (has_bound) {
      if (bound_value>objective.max()) return ES_FAILED;
      if (bound_value>objective.min())
        GECODE_ME_CHECK(objective.gq(home,static_cast<int>(bound_value)));
    }
    // Legacy root-only bounding is one-shot. Root-only fixing keeps its
    // certificate for future objective cuts, but never solves another LP.
    if (options.frequency==Frequency::Root &&
        (!options.reduced_cost_fixing || !certificate))
      return home.ES_SUBSUMED(*this);
    return fixed ? ES_NOFIX : ES_FIX;
  }
};

/// Post all native rows/objective equality plus the optional LP bound actor.
template<class Engine,
         typename std::enable_if<std::is_base_of<SparseBackend,Engine>::value,int>::type=0>
inline void binary_linear_minimize(Home home,const IntVarArgs& x,IntVar objective,
                                   const std::shared_ptr<Engine>& backend,
                                   const Options& options) {
  if (!backend) throw std::invalid_argument("LP backend is null");
  if (!options.assignment_interval)
    throw std::invalid_argument("LP assignment interval must be positive");
  const std::shared_ptr<SparseBackend> engine=backend;
  post_native(home,x,objective,engine->model);
  if (!home.failed())
    (void) new(home) BoundPropagator(home,x,objective,engine,options);
}

template<class Engine,
         typename std::enable_if<std::is_base_of<SparseBackend,Engine>::value,int>::type=0>
inline void binary_linear_minimize(Home home,const IntVarArgs& x,IntVar objective,
                                   const std::shared_ptr<Engine>& backend,
                                   Frequency frequency=Frequency::EveryNode) {
  binary_linear_minimize(home,x,objective,backend,Options{frequency,false,1});
}

// Keep the original posting signatures for source/function-pointer compatibility.
inline void binary_linear_minimize(Home home,const IntVarArgs& x,IntVar objective,
                                   const std::shared_ptr<Backend>& backend,
                                   const Options& options) {
  binary_linear_minimize(home,x,objective,
                         std::shared_ptr<SparseBackend>(backend),options);
}
inline void binary_linear_minimize(Home home,const IntVarArgs& x,IntVar objective,
                                   const std::shared_ptr<Backend>& backend,
                                   Frequency frequency=Frequency::EveryNode) {
  binary_linear_minimize(home,x,objective,backend,Options{frequency,false,1});
}

/// Explicit integer policy; no binary-assignment scheduling is inferred.
struct IntegerOptions {
  Frequency frequency=Frequency::EveryNode;
  bool bound_tightening=false;
  /// Reoptimize after this many observed variable interval changes (root always).
  unsigned int bound_change_interval=1;
};

namespace Detail {
class IntegerBoundPropagator : public Propagator {
  ViewArray<Int::IntView> x;
  Int::IntView objective;
  std::shared_ptr<BoundedIntegerBackend> backend;
  std::shared_ptr<const LpCertificate::Certificate> certificate;
  std::vector<std::int64_t> last_lower,last_upper;
  IntegerOptions options;
  std::uint64_t pending_changes=0;
  bool attempted=false,has_bound=false;
  std::int64_t bound_value=0;
public:
  IntegerBoundPropagator(Home home,const IntVarArgs& variables,IntVar cost,
                         const std::shared_ptr<BoundedIntegerBackend>& engine,const IntegerOptions& policy)
    : Propagator(home),x(home,variables),objective(cost),backend(engine),options(policy) {
    home.notice(*this,AP_DISPOSE);
    x.subscribe(home,*this,Int::PC_INT_BND);
    objective.subscribe(home,*this,Int::PC_INT_BND);
  }
  IntegerBoundPropagator(Space& home,IntegerBoundPropagator& other)
    : Propagator(home,other),backend(other.backend),certificate(other.certificate),
      last_lower(other.last_lower),last_upper(other.last_upper),options(other.options),
      pending_changes(other.pending_changes),attempted(other.attempted),
      has_bound(other.has_bound),bound_value(other.bound_value) {
    x.update(home,other.x);objective.update(home,other.objective);
  }
  Actor* copy(Space& home) override {return new(home) IntegerBoundPropagator(home,*this);}
  PropCost cost(const Space&,const ModEventDelta&) const override {
    return PropCost::crazy(PropCost::HI,x.size());
  }
  void reschedule(Space& home) override {
    x.reschedule(home,*this,Int::PC_INT_BND);objective.reschedule(home,*this,Int::PC_INT_BND);
  }
  std::size_t dispose(Space& home) override {
    home.ignore(*this,AP_DISPOSE);
    if (!home.failed()) {
      x.cancel(home,*this,Int::PC_INT_BND);objective.cancel(home,*this,Int::PC_INT_BND);
    }
    last_lower.~vector();last_upper.~vector();
    certificate.~shared_ptr();backend.~shared_ptr();
    (void) Propagator::dispose(home);return sizeof(*this);
  }
  ExecStatus propagate(Space& home,const ModEventDelta&) override {
    std::vector<std::int64_t> lower(x.size()),upper(x.size());
    int assigned=0;std::uint64_t changed=0;
    for (int j=0;j<x.size();++j) {
      lower[j]=x[j].min();upper[j]=x[j].max();assigned+=x[j].assigned();
      if (attempted && (lower[j]!=last_lower[j] || upper[j]!=last_upper[j])) ++changed;
    }
    // Original native rows and equality enforce all fully assigned solutions.
    if (assigned==x.size()) return home.ES_SUBSUMED(*this);
    pending_changes=std::min<std::uint64_t>(options.bound_change_interval,pending_changes+changed);
    last_lower=lower;last_upper=upper;
    const bool solve_lp=!attempted || (options.frequency==Frequency::EveryNode &&
                                      pending_changes>=options.bound_change_interval);
    if (solve_lp) {
      const bool retain=options.bound_tightening || options.bound_change_interval>1;
      auto result=backend->bound(lower,upper,retain);
      attempted=true;pending_changes=0;
      if (result.certificate) certificate=std::move(result.certificate);
      if (result.valid && (!has_bound || result.lower_bound>bound_value)) {
        has_bound=true;bound_value=result.lower_bound;
      }
    }
    bool modified=false;
    if (certificate) {
      if (options.bound_tightening) {
        LpCertificate::IntegerFilterResult result;
        if (certificate->filter_integer(lower,upper,objective.max(),result)) {
          if (!has_bound || result.lower_bound>bound_value) {has_bound=true;bound_value=result.lower_bound;}
          const auto checks=static_cast<std::uint64_t>(x.size()-assigned);
          std::uint64_t tightened=0,fixed=0;
          if (result.infeasible || bound_value>objective.max()) {
            backend->record_filtering(checks,tightened,fixed);return ES_FAILED;
          }
          // All intersections were certified against the same original box.
          for (int j=0;j<x.size();++j) {
            if (result.lower[j]==lower[j] && result.upper[j]==upper[j]) continue;
            const bool was_assigned=x[j].assigned();
            const auto a=x[j].gq(home,static_cast<int>(result.lower[j]));
            if (me_failed(a)) {backend->record_filtering(checks,tightened,fixed);return ES_FAILED;}
            const auto b=x[j].lq(home,static_cast<int>(result.upper[j]));
            if (me_failed(b)) {backend->record_filtering(checks,tightened,fixed);return ES_FAILED;}
            if (me_modified(a) || me_modified(b)) {
              modified=true;++tightened;
              if (!was_assigned && x[j].assigned()) ++fixed;
            }
          }
          backend->record_filtering(checks,tightened,fixed);
        }
      } else {
        std::int64_t evaluated;
        if (certificate->lower_bound_integer(lower,upper,evaluated) &&
            (!has_bound || evaluated>bound_value)) {has_bound=true;bound_value=evaluated;}
      }
    }
    if (has_bound) {
      if (bound_value>objective.max()) return ES_FAILED;
      if (bound_value>objective.min()) GECODE_ME_CHECK(objective.gq(home,static_cast<int>(bound_value)));
    }
    if (options.frequency==Frequency::Root && (!options.bound_tightening || !certificate))
      return home.ES_SUBSUMED(*this);
    return modified ? ES_NOFIX : ES_FIX;
  }
};
}

inline void integer_linear_minimize(Home home,const IntVarArgs& x,IntVar objective,
                                    const std::shared_ptr<BoundedIntegerBackend>& backend,
                                    const IntegerOptions& options={}) {
  if (!backend) throw std::invalid_argument("Integer LP backend is null");
  if (!options.bound_change_interval ||
      (options.frequency!=Frequency::Root && options.frequency!=Frequency::EveryNode))
    throw std::invalid_argument("Invalid integer LP frequency or bound-change interval");
  post_native_integer(home,x,objective,backend->model);
  if (!home.failed()) (void) new(home) Detail::IntegerBoundPropagator(home,x,objective,backend,options);
}

}}}
#endif
