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

#include <algorithm>
#include <cmath>

namespace Gecode {
  namespace {
    /// Encoding layout of a float format
    struct FloatLayout {
      /// Number of stored fraction bits
      unsigned int fraction_bits;
      /// Exponent bias
      int bias;
      /// Largest finite magnitude code
      WordValue maximum;
      /// Code of positive infinity, or 0 for a format without infinities
      WordValue infinity;
    };

    /// Return the layout of \a format; throw Word::OutOfLimits at
    /// \a location for another domain type
    FloatLayout float_layout(WordDomainType format, const char* location) {
      switch (format) {
      case WDT_FP32:
        return {23, 127, 0x7f7fffff, 0x7f800000};
      case WDT_BF16:
        return {7, 127, 0x7f7f, 0x7f80};
      case WDT_BINARY16:
        return {10, 15, 0x7bff, 0x7c00};
      case WDT_E4M3FN:
        return {3, 7, 0x7e, 0};
      case WDT_E2M1:
        return {1, 1, 0x7, 0};
      default:
        throw Word::OutOfLimits(location);
      }
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
      if (std::isnan(value))
        throw Word::OutOfLimits(location);
      const WordValue sign =
          std::signbit(value)
              ? WordValue(1) << (Word::float_domain_width(format) - 1)
              : 0;
      if (std::isinf(value)) {
        if (layout.infinity == 0)
          throw Word::OutOfLimits(location);
        return sign | layout.infinity;
      }
      if (value == 0)
        return sign;
      return sign | magnitude_code(layout, std::fabs(value), location);
    }
  } // namespace Word

  unsigned int float_word_width(WordDomainType format) {
    if (!Word::is_float_domain_type(format))
      throw Word::OutOfLimits("float_word_width");
    return Word::float_domain_width(format);
  }

  WordVar float_constant(Space& home, WordDomainType format, double value) {
    return WordVar(home, format, value, value);
  }

  namespace Word {
    namespace Float {
      void check(const FloatWordPolicy& p, const char* l) {
        using Policy = FloatWordPolicy;
        if (p.rounding < Policy::RNE || p.rounding > Policy::RUP ||
            p.input_subnormals < Policy::Preserve ||
            p.input_subnormals > Policy::SignedZero ||
            p.output_subnormals < Policy::Preserve ||
            p.output_subnormals > Policy::SignedZero ||
            p.overflow < Policy::Format || p.overflow > Policy::SaturateFinite ||
            p.zeros < Policy::Signed || p.zeros > Policy::Positive ||
            p.nan < Policy::SignalFirst || p.nan > Policy::Canonical ||
            p.tininess < Policy::AfterPrecision ||
            p.tininess > Policy::BeforeRounding ||
            p.comparison < Policy::Quiet || p.comparison > Policy::Signaling)
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
      const unsigned int sign = format == WDT_E2M1 ? 8U : 128U;
      const unsigned int magnitude = code & (sign - 1U);
      const bool zero = magnitude == 0;
      const bool subnormal =
          magnitude > 0 && magnitude < (format == WDT_E2M1 ? 2U : 8U);
      const bool nan = format == WDT_E4M3FN && magnitude == 127U;
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
        return (code & sign) != 0;
      default:
        throw Word::OutOfLimits("fp_classify");
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

    using TinyClassifyBase =
        MixBinaryPropagator<Word::WordView, Word::PC_WORD_DOM, Int::BoolView,
                            Int::PC_BOOL_VAL>;
    /** \brief Truth of a tiny word's membership in the encodings of a class
     *
     * Truth is decided once every remaining encoding agrees. An assigned
     * truth keeps the encodings of its outcome: exactly on a value-set word,
     * otherwise their cube and bounds, so the actor stays until the word
     * holds only encodings of that outcome.
     */
    class TinyClassify : public TinyClassifyBase {
      const Word::WordValueSet members;

      enum class Decision { failed, open, decided };
      // Filtering is idempotent, so an open decision is a fixpoint: an
      // assigned truth publishes the same set again, and an unassigned
      // truth changes nothing.
      static Decision decide(Space& home, Word::WordView x,
                             Int::BoolView truth,
                             const Word::WordValueSet& members) {
        if (truth.assigned()) {
          const Word::WordValueSet selected =
              truth.one() ? members
                          : Word::subtract_value_sets(
                                Word::full_value_set(x.width()), members);
          if (me_failed(x.narrow_values(home, selected)))
            return Decision::failed;
          return Word::is_empty_value_set(
                     Word::subtract_value_sets(x.values(), selected))
                     ? Decision::decided
                     : Decision::open;
        }
        const Word::WordValueSet values = x.values();
        if (Word::is_empty_value_set(
                Word::subtract_value_sets(values, members)))
          return me_failed(truth.one(home)) ? Decision::failed
                                            : Decision::decided;
        if (Word::is_empty_value_set(
                Word::intersect_value_sets(values, members)))
          return me_failed(truth.zero(home)) ? Decision::failed
                                             : Decision::decided;
        return Decision::open;
      }

      TinyClassify(Home home, Word::WordView x, Int::BoolView truth,
                   const Word::WordValueSet& m)
          : TinyClassifyBase(home, x, truth), members(m) {}
      TinyClassify(Space& home, TinyClassify& other)
          : TinyClassifyBase(home, other), members(other.members) {}

    public:
      /// Post unless the word or truth already decides the relation.
      static ExecStatus post(Home home, Word::WordView x, Int::BoolView truth,
                             const Word::WordValueSet& members) {
        const Decision decision = decide(home, x, truth, members);
        if (decision == Decision::failed)
          return ES_FAILED;
        if (decision == Decision::open)
          (void)new (home) TinyClassify(home, x, truth, members);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) TinyClassify(home, *this);
      }
      size_t dispose(Space& home) override {
        (void)TinyClassifyBase::dispose(home);
        return sizeof(*this);
      }
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        const Decision decision = decide(home, x0, x1, members);
        if (decision == Decision::failed)
          return ES_FAILED;
        return decision == Decision::decided ? home.ES_SUBSUMED(*this)
                                             : ES_FIX;
      }
    };

