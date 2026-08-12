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

  /// Largest Cartesian product enumerated for domain propagation.
  const unsigned long long gcd_support_limit = 200000ULL;

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
  gcd_alias_ok(const IntView& x0, int a, const IntView& x1, int b,
               const IntView& x2, int c) {
    return ((x0 != x1) || (a == b)) &&
           ((x0 != x2) || (a == c)) &&
           ((x1 != x2) || (b == c));
  }

  forceinline bool
  gcd_enumerable(const IntView& x0, const IntView& x1, const IntView& x2) {
    unsigned long long n = x0.size();
    n *= x1.size();
    if (n > gcd_support_limit)
      return false;
    n *= x2.size();
    return n <= gcd_support_limit;
  }

  /// Determine whether satisfying and violating assignments exist.
  inline void
  gcd_status(const IntView& x0, const IntView& x1, const IntView& x2,
             bool& has_true, bool& has_false) {
    has_true = has_false = false;
    for (ViewValues<IntView> i0(x0); i0(); ++i0)
      for (ViewValues<IntView> i1(x1); i1(); ++i1)
        for (ViewValues<IntView> i2(x2); i2(); ++i2) {
          int a=i0.val(), b=i1.val(), c=i2.val();
          if (!gcd_alias_ok(x0,a,x1,b,x2,c))
            continue;
          if (gcd_value(a,b) == c)
            has_true = true;
          else
            has_false = true;
          if (has_true && has_false)
            return;
        }
  }

  /// Keep precisely the values supported by either the relation or its negation.
  inline ExecStatus
  gcd_filter(Space& home, IntView x0, IntView x1, IntView x2, bool positive) {
    Region r;
    unsigned int cap = static_cast<unsigned int>(
      static_cast<unsigned long long>(x0.size()) * x1.size() * x2.size());
    int* s0 = r.alloc<int>(cap);
    int* s1 = r.alloc<int>(cap);
    int* s2 = r.alloc<int>(cap);
    unsigned int n = 0;
    for (ViewValues<IntView> i0(x0); i0(); ++i0)
      for (ViewValues<IntView> i1(x1); i1(); ++i1)
        for (ViewValues<IntView> i2(x2); i2(); ++i2) {
          int a=i0.val(), b=i1.val(), c=i2.val();
          if (!gcd_alias_ok(x0,a,x1,b,x2,c))
            continue;
          if ((gcd_value(a,b) == c) == positive) {
            s0[n]=a; s1[n]=b; s2[n]=c; n++;
          }
        }
    if (n == 0)
      return ES_FAILED;
    std::sort(s0,s0+n); std::sort(s1,s1+n); std::sort(s2,s2+n);
    unsigned int n0=1, n1=1, n2=1;
    for (unsigned int i=1; i<n; i++) {
      if (s0[i] != s0[n0-1]) s0[n0++]=s0[i];
      if (s1[i] != s1[n1-1]) s1[n1++]=s1[i];
      if (s2[i] != s2[n2-1]) s2[n2++]=s2[i];
    }
    Iter::Values::Array i0(s0,n0);
    GECODE_ME_CHECK(x0.inter_v(home,i0,false));
    Iter::Values::Array i1(s1,n1);
    GECODE_ME_CHECK(x1.inter_v(home,i1,false));
    Iter::Values::Array i2(s2,n2);
    GECODE_ME_CHECK(x2.inter_v(home,i2,false));
    return ES_OK;
  }

  forceinline
  Gcd::Gcd(Home home, IntView y0, IntView y1, IntView y2)
    : TernaryPropagator<IntView,PC_INT_DOM>(home,y0,y1,y2) {}

  inline ExecStatus
  Gcd::post(Home home, IntView x0, IntView x1, IntView x2) {
    GECODE_ME_CHECK(x2.gq(home,0));
    GECODE_ME_CHECK(x2.lq(home,std::max(gcd_max_abs(x0),gcd_max_abs(x1))));
    if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(x2.eq(home,gcd_value(x0.val(),x1.val())));
      return ES_OK;
    }
    (void) new (home) Gcd(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline
  Gcd::Gcd(Space& home, Gcd& p)
    : TernaryPropagator<IntView,PC_INT_DOM>(home,p) {}

  forceinline Actor*
  Gcd::copy(Space& home) {
    return new (home) Gcd(home,*this);
  }

  forceinline PropCost
  Gcd::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::HI);
  }

  inline ExecStatus
  Gcd::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x2.gq(home,0));
    GECODE_ME_CHECK(x2.lq(home,std::max(gcd_max_abs(x0),gcd_max_abs(x1))));
    if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(x2.eq(home,gcd_value(x0.val(),x1.val())));
      return home.ES_SUBSUMED(*this);
    }
    if (gcd_enumerable(x0,x1,x2))
      GECODE_ES_CHECK(gcd_filter(home,x0,x1,x2,true));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  template<ReifyMode rm>
  forceinline
  ReGcd<rm>::ReGcd(Home home, IntView y0, IntView y1, IntView y2, BoolView b0)
    : Propagator(home), x0(y0), x1(y1), x2(y2), b(b0) {
    x0.subscribe(home,*this,PC_INT_DOM);
    x1.subscribe(home,*this,PC_INT_DOM);
    x2.subscribe(home,*this,PC_INT_DOM);
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
    return PropCost::ternary(PropCost::HI);
  }

  template<ReifyMode rm>
  forceinline void
  ReGcd<rm>::reschedule(Space& home) {
    x0.reschedule(home,*this,PC_INT_DOM);
    x1.reschedule(home,*this,PC_INT_DOM);
    x2.reschedule(home,*this,PC_INT_DOM);
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
      if (gcd_enumerable(x0,x1,x2))
        GECODE_ES_CHECK(gcd_filter(home,x0,x1,x2,false));
      if (x0.assigned() && x1.assigned() && x2.assigned())
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }

    bool has_true, has_false;
    if (gcd_enumerable(x0,x1,x2)) {
      gcd_status(x0,x1,x2,has_true,has_false);
      if (!has_true) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        return home.ES_SUBSUMED(*this);
      }
      if (!has_false) {
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one(home));
        return home.ES_SUBSUMED(*this);
      }
    }
    return ES_FIX;
  }

  template<ReifyMode rm>
  forceinline size_t
  ReGcd<rm>::dispose(Space& home) {
    x0.cancel(home,*this,PC_INT_DOM);
    x1.cancel(home,*this,PC_INT_DOM);
    x2.cancel(home,*this,PC_INT_DOM);
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}
