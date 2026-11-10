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

#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <gecode/word/float/classify.hpp>

namespace Gecode {
  namespace {
    using FloatLayout = Word::Float::FormatFacts;

    /// Return the layout of \a format; throw Word::OutOfLimits at
    /// \a location for another domain type
    FloatLayout float_layout(WordDomainType format, const char* location) {
      if (!Word::is_float_domain_type(format))
        throw Word::OutOfLimits(location);
      return Word::Float::format_facts(format);
    }

    /// Kind of a double value
    enum class DoubleKind {
      Zero,     ///< Positive or negative zero
      Finite,   ///< Finite and not zero
      Infinite, ///< Positive or negative infinity
      NaN       ///< Quiet or signaling NaN of either sign
    };

    /// Kind and sign of a double value
    struct DoubleClass {
      /// Kind of the value
      DoubleKind kind;
      /// Whether the sign bit is set, also for zeros and NaNs
      bool negative;
    };

    /** \brief Return the kind and sign of \a value
     *
     * Reads the IEEE 754 binary64 bit pattern of \a value. Unlike
     * std::isnan and std::isinf this cannot be folded away by a compiler
     * that assumes finite math (as the Autoconf build's -ffinite-math-only
     * lets it), so infinities and NaNs are recognized under every set of
     * floating-point flags.
     */
    DoubleClass double_class(double value) {
      static_assert(sizeof(double) == sizeof(uint64_t),
                    "double must be IEEE 754 binary64");
      uint64_t bits;
      std::memcpy(&bits, &value, sizeof(bits));
      const bool negative = (bits >> 63) != 0;
      const uint64_t exponent = (bits >> 52) & 0x7ff;
      const uint64_t fraction = bits & ((uint64_t(1) << 52) - 1);
      if (exponent == 0x7ff)
        return {fraction == 0 ? DoubleKind::Infinite : DoubleKind::NaN,
                negative};
      if ((exponent == 0) && (fraction == 0))
        return {DoubleKind::Zero, negative};
      return {DoubleKind::Finite, negative};
    }

    /** \brief Return the magnitude code of the positive finite \a magnitude
     *
     * Throws Word::OutOfLimits at \a location unless \a layout represents
     * \a magnitude exactly. The decomposition by frexp and ldexp only moves the binary
     * point, so it is exact for every finite double.
     */
    WordValue magnitude_code(const FloatLayout& layout, double magnitude,
                             const char* location) {
      int exponent;
      const double mantissa = std::frexp(magnitude, &exponent);
      // magnitude = significand * 2^lowest with a significand of exactly 53
      // bits, so its leading bit has the exponent leading.
      const uint64_t significand =
          static_cast<uint64_t>(std::ldexp(mantissa, 53));
      const int leading = exponent - 1;
      const int lowest = leading - 52;
      const int lowest_set =
          lowest +
          static_cast<int>(Support::count_trailing_zeros_64(significand));
      // The quantum is the exponent of the last stored fraction bit at this
      // magnitude; subnormals share the quantum of the smallest normal.
      const int min_exponent = 1 - layout.bias;
      const int quantum = std::max(leading, min_exponent) -
                          static_cast<int>(layout.fraction_bits);
      if (lowest_set < quantum)
        throw Word::OutOfLimits(location);
      // The units of 2^quantum are the code of a subnormal. In a normal they
      // include the implicit bit 2^fraction_bits, which raises the field
      // leading - min_exponent to the biased exponent leading + bias.
      const WordValue units = significand >> (quantum - lowest);
      const WordValue code =
          (WordValue(std::max(leading - min_exponent, 0))
           << layout.fraction_bits) +
          units;
      if (code > layout.maximum)
        throw Word::OutOfLimits(location);
      return code;
    }

  } // namespace

  namespace Word {
    WordValue float_encoding(WordDomainType format, double value,
                             const char* location) {
      const FloatLayout layout = float_layout(format, location);
      // Classify from the bit pattern: std::fabs, std::frexp and std::ldexp
      // are reached only with finite non-zero values.
      const DoubleClass c = double_class(value);
      if (c.kind == DoubleKind::NaN)
        throw Word::OutOfLimits(location);
      const WordValue sign =
          c.negative ? WordValue(1) << (Word::float_domain_width(format) - 1)
                     : 0;
      switch (c.kind) {
      case DoubleKind::Infinite:
        if (!layout.has_infinity())
          throw Word::OutOfLimits(location);
        return sign | layout.infinity;
      case DoubleKind::Zero:
        return sign;
      default:
        return sign | magnitude_code(layout, std::fabs(value), location);
      }
    }
  } // namespace Word

  unsigned int float_word_width(WordDomainType format) {
    if (!Word::is_float_domain_type(format))
      throw Word::OutOfLimits("Word::float_word_width");
    return Word::float_domain_width(format);
  }

