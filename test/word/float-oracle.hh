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

#ifndef GECODE_TEST_WORD_FLOAT_ORACLE_HH
#define GECODE_TEST_WORD_FLOAT_ORACLE_HH

// Why float tests use these oracles, not Test::Word::Test: see test/word.hh.

#include "test/word.hh"

#include <gecode/float-word.hh>

#include <cstdint>
#include <initializer_list>
#include <stdexcept>

namespace Test { namespace Word { namespace Float {

  /**
   * \defgroup TaskTestWordFloat Float word oracles
   * \ingroup TaskTestWord
   *
   * The expected results of the float word tests. They decode encodings
   * from the format layout and compute with their own exact arithmetic, so
   * they share no decoding, rounding or big-integer code with the library.
   */
  //@{

  /**
   * \brief Exact signed integer of the float word oracles
   *
   * Sign and magnitude, the magnitude in 32-bit limbs, least significant
   * first. Every result beyond the capacity throws std::overflow_error, so
   * an oracle never computes with a truncated value.
   */
  class ExactInteger {
  public:
    /// Number of limbs, enough for every exact float word comparison
    static const int capacity = 64;
  private:
    /// The limbs of the magnitude, least significant first
    std::uint32_t limb[capacity];
    /// Number of limbs in use: limb[size-1] is nonzero, or size is zero
    int size;
    /// Whether the value is negative (never for zero)
    bool negative;

