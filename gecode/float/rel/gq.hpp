/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *
 *  Copyright:
 *
 *  Last modified:
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

namespace Gecode { namespace Float { namespace Rel {

  /*
   * Less or equal propagator
   *
   */

  template<class View>
  forceinline
  Gq<View>::Gq(Home home, View x0, View x1)
    : BinaryPropagator<View,PC_FLOAT_BND>(home,x0,x1) {}

  template<class View>
  ExecStatus
  Gq<View>::post(Home home, View x0, View x1) {
    GECODE_ME_CHECK(x0.lq(home,x1.max()));
    GECODE_ME_CHECK(x1.gq(home,x0.min()));
    if (!same(x0,x1) && (x0.max() > x1.min()))
      (void) new (home) Gq<View>(home,x0,x1);
    return ES_OK;
  }

  template<class View>
  forceinline
  Gq<View>::Gq(Space& home, bool share, Gq<View>& p)
    : BinaryPropagator<View,PC_FLOAT_BND>(home,share,p) {}

  template<class View>
  Actor*
  Gq<View>::copy(Space& home, bool share) {
    return new (home) Gq<View>(home,share,*this);
  }

  template<class View>
  ExecStatus
  Gq<View>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x0.gq(home,x1.min()));
    GECODE_ME_CHECK(x1.lq(home,x0.max()));
    return (x0.min() >= x1.max()) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }




  /*
   * Reified less or equal propagator
   *
   */

  template<class View, class CtrlView, ReifyMode rm>
  forceinline
  ReGq<View,CtrlView,rm>::ReGq(Home home, View x0, View x1, CtrlView b)
    : Int::ReBinaryPropagator<View,PC_FLOAT_BND,CtrlView>(home,x0,x1,b) {}

  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus
  ReGq<View,CtrlView,rm>::post(Home home, View x0, View x1, CtrlView b) {
    if (b.one()) {
      if (rm == RM_PMI)
        return ES_OK;
      return Gq<View>::post(home,x0,x1);
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return ES_OK;
      (void) new (home) Nq<View,View>(home,x0,x1); 
      return Gq<View>::post(home,x1,x0);
    }
    if (!same(x0,x1)) {
      switch (rtest_gq(x0,x1)) {
      case Int::RT_TRUE:
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one_none(home)); 
        break;
      case Int::RT_FALSE:
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero_none(home)); 
        break;
      case Int::RT_MAYBE:
        (void) new (home) ReGq<View,CtrlView,rm>(home,x0,x1,b); 
        break;
      default: GECODE_NEVER;
      }
    } else if (rm != RM_IMP) {
      GECODE_ME_CHECK(b.one_none(home));
    }
    return ES_OK;
  }

  template<class View, class CtrlView, ReifyMode rm>
  forceinline
  ReGq<View,CtrlView,rm>::ReGq(Space& home, bool share, ReGq& p)
    : Int::ReBinaryPropagator<View,PC_FLOAT_BND,CtrlView>(home,share,p) {}

  template<class View, class CtrlView, ReifyMode rm>
  Actor*
  ReGq<View,CtrlView,rm>::copy(Space& home, bool share) {
    return new (home) ReGq<View,CtrlView,rm>(home,share,*this);
  }

  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus
  ReGq<View,CtrlView,rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm != RM_PMI)
        GECODE_REWRITE(*this,Gq<View>::post(home(*this),x0,x1));
    }
    if (b.zero()) {
      if (rm != RM_IMP)
      {
        GECODE_ES_CHECK((Nq<View,View>::post(home(*this),x0,x1)));
        GECODE_REWRITE(*this,Gq<View>::post(home(*this),x1,x0));
      }
    }
    switch (rtest_gq(x0,x1)) {
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
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

  /*
   * Reified less or equal propagator involving one variable
   *
   */

  template<class View, class CtrlView, ReifyMode rm>
  forceinline
  ReGqFloat<View,CtrlView,rm>::ReGqFloat(Home home, View x, FloatVal c0, CtrlView b)
    : Int::ReUnaryPropagator<View,PC_FLOAT_BND,CtrlView>(home,x,b), c(c0) {}

  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus
  ReGqFloat<View,CtrlView,rm>::post(Home home, View x, FloatVal c, CtrlView b) {
    if (b.one()) {
      if (rm != RM_PMI)
        GECODE_ME_CHECK(x.lq(home,c));
    } else {
      switch (rtest_gq(x,c)) {
      case Int::RT_TRUE:
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one(home));
        break;
      case Int::RT_FALSE:
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        break;
      case Int::RT_MAYBE:
        (void) new (home) ReGqFloat<View,CtrlView,rm>(home,x,c,b);
        break;
      default: GECODE_NEVER;
      }
    }
    return ES_OK;
  }


  template<class View, class CtrlView, ReifyMode rm>
  forceinline
  ReGqFloat<View,CtrlView,rm>::ReGqFloat(Space& home, bool share, ReGqFloat& p)
    : Int::ReUnaryPropagator<View,PC_FLOAT_BND,CtrlView>(home,share,p), c(p.c) {}

  template<class View, class CtrlView, ReifyMode rm>
  Actor*
  ReGqFloat<View,CtrlView,rm>::copy(Space& home, bool share) {
    return new (home) ReGqFloat<View,CtrlView,rm>(home,share,*this);
  }

  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus
  ReGqFloat<View,CtrlView,rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm != RM_PMI)
        GECODE_ME_CHECK(x0.lq(home,c));
    } else {
      switch (rtest_gq(x0,c)) {
      case Int::RT_TRUE:
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one(home));
        break;
      case Int::RT_FALSE:
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        break;
      case Int::RT_MAYBE:
        return ES_FIX;
      default: GECODE_NEVER;
      }
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: float-prop

