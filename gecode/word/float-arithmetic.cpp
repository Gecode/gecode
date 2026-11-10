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

#include <algorithm>
#include <cstdint>
#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float-wide.hpp>
#include <gecode/word/float.hpp>
#include <optional>
#include <utility>
#include <gecode/word/float/arithmetic.hpp>

namespace Gecode {
  namespace {
    /** \brief Return the common format of a, b and result after checking
     * the formats and then the policy, reporting errors at \a name
     */
    WordDomainType validate_arithmetic(const Policy& policy, WordVar a,
                                       WordVar b, WordVar result,
                                       const char* name) {
      const WordDomainType format = Word::Float::float_format(a, name);
      const WordDomainType b_format = Word::Float::float_format(b, name);
      const WordDomainType result_format =
          Word::Float::float_format(result, name);
      if (b_format != format || result_format != format)
        throw Word::FormatMismatch(name);
      if (format != WDT_BF16 && format != WDT_FP32 && format != WDT_E2M1 &&
          format != WDT_E4M3FN)
        throw Word::OutOfLimits(name);
      Word::Float::check(policy, name);
      return format;
    }

    /// Exact result and flags of \a operation on the encodings a and b
    WideResult evaluate_assigned(const WideFormat& format,
                                 const Policy& policy, Arithmetic operation,
                                 WordValue a, WordValue b) {
      switch (operation) {
      case Arithmetic::Multiply:
        return evaluate_multiply(format, policy, a, b);
      case Arithmetic::Divide:
        return evaluate_divide(format, policy, a, b);
      default:
        return evaluate_wide(format, policy, operation, a, b);
      }
    }

    void post_arithmetic(Home home, const Policy& policy, WordVar a,
                         WordVar b, WordVar result, IntVar* status,
                         Arithmetic operation, const char* name,
                         IntPropLevel ipl) {
      const WordDomainType format =
          validate_arithmetic(policy, a, b, result, name);
      const bool wide = format == WDT_BF16 || format == WDT_FP32;
      GECODE_POST;
      if (status != nullptr) {
        Int::IntView flags(*status);
        GECODE_ME_FAIL(flags.gq(home, 0));
        GECODE_ME_FAIL(flags.lq(home, 31));
      }
      if (wide && a.assigned() && b.assigned()) {
        const WideResult outcome = evaluate_assigned(
            WideFormat(format), policy, operation, a.val(), b.val());
        if (status != nullptr)
          GECODE_ME_FAIL(Int::IntView(*status).eq(home, outcome.status));
        GECODE_ME_FAIL(WordView(result).eq(home, outcome.code));
        return;
      }
      if (wide) {
        (void)new (home)
            WideArithmetic(home, a, b, result, status, policy, operation);
        return;
      }
      const TinyArithmetic tiny{TinyFormat(format), policy, operation};
      const Word::Float::Consistency level = Word::Float::consistency(ipl);
      if (status != nullptr)
        GECODE_ES_FAIL(
            (Word::Float::TinyBinary<TinyArithmetic, WordView,
                                     Int::IntView>::post(
                home, a, b, result, *status, tiny, level)));
      else
        GECODE_ES_FAIL(
            (Word::Float::TinyBinary<TinyArithmetic, WordView,
                                     Word::Float::OmittedStatus>::post(
                home, a, b, result, Word::Float::OmittedStatus(), tiny,
                level)));
    }
  } // namespace

  namespace Word {
    namespace Float {
      TinyResult round_tiny(const TinyFormat& format,
                            const FloatWordPolicy& policy, uint64_t numerator,
                            uint64_t denominator, bool negative) {
        return round_finite(format, policy, numerator, denominator, negative);
      }

      TinySummary summarise(const TinyFormat& format,
                            const FloatWordPolicy& policy,
                            const TinyValues& values) {
        TinySummary s;
        for (unsigned int i = 0; i < values.size(); ++i) {
          const WordValue code = values[i];
          if (format.is_nan(code)) {
            s.nans = with_value(s.nans, code);
            continue;
          }
          const int64_t v = format.value(code, policy);
          s.least = s.has_number ? std::min(s.least, v) : v;
          s.greatest = s.has_number ? std::max(s.greatest, v) : v;
          s.has_number = true;
          if (v < 0) {
            s.greatest_negative =
                s.has_negative ? std::max(s.greatest_negative, v) : v;
            s.has_negative = true;
          } else if (v > 0) {
            s.least_positive =
                s.has_positive ? std::min(s.least_positive, v) : v;
            s.has_positive = true;
          } else {
            s.has_zero = true;
          }
        }
        return s;
      }