    /// Throw unless \a n limbs fit
    static void reserve(int n) {
      if (n > capacity)
        throw std::overflow_error("ExactInteger: value too large");
    }
    /// Drop leading zero limbs and the sign of zero
    void trim(void) {
      while ((size > 0) && (limb[size-1] == 0))
        size--;
      if (size == 0)
        negative = false;
    }
    /// Compare the magnitudes of \a a and \a b
    static int compare_magnitude(const ExactInteger& a,
                                 const ExactInteger& b) {
      if (a.size != b.size)
        return (a.size < b.size) ? -1 : 1;
      for (int i=a.size; i--; )
        if (a.limb[i] != b.limb[i])
          return (a.limb[i] < b.limb[i]) ? -1 : 1;
      return 0;
    }
    /// Return the sum of the magnitudes of \a a and \a b, nonnegative
    static ExactInteger add_magnitude(const ExactInteger& a,
                                      const ExactInteger& b) {
      ExactInteger r;
      const int n = (a.size > b.size) ? a.size : b.size;
      std::uint64_t carry = 0;
      for (int i=0; i<n; i++) {
        carry += (i < a.size) ? a.limb[i] : 0;
        carry += (i < b.size) ? b.limb[i] : 0;
        r.limb[i] = static_cast<std::uint32_t>(carry);
        carry >>= 32;
      }
      r.size = n;
      if (carry != 0) {
        reserve(n+1);
        r.limb[r.size++] = static_cast<std::uint32_t>(carry);
      }
      return r;
    }
    /// Return magnitude \a a minus magnitude \a b, where \a a is not smaller
    static ExactInteger subtract_magnitude(const ExactInteger& a,
                                           const ExactInteger& b) {
      ExactInteger r;
      std::uint64_t borrow = 0;
      for (int i=0; i<a.size; i++) {
        const std::uint64_t subtrahend =
          ((i < b.size) ? b.limb[i] : 0) + borrow;
        borrow = (a.limb[i] < subtrahend) ? 1 : 0;
        r.limb[i] = static_cast<std::uint32_t>
          ((std::uint64_t(1) << 32)*borrow + a.limb[i] - subtrahend);
      }
      r.size = a.size;
      r.trim();
      return r;
    }
    /// Return \a a plus \a b where \a b has sign \a b_negative
    static ExactInteger signed_sum(const ExactInteger& a,
                                   const ExactInteger& b, bool b_negative) {
      if (a.negative == b_negative) {
        ExactInteger r = add_magnitude(a,b);
        r.negative = a.negative;
        r.trim();
        return r;
      }
      if (compare_magnitude(a,b) >= 0) {
        ExactInteger r = subtract_magnitude(a,b);
        r.negative = a.negative;
        r.trim();
        return r;
      }
      ExactInteger r = subtract_magnitude(b,a);
      r.negative = b_negative;
      r.trim();
      return r;
    }
  public:
    /// Zero
    ExactInteger(void) : size(0), negative(false) {}
    /// The value \a value
    explicit ExactInteger(std::uint64_t value) : size(0), negative(false) {
      while (value != 0) {
        limb[size++] = static_cast<std::uint32_t>(value);
        value >>= 32;
      }
    }
    /// Copy \a x
    ExactInteger(const ExactInteger& x) : size(x.size), negative(x.negative) {
      for (int i=0; i<size; i++)
        limb[i] = x.limb[i];
    }
    /// Assign \a x
    ExactInteger& operator =(const ExactInteger& x) {
      size = x.size;
      negative = x.negative;
      for (int i=0; i<size; i++)
        limb[i] = x.limb[i];
      return *this;
    }
    /// Return 2 to the power \a k
    static ExactInteger power_two(unsigned int k) {
      return ExactInteger(1).shift_left(k);
    }
    /// Test whether the value is zero
    bool zero(void) const { return size == 0; }
    /// Test whether the value is negative
    bool neg(void) const { return negative; }
    /// Return the absolute value
    ExactInteger abs(void) const {
      ExactInteger r(*this);
      r.negative = false;
      return r;
    }
    /// Return the number of bits of the magnitude
    int bits(void) const {
      if (size == 0)
        return 0;
      int n = 32*(size-1);
      for (std::uint32_t top = limb[size-1]; top != 0; top >>= 1)
        n++;
      return n;
    }
    /// Return bit \a i of the magnitude
    bool bit(unsigned int i) const {
      return (static_cast<int>(i / 32) < size) &&
        (((limb[i / 32] >> (i % 32)) & 1U) != 0);
    }
    /// Return the value times 2 to the power \a k
    ExactInteger shift_left(unsigned int k) const {
      if (size == 0)
        return *this;
      const int limbs = static_cast<int>(k / 32);
      const unsigned int bits = k % 32;
      ExactInteger r;
      reserve(size+limbs+1);
      for (int i=0; i<limbs; i++)
        r.limb[i] = 0;
      std::uint32_t carry = 0;
      for (int i=0; i<size; i++) {
        const std::uint64_t shifted = std::uint64_t(limb[i]) << bits;
        r.limb[i+limbs] = static_cast<std::uint32_t>(shifted) | carry;
        carry = static_cast<std::uint32_t>(shifted >> 32);
      }
      r.limb[size+limbs] = carry;
      r.size = size+limbs+1;
      r.negative = negative;
      r.trim();
      return r;
    }
    /** \brief Return the magnitude divided by \a divisor, rounded down,
     * and set \a remainder to the rest
     *
     * The quotient is nonnegative. Requires a nonzero \a divisor.
     */
    ExactInteger divide(std::uint32_t divisor,
                        std::uint32_t& remainder) const {
      ExactInteger q;
      std::uint64_t rest = 0;
      for (int i=size; i--; ) {
        rest = (rest << 32) | limb[i];
        q.limb[i] = static_cast<std::uint32_t>(rest / divisor);
        rest %= divisor;
      }
      q.size = size;
      q.trim();
      remainder = static_cast<std::uint32_t>(rest);
      return q;
    }
    /// Return the negated value
    ExactInteger operator -(void) const {
      ExactInteger r(*this);
      r.negative = (size != 0) && !negative;
      return r;
    }
    /// Return the sum of \a a and \a b
    friend ExactInteger operator +(const ExactInteger& a,
                                   const ExactInteger& b) {
      return signed_sum(a,b,b.negative);
    }
    /// Return \a a minus \a b
    friend ExactInteger operator -(const ExactInteger& a,
                                   const ExactInteger& b) {
      return signed_sum(a,b,(b.size != 0) && !b.negative);
    }
    /// Return the product of \a a and \a b
    friend ExactInteger operator *(const ExactInteger& a,
                                   const ExactInteger& b) {
      ExactInteger r;
      if ((a.size == 0) || (b.size == 0))
        return r;
      reserve(a.size+b.size);
      for (int i=0; i<a.size+b.size; i++)
        r.limb[i] = 0;
      for (int i=0; i<a.size; i++) {
        std::uint64_t carry = 0;
        for (int j=0; j<b.size; j++) {
          carry += std::uint64_t(a.limb[i])*b.limb[j] + r.limb[i+j];
          r.limb[i+j] = static_cast<std::uint32_t>(carry);
          carry >>= 32;
        }
        r.limb[i+b.size] = static_cast<std::uint32_t>(carry);
      }
      r.size = a.size+b.size;
      r.negative = a.negative != b.negative;
      r.trim();
      return r;
    }
    /// Return -1, 0 or 1 as \a a is less than, equal to or greater than \a b
    friend int compare(const ExactInteger& a, const ExactInteger& b) {
      if (a.negative != b.negative)
        return a.negative ? -1 : 1;
      const int c = compare_magnitude(a,b);
      return a.negative ? -c : c;
    }
    /// Test whether \a a equals \a b
    friend bool operator ==(const ExactInteger& a, const ExactInteger& b) {
      return compare(a,b) == 0;
    }
    /// Test whether \a a differs from \a b
    friend bool operator !=(const ExactInteger& a, const ExactInteger& b) {
      return compare(a,b) != 0;
    }
    /// Test whether \a a is less than \a b
    friend bool operator <(const ExactInteger& a, const ExactInteger& b) {
      return compare(a,b) < 0;
    }
    /// Test whether \a a is at most \a b
    friend bool operator <=(const ExactInteger& a, const ExactInteger& b) {
      return compare(a,b) <= 0;
    }
    /// Test whether \a a is greater than \a b
    friend bool operator >(const ExactInteger& a, const ExactInteger& b) {
      return compare(a,b) > 0;
    }
    /// Test whether \a a is at least \a b
    friend bool operator >=(const ExactInteger& a, const ExactInteger& b) {
      return compare(a,b) >= 0;
    }
  };

