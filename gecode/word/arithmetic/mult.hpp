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
  mult_known_low(WordValue lo, WordValue hi, unsigned int width) {
    const WordValue unknown = hi & ~lo;
    unsigned int bits = 0;
    while ((bits < width) &&
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

  forceinline bool
  mult_narrow(WordValue& lo, WordValue& hi,
              WordValue next_lo, WordValue next_hi) {
    lo |= next_lo;
    hi &= next_hi;
    return (lo & ~hi) == 0;
  }

  forceinline bool
  mult_narrow_low(WordValue& lo, WordValue& hi, unsigned int bits,
                  WordValue value) {
    const WordValue field = mult_low_mask(bits);
    return mult_narrow(lo,hi,value & field,(value & field) | ~field);
  }

  forceinline bool
  mult_equal(WordValue& xlo, WordValue& xhi,
             WordValue& ylo, WordValue& yhi) {
    const WordValue lo = xlo | ylo;
    const WordValue hi = xhi & yhi;
    xlo=ylo=lo;
    xhi=yhi=hi;
    return (lo & ~hi) == 0;
  }

  forceinline void
  mult_range_hull(unsigned int width, WordValue minimum,
                  WordValue maximum, WordValue& lo, WordValue& hi) {
    WordValue varying = minimum^maximum;
    if (varying == 0) {
      lo=hi=minimum;
      return;
    }
    unsigned int bits = 0;
    while (varying != 0) {
      varying >>= 1;
      bits++;
    }
    varying=mult_low_mask(bits);
    lo=minimum&~varying;
    hi=(lo|varying)&mult_low_mask(width);
  }

  forceinline bool
  mult_in_cube(WordValue value, WordValue lo, WordValue hi) {
    return ((value&lo) == lo) && ((value&~hi) == 0);
  }

  forceinline bool
  mult_fixed_product_inverse(WordValue& xlo, WordValue& xhi,
                             WordValue& ylo, WordValue& yhi,
                             WordValue product, unsigned int width,
                             bool xy, bool xz, bool yz) {
    if (product == 0)
      return true;
    const WordValue mask = mult_low_mask(width);
    if ((xhi == 0) || (yhi == 0))
      return false;
    // The division guard proves xhi*yhi fits both the word and host type.
    if ((xhi != 0) && (yhi > mask/xhi))
      return true;

    WordValue xmin = product/yhi+(product%yhi != 0 ? 1U : 0U);
    WordValue xmax = (ylo == 0) ? xhi : product/ylo;
    WordValue ymin = product/xhi+(product%xhi != 0 ? 1U : 0U);
    WordValue ymax = (xlo == 0) ? yhi : product/xlo;
    xmin=std::max(xmin,xlo); xmax=std::min(xmax,xhi);
    ymin=std::max(ymin,ylo); ymax=std::min(ymax,yhi);
    if ((xmin > xmax) || (ymin > ymax))
      return false;

    WordValue lo, hi;
    mult_range_hull(width,xmin,xmax,lo,hi);
    if (!mult_narrow(xlo,xhi,lo,hi))
      return false;
    mult_range_hull(width,ymin,ymax,lo,hi);
    if (!mult_narrow(ylo,yhi,lo,hi))
      return false;

    const WordValue xspan = xmax-xmin;
    const WordValue yspan = ymax-ymin;
    const bool small_x = xspan < 64U;
    const bool small_y = yspan < 64U;
    if (!small_x && !small_y)
      return true;

    bool any = false;
    WordValue sxlo=0, sxhi=0, sylo=0, syhi=0;
    const bool enumerate_x = small_x && (!small_y || (xspan <= yspan));
    const WordValue first = enumerate_x ? xmin : ymin;
    const WordValue last = enumerate_x ? xmax : ymax;
    for (WordValue value=first;; value++) {
      if ((value != 0) && (product%value == 0)) {
        const WordValue other = product/value;
        const WordValue xv = enumerate_x ? value : other;
        const WordValue yv = enumerate_x ? other : value;
        if ((xv >= xmin) && (xv <= xmax) &&
            (yv >= ymin) && (yv <= ymax) &&
            mult_in_cube(xv,xlo,xhi) && mult_in_cube(yv,ylo,yhi) &&
            (!xy || (xv == yv)) && (!xz || (xv == product)) &&
            (!yz || (yv == product))) {
          if (!any) {
            sxlo=sxhi=xv; sylo=syhi=yv; any=true;
          } else {
            sxlo &= xv; sxhi |= xv;
            sylo &= yv; syhi |= yv;
          }
        }
      }
      if (value == last)
        break;
    }
    return any && mult_narrow(xlo,xhi,sxlo,sxhi) &&
      mult_narrow(ylo,yhi,sylo,syhi);
  }

  /** Propagate c*y=z modulo 2^bits for fixed low prefixes c and z. */
  forceinline bool
  mult_inverse_prefix(WordValue clo, WordValue chi,
                      WordValue& ylo, WordValue& yhi,
                      WordValue zlo, WordValue zhi,
                      unsigned int width) {
    const unsigned int bits =
      std::min(mult_known_low(clo,chi,width),
               mult_known_low(zlo,zhi,width));
    if (bits == 0)
      return true;
    const WordValue field = mult_low_mask(bits);
    const WordValue cv = clo & field;
    const WordValue zv = zlo & field;
    const unsigned int zeros = mult_trailing_zeros(cv,bits);
    if (zeros == bits) {
      if (zv != 0)
        return false;
      return true;
    }
    if ((zv & mult_low_mask(zeros)) != 0)
      return false;
    const unsigned int result_bits = bits-zeros;
    const WordValue odd = cv >> zeros;
    const WordValue result =
      ((zv >> zeros) * mult_inverse_odd(odd)) &
      mult_low_mask(result_bits);
    return mult_narrow_low(ylo,yhi,result_bits,result);
  }

  template<class View>
  forceinline ExecStatus
  mult_narrow_views(Home home, View x, View y, View z) {
    const unsigned int width = x.width();
    for (;;) {
      WordValue lo[] = {x.lo(),y.lo(),z.lo()};
      WordValue hi[] = {x.hi(),y.hi(),z.hi()};
      bool changed;
      do {
        const WordValue old_lo[] = {lo[0],lo[1],lo[2]};
        const WordValue old_hi[] = {hi[0],hi[1],hi[2]};

        if (((lo[0] == hi[0]) && (lo[0] == 0)) ||
            ((lo[1] == hi[1]) && (lo[1] == 0)))
          if (!mult_narrow(lo[2],hi[2],0,0))
            return ES_FAILED;
        if ((lo[0] == hi[0]) && (lo[0] == 1))
          if (!mult_equal(lo[1],hi[1],lo[2],hi[2]))
            return ES_FAILED;
        if ((lo[1] == hi[1]) && (lo[1] == 1))
          if (!mult_equal(lo[0],hi[0],lo[2],hi[2]))
            return ES_FAILED;

        if ((lo[2] == hi[2]) &&
            !mult_fixed_product_inverse(lo[0],hi[0],lo[1],hi[1],lo[2],width,
                                        x == y,x == z,y == z))
          return ES_FAILED;

        // Multiplication modulo 2^k depends only on the low k operand bits.
        const unsigned int known =
          std::min(mult_known_low(lo[0],hi[0],width),
                   mult_known_low(lo[1],hi[1],width));
        if (known != 0)
          if (!mult_narrow_low(lo[2],hi[2],known,lo[0]*lo[1]))
            return ES_FAILED;

        // Guaranteed powers of two in both operands force low product zeros.
        const unsigned int xz = mult_trailing_zeros(hi[0],width);
        const unsigned int yz = mult_trailing_zeros(hi[1],width);
        const unsigned int zeros = (xz > width-yz) ? width : xz+yz;
        if (zeros != 0)
          if (!mult_narrow_low(lo[2],hi[2],zeros,0))
            return ES_FAILED;

        // A fixed result prefix and multiplicand prefix determine the other
        // operand prefix after stripping powers of two and inverting the odd
        // factor in the corresponding modular ring.
        if (!mult_inverse_prefix(lo[0],hi[0],lo[1],hi[1],
                                 lo[2],hi[2],width) ||
            !mult_inverse_prefix(lo[1],hi[1],lo[0],hi[0],
                                 lo[2],hi[2],width))
          return ES_FAILED;

        if ((x == y) && !mult_equal(lo[0],hi[0],lo[1],hi[1]))
          return ES_FAILED;
        if ((x == z) && !mult_equal(lo[0],hi[0],lo[2],hi[2]))
          return ES_FAILED;
        if ((y == z) && !mult_equal(lo[1],hi[1],lo[2],hi[2]))
          return ES_FAILED;

        changed = (old_lo[0] != lo[0]) || (old_hi[0] != hi[0]) ||
          (old_lo[1] != lo[1]) || (old_hi[1] != hi[1]) ||
          (old_lo[2] != lo[2]) || (old_hi[2] != hi[2]);
      } while (changed);

      if ((x.lo() != lo[0]) || (x.hi() != hi[0]))
        GECODE_ME_CHECK(x.narrow(home,lo[0],hi[0]));
      if ((y.lo() != lo[1]) || (y.hi() != hi[1]))
        GECODE_ME_CHECK(y.narrow(home,lo[1],hi[1]));
      if ((z.lo() != lo[2]) || (z.hi() != hi[2]))
        GECODE_ME_CHECK(z.narrow(home,lo[2],hi[2]));
      if ((x.lo() != lo[0]) || (x.hi() != hi[0]) ||
          (y.lo() != lo[1]) || (y.hi() != hi[1]) ||
          (z.lo() != lo[2]) || (z.hi() != hi[2]))
        continue;
      return ES_OK;
    }
  }

  forceinline ExecStatus
  Mult::narrow(Home home, WordView x, WordView y, WordView z) {
    return mult_narrow_views(home,x,y,z);
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
