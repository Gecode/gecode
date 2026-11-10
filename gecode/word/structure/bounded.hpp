/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment: http://www.gecode.dev
 */

#ifndef GECODE_WORD_STRUCTURE_BOUNDED_HPP
#define GECODE_WORD_STRUCTURE_BOUNDED_HPP

namespace Gecode { namespace Word { namespace Structure {

  /** \brief Local shift domain of one view
   *
   * Interval narrowing is exact. Publishing a synchronized record reproduces
   * it unless a value set closes the domain further, as for
   * Arithmetic::BoundLocalDomain.
   */
  struct FixedLocalDomain {
    unsigned int width;
    WordDomainType kind;
    WordValue lo, hi, minimum, maximum;
    /// Whether cube/interval synchronization is batched by the current phase
    bool deferred;
    bool synchronize(void) {
      return synchronize_domain(width,kind,lo,hi,minimum,maximum);
    }
    bool range(WordValue min, WordValue max) {
      minimum=std::max(minimum,min); maximum=std::min(maximum,max);
      return (minimum <= maximum) && (deferred || synchronize());
    }
  };

  forceinline bool operator ==(const FixedLocalDomain& x,
                               const FixedLocalDomain& y) {
    return (x.lo == y.lo) && (x.hi == y.hi) &&
      (x.minimum == y.minimum) && (x.maximum == y.maximum);
  }

  class FixedLocalView {
  private:
    FixedLocalDomain* d;
  public:
    explicit FixedLocalView(FixedLocalDomain& d0) : d(&d0) {}
    unsigned int width(void) const { return d->width; }
    WordValue mask(void) const { return width_mask(d->width); }
    WordValue lo(void) const { return d->lo; }
    WordValue hi(void) const { return d->hi; }
    WordValue unknown(void) const { return d->hi & ~d->lo; }
    bool assigned(void) const {
      return (d->lo == d->hi) && (d->minimum == d->maximum);
    }
    ModEvent narrow(Space&, WordValue lo0, WordValue hi0) {
      d->lo |= lo0; d->hi &= hi0;
      if (((d->lo & ~d->hi) != 0) ||
          (!d->deferred && !d->synchronize()))
        return ME_WORD_FAILED;
      return assigned() ? ME_WORD_VAL : ME_WORD_DOM;
    }
  };

  /// Changes of the local records in one shift pass (see PassProgress)
  class FixedPassChanges {
  public:
    /// Note a record from the start of the pass, its deductions and closure
    void add(const FixedLocalDomain& previous, const FixedLocalDomain& deduced,
             const FixedLocalDomain& closed) {
      has_changed |= !(closed == previous);
      has_new_bits |= (closed.lo != previous.lo) || (closed.hi != previous.hi);
      has_jump |= (closed.minimum != deduced.minimum) ||
        (closed.maximum != deduced.maximum);
    }
    PassProgress progress(void) const {
      return classify_pass(has_changed,has_new_bits,has_jump);
    }
  private:
    bool has_changed=false;
    bool has_new_bits=false;
    bool has_jump=false;
  };

  template<class View>
  forceinline FixedLocalDomain fixed_snapshot(View x) {
    return FixedLocalDomain{x.width(),x.domain_type(),x.lo(),x.hi(),
                            x.rank_minimum(),x.rank_maximum(),false};
  }

  /** \brief Tell a local record to \a x and report whether it is a fixpoint
   *
   * The result is ES_NOFIX when a value set closes the domain beyond the
   * record, which the local pass has not filtered with, and otherwise ES_FIX.
   */
  template<class View>
  forceinline ExecStatus fixed_publish(Home home, View x,
                                       const FixedLocalDomain& d) {
    if (me_failed(narrow_word_domain(home,x,{d.lo,d.hi},{d.minimum,d.maximum})))
      return ES_FAILED;
    return (x.has_value_set() && !(fixed_snapshot(x) == d)) ? ES_NOFIX : ES_FIX;
  }