  /// A nonnegative exact rational with a positive denominator
  struct ExactRatio {
    ExactInteger numerator, denominator;
  };

  /// Return \a significand times 2 to the power \a exponent
  inline ExactRatio
  dyadic(std::uint64_t significand, int exponent) {
    if (exponent < 0)
      return {ExactInteger(significand),
              ExactInteger::power_two(static_cast<unsigned int>(-exponent))};
    return {ExactInteger(significand).
              shift_left(static_cast<unsigned int>(exponent)),
            ExactInteger(1)};
  }

  /// Return -1, 0 or 1 as \a a is less than, equal to or greater than \a b
  inline int
  compare(const ExactRatio& a, const ExactRatio& b) {
    return compare(a.numerator*b.denominator,b.numerator*a.denominator);
  }

  /**
   * \brief Layout of a float word format, written down from the format
   * definitions
   *
   * Magnitudes are encodings without the sign bit.
   */
  struct FloatFormat {
    /// The domain type
    Gecode::WordDomainType type;
    /// Width and number of fraction bits
    unsigned int width, fraction;
    /// Exponent bias
    int bias;
    /// The sign bit
    Gecode::WordValue sign;
    /// Magnitude of the least normal value
    Gecode::WordValue normal;
    /// Magnitude of the greatest finite value
    Gecode::WordValue maximum;
    /// Magnitude of infinity, or 0 without infinities
    Gecode::WordValue infinity;
    /// The canonical NaN, or 0 without NaNs
    Gecode::WordValue nan;
    /// The quiet bit of a NaN, or 0 without signaling NaNs
    Gecode::WordValue quiet;
    /// The encoding of one
    Gecode::WordValue one;

