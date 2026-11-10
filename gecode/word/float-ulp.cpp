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
#include <cstdint>
#include <optional>

namespace Gecode {
  namespace {
    using Word::WordView;
    using Word::WordValueSet;
    using Word::Float::TinyOutcome;
    using Word::Float::TinyResult;
    using Word::Float::TinyValues;
    using Word::Float::failed_change;

    struct Format {
      unsigned int width, fraction;
      int bias;
      WordValue maximum;
    };

    Format layout(WordDomainType format) {
      switch (format) {
      case WDT_FP32:
        return {32, 23, 127, 0x7f7fffff};
      case WDT_BF16:
        return {16, 7, 127, 0x7f7f};
      case WDT_BINARY16:
        return {16, 10, 15, 0x7bff};
      case WDT_E4M3FN:
        return {8, 3, 7, 0x7e};
      case WDT_E2M1:
        return {4, 1, 1, 7};
      default:
        throw Word::OutOfLimits("ulp observation");
      }
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

    /// Test whether \a code encodes a finite value of the tiny \a source
    bool finite_code(Format source, WordValue code) {
      return (code & ((WordValue(1) << (source.width - 1)) - 1)) <=
             source.maximum;
    }

    /// Spacing exponent of the \a reference lattice at a finite tiny value
    int spacing_exponent(Format source, Format reference, WordValue code) {
      return std::max(tiny_binade(source, static_cast<unsigned int>(code)),
                      1 - reference.bias) -
             static_cast<int>(reference.fraction);
    }

    /** \brief Spacing exponent q of a tiny reference x
     *
     * One pass over the at most 256 remaining encodings keeps the finite
     * encodings whose spacing exponent remains, exactly on a value-set word
     * and as their tightest cube and bounds otherwise, and narrows q
     * exactly to the exponents of the kept encodings.
     */
    class TinySpacing
        : public MixBinaryPropagator<WordView, Word::PC_WORD_DOM,
                                     Int::IntView, Int::PC_INT_DOM> {
      using Base = MixBinaryPropagator<WordView, Word::PC_WORD_DOM,
                                       Int::IntView, Int::PC_INT_DOM>;
      const Format source, reference;

      TinySpacing(Home home, WordView x, Int::IntView q, Format s, Format r)
          : Base(home, x, q), source(s), reference(r) {}
      TinySpacing(Space& home, TinySpacing& other)
          : Base(home, other), source(other.source),
            reference(other.reference) {}

      /// Keep the encodings and exponents that support each other.
      static ExecStatus narrow_to_supports(Space& home, WordView x,
                                           Int::IntView q, Format source,
                                           Format reference) {
        const TinyValues codes(x);
        WordValueSet kept = {{0, 0, 0, 0}};
        int exponents[256];
        int count = 0;
        for (unsigned int i = 0; i < codes.size(); ++i) {
          if (!finite_code(source, codes[i]))
            continue;
          const int exponent = spacing_exponent(source, reference, codes[i]);
          if (!q.in(exponent))
            continue;
          kept = Word::with_value(kept, codes[i]);
          exponents[count++] = exponent;
        }
        if (count == 0)
          return ES_FAILED;
        GECODE_ME_CHECK(x.narrow_values(home, kept));
        std::sort(exponents, exponents + count);
        count = static_cast<int>(std::unique(exponents, exponents + count) -
                                 exponents);
        Iter::Values::Array supported(exponents, count);
        GECODE_ME_CHECK(q.inter_v(home, supported, false));
        return ES_OK;
      }

    public:
      /// Post unless an assigned reference decides the exponent.
      static ExecStatus post(Home home, WordView x, Int::IntView q,
                             Format source, Format reference) {
        if (x.assigned())
          return narrow_to_supports(home, x, q, source, reference);
        (void)new (home) TinySpacing(home, x, q, source, reference);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) TinySpacing(home, *this);
      }
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::binary(PropCost::HI);
      }
      size_t dispose(Space& home) override {
        (void)Base::dispose(home);
        return sizeof(*this);
      }
      // One pass is a fixpoint: the kept encodings K and exponents E support
      // each other, narrow_values never shrinks x below K and q keeps
      // exactly E. A second pass finds K and E again and nothing outside the
      // published domains, so it changes nothing.
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        GECODE_ES_CHECK(narrow_to_supports(home, x0, x1, source, reference));
        return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
    };

