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
  const unsigned long long divides_support_limit = 200000ULL;

  forceinline bool
  divides_value(int divisor, int dividend) {
    return (divisor == 0) ? (dividend == 0) : (dividend % divisor == 0);
  }

  forceinline bool
  divides_enumerable(const IntView& x0, const IntView& x1) {
    return (static_cast<unsigned long long>(x0.size()) * x1.size()) <=
      divides_support_limit;
  }

  inline RelTest
  divides_status(const IntView& x0, const IntView& x1) {
    if (x0 == x1)
      return RT_TRUE;
    if (x1.assigned() && (x1.val() == 0))
      return RT_TRUE;
    if (x0.assigned() && ((x0.val() == 1) || (x0.val() == -1)))
      return RT_TRUE;
    if (x0.assigned() && (x0.val() == 0) && !x1.in(0))
      return RT_FALSE;
    if (x0.assigned() && x1.assigned())
      return divides_value(x0.val(),x1.val()) ? RT_TRUE : RT_FALSE;
    if (!divides_enumerable(x0,x1))
      return RT_MAYBE;
    bool has_true = false, has_false = false;
    for (ViewValues<IntView> i0(x0); i0(); ++i0)
      for (ViewValues<IntView> i1(x1); i1(); ++i1) {
        if (divides_value(i0.val(),i1.val()))
          has_true = true;
        else
          has_false = true;
        if (has_true && has_false)
          return RT_MAYBE;
      }
    return has_true ? RT_TRUE : RT_FALSE;
  }

  /// Keep precisely values with support in the relation (or its negation).
  inline ExecStatus
  divides_filter(Space& home, IntView x0, IntView x1, bool positive) {
    if (x0 == x1)
      return positive ? ES_OK : ES_FAILED;
    if (!divides_enumerable(x0,x1))
      return ES_OK;

    Region r;
    unsigned int cap = x0.size() * x1.size();
    int* s0 = r.alloc<int>(cap);
    int* s1 = r.alloc<int>(cap);
    unsigned int n = 0;
    for (ViewValues<IntView> i0(x0); i0(); ++i0)
      for (ViewValues<IntView> i1(x1); i1(); ++i1)
        if (divides_value(i0.val(),i1.val()) == positive) {
          s0[n] = i0.val(); s1[n] = i1.val(); n++;
        }
    if (n == 0)
      return ES_FAILED;
    std::sort(s0,s0+n); std::sort(s1,s1+n);
    unsigned int n0=1, n1=1;
    for (unsigned int i=1; i<n; i++) {
      if (s0[i] != s0[n0-1]) s0[n0++]=s0[i];
      if (s1[i] != s1[n1-1]) s1[n1++]=s1[i];
    }
    Iter::Values::Array i0(s0,n0);
    GECODE_ME_CHECK(x0.inter_v(home,i0,false));
    Iter::Values::Array i1(s1,n1);
    GECODE_ME_CHECK(x1.inter_v(home,i1,false));
    return ES_OK;
  }

  template<ReifyMode rm>
  forceinline
  ReDivides<rm>::ReDivides(Home home, IntView y0, IntView y1, BoolView b0)
    : ReBinaryPropagator<IntView,PC_INT_DOM,BoolView>(home,y0,y1,b0) {}

  template<ReifyMode rm>
  inline ExecStatus
  ReDivides<rm>::post(Home home, IntView x0, IntView x1, BoolView b) {
    if (b.one() && (rm == RM_PMI))
      return ES_OK;
    if (b.zero() && (rm == RM_IMP))
      return ES_OK;
    RelTest rt = divides_status(x0,x1);
    if (rt == RT_TRUE) {
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    if (rt == RT_FALSE) {
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    }
    (void) new (home) ReDivides<rm>(home,x0,x1,b);
    return ES_OK;
  }

  template<ReifyMode rm>
  forceinline
  ReDivides<rm>::ReDivides(Space& home, ReDivides<rm>& p)
    : ReBinaryPropagator<IntView,PC_INT_DOM,BoolView>(home,p) {}

  template<ReifyMode rm>
  forceinline Actor*
  ReDivides<rm>::copy(Space& home) {
    return new (home) ReDivides<rm>(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReDivides<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::HI);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReDivides<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      GECODE_ES_CHECK(divides_filter(home,x0,x1,true));
      if (divides_status(x0,x1) == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      GECODE_ES_CHECK(divides_filter(home,x0,x1,false));
      if (divides_status(x0,x1) == RT_FALSE)
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }

    switch (divides_status(x0,x1)) {
    case RT_TRUE:
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one_none(home));
      break;
    case RT_FALSE:
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero_none(home));
      break;
    case RT_MAYBE:
      return ES_FIX;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}
