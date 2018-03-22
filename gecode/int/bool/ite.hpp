/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2013
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

#include <algorithm>

namespace Gecode { namespace Int { namespace Bool {

  template<class V0, class V1, class V2, PropCond pc>
  forceinline
  IteBase<V0,V1,V2,pc>::IteBase(Home home, BoolView b0, V0 y0, V1 y1, V2 y2)
    : Propagator(home), b(b0), x0(y0), x1(y1), x2(y2) {
    b.subscribe(home,*this,PC_BOOL_VAL);
    x0.subscribe(home,*this,pc);
    x1.subscribe(home,*this,pc);
    x2.subscribe(home,*this,pc);
  }

  template<class V0, class V1, class V2, PropCond pc>
  forceinline
  IteBase<V0,V1,V2,pc>::IteBase(Space& home, IteBase<V0,V1,V2,pc>& p)
    : Propagator(home,p) {
    b.update(home,p.b);
    x0.update(home,p.x0);
    x1.update(home,p.x1);
    x2.update(home,p.x2);
  }

  template<class V0, class V1, class V2, PropCond pc>
  PropCost
  IteBase<V0,V1,V2,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class V0, class V1, class V2, PropCond pc>
  void
  IteBase<V0,V1,V2,pc>::reschedule(Space& home) {
    b.reschedule(home,*this,PC_BOOL_VAL);
    x0.reschedule(home,*this,pc);
    x1.reschedule(home,*this,pc);
    x2.reschedule(home,*this,pc);
  }

  template<class V0, class V1, class V2, PropCond pc>
  forceinline size_t
  IteBase<V0,V1,V2,pc>::dispose(Space& home) {
    b.cancel(home,*this,PC_BOOL_VAL);
    x0.cancel(home,*this,pc);
    x1.cancel(home,*this,pc);
    x2.cancel(home,*this,pc);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }



  template<class V0, class V1, class V2>
  forceinline
  IteBnd<V0,V1,V2>::IteBnd(Home home, BoolView b, V0 x0, V1 x1, V2 x2)
    : IteBase<V0,V1,V2,PC_INT_BND>(home,b,x0,x1,x2) {}

  template<class V0, class V1, class V2>
  forceinline
  IteBnd<V0,V1,V2>::IteBnd(Space& home, IteBnd<V0,V1,V2>& p)
    : IteBase<V0,V1,V2,PC_INT_BND>(home,p) {}

  template<class V0, class V1, class V2>
  Actor*
  IteBnd<V0,V1,V2>::copy(Space& home) {
    return new (home) IteBnd<V0,V1,V2>(home,*this);
  }

