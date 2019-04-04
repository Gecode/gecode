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

#include <gecode/int/order.hh>
 
namespace Gecode { namespace Int { namespace Order {

  Actor*
  OrderProp::copy(Space& home) {
    return new (home) OrderProp(home,*this);
  }

  PropCost
  OrderProp::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  void
  OrderProp::reschedule(Space& home) {
    s0.reschedule(home,*this,PC_INT_BND);
    s1.reschedule(home,*this,PC_INT_BND);
    b.reschedule(home,*this,PC_INT_VAL);
  }

  ExecStatus
  OrderProp::propagate(Space& home, const ModEventDelta&) {
    if (s0.max() + p0 <= s1.min()) {
      GECODE_ME_CHECK(b.zero(home));
      return home.ES_SUBSUMED(*this);
    } else if (s1.max() + p1 <= s0.min()) {
      GECODE_ME_CHECK(b.one(home));
      return home.ES_SUBSUMED(*this);
    } else if ((s1.min() + p1 > s0.max()) || b.zero()) {
      GECODE_ME_CHECK(b.zero(home));
      GECODE_REWRITE(*this,(Rel::Lq<OffsetView,IntView>::post
                            (home(*this),OffsetView(s0,p0),s1)));
    } else if ((s0.min() + p0 > s1.max()) || b.one()) {
      GECODE_ME_CHECK(b.one(home));      
      GECODE_REWRITE(*this,(Rel::Lq<OffsetView,IntView>::post
                            (home(*this),OffsetView(s1,p1),s0)));
    }
    return ES_FIX;
  }

  size_t
  OrderProp::dispose(Space& home) {
    s0.cancel(home,*this,PC_INT_BND);
    s1.cancel(home,*this,PC_INT_BND);
    b.cancel(home,*this,PC_INT_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: int-prop
