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

#ifndef GECODE_WORD_REL_BOUNDED_HPP
#define GECODE_WORD_REL_BOUNDED_HPP

namespace Gecode { namespace Word { namespace Rel {

  /// Constant word view with a rank in the selected numeric order
  template<bool sign>
  class RankConstView : public ConstWordView {
  private:
    WordValue _rank;
  public:
    RankConstView(void) : _rank(0) {}
    RankConstView(unsigned int width, WordValue value)
      : ConstWordView(width,value),
        _rank(Word::rank(sign ? WDT_SIGNED : WDT_UNSIGNED,width,value)) {}
    WordValue minimum(void) const { return _rank; }
    WordValue maximum(void) const { return _rank; }
    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum) {
      ModEvent me = narrow(home,lo,hi);
      if (me_failed(me))
        return me;
      return ((_rank >= minimum) && (_rank <= maximum)) ?
        ME_WORD_NONE : ME_WORD_FAILED;
    }
    ModEvent narrow_range(Space&, WordValue minimum, WordValue maximum) {
      return ((_rank >= minimum) && (_rank <= maximum)) ?
        ME_WORD_NONE : ME_WORD_FAILED;
    }
    void update(Space& home, RankConstView& y) {
      ConstWordView::update(home,y);
      _rank = y._rank;
    }
  };

  template<class View0, class View1>
  struct BoundAliases {
    static bool test(View0, View1) { return false; }
  };
  template<>
  struct BoundAliases<UnsignedWordView,UnsignedWordView> {
    static bool test(UnsignedWordView x, UnsignedWordView y) {
      return x.varimp() == y.varimp();
    }
  };
  template<>
  struct BoundAliases<SignedWordView,SignedWordView> {
    static bool test(SignedWordView x, SignedWordView y) {
      return x.varimp() == y.varimp();
    }
  };
  template<class View0, class View1>
  forceinline bool bound_aliases(View0 x, View1 y) {
    return BoundAliases<View0,View1>::test(x,y);
  }

  template<class View0, class View1>
  forceinline ExecStatus bound_nq_post(Home home, View0 x0, View1 x1) {
    return Nq<View0,View1>::post(home,x0,x1);
  }
  forceinline ExecStatus bound_nq_post(
    Home home, UnsignedWordView x0, UnsignedWordView x1) {
    return (x0.varimp() == x1.varimp()) ? ES_FAILED :
      Nq<UnsignedWordView,UnsignedWordView>::post(home,x0,x1);
  }
  forceinline ExecStatus bound_nq_post(
    Home home, SignedWordView x0, SignedWordView x1) {
    return (x0.varimp() == x1.varimp()) ? ES_FAILED :
      Nq<SignedWordView,SignedWordView>::post(home,x0,x1);
  }

  template<class View0, class View1>
  forceinline Int::RelTest bound_eq_test(View0 x0, View1 x1) {
    if (bound_aliases(x0,x1))
      return Int::RT_TRUE;
    if (disjoint(x0,x1) || (x0.maximum() < x1.minimum()) ||
        (x1.maximum() < x0.minimum()))
      return Int::RT_FALSE;
    if (x0.assigned() && x1.assigned())
      return Int::RT_TRUE;
    return Int::RT_MAYBE;
  }

  template<class View0, class View1>
  ExecStatus narrow_bound_eq(Home home, View0 x0, View1 x1) {
    const WordValue lo = x0.lo() | x1.lo();
    const WordValue hi = x0.hi() & x1.hi();
    const WordValue minimum = std::max(x0.minimum(),x1.minimum());
    const WordValue maximum = std::min(x0.maximum(),x1.maximum());
    if ((lo & ~hi) != 0 || minimum > maximum)
      return ES_FAILED;
    GECODE_ME_CHECK(x0.narrow_domain(home,lo,hi,minimum,maximum));
    GECODE_ME_CHECK(x1.narrow_domain(home,lo,hi,minimum,maximum));
    return ES_OK;
  }

