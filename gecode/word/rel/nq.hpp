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

  template<class View>
  forceinline ExecStatus
  exclude(Home home, View x, WordValue value) {
    if (!x.in(value))
      return ES_OK;
    if (x.assigned())
      return ES_FAILED;
    if (x.unknown_size() == 1) {
      const WordValue bit = x.unknown();
      if ((value & bit) != 0)
        GECODE_ME_CHECK(x.narrow(home,x.lo(),x.hi() & ~bit));
      else
        GECODE_ME_CHECK(x.narrow(home,x.lo() | bit,x.hi()));
    }
    return ES_OK;
  }

  template<class View0, class View1>
  forceinline
  Nq<View0,View1>::Nq(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1) {}

  template<class View0, class View1>
  forceinline
  Nq<View0,View1>::Nq(Space& home, Nq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1>
  ExecStatus
  Nq<View0,View1>::post(Home home, View0 x0, View1 x1) {
    if (aliases(x0,x1))
      return ES_FAILED;
    if (disjoint(x0,x1))
      return ES_OK;
    if (x0.assigned()) {
      GECODE_ES_CHECK(exclude(home,x1,x0.val()));
      if (x1.in(x0.val()))
        (void) new (home) Nq(home,x0,x1);
      return ES_OK;
    }
    if (x1.assigned()) {
      GECODE_ES_CHECK(exclude(home,x0,x1.val()));
      if (x0.in(x1.val()))
        (void) new (home) Nq(home,x0,x1);
      return ES_OK;
    }
    (void) new (home) Nq(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1>
  Actor*
  Nq<View0,View1>::copy(Space& home) {
    return new (home) Nq(home,*this);
  }

  template<class View0, class View1>
  ExecStatus
  Nq<View0,View1>::propagate(Space& home, const ModEventDelta&) {
    if (disjoint(x0,x1))
      return home.ES_SUBSUMED(*this);
    if (x0.assigned()) {
      GECODE_ES_CHECK(exclude(home,x1,x0.val()));
      return x1.in(x0.val()) ? ES_FIX : home.ES_SUBSUMED(*this);
    }
    if (x1.assigned()) {
      GECODE_ES_CHECK(exclude(home,x0,x1.val()));
      return x0.in(x1.val()) ? ES_FIX : home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