      // Soundness of the tiny bound reasoning. The numerical value of
      // round_finite is non-decreasing in the exact result under RNE, RTZ,
      // RDN and RUP. Flushing output subnormals maps (-N, N) to a zero and
      // saturation maps beyond the greatest finite value M to M, both
      // monotone, and overflow, which needs |r| > M, gives the overflow
      // NaN only beyond the rounded bound that reaches it. Hence every
      // number that an exact result in [least, greatest] rounds to lies
      // between the rounded bounds, and overflow to a NaN on a side is
      // reachable exactly when that bound overflows to it. Underflow needs
      // 0 < |r| < N and overflow needs the overflowing bound. Every rounding
      // returns one of the two lattice neighbours of r, possibly flushed to
      // zero, so a result w comes from r between the neighbours of w, the
      // neighbours of zero being -N and N under output flushing, and a
      // result of magnitude M or an overflow NaN from any r beyond. Zero
      // signs are kept conservatively.
      TinyRounding round_interval(const TinyFormat& format,
                                  const FloatWordPolicy& policy,
                                  const TinyInterval& exact) {
        assert(!exact.unbounded_below && !exact.unbounded_above);
        TinyRounding rounding;
        const RoundedBound low = round_bound(format, policy, exact.least);
        const RoundedBound high = round_bound(format, policy, exact.greatest);
        if (low.is_nan)
          rounding.nan_below = low.code;
        if (high.is_nan)
          rounding.nan_above = high.code;
        // A NaN bound on the far side means every result overflows.
        if ((low.is_nan && exact.least.numerator > 0) ||
            (high.is_nan && exact.greatest.numerator < 0)) {
          if (low.is_nan && exact.least.numerator > 0)
            rounding.nan_below.reset();
          else
            rounding.nan_above.reset();
          return rounding;
        }
        const int least = low.is_nan ? -format.maximum_code : low.key;
        const int greatest = high.is_nan ? format.maximum_code : high.key;
        const bool flushes =
            policy.output_subnormals == FloatWordPolicy::SignedZero;
        for (int m = 0; m <= format.maximum_code; ++m) {
          if (flushes && m > 0 && m < format.normal())
            continue;
          if (least <= m && m <= greatest)
            rounding.numbers =
                with_value(rounding.numbers, static_cast<WordValue>(m));
          if (least <= -m && -m <= greatest &&
              !(m == 0 && policy.zeros == FloatWordPolicy::Positive))
            rounding.numbers = with_value(
                rounding.numbers, static_cast<WordValue>(format.sign_bit | m));
        }
        const TinyRational normal = key_value(format, format.normal());
        const TinyRational minus_normal = key_value(format, -format.normal());
        const bool underflows = (exact.greatest.numerator > 0 &&
                                 exact.least < normal) ||
                                (exact.least.numerator < 0 &&
                                 minus_normal < exact.greatest);
        rounding.number_flags = flag_bit(0) | flag_bit(inexact);
        if (underflows)
          rounding.number_flags |= flag_bit(underflow | inexact);
        if (low.overflows || high.overflows)
          rounding.number_flags |= flag_bit(overflow | inexact);
        return rounding;
      }

