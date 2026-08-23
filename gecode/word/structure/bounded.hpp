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

  template<class View>
  forceinline FixedLocalDomain fixed_snapshot(View x) {
    return FixedLocalDomain{x.width(),x.domain_type(),x.lo(),x.hi(),
                            x.rank_minimum(),x.rank_maximum(),false};
  }

  template<class View>
  forceinline ExecStatus fixed_publish(Home home, View x,
                                       const FixedLocalDomain& d) {
    return me_failed(x.narrow_domain(home,d.lo,d.hi,d.minimum,d.maximum)) ?
      ES_FAILED : ES_OK;
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
    static ExecStatus narrow_bounds(Home home, View x, View result,
                                    unsigned int amount, bool& bits) {
      FixedLocalDomain dx=fixed_snapshot(x), dr=fixed_snapshot(result);
      const WordValue initial_xlo=dx.lo, initial_xhi=dx.hi;
      const WordValue initial_rlo=dr.lo, initial_rhi=dr.hi;
      for (;;) {
        const FixedLocalDomain old_x=dx, old_r=dr;
        dx.deferred=dr.deferred=true;
        if (!narrow_ranges(dx,dr,amount)) return ES_FAILED;
        dx.deferred=dr.deferred=false;
        if (!dx.synchronize() || !dr.synchronize()) return ES_FAILED;
        if ((dx == old_x) && (dr == old_r)) break;
      }
      bits=(dx.lo != initial_xlo) || (dx.hi != initial_xhi) ||
        (dr.lo != initial_rlo) || (dr.hi != initial_rhi);
      GECODE_ES_CHECK(fixed_publish(home,x,dx));
      GECODE_ES_CHECK(fixed_publish(home,result,dr));
      return ES_OK;
    }
    static ExecStatus narrow(Home home, View x, View result,
                             unsigned int amount, bool cube) {
      FixedLocalDomain dx=fixed_snapshot(x), dr=fixed_snapshot(result);
      FixedLocalView vx(dx), vr(dr);
      for (;;) {
        const FixedLocalDomain old_x=dx, old_r=dr;
        dx.deferred=dr.deferred=true;
        if (cube)
          GECODE_ES_CHECK((Fixed<FixedLocalView,FixedLocalView>::narrow(
            home,vx,vr,FO_SHIFT_LEFT,amount,0U)));
        const WordValue before_xlo=dx.lo, before_xhi=dx.hi;
        const WordValue before_rlo=dr.lo, before_rhi=dr.hi;
        if (!narrow_ranges(dx,dr,amount)) return ES_FAILED;
        dx.deferred=dr.deferred=false;
        if (!dx.synchronize() || !dr.synchronize()) return ES_FAILED;
        cube=(dx.lo != before_xlo) || (dx.hi != before_xhi) ||
          (dr.lo != before_rlo) || (dr.hi != before_rhi);
        if ((dx == old_x) && (dr == old_r)) break;
      }
      GECODE_ES_CHECK(fixed_publish(home,x,dx));
      GECODE_ES_CHECK(fixed_publish(home,result,dr));
      return ES_OK;
    }
  public:
    static bool numeric_regime(View x, unsigned int amount) {
      return (amount >= x.width()) ||
        (x.rank_maximum() <= (x.mask() >> amount));
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundedShiftLeft(home,*this);
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
      GECODE_ES_CHECK(narrow(home,x0,x1,amount,true));
      return (x0.assigned() && x1.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, View x, View result,
                           unsigned int amount) {
      GECODE_ES_CHECK(narrow(home,x,result,amount,true));
      if (!(x.assigned() && result.assigned()))
        (void) new (home) BoundedShiftLeft(home,x,result,amount);
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

    static ExecStatus narrow(Home home, UnsignedWordView x,
                             UnsignedWordView amount,
                             UnsignedWordView result) {
      FixedLocalDomain dx=fixed_snapshot(x), da=fixed_snapshot(amount);
      FixedLocalDomain dr=fixed_snapshot(result);
      FixedLocalDomain* role[3]={&dx,&da,&dr};
      if (amount.varimp() == x.varimp()) role[1]=role[0];
      if (result.varimp() == x.varimp()) role[2]=role[0];
      else if (result.varimp() == amount.varimp()) role[2]=role[1];
      const FixedLocalDomain initial[3]={dx,da,dr};
      for (;;) {
        const FixedLocalDomain old_x=dx, old_a=da, old_r=dr;
        dx.deferred=da.deferred=dr.deferred=true;
        const WordValue low_factor=WordValue(1) << role[1]->minimum;
        const WordValue high_factor=WordValue(1) << role[1]->maximum;
        if (!role[2]->range(role[0]->minimum*low_factor,
                            role[0]->maximum*high_factor))
          return ES_FAILED;
        const WordValue xmin=role[2]->minimum/high_factor+
          ((role[2]->minimum%high_factor) != 0);
        if (!role[0]->range(xmin,role[2]->maximum/low_factor))
          return ES_FAILED;

        bool supported=false;
        WordValue amin=0, amax=0;
        for (WordValue a=role[1]->minimum; a<=role[1]->maximum; a++) {
          if (((role[1]->lo & ~a) != 0) || ((a & ~role[1]->hi) != 0))
            continue;
          const WordValue factor=WordValue(1) << a;
          if ((role[0]->minimum*factor > role[2]->maximum) ||
              (role[0]->maximum*factor < role[2]->minimum))
            continue;
          if (!supported) amin=a;
          amax=a; supported=true;
        }
        if (!supported || !role[1]->range(amin,amax)) return ES_FAILED;
        dx.deferred=da.deferred=dr.deferred=false;
        for (unsigned int i=0; i<3; i++) {
          bool distinct=true;
          for (unsigned int j=0; j<i; j++)
            if (role[i] == role[j]) { distinct=false; break; }
          if (distinct && !role[i]->synchronize()) return ES_FAILED;
        }
        if ((dx == old_x) && (da == old_a) && (dr == old_r)) break;
      }
      if (!(dx == initial[0])) GECODE_ES_CHECK(fixed_publish(home,x,dx));
      if ((amount.varimp() != x.varimp()) && !(da == initial[1]))
        GECODE_ES_CHECK(fixed_publish(home,amount,da));
      if ((result.varimp() != x.varimp()) &&
          (result.varimp() != amount.varimp()) && !(dr == initial[2]))
        GECODE_ES_CHECK(fixed_publish(home,result,dr));
      return ES_OK;
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
      GECODE_ES_CHECK(narrow(home,x0,x1,x2));
      return (x0.assigned() && x1.assigned() && x2.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView x,
                           UnsignedWordView amount,
                           UnsignedWordView result) {
      GECODE_ES_CHECK(narrow(home,x,amount,result));
      if (!(x.assigned() && amount.assigned() && result.assigned()))
        (void) new (home) VariableShiftLeftBounds(home,x,amount,result);
      return ES_OK;
    }
  };

}}}

#endif

// STATISTICS: word-prop
