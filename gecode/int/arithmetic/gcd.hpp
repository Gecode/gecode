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
 *
 */

#include <algorithm>

namespace Gecode { namespace Int { namespace Arithmetic {

  forceinline int
  gcd_value(int a, int b) {
    unsigned int x = static_cast<unsigned int>((a < 0) ? -a : a);
    unsigned int y = static_cast<unsigned int>((b < 0) ? -b : b);
    while (y != 0U) {
      unsigned int t = x % y;
      x = y; y = t;
    }
    return static_cast<int>(x);
  }

  forceinline int
  gcd_max_abs(const IntView& x) {
    return std::max((x.min() < 0) ? -x.min() : x.min(),
                    (x.max() < 0) ? -x.max() : x.max());
  }

  forceinline bool
  gcd_excludes_zero_bnd(const IntView& x) {
    return (x.min() > 0) || (x.max() < 0);
  }

  /// Upper bound on a gcd from the operand bounds.
  forceinline int
  gcd_upper(const IntView& x0, const IntView& x1) {
    const int a0=gcd_max_abs(x0);
    const int a1=gcd_max_abs(x1);
    int u=std::max(a0,a1);
    if (gcd_excludes_zero_bnd(x0)) u=std::min(u,a0);
    if (gcd_excludes_zero_bnd(x1)) u=std::min(u,a1);
    return u;
  }

  /// Tighten an operand to the extreme multiples of a positive gcd.
  inline ExecStatus
  gcd_multiple_bounds(Home home, IntView x, int g) {
    assert(g > 0);
    const long long int l =
      ceil_div_xx(static_cast<long long int>(x.min()),
                  static_cast<long long int>(g)) * g;
    const long long int u =
      floor_div_xx(static_cast<long long int>(x.max()),
                   static_cast<long long int>(g)) * g;
    if (l > u)
      return ES_FAILED;
    GECODE_ME_CHECK(x.gq(home,static_cast<int>(l)));
    GECODE_ME_CHECK(x.lq(home,static_cast<int>(u)));
    return ES_OK;
  }

  /// Status of abs(x0)=x1 using assignments and interval bounds only.
  inline RelTest
  gcd_abs_status(const IntView& x0, const IntView& x1) {
    if (x1.max() < 0)
      return RT_FALSE;
    const int l = ((x0.min() <= 0) && (x0.max() >= 0)) ? 0 :
      std::min((x0.min() < 0) ? -x0.min() : x0.min(),
               (x0.max() < 0) ? -x0.max() : x0.max());
    const int u=gcd_max_abs(x0);
    if ((x1.max() < l) || (x1.min() > u))
      return RT_FALSE;
    if (x0 == x1) {
      if (x0.min() >= 0)
        return RT_TRUE;
      if (x0.max() < 0)
        return RT_FALSE;
    }
    if (x0.assigned()) {
      const int a=(x0.val() < 0) ? -x0.val() : x0.val();
      if (!x1.in(a))
        return RT_FALSE;
      return x1.assigned() ? RT_TRUE : RT_MAYBE;
    }
    if (x1.assigned() && (x1.val() == 0) && !x0.in(0))
      return RT_FALSE;
    return RT_MAYBE;
  }

