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

namespace Gecode {
  unsigned int float_word_width(FloatWordFormat format) {
    switch (format) {
    case FWF_FP32:
      return 32;
    case FWF_BF16:
    case FWF_BINARY16:
      return 16;
    case FWF_E4M3FN:
      return 8;
    case FWF_E2M1:
      return 4;
    default:
      throw Word::OutOfLimits("float_word_width");
    }
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
    } // namespace Float
  } // namespace Word

  FloatWordVar::FloatWordVar(void) : format_id(FWF_FP32) {}
  FloatWordVar::FloatWordVar(Space& home, FloatWordFormat format,
                             WordDomainType domain)
      : word(home, float_word_width(format), domain), format_id(format) {}
  FloatWordVar::FloatWordVar(FloatWordFormat format, WordVar bits)
      : word(bits), format_id(format) {
    if (bits.width() != float_word_width(format))
      throw Word::WidthMismatch("FloatWordVar");
  }
  FloatWordVar::FloatWordVar(Space& home, FloatWordFormat format, WordVar bits,
                             IntVar encoding)
      : FloatWordVar(format, bits) {
    if (bits.width() > 8)
      throw Word::OutOfLimits("FloatWordVar::encoding");
    codes = encoding;
    channel_encoding(home, word, codes);
  }
  FloatWordVar FloatWordVar::from_bits(Space& home, FloatWordFormat format,
                                       WordValue bits) {
    return FloatWordVar(format,
                        WordVar(home, float_word_width(format), bits, bits));
  }
  WordVar FloatWordVar::bits(void) const { return word; }
  FloatWordFormat FloatWordVar::format(void) const { return format_id; }
  bool FloatWordVar::has_encoding(void) const {
    return codes.varimp() != nullptr;
  }
  IntVar FloatWordVar::encoding(void) const {
    if (!has_encoding())
      throw Word::OutOfLimits("FloatWordVar::encoding");
    return codes;
  }
  void FloatWordVar::update(Space& home, FloatWordVar& other) {
    word.update(home, other.word);
    format_id = other.format_id;
    if (other.has_encoding())
      codes.update(home, other.codes);
    else
      codes = IntVar();
  }
  FloatWordVar FloatWordVar::reinterpret_bits(FloatWordFormat format) const {
    if (float_word_width(format) != word.width())
      throw Word::WidthMismatch("FloatWordVar::reinterpret_bits");
    FloatWordVar result(*this);
    result.format_id = format;
    return result;
  }

  namespace {
    bool has_tiny_class(FloatWordFormat format, unsigned int code,
                        FloatWordClass cls) {
      const unsigned int sign = format == FWF_E2M1 ? 8U : 128U;
      const unsigned int magnitude = code & (sign - 1U);
      const bool zero = magnitude == 0;
      const bool subnormal =
          magnitude > 0 && magnitude < (format == FWF_E2M1 ? 2U : 8U);
      const bool nan = format == FWF_E4M3FN && magnitude == 127U;
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

    // A field equality expressed through existing Word extraction/reification.
    BoolVar field_equal(Home home, WordVar bits, unsigned int first,
                        unsigned int width, WordValue value) {
      WordVar field(home, width);
      extract(home, bits, first, width, field);
      BoolVar equal(home, 0, 1);
      rel(home, field, WRT_EQ, value, Reify(equal));
      return equal;
    }

    void sign_operation(Home home, FloatWordVar x, FloatWordVar y,
                        bool negate) {
      if (x.format() != y.format())
        throw Word::FormatMismatch(negate ? "fp_negate" : "fp_abs");
      const unsigned int width = float_word_width(x.format());
      const WordValue sign = WordValue(1) << (width - 1);
      if (width <= 8) {
        IntVar a = x.encoding(), b = y.encoding();
        GECODE_POST;
        if (a.varimp() == b.varimp()) {
          if (negate)
            home.fail();
          else
            rel(home, a, IRT_LE, static_cast<int>(sign));
          return;
        }
        TupleSet tuples(2);
        for (int code = 0; code < static_cast<int>(sign * 2); ++code)
          tuples.add(IntArgs{code, negate ? code ^ static_cast<int>(sign)
                                          : code & static_cast<int>(sign - 1)});
        tuples.finalize();
        extensional(home, IntVarArgs{a, b}, tuples);
      } else {
        GECODE_POST;
        rel(home, x.bits(), negate ? WOT_XOR : WOT_AND,
            negate ? sign : sign - 1, y.bits());
      }
    }
  } // namespace

  void fp_classify(Home home, FloatWordVar x, FloatWordClass cls,
                   BoolVar truth) {
    if (cls < FWC_ZERO || cls > FWC_SIGN)
      throw Word::OutOfLimits("fp_classify");
    const unsigned int width = float_word_width(x.format());
    if (width <= 8) {
      IntVar codes = x.encoding();
      GECODE_POST;
      int values[256];
      int count = 0;
      for (unsigned int code = 0; code < (1U << width); ++code)
        if (has_tiny_class(x.format(), code, cls))
          values[count++] = static_cast<int>(code);
      dom(home, codes, IntSet(values, count), Reify(truth));
      return;
    }
    GECODE_POST;
    if (cls == FWC_SIGN) {
      channel(home, x.bits(), width - 1, truth);
      return;
    }
    const unsigned int fraction = x.format() == FWF_FP32   ? 23U
                                  : x.format() == FWF_BF16 ? 7U
                                                           : 10U;
    const unsigned int exponent = width - fraction - 1;
    BoolVar exp_zero = field_equal(home, x.bits(), fraction, exponent, 0);
    BoolVar exp_full =
        field_equal(home, x.bits(), fraction, exponent, (1U << exponent) - 1);
    BoolVar frac_zero = field_equal(home, x.bits(), 0, fraction, 0);
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
        channel(home, x.bits(), fraction - 1, quiet);
        rel(home, quiet, cls == FWC_QUIET_NAN ? IRT_EQ : IRT_NQ, selected);
        rel(home, nan, BOT_AND, selected, truth);
      }
      break;
    }
    default:
      GECODE_NEVER;
    }
  }
  void fp_negate(Home home, FloatWordVar x, FloatWordVar y) {
    sign_operation(home, x, y, true);
  }
  void fp_abs(Home home, FloatWordVar x, FloatWordVar y) {
    sign_operation(home, x, y, false);
  }
} // namespace Gecode

// STATISTICS: word-post
