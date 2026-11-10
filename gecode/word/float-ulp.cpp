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
#include <gecode/word/arithmetic/global-integer.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <gecode/word/float/ulp.hpp>

namespace Gecode {
  namespace {
    /// Return the layout of \a format; throw Word::OutOfLimits with \a l if
    /// \a format is not a float format.
    Format layout(WordDomainType format, const char* l) {
      if (!Word::is_float_domain_type(format))
        throw Word::OutOfLimits(l);
      return layout(format);
    }

    WordVar field(Home home, WordVar bits, unsigned int first,
                  unsigned int width) {
      WordVar result(home, width, WDT_UNSIGNED);
      extract(home, bits, first, width, result);
      return result;
    }

    IntVar integer_field(Home home, WordVar bits, unsigned int first,
                         unsigned int width) {
      IntVar result(home, 0, static_cast<int>((WordValue(1) << width) - 1));
      channel(home, field(home, bits, first, width), result, WI_UNSIGNED);
      return result;
    }

    void wide_spacing(Home home, Format reference, WordVar x,
                      WordDomainType format, IntVar q) {
      const Format source = layout(format);
      Word::Float::classify(home, x, format, FWC_FINITE, BoolVar(home, 1, 1));
      IntVar exponent = integer_field(home, x, source.fraction_bits,
                                      source.width - source.fraction_bits - 1);
      IntVar leading(home, 0, static_cast<int>(source.fraction_bits));
      count_leading_zeros(home, field(home, x, 0, source.fraction_bits), leading);
      IntVar subnormal(home, -150, 127), normal(home, -150, 127);
      TupleSet subnormals(2);
      for (unsigned int z = 0; z <= source.fraction_bits; ++z)
        subnormals.add(IntArgs{
            static_cast<int>(z),
            z == source.fraction_bits ? -150 : -source.bias - static_cast<int>(z)});
      subnormals.finalize();
      extensional(home, IntVarArgs{leading, subnormal}, subnormals);
      linear(home, IntArgs{1, -1}, IntVarArgs{exponent, normal}, IRT_EQ,
             source.bias);
      BoolVar is_subnormal(home, 0, 1);
      rel(home, exponent, IRT_EQ, 0, Reify(is_subnormal));
      IntVar binade(home, -150, 127), scale(home, 1 - reference.bias, 127);
      ite(home, is_subnormal, subnormal, normal, binade);
      max(home, binade, IntVar(home, 1 - reference.bias, 1 - reference.bias),
          scale);
      linear(home, IntArgs{1, -1}, IntVarArgs{scale, q}, IRT_EQ,
             static_cast<int>(reference.fraction_bits));
    }

    /// Assign the spacing exponent q in \a reference of the assigned wide
    /// word \a x of format \a source, or fail when x is not finite or q
    /// excludes the exponent.
    void decide_spacing(Home home, Format reference, WordVar x, Format source,
                        IntVar q) {
      const WordValue magnitude = x.val() & (source.sign() - 1);
      if (magnitude > source.maximum) {
        home.fail();
        return;
      }
      GECODE_ME_FAIL(Int::IntView(q).eq(
          home, magnitude_spacing(source, reference, magnitude)));
    }

    /// Exact FP32 embedding of a finite tiny value; nonfinite inputs fail.
    WordVar tiny_fp32(Home home, WordVar x, WordDomainType format) {
      const Format source = layout(format);
      WordVar magnitude(home, 31, WDT_UNSIGNED), result(home, 32, WDT_UNSIGNED);
      if (Word::Float::TinyUnary<Fp32Magnitude, Int::ConstIntView>::post(
              home, x, magnitude, Int::ConstIntView(0),
              Fp32Magnitude{source}) == ES_FAILED)
        home.fail();
      concat(home, field(home, x, source.width - 1, 1), magnitude, result);
      return result;
    }

