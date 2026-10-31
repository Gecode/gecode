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

#include <algorithm>

namespace Gecode {
  namespace {
    struct Format {
      unsigned int width, fraction;
      int bias;
      WordValue maximum;
    };

    Format layout(FloatWordFormat format) {
      switch (format) {
      case FWF_FP32:
        return {32, 23, 127, 0x7f7fffff};
      case FWF_BF16:
        return {16, 7, 127, 0x7f7f};
      case FWF_BINARY16:
        return {16, 10, 15, 0x7bff};
      case FWF_E4M3FN:
        return {8, 3, 7, 0x7e};
      case FWF_E2M1:
        return {4, 1, 1, 7};
      default:
        throw Word::OutOfLimits("ulp observation");
      }
    }

    void validate_operand(FloatWordVar x) {
      if (layout(x.format()).width <= 8)
        (void)x.encoding();
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

    // Never channel a caller's signed-bound word as an unsigned integer.
    IntVarArgs distance_digits(Home home, WordVar distance) {
      if (distance.width() == 32)
        return IntVarArgs{integer_field(home, distance, 30, 2),
                          integer_field(home, distance, 0, 30)};
      return IntVarArgs{integer_field(home, distance, 0, distance.width())};
    }

    int tiny_binade(Format source, unsigned int code) {
      unsigned int m = code & ((1U << (source.width - 1)) - 1);
      unsigned int e = m >> source.fraction;
      if (e != 0)
        return static_cast<int>(e) - source.bias;
      if (m == 0)
        return -150; // Below every supported minimum normal exponent.
      int exponent = 1 - source.bias - static_cast<int>(source.fraction);
      while (m >>= 1)
        ++exponent;
      return exponent;
    }

    // Decode a tiny finite value and encode it exactly in the target lattice.
    bool tiny_encoding(Format source, unsigned int code, Format target,
                       WordValue& encoding) {
      const unsigned int sign = code >> (source.width - 1);
      const unsigned int m = code & ((1U << (source.width - 1)) - 1);
      if (m > source.maximum)
        return false;
      if (m == 0) {
        encoding = WordValue(sign) << (target.width - 1);
        return true;
      }
      unsigned int e = m >> source.fraction;
      WordValue significand = m & ((1U << source.fraction) - 1);
      if (e != 0)
        significand |= WordValue(1) << source.fraction;
      const int exponent = (e == 0 ? 1 : static_cast<int>(e)) - source.bias -
                           static_cast<int>(source.fraction);
      const int binade = tiny_binade(source, code);
      const int target_e = std::max(binade + target.bias, 1);
      const int shift = exponent - (target_e - target.bias -
                                    static_cast<int>(target.fraction));
      WordValue scaled = significand;
      if (shift >= 0) {
        if (shift >= 64)
          return false;
        scaled <<= shift;
      } else {
        if (-shift >= 64 || (scaled & ((WordValue(1) << -shift) - 1)) != 0)
          return false;
        scaled >>= -shift;
      }
      const WordValue hidden = WordValue(1) << target.fraction;
      const WordValue magnitude =
          scaled < hidden
              ? scaled
              : (WordValue(target_e) << target.fraction) + scaled - hidden;
      if (magnitude > target.maximum)
        return false;
      encoding = (WordValue(sign) << (target.width - 1)) | magnitude;
      return true;
    }

    void tiny_spacing(Home home, Format reference, FloatWordVar x, IntVar q) {
      const Format source = layout(x.format());
      const IntVar codes = x.encoding();
      const bool alias = codes.varimp() == q.varimp();
      TupleSet tuples(alias ? 1 : 2);
      for (unsigned int c = 0; c < (WordValue(1) << source.width); ++c) {
        if ((c & ((1U << (source.width - 1)) - 1)) > source.maximum)
          continue;
        const int exponent =
            std::max(tiny_binade(source, c), 1 - reference.bias) -
            static_cast<int>(reference.fraction);
        if (alias) {
          if (static_cast<int>(c) == exponent)
            tuples.add(IntArgs{static_cast<int>(c)});
        } else {
          tuples.add(IntArgs{static_cast<int>(c), exponent});
        }
      }
      tuples.finalize();
      extensional(home, alias ? IntVarArgs{codes} : IntVarArgs{codes, q},
                  tuples);
    }

    void wide_spacing(Home home, Format reference, FloatWordVar x, IntVar q) {
      const Format source = layout(x.format());
      fp_classify(home, x, FWC_FINITE, BoolVar(home, 1, 1));
      IntVar exponent = integer_field(home, x.bits(), source.fraction,
                                      source.width - source.fraction - 1);
      IntVar leading(home, 0, static_cast<int>(source.fraction));
      count_leading_zeros(home, field(home, x.bits(), 0, source.fraction),
                          leading);
      IntVar subnormal(home, -150, 127), normal(home, -150, 127);
      TupleSet subnormals(2);
      for (unsigned int z = 0; z <= source.fraction; ++z)
        subnormals.add(IntArgs{
            static_cast<int>(z),
            z == source.fraction ? -150 : -source.bias - static_cast<int>(z)});
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
             static_cast<int>(reference.fraction));
    }