      std::optional<TinyInterval> preimage(const TinyFormat& format,
                                           const FloatWordPolicy& policy,
                                           WordValueSet numbers,
                                           const TinyRounding& rounding,
                                           WordValueSet nans) {
        bool found = false;
        int least = 0, greatest = 0;
        for (int code = 0; code < 2 * format.sign_bit; ++code) {
          const WordValue c = static_cast<WordValue>(code);
          if (!value_set_contains(numbers, c) || format.is_nan(c))
            continue;
          const int k = key(format, c);
          least = found ? std::min(least, k) : k;
          greatest = found ? std::max(greatest, k) : k;
          found = true;
        }
        const bool below =
            rounding.nan_below && value_set_contains(nans, *rounding.nan_below);
        const bool above =
            rounding.nan_above && value_set_contains(nans, *rounding.nan_above);
        const TinyRational maximum = key_value(format, format.maximum_code);
        TinyInterval reals;
        if (!found) {
          if (!below && !above)
            return std::nullopt;
          // Only overflow results: the reals beyond M on each side, as crossed
          // bounds whose unbounded sides give the rays (see TinyInterval).
          reals.least = maximum;
          reals.greatest = {-maximum.numerator, maximum.denominator};
          reals.unbounded_below = below;
          reals.unbounded_above = above;
          return reals;
        }
        const bool flushes =
            policy.output_subnormals == FloatWordPolicy::SignedZero;
        reals.unbounded_below = below || least == -format.maximum_code;
        reals.unbounded_above = above || greatest == format.maximum_code;
        if (!reals.unbounded_below)
          reals.least = key_value(format, least == 0 && flushes
                                              ? -format.normal()
                                              : least - 1);
        if (!reals.unbounded_above)
          reals.greatest = key_value(format, greatest == 0 && flushes
                                                 ? format.normal()
                                                 : greatest + 1);
        return reals;
      }
    } // namespace Float
  } // namespace Word

  void fp_add(Home home, WordVar a, WordVar b, WordVar result,
              IntPropLevel ipl) {
    fp_add(home, a, b, result, FloatWordPolicy(), ipl);
  }

  void fp_add(Home home, WordVar a, WordVar b, WordVar result,
              IntVar status, IntPropLevel ipl) {
    fp_add(home, a, b, result, status, FloatWordPolicy(), ipl);
  }

  void fp_add(Home home, WordVar a, WordVar b, WordVar result,
              const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Add,
                    "Word::fp_add", ipl);
  }

  void fp_add(Home home, WordVar a, WordVar b, WordVar result,
              IntVar status, const FloatWordPolicy& policy,
              IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Add,
                    "Word::fp_add", ipl);
  }

  void fp_subtract(Home home, WordVar a, WordVar b, WordVar result,
                   IntPropLevel ipl) {
    fp_subtract(home, a, b, result, FloatWordPolicy(), ipl);
  }

  void fp_subtract(Home home, WordVar a, WordVar b, WordVar result,
                   IntVar status, IntPropLevel ipl) {
    fp_subtract(home, a, b, result, status, FloatWordPolicy(), ipl);
  }

  void fp_subtract(Home home, WordVar a, WordVar b, WordVar result,
                   const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Subtract,
                    "Word::fp_subtract", ipl);
  }

  void fp_subtract(Home home, WordVar a, WordVar b, WordVar result,
                   IntVar status, const FloatWordPolicy& policy,
                   IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Subtract,
                    "Word::fp_subtract", ipl);
  }

  void fp_multiply(Home home, WordVar a, WordVar b, WordVar result,
                   IntPropLevel ipl) {
    fp_multiply(home, a, b, result, FloatWordPolicy(), ipl);
  }

  void fp_multiply(Home home, WordVar a, WordVar b, WordVar result,
                   IntVar status, IntPropLevel ipl) {
    fp_multiply(home, a, b, result, status, FloatWordPolicy(), ipl);
  }

  void fp_multiply(Home home, WordVar a, WordVar b, WordVar result,
                   const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Multiply,
                    "Word::fp_multiply", ipl);
  }

  void fp_multiply(Home home, WordVar a, WordVar b, WordVar result,
                   IntVar status, const FloatWordPolicy& policy,
                   IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Multiply,
                    "Word::fp_multiply", ipl);
  }

  void fp_divide(Home home, WordVar a, WordVar b, WordVar result,
                 IntPropLevel ipl) {
    fp_divide(home, a, b, result, FloatWordPolicy(), ipl);
  }

  void fp_divide(Home home, WordVar a, WordVar b, WordVar result,
                 IntVar status, IntPropLevel ipl) {
    fp_divide(home, a, b, result, status, FloatWordPolicy(), ipl);
  }

  void fp_divide(Home home, WordVar a, WordVar b, WordVar result,
                 const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Divide,
                    "Word::fp_divide", ipl);
  }

  void fp_divide(Home home, WordVar a, WordVar b, WordVar result,
                 IntVar status, const FloatWordPolicy& policy,
                 IntPropLevel ipl) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Divide,
                    "Word::fp_divide", ipl);
  }
} // namespace Gecode

// STATISTICS: word-post
