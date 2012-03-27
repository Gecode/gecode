/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2012
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

namespace Gecode { namespace Float { namespace Arithmetic {

  /*
   * Bounds consistent square operator
   *
   */

  template<class A, class B>
  forceinline
  Square<A,B>::Square(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Square<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) Square<A,B>(home,x0,x1);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  Square<A,B>::Square(Space& home, bool share, Square<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}


  template<class A, class B>
  Actor*
  Square<A,B>::copy(Space& home, bool share) {
    return new (home) Square<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  Square<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,boost::numeric::square(x0.domain())));
    if (x0.min() >= 0)
      GECODE_ME_CHECK(x0.eq(home,boost::numeric::sqrt(x1.domain())));
    else if (x0.max() <= 0)
      GECODE_ME_CHECK(x0.eq(home,-boost::numeric::sqrt(x1.domain())));
    else
      GECODE_ME_CHECK(x0.eq(home,
                            boost::numeric::hull(
                                boost::numeric::sqrt(x1.domain()),
                                -boost::numeric::sqrt(x1.domain())
                            )
                      ));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  /*
   * Bounds consistent square root operator
   *
   */

  template<class A, class B>
  forceinline
  Sqrt<A,B>::Sqrt(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Sqrt<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) Sqrt<A,B>(home,x0,x1);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  Sqrt<A,B>::Sqrt(Space& home, bool share, Sqrt<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  Sqrt<A,B>::copy(Space& home, bool share) {
    return new (home) Sqrt<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  Sqrt<A,B>::propagate(Space& home, const ModEventDelta&) {
    if (x0.max() < 0) return ES_FAILED;
    GECODE_ME_CHECK(x1.eq(home,boost::numeric::sqrt(x0.domain())));
    GECODE_ME_CHECK(x0.eq(home,boost::numeric::square(x1.domain())));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  /*
   * Absolute consistent square operator
   *
   */

  template<class A, class B>
  forceinline
  Abs<A,B>::Abs(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Abs<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) Abs<A,B>(home,x0,x1);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  Abs<A,B>::Abs(Space& home, bool share, Abs<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  Abs<A,B>::copy(Space& home, bool share) {
    return new (home) Abs<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  Abs<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,boost::numeric::abs(x0.domain())));
    if (x1.max() >= 0)
      GECODE_ME_CHECK(x0.eq(home,FloatInterval(-x1.max(), x1.max())));
    else
      return ES_FAILED;
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: float-prop

