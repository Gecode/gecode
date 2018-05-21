/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Vincent Barichard, 2012
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

namespace Gecode { namespace Float { namespace Channel {

  template<class A, class B>
  forceinline
  Channel<A,B>::Channel(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,Int::PC_INT_BND>(home,x0,x1) {}

  template<class A, class B>
  forceinline
  Channel<A,B>::Channel(Space& home, Channel<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,Int::PC_INT_BND>(home,p) {}

  template<class A, class B>
  Actor*
  Channel<A,B>::copy(Space& home) {
    return new (home) Channel<A,B>(home,*this);
  }

  template<class A, class B>
  ExecStatus
  Channel<A,B>::post(Home home, A x0, B x1) {
    GECODE_ME_CHECK(x0.eq(home,FloatVal(Int::Limits::min,
                                        Int::Limits::max)));
    GECODE_ME_CHECK(x1.gq(home,static_cast<int>(std::ceil(x0.min()))));
    GECODE_ME_CHECK(x1.lq(home,static_cast<int>(std::floor(x0.max()))));
    GECODE_ME_CHECK(x0.eq(home,FloatVal(x1.min(),x1.max())));
    (void) new (home) Channel<A,B>(home,x0,x1);
    return ES_OK;
  }

  template<class A, class B>
  ExecStatus
  Channel<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.gq(home,static_cast<int>(std::ceil(x0.min()))));
    GECODE_ME_CHECK(x1.lq(home,static_cast<int>(std::floor(x0.max()))));
    GECODE_ME_CHECK(x0.eq(home,FloatVal(x1.min(),x1.max())));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: float-prop