  template<class View>
  class BoundedShiftLeft : public MixBinaryPropagator<
    View,PC_WORD_DOM,View,PC_WORD_DOM> {
  protected:
    using MixBinaryPropagator<View,PC_WORD_DOM,
                              View,PC_WORD_DOM>::x0;
    using MixBinaryPropagator<View,PC_WORD_DOM,
                              View,PC_WORD_DOM>::x1;
    unsigned int amount;
    BoundedShiftLeft(Home home, View x, View result, unsigned int amount0)
      : MixBinaryPropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>(
          home,x,result), amount(amount0) {}
    BoundedShiftLeft(Space& home, BoundedShiftLeft& p)
      : MixBinaryPropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>(home,p),
        amount(p.amount) {}

    static bool narrow_ranges(FixedLocalDomain& dx, FixedLocalDomain& dr,
                              unsigned int amount) {
      if (amount >= dx.width)
        return dr.range(0,0);
      const WordValue factor=WordValue(1) << amount;
      if (!dr.range(dx.minimum*factor,dx.maximum*factor))
        return false;
      const WordValue xmin=dr.minimum/factor+
        ((dr.minimum%factor) != 0);
      return dx.range(xmin,dr.maximum/factor);
    }
    /** \brief Narrow the intervals and publish both records
     *
     * Set \a bits when the cube stage must run: the pass fixed bits or
     * stopped after a closure jump.
     */
    static ExecStatus narrow_bounds(Home home, View x, View result,
                                    unsigned int amount, bool& bits) {
      FixedLocalDomain dx=fixed_snapshot(x), dr=fixed_snapshot(result);
      const WordValue initial_xlo=dx.lo, initial_xhi=dx.hi;
      const WordValue initial_rlo=dr.lo, initial_rhi=dr.hi;
      PassProgress progress;
      // Bounds-only stage: a closure jump stops it and hands over to the cube stage.
      do {
        const FixedLocalDomain old_x=dx, old_r=dr;
        dx.deferred=dr.deferred=true;
        if (!narrow_ranges(dx,dr,amount)) return ES_FAILED;
        const FixedLocalDomain deduced_x=dx, deduced_r=dr;
        dx.deferred=dr.deferred=false;
        if (!dx.synchronize() || !dr.synchronize()) return ES_FAILED;
        FixedPassChanges changes;
        changes.add(old_x,deduced_x,dx);
        changes.add(old_r,deduced_r,dr);
        progress=changes.progress();
      } while (progress == PassProgress::CHANGED);
      bits=(dx.lo != initial_xlo) || (dx.hi != initial_xhi) ||
        (dr.lo != initial_rlo) || (dr.hi != initial_rhi);
      switch (pass_result(publish(home,x,result,dx,dr),progress)) {
      case PassResult::FAILED:
        return ES_FAILED;
      case PassResult::FIXPOINT:
        return ES_FIX;
      case PassResult::STOPPED:
        bits=true;
        return ES_NOFIX;
      case PassResult::CLOSED_BEYOND: {
        const ExecStatus status=narrow(home,x,result,amount);
        bits=(status == ES_NOFIX);
        return status;
      }
      }
      GECODE_NEVER;
      return ES_FAILED;
    }
    /// Publish both records; ES_NOFIX if a value set closed beyond one
    static ExecStatus publish(Home home, View x, View result,
                              const FixedLocalDomain& dx,
                              const FixedLocalDomain& dr) {
      const ExecStatus status=fixed_publish(home,x,dx);
      GECODE_ES_CHECK(status);
      return combine_publication(status,fixed_publish(home,result,dr));
    }
    /// Filter cube and interval until publication closes no value set further
    static ExecStatus narrow(Home home, View x, View result,
                             unsigned int amount) {
      return repeat_until_closed([&](void) {
        return narrow_once(home,x,result,amount);
      });
    }
    /// Filter cube and interval locally to a fixpoint and publish them
    static PassResult narrow_once(Home home, View x, View result,
                                  unsigned int amount) {
      FixedLocalDomain dx=fixed_snapshot(x), dr=fixed_snapshot(result);
      FixedLocalView vx(dx), vr(dr);
      bool cube=true;
      PassProgress progress;
      // Expensive pass: a closure jump stops it, so cheaper propagators run first.
      do {
        const FixedLocalDomain old_x=dx, old_r=dr;
        dx.deferred=dr.deferred=true;
        const bool has_failed_cube=cube &&
          ((Fixed<FixedLocalView,FixedLocalView>::narrow(
             home,vx,vr,FixedOp::SHIFT_LEFT,amount,0U)) < ES_OK);
        if (has_failed_cube) return PassResult::FAILED;
        const WordValue before_xlo=dx.lo, before_xhi=dx.hi;
        const WordValue before_rlo=dr.lo, before_rhi=dr.hi;
        if (!narrow_ranges(dx,dr,amount)) return PassResult::FAILED;
        const FixedLocalDomain deduced_x=dx, deduced_r=dr;
        dx.deferred=dr.deferred=false;
        if (!dx.synchronize() || !dr.synchronize()) return PassResult::FAILED;
        cube=(dx.lo != before_xlo) || (dx.hi != before_xhi) ||
          (dr.lo != before_rlo) || (dr.hi != before_rhi);
        FixedPassChanges changes;
        changes.add(old_x,deduced_x,dx);
        changes.add(old_r,deduced_r,dr);
        progress=changes.progress();
      } while (progress == PassProgress::CHANGED);
      return pass_result(publish(home,x,result,dx,dr),progress);
    }
  public:
    static bool numeric_regime(View x, unsigned int amount) {
      return (amount >= x.width()) ||
        (x.rank_maximum() <= (x.mask() >> amount));
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundedShiftLeft(home,*this);
    }
    virtual size_t dispose(Space& home) {
      (void) MixBinaryPropagator<View,PC_WORD_DOM,View,PC_WORD_DOM>::dispose(home);
      return sizeof(*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::binary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        bool bits;
        GECODE_ES_CHECK(narrow_bounds(home,x0,x1,amount,bits));
        if (x0.assigned() && x1.assigned())
          return home.ES_SUBSUMED(*this);
        if (bits)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      const ExecStatus status=narrow(home,x0,x1,amount);
      GECODE_ES_CHECK(status);
      return (x0.assigned() && x1.assigned()) ?
        home.ES_SUBSUMED(*this) : status;
    }
    static ExecStatus post(Home home, View x, View result,
                           unsigned int amount) {
      const ExecStatus status=narrow(home,x,result,amount);
      GECODE_ES_CHECK(status);
      if (!(x.assigned() && result.assigned()))
        schedule_unless_fixpoint(
          home,*new (home) BoundedShiftLeft(home,x,result,amount),status);
      return ES_OK;
    }
  };

  class VariableShiftLeftBounds : public Propagator {
  protected:
    UnsignedWordView x0, x1, x2;
    VariableShiftLeftBounds(Home home, UnsignedWordView x,
                            UnsignedWordView amount,
                            UnsignedWordView result)
      : Propagator(home), x0(x), x1(amount), x2(result) {
      x0.subscribe(home,*this,PC_WORD_BND);
      x1.subscribe(home,*this,PC_WORD_DOM);
      x2.subscribe(home,*this,PC_WORD_BND);
    }
    VariableShiftLeftBounds(Space& home, VariableShiftLeftBounds& p)
      : Propagator(home,p) {
      x0.update(home,p.x0);
      x1.update(home,p.x1);
      x2.update(home,p.x2);
    }

    /// One deferred interval pass; references can denote the same local role.
    static bool narrow_ranges(FixedLocalDomain& input, FixedLocalDomain& count,
                              FixedLocalDomain& result) {
      const WordValue low_factor=WordValue(1) << count.minimum;
      const WordValue high_factor=WordValue(1) << count.maximum;
      if (!result.range(input.minimum*low_factor,
                          input.maximum*high_factor))
        return false;
      const WordValue xmin=result.minimum/high_factor+
        ((result.minimum%high_factor) != 0);
      if (!input.range(xmin,result.maximum/low_factor))
        return false;
      return true;
    }

    /// Restrict the count to its supported non-wrapping shift endpoints.
    static bool narrow_counts(const FixedLocalDomain& input, FixedLocalDomain& count,
                              const FixedLocalDomain& result) {
      bool supported=false;
      WordValue amin=0, amax=0;
      for (WordValue a=count.minimum; a<=count.maximum; a++) {
        if (((count.lo & ~a) != 0) || ((a & ~count.hi) != 0))
          continue;
        const WordValue factor=WordValue(1) << a;
        if ((input.minimum*factor > result.maximum) ||
            (input.maximum*factor < result.minimum))
          continue;
        if (!supported) amin=a;
        amax=a; supported=true;
      }
      return supported && count.range(amin,amax);
    }

    /// Filter until publication closes no value set further
    static ExecStatus narrow(Home home, UnsignedWordView x,
                             UnsignedWordView amount,
                             UnsignedWordView result) {
      return repeat_until_closed([&](void) {
        return narrow_once(home,x,amount,result);
      });
    }
    /// Filter intervals and counts locally and publish changed roles
    static PassResult narrow_once(Home home, UnsignedWordView x,
                                  UnsignedWordView amount,
                                  UnsignedWordView result) {
      FixedLocalDomain dx=fixed_snapshot(x), da=fixed_snapshot(amount);
      FixedLocalDomain dr=fixed_snapshot(result);
      FixedLocalDomain* role[3]={&dx,&da,&dr};
      if (amount.varimp() == x.varimp()) role[1]=role[0];
      if (result.varimp() == x.varimp()) role[2]=role[0];
      else if (result.varimp() == amount.varimp()) role[2]=role[1];
      const FixedLocalDomain initial[3]={dx,da,dr};
      PassProgress progress;
      // The cube propagator of the shift is separate: a closure jump stops this
      // pass so that it runs first.
      do {
        const FixedLocalDomain old_x=dx, old_a=da, old_r=dr;
        dx.deferred=da.deferred=dr.deferred=true;
        if (!narrow_ranges(*role[0],*role[1],*role[2]) ||
            !narrow_counts(*role[0],*role[1],*role[2]))
          return PassResult::FAILED;
        const FixedLocalDomain deduced_x=dx, deduced_a=da, deduced_r=dr;
        dx.deferred=da.deferred=dr.deferred=false;
        for (unsigned int i=0; i<3; i++) {
          bool distinct=true;
          for (unsigned int j=0; j<i; j++)
            if (role[i] == role[j]) { distinct=false; break; }
          if (distinct && !role[i]->synchronize()) return PassResult::FAILED;
        }
        FixedPassChanges changes;
        changes.add(old_x,deduced_x,dx);
        changes.add(old_a,deduced_a,da);
        changes.add(old_r,deduced_r,dr);
        progress=changes.progress();
      } while (progress == PassProgress::CHANGED);
      ExecStatus status=ES_FIX;
      if (!(dx == initial[0])) {
        status=fixed_publish(home,x,dx);
        if (status < ES_OK) return PassResult::FAILED;
      }
      if ((amount.varimp() != x.varimp()) && !(da == initial[1])) {
        status=combine_publication(status,fixed_publish(home,amount,da));
        if (status < ES_OK) return PassResult::FAILED;
      }
      if ((result.varimp() != x.varimp()) &&
          (result.varimp() != amount.varimp()) && !(dr == initial[2]))
        status=combine_publication(status,fixed_publish(home,result,dr));
      return pass_result(status,progress);
    }
  public:
    static bool numeric_regime(UnsignedWordView x,
                               UnsignedWordView amount) {
      return (amount.rank_maximum() < x.width()) &&
        (x.rank_maximum() <= (x.mask() >> amount.rank_maximum()));
    }
    virtual Actor* copy(Space& home) {
      return new (home) VariableShiftLeftBounds(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual void reschedule(Space& home) {
      x0.reschedule(home,*this,PC_WORD_BND);
      x1.reschedule(home,*this,PC_WORD_DOM);
      x2.reschedule(home,*this,PC_WORD_BND);
    }
    virtual size_t dispose(Space& home) {
      x0.cancel(home,*this,PC_WORD_BND);
      x1.cancel(home,*this,PC_WORD_DOM);
      x2.cancel(home,*this,PC_WORD_BND);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      const ExecStatus status=narrow(home,x0,x1,x2);
      GECODE_ES_CHECK(status);
      return (x0.assigned() && x1.assigned() && x2.assigned()) ?
        home.ES_SUBSUMED(*this) : status;
    }
    static ExecStatus post(Home home, UnsignedWordView x,
                           UnsignedWordView amount,
                           UnsignedWordView result) {
      const ExecStatus status=narrow(home,x,amount,result);
      GECODE_ES_CHECK(status);
      if (!(x.assigned() && amount.assigned() && result.assigned()))
        schedule_unless_fixpoint(
          home,*new (home) VariableShiftLeftBounds(home,x,amount,result),
          status);
      return ES_OK;
    }
  };

}}}

#endif

// STATISTICS: word-prop