    /// Return the magnitude of \a code
    Gecode::WordValue magnitude(Gecode::WordValue code) const {
      return code & (sign - 1);
    }
    /// Test whether \a code has the sign bit set
    bool negative(Gecode::WordValue code) const {
      return (code & sign) != 0;
    }
    /// Test whether \a code is finite
    bool is_finite(Gecode::WordValue code) const {
      return magnitude(code) <= maximum;
    }
    /// Test whether \a code is a zero
    bool is_zero(Gecode::WordValue code) const {
      return magnitude(code) == 0;
    }
    /// Test whether \a code is subnormal
    bool is_subnormal(Gecode::WordValue code) const {
      return (magnitude(code) != 0) && (magnitude(code) < normal);
    }
    /// Test whether \a code is an infinity
    bool is_infinity(Gecode::WordValue code) const {
      return (infinity != 0) && (magnitude(code) == infinity);
    }
    /// Test whether \a code is a NaN
    bool is_nan(Gecode::WordValue code) const {
      return (nan != 0) && !is_finite(code) && !is_infinity(code);
    }
    /// Test whether \a code is a signaling NaN
    bool is_signaling(Gecode::WordValue code) const {
      return is_nan(code) && (quiet != 0) && ((code & quiet) == 0);
    }
    /** \brief Return the finite magnitude \a m in units of the least
     * subnormal
     *
     * Requires the result to fit, as for the formats up to binary16.
     */
    std::uint64_t units(Gecode::WordValue m) const {
      const Gecode::WordValue field = m >> fraction;
      return (field == 0) ? m : ((normal | (m & (normal-1))) << (field-1));
    }
    /// Return the value of the finite magnitude \a m
    ExactRatio value(Gecode::WordValue m) const {
      const Gecode::WordValue field = m >> fraction;
      const Gecode::WordValue significand =
        (field == 0) ? m : (normal | (m & (normal-1)));
      return dyadic(significand,
                    ((field == 0) ? 1 : static_cast<int>(field)) - bias -
                    static_cast<int>(fraction));
    }
  };

  /// Return the layout of float format \a type
  inline FloatFormat
  float_format(Gecode::WordDomainType type) {
    using namespace Gecode;
    switch (type) {
    case WDT_FP32:
      return {type,32,23,127,0x80000000,0x800000,0x7f7fffff,0x7f800000,
              0x7fc00000,0x400000,0x3f800000};
    case WDT_BF16:
      return {type,16,7,127,0x8000,0x80,0x7f7f,0x7f80,0x7fc0,0x40,0x3f80};
    case WDT_BINARY16:
      return {type,16,10,15,0x8000,0x400,0x7bff,0x7c00,0x7e00,0x200,
              0x3c00};
    case WDT_E4M3FN:
      return {type,8,3,7,0x80,0x8,0x7e,0,0x7f,0,0x38};
    case WDT_E2M1:
      return {type,4,1,1,0x8,0x2,0x7,0,0,0,0x2};
    default:
      throw std::invalid_argument("float_format: not a float format");
    }
  }

  /// The formats of float words, widest first
  const Gecode::WordDomainType float_formats[] = {
    Gecode::WDT_FP32, Gecode::WDT_BF16, Gecode::WDT_BINARY16,
    Gecode::WDT_E4M3FN, Gecode::WDT_E2M1
  };

  /// An expected result: the encoding and the status flags
  struct FloatResult {
    Gecode::WordValue bits;
    int flags;
  };

  /// Return the result of an overflow of sign \a negative in format \a f
  inline FloatResult
  overflow_result(const FloatFormat& f, const Gecode::FloatWordPolicy& p,
                  bool negative) {
    using Policy = Gecode::FloatWordPolicy;
    const Gecode::WordValue sign = negative ? f.sign : 0;
    if ((p.overflow == Policy::SaturateFinite) || (f.nan == 0))
      return {sign | f.maximum, 20};
    if (f.infinity == 0)
      return {((p.nan == Policy::Canonical) ? 0 : sign) | f.nan, 20};
    const bool infinite = (p.rounding == Policy::RNE) ||
      ((p.rounding == Policy::RUP) && !negative) ||
      ((p.rounding == Policy::RDN) && negative);
    return {sign | (infinite ? f.infinity : f.maximum), 20};
  }