    struct SignChange {
      WordValue sign;
      bool negate;
      Word::Float::TinyResult operator()(WordValue code) const {
        return {static_cast<int>(negate ? code ^ sign : code & (sign - 1)), 0};
      }
    };

    // A field equality expressed through existing Word extraction/reification.
    BoolVar field_equal(Home home, WordVar bits, unsigned int first,
                        unsigned int width, WordValue value) {
      WordVar field(home, width);
      extract(home, bits, first, width, field);
      BoolVar equal(home, 0, 1);
      rel(home, field, WRT_EQ, value, Reify(equal));
      return equal;
    }

    void sign_operation(Home home, WordVar x, WordVar y, bool negate) {
      const char* name = negate ? "fp_negate" : "fp_abs";
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
                SignChange{sign, negate})));
      else
        rel(home, x, negate ? WOT_XOR : WOT_AND, negate ? sign : sign - 1, y);
    }
  } // namespace

  void fp_classify(Home home, WordVar x, FloatWordClass cls, BoolVar truth) {
    Word::Float::classify(home, x, Word::Float::float_format(x, "fp_classify"),
                          cls, truth);
  }
  void Word::Float::classify(Home home, WordVar x, WordDomainType format,
                             FloatWordClass cls, BoolVar truth) {
    if (cls < FWC_ZERO || cls > FWC_SIGN)
      throw Word::OutOfLimits("fp_classify");
    const unsigned int width = float_word_width(format);
    if (x.width() != width)
      throw Word::WidthMismatch("fp_classify");
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
    const unsigned int fraction = format == WDT_FP32   ? 23U
                                  : format == WDT_BF16 ? 7U
                                                       : 10U;
    const unsigned int exponent = width - fraction - 1;
    BoolVar exp_zero = field_equal(home, x, fraction, exponent, 0);
    BoolVar exp_full =
        field_equal(home, x, fraction, exponent, (1U << exponent) - 1);
    BoolVar frac_zero = field_equal(home, x, 0, fraction, 0);
    switch (cls) {
    case FWC_ZERO:
      rel(home, exp_zero, BOT_AND, frac_zero, truth);
      break;
    case FWC_SUBNORMAL: {
      BoolVar nonzero(home, 0, 1);
      rel(home, frac_zero, IRT_NQ, nonzero);
      rel(home, exp_zero, BOT_AND, nonzero, truth);
      break;
    }
    case FWC_NORMAL: {
      BoolVar special(home, 0, 1);
      rel(home, exp_zero, BOT_OR, exp_full, special);
      rel(home, special, IRT_NQ, truth);
      break;
    }
    case FWC_FINITE:
      rel(home, exp_full, IRT_NQ, truth);
      break;
    case FWC_INFINITE:
      rel(home, exp_full, BOT_AND, frac_zero, truth);
      break;
    case FWC_NAN:
    case FWC_QUIET_NAN:
    case FWC_SIGNALING_NAN: {
      BoolVar nonzero(home, 0, 1), nan(home, 0, 1);
      rel(home, frac_zero, IRT_NQ, nonzero);
      rel(home, exp_full, BOT_AND, nonzero, nan);
      if (cls == FWC_NAN)
        rel(home, nan, IRT_EQ, truth);
      else {
        BoolVar quiet(home, 0, 1), selected(home, 0, 1);
        channel(home, x, fraction - 1, quiet);
        rel(home, quiet, cls == FWC_QUIET_NAN ? IRT_EQ : IRT_NQ, selected);
        rel(home, nan, BOT_AND, selected, truth);
      }
      break;
    }
    default:
      GECODE_NEVER;
    }
  }
  void fp_negate(Home home, WordVar x, WordVar y) {
    sign_operation(home, x, y, true);
  }
  void fp_abs(Home home, WordVar x, WordVar y) {
    sign_operation(home, x, y, false);
  }
} // namespace Gecode

// STATISTICS: word-post
