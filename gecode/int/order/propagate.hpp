/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2019
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

#include <gecode/int/rel.hh>

namespace Gecode { namespace Int { namespace Order {

  forceinline
  OrderProp::OrderProp(Home home, IntView _s0, int _p0, IntView _s1, int _p1,
                       BoolView _b)
    : Propagator(home), s0(_s0), s1(_s1), b(_b), p0(_p0), p1(_p1) {
    s0.subscribe(home,*this,PC_INT_BND);
    s1.subscribe(home,*this,PC_INT_BND);
    b.subscribe(home,*this,PC_INT_VAL);
  }

  forceinline ExecStatus
  OrderProp::post(Home home, IntView s0, int p0, IntView s1, int p1,
                  BoolView b) {
    if (s0.max() + p0 <= s1.min()) {
      GECODE_ME_CHECK(b.zero(home));
    } else if (s1.max() + p1 <= s0.min()) {
      GECODE_ME_CHECK(b.one(home));
    } else if ((s1.min() + p1 > s0.max()) || b.zero()) {
      GECODE_ME_CHECK(b.zero(home));
      return Rel::Lq<OffsetView,IntView>::post(home,OffsetView(s0,p0),s1);
    } else if ((s0.min() + p0 > s1.max()) || b.one()) {
      GECODE_ME_CHECK(b.one(home));
      return Rel::Lq<OffsetView,IntView>::post(home,OffsetView(s1,p1),s0);
    } else {
      (void) new (home) OrderProp(home,s0,p0,s1,p1,b);
    }
    return ES_OK;
  }

  forceinline
  OrderProp::OrderProp(Space& home, OrderProp& p)
    : Propagator(home,p), p0(p.p0), p1(p.p1) {
    s0.update(home,p.s0);
    s1.update(home,p.s1);
    b.update(home,p.b);
  }

}}}

// STATISTICS: int-prop