  namespace {
    /// Encoded endpoints of a float word and its resolved value-set option
    struct FloatWordBounds {
      unsigned int width;
      WordValue first;
      WordValue last;
      WordValueSetOption value_set;
    };

    /** \brief Return the bounds of a word of float \a format with the values
     * \a minimum through \a maximum
     *
     * Validates everything the bounded WordVar constructor checks, so that
     * every error names \a location instead of that constructor.
     */
    FloatWordBounds float_word_bounds(WordDomainType format, double minimum,
                                      double maximum,
                                      WordValueSetOption value_set,
                                      const char* location) {
      const WordValue first = Word::float_encoding(format, minimum, location);
      const WordValue last = Word::float_encoding(format, maximum, location);
      const unsigned int width = Word::float_domain_width(format);
      (void)Word::prepare_bounded_domain(width, 0, Word::width_mask(width),
                                         format, first, last, location);
      return {width, first, last,
              Word::checked_value_set(width, format, value_set, location)};
    }
  } // namespace

  WordVar float_constant(Space& home, WordDomainType format, double value) {
    const FloatWordBounds b = float_word_bounds(
        format, value, value, WVS_DEFAULT, "Word::float_constant");
    return WordVar(home, b.width, format, b.first, b.last, b.value_set);
  }

  WordVar float_word(Space& home, WordDomainType format, double minimum,
                     double maximum, WordValueSetOption value_set) {
    const FloatWordBounds b = float_word_bounds(format, minimum, maximum,
                                                value_set, "Word::float_word");
    return WordVar(home, b.width, format, b.first, b.last, b.value_set);
  }

  WordVarArray float_word_array(Space& home, int n, WordDomainType format,
                                double minimum, double maximum,
                                WordValueSetOption value_set) {
    const char* location = "Word::float_word_array";
    (void)Word::require_array_count(n, location);
    const FloatWordBounds b =
        float_word_bounds(format, minimum, maximum, value_set, location);
    return WordVarArray(home, n, b.width, format, b.first, b.last,
                        b.value_set);
  }

  namespace Word {
    namespace Float {
      void check(const FloatWordPolicy& p, const char* l) {
        if (p.rounding < Policy::RNE || p.rounding > Policy::RUP ||
            p.input_subnormals < Policy::Preserve ||
            p.input_subnormals > Policy::SignedZero ||
            p.output_subnormals < Policy::Preserve ||
            p.output_subnormals > Policy::SignedZero ||
            p.overflow < Policy::FormatDefault || p.overflow > Policy::SaturateFinite ||
            p.zeros < Policy::Signed || p.zeros > Policy::Positive ||
            p.nan < Policy::SignalFirst || p.nan > Policy::Canonical ||
            p.tininess < Policy::AfterPrecision ||
            p.tininess > Policy::BeforeRounding ||
            p.comparison < Policy::QuietComparison || p.comparison > Policy::SignalingComparison)
          throw Word::OutOfLimits(l);
      }

      WordDomainType float_format(WordVar x, const char* l) {
        if (!is_float_domain_type(x.domain_type()))
          throw Word::OutOfLimits(l);
        return x.domain_type();
      }
    } // namespace Float
  } // namespace Word

  namespace {
    bool has_tiny_class(WordDomainType format, unsigned int code,
                        FloatWordClass cls) {
      const Word::Float::FormatFacts f = Word::Float::format_facts(format);
      const WordValue magnitude = code & (f.sign() - 1);
      const bool zero = magnitude == 0;
      const bool subnormal = magnitude > 0 && magnitude < f.normal();
      const bool nan = f.is_nan(magnitude);
      switch (cls) {
      case FWC_ZERO:
        return zero;
      case FWC_SUBNORMAL:
        return subnormal;
      case FWC_NORMAL:
        return !zero && !subnormal && !nan;
      case FWC_FINITE:
        return !nan;
      case FWC_INFINITE:
      case FWC_SIGNALING_NAN:
        return false;
      case FWC_NAN:
      case FWC_QUIET_NAN:
        return nan;
      case FWC_SIGN:
        return (code & f.sign()) != 0;
      default:
        throw Word::OutOfLimits("Word::fp_classify");
      }
    }

    Word::WordValueSet tiny_class_members(WordDomainType format,
                                          FloatWordClass cls) {
      Word::WordValueSet members = {{0, 0, 0, 0}};
      for (unsigned int code = 0; code < (1U << float_word_width(format));
           ++code)
        if (has_tiny_class(format, code, cls))
          members = Word::with_value(members, code);
      return members;
    }

    /// Operation on the sign bit of a float word
    enum class SignOperation {
      Negate, ///< Flip the sign bit
      Abs     ///< Clear the sign bit
    };

