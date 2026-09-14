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

namespace Gecode { namespace Word { namespace Count {

  forceinline unsigned int
  Popcount::ones(WordValue bits) {
    unsigned int n = 0;
    while (bits != 0) {
      bits &= bits-1;
      n++;
    }
    return n;
  }

  forceinline
  Popcount::Popcount(Home home, WordView x, Int::IntView count)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>(home,x,count) {}

  forceinline
  Popcount::Popcount(Space& home, Popcount& p)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>(home,p) {}

  forceinline ExecStatus
  Popcount::prune(Home home, WordView x, Int::IntView count) {
    const unsigned int known = ones(x.lo());
    const unsigned int possible = ones(x.hi());
    GECODE_ME_CHECK(count.gq(home,static_cast<int>(known)));
    GECODE_ME_CHECK(count.lq(home,static_cast<int>(possible)));

    if (count.max() == static_cast<int>(known))
      GECODE_ME_CHECK(x.narrow(home,x.lo(),x.lo()));
    else if (count.min() == static_cast<int>(possible))
      GECODE_ME_CHECK(x.narrow(home,x.hi(),x.hi()));

    const unsigned int final_known = ones(x.lo());
    const unsigned int final_possible = ones(x.hi());
    GECODE_ME_CHECK(count.gq(home,static_cast<int>(final_known)));
    GECODE_ME_CHECK(count.lq(home,static_cast<int>(final_possible)));
    return ((final_known == final_possible) && count.assigned())
      ? ES_OK : ES_FIX;
  }

  forceinline ExecStatus
  Popcount::post(Home home, WordView x, Int::IntView count) {
    GECODE_ME_CHECK(count.gq(home,0));
    GECODE_ME_CHECK(count.lq(home,static_cast<int>(x.width())));
    ExecStatus es = prune(home,x,count);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) Popcount(home,x,count);
    return ES_OK;
  }

  forceinline Actor*
  Popcount::copy(Space& home) {
    return new (home) Popcount(home,*this);
  }

  forceinline ExecStatus
  Popcount::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es = prune(home,x0,x1);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