  /// The integer part of a scaled exact value and how its rest compares
  struct Truncation {
    /// The integer part
    Gecode::WordValue integer;
    /// Whether the rest is zero
    bool exact;
    /// -1, 0 or 1 as the rest is less than, equal to or greater than 1/2
    int half;
  };

  /**
   * \brief Return the integer part of \a x divided by 2 to the power
   * \a grid, which must be less than 2 to the power \a bits
   */
  inline Truncation
  truncate(const ExactRatio& x, int grid, int bits) {
    ExactInteger scaled = x.numerator, unit = x.denominator;
    if (grid < 0)
      scaled = scaled.shift_left(static_cast<unsigned int>(-grid));
    else
      unit = unit.shift_left(static_cast<unsigned int>(grid));
    Gecode::WordValue integer = 0;
    ExactInteger product;
    for (int i=bits; i--; ) {
      const ExactInteger candidate =
        product + unit.shift_left(static_cast<unsigned int>(i));
      if (candidate <= scaled) {
        integer |= Gecode::WordValue(1) << i;
        product = candidate;
      }
    }
    const ExactInteger rest = scaled - product;
    return {integer, rest.zero(), compare(rest.shift_left(1),unit)};
  }

  /// Return whether rounding the magnitude \a t of sign \a negative goes up
  inline bool
  rounds_up(const Truncation& t, bool negative,
            Gecode::FloatWordPolicy::Rounding rounding) {
    using Policy = Gecode::FloatWordPolicy;
    if (t.exact)
      return false;
    if (rounding == Policy::RNE)
      return (t.half > 0) || ((t.half == 0) && ((t.integer & 1) != 0));
    return ((rounding == Policy::RUP) && !negative) ||
      ((rounding == Policy::RDN) && negative);
  }

  /**
   * \brief Round the positive exact magnitude \a exact of sign \a negative
   * once to format \a f under policy \a p
   *
   * Finds the binade of \a exact and rounds its significand on the grid of
   * the binade, or of the least normal binade for a subnormal result. A
   * carry into the next binade is a carry into the exponent field, so the
   * encoding follows from the significand. Overflow is decided at the
   * format's precision with an unbounded exponent; tininess is detected on
   * \a exact under BeforeRounding, and otherwise on \a exact rounded at
   * that precision. Applies output_subnormals and zeros. Special operands
   * and exact zeros are the caller's.
   */
  inline FloatResult
  round_exact(const FloatFormat& f, const ExactRatio& exact, bool negative,
              const Gecode::FloatWordPolicy& p) {
    using Policy = Gecode::FloatWordPolicy;
    using Gecode::WordValue;
    const int fraction = static_cast<int>(f.fraction);
    // The binade: the greatest e with 2^e <= exact
    int e = exact.numerator.bits() - exact.denominator.bits();
    if (truncate(exact,e,2).integer == 0)
      e--;
    const int least = 1 - f.bias;
    const int greatest = static_cast<int>(f.maximum >> f.fraction) - f.bias;
    if (e > greatest)
      return overflow_result(f,p,negative);
    const Truncation t = truncate(exact,(e < least ? least : e) - fraction,
                                  fraction+1);
    const WordValue significand =
      t.integer + (rounds_up(t,negative,p.rounding) ? 1 : 0);
    WordValue chosen = significand;
    if (e >= least)
      chosen = (static_cast<WordValue>(e + f.bias - 1) << f.fraction) +
        significand;
    if (chosen > f.maximum)
      return overflow_result(f,p,negative);
    int flags = t.exact ? 0 : 16;
    if (!t.exact) {
      bool tiny = e < least;
      if ((p.tininess != Policy::BeforeRounding) && (e == least-1)) {
        const Truncation precision = truncate(exact,e-fraction,fraction+1);
        tiny = precision.integer + (rounds_up(precision,negative,p.rounding)
                                    ? 1 : 0) < 2*f.normal;
      }
      if (tiny)
        flags |= 8;
    }
    if ((p.output_subnormals == Policy::SignedZero) && (chosen > 0) &&
        (chosen < f.normal)) {
      chosen = 0;
      flags |= 24;
    }
    const bool sign = negative && ((chosen != 0) || (p.zeros == Policy::Signed));
    return {chosen | (sign ? f.sign : 0), flags};
  }