    // Finite exact embedding of x, read in format. No rounding or operation
    // policy is involved.
    WordVar exact_fp32(Home home, WordVar x, WordDomainType format) {
      if (layout(format).width <= 8)
        return tiny_fp32(home, x, format);
      Word::Float::classify(home, x, format, FWC_FINITE, BoolVar(home, 1, 1));
      if (format == WDT_FP32)
        return x;
      WordVar result(home, 32, WDT_UNSIGNED);
      if (format == WDT_BF16) {
        concat(home, x, 16, 0, result);
        return result;
      }
      // Binary16 normals need a bias adjustment; subnormals need normalization.
      IntVar exponent = integer_field(home, x, 10, 5);
      WordVar fraction = field(home, x, 0, 10);
      IntVar leading(home, 0, 10), mapped_exponent(home, 0, 142),
          shift(home, 13, 24);
      count_leading_zeros(home, fraction, leading);
      TupleSet normalization(4);
      for (int e = 0; e <= 30; ++e)
        for (int z = 0; z <= 10; ++z)
          normalization.add(IntArgs{e, z,
                                    e         ? e + 112
                                    : z == 10 ? 0
                                              : 112 - z,
                                    e ? 13 : z + 14});
      normalization.finalize();
      extensional(home, IntVarArgs{exponent, leading, mapped_exponent, shift},
                  normalization);
      WordVar fraction32(home, 23, WDT_UNSIGNED),
          extended(home, 23, WDT_UNSIGNED), amount(home, 23, WDT_UNSIGNED),
          exponent32(home, 8, WDT_UNSIGNED), magnitude(home, 31, WDT_UNSIGNED);
      zero_extend(home, fraction, 23, extended);
      channel(home, amount, shift, WI_UNSIGNED);
      shift_left(home, extended, amount, fraction32);
      channel(home, exponent32, mapped_exponent, WI_UNSIGNED);
      concat(home, exponent32, fraction32, magnitude);
      concat(home, field(home, x, 15, 1), magnitude, result);
      return result;
    }

    // Encoding of the value of float word x in the reference format; x
    // must be exactly representable there.
    WordVar reference_encoding(Home home, WordDomainType reference, WordVar x) {
      if (x.domain_type() == reference) {
        Word::Float::classify(home, x, reference, FWC_FINITE,
                              BoolVar(home, 1, 1));
        return x;
      }
      const unsigned int width = float_word_width(reference);
      const WordVar witness =
          WordVar(home, width, WDT_UNSIGNED, width <= 8 ? WVS_EXACT : WVS_NONE);
      WordVar input = exact_fp32(home, x, x.domain_type());
      WordVar target = exact_fp32(home, witness, reference);
      rel(home, input, WRT_EQ, target);
      return witness;
    }

    void rank_distance(Home home, WordVar a, WordVar b, WordVar distance) {
      const unsigned int width = distance.width();
      WordVar am(home, width, WDT_UNSIGNED), bm(home, width, WDT_UNSIGNED);
      zero_extend(home, field(home, a, 0, width - 1), width, am);
      zero_extend(home, field(home, b, 0, width - 1), width, bm);
      BoolVar as(home, 0, 1), bs(home, 0, 1), opposite(home, 0, 1),
          less(home, 0, 1);
      channel(home, a, width - 1, as);
      channel(home, b, width - 1, bs);
      rel(home, as, BOT_XOR, bs, opposite);
      rel(home, am, WRT_ULE, bm, Reify(less));
      WordVar lower(home, width, WDT_UNSIGNED),
          upper(home, width, WDT_UNSIGNED),
          difference(home, width, WDT_UNSIGNED), sum(home, width, WDT_UNSIGNED);
      ite(home, less, am, bm, lower);
      ite(home, less, bm, am, upper);
      sub(home, upper, lower, difference);
      add(home, am, bm, sum);
      // Finite magnitudes are below 2^(width-1), so neither path wraps.
      ite(home, opposite, sum, difference, distance);
    }

    /// Return the error layout of \a reference_format; throw
    /// Word::OutOfLimits with \a l if it is not a float format.
    ErrorLayout error_layout(WordDomainType reference_format, const char* l) {
      const Format format = layout(reference_format, l);
      ErrorLayout error;
      error.qmin = 1 - format.bias - static_cast<int>(format.fraction_bits);
      error.qmax = 127 - static_cast<int>(format.fraction_bits);
      const int numerator_bits = 278 + error.qmax - error.qmin;
      error.digits = (numerator_bits + 29) / 30;
      error.leading_bits = numerator_bits - 30 * (error.digits - 1);
      error.scale = 149 + error.qmax;
      return error;
    }
  } // namespace

  void fp_ulp_spacing(Home home, WordDomainType reference_format,
                      WordVar reference, IntVar exponent) {
    const char* name = "Word::fp_ulp_spacing";
    const WordDomainType reference_type =
        Word::Float::float_format(reference, name);
    const Format format = layout(reference_format, name);
    const Format source = layout(reference_type);
    GECODE_POST;
    if (source.width <= 8)
      GECODE_ES_FAIL(
          TinySpacing::post(home, reference, exponent, source, format));
    else if (reference.assigned())
      decide_spacing(home, format, reference, source, exponent);
    else
      wide_spacing(home, format, reference, reference_type, exponent);
  }

