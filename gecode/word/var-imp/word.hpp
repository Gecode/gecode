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

namespace Gecode { namespace Word {
  forceinline WordValue width_mask(unsigned int width) {
    return (width >= 64U) ? ~WordValue(0)
                          : ((WordValue(1) << width) - WordValue(1));
  }

  /** \brief Return \a value taken modulo \f$2^{width}\f$
   *
   * Throws OutOfLimits at \a location unless \a value fits \a width as an
   * unsigned number or, read as a 64-bit two's-complement number, as a
   * signed number. A \a width of 64 or more admits every value.
   */
  forceinline WordValue fit_to_width(unsigned int width, WordValue value,
                                     const char* location) {
    if (width >= 64U)
      return value;
    const WordValue unsigned_limit = WordValue(1) << width;
    const bool fits_unsigned = value < unsigned_limit;
    // A negative reading v fits when v >= -2^(width-1), that is ~v < 2^(width-1).
    const bool fits_negative = ~value < (unsigned_limit >> 1);
    if (!fits_unsigned && !fits_negative)
      throw OutOfLimits(location);
    return value & (unsigned_limit - 1U);
  }

  forceinline void check_domain(unsigned int width, WordValue lo,
                                WordValue hi, const char* location) {
    if ((width == 0U) || (width > 64U))
      throw OutOfLimits(location);
    const WordValue m = width_mask(width);
    if (((lo | hi) & ~m) != 0)
      throw OutOfLimits(location);
    if ((lo & ~hi) != 0)
      throw VariableEmptyDomain(location);
  }

  forceinline WordValue low_through_highest(WordValue value) {
    // The zero count is 64 for zero; do not shift by the word width.
    return value ?
      (~WordValue(0) >> Support::count_leading_zeros_64(value)) : 0;
  }

  forceinline bool cube_successor(WordValue lo, WordValue hi,
                                       WordValue bound, WordValue mask,
                                       WordValue& result) {
    const WordValue conflicts = ((bound & ~hi) | (~bound & lo)) & mask;
    if (conflicts == 0) {
      result = bound;
      return true;
    }
    const WordValue through_highest = low_through_highest(conflicts);
    const WordValue highest = through_highest ^ (through_highest >> 1);
    WordValue step;
    if ((highest & lo) != 0) {
      step = highest;
    } else {
      const WordValue above = ~(highest | (highest - 1)) & mask;
      const WordValue eligible = (hi & ~lo) & ~bound & above;
      if (eligible == 0)
        return false;
      step = eligible & (WordValue(0) - eligible);
    }
    const WordValue lower = step - 1;
    result = (bound & ~(step | lower)) | step | (lo & lower);
    return true;
  }

  forceinline bool cube_predecessor(WordValue lo, WordValue hi,
                                         WordValue bound, WordValue mask,
                                         WordValue& result) {
    WordValue complement;
    if (!cube_successor(mask ^ hi,mask ^ lo,mask ^ bound,mask,complement))
      return false;
    result = mask ^ complement;
    return true;
  }

  forceinline WordValue sign_bit(unsigned int width) {
    return WordValue(1) << (width-1);
  }

  /// Integer order in which integer reasoning may read a word's rank interval
  enum class IntegerRankOrder {
    NONE,        ///< No rank interval (WDT_CUBE)
    UNSIGNED,    ///< Ranks are unsigned values (WDT_UNSIGNED)
    SIGNED,      ///< Ranks are signed values with the sign bit flipped
    NON_INTEGER  ///< Ranks in an order that integer reasoning must not read
  };

