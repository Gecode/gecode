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

namespace Gecode { namespace Word { namespace Rel {

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  forceinline
  ReLq<View0,View1,CtrlView,rm,sign>::ReLq(
    Home home, View0 y0, View1 y1, CtrlView c)
    : Propagator(home), x0(y0), x1(y1), b(c) {
    x0.subscribe(home,*this,PC_WORD_BITS);
    x1.subscribe(home,*this,PC_WORD_BITS);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  forceinline
  ReLq<View0,View1,CtrlView,rm,sign>::ReLq(Space& home, ReLq& p)
    : Propagator(home,p) {
    x0.update(home,p.x0);
    x1.update(home,p.x1);
    b.update(home,p.b);
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  ExecStatus
  ReLq<View0,View1,CtrlView,rm,sign>::post(
    Home home, View0 x0, View1 x1, CtrlView b) {
    if (b.one()) {
      if (rm == RM_PMI)
        return ES_OK;
      return Lq<View0,View1,sign>::post(home,x0,x1);
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return ES_OK;
      return Le<View1,View0,sign>::post(home,x1,x0);
    }
    if (aliases(x0,x1)) {
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    switch (lq_test<View0,View1,sign>(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    case Int::RT_FALSE:
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    case Int::RT_MAYBE:
      (void) new (home) ReLq(home,x0,x1,b);
      return ES_OK;
    default:
      GECODE_NEVER;
    }
    return ES_FAILED;
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  Actor*
  ReLq<View0,View1,CtrlView,rm,sign>::copy(Space& home) {
    return new (home) ReLq(home,*this);
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  PropCost
  ReLq<View0,View1,CtrlView,rm,sign>::cost(
    const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  void
  ReLq<View0,View1,CtrlView,rm,sign>::reschedule(Space& home) {
    x0.reschedule(home,*this,PC_WORD_BITS);
    x1.reschedule(home,*this,PC_WORD_BITS);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  forceinline size_t
  ReLq<View0,View1,CtrlView,rm,sign>::dispose(Space& home) {
    x0.cancel(home,*this,PC_WORD_BITS);
    x1.cancel(home,*this,PC_WORD_BITS);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View0, class View1, class CtrlView,
           ReifyMode rm, bool sign>
  ExecStatus
  ReLq<View0,View1,CtrlView,rm,sign>::propagate(
    Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(Lq<View0,View1,sign>::post(
        home(*this),x0,x1)));
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(Le<View1,View0,sign>::post(
        home(*this),x1,x0)));
    }
    switch (lq_test<View0,View1,sign>(x0,x1)) {
    case Int::RT_TRUE:
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one_none(home));
      break;
    case Int::RT_FALSE:
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero_none(home));
      break;
    case Int::RT_MAYBE:
      return ES_FIX;
    default:
      GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