    // Finite exact embedding only. No rounding or operation policy is involved.
    WordVar exact_fp32(Home home, FloatWordVar x) {
      const Format source = layout(x.format());
      fp_classify(home, x, FWC_FINITE, BoolVar(home, 1, 1));
      if (x.format() == FWF_FP32)
        return x.bits();
      WordVar result(home, 32, WDT_UNSIGNED);
      if (x.format() == FWF_BF16) {
        concat(home, x.bits(), 16, 0, result);
        return result;
      }
      if (source.width <= 8) {
        IntVar magnitude(home, 0, 0x7f7fffff);
        TupleSet tuples(2);
        for (unsigned int c = 0; c < (WordValue(1) << source.width); ++c) {
          WordValue encoded;
          if (tiny_encoding(source, c, layout(FWF_FP32), encoded))
            tuples.add(IntArgs{static_cast<int>(c),
                               static_cast<int>(encoded & 0x7fffffff)});
        }
        tuples.finalize();
        extensional(home, IntVarArgs{x.encoding(), magnitude}, tuples);
        WordVar mag(home, 31, WDT_UNSIGNED);
        channel(home, mag, magnitude, WI_UNSIGNED);
        concat(home, field(home, x.bits(), source.width - 1, 1), mag, result);
        return result;
      }
      // Binary16 normals need a bias adjustment; subnormals need normalization.
      IntVar exponent = integer_field(home, x.bits(), 10, 5);
      WordVar fraction = field(home, x.bits(), 0, 10);
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
      concat(home, field(home, x.bits(), 15, 1), magnitude, result);
      return result;
    }

    // Fixed-size, most-significant-first 30-bit limbs. Each selected shift
    // fits by construction; discarded bits of an unselected path impose no
    // restriction on the input.
    WordVarArgs shift_limbs_left(Home home, WordVarArgs limbs, IntVar shift) {
      WordVar amount(home, 8, WDT_UNSIGNED), zero(home, 30, 0, 0);
      channel(home, amount, shift, WI_UNSIGNED);
      for (unsigned int bit = 0; bit < 8; ++bit) {
        const int whole = (1U << bit) / 30;
        const unsigned int remainder = (1U << bit) % 30;
        BoolVar selected(home, 0, 1);
        channel(home, amount, bit, selected);
        WordVarArgs next(limbs.size());
        for (int i = 0; i < limbs.size(); ++i) {
          const int source = i + whole;
          WordVar upper = source < limbs.size() ? limbs[source] : zero;
          WordVar lower = source + 1 < limbs.size() ? limbs[source + 1] : zero;
          WordVar shifted(home, 30, WDT_UNSIGNED);
          concat(home, field(home, upper, 0, 30 - remainder),
                 field(home, lower, 30 - remainder, remainder), shifted);
          next[i] = WordVar(home, 30, WDT_UNSIGNED);
          ite(home, selected, shifted, limbs[i], next[i]);
        }
        limbs = next;
      }
      return limbs;
    }

    // Decode a finite FP32 magnitude as an integer multiple of 2^-149.
    WordVarArgs lattice_magnitude(Home home, WordVar encoded) {
      IntVar exponent = integer_field(home, encoded, 23, 8);
      BoolVar normal(home, 0, 1);
      rel(home, exponent, IRT_NQ, 0, Reify(normal));
      WordVar hidden(home, 1, WDT_UNSIGNED),
          significand(home, 24, WDT_UNSIGNED);
      channel(home, hidden, 0, normal);
      concat(home, hidden, field(home, encoded, 0, 23), significand);
      IntVar normal_shift(home, -1, 253), shift(home, 0, 253);
      linear(home, IntArgs{1, -1}, IntVarArgs{exponent, normal_shift}, IRT_EQ,
             1);
      ite(home, normal, normal_shift, IntVar(home, 0, 0), shift);
      WordVarArgs limbs(10);
      for (int i = 0; i < 9; ++i)
        limbs[i] = WordVar(home, 30, 0, 0);
      limbs[9] = WordVar(home, 30, WDT_UNSIGNED);
      zero_extend(home, significand, 30, limbs[9]);
      return shift_limbs_left(home, limbs, shift);
    }

