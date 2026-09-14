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

namespace Gecode { namespace Word { namespace Channel {

  template<ReductionType rt>
  forceinline bool
  Reduction<rt>::single_bit(WordValue bits) {
    return (bits != 0) && ((bits & (bits-1)) == 0);
  }

  template<ReductionType rt>
  forceinline bool
  Reduction<rt>::parity(WordValue bits) {
    bits ^= bits >> 32;
    bits ^= bits >> 16;
    bits ^= bits >> 8;
    bits ^= bits >> 4;
    bits ^= bits >> 2;
    bits ^= bits >> 1;
    return (bits & 1U) != 0;
  }

  template<ReductionType rt>
  forceinline
  Reduction<rt>::Reduction(Home home, WordView x, Int::BoolView b)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,x,b) {}

  template<ReductionType rt>
  forceinline
  Reduction<rt>::Reduction(Space& home, Reduction& p)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,p) {}

  template<ReductionType rt>
  forceinline ExecStatus
  Reduction<rt>::prune(Home home, WordView x, Int::BoolView b) {
    const WordValue unknown = x.unknown();
    if (rt == RT_AND) {
      if (x.lo() == x.mask()) {
        GECODE_ME_CHECK(b.one(home));
        return ES_OK;
      }
      if (x.hi() != x.mask()) {
        GECODE_ME_CHECK(b.zero(home));
        return ES_OK;
      }
      if (b.one()) {
        GECODE_ME_CHECK(x.narrow(home,x.mask(),x.mask()));
        return ES_OK;
      }
      if (b.zero() && single_bit(unknown)) {
        GECODE_ME_CHECK(x.narrow(home,x.lo(),x.hi() & ~unknown));
        return ES_OK;
      }
    } else if (rt == RT_OR) {
      if (x.lo() != 0) {
        GECODE_ME_CHECK(b.one(home));
        return ES_OK;
      }
      if (x.hi() == 0) {
        GECODE_ME_CHECK(b.zero(home));
        return ES_OK;
      }
      if (b.zero()) {
        GECODE_ME_CHECK(x.narrow(home,0,0));
        return ES_OK;
      }
      if (b.one() && single_bit(unknown)) {
        GECODE_ME_CHECK(x.narrow(home,x.lo() | unknown,x.hi()));
        return ES_OK;
      }
    } else {
      const bool known_parity = parity(x.lo());
      if (unknown == 0) {
        GECODE_ME_CHECK(known_parity ? b.one(home) : b.zero(home));
        return ES_OK;
      }
      if ((b.one() || b.zero()) && single_bit(unknown)) {
        const bool unknown_one = b.one() != known_parity;
        if (unknown_one)
          GECODE_ME_CHECK(x.narrow(home,x.lo() | unknown,x.hi()));
        else
          GECODE_ME_CHECK(x.narrow(home,x.lo(),x.hi() & ~unknown));
        return ES_OK;
      }
    }
    return ES_FIX;
  }

  template<ReductionType rt>
  forceinline ExecStatus
  Reduction<rt>::post(Home home, WordView x, Int::BoolView b) {
    if (x.mask() == 1U)
      return Bit::post(home,x,1U,b);
    ExecStatus es = prune(home,x,b);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) Reduction(home,x,b);
    return ES_OK;
  }

  template<ReductionType rt>
  forceinline Actor*
  Reduction<rt>::copy(Space& home) {
    return new (home) Reduction(home,*this);
  }

  template<ReductionType rt>
  forceinline ExecStatus
  Reduction<rt>::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es = prune(home,x0,x1);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
