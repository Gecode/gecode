/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Vincent Barichard, 2012
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

namespace Gecode { namespace Float { namespace Arithmetic {

  /// Test whether \a x is positive
  template<class View>
  forceinline bool
  pos(const View& x) {
    return x.min() >= 0.0;
  }
  /// Test whether \a x is negative
  template<class View>
  forceinline bool
  neg(const View& x) {
    return x.max() <= 0.0;
  }
  /// Test whether \a x is neither positive nor negative
  template<class View>
  forceinline bool
  any(const View& x) {
    return (x.min() <= 0.0) && (x.max() >= 0.0);
  }

  /*
   * Propagator for x * y = x
   *
   */

  template<class View>
  forceinline
  MultZeroOne<View>::MultZeroOne(Home home, View x0, View x1)
    : BinaryPropagator<View,PC_FLOAT_BND>(home,x0,x1) {}

  template<class View>
  forceinline ExecStatus
  MultZeroOne<View>::post(Home home, View x0, View x1) {
    switch (rtest_eq(x0,0.0)) {
    case RT_FALSE:
      GECODE_ME_CHECK(x1.eq(home,1.0));
      break;
    case RT_TRUE:
      break;
    case RT_MAYBE:
      switch (rtest_eq(x1,1.0)) {
      case RT_FALSE:
        GECODE_ME_CHECK(x0.eq(home,0.0));
        break;
      case RT_TRUE:
        break;
      case RT_MAYBE:
        (void) new (home) MultZeroOne<View>(home,x0,x1);
        break;
      default: GECODE_NEVER;
      }
      break;
    default: GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  MultZeroOne<View>::MultZeroOne(Space& home, MultZeroOne<View>& p)
    : BinaryPropagator<View,PC_FLOAT_BND>(home,p) {}

  template<class View>
  Actor*
  MultZeroOne<View>::copy(Space& home) {
    return new (home) MultZeroOne<View>(home,*this);
  }

  template<class View>
  ExecStatus
  MultZeroOne<View>::propagate(Space& home, const ModEventDelta&) {
    switch (rtest_eq(x0,0.0)) {
    case RT_FALSE:
      GECODE_ME_CHECK(x1.eq(home,1.0));
      break;
    case RT_TRUE:
      break;
    case RT_MAYBE:
      switch (rtest_eq(x1,1.0)) {
      case RT_FALSE:
        GECODE_ME_CHECK(x0.eq(home,0.0));
        break;
      case RT_TRUE:
        break;
      case RT_MAYBE:
        return ES_FIX;
      default: GECODE_NEVER;
      }
      break;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }


  /*
   * Positive bounds consistent multiplication
   *
   */
  template<class VA, class VB, class VC>
  forceinline
  MultPlus<VA,VB,VC>::MultPlus(Home home, VA x0, VB x1, VC x2)
    : MixTernaryPropagator<VA,PC_FLOAT_BND,VB,PC_FLOAT_BND,VC,PC_FLOAT_BND>
  (home,x0,x1,x2) {}

  template<class VA, class VB, class VC>
  forceinline
  MultPlus<VA,VB,VC>::MultPlus(Space& home, MultPlus<VA,VB,VC>& p)
    : MixTernaryPropagator<VA,PC_FLOAT_BND,VB,PC_FLOAT_BND,VC,PC_FLOAT_BND>
  (home,p) {}

  template<class VA, class VB, class VC>
  Actor*
  MultPlus<VA,VB,VC>::copy(Space& home) {
    return new (home) MultPlus<VA,VB,VC>(home,*this);
  }

  template<class VA, class VB, class VC>
  ExecStatus
  MultPlus<VA,VB,VC>::propagate(Space& home, const ModEventDelta&) {
    if (x1.min() != 0.0)
      GECODE_ME_CHECK(x0.eq(home,x2.val() / x1.val()));
    if (x0.min() != 0.0)
      GECODE_ME_CHECK(x1.eq(home,x2.val() / x0.val()));
    GECODE_ME_CHECK(x2.eq(home,x0.val() * x1.val()));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_NOFIX;
  }

  template<class VA, class VB, class VC>
  forceinline ExecStatus
  MultPlus<VA,VB,VC>::post(Home home, VA x0, VB x1, VC x2) {
    GECODE_ME_CHECK(x0.gq(home,0.0));
    GECODE_ME_CHECK(x1.gq(home,0.0));
    Rounding r;
    GECODE_ME_CHECK(x2.gq(home,r.mul_down(x0.min(),x1.min())));
    (void) new (home) MultPlus<VA,VB,VC>(home,x0,x1,x2);
    return ES_OK;
  }


  /*
   * Bounds consistent multiplication
   *
   */
  template<class View>
  forceinline
  Mult<View>::Mult(Home home, View x0, View x1, View x2)
    : TernaryPropagator<View,PC_FLOAT_BND>(home,x0,x1,x2) {}

  template<class View>
  forceinline
  Mult<View>::Mult(Space& home, Mult<View>& p)
    : TernaryPropagator<View,PC_FLOAT_BND>(home,p) {}

  template<class View>
  Actor*
  Mult<View>::copy(Space& home) {
    return new (home) Mult<View>(home,*this);
  }

  template<class View>
  ExecStatus
  Mult<View>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x2.eq(home,x0.val()*x1.val()));
    if (!x1.zero_in())
      GECODE_ME_CHECK(x0.eq(home,x2.val() / x1.val()));
    if (!x0.zero_in())
      GECODE_ME_CHECK(x1.eq(home,x2.val() / x0.val()));
    GECODE_ME_CHECK(x2.eq(home,x0.val()*x1.val()));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_NOFIX;
  }

  template<class View>
  ExecStatus
  Mult<View>::post(Home home, View x0, View x1, View x2) {
    if (x0 == x1)
      return Sqr<View>::post(home,x0,x2);
    if (x0 == x2)
      return MultZeroOne<View>::post(home,x0,x1);
    if (x1 == x2)
      return MultZeroOne<View>::post(home,x1,x0);
    GECODE_ME_CHECK(x2.eq(home,x0.val()*x1.val()));
    if (!x1.zero_in())
      GECODE_ME_CHECK(x0.eq(home,x2.val() / x1.val()));
    if (!x0.zero_in())
      GECODE_ME_CHECK(x1.eq(home,x2.val() / x0.val()));
    GECODE_ME_CHECK(x2.eq(home,x0.val()*x1.val()));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return ES_OK;

    (void) new (home) Mult<View>(home,x0,x1,x2);
    return ES_OK;
  }


}}}

// STATISTICS: float-prop
