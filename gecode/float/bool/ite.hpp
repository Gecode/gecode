/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#include <gecode/float/rel.hh>

namespace Gecode { namespace Float { namespace Bool {

  template<class View>
  forceinline
  Ite<View>::Ite(Home home, Int::BoolView b0, View y0, View y1, View y2)
    : Propagator(home), b(b0), x0(y0), x1(y1), x2(y2) {
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
    x0.subscribe(home,*this,PC_FLOAT_BND);
    x1.subscribe(home,*this,PC_FLOAT_BND);
    x2.subscribe(home,*this,PC_FLOAT_BND);
  }

  template<class View>
  forceinline
  Ite<View>::Ite(Space& home, Ite<View>& p)
    : Propagator(home,p) {
    b.update(home,p.b);
    x0.update(home,p.x0);
    x1.update(home,p.x1);
    x2.update(home,p.x2);
  }

  template<class View>
  PropCost
  Ite<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class View>
  void
  Ite<View>::reschedule(Space& home) {
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
    x0.reschedule(home,*this,PC_FLOAT_BND);
    x1.reschedule(home,*this,PC_FLOAT_BND);
    x2.reschedule(home,*this,PC_FLOAT_BND);
  }

  template<class View>
  forceinline size_t
  Ite<View>::dispose(Space& home) {
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    x0.cancel(home,*this,PC_FLOAT_BND);
    x1.cancel(home,*this,PC_FLOAT_BND);
    x2.cancel(home,*this,PC_FLOAT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }



  template<class View>
  Actor*
  Ite<View>::copy(Space& home) {
    return new (home) Ite<View>(home,*this);
  }

  template<class View>
  inline ExecStatus
  Ite<View>::post(Home home, Int::BoolView b, View x0, View x1, View x2) {
    if (b.one())
      return Rel::Eq<View,View>::post(home,x2,x0);
    if (b.zero())
      return Rel::Eq<View,View>::post(home,x2,x1);
    GECODE_ME_CHECK(x2.lq(home,std::max(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,std::min(x0.min(),x1.min())));
    (void) new (home) Ite<View>(home,b,x0,x1,x2);
    return ES_OK;
  }

  template<class View>
  ExecStatus
  Ite<View>::propagate(Space& home, const ModEventDelta&) {
    if (b.one())
      GECODE_REWRITE(*this,(Rel::Eq<View,View>::post(home(*this),x2,x0)));
    if (b.zero())
      GECODE_REWRITE(*this,(Rel::Eq<View,View>::post(home(*this),x2,x1)));

    GECODE_ME_CHECK(x2.lq(home,std::max(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,std::min(x0.min(),x1.min())));

    RelTest eq20 = rtest_eq(x2,x0);
    RelTest eq21 = rtest_eq(x2,x1);

    if ((eq20 == RT_FALSE) && (eq21 == RT_FALSE))
      return ES_FAILED;

    if (eq20 == RT_FALSE) {
      GECODE_ME_CHECK(b.zero_none(home));
      if (eq21 == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,(Rel::Eq<View,View>::post(home(*this),x2,x1)));
    }

    if (eq21 == RT_FALSE) {
      GECODE_ME_CHECK(b.one_none(home));
      if (eq20 == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,(Rel::Eq<View,View>::post(home(*this),x2,x0)));
    }

    if ((eq20 == RT_TRUE) && (eq21 == RT_TRUE))
      return home.ES_SUBSUMED(*this);

    return ES_FIX;
  }

}}}

// STATISTICS: float-prop
