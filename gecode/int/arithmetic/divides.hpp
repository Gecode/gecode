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

  forceinline bool
  divides_value(int divisor, int dividend) {
    return (divisor == 0) ? (dividend == 0) : (dividend % divisor == 0);
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
    return RT_MAYBE;
  }

  template<ReifyMode rm>
  forceinline
  ReDivides<rm>::ReDivides(Home home, IntView y0, IntView y1, BoolView b0)
    : ReBinaryPropagator<IntView,PC_INT_BND,BoolView>(home,y0,y1,b0) {
    home.notice(*this,AP_WEAKLY);
  }

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
    : ReBinaryPropagator<IntView,PC_INT_BND,BoolView>(home,p) {}

  template<ReifyMode rm>
  forceinline Actor*
  ReDivides<rm>::copy(Space& home) {
    return new (home) ReDivides<rm>(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReDivides<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<ReifyMode rm>
  inline ExecStatus
  ReDivides<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      const RelTest rt=divides_status(x0,x1);
      if (rt == RT_FALSE) return ES_FAILED;
      if (rt == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      const RelTest rt=divides_status(x0,x1);
      if (rt == RT_TRUE) return ES_FAILED;
      if (rt == RT_FALSE)
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

  template<ReifyMode rm>
  forceinline size_t
  ReDivides<rm>::dispose(Space& home) {
    home.ignore(*this,AP_WEAKLY);
    (void) ReBinaryPropagator<IntView,PC_INT_BND,BoolView>::dispose(home);
    return sizeof(*this);
  }

}}}
