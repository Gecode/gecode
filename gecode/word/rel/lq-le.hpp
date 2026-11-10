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

  template<class View, bool is_signed>
  forceinline WordValue
  order_lo(View x) {
    if (!is_signed)
      return x.lo();
    const WordValue s = WordValue(1) << (x.width()-1);
    return (x.lo() & ~s) | (~x.hi() & s);
  }

  template<class View, bool is_signed>
  forceinline WordValue
  order_hi(View x) {
    if (!is_signed)
      return x.hi();
    const WordValue s = WordValue(1) << (x.width()-1);
    return (x.hi() & ~s) | (~x.lo() & s);
  }

  template<class View, bool is_signed>
  forceinline ModEvent
  narrow_order(Home home, View x, WordValue lo, WordValue hi) {
    if (!is_signed)
      return x.narrow(home,lo,hi);
    const WordValue s = WordValue(1) << (x.width()-1);
    const WordValue actual_lo = (lo & ~s) | (~hi & s);
    const WordValue actual_hi = (hi & ~s) | (~lo & s);
    return x.narrow(home,actual_lo,actual_hi);
  }

  /// Classify ordering using masks in the selected numeric order
  template<class View0, class View1, bool is_signed>
  forceinline Int::RelTest
  lq_test(View0 x0, View1 x1) {
    const bool is_entailed=
      order_hi<View0,is_signed>(x0) <= order_lo<View1,is_signed>(x1);
    if (is_entailed)
      return Int::RT_TRUE;
    const bool is_contradicted=
      order_lo<View0,is_signed>(x0) > order_hi<View1,is_signed>(x1);
    if (is_contradicted)
      return Int::RT_FALSE;
    return Int::RT_MAYBE;
  }

  /// A cube expressed in the numeric order used by an ordering relation
  struct OrderedWordCube {
    const unsigned int width;
    const WordValue lo;
    const WordValue hi;
  };

  /// Compute forced zero bits from the most significant undecided bit
  template<bool is_strict>
  forceinline WordValue
  compute_left_upper(const OrderedWordCube& cube, WordValue right_upper) {
    WordValue hi=cube.hi;
    for (unsigned int i=cube.width; i--;) {
      const WordValue bit=WordValue(1) << i;
      if (((cube.lo ^ hi) & bit) != 0) {
        const WordValue pretend_one=cube.lo | bit;
        const bool is_infeasible=is_strict ?
          (pretend_one >= right_upper) : (pretend_one > right_upper);
        if (is_infeasible)
          hi &= ~bit;
        else
          break;
      }
    }
    return hi;
  }

  /// Compute forced one bits using the synchronized left lower bound
  template<bool is_strict>
  forceinline WordValue
  compute_right_lower(const OrderedWordCube& cube, WordValue left_lower) {
    WordValue lo=cube.lo;
    for (unsigned int i=cube.width; i--;) {
      const WordValue bit=WordValue(1) << i;
      if (((lo ^ cube.hi) & bit) != 0) {
        const WordValue pretend_zero=cube.hi & ~bit;
        const bool is_infeasible=is_strict ?
          (left_lower >= pretend_zero) : (left_lower > pretend_zero);
        if (is_infeasible)
          lo |= bit;
        else
          break;
      }
    }
    return lo;
  }

  /// Alternate ordered-mask deductions and native domain synchronization
  template<class View0, class View1, bool is_signed, bool is_strict>
  ExecStatus
  narrow_ordering(Home home, View0 x0, View1 x1) {
    bool needs_repeat;
    do {
      WordValue lo0=order_lo<View0,is_signed>(x0);
      WordValue hi0=order_hi<View0,is_signed>(x0);
      WordValue lo1=order_lo<View1,is_signed>(x1);
      const WordValue hi1=order_hi<View1,is_signed>(x1);
      needs_repeat=false;

      const bool is_infeasible=is_strict ? (lo0 >= hi1) : (lo0 > hi1);
      if (is_infeasible)
        return ES_FAILED;

      const OrderedWordCube left={x0.width(),lo0,hi0};
      hi0=compute_left_upper<is_strict>(left,hi1);
      if (hi0 != order_hi<View0,is_signed>(x0)) {
        GECODE_ME_CHECK((narrow_order<View0,is_signed>(home,x0,lo0,hi0)));
        // Bounded synchronization can fix more bits than the local tell.
        needs_repeat=(lo0 != order_lo<View0,is_signed>(x0)) ||
          (hi0 != order_hi<View0,is_signed>(x0));
        lo0=order_lo<View0,is_signed>(x0);
      }

      const OrderedWordCube right={x1.width(),lo1,hi1};
      lo1=compute_right_lower<is_strict>(right,lo0);
      if (lo1 != order_lo<View1,is_signed>(x1)) {
        GECODE_ME_CHECK((narrow_order<View1,is_signed>(home,x1,lo1,hi1)));
        needs_repeat=needs_repeat ||
          (lo1 != order_lo<View1,is_signed>(x1)) ||
          (hi1 != order_hi<View1,is_signed>(x1));
      }
    } while (needs_repeat);
    return ES_OK;
  }

  template<class View0, class View1, bool is_signed>
  forceinline
  Lq<View0,View1,is_signed>::Lq(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1) {}

  template<class View0, class View1, bool is_signed>
  forceinline
  Lq<View0,View1,is_signed>::Lq(Space& home, Lq& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1, bool is_signed>
  ExecStatus
  Lq<View0,View1,is_signed>::post(Home home, View0 x0, View1 x1) {
    if (aliases(x0,x1))
      return ES_OK;
    GECODE_ES_CHECK((narrow_ordering<View0,View1,is_signed,false>(
      home,x0,x1)));
    switch (lq_test<View0,View1,is_signed>(x0,x1)) {
    case Int::RT_FALSE:
      return ES_FAILED;
    case Int::RT_MAYBE:
      (void) new (home) Lq(home,x0,x1);
      break;
    case Int::RT_TRUE:
      break;
    default:
      GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class View0, class View1, bool is_signed>
  Actor*
  Lq<View0,View1,is_signed>::copy(Space& home) {
    return new (home) Lq(home,*this);
  }

  template<class View0, class View1, bool is_signed>
  ExecStatus
  Lq<View0,View1,is_signed>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((narrow_ordering<View0,View1,is_signed,false>(
      home,x0,x1)));
    switch (lq_test<View0,View1,is_signed>(x0,x1)) {
    case Int::RT_FALSE:
      return ES_FAILED;
    case Int::RT_TRUE:
      return home.ES_SUBSUMED(*this);
    case Int::RT_MAYBE:
      return ES_FIX;
    default:
      GECODE_NEVER;
    }
    return ES_FAILED;
  }

  template<class View0, class View1, bool is_signed>
  forceinline
  Le<View0,View1,is_signed>::Le(Home home, View0 y0, View1 y1)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,y0,y1) {}

  template<class View0, class View1, bool is_signed>
  forceinline
  Le<View0,View1,is_signed>::Le(Space& home, Le& p)
    : MixBinaryPropagator<
        View0,PC_WORD_BITS,View1,PC_WORD_BITS>(home,p) {}

  template<class View0, class View1, bool is_signed>
  ExecStatus
  Le<View0,View1,is_signed>::post(Home home, View0 x0, View1 x1) {
    if (aliases(x0,x1))
      return ES_FAILED;
    GECODE_ES_CHECK((narrow_ordering<View0,View1,is_signed,true>(home,x0,x1)));
    const bool is_contradicted=
      order_lo<View0,is_signed>(x0) >= order_hi<View1,is_signed>(x1);
    if (is_contradicted)
      return ES_FAILED;
    const bool needs_actor=
      order_hi<View0,is_signed>(x0) >= order_lo<View1,is_signed>(x1);
    if (needs_actor)
      (void) new (home) Le(home,x0,x1);
    return ES_OK;
  }

  template<class View0, class View1, bool is_signed>
  Actor*
  Le<View0,View1,is_signed>::copy(Space& home) {
    return new (home) Le(home,*this);
  }

  template<class View0, class View1, bool is_signed>
  ExecStatus
  Le<View0,View1,is_signed>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((narrow_ordering<View0,View1,is_signed,true>(home,x0,x1)));
    const bool is_contradicted=
      order_lo<View0,is_signed>(x0) >= order_hi<View1,is_signed>(x1);
    if (is_contradicted)
      return ES_FAILED;
    const bool is_entailed=
      order_hi<View0,is_signed>(x0) < order_lo<View1,is_signed>(x1);
    return is_entailed ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
