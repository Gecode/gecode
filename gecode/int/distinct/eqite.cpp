/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <gecode/int/distinct.hh>

#include <algorithm>

namespace Gecode { namespace Int { namespace Distinct {

  PropCost
  EqIte::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::HI);
  }

  Actor*
  EqIte::copy(Space& home) {
    return new (home) EqIte(home,*this);
  }

  ExecStatus
  EqIte::propagate(Space& home, const ModEventDelta&) {
    switch (rtest_eq_dom(x0,c0)) {
    case RT_TRUE:
      GECODE_ME_CHECK(x1.eq(home,c1));
      return home.ES_SUBSUMED(*this);
    case RT_FALSE:
      GECODE_REWRITE(*this,(Rel::EqDom<IntView,IntView>
                            ::post(home(*this),x0,x1)));
      GECODE_NEVER;
    case RT_MAYBE:
      break;
    default: GECODE_NEVER;
    }

    GECODE_ME_CHECK(x1.lq(home,std::max(x0.max(),c1)));
    GECODE_ME_CHECK(x1.gq(home,std::min(x0.min(),c1)));

    RelTest eq_then = rtest_eq_dom(x1,c1);
    RelTest eq_else = rtest_eq_dom(x1,x0);

    if ((eq_then == RT_FALSE) && (eq_else == RT_FALSE))
      return ES_FAILED;

    if (eq_then == RT_FALSE) {
      // x1 and c1 are not equal
      GECODE_ME_CHECK(x0.nq(home,c0));
      if (eq_else == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,
                       (Rel::EqDom<IntView,IntView>::post(home(*this),x0,x1)));
    }

    if (eq_else == RT_FALSE) {
      // x0 and x1 are not equal
      GECODE_ME_CHECK(x0.eq(home,c0));
      GECODE_ME_CHECK(x1.eq(home,c1));
      return home.ES_SUBSUMED(*this);
    }


    assert((eq_then != RT_TRUE) || (eq_else != RT_TRUE));

    ViewRanges<IntView> r0(x0);
    Iter::Ranges::Singleton r1(c1,c1);
    Iter::Ranges::Union<ViewRanges<IntView>,
      Iter::Ranges::Singleton > u(r0,r1);

    GECODE_ME_CHECK(x1.inter_r(home,u,true));

    return ES_FIX;
  }

}}}

// STATISTICS: int-prop
