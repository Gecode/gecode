/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GECODE_WORD_ARITHMETIC_REWRITING_HPP
#define GECODE_WORD_ARITHMETIC_REWRITING_HPP

#include <gecode/word/arithmetic.hh>

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Access existing protected filters without posting a second actor.
  template<class Fallback>
  class RewriteFilter : public Fallback {
  public:
    using Fallback::narrow;
  };

  /// Arithmetic bit filtering until signed/unsigned bounds exclude wrapping.
  template<class View, BoundArithmeticOperation op>
  class ArithmeticRewritePolicy {
  public:
    static const PropCond pc0=PC_WORD_DOM;
    static const bool has_high_cost=false;
    static bool can_rewrite(View x, View y) {
      return BoundArithmetic<View,op>::numeric_regime(x,y);
    }
    static ExecStatus post_target(Home home, View x, View y, View z) {
      return BoundArithmetic<View,op>::post(home,x,y,z);
    }
    static ExecStatus filter(Home home, View x, View y, View z) {
      if (op == BA_ADD) {
        GECODE_ES_CHECK(narrow_add(home,x,y,z,BT_ANY).status);
      } else if (op == BA_SUB) {
        GECODE_ES_CHECK(narrow_subtract(home,x,y,z,BT_ANY).status);
      } else {
        GECODE_ES_CHECK(narrow_product(home,x,y,z));
      }
      const bool is_assigned=x.assigned() && y.assigned() && z.assigned();
      return is_assigned ? ES_OK : ES_FIX;
    }
  };

  /// Upgrade unsigned division when the divisor is zero or excludes zero.
  template<BoundUnsignedDivModOperation op>
  class UnsignedDivRewritePolicy {
  public:
    static const PropCond pc0=PC_WORD_BITS;
    static const bool has_high_cost=true;
    static bool can_rewrite(UnsignedWordView, UnsignedWordView y) {
      return BoundUnsignedDivMod<op>::numeric_regime(y);
    }
    static ExecStatus post_target(Home home, UnsignedWordView x,
                             UnsignedWordView y, UnsignedWordView z) {
      return BoundUnsignedDivMod<op>::post(home,x,y,z);
    }
    static ExecStatus filter(Home home, UnsignedWordView x,
                             UnsignedWordView y, UnsignedWordView z) {
      // Preserve Mod::post's self-remainder identity before calling its
      // lower-level filter, including a divisor that can still be zero.
      const bool is_self_remainder=(op == BUD_MOD) && (x == y);
      if (is_self_remainder) {
        GECODE_ME_CHECK(z.eq(home,0));
        return ES_OK;
      }
      return (op == BUD_DIV) ? RewriteFilter<Div>::narrow(home,x,y,z) :
        RewriteFilter<Mod>::narrow(home,x,y,z);
    }
  };

  /// Upgrade signed division when the divisor has one fixed sign or is zero.
  template<SignedDivModOperation op>
  class SignedDivRewritePolicy {
  public:
    static const PropCond pc0=PC_WORD_BITS;
    static const bool has_high_cost=true;
    static bool can_rewrite(SignedWordView, SignedWordView y) {
      return BoundSignedDivMod<op>::numeric_regime(y);
    }
    static ExecStatus post_target(Home home, SignedWordView x,
                             SignedWordView y, SignedWordView z) {
      return BoundSignedDivMod<op>::post(home,x,y,z);
    }
    static ExecStatus filter(Home home, SignedWordView x,
                             SignedWordView y, SignedWordView z) {
      const bool is_self_remainder=(op != SDO_DIV) && (x == y);
      if (is_self_remainder) {
        GECODE_ME_CHECK(z.eq(home,0));
        return ES_OK;
      }
      return RewriteFilter<SignedDivMod<op> >::narrow(home,x,y,z);
    }
  };

  /** \brief Bit filter that rewrites once operand bounds justify its target
   *
   * The policies' predicates are monotone under domain contraction. DOM is
   * one subscription for the union of bit and endpoint changes; subscribing
   * separately to BITS and BND would inflate degree and AFC counts. The first
   * operand needs DOM only for arithmetic; division's eligibility depends
   * solely on its divisor. A policy filter returns ES_OK only when entailed,
   * ES_FIX while an actor is still needed, or ES_FAILED.
   */
  template<class View, class Policy>
  class RewritingTernary : public Propagator {
  public:
    virtual Actor* copy(Space& home) {
      return new (home) RewritingTernary(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return View::me(med) == ME_WORD_BND ?
        PropCost::ternary(PropCost::LO) :
        PropCost::linear(Policy::has_high_cost ? PropCost::HI : PropCost::LO,x0.width());
    }
    virtual void reschedule(Space& home) {
      x0.reschedule(home,*this,Policy::pc0);
      x1.reschedule(home,*this,PC_WORD_DOM);
      x2.reschedule(home,*this,PC_WORD_BITS);
    }
    virtual size_t dispose(Space& home) {
      x0.cancel(home,*this,Policy::pc0);
      x1.cancel(home,*this,PC_WORD_DOM);
      x2.cancel(home,*this,PC_WORD_BITS);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) != ME_WORD_BND) {
        const ExecStatus es=Policy::filter(home,x0,x1,x2);
        if (es == ES_FAILED) return ES_FAILED;
        if (es == ES_OK) return home.ES_SUBSUMED(*this);
      }
      if (Policy::can_rewrite(x0,x1))
        GECODE_REWRITE(*this,(Policy::post_target(home(*this),x0,x1,x2)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, View x, View y, View z) {
      if (Policy::can_rewrite(x,y)) return Policy::post_target(home,x,y,z);
      const ExecStatus es=Policy::filter(home,x,y,z);
      if (es != ES_FIX) return es;
      if (Policy::can_rewrite(x,y)) return Policy::post_target(home,x,y,z);
      (void) new (home) RewritingTernary(home,x,y,z);
      return ES_OK;
    }
  protected:
    View x0, x1, x2;
    RewritingTernary(Home home, View x, View y, View z)
      : Propagator(home), x0(x), x1(y), x2(z) {
      x0.subscribe(home,*this,Policy::pc0);
      x1.subscribe(home,*this,PC_WORD_DOM);
      x2.subscribe(home,*this,PC_WORD_BITS);
    }
    RewritingTernary(Space& home, RewritingTernary& p)
      : Propagator(home,p) {
      x0.update(home,p.x0);
      x1.update(home,p.x1);
      x2.update(home,p.x2);
    }
  };

  template<class View, BoundArithmeticOperation op>
  using RewritingArithmetic =
    RewritingTernary<View,ArithmeticRewritePolicy<View,op> >;
  template<BoundUnsignedDivModOperation op>
  using RewritingUnsignedDivMod =
    RewritingTernary<UnsignedWordView,UnsignedDivRewritePolicy<op> >;
  template<SignedDivModOperation op>
  using RewritingSignedDivMod =
    RewritingTernary<SignedWordView,SignedDivRewritePolicy<op> >;

  /// Signed negation upgrades after the signed minimum is excluded.
  class RewritingNeg : public Propagator {
  public:
    virtual Actor* copy(Space& home) { return new (home) RewritingNeg(home,*this); }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return SignedWordView::me(med) == ME_WORD_BND ?
        PropCost::binary(PropCost::LO) : PropCost::linear(PropCost::LO,x.width());
    }
    virtual void reschedule(Space& home) {
      x.reschedule(home,*this,PC_WORD_DOM);
      z.reschedule(home,*this,PC_WORD_BITS);
    }
    virtual size_t dispose(Space& home) {
      x.cancel(home,*this,PC_WORD_DOM);
      z.cancel(home,*this,PC_WORD_BITS);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (SignedWordView::me(med) != ME_WORD_BND)
        GECODE_ES_CHECK(narrow_negation(home,x,z));
      const bool is_assigned=x.assigned() && z.assigned();
      if (is_assigned) return home.ES_SUBSUMED(*this);
      if (BoundNeg<SignedWordView>::numeric_regime(x))
        GECODE_REWRITE(*this,(BoundNeg<SignedWordView>::post(home(*this),x,z)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, SignedWordView x, SignedWordView z) {
      if (BoundNeg<SignedWordView>::numeric_regime(x))
        return BoundNeg<SignedWordView>::post(home,x,z);
      GECODE_ES_CHECK(narrow_negation(home,x,z));
      const bool is_assigned=x.assigned() && z.assigned();
      if (is_assigned) return ES_OK;
      if (BoundNeg<SignedWordView>::numeric_regime(x))
        return BoundNeg<SignedWordView>::post(home,x,z);
      (void) new (home) RewritingNeg(home,x,z);
      return ES_OK;
    }
  protected:
    SignedWordView x, z;
    RewritingNeg(Home home, SignedWordView a, SignedWordView b)
      : Propagator(home), x(a), z(b) {
      x.subscribe(home,*this,PC_WORD_DOM);
      z.subscribe(home,*this,PC_WORD_BITS);
    }
    RewritingNeg(Space& home, RewritingNeg& p) : Propagator(home,p) {
      x.update(home,p.x);
      z.update(home,p.z);
    }
  };

  /// Carry/borrow upgrades on flag assignment or a fixed wrapping regime.
  template<BoundArithmeticOperation op>
  class RewritingFlagArithmetic : public Propagator {
  public:
    virtual Actor* copy(Space& home) {
      return new (home) RewritingFlagArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return WordView::me(med) == ME_WORD_BND ? PropCost::ternary(PropCost::LO) :
        PropCost::linear(PropCost::LO,x.width());
    }
    virtual void reschedule(Space& home) {
      x.reschedule(home,*this,PC_WORD_DOM);
      y.reschedule(home,*this,PC_WORD_DOM);
      z.reschedule(home,*this,PC_WORD_BITS);
      flag.reschedule(home,*this,Int::PC_BOOL_VAL);
    }
    virtual size_t dispose(Space& home) {
      x.cancel(home,*this,PC_WORD_DOM);
      y.cancel(home,*this,PC_WORD_DOM);
      z.cancel(home,*this,PC_WORD_BITS);
      flag.cancel(home,*this,Int::PC_BOOL_VAL);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      const bool needs_bit_filter=(WordView::me(med) != ME_WORD_BND) ||
        (Int::BoolView::me(med) == Int::ME_BOOL_VAL);
      if (needs_bit_filter)
        GECODE_ES_CHECK(filter(home,x,y,z,flag));
      const bool is_assigned=x.assigned() && y.assigned() && z.assigned() &&
        flag.assigned();
      if (is_assigned) return home.ES_SUBSUMED(*this);
      if (BoundFlagArithmetic<UnsignedWordView,op>::numeric_regime(x,y,flag))
        GECODE_REWRITE(*this,(post_target(home(*this),x,y,z,flag)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView x, UnsignedWordView y,
                           UnsignedWordView z, Int::BoolView flag) {
      if (BoundFlagArithmetic<UnsignedWordView,op>::numeric_regime(x,y,flag))
        return post_target(home,x,y,z,flag);
      GECODE_ES_CHECK(filter(home,x,y,z,flag));
      const bool is_assigned=x.assigned() && y.assigned() && z.assigned() &&
        flag.assigned();
      if (is_assigned) return ES_OK;
      if (BoundFlagArithmetic<UnsignedWordView,op>::numeric_regime(x,y,flag))
        return post_target(home,x,y,z,flag);
      (void) new (home) RewritingFlagArithmetic(home,x,y,z,flag);
      return ES_OK;
    }
  protected:
    UnsignedWordView x, y, z;
    Int::BoolView flag;
    RewritingFlagArithmetic(Home home, UnsignedWordView a, UnsignedWordView b,
                            UnsignedWordView c, Int::BoolView f)
      : Propagator(home), x(a), y(b), z(c), flag(f) {
      x.subscribe(home,*this,PC_WORD_DOM);
      y.subscribe(home,*this,PC_WORD_DOM);
      z.subscribe(home,*this,PC_WORD_BITS);
      flag.subscribe(home,*this,Int::PC_BOOL_VAL);
    }
    RewritingFlagArithmetic(Space& home, RewritingFlagArithmetic& p)
      : Propagator(home,p) {
      x.update(home,p.x);
      y.update(home,p.y);
      z.update(home,p.z);
      flag.update(home,p.flag);
    }
    static ExecStatus filter(Home home, UnsignedWordView x, UnsignedWordView y,
                             UnsignedWordView z, Int::BoolView flag) {
      return op == BA_ADD ? narrow_add_carry(home,x,y,z,flag) :
        narrow_subtract_borrow(home,x,y,z,flag);
    }
    static ExecStatus post_target(Home home, UnsignedWordView x, UnsignedWordView y,
                             UnsignedWordView z, Int::BoolView flag) {
      if (flag.zero())
        return BoundArithmetic<UnsignedWordView,op,BT_CLEAR>::post(home,x,y,z);
      if (flag.one())
        return BoundArithmetic<UnsignedWordView,op,BT_SET>::post(home,x,y,z);
      return BoundFlagArithmetic<UnsignedWordView,op>::post(home,x,y,z,flag);
    }
  };

  /// Joint quotient/remainder fallback retains one shared four-role relation.
  class RewritingUnsignedDivModBoth : public Propagator {
  public:
    virtual Actor* copy(Space& home) {
      return new (home) RewritingUnsignedDivModBoth(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return WordView::me(med) == ME_WORD_BND ? PropCost::binary(PropCost::LO) :
        PropCost::linear(PropCost::HI,a.width());
    }
    virtual void reschedule(Space& home) {
      a.reschedule(home,*this,PC_WORD_BITS);
      b.reschedule(home,*this,PC_WORD_DOM);
      q.reschedule(home,*this,PC_WORD_BITS);
      r.reschedule(home,*this,PC_WORD_BITS);
    }
    virtual size_t dispose(Space& home) {
      a.cancel(home,*this,PC_WORD_BITS);
      b.cancel(home,*this,PC_WORD_DOM);
      q.cancel(home,*this,PC_WORD_BITS);
      r.cancel(home,*this,PC_WORD_BITS);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (WordView::me(med) != ME_WORD_BND) {
        const ExecStatus es=RewriteFilter<DivModBoth>::narrow(home,a,b,q,r);
        if (es == ES_FAILED) return ES_FAILED;
        if (es == ES_OK) return home.ES_SUBSUMED(*this);
      }
      if (BoundUnsignedDivModBoth::numeric_regime(b))
        GECODE_REWRITE(*this,(BoundUnsignedDivModBoth::post(home(*this),a,b,q,r)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView a, UnsignedWordView b,
                           UnsignedWordView q, UnsignedWordView r) {
      if (BoundUnsignedDivModBoth::numeric_regime(b))
        return BoundUnsignedDivModBoth::post(home,a,b,q,r);
      if (a == b) {
        GECODE_ME_CHECK(r.eq(home,0));
        return RewritingUnsignedDivMod<BUD_DIV>::post(home,a,b,q);
      }
      const ExecStatus es=RewriteFilter<DivModBoth>::narrow(home,a,b,q,r);
      if (es != ES_FIX) return es;
      if (BoundUnsignedDivModBoth::numeric_regime(b))
        return BoundUnsignedDivModBoth::post(home,a,b,q,r);
      (void) new (home) RewritingUnsignedDivModBoth(home,a,b,q,r);
      return ES_OK;
    }
  protected:
    UnsignedWordView a, b, q, r;
    RewritingUnsignedDivModBoth(Home home, UnsignedWordView a0,
        UnsignedWordView b0, UnsignedWordView q0, UnsignedWordView r0)
      : Propagator(home), a(a0), b(b0), q(q0), r(r0) {
      a.subscribe(home,*this,PC_WORD_BITS);
      b.subscribe(home,*this,PC_WORD_DOM);
      q.subscribe(home,*this,PC_WORD_BITS);
      r.subscribe(home,*this,PC_WORD_BITS);
    }
    RewritingUnsignedDivModBoth(Space& home, RewritingUnsignedDivModBoth& p)
      : Propagator(home,p) {
      a.update(home,p.a);
      b.update(home,p.b);
      q.update(home,p.q);
      r.update(home,p.r);
    }
  };

  /** \brief N-ary bit addition that rewrites once the sum cannot wrap
   *
   * The constant is folded modulo the word width before posting. Both this
   * predicate and BoundNaryAdd use that same constant and operand order, so
   * a discarded historical overflow cannot select the wrong numeric regime.
   */
  template<class View>
  class RewritingNaryAdd : public MixNaryOnePropagator<
      View,PC_WORD_DOM,View,PC_WORD_BITS> {
  public:
    virtual Actor* copy(Space& home) { return new (home) RewritingNaryAdd(home,*this); }
    virtual size_t dispose(Space& home) {
      (void) MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_BITS>::dispose(home);
      return sizeof(*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      if (View::me(med) == ME_WORD_BND) return PropCost::linear(PropCost::LO,x.size());
      return has_aliases ? PropCost::quadratic(PropCost::HI,x.size()+1) :
        PropCost::linear(PropCost::HI,static_cast<unsigned int>(x.size())*y.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      const bool compacted=compact_nary_add(x,constant,y.mask());
      if ((View::me(med) != ME_WORD_BND) || compacted) {
        do {
          const ExecStatus es=RewriteFilter<NaryAdd>::narrow(home,x,y,constant);
          if (es == ES_FAILED) return ES_FAILED;
          if (es == ES_OK) return home.ES_SUBSUMED(*this);
        } while (compact_nary_add(x,constant,y.mask()));
      }
      if (can_bound_nary_add(x,y,constant))
        GECODE_REWRITE(*this,(BoundNaryAdd<View>::post(home(*this),x,y,constant)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, ViewArray<View>& x, View y, WordValue c) {
      const ExecStatus es=cancel_nary_add_result(home,x,y,c);
      if (es != ES_FIX) return es;
      compact_nary_add(x,c,y.mask());
      if (can_bound_nary_add(x,y,c)) return BoundNaryAdd<View>::post(home,x,y,c);
      const bool has_aliases=has_nary_add_aliases(x,y);
      do {
        const ExecStatus filtered=RewriteFilter<NaryAdd>::narrow(home,x,y,c);
        if (filtered != ES_FIX) return filtered;
      } while (compact_nary_add(x,c,y.mask()));
      if (can_bound_nary_add(x,y,c)) return BoundNaryAdd<View>::post(home,x,y,c);
      (void) new (home) RewritingNaryAdd(home,x,y,c,has_aliases);
      return ES_OK;
    }
  protected:
    using MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_BITS>::x;
    using MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_BITS>::y;
    WordValue constant;
    bool has_aliases;
    RewritingNaryAdd(Home home, ViewArray<View>& x0, View y0, WordValue c, bool has_aliases)
      : MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_BITS>(home,x0,y0),
        constant(c), has_aliases(has_aliases) {}
    RewritingNaryAdd(Space& home, RewritingNaryAdd& p)
      : MixNaryOnePropagator<View,PC_WORD_DOM,View,PC_WORD_BITS>(home,p),
        constant(p.constant), has_aliases(p.has_aliases) {}
  };

}}}
#endif

// STATISTICS: word-prop
