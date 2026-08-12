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

namespace Gecode { namespace Word { namespace Conditional {

  template<class V0, class V1, class V2>
  forceinline
  Ite<V0,V1,V2>::Ite(Home home, Int::BoolView b0,
                     V0 y0, V1 y1, V2 y2)
    : Propagator(home), b(b0), x0(y0), x1(y1), x2(y2) {
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
    x0.subscribe(home,*this,PC_WORD_BITS);
    x1.subscribe(home,*this,PC_WORD_BITS);
    x2.subscribe(home,*this,PC_WORD_BITS);
  }

  template<class V0, class V1, class V2>
  forceinline
  Ite<V0,V1,V2>::Ite(Space& home, Ite<V0,V1,V2>& p)
    : Propagator(home,p) {
    b.update(home,p.b);
    x0.update(home,p.x0);
    x1.update(home,p.x1);
    x2.update(home,p.x2);
  }

  template<class V0, class V1, class V2>
  forceinline Actor*
  Ite<V0,V1,V2>::copy(Space& home) {
    return new (home) Ite<V0,V1,V2>(home,*this);
  }

  template<class V0, class V1, class V2>
  forceinline PropCost
  Ite<V0,V1,V2>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class V0, class V1, class V2>
  forceinline void
  Ite<V0,V1,V2>::reschedule(Space& home) {
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
    x0.reschedule(home,*this,PC_WORD_BITS);
    x1.reschedule(home,*this,PC_WORD_BITS);
    x2.reschedule(home,*this,PC_WORD_BITS);
  }

  template<class V0, class V1, class V2>
  forceinline size_t
  Ite<V0,V1,V2>::dispose(Space& home) {
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    x0.cancel(home,*this,PC_WORD_BITS);
    x1.cancel(home,*this,PC_WORD_BITS);
    x2.cancel(home,*this,PC_WORD_BITS);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class V0, class V1, class V2>
  forceinline ExecStatus
  Ite<V0,V1,V2>::post(Home home, Int::BoolView b,
                      V0 x0, V1 x1, V2 x2) {
    if (b.one())
      return Rel::Eq<V2,V0>::post(home,x2,x0);
    if (b.zero())
      return Rel::Eq<V2,V1>::post(home,x2,x1);
    if (Rel::aliases(x0,x1))
      return Rel::Eq<V2,V0>::post(home,x2,x0);

    GECODE_ME_CHECK(x2.narrow(home,x0.lo()&x1.lo(),x0.hi()|x1.hi()));
    const Int::RelTest eq20=Rel::eq_test(x2,x0);
    const Int::RelTest eq21=Rel::eq_test(x2,x1);
    if ((eq20 == Int::RT_FALSE) && (eq21 == Int::RT_FALSE))
      return ES_FAILED;
    if (eq20 == Int::RT_FALSE) {
      GECODE_ME_CHECK(b.zero(home));
      return Rel::Eq<V2,V1>::post(home,x2,x1);
    }
    if (eq21 == Int::RT_FALSE) {
      GECODE_ME_CHECK(b.one(home));
      return Rel::Eq<V2,V0>::post(home,x2,x0);
    }
    if ((eq20 == Int::RT_TRUE) && (eq21 == Int::RT_TRUE))
      return ES_OK;
    (void) new (home) Ite<V0,V1,V2>(home,b,x0,x1,x2);
    return ES_OK;
  }

  template<class V0, class V1, class V2>
  ExecStatus
  Ite<V0,V1,V2>::propagate(Space& home, const ModEventDelta&) {
    if (b.one())
      GECODE_REWRITE(*this,(Rel::Eq<V2,V0>::post(home(*this),x2,x0)));
    if (b.zero())
      GECODE_REWRITE(*this,(Rel::Eq<V2,V1>::post(home(*this),x2,x1)));

    GECODE_ME_CHECK(x2.narrow(home,x0.lo()&x1.lo(),x0.hi()|x1.hi()));
    const Int::RelTest eq20=Rel::eq_test(x2,x0);
    const Int::RelTest eq21=Rel::eq_test(x2,x1);
    if ((eq20 == Int::RT_FALSE) && (eq21 == Int::RT_FALSE))
      return ES_FAILED;
    if (eq20 == Int::RT_FALSE) {
      GECODE_ME_CHECK(b.zero_none(home));
      GECODE_REWRITE(*this,(Rel::Eq<V2,V1>::post(home(*this),x2,x1)));
    }
    if (eq21 == Int::RT_FALSE) {
      GECODE_ME_CHECK(b.one_none(home));
      GECODE_REWRITE(*this,(Rel::Eq<V2,V0>::post(home(*this),x2,x0)));
    }
    if ((eq20 == Int::RT_TRUE) && (eq21 == Int::RT_TRUE))
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
