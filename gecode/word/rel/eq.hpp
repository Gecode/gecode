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
 *
 */

namespace Gecode { namespace Word { namespace Rel {

  template<class View0, class View1>
  forceinline bool
  aliases(View0, View1) {
    return false;
  }

  forceinline bool
  aliases(WordView x, WordView y) {
    return x == y;
  }

  template<class View0, class View1>
  forceinline bool
  disjoint(View0 x, View1 y) {
    return ((x.lo() & ~y.hi()) != 0) || ((y.lo() & ~x.hi()) != 0);
  }

  template<class View0, class View1>
  forceinline Int::RelTest
  eq_test(View0 x, View1 y) {
    if (disjoint(x,y))
      return Int::RT_FALSE;
    if (x.assigned() && y.assigned())
      return Int::RT_TRUE;
    return Int::RT_MAYBE;
  }

  template<class View0, class View1>
  forceinline
  Eq<View0,View1>::Eq(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1) {}

  template<class View0, class View1>
  forceinline
  Eq<View0,View1>::Eq(Space& home, Eq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1>
  ExecStatus
  Eq<View0,View1>::post(Home home, View0 x0, View1 x1) {
    if (aliases(x0,x1))
      return ES_OK;
    const WordValue lo = x0.lo() | x1.lo();
    const WordValue hi = x0.hi() & x1.hi();
    GECODE_ME_CHECK(x0.narrow(home,lo,hi));
    GECODE_ME_CHECK(x1.narrow(home,lo,hi));
    if (!x0.assigned())
      (void) new (home) Eq(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1>
  Actor*
  Eq<View0,View1>::copy(Space& home) {
    return new (home) Eq(home,*this);
  }

  template<class View0, class View1>
  ExecStatus
  Eq<View0,View1>::propagate(Space& home, const ModEventDelta&) {
    const WordValue lo = x0.lo() | x1.lo();
    const WordValue hi = x0.hi() & x1.hi();
    GECODE_ME_CHECK(x0.narrow(home,lo,hi));
    GECODE_ME_CHECK(x1.narrow(home,lo,hi));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