  template<class V0, class V1, class V2>
  inline ExecStatus
  IteBnd<V0,V1,V2>::post(Home home, BoolView b, V0 x0, V1 x1, V2 x2) {
    if (b.one())
      return Rel::EqBnd<V2,V0>::post(home,x2,x0);
    if (b.zero())
      return Rel::EqBnd<V2,V1>::post(home,x2,x1);
    GECODE_ME_CHECK(x2.lq(home,std::max(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,std::min(x0.min(),x1.min())));
    (void) new (home) IteBnd<V0,V1,V2>(home,b,x0,x1,x2);
    return ES_OK;
  }

  template<class V0, class V1, class V2>
  ExecStatus
  IteBnd<V0,V1,V2>::propagate(Space& home, const ModEventDelta&) {
    if (b.one())
      GECODE_REWRITE(*this,(Rel::EqBnd<V2,V0>::post(home(*this),x2,x0)));
    if (b.zero())
      GECODE_REWRITE(*this,(Rel::EqBnd<V2,V1>::post(home(*this),x2,x1)));

    GECODE_ME_CHECK(x2.lq(home,std::max(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,std::min(x0.min(),x1.min())));

    RelTest eq20 = rtest_eq_bnd(x2,x0);
    RelTest eq21 = rtest_eq_bnd(x2,x1);

    if ((eq20 == RT_FALSE) && (eq21 == RT_FALSE))
      return ES_FAILED;

    if (eq20 == RT_FALSE) {
      GECODE_ME_CHECK(b.zero_none(home));
      if (eq21 == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,(Rel::EqBnd<V2,V1>::post(home(*this),x2,x1)));
    }

    if (eq21 == RT_FALSE) {
      GECODE_ME_CHECK(b.one_none(home));
      if (eq20 == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,(Rel::EqBnd<V2,V0>::post(home(*this),x2,x0)));
    }

    if ((eq20 == RT_TRUE) && (eq21 == RT_TRUE))
      return home.ES_SUBSUMED(*this);

    return ES_FIX;
  }



  template<class V0, class V1, class V2>
  forceinline
  IteDom<V0,V1,V2>::IteDom(Home home, BoolView b, V0 x0, V1 x1, V2 x2)
    : IteBase<V0,V1,V2,PC_INT_DOM>(home,b,x0,x1,x2) {}

  template<class V0, class V1, class V2>
  forceinline
  IteDom<V0,V1,V2>::IteDom(Space& home, IteDom<V0,V1,V2>& p)
    : IteBase<V0,V1,V2,PC_INT_DOM>(home,p) {}

  template<class V0, class V1, class V2>
  Actor*
  IteDom<V0,V1,V2>::copy(Space& home) {
    return new (home) IteDom<V0,V1,V2>(home,*this);
  }

  template<class V0, class V1, class V2>
  inline ExecStatus
  IteDom<V0,V1,V2>::post(Home home, BoolView b, V0 x0, V1 x1, V2 x2) {
    if (b.one())
      return Rel::EqDom<V2,V0>::post(home,x2,x0);
    if (b.zero())
      return Rel::EqDom<V2,V1>::post(home,x2,x1);
    GECODE_ME_CHECK(x2.lq(home,std::max(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,std::min(x0.min(),x1.min())));
    (void) new (home) IteDom<V0,V1,V2>(home,b,x0,x1,x2);
    return ES_OK;
  }

  template<class V0, class V1, class V2>
  PropCost
  IteDom<V0,V1,V2>::cost(const Space&, const ModEventDelta& med) const {
    if (V0::me(med) == ME_INT_DOM)
      return PropCost::ternary(PropCost::HI);
    else
      return PropCost::ternary(PropCost::LO);
  }

  template<class V0, class V1, class V2>
  ExecStatus
  IteDom<V0,V1,V2>::propagate(Space& home, const ModEventDelta& med) {
    if (b.one())
      GECODE_REWRITE(*this,(Rel::EqDom<V2,V0>::post(home(*this),x2,x0)));
    if (b.zero())
      GECODE_REWRITE(*this,(Rel::EqDom<V2,V1>::post(home(*this),x2,x1)));

    GECODE_ME_CHECK(x2.lq(home,std::max(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,std::min(x0.min(),x1.min())));

    if (V0::me(med) != ME_INT_DOM) {
      RelTest eq20 = rtest_eq_bnd(x2,x0);
      RelTest eq21 = rtest_eq_bnd(x2,x1);

      if ((eq20 == RT_FALSE) && (eq21 == RT_FALSE))
        return ES_FAILED;

      if (eq20 == RT_FALSE) {
        GECODE_ME_CHECK(b.zero_none(home));
        if (eq21 == RT_TRUE)
          return home.ES_SUBSUMED(*this);
        else
          GECODE_REWRITE(*this,
                         (Rel::EqDom<V2,V1>::post(home(*this),x2,x1)));
      }

      if (eq21 == RT_FALSE) {
        GECODE_ME_CHECK(b.one_none(home));
        if (eq20 == RT_TRUE)
          return home.ES_SUBSUMED(*this);
        else
          GECODE_REWRITE(*this,
                         (Rel::EqDom<V2,V0>::post(home(*this),x2,x0)));
      }

      if ((eq20 == RT_TRUE) && (eq21 == RT_TRUE))
        return home.ES_SUBSUMED(*this);

      return home.ES_FIX_PARTIAL(*this,V0::med(ME_INT_DOM));
    }

    RelTest eq20 = rtest_eq_dom(x2,x0);
    RelTest eq21 = rtest_eq_dom(x2,x1);

    if ((eq20 == RT_FALSE) && (eq21 == RT_FALSE))
      return ES_FAILED;

    if (eq20 == RT_FALSE) {
      GECODE_ME_CHECK(b.zero_none(home));
      if (eq21 == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,
                       (Rel::EqDom<V2,V1>::post(home(*this),x2,x1)));
    }

    if (eq21 == RT_FALSE) {
      GECODE_ME_CHECK(b.one_none(home));
      if (eq20 == RT_TRUE)
        return home.ES_SUBSUMED(*this);
      else
        GECODE_REWRITE(*this,
                       (Rel::EqDom<V2,V0>::post(home(*this),x2,x0)));
    }

    assert((eq20 != RT_TRUE) || (eq21 != RT_TRUE));

    ViewRanges<V0> r0(x0);
    ViewRanges<V1> r1(x1);
    Iter::Ranges::Union<ViewRanges<V0>,ViewRanges<V1> > u(r0,r1);

    if (!shared<V0,V2>(x0,x2) && !shared<V1,V2>(x1,x2))
      GECODE_ME_CHECK(x2.inter_r(home,u,false));
    else
      GECODE_ME_CHECK(x2.inter_r(home,u,true));

    return ES_FIX;
  }

}}}

// STATISTICS: int-prop
