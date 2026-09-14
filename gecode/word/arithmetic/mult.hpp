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

  forceinline unsigned int
  count_limited_trailing_zeros(WordValue value, unsigned int limit) {
    return std::min(limit,Support::count_trailing_zeros_64(value));
  }

  forceinline unsigned int
  count_fixed_low_bits(WordValue lo, WordValue hi, unsigned int width) {
    return count_limited_trailing_zeros(hi & ~lo,width);
  }

  forceinline WordValue
  invert_odd_word(WordValue value) {
    WordValue inverse = 1;
    // Newton iteration doubles the number of correct low bits each time.
    for (unsigned int i=0; i<6; i++)
      inverse *= WordValue(2)-value*inverse;
    return inverse;
  }

  forceinline bool
  narrow_cube(WordValue& lo, WordValue& hi,
              WordValue next_lo, WordValue next_hi) {
    lo |= next_lo;
    hi &= next_hi;
    return (lo & ~hi) == 0;
  }

  forceinline bool
  narrow_low_bits(WordValue& lo, WordValue& hi, unsigned int bits,
                  WordValue value) {
    const WordValue field = width_mask(bits);
    return narrow_cube(lo,hi,value & field,(value & field) | ~field);
  }

  forceinline bool
  narrow_equal_cubes(WordValue& xlo, WordValue& xhi,
             WordValue& ylo, WordValue& yhi) {
    const WordValue lo = xlo | ylo;
    const WordValue hi = xhi & yhi;
    xlo=ylo=lo;
    xhi=yhi=hi;
    return (lo & ~hi) == 0;
  }

  forceinline WordCube
  compute_product_hull(unsigned int width, WordValue minimum,
                       WordValue maximum) {
    const WordValue varying=low_through_highest(minimum^maximum);
    const WordValue lo=minimum&~varying;
    return {lo,(lo|varying)&width_mask(width)};
  }

  forceinline bool
  contains_cube_value(WordValue value, WordCube cube) {
    return ((value&cube.lo) == cube.lo) && ((value&~cube.hi) == 0);
  }

  /// Equality relationships between the three multiplication roles
  struct ProductAliases {
    bool has_xy;
    bool has_xz;
    bool has_yz;
  };

  /// Nonwrapping numeric factor intervals and their intersected cube hulls
  struct ProductFactors {
    WordCube x, y;
    WordRankInterval x_interval, y_interval;
  };

  /// Distinguish an unavailable inverse deduction from contradiction or support
  struct ProductFactorResult {
    enum Status { UNAVAILABLE, FAILED, PREPARED };
    Status status;
    ProductFactors factors;
  };

  forceinline ProductFactorResult
  prepare_product_factors(WordCube x, WordCube y, WordValue product,
                           unsigned int width) {
    if (product == 0)
      return {ProductFactorResult::UNAVAILABLE,{}};
    const WordValue mask=width_mask(width);
    const bool has_zero_factor=(x.hi == 0) || (y.hi == 0);
    if (has_zero_factor)
      return {ProductFactorResult::FAILED,{}};
    // The division guard proves x.hi*y.hi fits both the word and host type.
    const bool can_wrap=(x.hi != 0) && (y.hi > mask/x.hi);
    if (can_wrap)
      return {ProductFactorResult::UNAVAILABLE,{}};

    WordValue xmin=product/y.hi+(product%y.hi != 0 ? 1U : 0U);
    WordValue xmax=(y.lo == 0) ? x.hi : product/y.lo;
    WordValue ymin=product/x.hi+(product%x.hi != 0 ? 1U : 0U);
    WordValue ymax=(x.lo == 0) ? y.hi : product/x.lo;
    xmin=std::max(xmin,x.lo); xmax=std::min(xmax,x.hi);
    ymin=std::max(ymin,y.lo); ymax=std::min(ymax,y.hi);
    const bool is_interval_empty=(xmin > xmax) || (ymin > ymax);
    if (is_interval_empty)
      return {ProductFactorResult::FAILED,{}};
    const WordCube x_hull=compute_product_hull(width,xmin,xmax);
    if (!narrow_cube(x.lo,x.hi,x_hull.lo,x_hull.hi))
      return {ProductFactorResult::FAILED,{}};
    const WordCube y_hull=compute_product_hull(width,ymin,ymax);
    if (!narrow_cube(y.lo,y.hi,y_hull.lo,y_hull.hi))
      return {ProductFactorResult::FAILED,{}};
    return {ProductFactorResult::PREPARED,{x,y,{xmin,xmax},{ymin,ymax}}};
  }

  forceinline bool
  is_product_tuple_admitted(const ProductFactors& factors, ProductAliases aliases,
                    WordValue product, WordValue x, WordValue y) {
    return (x >= factors.x_interval.minimum) &&
      (x <= factors.x_interval.maximum) &&
      (y >= factors.y_interval.minimum) &&
      (y <= factors.y_interval.maximum) &&
      contains_cube_value(x,factors.x) && contains_cube_value(y,factors.y) &&
      (!aliases.has_xy || (x == y)) && (!aliases.has_xz || (x == product)) &&
      (!aliases.has_yz || (y == product));
  }

  forceinline ProductFactorResult
  enumerate_product_factors(ProductFactors factors, WordValue product,
                             ProductAliases aliases) {
    const WordValue xspan=factors.x_interval.maximum-factors.x_interval.minimum;
    const WordValue yspan=factors.y_interval.maximum-factors.y_interval.minimum;
    // A span of 63 contains 64 values, including both interval endpoints.
    const WordValue factor_enumeration_limit=64U;
    const bool is_x_enumerable=xspan < factor_enumeration_limit;
    const bool is_y_enumerable=yspan < factor_enumeration_limit;
    const bool has_enumerable_factor=is_x_enumerable || is_y_enumerable;
    if (!has_enumerable_factor)
      return {ProductFactorResult::PREPARED,factors};
    const bool should_enumerate_x=
      is_x_enumerable && (!is_y_enumerable || (xspan <= yspan));
    const WordRankInterval interval=should_enumerate_x ?
      factors.x_interval : factors.y_interval;
    bool has_support=false;
    WordCube x={0,0}, y={0,0};
    for (WordValue value=interval.minimum;; value++) {
      const bool is_divisor=(value != 0) && (product%value == 0);
      if (is_divisor) {
        const WordValue other=product/value;
        const WordValue xv=should_enumerate_x ? value : other;
        const WordValue yv=should_enumerate_x ? other : value;
        if (is_product_tuple_admitted(factors,aliases,product,xv,yv)) {
          if (!has_support) {
            x={xv,xv}; y={yv,yv}; has_support=true;
          } else {
            x.lo &= xv; x.hi |= xv;
            y.lo &= yv; y.hi |= yv;
          }
        }
      }
      if (value == interval.maximum)
        break;
    }
    if (!has_support)
      return {ProductFactorResult::FAILED,{}};
    factors.x=x;
    factors.y=y;
    return {ProductFactorResult::PREPARED,factors};
  }

  /// Apply nonwrapping inverse bounds, then exact support when a factor is small
  forceinline bool
  narrow_fixed_product(WordValue& xlo, WordValue& xhi,
                        WordValue& ylo, WordValue& yhi,
                        WordValue product, unsigned int width,
                        ProductAliases aliases) {
    const ProductFactorResult prepared=
      prepare_product_factors({xlo,xhi},{ylo,yhi},product,width);
    if (prepared.status == ProductFactorResult::UNAVAILABLE)
      return true;
    if (prepared.status == ProductFactorResult::FAILED)
      return false;
    xlo=prepared.factors.x.lo; xhi=prepared.factors.x.hi;
    ylo=prepared.factors.y.lo; yhi=prepared.factors.y.hi;
    const ProductFactorResult support=
      enumerate_product_factors(prepared.factors,product,aliases);
    if (support.status == ProductFactorResult::FAILED)
      return false;
    if (!narrow_cube(xlo,xhi,support.factors.x.lo,support.factors.x.hi))
      return false;
    return narrow_cube(ylo,yhi,support.factors.y.lo,support.factors.y.hi);
  }

  /** Propagate c*y=z modulo 2^bits for fixed low prefixes c and z. */
  forceinline bool
  narrow_inverse_product_prefix(WordValue clo, WordValue chi,
                      WordValue& ylo, WordValue& yhi,
                      WordValue zlo, WordValue zhi,
                      unsigned int width) {
    const unsigned int bits =
      std::min(count_fixed_low_bits(clo,chi,width),
               count_fixed_low_bits(zlo,zhi,width));
    if (bits == 0)
      return true;
    const WordValue field = width_mask(bits);
    const WordValue cv = clo & field;
    const WordValue zv = zlo & field;
    const unsigned int zeros = count_limited_trailing_zeros(cv,bits);
    if (zeros == bits) {
      if (zv != 0)
        return false;
      return true;
    }
    if ((zv & width_mask(zeros)) != 0)
      return false;
    const unsigned int result_bits = bits-zeros;
    const WordValue odd = cv >> zeros;
    const WordValue result =
      ((zv >> zeros) * invert_odd_word(odd)) &
      width_mask(result_bits);
    return narrow_low_bits(ylo,yhi,result_bits,result);
  }

  /// Completed local multiplication closure before solver publication
  struct ProductSupportResult {
    bool has_support;
    BinaryCubes cubes;
  };

  forceinline ProductSupportResult
  compute_product_closure(unsigned int width, BinaryCubes cubes,
                           ProductAliases aliases) {
    WordValue* lo=cubes.lo;
    WordValue* hi=cubes.hi;
    bool has_changed;
    do {
      const BinaryCubes old=cubes;
      // An odd result provides low operand prefixes before either is assigned.
      if ((lo[2]&1U) != 0U) {
        if (!narrow_low_bits(lo[0],hi[0],1U,1U))
          return {false,{}};
        if (!narrow_low_bits(lo[1],hi[1],1U,1U))
          return {false,{}};
      }
      const bool has_zero_factor=((lo[0] == hi[0]) && (lo[0] == 0)) ||
        ((lo[1] == hi[1]) && (lo[1] == 0));
      if (has_zero_factor)
        if (!narrow_cube(lo[2],hi[2],0,0))
          return {false,{}};
      const bool is_x_one=(lo[0] == hi[0]) && (lo[0] == 1);
      if (is_x_one)
        if (!narrow_equal_cubes(lo[1],hi[1],lo[2],hi[2]))
          return {false,{}};
      const bool is_y_one=(lo[1] == hi[1]) && (lo[1] == 1);
      if (is_y_one)
        if (!narrow_equal_cubes(lo[0],hi[0],lo[2],hi[2]))
          return {false,{}};
      if (lo[2] == hi[2])
        if (!narrow_fixed_product(lo[0],hi[0],lo[1],hi[1],lo[2],width,aliases))
          return {false,{}};

      // Multiplication modulo 2^k depends only on the low k operand bits.
      const unsigned int known=std::min(count_fixed_low_bits(lo[0],hi[0],width),
                                        count_fixed_low_bits(lo[1],hi[1],width));
      if (known != 0)
        if (!narrow_low_bits(lo[2],hi[2],known,lo[0]*lo[1]))
          return {false,{}};
      // Guaranteed powers of two in both operands force low product zeros.
      const unsigned int xz=count_limited_trailing_zeros(hi[0],width);
      const unsigned int yz=count_limited_trailing_zeros(hi[1],width);
      const unsigned int zeros=(xz > width-yz) ? width : xz+yz;
      if (zeros != 0)
        if (!narrow_low_bits(lo[2],hi[2],zeros,0))
          return {false,{}};
      // Strip powers of two before inverting the odd factor in the modular ring.
      if (!narrow_inverse_product_prefix(lo[0],hi[0],lo[1],hi[1],
                                          lo[2],hi[2],width))
        return {false,{}};
      if (!narrow_inverse_product_prefix(lo[1],hi[1],lo[0],hi[0],
                                          lo[2],hi[2],width))
        return {false,{}};
      if (aliases.has_xy)
        if (!narrow_equal_cubes(lo[0],hi[0],lo[1],hi[1]))
          return {false,{}};
      if (aliases.has_xz)
        if (!narrow_equal_cubes(lo[0],hi[0],lo[2],hi[2]))
          return {false,{}};
      if (aliases.has_yz)
        if (!narrow_equal_cubes(lo[1],hi[1],lo[2],hi[2]))
          return {false,{}};
      has_changed=(old.lo[0] != lo[0]) || (old.hi[0] != hi[0]) ||
        (old.lo[1] != lo[1]) || (old.hi[1] != hi[1]) ||
        (old.lo[2] != lo[2]) || (old.hi[2] != hi[2]);
    } while (has_changed);
    return {true,cubes};
  }

  /// Alternate local product closure and publication until actual cubes agree
  template<class View>
  forceinline ExecStatus
  narrow_product(Home home, View x, View y, View z) {
    const unsigned int width=x.width();
    const ProductAliases aliases={x == y,x == z,y == z};
    for (;;) {
      const BinaryCubes input={{x.lo(),y.lo(),z.lo()},
                                {x.hi(),y.hi(),z.hi()}};
      const ProductSupportResult support=
        compute_product_closure(width,input,aliases);
      if (!support.has_support)
        return ES_FAILED;
      const CubePublicationResult publication=
        publish_binary_cubes(home,x,y,z,support.cubes);
      if (publication == CPR_FAILED)
        return ES_FAILED;
      if (publication == CPR_STABLE)
        return ES_OK;
    }
  }

  forceinline ExecStatus
  Mult::narrow(Home home, WordView x, WordView y, WordView z) {
    return narrow_product(home,x,y,z);
  }

  forceinline ExecStatus
  Mult::post(Home home, WordView x0, WordView x1, WordView x2) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool has_only_assigned_views=
      x0.assigned() && x1.assigned() && x2.assigned();
    if (!has_only_assigned_views)
      (void) new (home) Mult(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline ExecStatus
  Mult::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool has_only_assigned_views=
      x0.assigned() && x1.assigned() && x2.assigned();
    if (has_only_assigned_views)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