    void wide_spacing(Home home, Format reference, WordVar x,
                      WordDomainType format, IntVar q) {
      const Format source = layout(format);
      Word::Float::classify(home, x, format, FWC_FINITE, BoolVar(home, 1, 1));
      IntVar exponent = integer_field(home, x, source.fraction,
                                      source.width - source.fraction - 1);
      IntVar leading(home, 0, static_cast<int>(source.fraction));
      count_leading_zeros(home, field(home, x, 0, source.fraction), leading);
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

    /// FP32 magnitude of a finite tiny encoding; nonfinite codes have none
    struct Fp32Magnitude {
      Format source;
      TinyResult operator()(WordValue code) const {
        WordValue encoded;
        if (!tiny_encoding(source, static_cast<unsigned int>(code),
                           layout(WDT_FP32), encoded))
          return {0, 1};
        return {static_cast<int>(encoded & 0x7fffffff), 0};
      }
    };

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

    // Fixed-size, most-significant-first 30-bit limbs. Each selected shift
    // fits by construction; discarded bits of an unselected path impose no
    // restriction on the input.
    WordVarArgs shift_limbs_left(Home home, WordVarArgs limbs, IntVar shift) {
      WordVar amount(home, 8, WDT_UNSIGNED), zero(home, 30, {0});
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
        limbs[i] = WordVar(home, 30, {0});
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
      // Carry and borrow words fix every bit but bit 0; a value list {0, 1}
      // would add a table propagator per word.
      const WordBits low_bit = known_bits(Word::width_mask(30) - 1, 0);
      for (int i = 9; i >= 0; --i) {
        WordVar lower(home, 30, WDT_UNSIGNED), upper(home, 30, WDT_UNSIGNED),
            carry_word(home, 30, low_bit), borrow_word(home, 30, low_bit),
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

    /// Signed rank of a tiny encoding in the \a reference lattice, if any
    std::optional<int64_t> reference_rank(Format source, Format reference,
                                          WordValue code) {
      WordValue encoded;
      if (!tiny_encoding(source, static_cast<unsigned int>(code), reference,
                         encoded))
        return std::nullopt;
      const WordValue sign = WordValue(1) << (reference.width - 1);
      const int64_t magnitude = static_cast<int64_t>(encoded & (sign - 1));
      return (encoded & sign) != 0 ? -magnitude : magnitude;
    }

    /// Least and greatest reference rank of the representable values
    struct RankRange {
      bool is_empty;
      int64_t least, greatest;
    };

    /** \brief ulp_steps between tiny endpoints in a reference lattice
     *
     * The distance of two representable endpoints is the absolute
     * difference of their signed ranks. The default reasoning narrows the
     * distance to the interval that the endpoints' rank ranges span, and
     * keeps each representable endpoint encoding whose rank lies within the
     * distance's unsigned interval of the other endpoint's rank range.
     */
    struct TinySteps {
      Format first, second, reference;

      TinyOutcome operator()(WordValue x, WordValue y) const {
        const std::optional<int64_t> rx = reference_rank(first, reference, x);
        const std::optional<int64_t> ry = reference_rank(second, reference, y);
        if (!rx || !ry)
          return {0, 1};
        return {static_cast<WordValue>(*rx >= *ry ? *rx - *ry : *ry - *rx),
                0};
      }
      RankRange rank_range(Format source, const TinyValues& codes) const {
        RankRange range = {true, 0, 0};
        for (unsigned int i = 0; i < codes.size(); ++i) {
          const std::optional<int64_t> rank =
              reference_rank(source, reference, codes[i]);
          if (!rank)
            continue;
          range.least = range.is_empty ? *rank : std::min(range.least, *rank);
          range.greatest =
              range.is_empty ? *rank : std::max(range.greatest, *rank);
          range.is_empty = false;
        }
        return range;
      }
      /// Keep the representable endpoint encodings near \a other.
      ModEvent narrow_endpoint(Space& home, WordView x, Format source,
                               const TinyValues& codes, RankRange other,
                               int64_t least, int64_t greatest) const {
        WordValueSet kept = {{0, 0, 0, 0}};
        for (unsigned int i = 0; i < codes.size(); ++i) {
          const std::optional<int64_t> rank =
              reference_rank(source, reference, codes[i]);
          if (!rank)
            continue;
          const int64_t r = *rank;
          if ((other.least <= r - least && r - greatest <= other.greatest) ||
              (other.least <= r + greatest && r + least <= other.greatest))
            kept = Word::with_value(kept, codes[i]);
        }
        return x.narrow_values(home, kept);
      }
      ExecStatus narrow_bounds(Space& home, WordView x, WordView y,
                               WordView distance, Int::ConstIntView) const {
        const TinyValues xs(x), ys(y);
        const RankRange rx = rank_range(first, xs), ry = rank_range(second, ys);
        if (rx.is_empty || ry.is_empty)
          return ES_FAILED;
        bool changed = false;
        const int64_t closest =
            std::max({int64_t(0), rx.least - ry.greatest,
                      ry.least - rx.greatest});
        const int64_t farthest =
            std::max(rx.greatest - ry.least, ry.greatest - rx.least);
        if (failed_change(Word::Float::narrow_unsigned_interval(
                              home, distance, static_cast<WordValue>(closest),
                              static_cast<WordValue>(farthest)),
                          changed))
          return ES_FAILED;
        const Word::Float::UnsignedInterval d =
            Word::Float::unsigned_interval(distance);
        const int64_t least = static_cast<int64_t>(d.least),
                      greatest = static_cast<int64_t>(d.greatest);
        if (failed_change(
                narrow_endpoint(home, x, first, xs, ry, least, greatest),
                changed) ||
            failed_change(
                narrow_endpoint(home, y, second, ys, rx, least, greatest),
                changed))
          return ES_FAILED;
        return changed ? ES_NOFIX : ES_FIX;
      }
    };

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

    ErrorLayout error_layout(WordDomainType reference_format) {
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

    /// A finite tiny value as a signed multiple of 2^-9
    int64_t scaled_value(Format source, WordValue code) {
      const WordValue m = code & ((WordValue(1) << (source.width - 1)) - 1);
      const WordValue e = m >> source.fraction;
      int64_t significand =
          static_cast<int64_t>(m & ((WordValue(1) << source.fraction) - 1));
      if (e != 0)
        significand |= int64_t(1) << source.fraction;
      const int shift = (e == 0 ? 1 : static_cast<int>(e)) - source.bias -
                        static_cast<int>(source.fraction) + 9;
      assert(shift >= 0);
      const int64_t value = significand << shift;
      return (code >> (source.width - 1)) != 0 ? -value : value;
    }

    /// Formats and digit layout of a tiny ulp_error
    struct ErrorFormats {
      Format a, b, r, reference;
      ErrorLayout error;
    };

    /// Digits of Q = K * 2^s for one spacing exponent of the reference
    struct ErrorGroup {
      int low;             ///< Position of the digit holding bit s
      unsigned int offset; ///< s modulo 30
      bool others_zero;    ///< Every digit outside low and low-1 admits 0
    };

    /// Digit of K * 2^offset at the low position
    int low_digit(WordValue k, unsigned int offset) {
      return static_cast<int>((k << offset) & ((WordValue(1) << 30) - 1));
    }
    /// Digit of K * 2^offset at the position above the low position
    int high_digit(WordValue k, unsigned int offset) {
      return static_cast<int>(k >> (30 - offset));
    }

    /** \brief Domain-consistent ulp_error on tiny a, b and reference r
     *
     * With tiny values a = Ka*2^-9 and b = Kb*2^-9 and the spacing exponent
     * q of r, the numerator is Q = K * 2^s with K = abs(Ka-Kb) < 2^19 and
     * s = 140+qmax-q, as A = a*2^149 = Ka*2^140. Its nonzero bits lie in
     * the base-2^30 digit holding bit s and the digit above it.
     *
     * Each propagation groups the remaining finite values of r by spacing
     * exponent, decides for every distinct K and group whether the digits
     * admit Q, and enumerates the remaining pairs of a and b. It keeps
     * exactly the encodings of a, b and r and the digit values of the
     * supported tuples, and the tightest cube and bounds of the supported
     * encodings on a word without a value set. Every alias among a, b, r
     * and among the digits is checked per tuple.
     */
    class TinyError : public Propagator {
      WordView a, b, r;
      ViewArray<Int::IntView> digits;
      const ErrorFormats formats;

      TinyError(Home home, WordView x, WordView y, WordView z,
                ViewArray<Int::IntView>& d, const ErrorFormats& f)
          : Propagator(home), a(x), b(y), r(z), digits(d), formats(f) {
        a.subscribe(home, *this, Word::PC_WORD_DOM);
        b.subscribe(home, *this, Word::PC_WORD_DOM);
        r.subscribe(home, *this, Word::PC_WORD_DOM);
        digits.subscribe(home, *this, Int::PC_INT_DOM);
      }
      TinyError(Space& home, TinyError& other)
          : Propagator(home, other), formats(other.formats) {
        a.update(home, other.a);
        b.update(home, other.b);
        r.update(home, other.r);
        digits.update(home, other.digits);
      }

      /// Test whether digits \a low and \a high at \a group respect aliases
      static bool consistent_aliases(const int* first, const int* size,
                                     const ErrorGroup& group, int low,
                                     int high) {
        const int l = group.low, h = group.low - 1;
        if (first[l] == first[h])
          return size[first[l]] == 2 ? low == high : low == 0 && high == 0;
        return (size[first[l]] == 1 || low == 0) &&
               (size[first[h]] == 1 || high == 0);
      }

      /// Keep the values of the tuples with a supported numerator.
      static ExecStatus narrow_to_supports(Space& home, WordView a, WordView b,
                                           WordView r,
                                           ViewArray<Int::IntView>& digits,
                                           const ErrorFormats& formats) {
        Region region;
        const TinyValues as(a), bs(b), rs(r);
        const int n = digits.size();
        // Group the finite references by spacing exponent.
        int exponents[32];
        ErrorGroup groups[32];
        int group_of[256];
        int group_count = 0;
        for (unsigned int i = 0; i < rs.size(); ++i) {
          group_of[i] = -1;
          if (!finite_code(formats.r, rs[i]))
            continue;
          const int q = spacing_exponent(formats.r, formats.reference, rs[i]);
          int g = 0;
          while (g < group_count && exponents[g] != q)
            ++g;
          if (g == group_count) {
            assert(group_count < 32);
            exponents[group_count++] = q;
          }
          group_of[i] = g;
        }
        if (group_count == 0)
          return ES_FAILED;
        for (int g = 0; g < group_count; ++g) {
          const int shift = 140 + formats.error.qmax - exponents[g];
          groups[g].low = n - 1 - shift / 30;
          groups[g].offset = static_cast<unsigned int>(shift % 30);
          assert(groups[g].low >= 1);
          groups[g].others_zero = true;
          for (int p = 0; p < n; ++p)
            if (p != groups[g].low && p != groups[g].low - 1 &&
                !digits[p].in(0))
              groups[g].others_zero = false;
        }
        // Classes of repeated digit variables: first position and size.
        int* first = region.alloc<int>(n);
        int* size = region.alloc<int>(n);
        for (int p = 0; p < n; ++p) {
          first[p] = p;
          for (int j = 0; j < p; ++j)
            if (digits[j] == digits[p]) {
              first[p] = first[j];
              break;
            }
          size[p] = 0;
        }
        for (int p = 0; p < n; ++p)
          ++size[first[p]];
        // Scaled values of the finite operands.
        const bool same_operands = a == b;
        const bool r_is_a = r == a, r_is_b = r == b;
        int64_t* ka = region.alloc<int64_t>(as.size());
        int64_t* kb = region.alloc<int64_t>(bs.size());
        bool* finite_a = region.alloc<bool>(as.size());
        bool* finite_b = region.alloc<bool>(bs.size());
        for (unsigned int i = 0; i < as.size(); ++i) {
          finite_a[i] = finite_code(formats.a, as[i]);
          ka[i] = finite_a[i] ? scaled_value(formats.a, as[i]) : 0;
        }
        for (unsigned int j = 0; j < bs.size(); ++j) {
          finite_b[j] = finite_code(formats.b, bs[j]);
          kb[j] = finite_b[j] ? scaled_value(formats.b, bs[j]) : 0;
        }
        // Distinct differences K and the groups whose digits admit them.
        WordValue* ks =
            region.alloc<WordValue>(std::max(1U, as.size() * bs.size()));
        int k_count = 0;
        for (unsigned int i = 0; i < as.size(); ++i)
          for (unsigned int j = same_operands ? i : 0;
               j < (same_operands ? i + 1 : bs.size()); ++j)
            if (finite_a[i] && finite_b[j])
              ks[k_count++] = static_cast<WordValue>(
                  ka[i] >= kb[j] ? ka[i] - kb[j] : kb[j] - ka[i]);
        std::sort(ks, ks + k_count);
        k_count = static_cast<int>(std::unique(ks, ks + k_count) - ks);
        uint32_t* feasible = region.alloc<uint32_t>(std::max(1, k_count));
        uint32_t* used = region.alloc<uint32_t>(std::max(1, k_count));
        for (int k = 0; k < k_count; ++k) {
          feasible[k] = 0;
          used[k] = 0;
          for (int g = 0; g < group_count; ++g) {
            const int low = low_digit(ks[k], groups[g].offset);
            const int high = high_digit(ks[k], groups[g].offset);
            if (groups[g].others_zero && digits[groups[g].low].in(low) &&
                digits[groups[g].low - 1].in(high) &&
                consistent_aliases(first, size, groups[g], low, high))
              feasible[k] |= uint32_t(1) << g;
          }
        }
        // Supported pairs.
        Word::Float::WordSupports a_supports(a), b_supports(b),
            r_supports(r);
        uint32_t used_groups = 0;
        for (unsigned int i = 0; i < as.size(); ++i)
          for (unsigned int j = same_operands ? i : 0;
               j < (same_operands ? i + 1 : bs.size()); ++j) {
            if (!finite_a[i] || !finite_b[j])
              continue;
            const WordValue k = static_cast<WordValue>(
                ka[i] >= kb[j] ? ka[i] - kb[j] : kb[j] - ka[i]);
            const int index =
                static_cast<int>(std::lower_bound(ks, ks + k_count, k) - ks);
            uint32_t mask = feasible[index];
            if (r_is_a)
              mask &= group_of[i] < 0 ? 0 : uint32_t(1) << group_of[i];
            if (r_is_b)
              mask &= group_of[j] < 0 ? 0 : uint32_t(1) << group_of[j];
            if (mask == 0)
              continue;
            a_supports.add(as[i]);
            b_supports.add(bs[j]);
            used[index] |= mask;
            used_groups |= mask;
          }
        if (a_supports.empty())
          return ES_FAILED;
        for (unsigned int i = 0; i < rs.size(); ++i)
          if (group_of[i] >= 0 &&
              (used_groups & (uint32_t(1) << group_of[i])) != 0)
            r_supports.add(rs[i]);
        GECODE_ME_CHECK(a_supports.publish(home, a));
        GECODE_ME_CHECK(b_supports.publish(home, b));
        GECODE_ME_CHECK(r_supports.publish(home, r));
        return narrow_digits(home, digits, groups, group_count, ks, k_count,
                             used, used_groups);
      }

      /// Keep exactly the digit values of the supported numerators.
      static ExecStatus narrow_digits(Space& home,
                                      ViewArray<Int::IntView>& digits,
                                      const ErrorGroup* groups,
                                      int group_count, const WordValue* ks,
                                      int k_count, const uint32_t* used,
                                      uint32_t used_groups) {
        Region region;
        const int n = digits.size();
        int* count = region.alloc<int>(n);
        bool* zero = region.alloc<bool>(n);
        for (int p = 0; p < n; ++p) {
          count[p] = 0;
          zero[p] = false;
        }
        int used_count[32];
        for (int g = 0; g < group_count; ++g) {
          used_count[g] = 0;
          if ((used_groups & (uint32_t(1) << g)) == 0)
            continue;
          for (int k = 0; k < k_count; ++k)
            if ((used[k] & (uint32_t(1) << g)) != 0)
              ++used_count[g];
          count[groups[g].low] += used_count[g];
          count[groups[g].low - 1] += used_count[g];
          for (int p = 0; p < n; ++p)
            if (p != groups[g].low && p != groups[g].low - 1)
              zero[p] = true;
        }
        for (int p = 0; p < n; ++p) {
          int* values = region.alloc<int>(count[p] + 1);
          int size = 0;
          if (zero[p])
            values[size++] = 0;
          for (int g = 0; g < group_count; ++g) {
            const bool low = p == groups[g].low;
            if (used_count[g] == 0 || (!low && p != groups[g].low - 1))
              continue;
            for (int k = 0; k < k_count; ++k)
              if ((used[k] & (uint32_t(1) << g)) != 0)
                values[size++] = low ? low_digit(ks[k], groups[g].offset)
                                     : high_digit(ks[k], groups[g].offset);
          }
          std::sort(values, values + size);
          size = static_cast<int>(std::unique(values, values + size) - values);
          Iter::Values::Array supported(values, size);
          GECODE_ME_CHECK(digits[p].inter_v(home, supported, false));
          region.free<int>(values, count[p] + 1);
        }
        return ES_OK;
      }

    public:
      /// Post unless assigned values decide the digits.
      static ExecStatus post(Home home, WordView a, WordView b, WordView r,
                             ViewArray<Int::IntView>& digits,
                             const ErrorFormats& formats) {
        if (a.assigned() && b.assigned() && r.assigned())
          return narrow_to_supports(home, a, b, r, digits, formats);
        (void)new (home) TinyError(home, a, b, r, digits, formats);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) TinyError(home, *this);
      }
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::quadratic(PropCost::HI, 256);
      }
      void reschedule(Space& home) override {
        a.reschedule(home, *this, Word::PC_WORD_DOM);
        b.reschedule(home, *this, Word::PC_WORD_DOM);
        r.reschedule(home, *this, Word::PC_WORD_DOM);
        digits.reschedule(home, *this, Int::PC_INT_DOM);
      }
      size_t dispose(Space& home) override {
        a.cancel(home, *this, Word::PC_WORD_DOM);
        b.cancel(home, *this, Word::PC_WORD_DOM);
        r.cancel(home, *this, Word::PC_WORD_DOM);
        digits.cancel(home, *this, Int::PC_INT_DOM);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      // One pass is a fixpoint. The supported tuples use only values that
      // remain after publishing: a, b and r keep their supported encodings
      // (WordSupports never shrinks a word below them), every digit keeps
      // the digit values of the used numerators, and the zero digits of a
      // used group stay admitted. A second pass therefore finds every
      // supported tuple again, with the same groups and feasible
      // differences, and no tuple outside the published domains, so it
      // publishes nothing new. Alias checks hold per tuple.
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        GECODE_ES_CHECK(narrow_to_supports(home, a, b, r, digits, formats));
        return (a.assigned() && b.assigned() && r.assigned())
                   ? home.ES_SUBSUMED(*this)
                   : ES_FIX;
      }
    };
  } // namespace

  void ulp_spacing(Home home, WordDomainType reference_format,
                   WordVar reference, IntVar exponent) {
    const WordDomainType reference_type =
        Word::Float::float_format(reference, "ulp observation");
    const Format format = layout(reference_format);
    const Format source = layout(reference_type);
    GECODE_POST;
    if (source.width <= 8)
      GECODE_ES_FAIL(
          TinySpacing::post(home, reference, exponent, source, format));
    else
      wide_spacing(home, format, reference, reference_type, exponent);
  }

  void ulp_steps(Home home, WordDomainType reference_format, WordVar a,
                 WordVar b, WordVar distance, IntPropLevel ipl) {
    const Format fa = layout(Word::Float::float_format(a, "ulp observation"));
    const Format fb = layout(Word::Float::float_format(b, "ulp observation"));
    const Format format = layout(reference_format);
    if (distance.width() != format.width)
      throw Word::WidthMismatch("ulp_steps");
    GECODE_POST;
    if (fa.width <= 8 && fb.width <= 8) {
      GECODE_ES_FAIL((Word::Float::TinyBinary<TinySteps, WordView,
                                              Int::ConstIntView>::post(
          home, a, b, distance, Int::ConstIntView(0),
          TinySteps{fa, fb, format}, vbd(ipl) == IPL_DOM)));
      return;
    }
    WordVar ae = reference_encoding(home, reference_format, a);
    if (a.varimp() == b.varimp()) {
      dom(home, distance, 0);
      return;
    }
    WordVar be = reference_encoding(home, reference_format, b);
    rank_distance(home, ae, be, distance);
  }
  void ulp_error(Home home, WordDomainType reference_format, WordVar a,
                 WordVar b, WordVar reference,
                 const IntVarArgs& numerator_digits, IntPropLevel ipl) {
    const WordDomainType a_format = Word::Float::float_format(a, "ulp_error");
    const WordDomainType b_format = Word::Float::float_format(b, "ulp_error");
    const WordDomainType r_format =
        Word::Float::float_format(reference, "ulp_error");
    const ErrorLayout error = error_layout(reference_format);
    const ErrorFormats formats = {layout(a_format), layout(b_format),
                                  layout(r_format), layout(reference_format),
                                  error};
    if (numerator_digits.size() != error.digits)
      throw Word::OutOfLimits("ulp_error");
    GECODE_POST;
    dom(home, numerator_digits, 0, (1 << 30) - 1);
    dom(home, numerator_digits[0], 0, (1 << error.leading_bits) - 1);
    if (vbd(ipl) == IPL_DOM && formats.a.width <= 8 && formats.b.width <= 8 &&
        formats.r.width <= 8) {
      if (home.failed())
        return;
      ViewArray<Int::IntView> digits(home, numerator_digits);
      GECODE_ES_FAIL(TinyError::post(home, a, b, reference, digits, formats));
      return;
    }
    WordVar ae = exact_fp32(home, a, a_format),
            be = exact_fp32(home, b, b_format);
    WordVarArgs difference = absolute_difference(home, ae, be);
    IntVar q(home, error.qmin, error.qmax),
        shift(home, 0, error.qmax - error.qmin);
    ulp_spacing(home, reference_format, reference, q);
    linear(home, IntVarArgs{q, shift}, IRT_EQ, error.qmax);
    WordVarArgs padded(error.digits);
    for (int i = 0; i < error.digits; ++i)
      padded[i] = i < error.digits - difference.size()
                      ? WordVar(home, 30, {0})
                      : difference[i - (error.digits - difference.size())];
    WordVarArgs numerator = shift_limbs_left(home, padded, shift);
    for (int i = 0; i < error.digits; ++i)
      channel(home, numerator[i], numerator_digits[i], WI_UNSIGNED);
  }

  IntVarArray ulp_error_digits(Space& home, WordDomainType reference_format) {
    const ErrorLayout error = error_layout(reference_format);
    IntVarArray digits(home, error.digits, 0, (1 << 30) - 1);
    digits[0] = IntVar(home, 0, (1 << error.leading_bits) - 1);
    return digits;
  }

  int ulp_error_scale(WordDomainType reference_format) {
    return error_layout(reference_format).scale;
  }
} // namespace Gecode

// STATISTICS: word-post