  /**
   * \brief Return the exact sum of \a x with sign \a x_negative and \a y
   * with sign \a y_negative, and set \a negative to the sign of the sum
   *
   * An exact zero sum has the common sign of the terms, or else is
   * negative only under RDN.
   */
  inline ExactRatio
  exact_sum(const ExactRatio& x, bool x_negative, const ExactRatio& y,
            bool y_negative, Gecode::FloatWordPolicy::Rounding rounding,
            bool& negative) {
    ExactInteger a = x.numerator*y.denominator;
    ExactInteger b = y.numerator*x.denominator;
    if (x_negative)
      a = -a;
    if (y_negative)
      b = -b;
    const ExactInteger sum = a + b;
    if (sum.zero())
      negative = (x_negative == y_negative) ? x_negative :
        (rounding == Gecode::FloatWordPolicy::RDN);
    else
      negative = sum.neg();
    return {sum.abs(),x.denominator*y.denominator};
  }

  /// A result in a format of width at most 8: the encoding and the flags
  struct TinyResult {
    int code, flags;
  };

  /// A format of width at most 8, with its encoding constants as integers
  struct TinyFormat {
    /// The domain type
    Gecode::WordDomainType format;
    /// The layout
    FloatFormat layout;
    /// The sign bit, the greatest finite and the least normal magnitude
    int sign, maximum, normal;
    /// Initialize for \a f
    explicit TinyFormat(Gecode::WordDomainType f)
      : format(f), layout(float_format(f)),
        sign(static_cast<int>(layout.sign)),
        maximum(static_cast<int>(layout.maximum)),
        normal(static_cast<int>(layout.normal)) {}
    /// Return the exact magnitude \a exact of sign \a negative rounded once
    TinyResult round(const ExactRatio& exact, bool negative,
                     const Gecode::FloatWordPolicy& p) const {
      if (exact.numerator.zero())
        return {(negative && (p.zeros == Gecode::FloatWordPolicy::Signed)) ?
                sign : 0, 0};
      const FloatResult r = round_exact(layout,exact,negative,p);
      return {static_cast<int>(r.bits),r.flags};
    }
  };

  /**
   * \brief Return the NaN result of an operation whose operands
   * \a operands of format \a f include a NaN
   *
   * Selects per the policy, quiets the selection and sets the invalid flag
   * for a signaling operand. Without signaling NaNs (E4M3FN) the selected
   * NaN is returned as it is.
   */
  inline FloatResult
  nan_result(const FloatFormat& f, const Gecode::FloatWordPolicy& p,
             std::initializer_list<Gecode::WordValue> operands) {
    using Policy = Gecode::FloatWordPolicy;
    bool signaling = false, found = false;
    Gecode::WordValue selected = 0;
    for (Gecode::WordValue x : operands)
      if (f.is_nan(x)) {
        if (!found || ((p.nan == Policy::SignalFirst) && !signaling &&
                       f.is_signaling(x)))
          selected = x;
        found = true;
        signaling = signaling || f.is_signaling(x);
      }
    return {(p.nan == Policy::Canonical) ? f.nan : (selected | f.quiet),
            signaling ? 1 : 0};
  }

  /**
   * \brief Return a word of float format \a format
   *
   * A word of width at most 8 has a value set unless \a value_set is
   * WVS_NONE; a wider word never has one.
   */
  inline Gecode::WordVar
  float_variable(Gecode::Space& home, Gecode::WordDomainType format,
                 Gecode::WordValueSetOption value_set = Gecode::WVS_EXACT) {
    return Gecode::WordVar(home,format,
                           (Gecode::float_word_width(format) <= 8) ?
                           value_set : Gecode::WVS_NONE);
  }
  //@}

}}}

#endif

// STATISTICS: test-word