  void fp_ulp_steps(Home home, WordDomainType reference_format, WordVar a,
                    WordVar b, WordVar distance, IntPropLevel ipl) {
    const char* name = "Word::fp_ulp_steps";
    const Format fa = layout(Word::Float::float_format(a, name));
    const Format fb = layout(Word::Float::float_format(b, name));
    const Format format = layout(reference_format, name);
    if (distance.width() != format.width)
      throw Word::WidthMismatch(name);
    GECODE_POST;
    if (fa.width <= 8 && fb.width <= 8) {
      GECODE_ES_FAIL((Word::Float::TinyBinary<TinySteps, WordView,
                                              Int::ConstIntView>::post(
          home, a, b, distance, Int::ConstIntView(0),
          TinySteps{fa, fb, format}, Word::Float::consistency(ipl))));
      return;
    }
    if (a.varimp() == b.varimp()) {
      // Decide the distance first; the encoding then only requires a
      // representable value.
      dom(home, distance, 0);
      if (home.failed())
        return;
      (void)reference_encoding(home, reference_format, a);
      return;
    }
    WordVar ae = reference_encoding(home, reference_format, a);
    WordVar be = reference_encoding(home, reference_format, b);
    rank_distance(home, ae, be, distance);
  }
  void fp_ulp_error(Home home, WordDomainType reference_format, WordVar a,
                    WordVar b, WordVar reference,
                    const IntVarArgs& numerator_digits, IntPropLevel ipl) {
    const WordDomainType a_format = Word::Float::float_format(a, "Word::fp_ulp_error");
    const WordDomainType b_format = Word::Float::float_format(b, "Word::fp_ulp_error");
    const WordDomainType r_format =
        Word::Float::float_format(reference, "Word::fp_ulp_error");
    const ErrorLayout error = error_layout(reference_format, "Word::fp_ulp_error");
    const ErrorFormats formats = {layout(a_format), layout(b_format),
                                  layout(r_format), layout(reference_format),
                                  error};
    if (numerator_digits.size() != error.digits)
      throw Word::OutOfLimits("Word::fp_ulp_error");
    GECODE_POST;
    dom(home, numerator_digits, 0, (1 << 30) - 1);
    dom(home, numerator_digits[0], 0, (1 << error.leading_bits) - 1);
    if (home.failed())
      return;
    ViewArray<Int::IntView> digits(home, numerator_digits);
    GECODE_ES_FAIL(ErrorBounds::post(home, a, b, reference, digits, formats,
                                     Word::Float::consistency(ipl)));
  }

  IntVarArray fp_ulp_error_digits(Space& home, WordDomainType reference_format) {
    const ErrorLayout error =
        error_layout(reference_format, "Word::fp_ulp_error_digits");
    IntVarArray digits(home, error.digits, 0, (1 << 30) - 1);
    digits[0] = IntVar(home, 0, (1 << error.leading_bits) - 1);
    return digits;
  }

  int fp_ulp_error_scale(WordDomainType reference_format) {
    return error_layout(reference_format, "Word::fp_ulp_error_scale").scale;
  }

  IntArgs fp_ulp_error_threshold(WordDomainType reference_format,
                                 long long numerator, int exponent) {
    const char* const l = "Word::fp_ulp_error_threshold";
    const ErrorLayout error = error_layout(reference_format, l);
    if (numerator < 0 || exponent < -error.scale)
      throw Word::OutOfLimits(l);
    IntArgs digits(error.digits);
    for (int i = 0; i < error.digits; ++i)
      digits[i] = 0;
    if (numerator == 0)
      return digits;
    const uint64_t n = static_cast<uint64_t>(numerator);
    int length = 0;
    while ((n >> length) != 0)
      ++length;
    // The value n * 2^shift fits when it has at most numerator_bits bits.
    const int numerator_bits = 30 * (error.digits - 1) + error.leading_bits;
    if (exponent > numerator_bits - length - error.scale)
      throw Word::OutOfLimits(l);
    const int shift = error.scale + exponent;
    for (int i = 0; i < error.digits; ++i) {
      // Position in n of the lowest bit of digit i
      const int low = 30 * (error.digits - 1 - i) - shift;
      uint64_t digit = 0;
      if (low >= 0)
        digit = low < 64 ? n >> low : 0;
      else if (low > -30)
        digit = n << -low;
      digits[i] = static_cast<int>(digit & ((uint64_t(1) << 30) - 1));
    }
    return digits;
  }
} // namespace Gecode

// STATISTICS: word-post