  /// Test whether \a domain_type is a float format; false for a value that
  /// is not a WordDomainType
  forceinline bool
  is_float_domain_type(WordDomainType domain_type) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED: case WDT_SIGNED:
      return false;
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return true;
    }
    return false;
  }

  /// Return the width of float \a domain_type, or 0 for another type
  forceinline unsigned int
  float_domain_width(WordDomainType domain_type) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED: case WDT_SIGNED:
      return 0;
    case WDT_FP32:
      return 32;
    case WDT_BF16: case WDT_BINARY16:
      return 16;
    case WDT_E4M3FN:
      return 8;
    case WDT_E2M1:
      return 4;
    }
    GECODE_NEVER;
    return 0;
  }

  /// Test whether WVS_DEFAULT gives a \a domain_type word a value set
  forceinline bool
  has_default_value_set(WordDomainType domain_type) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED: case WDT_SIGNED: case WDT_FP32:
    case WDT_BF16: case WDT_BINARY16:
      return false;
    case WDT_E4M3FN: case WDT_E2M1:
      return true;
    }
    GECODE_NEVER;
    return false;
  }

  /// Return \a value_set for a \a domain_type word as WVS_NONE or WVS_EXACT
  forceinline WordValueSetOption
  resolve_value_set(WordDomainType domain_type,
                    WordValueSetOption value_set) {
    if (value_set != WVS_DEFAULT)
      return value_set;
    return has_default_value_set(domain_type) ? WVS_EXACT : WVS_NONE;
  }

  /** \brief Return the integer order of the ranks of a \a domain_type word
   *
   * A value outside WordDomainType has no integer order (NON_INTEGER).
   */
  forceinline IntegerRankOrder
  integer_rank_order(WordDomainType domain_type) {
    switch (domain_type) {
    case WDT_CUBE:
      return IntegerRankOrder::NONE;
    case WDT_UNSIGNED:
      return IntegerRankOrder::UNSIGNED;
    case WDT_SIGNED:
      return IntegerRankOrder::SIGNED;
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return IntegerRankOrder::NON_INTEGER;
    }
    return IntegerRankOrder::NON_INTEGER;
  }

  /// Return the integer rank order shared by \a a and \a b, or NONE if none
  forceinline IntegerRankOrder
  common_integer_rank_order(WordDomainType a, WordDomainType b) {
    return (a == b) ? integer_rank_order(a) : IntegerRankOrder::NONE;
  }

  /// Return the integer rank order shared by \a a, \a b and \a c, or NONE
  forceinline IntegerRankOrder
  common_integer_rank_order(WordDomainType a, WordDomainType b,
                            WordDomainType c) {
    return (b == c) ? common_integer_rank_order(a,b) : IntegerRankOrder::NONE;
  }

  /// Test whether integer reasoning may read the ranks of a \a domain_type word
  forceinline bool
  has_integer_rank_order(WordDomainType domain_type) {
    switch (integer_rank_order(domain_type)) {
    case IntegerRankOrder::UNSIGNED: case IntegerRankOrder::SIGNED:
      return true;
    case IntegerRankOrder::NONE: case IntegerRankOrder::NON_INTEGER:
      return false;
    }
    GECODE_NEVER;
    return false;
  }

  /** \brief Return the domain type of a word that integer reasoning creates
   * for an operand of type \a domain_type
   *
   * Temporaries keep an integer order and are cube words otherwise.
   */
  forceinline WordDomainType
  intermediate_domain_type(WordDomainType domain_type) {
    switch (integer_rank_order(domain_type)) {
    case IntegerRankOrder::NONE: case IntegerRankOrder::UNSIGNED:
    case IntegerRankOrder::SIGNED:
      return domain_type;
    case IntegerRankOrder::NON_INTEGER:
      return WDT_CUBE;
    }
    GECODE_NEVER;
    return WDT_CUBE;
  }

  /// Test whether a \a domain_type word keeps a rank interval
  forceinline bool
  is_bounded_domain_type(WordDomainType domain_type) {
    switch (domain_type) {
    case WDT_CUBE:
      return false;
    case WDT_UNSIGNED: case WDT_SIGNED: case WDT_FP32: case WDT_BF16:
    case WDT_BINARY16: case WDT_E4M3FN: case WDT_E2M1:
      return true;
    }
    return false;
  }

  /*
   * The float order is the IEEE 754 total order. Let s be the sign bit and
   * m the width mask. An encoding v with the sign bit set has rank v ^ m,
   * so the most negative values come first; any other encoding has rank
   * v ^ s. Unlike the signed order, the map is not its own inverse: a rank
   * with the sign bit set decodes with ^ s, any other rank with ^ m.
   */

  /** \brief Return the rank of \a value in the order of \a domain_type
   *
   * The inverse is encode_rank().
   */
  forceinline WordValue rank(WordDomainType domain_type,
                             unsigned int width, WordValue value) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return value;
    case WDT_SIGNED:
      return value ^ sign_bit(width);
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1: {
      const WordValue s = sign_bit(width);
      return ((value & s) != 0) ? (~value & width_mask(width)) : (value | s);
    }
    }
    GECODE_NEVER;
    return value;
  }

  /// Return the encoding whose rank in the order of \a domain_type is \a r
  forceinline WordValue encode_rank(WordDomainType domain_type,
                                    unsigned int width, WordValue r) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return r;
    case WDT_SIGNED:
      return r ^ sign_bit(width);
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1: {
      const WordValue s = sign_bit(width);
      return ((r & s) != 0) ? (r & ~s) : (~r & width_mask(width));
    }
    }
    GECODE_NEVER;
    return r;
  }

  /** \brief Return the ranks of the encodings \a first to \a last in the
   * order of \a domain_type
   *
   * For a float \a domain_type the encodings must share their sign bit.
   * Within a sign half the rank then falls as the encoding rises on the
   * negative side and rises on the positive side, so the result is the
   * least and greatest of the two endpoint ranks, never empty.
   */
  forceinline WordRankInterval
  rank_interval(WordDomainType domain_type, unsigned int width,
                WordValue first, WordValue last) {
    const WordValue a = rank(domain_type,width,first);
    const WordValue b = rank(domain_type,width,last);
    return {std::min(a,b),std::max(a,b)};
  }

  /** \brief Return the encodings of the ranks in \a interval in the order of
   * \a domain_type
   *
   * For a float \a domain_type the interval must lie within one sign half,
   * either below or from the sign bit up. The result is the least and
   * greatest of the two endpoint encodings as unsigned values.
   */
  forceinline WordEncodingRange
  encoding_range(WordDomainType domain_type, unsigned int width,
                 WordRankInterval interval) {
    const WordValue a = encode_rank(domain_type,width,interval.minimum);
    const WordValue b = encode_rank(domain_type,width,interval.maximum);
    return {std::min(a,b),std::max(a,b)};
  }

  /// Return \a cube with the known bits among \a flip inverted
  forceinline WordCube xor_cube(WordCube cube, WordValue flip) {
    const WordValue known_flip = flip & ~(cube.lo ^ cube.hi);
    return {cube.lo ^ known_flip,cube.hi ^ known_flip};
  }

  /** \brief Test whether a \a domain_type cube maps to two rank cubes
   *
   * The float order inverts negative and positive encodings differently,
   * so a float cube whose sign bit is unknown maps to one rank cube per
   * sign half.
   */
  forceinline bool
  splits_by_sign(WordDomainType domain_type, unsigned int width,
                 WordCube cube) {
    return is_float_domain_type(domain_type) &&
      ((cube.hi & ~cube.lo & sign_bit(width)) != 0);
  }

  /// The two halves of a cube whose sign bit is unknown
  struct SignHalves {
    WordCube with_sign;    ///< Half with the sign bit one
    WordCube without_sign; ///< Half with the sign bit zero
  };

  /// Split \a cube, whose sign bit \a s is unknown, into its halves
  forceinline SignHalves
  sign_halves(WordCube cube, WordValue s) {
    return {{cube.lo | s,cube.hi},{cube.lo,cube.hi & ~s}};
  }

  /// Return the bit hull of cubes \a a and \a b
  forceinline WordCube
  cube_hull(WordCube a, WordCube b) {
    return {a.lo & b.lo,a.hi | b.hi};
  }

  /** \brief Map an encoding cube to its rank cube in float order
   *
   * The result is exact when the sign bit is known, and otherwise the hull
   * of the rank cubes of both sign halves.
   */
  forceinline WordCube
  order_float_cube(unsigned int width, WordCube cube) {
    const WordValue s = sign_bit(width);
    const WordValue m = width_mask(width);
    if ((cube.hi & s) == 0)
      return xor_cube(cube,s);
    if ((cube.lo & s) != 0)
      return xor_cube(cube,m);
    const SignHalves halves = sign_halves(cube,s);
    return cube_hull(xor_cube(halves.with_sign,m),
                     xor_cube(halves.without_sign,s));
  }

  /** \brief Map a rank cube in float order to its encoding cube
   *
   * The result is exact when the sign bit is known, and otherwise the hull
   * of the encoding cubes of both rank halves.
   */
  forceinline WordCube
  encode_float_cube(unsigned int width, WordCube cube) {
    const WordValue s = sign_bit(width);
    const WordValue m = width_mask(width);
    if ((cube.hi & s) == 0)
      return xor_cube(cube,m);
    if ((cube.lo & s) != 0)
      return xor_cube(cube,s);
    const SignHalves halves = sign_halves(cube,s);
    return cube_hull(xor_cube(halves.with_sign,s),
                     xor_cube(halves.without_sign,m));
  }

  /** \brief Map an encoding cube to its cube of ranks
   *
   * The map is exact unless splits_by_sign(), where the result is a
   * superset.
   */
  forceinline WordCube
  order_cube(WordDomainType domain_type, unsigned int width, WordCube cube) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return cube;
    case WDT_SIGNED:
      return xor_cube(cube,sign_bit(width));
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return order_float_cube(width,cube);
    }
    GECODE_NEVER;
    return cube;
  }

  /** \brief Map a cube of ranks to its encoding cube
   *
   * The map is exact when the rank cube fixes the sign bit or the order is
   * not a float order, and otherwise the result is a superset.
   */
  forceinline WordCube
  encode_cube(WordDomainType domain_type, unsigned int width, WordCube cube) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return cube;
    case WDT_SIGNED:
      return xor_cube(cube,sign_bit(width));
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return encode_float_cube(width,cube);
    }
    GECODE_NEVER;
    return cube;
  }

  forceinline bool cube_contains(WordValue lo, WordValue hi,
                                 WordValue value, WordValue mask) {
    return ((value & ~mask) == 0) && ((value & lo) == lo) &&
      ((value & ~hi) == 0);
  }

  forceinline void
  check_value_set_width(unsigned int width, WordValueSetOption value_set,
                        const char* location) {
    if ((value_set == WVS_EXACT) && (width > max_value_set_width))
      throw OutOfLimits(location);
  }

  /** \brief Return \a value_set for a \a domain_type word of \a width as
   * WVS_NONE or WVS_EXACT
   *
   * Throws OutOfLimits when the result is WVS_EXACT and \a width is too
   * wide for a value set.
   */
  forceinline WordValueSetOption
  checked_value_set(unsigned int width, WordDomainType domain_type,
                    WordValueSetOption value_set, const char* location) {
    const WordValueSetOption resolved =
      resolve_value_set(domain_type,value_set);
    check_value_set_width(width,resolved,location);
    return resolved;
  }

  /*
   * Value sets are closed with word-parallel operations. Within a block of
   * 64 values, encoding bit b < 6 of value v is bit b of v % 64, so the
   * values with that bit one are a constant mask. Encoding bits 6 and 7 are
   * the bits of the block index.
   */

  /// Values in a block of 64 whose encoding bit b is one, for b < 6
  const WordValue value_bit_masks[6] = {
    0xAAAAAAAAAAAAAAAAULL, 0xCCCCCCCCCCCCCCCCULL, 0xF0F0F0F0F0F0F0F0ULL,
    0xFF00FF00FF00FF00ULL, 0xFFFF0000FFFF0000ULL, 0xFFFFFFFF00000000ULL
  };
  /// Number of encoding bits that select a value within a block
  const unsigned int block_value_bits = 6;

  forceinline unsigned int value_set_block_count(unsigned int width) {
    return (width <= block_value_bits) ?
      1U : (1U << (width-block_value_bits));
  }

  /// Return the encoding bits of \a width that select a block
  forceinline WordValue block_index_bits(unsigned int width) {
    return width_mask(width) & ~width_mask(block_value_bits);
  }

  forceinline WordValueSet full_value_set(unsigned int width) {
    WordValueSet set = {{0,0,0,0}};
    const unsigned int count = value_set_block_count(width);
    const WordValue block =
      width_mask(1U << std::min(width,block_value_bits));
    for (unsigned int i=0; i<count; i++)
      set.blocks[i] = block;
    return set;
  }

  forceinline WordValueSet singleton_value_set(WordValue value) {
    WordValueSet set = {{0,0,0,0}};
    set.blocks[value >> block_value_bits] = WordValue(1) << (value & 63U);
    return set;
  }

  forceinline bool
  value_set_contains(const WordValueSet& set, WordValue value) {
    return ((set.blocks[value >> block_value_bits] >> (value & 63U)) & 1U)
      != 0;
  }

  forceinline bool is_empty_value_set(const WordValueSet& set) {
    return (set.blocks[0] | set.blocks[1] | set.blocks[2] | set.blocks[3])
      == 0;
  }

  forceinline bool
  same_value_sets(const WordValueSet& a, const WordValueSet& b) {
    return (a.blocks[0] == b.blocks[0]) && (a.blocks[1] == b.blocks[1]) &&
      (a.blocks[2] == b.blocks[2]) && (a.blocks[3] == b.blocks[3]);
  }

  forceinline WordValueSet
  intersect_value_sets(const WordValueSet& a, const WordValueSet& b) {
    return {{a.blocks[0] & b.blocks[0], a.blocks[1] & b.blocks[1],
             a.blocks[2] & b.blocks[2], a.blocks[3] & b.blocks[3]}};
  }

  /// Return the values of \a a that \a b excludes
  forceinline WordValueSet
  subtract_value_sets(const WordValueSet& a, const WordValueSet& b) {
    return {{a.blocks[0] & ~b.blocks[0], a.blocks[1] & ~b.blocks[1],
             a.blocks[2] & ~b.blocks[2], a.blocks[3] & ~b.blocks[3]}};
  }

  forceinline WordValueSet
  without_value(WordValueSet set, WordValue value) {
    set.blocks[value >> block_value_bits] &= ~(WordValue(1) << (value & 63U));
    return set;
  }

  forceinline WordValueSet
  with_value(WordValueSet set, WordValue value) {
    set.blocks[value >> block_value_bits] |= WordValue(1) << (value & 63U);
    return set;
  }

  /// Keep the values of \a set that \a cube admits
  forceinline WordValueSet
  restrict_value_set_to_cube(WordValueSet set, unsigned int width,
                             WordCube cube) {
    const unsigned int value_bits = std::min(width,block_value_bits);
    WordValue admitted_in_block = ~WordValue(0);
    for (unsigned int b=0; b<value_bits; b++) {
      const WordValue bit = WordValue(1) << b;
      if ((cube.lo & bit) != 0)
        admitted_in_block &= value_bit_masks[b];
      else if ((cube.hi & bit) == 0)
        admitted_in_block &= ~value_bit_masks[b];
    }
    const WordValue index_bits = block_index_bits(width);
    const unsigned int count = value_set_block_count(width);
    for (unsigned int i=0; i<count; i++) {
      const WordValue index = WordValue(i) << block_value_bits;
      const bool is_block_admitted = ((index & ~cube.hi) == 0) &&
        ((~index & cube.lo & index_bits) == 0);
      set.blocks[i] &= is_block_admitted ? admitted_in_block : 0;
    }
    return set;
  }

  /// Return the bit hull of the nonempty \a set
  forceinline WordCube
  value_set_hull(const WordValueSet& set, unsigned int width) {
    WordValue occupied = 0;
    WordValue index_ones = 0;
    WordValue index_all_ones = block_index_bits(width);
    const unsigned int count = value_set_block_count(width);
    for (unsigned int i=0; i<count; i++)
      if (set.blocks[i] != 0) {
        const WordValue index = WordValue(i) << block_value_bits;
        occupied |= set.blocks[i];
        index_ones |= index;
        index_all_ones &= index;
      }
    WordCube hull = {index_all_ones,index_ones};
    const unsigned int value_bits = std::min(width,block_value_bits);
    for (unsigned int b=0; b<value_bits; b++) {
      const WordValue bit = WordValue(1) << b;
      const bool has_one = (occupied & value_bit_masks[b]) != 0;
      const bool has_zero = (occupied & ~value_bit_masks[b]) != 0;
      if (has_one)
        hull.hi |= bit;
      if (has_one && !has_zero)
        hull.lo |= bit;
    }
    return hull;
  }

  /// Swap the upper and lower half of the values of \a set
  forceinline WordValueSet
  swap_value_set_halves(unsigned int width, WordValueSet set) {
    if (width <= block_value_bits) {
      // Rotate the 2^width values of the only block by half their number.
      const unsigned int half = 1U << (width-1);
      const WordValue block = set.blocks[0];
      set.blocks[0] = ((block & width_mask(half)) << half) | (block >> half);
      return set;
    }
    // The sign bit selects the upper half of the blocks.
    const unsigned int half = value_set_block_count(width) / 2;
    for (unsigned int i=0; i<half; i++)
      std::swap(set.blocks[i],set.blocks[i+half]);
    return set;
  }

  /// Return \a x with the order of its 64 bits reversed
  forceinline WordValue reverse_bits(WordValue x) {
    x = ((x >> 1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) << 1);
    x = ((x >> 2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) << 2);
    x = ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((x & 0x0F0F0F0F0F0F0F0FULL) << 4);
    x = ((x >> 8) & 0x00FF00FF00FF00FFULL) | ((x & 0x00FF00FF00FF00FFULL) << 8);
    x = ((x >> 16) & 0x0000FFFF0000FFFFULL) |
      ((x & 0x0000FFFF0000FFFFULL) << 16);
    return (x >> 32) | (x << 32);
  }

  /// Reverse the order of the values in the lower half of \a set
  forceinline WordValueSet
  reverse_lower_half(unsigned int width, WordValueSet set) {
    const unsigned int half_bits = width-1;
    if (half_bits <= block_value_bits) {
      const unsigned int half = 1U << half_bits;
      const WordValue lower = set.blocks[0] & width_mask(half);
      set.blocks[0] = (set.blocks[0] & ~width_mask(half)) |
        (reverse_bits(lower) >> (64U - half));
      return set;
    }
    const unsigned int count = 1U << (half_bits-block_value_bits);
    for (unsigned int i=0; i<count/2; i++)
      std::swap(set.blocks[i],set.blocks[count-1-i]);
    for (unsigned int i=0; i<count; i++)
      set.blocks[i] = reverse_bits(set.blocks[i]);
    return set;
  }

  /** \brief Move every value of \a set from its encoding to its rank position
   *
   * For the float order, swapping the halves moves the negative encodings
   * below the positive ones, and reversing the lower half orders them by
   * decreasing magnitude.
   */
  forceinline WordValueSet
  order_value_set(WordDomainType domain_type, unsigned int width,
                  const WordValueSet& set) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return set;
    case WDT_SIGNED:
      return swap_value_set_halves(width,set);
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return reverse_lower_half(width,swap_value_set_halves(width,set));
    }
    GECODE_NEVER;
    return set;
  }

  /// Move every value of \a set from its rank to its encoding position
  forceinline WordValueSet
  encode_value_set(WordDomainType domain_type, unsigned int width,
                   const WordValueSet& set) {
    switch (domain_type) {
    case WDT_CUBE: case WDT_UNSIGNED:
      return set;
    case WDT_SIGNED:
      return swap_value_set_halves(width,set);
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return swap_value_set_halves(width,reverse_lower_half(width,set));
    }
    GECODE_NEVER;
    return set;
  }

  /// Return the least and greatest position in the nonempty \a ordered set
  forceinline WordRankInterval
  value_set_rank_interval(const WordValueSet& ordered, unsigned int width) {
    const unsigned int count = value_set_block_count(width);
    WordRankInterval interval = {0,0};
    for (unsigned int i=0; i<count; i++)
      if (ordered.blocks[i] != 0) {
        interval.minimum = (WordValue(i) << block_value_bits) +
          Support::count_trailing_zeros_64(ordered.blocks[i]);
        break;
      }
    for (unsigned int i=count; i--; )
      if (ordered.blocks[i] != 0) {
        interval.maximum = (WordValue(i) << block_value_bits) + 63U -
          Support::count_leading_zeros_64(ordered.blocks[i]);
        break;
      }
    return interval;
  }

  /// Return the number of values in \a set
  forceinline unsigned int value_set_size(const WordValueSet& set) {
    unsigned int size = 0;
    for (WordValue block : set.blocks)
      for (; block != 0; block &= block-1)
        size++;
    return size;
  }

  /// Return the element of \a set with \a index elements below it
  forceinline WordValue
  value_set_element(const WordValueSet& set, unsigned int index) {
    for (unsigned int i=0; i<4; i++)
      for (WordValue block = set.blocks[i]; block != 0; block &= block-1) {
        if (index == 0)
          return (WordValue(i) << block_value_bits) +
            Support::count_trailing_zeros_64(block);
        index--;
      }
    GECODE_NEVER;
    return 0;
  }

  /// Keep the positions of the rank-ordered set \a ordered inside \a interval
  forceinline WordValueSet
  restrict_ordered_value_set(WordValueSet ordered, WordRankInterval interval) {
    for (unsigned int i=0; i<4; i++) {
      const WordValue first = WordValue(i) << block_value_bits;
      const WordValue last = first + 63U;
      if ((interval.maximum < first) || (interval.minimum > last)) {
        ordered.blocks[i] = 0;
        continue;
      }
      if (interval.minimum > first)
        ordered.blocks[i] &= ~WordValue(0) << (interval.minimum - first);
      if (interval.maximum < last)
        ordered.blocks[i] &= ~WordValue(0) >> (last - interval.maximum);
    }
    return ordered;
  }

  /// Keep the values of \a set whose rank lies in \a interval
  forceinline WordValueSet
  restrict_value_set_to_ranks(WordDomainType domain_type, unsigned int width,
                              const WordValueSet& set,
                              WordRankInterval interval) {
    return encode_value_set(domain_type,width,restrict_ordered_value_set(
      order_value_set(domain_type,width,set),interval));
  }

  /// Return the values of \a domain; its interval counts only when bounded
  forceinline WordValueSet
  domain_values(unsigned int width, WordDomainType domain_type,
                WordDomain domain) {
    const WordValueSet in_cube =
      restrict_value_set_to_cube(full_value_set(width),width,domain.cube);
    if (domain_type == WDT_CUBE)
      return in_cube;
    return restrict_value_set_to_ranks(domain_type,width,in_cube,
                                       domain.interval);
  }

  forceinline WordIntervalResult
  find_admitted_interval(WordCube cube, WordRankInterval interval,
                         WordValue mask) {
    WordValue first, last;
    if (!cube_successor(cube.lo,cube.hi,interval.minimum,mask,first))
      return {false,{0,0}};
    if (!cube_predecessor(cube.lo,cube.hi,interval.maximum,mask,last))
      return {false,{0,0}};
    if (first > last)
      return {false,{0,0}};
    return {true,{first,last}};
  }

  forceinline WordCube
  tighten_cube_prefix(WordCube cube, WordRankInterval interval,
                       WordValue mask) {
    const WordValue difference = interval.minimum ^ interval.maximum;
    const WordValue fixed = mask & ~low_through_highest(difference);
    cube.lo |= interval.minimum & fixed;
    cube.hi &= interval.minimum | ~fixed;
    return cube;
  }

  /** \brief Close a cube/interval intersection whose cube maps to one rank
   * cube
   *
   * The result is the bit hull of the values of \a domain and their least
   * and greatest rank.
   */
  forceinline WordDomainResult
  close_ordered_cube(unsigned int width, WordDomainType domain_type,
                     WordDomain domain) {
    const WordValue mask = width_mask(width);
    const WordCube ordered = order_cube(domain_type,width,domain.cube);
    const WordIntervalResult admitted =
      find_admitted_interval(ordered,domain.interval,mask);
    if (!admitted.is_admitted)
      return {false,{{0,0},{0,0}}};
    const WordCube tightened =
      tighten_cube_prefix(ordered,admitted.interval,mask);
    return {true,{encode_cube(domain_type,width,tightened),admitted.interval}};
  }

  /** \brief Close a cube/interval intersection without publishing any
   * variable change
   *
   * The result is the bit hull of the values of \a domain and their least
   * and greatest rank. A float cube whose sign bit is unknown is closed one
   * sign half at a time: negative encodings have the ranks below the sign
   * bit, and the others the ranks from the sign bit up.
   */
  forceinline WordDomainResult
  compute_domain_closure(unsigned int width, WordDomainType domain_type,
                          WordDomain domain) {
    if (!splits_by_sign(domain_type,width,domain.cube))
      return close_ordered_cube(width,domain_type,domain);
    const WordValue s = sign_bit(width);
    const SignHalves halves = sign_halves(domain.cube,s);
    const WordRankInterval interval = domain.interval;
    WordDomainResult negative = {false,{{0,0},{0,0}}};
    if (interval.minimum < s)
      negative = close_ordered_cube(
        width,domain_type,
        {halves.with_sign,{interval.minimum,std::min(interval.maximum,s-1)}});
    WordDomainResult positive = {false,{{0,0},{0,0}}};
    if (interval.maximum >= s)
      positive = close_ordered_cube(
        width,domain_type,
        {halves.without_sign,{std::max(interval.minimum,s),interval.maximum}});
    if (!positive.is_consistent)
      return negative;
    if (!negative.is_consistent)
      return positive;
    return {true,{cube_hull(negative.domain.cube,positive.domain.cube),
                  {negative.domain.interval.minimum,
                   positive.domain.interval.maximum}}};
  }

  /// Commit closure into local domain storage only after successful calculation
  forceinline bool synchronize_domain(unsigned int width,
                                      WordDomainType domain_type,
                                      WordValue& lo, WordValue& hi,
                                      WordValue& minimum,
                                      WordValue& maximum) {
    const WordDomainResult result =
      compute_domain_closure(width,domain_type,{{lo,hi},{minimum,maximum}});
    if (!result.is_consistent)
      return false;
    lo = result.domain.cube.lo;
    hi = result.domain.cube.hi;
    minimum = result.domain.interval.minimum;
    maximum = result.domain.interval.maximum;
    return true;
  }

  forceinline WordDomain
  intersect_word_domains(WordDomain current, WordDomain requested) {
    return {{current.cube.lo | requested.cube.lo,
             current.cube.hi & requested.cube.hi},
            {std::max(current.interval.minimum,requested.interval.minimum),
             std::min(current.interval.maximum,requested.interval.maximum)}};
  }

  forceinline ModEvent
  classify_domain_change(WordDomain current, WordDomain next) {
    const bool has_new_bits = (next.cube.lo != current.cube.lo) ||
      (next.cube.hi != current.cube.hi);
    const bool has_new_bounds =
      (next.interval.minimum != current.interval.minimum) ||
      (next.interval.maximum != current.interval.maximum);
    if (!has_new_bits && !has_new_bounds)
      return ME_WORD_NONE;
    if (next.cube.lo == next.cube.hi)
      return ME_WORD_VAL;
    if (has_new_bits) {
      if (has_new_bounds)
        return ME_WORD_DOM;
      return ME_WORD_BITS;
    }
    return ME_WORD_BND;
  }

  /** \brief Test whether \a interval is closed for \a cube, so that only
   * the bounds change
   *
   * A float cube with an unknown sign bit never qualifies: an interval
   * within one sign half fixes the sign bit, and one that crosses it can
   * tighten the cubes of both halves.
   */
  forceinline bool
  can_publish_bounds_only(unsigned int width, WordDomainType domain_type,
                           WordCube cube, WordRankInterval interval) {
    if (splits_by_sign(domain_type,width,cube))
      return false;
    const WordValue mask = width_mask(width);
    const WordCube ordered = order_cube(domain_type,width,cube);
    const WordValue difference = interval.minimum ^ interval.maximum;
    const WordValue fixed = mask & ~low_through_highest(difference);
    const bool are_endpoints_admitted =
      cube_contains(ordered.lo,ordered.hi,interval.minimum,mask) &&
      cube_contains(ordered.lo,ordered.hi,interval.maximum,mask);
    const bool is_prefix_unchanged =
      ((interval.minimum & fixed & ~ordered.lo) == 0) &&
      (((~interval.minimum) & fixed & ordered.hi) == 0);
    return are_endpoints_admitted && is_prefix_unchanged;
  }

  /// Throw OutOfLimits when float \a domain_type has a width other than \a width
  forceinline void
  check_float_domain_width(unsigned int width, WordDomainType domain_type,
                           const char* location) {
    if (is_float_domain_type(domain_type) &&
        (width != float_domain_width(domain_type)))
      throw OutOfLimits(location);
  }

  forceinline PreparedWordDomain
  prepare_bounded_domain(unsigned int width, WordValue lo, WordValue hi,
                         WordDomainType domain_type,
                         WordValue minimum, WordValue maximum,
                         const char* location) {
    check_domain(width,lo,hi,location);
    if (!is_bounded_domain_type(domain_type))
      throw OutOfLimits(location);
    check_float_domain_width(width,domain_type,location);
    const WordValue mask = width_mask(width);
    if (((minimum | maximum) & ~mask) != 0)
      throw OutOfLimits(location);
    const WordValue rank_minimum = rank(domain_type,width,minimum);
    const WordValue rank_maximum = rank(domain_type,width,maximum);
    if (rank_minimum > rank_maximum)
      throw VariableEmptyDomain(location);
    const WordDomainResult result = compute_domain_closure(
      width,domain_type,{{lo,hi},{rank_minimum,rank_maximum}});
    if (!result.is_consistent)
      throw VariableEmptyDomain(location);
    return {width,domain_type,result.domain.cube.lo,result.domain.cube.hi,
            result.domain.interval.minimum,result.domain.interval.maximum};
  }

  forceinline PreparedWordDomain
  prepare_full_domain(unsigned int width, WordDomainType domain_type,
                      const char* location) {
    const bool is_valid_width = (width >= 1U) && (width <= 64U);
    const bool is_valid_type = (domain_type == WDT_CUBE) ||
      is_bounded_domain_type(domain_type);
    if (!is_valid_width || !is_valid_type)
      throw OutOfLimits(location);
    check_float_domain_width(width,domain_type,location);
    // Every rank names one value, so the full cube admits every rank.
    const WordValue mask = width_mask(width);
    return {width,domain_type,0,mask,0,mask};
  }

  /// Prepare the full domain of float \a domain_type at the width of its format
  forceinline PreparedWordDomain
  prepare_float_domain(WordDomainType domain_type, const char* location) {
    if (!is_float_domain_type(domain_type))
      throw OutOfLimits(location);
    return prepare_full_domain(float_domain_width(domain_type),domain_type,
                               location);
  }

  /** \brief Return the least rank at or above \a r that \a cube admits in
   * the order of \a domain_type
   *
   * Returns false when there is none.
   */
  forceinline bool
  next_admitted_rank(WordDomainType domain_type, unsigned int width,
                     WordCube cube, WordValue r, WordValue& result) {
    const WordValue mask = width_mask(width);
    if (!splits_by_sign(domain_type,width,cube)) {
      const WordCube ordered = order_cube(domain_type,width,cube);
      return cube_successor(ordered.lo,ordered.hi,r,mask,result);
    }
    // Negative encodings have the ranks below the sign bit.
    const WordValue s = sign_bit(width);
    const SignHalves halves = sign_halves(cube,s);
    if (r < s) {
      const WordCube negative = order_cube(domain_type,width,halves.with_sign);
      if (cube_successor(negative.lo,negative.hi,r,mask,result))
        return true;
    }
    const WordCube positive = order_cube(domain_type,width,halves.without_sign);
    return cube_successor(positive.lo,positive.hi,std::max(r,s),mask,result);
  }

  /** \brief Return the greatest rank at or below \a r that \a cube admits
   * in the order of \a domain_type
   *
   * Returns false when there is none.
   */
  forceinline bool
  previous_admitted_rank(WordDomainType domain_type, unsigned int width,
                         WordCube cube, WordValue r, WordValue& result) {
    const WordValue mask = width_mask(width);
    if (!splits_by_sign(domain_type,width,cube)) {
      const WordCube ordered = order_cube(domain_type,width,cube);
      return cube_predecessor(ordered.lo,ordered.hi,r,mask,result);
    }
    // Negative encodings have the ranks below the sign bit.
    const WordValue s = sign_bit(width);
    const SignHalves halves = sign_halves(cube,s);
    if (r >= s) {
      const WordCube positive =
        order_cube(domain_type,width,halves.without_sign);
      if (cube_predecessor(positive.lo,positive.hi,r,mask,result))
        return true;
    }
    const WordCube negative = order_cube(domain_type,width,halves.with_sign);
    return cube_predecessor(negative.lo,negative.hi,std::min(r,s-1),mask,
                            result);
  }

  forceinline WordVarImp::WordVarImp(Space& home, unsigned int width,
                                     WordValue lo, WordValue hi)
    : WordVarImpBase(home), _width(static_cast<std::uint8_t>(width)),
      _has_value_set(false), _domain_type(WDT_CUBE), _lo(lo), _hi(hi) {}
  forceinline WordVarImp::WordVarImp(Space& home, unsigned int width,
                                     WordValue lo, WordValue hi,
                                     WordDomainType domain_type)
    : WordVarImpBase(home), _width(static_cast<std::uint8_t>(width)),
      _has_value_set(false), _domain_type(domain_type), _lo(lo), _hi(hi) {}
  forceinline WordVarImp::WordVarImp(Space& home, WordVarImp& x)
    : WordVarImpBase(home,x), _width(x._width),
      _has_value_set(x._has_value_set), _domain_type(x._domain_type),
      _lo(x._lo), _hi(x._hi) {}

  forceinline
  BoundedWordVarImp::BoundedWordVarImp(Space& home,
                                      const PreparedWordDomain& domain)
    : WordVarImp(home,domain.width,domain.lo,domain.hi,domain.domain_type),
      _minimum(domain.rank_minimum), _maximum(domain.rank_maximum) {}
  forceinline
  BoundedWordVarImp::BoundedWordVarImp(Space& home, BoundedWordVarImp& x)
    : WordVarImp(home,x), _minimum(x._minimum), _maximum(x._maximum) {}

  /// Start from the values of the prepared domain and take cube and interval
  forceinline
  ValueSetWordVarImp::ValueSetWordVarImp(Space& home,
                                         const PreparedWordDomain& domain)
    : BoundedWordVarImp(home,domain),
      _values(domain_values(domain.width,domain.domain_type,
                            {{domain.lo,domain.hi},
                             {domain.rank_minimum,domain.rank_maximum}})) {
    assert(domain.width <= max_value_set_width);
    _has_value_set = true;
    const WordDomain closed = domain_of(_values);
    _lo = closed.cube.lo;
    _hi = closed.cube.hi;
    _minimum = closed.interval.minimum;
    _maximum = closed.interval.maximum;
  }
  forceinline
  ValueSetWordVarImp::ValueSetWordVarImp(Space& home, ValueSetWordVarImp& x)
    : BoundedWordVarImp(home,x), _values(x._values) {}

  forceinline WordDomain
  ValueSetWordVarImp::domain_of(const WordValueSet& values) const {
    const WordCube cube = value_set_hull(values,_width);
    if (!bounded())
      return {cube,{_minimum,_maximum}};
    return {cube,value_set_rank_interval(
      order_value_set(_domain_type,_width,values),_width)};
  }

  forceinline ModEvent
  ValueSetWordVarImp::replace_values(Space& home, const WordValueSet& next) {
    if (same_value_sets(next,_values))
      return ME_WORD_NONE;
    if (is_empty_value_set(next))
      return fail(home);
    const WordDomain current = {{_lo,_hi},{_minimum,_maximum}};
    const WordDomain closed = domain_of(next);
    const ModEvent change = classify_domain_change(current,closed);
    const ModEvent event = (change == ME_WORD_NONE) ? ME_WORD_HOLE : change;
    const WordValue zero = _hi & ~closed.cube.hi;
    const WordValue one = closed.cube.lo & ~_lo;
    WordDelta delta = bounded() ?
      WordDelta(zero,one,_domain_type,_minimum,_maximum,
                closed.interval.minimum,closed.interval.maximum) :
      WordDelta(zero,one);
    _values = next;
    _lo = closed.cube.lo;
    _hi = closed.cube.hi;
    _minimum = closed.interval.minimum;
    _maximum = closed.interval.maximum;
    return notify(home,event,delta);
  }

  forceinline unsigned int WordVarImp::width(void) const { return _width; }
  forceinline WordValue WordVarImp::mask(void) const { return width_mask(_width); }
  forceinline WordValue WordVarImp::lo(void) const { return _lo; }
  forceinline WordValue WordVarImp::hi(void) const { return _hi; }
  forceinline WordDomainType WordVarImp::domain_type(void) const {
    return _domain_type;
  }
  forceinline bool WordVarImp::bounded(void) const {
    return _domain_type != WDT_CUBE;
  }
  forceinline bool WordVarImp::has_value_set(void) const {
    return _has_value_set;
  }
  forceinline bool WordVarImp::may_close_beyond_request(void) const {
    return bounded() || _has_value_set;
  }
  forceinline WordValue WordVarImp::minimum(void) const {
    if (!bounded())
      throw BoundsOfCubeVar("WordVar::minimum");
    const WordValue r =
      static_cast<const BoundedWordVarImp*>(this)->minimum();
    return encode_rank(_domain_type,_width,r);
  }
  forceinline WordValue WordVarImp::maximum(void) const {
    if (!bounded())
      throw BoundsOfCubeVar("WordVar::maximum");
    const WordValue r =
      static_cast<const BoundedWordVarImp*>(this)->maximum();
    return encode_rank(_domain_type,_width,r);
  }
  forceinline WordValue BoundedWordVarImp::minimum(void) const {
    return _minimum;
  }
  forceinline WordValue BoundedWordVarImp::maximum(void) const {
    return _maximum;
  }
  forceinline WordValue BoundedWordVarImp::rank(WordValue value) const {
    return Word::rank(_domain_type,_width,value);
  }
  forceinline WordValue WordVarImp::unknown(void) const { return _hi & ~_lo; }
  forceinline WordValue WordVarImp::val(void) const { return _lo; }
  forceinline unsigned int WordVarImp::unknown_size(void) const {
    WordValue u = unknown();
    unsigned int n = 0;
    while (u != 0) { u &= u-1; n++; }
    return n;
  }
  forceinline bool WordVarImp::assigned(void) const { return _lo == _hi; }
  forceinline bool WordVarImp::in(WordValue n) const {
    if (!cube_contains(_lo,_hi,n,mask()))
      return false;
    if (_has_value_set)
      return value_set_contains(
        static_cast<const ValueSetWordVarImp*>(this)->_values,n);
    if (!bounded())
      return true;
    const BoundedWordVarImp* b =
      static_cast<const BoundedWordVarImp*>(this);
    const WordValue r = b->rank(n);
    return (r >= b->_minimum) && (r <= b->_maximum);
  }
  forceinline WordValueSet WordVarImp::values(void) const {
    assert(_width <= max_value_set_width);
    if (_has_value_set)
      return static_cast<const ValueSetWordVarImp*>(this)->_values;
    if (!bounded())
      return domain_values(_width,_domain_type,{{_lo,_hi},{0,0}});
    const BoundedWordVarImp* b =
      static_cast<const BoundedWordVarImp*>(this);
    return domain_values(_width,_domain_type,
                         {{_lo,_hi},{b->_minimum,b->_maximum}});
  }

  forceinline ModEvent
  WordVarImp::narrow(Space& home, WordValue lo, WordValue hi) {
    if (bounded()) {
      BoundedWordVarImp* b = static_cast<BoundedWordVarImp*>(this);
      return b->narrow_domain(home,lo,hi,b->_minimum,b->_maximum);
    }
    const bool has_excess_bits = ((lo | hi) & ~mask()) != 0;
    const bool is_empty_cube = (lo & ~hi) != 0;
    if (has_excess_bits || is_empty_cube)
      return fail(home);
    if (_has_value_set)
      return static_cast<ValueSetWordVarImp*>(this)->narrow_cube(home,{lo,hi});
    const WordValue new_lo = _lo | lo;
    const WordValue new_hi = _hi & hi;
    if ((new_lo & ~new_hi) != 0)
      return fail(home);
    const bool is_unchanged = (new_lo == _lo) && (new_hi == _hi);
    if (is_unchanged)
      return ME_WORD_NONE;
    WordDelta d(_hi & ~new_hi,new_lo & ~_lo);
    _lo = new_lo;
    _hi = new_hi;
    return notify(home, assigned() ? ME_WORD_VAL : ME_WORD_BITS, d);
  }

  /// Intersect and classify a canonical domain before committing and notifying
  forceinline ModEvent
  BoundedWordVarImp::narrow_domain(Space& home,
                                   WordValue lo, WordValue hi,
                                   WordValue minimum, WordValue maximum) {
    const bool has_excess_bits =
      ((lo | hi | minimum | maximum) & ~mask()) != 0;
    const bool is_empty = ((lo & ~hi) != 0) || (minimum > maximum);
    if (has_excess_bits || is_empty)
      return fail(home);
    if (_has_value_set)
      return static_cast<ValueSetWordVarImp*>(this)->
        narrow_domain_values(home,{lo,hi},{minimum,maximum});
    const WordDomain current = {{_lo,_hi},{_minimum,_maximum}};
    const WordDomain requested = {{lo,hi},{minimum,maximum}};
    const WordDomain intersection = intersect_word_domains(current,requested);
    // An unchanged intersection already has the current domain's closure.
    if (classify_domain_change(current,intersection) == ME_WORD_NONE)
      return ME_WORD_NONE;
    return close_intersection(home,intersection);
  }

  /// Publish admitted endpoint changes directly when the cube stays unchanged
  forceinline ModEvent
  BoundedWordVarImp::narrow_range(Space& home,
                                  WordValue minimum, WordValue maximum) {
    const bool has_excess_bits = ((minimum | maximum) & ~mask()) != 0;
    const bool is_empty_interval = minimum > maximum;
    if (has_excess_bits || is_empty_interval)
      return fail(home);
    if (_has_value_set)
      return static_cast<ValueSetWordVarImp*>(this)->
        narrow_ranks(home,{minimum,maximum});
    const WordValue new_minimum = std::max(_minimum,minimum);
    const WordValue new_maximum = std::min(_maximum,maximum);
    if (new_minimum > new_maximum)
      return fail(home);
    const bool is_unchanged =
      (new_minimum == _minimum) && (new_maximum == _maximum);
    if (is_unchanged)
      return ME_WORD_NONE;

    if (!can_publish_bounds_only(_width,_domain_type,{_lo,_hi},
                                 {new_minimum,new_maximum}))
      return narrow_domain(home,_lo,_hi,new_minimum,new_maximum);

    WordDelta d(0,0,_domain_type,_minimum,_maximum,
                new_minimum,new_maximum);
    _minimum = new_minimum;
    _maximum = new_maximum;
    return notify(home,ME_WORD_BND,d);
  }

  forceinline ModEvent WordVarImp::eq(Space& home, WordValue value) {
    if (bounded()) {
      BoundedWordVarImp* b = static_cast<BoundedWordVarImp*>(this);
      const WordValue r = b->rank(value);
      return b->narrow_domain(home,value,value,r,r);
    }
    if (!in(value))
      return fail(home);
    if (assigned())
      return ME_WORD_NONE;
    if (_has_value_set)
      return static_cast<ValueSetWordVarImp*>(this)->assign_value(home,value);
    WordDelta d(_hi & ~value, value & ~_lo);
    _lo = _hi = value;
    return notify(home,ME_WORD_VAL,d);
  }

  forceinline ModEvent WordVarImp::nq(Space& home, WordValue value) {
    if (!in(value))
      return ME_WORD_NONE;
    if (assigned())
      return fail(home);
    if (_has_value_set)
      return static_cast<ValueSetWordVarImp*>(this)->remove_value(home,value);
    if (bounded()) {
      BoundedWordVarImp* b = static_cast<BoundedWordVarImp*>(this);
      const WordValue r = b->rank(value);
      if (r == b->_minimum)
        return b->narrow_range(home,r+1,b->_maximum);
      if (r == b->_maximum)
        return b->narrow_range(home,b->_minimum,r-1);
      return ME_WORD_NONE;
    }
    if (unknown_size() == 1)
      return eq(home,value ^ unknown());
    return ME_WORD_NONE;
  }

  forceinline WordVarImp* WordVarImp::copy(Space& home) {
    return copied() ? static_cast<WordVarImp*>(forward()) : perform_copy(home);
  }
  forceinline WordVarImp* WordVarImp::perform_copy(Space& home) {
    if (_has_value_set)
      return new (home) ValueSetWordVarImp(
        home,*static_cast<ValueSetWordVarImp*>(this));
    if (_domain_type == WDT_CUBE)
      return new (home) WordVarImp(home,*this);
    return new (home) BoundedWordVarImp(
      home,*static_cast<BoundedWordVarImp*>(this));
  }
  forceinline ModEventDelta WordVarImp::med(ModEvent me) {
    return WordVarImpBase::med(me);
  }
  forceinline WordValue WordVarImp::zero(const Delta& d) {
    return static_cast<const WordDelta&>(d).zero();
  }
  forceinline WordValue WordVarImp::one(const Delta& d) {
    return static_cast<const WordDelta&>(d).one();
  }
  forceinline WordDomainType WordVarImp::domain_type(const Delta& d) {
    return static_cast<const WordDelta&>(d).domain_type();
  }
  forceinline WordValue WordVarImp::old_minimum(const Delta& d) {
    return static_cast<const WordDelta&>(d).old_minimum();
  }
  forceinline WordValue WordVarImp::old_maximum(const Delta& d) {
    return static_cast<const WordDelta&>(d).old_maximum();
  }
  forceinline WordValue WordVarImp::new_minimum(const Delta& d) {
    return static_cast<const WordDelta&>(d).new_minimum();
  }
  forceinline WordValue WordVarImp::new_maximum(const Delta& d) {
    return static_cast<const WordDelta&>(d).new_maximum();
  }
}}

// STATISTICS: word-var
