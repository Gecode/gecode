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

  template<class View, bool sign>
  forceinline WordValue
  order_lo(View x) {
    if (!sign)
      return x.lo();
    const WordValue s = WordValue(1) << (x.width()-1);
    return (x.lo() & ~s) | (~x.hi() & s);
  }

  template<class View, bool sign>
  forceinline WordValue
  order_hi(View x) {
    if (!sign)
      return x.hi();
    const WordValue s = WordValue(1) << (x.width()-1);
    return (x.hi() & ~s) | (~x.lo() & s);
  }

  template<class View, bool sign>
  forceinline ModEvent
  narrow_order(Home home, View x, WordValue lo, WordValue hi) {
    if (!sign)
      return x.narrow(home,lo,hi);
    const WordValue s = WordValue(1) << (x.width()-1);
    const WordValue actual_lo = (lo & ~s) | (~hi & s);
    const WordValue actual_hi = (hi & ~s) | (~lo & s);
    return x.narrow(home,actual_lo,actual_hi);
  }

  template<class View0, class View1, bool sign>
  forceinline Int::RelTest
  lq_test(View0 x0, View1 x1) {
    if (order_hi<View0,sign>(x0) <= order_lo<View1,sign>(x1))
      return Int::RT_TRUE;
    if (order_lo<View0,sign>(x0) > order_hi<View1,sign>(x1))
      return Int::RT_FALSE;
    return Int::RT_MAYBE;
  }

  template<class View0, class View1, bool sign, bool strict>
  ExecStatus
  narrow_ordering(Home home, View0 x0, View1 x1) {
    WordValue lo0 = order_lo<View0,sign>(x0);
    WordValue hi0 = order_hi<View0,sign>(x0);
    WordValue lo1 = order_lo<View1,sign>(x1);
    WordValue hi1 = order_hi<View1,sign>(x1);

    if (strict ? (lo0 >= hi1) : (lo0 > hi1))
      return ES_FAILED;

    for (unsigned int i=x0.width(); i--;) {
      const WordValue bit = WordValue(1) << i;
      if (((lo0 ^ hi0) & bit) != 0) {
        const WordValue pretend_one = lo0 | bit;
        if (strict ? (pretend_one >= hi1) : (pretend_one > hi1))
          hi0 &= ~bit;
        else
          break;
      }
    }
    GECODE_ME_CHECK((narrow_order<View0,sign>(home,x0,lo0,hi0)));

    for (unsigned int i=x1.width(); i--;) {
      const WordValue bit = WordValue(1) << i;
      if (((lo1 ^ hi1) & bit) != 0) {
        const WordValue pretend_zero = hi1 & ~bit;
        if (strict ? (lo0 >= pretend_zero) : (lo0 > pretend_zero))
          lo1 |= bit;
        else
          break;
      }
    }
    GECODE_ME_CHECK((narrow_order<View1,sign>(home,x1,lo1,hi1)));
    return ES_OK;
  }

  template<class View0, class View1, bool sign>
  forceinline
  Lq<View0,View1,sign>::Lq(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1) {}

  template<class View0, class View1, bool sign>
  forceinline
  Lq<View0,View1,sign>::Lq(Space& home, Lq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1, bool sign>
  ExecStatus
  Lq<View0,View1,sign>::post(Home home, View0 x0, View1 x1) {
    if (aliases(x0,x1))
      return ES_OK;
    GECODE_ES_CHECK((narrow_ordering<View0,View1,sign,false>(home,x0,x1)));
    if (lq_test<View0,View1,sign>(x0,x1) != Int::RT_TRUE)
      (void) new (home) Lq(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1, bool sign>
  Actor*
  Lq<View0,View1,sign>::copy(Space& home) {
    return new (home) Lq(home,*this);
  }

  template<class View0, class View1, bool sign>
  ExecStatus
  Lq<View0,View1,sign>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((narrow_ordering<View0,View1,sign,false>(home,x0,x1)));
    return (lq_test<View0,View1,sign>(x0,x1) == Int::RT_TRUE) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View0, class View1, bool sign>
  forceinline
  Le<View0,View1,sign>::Le(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1) {}

  template<class View0, class View1, bool sign>
  forceinline
  Le<View0,View1,sign>::Le(Space& home, Le& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1, bool sign>
  ExecStatus
  Le<View0,View1,sign>::post(Home home, View0 x0, View1 x1) {
    if (aliases(x0,x1))
      return ES_FAILED;
    GECODE_ES_CHECK((narrow_ordering<View0,View1,sign,true>(home,x0,x1)));
    if (order_hi<View0,sign>(x0) >= order_lo<View1,sign>(x1))
      (void) new (home) Le(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1, bool sign>
  Actor*
  Le<View0,View1,sign>::copy(Space& home) {
    return new (home) Le(home,*this);
  }

  template<class View0, class View1, bool sign>
  ExecStatus
  Le<View0,View1,sign>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((narrow_ordering<View0,View1,sign,true>(home,x0,x1)));
    return (order_hi<View0,sign>(x0) < order_lo<View1,sign>(x1)) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
