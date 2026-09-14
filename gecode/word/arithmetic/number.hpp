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


#include <gecode/word/rel.hh>
#include <gecode/word/arithmetic/gcd-filter.hpp>
#include <gecode/word/arithmetic/divides-filter.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Match the membership read by both ordinary and reified GCD filters.
  forceinline PropCond
  select_gcd_membership_condition(WordView view) {
    return view.domain_type() == WDT_CUBE ? PC_WORD_BITS : PC_WORD_DOM;
  }

  template<bool is_signed>
  forceinline
  Gcd<is_signed>::Gcd(Home home, WordView x, WordView y, WordView result)
    : TernaryPropagator<WordView,PC_GEN_NONE>(home,x,y,result) {
    x0.subscribe(home,*this,select_gcd_membership_condition(x0));
    x1.subscribe(home,*this,select_gcd_membership_condition(x1));
    x2.subscribe(home,*this,select_gcd_membership_condition(x2));
  }

  template<bool is_signed>
  forceinline
  Gcd<is_signed>::Gcd(Space& home, Gcd& p)
    : TernaryPropagator<WordView,PC_GEN_NONE>(home,p) {}

  template<bool is_signed>
  ExecStatus
  Gcd<is_signed>::post(Home home, WordView x, WordView y, WordView result) {
    const NumberFilterResult filtered=filter_gcd_cube<is_signed>(home,x,y,result);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    if (filtered == NumberFilterResult::active)
      (void) new (home) Gcd(home,x,y,result);
    return ES_OK;
  }

  template<bool is_signed>
  Actor*
  Gcd<is_signed>::copy(Space& home) { return new (home) Gcd(home,*this); }

  template<bool is_signed>
  PropCost
  Gcd<is_signed>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<bool is_signed>
  void
  Gcd<is_signed>::reschedule(Space& home) {
    x0.reschedule(home,*this,select_gcd_membership_condition(x0));
    x1.reschedule(home,*this,select_gcd_membership_condition(x1));
    x2.reschedule(home,*this,select_gcd_membership_condition(x2));
  }

  template<bool is_signed>
  size_t
  Gcd<is_signed>::dispose(Space& home) {
    x0.cancel(home,*this,select_gcd_membership_condition(x0));
    x1.cancel(home,*this,select_gcd_membership_condition(x1));
    x2.cancel(home,*this,select_gcd_membership_condition(x2));
    (void) TernaryPropagator<WordView,PC_GEN_NONE>::dispose(home);
    return sizeof(*this);
  }

  template<bool is_signed>
  ExecStatus
  Gcd<is_signed>::propagate(Space& home, const ModEventDelta&) {
    const NumberFilterResult filtered=filter_gcd_cube<is_signed>(home,x0,x1,x2);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    return (filtered == NumberFilterResult::entailed) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View, bool is_signed>
  forceinline
  BoundGcd<View,is_signed>::BoundGcd(Home home, View x0, View y0,
                                UnsignedWordView result0)
    : Propagator(home), x(x0), y(y0), result(result0) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_WORD_DOM);
    y.subscribe(home,*this,PC_WORD_DOM);
    result.subscribe(home,*this,PC_WORD_DOM);
  }

  template<class View, bool is_signed>
  forceinline
  BoundGcd<View,is_signed>::BoundGcd(Space& home, BoundGcd& p)
    : Propagator(home,p) {
    x.update(home,p.x); y.update(home,p.y); result.update(home,p.result);
  }

  /// Local GCD domains retain their storage kinds and first representative.
  struct BoundGcdState {
    BoundLocalDomain domains[3];
    unsigned int representatives[3];
  };

  template<class View>
  forceinline BoundGcdState
  snapshot_gcd_roles(View x, View y, UnsignedWordView result) {
    BoundGcdState state={{snapshot_bound_domain(x),snapshot_bound_domain(y),
                         snapshot_bound_domain(result)},{0,1,2}};
    if (x.varimp() == y.varimp()) state.representatives[1]=0;
    if (x.varimp() == result.varimp()) state.representatives[2]=0;
    else if (y.varimp() == result.varimp())
      state.representatives[2]=state.representatives[1];
    return state;
  }

  /// Publish each distinct typed role in order, stopping at the first failure.
  template<class View>
  forceinline ExecStatus
  publish_gcd_roles(Home home, View x, View y, UnsignedWordView result,
                    const BoundGcdState& state) {
    if (publish_bound_domain(home,x,state.domains[0]) == ES_FAILED)
      return ES_FAILED;
    if (state.representatives[1] == 1U) {
      if (publish_bound_domain(home,y,state.domains[1]) == ES_FAILED)
        return ES_FAILED;
    }
    if (state.representatives[2] != 2U) return ES_OK;
    return publish_bound_domain(home,result,state.domains[2]);
  }

  template<class View, bool is_signed>
  NumberFilterResult
  BoundGcd<View,is_signed>::narrow(Home home, View x, View y,
                                  UnsignedWordView result) {
    BoundGcdState state=snapshot_gcd_roles(x,y,result);
    NumberFilterResult filtered=close_gcd_mandatory<is_signed>(
      state.domains,state.representatives);
    if (filtered == NumberFilterResult::failed) return filtered;
    filtered=try_gcd_optional<is_signed>(
      state.domains,state.representatives,filtered);
    if (filtered == NumberFilterResult::failed) return filtered;
    if (publish_gcd_roles(home,x,y,result,state) == ES_FAILED)
      return NumberFilterResult::failed;
    return filtered;
  }

  template<class View, bool is_signed>
  ExecStatus
  BoundGcd<View,is_signed>::post(Home home, View x, View y,
                            UnsignedWordView result) {
    const NumberFilterResult filtered=narrow(home,x,y,result);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    if (filtered == NumberFilterResult::active)
      (void) new (home) BoundGcd(home,x,y,result);
    return ES_OK;
  }

  template<class View, bool is_signed>
  Actor*
  BoundGcd<View,is_signed>::copy(Space& home) {
    return new (home) BoundGcd(home,*this);
  }

  template<class View, bool is_signed>
  PropCost
  BoundGcd<View,is_signed>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.width());
  }

  template<class View, bool is_signed>
  void
  BoundGcd<View,is_signed>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_DOM);
    y.reschedule(home,*this,PC_WORD_DOM);
    result.reschedule(home,*this,PC_WORD_DOM);
  }

  template<class View, bool is_signed>
  size_t
  BoundGcd<View,is_signed>::dispose(Space& home) {
    home.ignore(*this,AP_WEAKLY);
    x.cancel(home,*this,PC_WORD_DOM);
    y.cancel(home,*this,PC_WORD_DOM);
    result.cancel(home,*this,PC_WORD_DOM);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View, bool is_signed>
  ExecStatus
  BoundGcd<View,is_signed>::propagate(Space& home, const ModEventDelta&) {
    const NumberFilterResult filtered=narrow(home,x,y,result);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    return (filtered == NumberFilterResult::entailed) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<bool is_signed>
  ExecStatus
  post_gcd(Home home, WordView x, WordView y, WordView result) {
    const bool can_use_unsigned_gcd=!is_signed &&
      (x.domain_type() == WDT_UNSIGNED) &&
      (y.domain_type() == WDT_UNSIGNED) &&
      (result.domain_type() == WDT_UNSIGNED);
    if (can_use_unsigned_gcd)
      return BoundGcd<UnsignedWordView,false>::post(
        home,UnsignedWordView(x.varimp()),UnsignedWordView(y.varimp()),
        UnsignedWordView(result.varimp()));
    const bool can_use_signed_gcd=is_signed &&
      (x.domain_type() == WDT_SIGNED) &&
      (y.domain_type() == WDT_SIGNED) &&
      (result.domain_type() == WDT_UNSIGNED);
    if (can_use_signed_gcd)
      return BoundGcd<SignedWordView,true>::post(
        home,SignedWordView(x.varimp()),SignedWordView(y.varimp()),
        UnsignedWordView(result.varimp()));
    return Gcd<is_signed>::post(home,x,y,result);
  }

  template<ReifyMode rm, bool is_signed>
  forceinline
  ReGcd<rm,is_signed>::ReGcd(Home home, WordView x0, WordView y0,
                        WordView result0, Int::BoolView b0)
    : Propagator(home), x(x0), y(y0), result(result0), b(b0) {
    x.subscribe(home,*this,select_gcd_membership_condition(x));
    y.subscribe(home,*this,select_gcd_membership_condition(y));
    result.subscribe(home,*this,select_gcd_membership_condition(result));
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool is_signed>
  ExecStatus
  ReGcd<rm,is_signed>::post(Home home, WordView x, WordView y, WordView result,
                       Int::BoolView b) {
    if (b.one()) return (rm == RM_PMI) ? ES_OK :
      post_gcd<is_signed>(home,x,y,result);
    const bool is_inactive_implication=b.zero() && (rm == RM_IMP);
    if (is_inactive_implication) return ES_OK;
    const Int::RelTest rt=test_gcd<is_signed>(x,y,result);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    }
    (void) new (home) ReGcd(home,x,y,result,b);
    return ES_OK;
  }

  template<ReifyMode rm, bool is_signed>
  forceinline
  ReGcd<rm,is_signed>::ReGcd(Space& home, ReGcd& p) : Propagator(home,p) {
    x.update(home,p.x); y.update(home,p.y); result.update(home,p.result);
    b.update(home,p.b);
  }

  template<ReifyMode rm, bool is_signed>
  Actor*
  ReGcd<rm,is_signed>::copy(Space& home) { return new (home) ReGcd(home,*this); }

  template<ReifyMode rm, bool is_signed>
  PropCost
  ReGcd<rm,is_signed>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<ReifyMode rm, bool is_signed>
  void
  ReGcd<rm,is_signed>::reschedule(Space& home) {
    x.reschedule(home,*this,select_gcd_membership_condition(x));
    y.reschedule(home,*this,select_gcd_membership_condition(y));
    result.reschedule(home,*this,select_gcd_membership_condition(result));
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool is_signed>
  size_t
  ReGcd<rm,is_signed>::dispose(Space& home) {
    x.cancel(home,*this,select_gcd_membership_condition(x));
    y.cancel(home,*this,select_gcd_membership_condition(y));
    result.cancel(home,*this,select_gcd_membership_condition(result));
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home); return sizeof(*this);
  }

  template<ReifyMode rm, bool is_signed>
  ExecStatus
  ReGcd<rm,is_signed>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,post_gcd<is_signed>(home(*this),x,y,result));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      const Int::RelTest rt=test_gcd<is_signed>(x,y,result);
      if (rt == Int::RT_TRUE) return ES_FAILED;
      return (rt == Int::RT_FALSE) ? home.ES_SUBSUMED(*this) : ES_FIX;
    }
    const Int::RelTest rt=test_gcd<is_signed>(x,y,result);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return home.ES_SUBSUMED(*this);
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

  template<bool is_signed>
  forceinline
  CubeDivides<is_signed>::CubeDivides(Home home, WordView divisor,
                                 WordView dividend)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,divisor,dividend) {}

  template<bool is_signed>
  forceinline
  CubeDivides<is_signed>::CubeDivides(Space& home, CubeDivides& p)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  template<bool is_signed>
  ExecStatus
  CubeDivides<is_signed>::post(Home home, WordView divisor, WordView dividend) {
    const NumberFilterResult filtered=filter_divides_cube<is_signed>(
      home,divisor,dividend);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    if (filtered == NumberFilterResult::active)
      (void) new (home) CubeDivides(home,divisor,dividend);
    return ES_OK;
  }

  template<bool is_signed>
  Actor*
  CubeDivides<is_signed>::copy(Space& home) {
    return new (home) CubeDivides(home,*this);
  }

  template<bool is_signed>
  PropCost
  CubeDivides<is_signed>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<bool is_signed>
  ExecStatus
  CubeDivides<is_signed>::propagate(Space& home, const ModEventDelta&) {
    const NumberFilterResult filtered=filter_divides_cube<is_signed>(home,x0,x1);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    return (filtered == NumberFilterResult::entailed) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View, bool is_signed>
  forceinline
  Divides<View,is_signed>::Divides(Home home, View divisor, View dividend)
    : BinaryPropagator<View,PC_WORD_DOM>(home,divisor,dividend) {
    home.notice(*this,AP_WEAKLY);
  }

  template<class View, bool is_signed>
  forceinline
  Divides<View,is_signed>::Divides(Space& home, Divides& p)
    : BinaryPropagator<View,PC_WORD_DOM>(home,p) {}

  template<class View, bool is_signed>
  NumberFilterResult
  Divides<View,is_signed>::narrow(Home home, View divisor, View dividend) {
    BoundLocalDomain domains[2]={snapshot_bound_domain(divisor),
                                 snapshot_bound_domain(dividend)};
    const bool is_aliased=divisor.varimp() == dividend.varimp();
    const NumberFilterResult filtered=close_divides<is_signed>(domains,is_aliased);
    if (filtered == NumberFilterResult::failed) return filtered;
    if (publish_bound_domain(home,divisor,domains[0]) == ES_FAILED)
      return NumberFilterResult::failed;
    if (is_aliased) return filtered;
    if (publish_bound_domain(home,dividend,domains[1]) == ES_FAILED)
      return NumberFilterResult::failed;
    return filtered;
  }

  template<class View, bool is_signed>
  ExecStatus
  Divides<View,is_signed>::post(Home home, View divisor, View dividend) {
    const NumberFilterResult filtered=narrow(home,divisor,dividend);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    if (filtered == NumberFilterResult::active)
      (void) new (home) Divides(home,divisor,dividend);
    return ES_OK;
  }

  template<class View, bool is_signed>
  Actor*
  Divides<View,is_signed>::copy(Space& home) {
    return new (home) Divides(home,*this);
  }

  template<class View, bool is_signed>
  size_t
  Divides<View,is_signed>::dispose(Space& home) {
    home.ignore(*this,AP_WEAKLY);
    (void) BinaryPropagator<View,PC_WORD_DOM>::dispose(home);
    return sizeof(*this);
  }

  template<class View, bool is_signed>
  PropCost
  Divides<View,is_signed>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x0.width());
  }

  template<class View, bool is_signed>
  ExecStatus
  Divides<View,is_signed>::propagate(Space& home, const ModEventDelta&) {
    const NumberFilterResult filtered=narrow(home,x0,x1);
    if (filtered == NumberFilterResult::failed) return ES_FAILED;
    return (filtered == NumberFilterResult::entailed) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<bool is_signed>
  ExecStatus
  post_divides(Home home, WordView divisor, WordView dividend) {
    const WordDomainType kind=is_signed ? WDT_SIGNED : WDT_UNSIGNED;
    const bool can_use_bounded_divides=(divisor.domain_type() == kind) &&
      (dividend.domain_type() == kind);
    if (can_use_bounded_divides) {
      if (is_signed)
        return Divides<SignedWordView,true>::post(
          home,SignedWordView(divisor.varimp()),
          SignedWordView(dividend.varimp()));
      return Divides<UnsignedWordView,false>::post(
        home,UnsignedWordView(divisor.varimp()),
        UnsignedWordView(dividend.varimp()));
    }
    return CubeDivides<is_signed>::post(home,divisor,dividend);
  }

  template<ReifyMode rm, bool is_signed>
  forceinline
  ReDivides<rm,is_signed>::ReDivides(Home home, WordView divisor0,
                                WordView dividend0, Int::BoolView b0)
    : Propagator(home), divisor(divisor0), dividend(dividend0), b(b0) {
    divisor.subscribe(home,*this,PC_WORD_BITS);
    dividend.subscribe(home,*this,
      dividend.bounded() ? PC_WORD_DOM : PC_WORD_BITS);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool is_signed>
  ExecStatus
  ReDivides<rm,is_signed>::post(Home home, WordView divisor, WordView dividend,
                           Int::BoolView b) {
    if (b.one()) return (rm == RM_PMI) ? ES_OK :
      post_divides<is_signed>(home,divisor,dividend);
    if (b.zero()) {
      if (rm == RM_IMP) return ES_OK;
      const bool is_zero_divisor=divisor.assigned() && (divisor.val() == 0U);
      if (is_zero_divisor)
        return Rel::Nq<WordView,WordView>::post(home,divisor,dividend);
    }
    const Int::RelTest rt=test_divides<is_signed>(divisor,dividend);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    }
    (void) new (home) ReDivides(home,divisor,dividend,b);
    return ES_OK;
  }

  template<ReifyMode rm, bool is_signed>
  forceinline
  ReDivides<rm,is_signed>::ReDivides(Space& home, ReDivides& p)
    : Propagator(home,p) {
    divisor.update(home,p.divisor); dividend.update(home,p.dividend);
    b.update(home,p.b);
  }

  template<ReifyMode rm, bool is_signed>
  Actor*
  ReDivides<rm,is_signed>::copy(Space& home) {
    return new (home) ReDivides(home,*this);
  }

  template<ReifyMode rm, bool is_signed>
  PropCost
  ReDivides<rm,is_signed>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<ReifyMode rm, bool is_signed>
  void
  ReDivides<rm,is_signed>::reschedule(Space& home) {
    divisor.reschedule(home,*this,PC_WORD_BITS);
    dividend.reschedule(home,*this,
      dividend.bounded() ? PC_WORD_DOM : PC_WORD_BITS);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool is_signed>
  size_t
  ReDivides<rm,is_signed>::dispose(Space& home) {
    divisor.cancel(home,*this,PC_WORD_BITS);
    dividend.cancel(home,*this,
      dividend.bounded() ? PC_WORD_DOM : PC_WORD_BITS);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home); return sizeof(*this);
  }

  template<ReifyMode rm, bool is_signed>
  ExecStatus
  ReDivides<rm,is_signed>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,post_divides<is_signed>(home(*this),divisor,dividend));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      const bool is_zero_divisor=divisor.assigned() && (divisor.val() == 0U);
      if (is_zero_divisor)
        GECODE_REWRITE(*this,(Rel::Nq<WordView,WordView>::post(
          home(*this),divisor,dividend)));
      const Int::RelTest rt=test_divides<is_signed>(divisor,dividend);
      if (rt == Int::RT_TRUE) return ES_FAILED;
      return (rt == Int::RT_FALSE) ? home.ES_SUBSUMED(*this) : ES_FIX;
    }
    const Int::RelTest rt=test_divides<is_signed>(divisor,dividend);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return home.ES_SUBSUMED(*this);
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