  inline RelTest
  gcd_status(const IntView& x0, const IntView& x1, const IntView& x2) {
    if (x2.max() < 0)
      return RT_FALSE;
    if (x2.min() > gcd_upper(x0,x1))
      return RT_FALSE;
    if (x0 == x1)
      return gcd_abs_status(x0,x2);
    if (x0.assigned() && (x0.val() == 0))
      return gcd_abs_status(x1,x2);
    if (x1.assigned() && (x1.val() == 0))
      return gcd_abs_status(x0,x2);
    if ((x0.assigned() && ((x0.val() == 1) || (x0.val() == -1))) ||
        (x1.assigned() && ((x1.val() == 1) || (x1.val() == -1)))) {
      if (!x2.in(1)) return RT_FALSE;
      return x2.assigned() ? RT_TRUE : RT_MAYBE;
    }
    if (x2.assigned()) {
      const int g=x2.val();
      if (g == 0) {
        if (!x0.in(0) || !x1.in(0)) return RT_FALSE;
      } else if (g > 0) {
        if ((x0.assigned() && ((x0.val() % g) != 0)) ||
            (x1.assigned() && ((x1.val() % g) != 0)))
          return RT_FALSE;
      }
    }
    if (x0.assigned() && x1.assigned()) {
      const int g=gcd_value(x0.val(),x1.val());
      if (!x2.in(g)) return RT_FALSE;
      return x2.assigned() ? RT_TRUE : RT_MAYBE;
    }
    return RT_MAYBE;
  }

  forceinline
  Gcd::Gcd(Home home, IntView y0, IntView y1, IntView y2)
    : TernaryPropagator<IntView,PC_INT_BND>(home,y0,y1,y2) {}