    WordVarArgs absolute_difference(Home home, WordVar a, WordVar b) {
      WordVarArgs a_magnitude = lattice_magnitude(home, a);
      WordVarArgs b_magnitude = lattice_magnitude(home, b);
      BoolVar less(home, 0, 1), a_negative(home, 0, 1), b_negative(home, 0, 1),
          opposite(home, 0, 1);
      rel(home, field(home, a, 0, 31), WRT_ULE, field(home, b, 0, 31),
          Reify(less));
      channel(home, a, 31, a_negative);
      channel(home, b, 31, b_negative);
      rel(home, a_negative, BOT_XOR, b_negative, opposite);
      BoolVar carry(home, 0, 0), borrow(home, 0, 0);
      WordVarArgs difference(10);
      for (int i = 9; i >= 0; --i) {
        WordVar lower(home, 30, WDT_UNSIGNED), upper(home, 30, WDT_UNSIGNED),
            carry_word(home, 30, 0, 1), borrow_word(home, 30, 0, 1),
            partial_sum(home, 30, WDT_UNSIGNED), sum(home, 30, WDT_UNSIGNED),
            partial_difference(home, 30, WDT_UNSIGNED),
            same_sign_difference(home, 30, WDT_UNSIGNED);
        ite(home, less, a_magnitude[i], b_magnitude[i], lower);
        ite(home, less, b_magnitude[i], a_magnitude[i], upper);
        channel(home, carry_word, 0, carry);
        channel(home, borrow_word, 0, borrow);
        BoolVar carry1(home, 0, 1), carry2(home, 0, 1), borrow1(home, 0, 1),
            borrow2(home, 0, 1);
        add(home, a_magnitude[i], b_magnitude[i], partial_sum, carry1);
        add(home, partial_sum, carry_word, sum, carry2);
        sub(home, upper, lower, partial_difference, borrow1);
        sub(home, partial_difference, borrow_word, same_sign_difference,
            borrow2);
        carry = BoolVar(home, 0, 1);
        borrow = BoolVar(home, 0, 1);
        rel(home, carry1, BOT_OR, carry2, carry);
        rel(home, borrow1, BOT_OR, borrow2, borrow);
        difference[i] = WordVar(home, 30, WDT_UNSIGNED);
        ite(home, opposite, sum, same_sign_difference, difference[i]);
      }
      // Both paths fit: two finite magnitudes sum to at most 278 bits, and
      // the ordered subtraction never borrows beyond the most significant limb.
      rel(home, carry, IRT_EQ, 0);
      rel(home, borrow, IRT_EQ, 0);
      rel(home, difference[0], WRT_ULQ, 255);
      return difference;
    }

    WordVar reference_encoding(Home home, FloatWordFormat reference,
                               FloatWordVar x) {
      if (x.format() == reference) {
        fp_classify(home, x, FWC_FINITE, BoolVar(home, 1, 1));
        return x.bits();
      }
      FloatWordVar witness(home, reference);
      if (float_word_width(reference) <= 8)
        witness =
            FloatWordVar(home, reference, witness.bits(), IntVar(home, 0, 255));
      WordVar input = exact_fp32(home, x);
      WordVar target = exact_fp32(home, witness);
      rel(home, input, WRT_EQ, target);
      return witness.bits();
    }

