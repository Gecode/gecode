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

  template<ZeroDirection direction>
  forceinline unsigned int
  ZeroCount<direction>::zeros(WordValue bits, unsigned int width) {
    if (bits == 0)
      return width;
    unsigned int n = 0;
    if (direction == ZD_LEADING) {
      WordValue bit = WordValue(1) << (width-1);
      while ((bits & bit) == 0) {
        n++;
        bit >>= 1;
      }
    } else {
      WordValue bit = 1;
      while ((bits & bit) == 0) {
        n++;
        bit <<= 1;
      }
    }
    return n;
  }

  template<ZeroDirection direction>
  forceinline WordValue
  ZeroCount<direction>::region(unsigned int width, unsigned int size) {
    if (size == 0)
      return 0;
    if (size == width)
      return width_mask(width);
    if (direction == ZD_LEADING)
      return width_mask(width) & ~width_mask(width-size);
    return width_mask(size);
  }

  template<ZeroDirection direction>
  forceinline bool
  ZeroCount<direction>::single_bit(WordValue bits) {
    return (bits != 0) && ((bits & (bits-1)) == 0);
  }

  template<ZeroDirection direction>
  forceinline
  ZeroCount<direction>::ZeroCount(Home home, WordView x, Int::IntView count)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>(home,x,count) {}

  template<ZeroDirection direction>
  forceinline
  ZeroCount<direction>::ZeroCount(Space& home, ZeroCount& p)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>(home,p) {}

  template<ZeroDirection direction>
  forceinline ExecStatus
  ZeroCount<direction>::prune(Home home, WordView x, Int::IntView count) {
    const unsigned int width = x.width();
    unsigned int minimum = zeros(x.hi(),width);
    unsigned int maximum = zeros(x.lo(),width);
    GECODE_ME_CHECK(count.gq(home,static_cast<int>(minimum)));
    GECODE_ME_CHECK(count.lq(home,static_cast<int>(maximum)));

    const unsigned int required_zero = static_cast<unsigned int>(count.min());
    const WordValue zero_region = region(width,required_zero);
    GECODE_ME_CHECK(x.narrow(home,x.lo(),x.hi() & ~zero_region));

    const unsigned int latest = static_cast<unsigned int>(count.max());
    if (latest < width) {
      const WordValue must_contain_one = region(width,latest+1);
      if ((x.lo() & must_contain_one) == 0) {
        const WordValue possible = x.hi() & must_contain_one;
        if (single_bit(possible))
          GECODE_ME_CHECK(x.narrow(home,x.lo() | possible,x.hi()));
      }
    }

    minimum = zeros(x.hi(),width);
    maximum = zeros(x.lo(),width);
    GECODE_ME_CHECK(count.gq(home,static_cast<int>(minimum)));
    GECODE_ME_CHECK(count.lq(home,static_cast<int>(maximum)));
    return ((minimum == maximum) && count.assigned()) ? ES_OK : ES_FIX;
  }

  template<ZeroDirection direction>
  forceinline ExecStatus
  ZeroCount<direction>::post(Home home, WordView x, Int::IntView count) {
    GECODE_ME_CHECK(count.gq(home,0));
    GECODE_ME_CHECK(count.lq(home,static_cast<int>(x.width())));
    ExecStatus es = prune(home,x,count);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) ZeroCount(home,x,count);
    return ES_OK;
  }

  template<ZeroDirection direction>
  forceinline Actor*
  ZeroCount<direction>::copy(Space& home) {
    return new (home) ZeroCount(home,*this);
  }

  template<ZeroDirection direction>
  forceinline ExecStatus
  ZeroCount<direction>::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es = prune(home,x0,x1);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