    struct SignChange {
      WordValue sign;
      SignOperation operation;
      Word::Float::TinyResult operator()(WordValue code) const {
        return {static_cast<int>(operation == SignOperation::Negate
                                     ? code ^ sign
                                     : code & (sign - 1)),
                0};
      }
    };

    /// Post \a truth as the conjunction of two relations on \a magnitude
    void within(Home home, WordVar magnitude, WordRelType first_relation,
                WordValue first, WordRelType second_relation, WordValue second,
                BoolVar truth) {
      BoolVar above(home, 0, 1), below(home, 0, 1);
      rel(home, magnitude, first_relation, first, Reify(above));
      rel(home, magnitude, second_relation, second, Reify(below));
      rel(home, above, BOT_AND, below, truth);
    }

    void sign_operation(Home home, WordVar x, WordVar y,
                        SignOperation operation) {
      const bool negate = operation == SignOperation::Negate;
      const char* name = negate ? "Word::fp_negate" : "Word::fp_abs";
      const WordDomainType format = Word::Float::float_format(x, name);
      if (Word::Float::float_format(y, name) != format)
        throw Word::FormatMismatch(name);
      const unsigned int width = x.width();
      const WordValue sign = WordValue(1) << (width - 1);
      GECODE_POST;
      if (width <= 8)
        GECODE_ES_FAIL(
            (Word::Float::TinyUnary<SignChange, Int::ConstIntView>::post(
                home, x, y, Int::ConstIntView(0),
                SignChange{sign, operation})));
      else
        rel(home, x, negate ? WOT_XOR : WOT_AND, negate ? sign : sign - 1, y);
    }
  } // namespace

  void fp_classify(Home home, WordVar x, FloatWordClass cls, BoolVar truth) {
    const char* name = "Word::fp_classify";
    const WordDomainType format = Word::Float::float_format(x, name);
    if (cls < FWC_ZERO || cls > FWC_SIGN)
      throw Word::OutOfLimits(name);
    Word::Float::classify(home, x, format, cls, truth);
  }
  void Word::Float::classify(Home home, WordVar x, WordDomainType format,
                             FloatWordClass cls, BoolVar truth) {
    assert(cls >= FWC_ZERO && cls <= FWC_SIGN);
    const unsigned int width = Word::float_domain_width(format);
    assert(width != 0 && x.width() == width);
    if (width <= 8) {
      GECODE_POST;
      GECODE_ES_FAIL(TinyClassify::post(home, x, truth,
                                        tiny_class_members(format, cls)));
      return;
    }
    GECODE_POST;
    if (cls == FWC_SIGN) {
      channel(home, x, width - 1, truth);
      return;
    }
    // Every other class is a range of the magnitude, the code without its
    // sign bit: the exponent field holds its high bits, so zeros and
    // subnormals lie below the least normal code, infinity follows the
    // greatest finite code, and quiet NaNs lie above the signaling ones.
    const Word::Float::FormatFacts f = Word::Float::format_facts(format);
    const WordValue normal = f.normal();
    const WordValue infinity = f.infinity;
    const WordValue quiet = f.infinity | f.quiet();
    WordVar magnitude(home, width - 1, WDT_UNSIGNED);
    extract(home, x, 0, width - 1, magnitude);
    switch (cls) {
    case FWC_ZERO:
      rel(home, magnitude, WRT_EQ, 0, Reify(truth));
      break;
    case FWC_SUBNORMAL:
      within(home, magnitude, WRT_NQ, 0, WRT_ULE, normal, truth);
      break;
    case FWC_NORMAL:
      within(home, magnitude, WRT_UGQ, normal, WRT_ULE, infinity, truth);
      break;
    case FWC_FINITE:
      rel(home, magnitude, WRT_ULE, infinity, Reify(truth));
      break;
    case FWC_INFINITE:
      rel(home, magnitude, WRT_EQ, infinity, Reify(truth));
      break;
    case FWC_NAN:
      rel(home, magnitude, WRT_UGR, infinity, Reify(truth));
      break;
    case FWC_QUIET_NAN:
      rel(home, magnitude, WRT_UGQ, quiet, Reify(truth));
      break;
    case FWC_SIGNALING_NAN:
      within(home, magnitude, WRT_UGR, infinity, WRT_ULE, quiet, truth);
      break;
    default:
      GECODE_NEVER;
    }
  }
  void fp_negate(Home home, WordVar x, WordVar y) {
    sign_operation(home, x, y, SignOperation::Negate);
  }
  void fp_abs(Home home, WordVar x, WordVar y) {
    sign_operation(home, x, y, SignOperation::Abs);
  }
} // namespace Gecode

// STATISTICS: word-post
