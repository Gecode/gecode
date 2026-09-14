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
  template<bool is_signed>
  class RankedWordConstView : public ConstWordView {
  public:
    RankedWordConstView(void) : _rank(0) {}
    RankedWordConstView(unsigned int width, WordValue value)
      : ConstWordView(width,value),
        _rank(Word::rank(is_signed ? WDT_SIGNED : WDT_UNSIGNED,width,value)) {}
    WordValue rank_minimum(void) const { return _rank; }
    WordValue rank_maximum(void) const { return _rank; }
    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum) {
      const ModEvent me = narrow(home,lo,hi);
      if (me_failed(me))
        return me;
      const bool is_in_range=(_rank >= minimum) && (_rank <= maximum);
      return is_in_range ? ME_WORD_NONE : ME_WORD_FAILED;
    }
    ModEvent narrow_range(Space&, WordValue minimum, WordValue maximum) {
      const bool is_in_range=(_rank >= minimum) && (_rank <= maximum);
      return is_in_range ? ME_WORD_NONE : ME_WORD_FAILED;
    }
    void update(Space& home, RankedWordConstView& y) {
      ConstWordView::update(home,y);
      _rank = y._rank;
    }
  private:
    WordValue _rank;
  };

  /// Recognize aliasing only for matching bounded variable view types
  template<class View0, class View1>
  struct BoundedWordAliasTest {
    static bool test(View0, View1) { return false; }
  };
  /// Compare unsigned bounded variable identities
  template<>
  struct BoundedWordAliasTest<UnsignedWordView,UnsignedWordView> {
    static bool test(UnsignedWordView x, UnsignedWordView y) {
      return x.varimp() == y.varimp();
    }
  };
  /// Compare signed bounded variable identities
  template<>
  struct BoundedWordAliasTest<SignedWordView,SignedWordView> {
    static bool test(SignedWordView x, SignedWordView y) {
      return x.varimp() == y.varimp();
    }
  };
  template<class View0, class View1>
  forceinline bool bound_aliases(View0 x, View1 y) {
    return BoundedWordAliasTest<View0,View1>::test(x,y);
  }

  /// Use the bit actor unless both operands have one bounded view type
  template<class View0, class View1>
  forceinline ExecStatus post_bounded_disequality(
    Home home, View0 x0, View1 x1) {
    return Nq<View0,View1>::post(home,x0,x1);
  }
  forceinline ExecStatus post_bounded_disequality(
    Home home, UnsignedWordView x0, UnsignedWordView x1) {
    return BoundNq<UnsignedWordView>::post(home,x0,x1);
  }
  forceinline ExecStatus post_bounded_disequality(
    Home home, SignedWordView x0, SignedWordView x1) {
    return BoundNq<SignedWordView>::post(home,x0,x1);
  }

  /// Test mask and interval separation in one compatible rank order
  template<class View0, class View1>
  forceinline bool are_bounded_domains_disjoint(View0 x0, View1 x1) {
    return disjoint(x0,x1) ||
      (x0.rank_maximum() < x1.rank_minimum()) ||
      (x1.rank_maximum() < x0.rank_minimum());
  }

  /// Classify equality after the bounded-variable alias shortcut
  template<class View0, class View1>
  forceinline Int::RelTest bound_eq_test(View0 x0, View1 x1) {
    if (bound_aliases(x0,x1))
      return Int::RT_TRUE;
    if (are_bounded_domains_disjoint(x0,x1))
      return Int::RT_FALSE;
    const bool are_assigned=x0.assigned() && x1.assigned();
    if (are_assigned)
      return Int::RT_TRUE;
    return Int::RT_MAYBE;
  }

  /// Masks and numeric endpoints in a bounded view's rank order
  struct BoundedRelationDomain {
    const WordValue lo;
    const WordValue hi;
    const WordValue minimum;
    const WordValue maximum;
  };

  template<class View>
  forceinline BoundedRelationDomain capture_bounded_domain(View x) {
    return {x.lo(),x.hi(),x.rank_minimum(),x.rank_maximum()};
  }

  /// A preliminary intersection; native synchronization can still reject it
  struct BoundedEqualityResult {
    const BoundedRelationDomain domain;
    const bool has_preliminary_conflict;
  };

  /// Intersect masks and endpoints without proving synchronized feasibility
  forceinline BoundedEqualityResult intersect_equality_domains(
    const BoundedRelationDomain& left, const BoundedRelationDomain& right) {
    const BoundedRelationDomain domain={
      left.lo | right.lo,left.hi & right.hi,
      std::max(left.minimum,right.minimum),
      std::min(left.maximum,right.maximum)};
    const bool has_preliminary_conflict=
      ((domain.lo & ~domain.hi) != 0) || (domain.minimum > domain.maximum);
    return {domain,has_preliminary_conflict};
  }

  /// Publish one shared candidate to views with compatible rank orders
  template<class View0, class View1>
  ExecStatus narrow_bound_eq(Home home, View0 x0, View1 x1) {
    const BoundedRelationDomain left=capture_bounded_domain(x0);
    const BoundedRelationDomain right=capture_bounded_domain(x1);
    const BoundedEqualityResult result=intersect_equality_domains(left,right);
    if (result.has_preliminary_conflict)
      return ES_FAILED;
    const BoundedRelationDomain& domain=result.domain;
    GECODE_ME_CHECK(x0.narrow_domain(
      home,domain.lo,domain.hi,domain.minimum,domain.maximum));
    GECODE_ME_CHECK(x1.narrow_domain(
      home,domain.lo,domain.hi,domain.minimum,domain.maximum));
    return ES_OK;
  }

  /// Classify ordering for endpoints expressed in one compatible rank order
  template<class View0, class View1, bool is_strict>
  forceinline Int::RelTest test_bounded_ordering(View0 x0, View1 x1) {
    const bool is_entailed=is_strict ?
      (x0.rank_maximum() < x1.rank_minimum()) :
      (x0.rank_maximum() <= x1.rank_minimum());
    if (is_entailed)
      return Int::RT_TRUE;
    const bool is_contradicted=is_strict ?
      (x0.rank_minimum() >= x1.rank_maximum()) :
      (x0.rank_minimum() > x1.rank_maximum());
    if (is_contradicted)
      return Int::RT_FALSE;
    return Int::RT_MAYBE;
  }

  /// Compatible rank ranges produced by one ordering calculation
  struct BoundedOrderingResult {
    const WordValue left_minimum;
    const WordValue left_maximum;
    const WordValue right_minimum;
    const WordValue right_maximum;
    const bool has_conflict;
  };

  /// Check strict feasibility before computing adjacent rank endpoints
  template<bool is_strict>
  forceinline BoundedOrderingResult
  compute_ordering_ranges(WordValue left_lower, WordValue right_upper) {
    const bool has_conflict=is_strict ?
      (left_lower >= right_upper) : (left_lower > right_upper);
    if (has_conflict)
      return {left_lower,right_upper,left_lower,right_upper,true};
    const WordValue left_upper=is_strict ? right_upper-1 : right_upper;
    const WordValue right_lower=is_strict ? left_lower+1 : left_lower;
    return {left_lower,left_upper,right_lower,right_upper,false};
  }

  /// Publish precomputed endpoints in the operands' shared rank order
  template<class View0, class View1, bool is_strict>
  ExecStatus narrow_bound_lq(Home home, View0 x0, View1 x1) {
    const WordValue left_lower=x0.rank_minimum();
    const WordValue right_upper=x1.rank_maximum();
    const BoundedOrderingResult result=
      compute_ordering_ranges<is_strict>(left_lower,right_upper);
    if (result.has_conflict)
      return ES_FAILED;
    GECODE_ME_CHECK(x0.narrow_range(
      home,result.left_minimum,result.left_maximum));
    GECODE_ME_CHECK(x1.narrow_range(
      home,result.right_minimum,result.right_maximum));
    return ES_OK;
  }

  template<class View0, class View1>
  forceinline BoundEq<View0,View1>::
  BoundEq(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM>(home,y0,y1) {}

  template<class View0, class View1>
  forceinline BoundEq<View0,View1>::
  BoundEq(Space& home, BoundEq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM>(home,p) {}

  template<class View0, class View1>
  ExecStatus BoundEq<View0,View1>::post(Home home, View0 x0, View1 x1) {
    if (bound_aliases(x0,x1))
      return ES_OK;
    GECODE_ES_CHECK(narrow_bound_eq(home,x0,x1));
    const bool needs_actor=!x0.assigned() || !x1.assigned();
    if (needs_actor)
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
    const bool are_assigned=x0.assigned() && x1.assigned();
    return are_assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View>
  forceinline BoundNq<View>::BoundNq(Home home, View y0, View y1)
    : MixBinaryPropagator<
        View,PC_WORD_DOM,View,PC_WORD_DOM>(home,y0,y1) {}

  template<class View>
  forceinline BoundNq<View>::BoundNq(Space& home, BoundNq& p)
    : MixBinaryPropagator<
        View,PC_WORD_DOM,View,PC_WORD_DOM>(home,p) {}

  template<class View>
  ExecStatus BoundNq<View>::post(Home home, View x0, View x1) {
    if (x0.varimp() == x1.varimp())
      return ES_FAILED;
    if (are_bounded_domains_disjoint(x0,x1))
      return ES_OK;
    if (x0.assigned()) {
      GECODE_ES_CHECK(prune_disequality(home,x1,x0.val()));
      if (!x1.in(x0.val()))
        return ES_OK;
    } else if (x1.assigned()) {
      GECODE_ES_CHECK(prune_disequality(home,x0,x1.val()));
      if (!x0.in(x1.val()))
        return ES_OK;
    }
    (void) new (home) BoundNq(home,x0,x1);
    return ES_OK;
  }

  template<class View>
  Actor* BoundNq<View>::copy(Space& home) {
    return new (home) BoundNq(home,*this);
  }

  template<class View>
  ExecStatus BoundNq<View>::propagate(Space& home,
                                      const ModEventDelta&) {
    if (are_bounded_domains_disjoint(x0,x1))
      return home.ES_SUBSUMED(*this);
    if (x0.assigned()) {
      GECODE_ES_CHECK(prune_disequality(home,x1,x0.val()));
      return x1.in(x0.val()) ? ES_FIX : home.ES_SUBSUMED(*this);
    }
    if (x1.assigned()) {
      GECODE_ES_CHECK(prune_disequality(home,x0,x1.val()));
      return x0.in(x1.val()) ? ES_FIX : home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

  template<class View0, class View1, bool is_strict>
  forceinline BoundLq<View0,View1,is_strict>::BoundLq(
    Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND>(home,y0,y1) {}

  template<class View0, class View1, bool is_strict>
  forceinline BoundLq<View0,View1,is_strict>::BoundLq(
    Space& home, BoundLq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND>(home,p) {}

  template<class View0, class View1, bool is_strict>
  ExecStatus BoundLq<View0,View1,is_strict>::post(
    Home home, View0 x0, View1 x1) {
    if (bound_aliases(x0,x1))
      return is_strict ? ES_FAILED : ES_OK;
    GECODE_ES_CHECK((narrow_bound_lq<View0,View1,is_strict>(home,x0,x1)));
    if (test_bounded_ordering<View0,View1,is_strict>(x0,x1) != Int::RT_TRUE)
      (void) new (home) BoundLq(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1, bool is_strict>
  Actor* BoundLq<View0,View1,is_strict>::copy(Space& home) {
    return new (home) BoundLq(home,*this);
  }

  template<class View0, class View1, bool is_strict>
  ExecStatus BoundLq<View0,View1,is_strict>::propagate(
    Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((narrow_bound_lq<View0,View1,is_strict>(home,x0,x1)));
    const bool is_entailed=
      test_bounded_ordering<View0,View1,is_strict>(x0,x1) == Int::RT_TRUE;
    return is_entailed ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundEq<View0,View1,CtrlView,rm>::
  ReBoundEq(Home home, View0 y0, View1 y1, CtrlView c)
    : MixTernaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM,
        CtrlView,Int::PC_BOOL_VAL>(home,y0,y1,c) {}

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundEq<View0,View1,CtrlView,rm>::
  ReBoundEq(Space& home, ReBoundEq& p)
    : MixTernaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM,
        CtrlView,Int::PC_BOOL_VAL>(home,p) {}

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundEq<View0,View1,CtrlView,rm>::post(
    Home home, View0 x0, View1 x1, CtrlView b) {
    if (b.one()) {
      if (rm == RM_PMI) return ES_OK;
      return BoundEq<View0,View1>::post(home,x0,x1);
    }
    if (b.zero()) {
      if (rm == RM_IMP) return ES_OK;
      return post_bounded_disequality(home,x0,x1);
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
  size_t ReBoundEq<View0,View1,CtrlView,rm>::dispose(Space& home) {
    (void) MixTernaryPropagator<
        View0,PC_WORD_DOM,View1,PC_WORD_DOM,
        CtrlView,Int::PC_BOOL_VAL>::dispose(home);
    return sizeof(*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundEq<View0,View1,CtrlView,rm>::propagate(
    Space& home, const ModEventDelta&) {
    if (x2.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(BoundEq<View0,View1>::post(
        home(*this),x0,x1)));
    }
    if (x2.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(post_bounded_disequality(home(*this),x0,x1)));
    }
    switch (bound_eq_test(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(x2.one_none(home));
      break;
    case Int::RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(x2.zero_none(home));
      break;
    case Int::RT_MAYBE: return ES_FIX;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundLq<View0,View1,CtrlView,rm>::
  ReBoundLq(Home home, View0 y0, View1 y1, CtrlView c)
    : MixTernaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND,
        CtrlView,Int::PC_BOOL_VAL>(home,y0,y1,c) {}

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  forceinline ReBoundLq<View0,View1,CtrlView,rm>::
  ReBoundLq(Space& home, ReBoundLq& p)
    : MixTernaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND,
        CtrlView,Int::PC_BOOL_VAL>(home,p) {}

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
    switch (test_bounded_ordering<View0,View1,false>(x0,x1)) {
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
  size_t ReBoundLq<View0,View1,CtrlView,rm>::dispose(Space& home) {
    (void) MixTernaryPropagator<
        View0,PC_WORD_BND,View1,PC_WORD_BND,
        CtrlView,Int::PC_BOOL_VAL>::dispose(home);
    return sizeof(*this);
  }

  template<class View0, class View1, class CtrlView, ReifyMode rm>
  ExecStatus ReBoundLq<View0,View1,CtrlView,rm>::propagate(
    Space& home, const ModEventDelta&) {
    if (x2.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(BoundLq<View0,View1,false>::post(
        home(*this),x0,x1)));
    }
    if (x2.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(BoundLq<View1,View0,true>::post(
        home(*this),x1,x0)));
    }
    switch (test_bounded_ordering<View0,View1,false>(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP) GECODE_ME_CHECK(x2.one_none(home));
      break;
    case Int::RT_FALSE:
      if (rm != RM_PMI) GECODE_ME_CHECK(x2.zero_none(home));
      break;
    case Int::RT_MAYBE: return ES_FIX;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

#endif

// STATISTICS: word-prop
