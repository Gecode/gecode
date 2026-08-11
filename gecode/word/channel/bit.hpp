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

namespace Gecode { namespace Word { namespace Channel {

  forceinline
  Bit::Bit(Home home, WordView x, WordValue bit_mask0, Int::BoolView b)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,x,b),
      bit_mask(bit_mask0) {}

  forceinline
  Bit::Bit(Space& home, Bit& p)
    : MixBinaryPropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,p),
      bit_mask(p.bit_mask) {}

  forceinline ExecStatus
  Bit::post(Home home, WordView x, WordValue bit_mask, Int::BoolView b) {
    if ((x.lo() & bit_mask) != 0) {
      GECODE_ME_CHECK(b.one(home));
    } else if ((x.hi() & bit_mask) == 0) {
      GECODE_ME_CHECK(b.zero(home));
    } else if (b.one()) {
      GECODE_ME_CHECK(x.narrow(home,x.lo() | bit_mask,x.hi()));
    } else if (b.zero()) {
      GECODE_ME_CHECK(x.narrow(home,x.lo(),x.hi() & ~bit_mask));
    } else {
      (void) new (home) Bit(home,x,bit_mask,b);
    }
    return ES_OK;
  }

}}}

// STATISTICS: word-prop