  inline ExecStatus
  Gcd::post(Home home, IntView x0, IntView x1, IntView x2) {
    if (x0 == x1)
      return AbsBnd<IntView>::post(home,x0,x2);
    if (x0.assigned() && (x0.val() == 0))
      return AbsBnd<IntView>::post(home,x1,x2);
    if (x1.assigned() && (x1.val() == 0))
      return AbsBnd<IntView>::post(home,x0,x2);
    if ((x0.assigned() && ((x0.val() == 1) || (x0.val() == -1))) ||
        (x1.assigned() && ((x1.val() == 1) || (x1.val() == -1)))) {
      GECODE_ME_CHECK(x2.eq(home,1));
      return ES_OK;
    }
    GECODE_ME_CHECK(x2.gq(home,0));
    if (gcd_excludes_zero_bnd(x0) || gcd_excludes_zero_bnd(x1))
      GECODE_ME_CHECK(x2.gq(home,1));
    GECODE_ME_CHECK(x2.lq(home,gcd_upper(x0,x1)));
    if (x2.assigned()) {
      const int g=x2.val();
      if (g == 0) {
        GECODE_ME_CHECK(x0.eq(home,0));
        GECODE_ME_CHECK(x1.eq(home,0));
        return ES_OK;
      }
      GECODE_ES_CHECK(gcd_multiple_bounds(home,x0,g));
      GECODE_ES_CHECK(gcd_multiple_bounds(home,x1,g));
      if (x0.assigned() && (x0.val() == 0))
        return AbsBnd<IntView>::post(home,x1,x2);
      if (x1.assigned() && (x1.val() == 0))
        return AbsBnd<IntView>::post(home,x0,x2);
    }
    if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(x2.eq(home,gcd_value(x0.val(),x1.val())));
      return ES_OK;
    }
    (void) new (home) Gcd(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline
  Gcd::Gcd(Space& home, Gcd& p)
    : TernaryPropagator<IntView,PC_INT_BND>(home,p) {}

  forceinline Actor*
  Gcd::copy(Space& home) {
    return new (home) Gcd(home,*this);
  }

  forceinline PropCost
  Gcd::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  inline ExecStatus
  Gcd::propagate(Space& home, const ModEventDelta&) {
    if (x0 == x1)
      GECODE_REWRITE(*this,AbsBnd<IntView>::post(home(*this),x0,x2));
    if (x0.assigned() && (x0.val() == 0))
      GECODE_REWRITE(*this,AbsBnd<IntView>::post(home(*this),x1,x2));
    if (x1.assigned() && (x1.val() == 0))
      GECODE_REWRITE(*this,AbsBnd<IntView>::post(home(*this),x0,x2));
    if ((x0.assigned() && ((x0.val() == 1) || (x0.val() == -1))) ||
        (x1.assigned() && ((x1.val() == 1) || (x1.val() == -1)))) {
      GECODE_ME_CHECK(x2.eq(home,1));
      return home.ES_SUBSUMED(*this);
    }
    GECODE_ME_CHECK(x2.gq(home,0));
    if (gcd_excludes_zero_bnd(x0) || gcd_excludes_zero_bnd(x1))
      GECODE_ME_CHECK(x2.gq(home,1));
    GECODE_ME_CHECK(x2.lq(home,gcd_upper(x0,x1)));
    if (x2.assigned()) {
      const int g=x2.val();
      if (g == 0) {
        GECODE_ME_CHECK(x0.eq(home,0));
        GECODE_ME_CHECK(x1.eq(home,0));
        return home.ES_SUBSUMED(*this);
      }
      GECODE_ES_CHECK(gcd_multiple_bounds(home,x0,g));
      GECODE_ES_CHECK(gcd_multiple_bounds(home,x1,g));
      if (x0.assigned() && (x0.val() == 0))
        GECODE_REWRITE(*this,AbsBnd<IntView>::post(home(*this),x1,x2));
      if (x1.assigned() && (x1.val() == 0))
        GECODE_REWRITE(*this,AbsBnd<IntView>::post(home(*this),x0,x2));
    }
    if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(x2.eq(home,gcd_value(x0.val(),x1.val())));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

  template<ReifyMode rm>
  forceinline
  ReGcd<rm>::ReGcd(Home home, IntView y0, IntView y1, IntView y2, BoolView b0)
    : Propagator(home), x0(y0), x1(y1), x2(y2), b(b0) {
    home.notice(*this,AP_WEAKLY);
    x0.subscribe(home,*this,PC_INT_BND);
    x1.subscribe(home,*this,PC_INT_BND);
    x2.subscribe(home,*this,PC_INT_BND);
    b.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReGcd<rm>::post(Home home, IntView x0, IntView x1, IntView x2, BoolView b) {
    if (b.one()) {
      if (rm == RM_PMI)
        return ES_OK;
      return Gcd::post(home,x0,x1,x2);
    }
    if (b.zero() && (rm == RM_IMP))
      return ES_OK;
    (void) new (home) ReGcd<rm>(home,x0,x1,x2,b);
    return ES_OK;
  }

  template<ReifyMode rm>
  forceinline
  ReGcd<rm>::ReGcd(Space& home, ReGcd<rm>& p)
    : Propagator(home,p) {
    x0.update(home,p.x0); x1.update(home,p.x1); x2.update(home,p.x2);
    b.update(home,p.b);
  }

  template<ReifyMode rm>
  forceinline Actor*
  ReGcd<rm>::copy(Space& home) {
    return new (home) ReGcd<rm>(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReGcd<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<ReifyMode rm>
  forceinline void
  ReGcd<rm>::reschedule(Space& home) {
    x0.reschedule(home,*this,PC_INT_BND);
    x1.reschedule(home,*this,PC_INT_BND);
    x2.reschedule(home,*this,PC_INT_BND);
    b.reschedule(home,*this,PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReGcd<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,Gcd::post(home(*this),x0,x1,x2));
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      const RelTest rt=gcd_status(x0,x1,x2);
      if (rt == RT_TRUE) return ES_FAILED;
      if (rt == RT_FALSE) return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }

    switch (gcd_status(x0,x1,x2)) {
    case RT_FALSE:
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        return home.ES_SUBSUMED(*this);
    case RT_TRUE:
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one(home));
        return home.ES_SUBSUMED(*this);
    case RT_MAYBE:
      return ES_FIX;
    default: GECODE_NEVER;
    }
  }

  template<ReifyMode rm>
  forceinline size_t
  ReGcd<rm>::dispose(Space& home) {
    x0.cancel(home,*this,PC_INT_BND);
    x1.cancel(home,*this,PC_INT_BND);
    x2.cancel(home,*this,PC_INT_BND);
    b.cancel(home,*this,PC_BOOL_VAL);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}