    void tiny_steps(Home home, Format reference, FloatWordVar a, FloatWordVar b,
                    WordVar distance) {
      const Format fa = layout(a.format()), fb = layout(b.format());
      IntVar ca = a.encoding(), cb = b.encoding();
      const bool alias = ca.varimp() == cb.varimp();
      IntVarArgs digits = distance_digits(home, distance);
      IntVarArgs vars = alias ? IntVarArgs{ca} : IntVarArgs{ca, cb};
      vars << digits;
      TupleSet tuples(vars.size());
      for (unsigned int ac = 0; ac < (WordValue(1) << fa.width); ++ac) {
        WordValue ae;
        if (!tiny_encoding(fa, ac, reference, ae))
          continue;
        for (unsigned int bc = 0; bc < (WordValue(1) << fb.width); ++bc) {
          WordValue be;
          if (alias && ac != bc)
            continue;
          if (!tiny_encoding(fb, bc, reference, be))
            continue;
          const WordValue sign = WordValue(1) << (reference.width - 1);
          const WordValue am = ae & (sign - 1), bm = be & (sign - 1);
          const WordValue d = ((ae ^ be) & sign) ? am + bm
                              : am >= bm         ? am - bm
                                                 : bm - am;
          IntArgs tuple{static_cast<int>(ac)};
          if (!alias)
            tuple << static_cast<int>(bc);
          if (reference.width == 32)
            tuple << static_cast<int>(d >> 30)
                  << static_cast<int>(d & 0x3fffffff);
          else
            tuple << static_cast<int>(d);
          tuples.add(tuple);
        }
      }
      tuples.finalize();
      extensional(home, vars, tuples);
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

    /// Numerator digits of ulp_error and its fixed scale S.
    struct ErrorLayout {
      int qmin, qmax;
      int digits, leading_bits;
      int scale;
    };

    ErrorLayout error_layout(FloatWordFormat reference_format) {
      const Format format = layout(reference_format);
      ErrorLayout error;
      error.qmin = 1 - format.bias - static_cast<int>(format.fraction);
      error.qmax = 127 - static_cast<int>(format.fraction);
      const int numerator_bits = 278 + error.qmax - error.qmin;
      error.digits = (numerator_bits + 29) / 30;
      error.leading_bits = numerator_bits - 30 * (error.digits - 1);
      error.scale = 149 + error.qmax;
      return error;
    }
  } // namespace

  void ulp_spacing(Home home, FloatWordFormat reference_format,
                   FloatWordVar reference, IntVar exponent) {
    const Format format = layout(reference_format);
    validate_operand(reference);
    GECODE_POST;
    if (float_word_width(reference.format()) <= 8)
      tiny_spacing(home, format, reference, exponent);
    else
      wide_spacing(home, format, reference, exponent);
  }

  void ulp_steps(Home home, FloatWordFormat reference_format, FloatWordVar a,
                 FloatWordVar b, WordVar distance) {
    const Format format = layout(reference_format);
    validate_operand(a);
    validate_operand(b);
    if (distance.width() != format.width)
      throw Word::WidthMismatch("ulp_steps");
    GECODE_POST;
    if (float_word_width(a.format()) <= 8 &&
        float_word_width(b.format()) <= 8) {
      tiny_steps(home, format, a, b, distance);
      return;
    }
    WordVar ae = reference_encoding(home, reference_format, a);
    if (a.format() == b.format() && a.bits().varimp() == b.bits().varimp()) {
      dom(home, distance, 0);
      return;
    }
    WordVar be = reference_encoding(home, reference_format, b);
    rank_distance(home, ae, be, distance);
  }
  void ulp_error(Home home, FloatWordFormat reference_format, FloatWordVar a,
                 FloatWordVar b, FloatWordVar reference,
                 const IntVarArgs& numerator_digits) {
    const ErrorLayout error = error_layout(reference_format);
    validate_operand(a);
    validate_operand(b);
    validate_operand(reference);
    if (numerator_digits.size() != error.digits)
      throw Word::OutOfLimits("ulp_error");
    GECODE_POST;
    dom(home, numerator_digits, 0, (1 << 30) - 1);
    dom(home, numerator_digits[0], 0, (1 << error.leading_bits) - 1);
    WordVar ae = exact_fp32(home, a), be = exact_fp32(home, b);
    WordVarArgs difference = absolute_difference(home, ae, be);
    IntVar q(home, error.qmin, error.qmax),
        shift(home, 0, error.qmax - error.qmin);
    ulp_spacing(home, reference_format, reference, q);
    linear(home, IntVarArgs{q, shift}, IRT_EQ, error.qmax);
    WordVarArgs padded(error.digits);
    for (int i = 0; i < error.digits; ++i)
      padded[i] = i < error.digits - difference.size()
                      ? WordVar(home, 30, 0, 0)
                      : difference[i - (error.digits - difference.size())];
    WordVarArgs numerator = shift_limbs_left(home, padded, shift);
    for (int i = 0; i < error.digits; ++i)
      channel(home, numerator[i], numerator_digits[i], WI_UNSIGNED);
  }

  IntVarArray ulp_error_digits(Space& home, FloatWordFormat reference_format) {
    const ErrorLayout error = error_layout(reference_format);
    IntVarArray digits(home, error.digits, 0, (1 << 30) - 1);
    digits[0] = IntVar(home, 0, (1 << error.leading_bits) - 1);
    return digits;
  }

  int ulp_error_scale(FloatWordFormat reference_format) {
    return error_layout(reference_format).scale;
  }
} // namespace Gecode

// STATISTICS: word-post
