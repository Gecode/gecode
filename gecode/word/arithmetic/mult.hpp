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

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline
  Mult::Mult(Home home, WordView y0, WordView y1, WordView y2)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,y0,y1,y2) {}

  forceinline
  Mult::Mult(Space& home, Mult& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline Actor*
  Mult::copy(Space& home) {
    return new (home) Mult(home,*this);
  }

  forceinline PropCost
  Mult::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  forceinline WordValue
  mult_low_mask(unsigned int bits) {
    if (bits == 0)
      return 0;
    if (bits >= 8U*sizeof(WordValue))
      return ~WordValue(0);
    return (WordValue(1) << bits)-1;
  }

  forceinline unsigned int
  mult_known_low(WordView x) {
    const WordValue unknown = x.unknown();
    unsigned int bits = 0;
    while ((bits < x.width()) &&
           ((unknown & (WordValue(1) << bits)) == 0))
      bits++;
    return bits;
  }

  forceinline unsigned int
  mult_trailing_zeros(WordValue value, unsigned int limit) {
    unsigned int bits = 0;
    while ((bits < limit) && ((value & (WordValue(1) << bits)) == 0))
      bits++;
    return bits;
  }

  forceinline WordValue
  mult_inverse_odd(WordValue value) {
    WordValue inverse = 1;
    // Newton iteration doubles the number of correct low bits each time.
    for (unsigned int i=0; i<6; i++)
      inverse *= WordValue(2)-value*inverse;
    return inverse;
  }

  forceinline ModEvent
  mult_narrow_low(Home home, WordView x, unsigned int bits,
                  WordValue value) {
    const WordValue field = mult_low_mask(bits);
    return x.narrow(home,x.lo() | (value & field),
                    x.hi() & ((value & field) | ~field));
  }

  forceinline ExecStatus
  mult_equal(Home home, WordView x, WordView y) {
    const WordValue lo = x.lo() | y.lo();
    const WordValue hi = x.hi() & y.hi();
    GECODE_ME_CHECK(x.narrow(home,lo,hi));
    GECODE_ME_CHECK(y.narrow(home,lo,hi));
    return ES_OK;
  }

  /** Propagate c*y=z modulo 2^bits for fixed low prefixes c and z. */
  forceinline ExecStatus
  mult_inverse_prefix(Home home, WordView c, WordView y, WordView z) {
    const unsigned int bits = std::min(mult_known_low(c),mult_known_low(z));
    if (bits == 0)
      return ES_OK;
    const WordValue field = mult_low_mask(bits);
    const WordValue cv = c.lo() & field;
    const WordValue zv = z.lo() & field;
    const unsigned int zeros = mult_trailing_zeros(cv,bits);
    if (zeros == bits) {
      if (zv != 0)
        return ES_FAILED;
      return ES_OK;
    }
    if ((zv & mult_low_mask(zeros)) != 0)
      return ES_FAILED;
    const unsigned int result_bits = bits-zeros;
    const WordValue odd = cv >> zeros;
    const WordValue result =
      ((zv >> zeros) * mult_inverse_odd(odd)) &
      mult_low_mask(result_bits);
    GECODE_ME_CHECK(mult_narrow_low(home,y,result_bits,result));
    return ES_OK;
  }

  forceinline ExecStatus
  Mult::narrow(Home home, WordView x, WordView y, WordView z) {
    const unsigned int width = x.width();
    bool changed;
    do {
      const WordValue old[] = {x.lo(),x.hi(),y.lo(),y.hi(),z.lo(),z.hi()};

      if ((x.assigned() && (x.val() == 0)) ||
          (y.assigned() && (y.val() == 0))) {
        GECODE_ME_CHECK(z.eq(home,0));
      }
      if (x.assigned() && (x.val() == 1))
        GECODE_ES_CHECK(mult_equal(home,y,z));
      if (y.assigned() && (y.val() == 1))
        GECODE_ES_CHECK(mult_equal(home,x,z));

      // Multiplication modulo 2^k depends only on the low k operand bits.
      const unsigned int known = std::min(mult_known_low(x),mult_known_low(y));
      if (known != 0)
        GECODE_ME_CHECK(mult_narrow_low(home,z,known,x.lo()*y.lo()));

      // Guaranteed powers of two in both operands force low product zeros.
      const unsigned int xz = mult_trailing_zeros(x.hi(),width);
      const unsigned int yz = mult_trailing_zeros(y.hi(),width);
      const unsigned int zeros = (xz > width-yz) ? width : xz+yz;
      if (zeros != 0)
        GECODE_ME_CHECK(mult_narrow_low(home,z,zeros,0));

      // A fixed result prefix and multiplicand prefix determine the other
      // operand prefix after stripping powers of two and inverting the odd
      // factor in the corresponding modular ring.
      GECODE_ES_CHECK(mult_inverse_prefix(home,x,y,z));
      GECODE_ES_CHECK(mult_inverse_prefix(home,y,x,z));

      changed = (old[0] != x.lo()) || (old[1] != x.hi()) ||
        (old[2] != y.lo()) || (old[3] != y.hi()) ||
        (old[4] != z.lo()) || (old[5] != z.hi());
    } while (changed);
    return ES_OK;
  }

  forceinline ExecStatus
  Mult::post(Home home, WordView x0, WordView x1, WordView x2) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    if (!(x0.assigned() && x1.assigned() && x2.assigned()))
      (void) new (home) Mult(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline ExecStatus
  Mult::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
