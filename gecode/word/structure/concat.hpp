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

namespace Gecode { namespace Word { namespace Structure {

  template<class View0, class View1, class View2>
  forceinline
  Concat<View0,View1,View2>::Concat(Home home, View0 y0, View1 y1, View2 y2)
    : MixTernaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS,
        View2,PC_WORD_BITS>(home,y0,y1,y2) {}

  template<class View0, class View1, class View2>
  forceinline
  Concat<View0,View1,View2>::Concat(Space& home, Concat& p)
    : MixTernaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS,
        View2,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1, class View2>
  ExecStatus
  Concat<View0,View1,View2>::narrow(Home home, View0 high, View1 low,
                                   View2 result) {
    const unsigned int shift = low.width();
    GECODE_ME_CHECK(result.narrow(home,(high.lo() << shift) | low.lo(),
                                  (high.hi() << shift) | low.hi()));
    GECODE_ME_CHECK(low.narrow(home,result.lo() & low.mask(),
                               result.hi() & low.mask()));
    GECODE_ME_CHECK(high.narrow(home,result.lo() >> shift,
                                result.hi() >> shift));
    return ES_OK;
  }

  template<class View0, class View1, class View2>
  ExecStatus
  Concat<View0,View1,View2>::post(Home home, View0 high, View1 low,
                                 View2 result) {
    GECODE_ES_CHECK(narrow(home,high,low,result));
    if (!high.assigned() || !low.assigned() || !result.assigned())
      (void) new (home) Concat(home,high,low,result);
    return ES_OK;
  }

  template<class View0, class View1, class View2>
  Actor*
  Concat<View0,View1,View2>::copy(Space& home) {
    return new (home) Concat(home,*this);
  }

  template<class View0, class View1, class View2>
  ExecStatus
  Concat<View0,View1,View2>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    return (x0.assigned() && x1.assigned() && x2.assigned())
      ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