  template<class View0, class View1, bool strict>
  forceinline Int::RelTest bound_lq_test(View0 x0, View1 x1) {
    if (strict ? (x0.maximum() < x1.minimum()) :
                 (x0.maximum() <= x1.minimum()))
      return Int::RT_TRUE;
    if (strict ? (x0.minimum() >= x1.maximum()) :
                 (x0.minimum() > x1.maximum()))
      return Int::RT_FALSE;
    return Int::RT_MAYBE;
  }

  template<class View0, class View1, bool strict>
  ExecStatus narrow_bound_lq(Home home, View0 x0, View1 x1) {
    const WordValue min0 = x0.minimum();
    const WordValue max1 = x1.maximum();
    if (strict ? (min0 >= max1) : (min0 > max1))
      return ES_FAILED;
    const WordValue new_max0 = strict ? max1-1 : max1;
    const WordValue new_min1 = strict ? min0+1 : min0;
    GECODE_ME_CHECK(x0.narrow_range(home,min0,new_max0));
    GECODE_ME_CHECK(x1.narrow_range(home,new_min1,max1));
    return ES_OK;
  }

  template<class View0, class View1>
  forceinline BoundEq<View0,View1>::BoundEq(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM>(home,y0,y1) {}

  template<class View0, class View1>
  forceinline BoundEq<View0,View1>::BoundEq(Space& home, BoundEq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM>(home,p) {}

  template<class View0, class View1>
  ExecStatus BoundEq<View0,View1>::post(Home home, View0 x0, View1 x1) {
    if (bound_aliases(x0,x1))
      return ES_OK;
    GECODE_ES_CHECK(narrow_bound_eq(home,x0,x1));
    if (!x0.assigned() || !x1.assigned())
      (void) new (home) BoundEq(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1>
  Actor* BoundEq<View0,View1>::copy(Space& home) {
    return new (home) BoundEq(home,*this);
  }

  template<class View0, class View1>
  ExecStatus BoundEq<View0,View1>::propagate(Space& home,
                                              const ModEventDelta&) {
    GECODE_ES_CHECK(narrow_bound_eq(home,x0,x1));
    return (x0.assigned() && x1.assigned()) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View0, class View1, bool strict>
  forceinline BoundLq<View0,View1,strict>::BoundLq(
    Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND>(home,y0,y1) {}

  template<class View0, class View1, bool strict>
  forceinline BoundLq<View0,View1,strict>::BoundLq(
    Space& home, BoundLq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND>(home,p) {}

  template<class View0, class View1, bool strict>
  ExecStatus BoundLq<View0,View1,strict>::post(
    Home home, View0 x0, View1 x1) {
    if (bound_aliases(x0,x1))
      return strict ? ES_FAILED : ES_OK;
    GECODE_ES_CHECK((narrow_bound_lq<View0,View1,strict>(home,x0,x1)));
    if (bound_lq_test<View0,View1,strict>(x0,x1) != Int::RT_TRUE)
      (void) new (home) BoundLq(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1, bool strict>
  Actor* BoundLq<View0,View1,strict>::copy(Space& home) {
    return new (home) BoundLq(home,*this);
  }

  template<class View0, class View1, bool strict>
  ExecStatus BoundLq<View0,View1,strict>::propagate(
    Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((narrow_bound_lq<View0,View1,strict>(home,x0,x1)));
    return (bound_lq_test<View0,View1,strict>(x0,x1) == Int::RT_TRUE) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundEq<View0,View1,CtrlView,rm>::ReBoundEq(
    Home home, View0 y0, View1 y1, CtrlView c)
    : Propagator(home), x0(y0), x1(y1), b(c) {
    x0.subscribe(home,*this,PC_WORD_DOM);
    x1.subscribe(home,*this,PC_WORD_DOM);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundEq<View0,View1,CtrlView,rm>::ReBoundEq(
    Space& home, ReBoundEq& p) : Propagator(home,p) {
    x0.update(home,p.x0); x1.update(home,p.x1); b.update(home,p.b);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundEq<View0,View1,CtrlView,rm>::post(
    Home home, View0 x0, View1 x1, CtrlView b) {
    if (b.one()) {
      if (rm == RM_PMI) return ES_OK;
      return BoundEq<View0,View1>::post(home,x0,x1);
    }
    if (b.zero()) {
      if (rm == RM_IMP) return ES_OK;
      return bound_nq_post(home,x0,x1);
    }
    switch (bound_eq_test(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    case Int::RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    case Int::RT_MAYBE:
      (void) new (home) ReBoundEq(home,x0,x1,b);
      return ES_OK;
    default: GECODE_NEVER;
    }
    return ES_FAILED;
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  Actor* ReBoundEq<View0,View1,CtrlView,rm>::copy(Space& home) {
    return new (home) ReBoundEq(home,*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  PropCost ReBoundEq<View0,View1,CtrlView,rm>::cost(
    const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  void ReBoundEq<View0,View1,CtrlView,rm>::reschedule(Space& home) {
    x0.reschedule(home,*this,PC_WORD_DOM);
    x1.reschedule(home,*this,PC_WORD_DOM);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  size_t ReBoundEq<View0,View1,CtrlView,rm>::dispose(Space& home) {
    x0.cancel(home,*this,PC_WORD_DOM);
    x1.cancel(home,*this,PC_WORD_DOM);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundEq<View0,View1,CtrlView,rm>::propagate(
    Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(BoundEq<View0,View1>::post(
        home(*this),x0,x1)));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(bound_nq_post(home(*this),x0,x1)));
    }
    switch (bound_eq_test(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one_none(home));
      break;
    case Int::RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero_none(home));
      break;
    case Int::RT_MAYBE: return ES_FIX;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundLq<View0,View1,CtrlView,rm>::ReBoundLq(
    Home home, View0 y0, View1 y1, CtrlView c)
    : Propagator(home), x0(y0), x1(y1), b(c) {
    x0.subscribe(home,*this,PC_WORD_BND);
    x1.subscribe(home,*this,PC_WORD_BND);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundLq<View0,View1,CtrlView,rm>::ReBoundLq(
    Space& home, ReBoundLq& p) : Propagator(home,p) {
    x0.update(home,p.x0); x1.update(home,p.x1); b.update(home,p.b);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundLq<View0,View1,CtrlView,rm>::post(
    Home home, View0 x0, View1 x1, CtrlView b) {
    if (b.one()) {
      if (rm == RM_PMI) return ES_OK;
      return BoundLq<View0,View1,false>::post(home,x0,x1);
    }
    if (b.zero()) {
      if (rm == RM_IMP) return ES_OK;
      return BoundLq<View1,View0,true>::post(home,x1,x0);
    }
    if (bound_aliases(x0,x1)) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    switch (bound_lq_test<View0,View1,false>(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    case Int::RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    case Int::RT_MAYBE:
      (void) new (home) ReBoundLq(home,x0,x1,b);
      return ES_OK;
    default: GECODE_NEVER;
    }
    return ES_FAILED;
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  Actor* ReBoundLq<View0,View1,CtrlView,rm>::copy(Space& home) {
    return new (home) ReBoundLq(home,*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  PropCost ReBoundLq<View0,View1,CtrlView,rm>::cost(
    const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  void ReBoundLq<View0,View1,CtrlView,rm>::reschedule(Space& home) {
    x0.reschedule(home,*this,PC_WORD_BND);
    x1.reschedule(home,*this,PC_WORD_BND);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  size_t ReBoundLq<View0,View1,CtrlView,rm>::dispose(Space& home) {
    x0.cancel(home,*this,PC_WORD_BND);
    x1.cancel(home,*this,PC_WORD_BND);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundLq<View0,View1,CtrlView,rm>::propagate(
    Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(BoundLq<View0,View1,false>::post(
        home(*this),x0,x1)));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(BoundLq<View1,View0,true>::post(
        home(*this),x1,x0)));
    }
    switch (bound_lq_test<View0,View1,false>(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one_none(home));
      break;
    case Int::RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero_none(home));
      break;
    case Int::RT_MAYBE: return ES_FIX;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

#endif

// STATISTICS: word-prop
