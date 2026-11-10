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

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    using Word::Float::admits_nonfinite;
    using Word::Float::binade_runs;
    using Word::Float::binade_unit;
    using Word::Float::BinadeRuns;
    using Word::Float::Direction;
    using Word::Float::divide_by_zero;
    using Word::Float::domain_codes;
    using Word::Float::exact_product;
    using Word::Float::ExactDyadic;
    using Word::Float::finite_parts;
    using Word::Float::Role;
    using Word::Float::FinitePart;
    using Word::Float::floor_encoding;
    using Word::Float::greatest_with_masks;
    using Word::Float::include_real;
    using Word::Float::include_rounded;
    using Word::Float::inexact;
    using Word::Float::invalid;
    using Word::Float::LatticeBudget;
    using Word::Float::LatticeLimit;
    using Word::Float::MagnitudeRounding;
    using Word::Float::magnitude_rounding;
    using Word::Float::known_run;
    using Word::Float::known_run_candidate;
    using Word::Float::KnownRun;
    using Word::Float::least_with_masks;
    using Word::Float::modular_candidate;
    using Word::Float::sum_term_extreme;
    using Word::Float::SumTarget;
    using Word::Float::SumTerm;
    using Word::Float::overflow;
    using Word::Float::overflow_code;
    using Word::Float::rounds_upward;
    using Word::Float::CellEnds;
    using Word::Float::rounding_cell;
    using Word::Float::quotient_bounds;
    using Word::Float::real_domain;
    using Word::Float::round_coefficient;
    using Word::Float::round_dyadic;
    using Word::Float::RoundingCell;
    using Word::Float::underflow;
    using Word::Float::visit_alias_classes;
    using Word::Float::TinyFormat;
    using Word::Float::wide_intersect;
    using Word::Float::wide_snapshot;
    using Word::Float::WideFormat;
    using Word::Float::WideProjection;
    using Word::Float::WideResult;
    using Word::Float::corner_quotients;
    using Word::Float::include_meeting;
    using Word::Float::include_values;
    using Word::Float::merge;
    using Word::Float::Read;
    using Word::Float::read_result;
    using Word::Float::read_word;
    using Word::Float::ResultCells;
    using Word::Float::ResultSign;
    using Word::Float::run_step_rule;
    using Word::Float::StepOutcome;
    using Word::Float::StepState;
    using Word::Float::ValueRange;

    enum class Arithmetic { Add, Subtract, Multiply, Divide };

    /// An operand of a binary operation
    enum class Operand { First, Second };

    using ArithmeticResult = Word::Float::TinyResult;

    bool rounds_up(uint64_t quotient, uint64_t remainder, uint64_t denominator,
                   bool negative, Policy::Rounding rounding) {
      if (remainder == 0)
        return false;
      switch (magnitude_rounding(rounding, negative)) {
      case MagnitudeRounding::Nearest:
        return remainder * 2 > denominator ||
               (remainder * 2 == denominator && (quotient & 1) != 0);
      case MagnitudeRounding::Down:
        return false;
      case MagnitudeRounding::Up:
        return true;
      }
      GECODE_NEVER;
      return false;
    }

    // Round a positive rational to p bits with an unbounded exponent.
    // Across both tiny formats n < 2^36, 0 < d <= 2^18 and -18 <=
    // floor(log2(n/d)) <= 17. The grid exponent lies in [-21,16]. Even using
    // independent bounds, n << 21 < 2^57 and d << 16 <= 2^34; all shifts are
    // unsigned.
    struct PrecisionResult {
      uint64_t significand;
      int exponent;
    };
    PrecisionResult round_precision(uint64_t n, uint64_t d, bool negative,
                                    const TinyFormat& format,
                                    Policy::Rounding rounding) {
      int exponent = 0;
      if (n >= d) {
        while (n >= (d << (exponent + 1)))
          ++exponent;
      } else {
        while ((n << -exponent) < d)
          --exponent;
      }
      exponent -= static_cast<int>(format.fraction_bits);
      const uint64_t numerator = exponent < 0 ? n << -exponent : n;
      const uint64_t denominator = exponent > 0 ? d << exponent : d;
      const uint64_t quotient = numerator / denominator;
      return {quotient + rounds_up(quotient, numerator % denominator,
                                   denominator, negative, rounding),
              exponent};
    }

    // Compare the precision-rounded value with an integer number of subnormal
    // units.
    int compare_precision(PrecisionResult rounded, uint64_t units,
                          const TinyFormat& format) {
      uint64_t left = rounded.significand * format.units_per_one;
      uint64_t right = units;
      if (rounded.exponent >= 0)
        left <<= rounded.exponent;
      else
        right <<= -rounded.exponent;
      // left <= 16*512*2^16; right < 2^18*2^21.
      return left < right ? -1 : left > right ? 1 : 0;
    }

    ArithmeticResult fallback(const TinyFormat& format, const Policy& policy,
                              bool negative, int status) {
      const bool nan =
          format.has_nan() && policy.overflow == Policy::FormatDefault;
      const int magnitude = nan ? format.canonical_nan() : format.maximum_code;
      return {magnitude |
                  ((negative && !(nan && policy.nan == Policy::Canonical))
                       ? format.sign_bit
                       : 0),
              status};
    }

    ArithmeticResult round_finite(const TinyFormat& format,
                                  const Policy& policy, uint64_t n, uint64_t d,
                                  bool negative) {
      if (n == 0)
        return {negative && policy.zeros == Policy::Signed ? format.sign_bit
                                                           : 0,
                0};
      const PrecisionResult precision =
          round_precision(n, d, negative, format, policy.rounding);
      if (compare_precision(precision, format.units(format.maximum_code),
                            format) > 0)
        return fallback(format, policy, negative, overflow | inexact);
      // Direct lattice rounding, never a second rounding of the auxiliary
      // value. n*units_per_one < 2^45; units*d < 2^36; midpoint comparisons
      // stay below 2^46.
      const uint64_t scaled = n * format.units_per_one;
      int lower = 0, upper = format.maximum_code;
      while (lower < upper) {
        const int middle = (lower + upper + 1) / 2;
        if (format.units(middle) * d <= scaled)
          lower = middle;
        else
          upper = middle - 1;
      }
      int magnitude = lower;
      if (lower < format.maximum_code) {
        const uint64_t low = format.units(lower) * d;
        const uint64_t interval =
            (format.units(lower + 1) - format.units(lower)) * d;
        if (rounds_up(static_cast<uint64_t>(lower), scaled - low, interval,
                      negative, policy.rounding))
          ++magnitude;
      }
      int status = format.units(magnitude) * d == scaled ? 0 : inexact;
      const uint64_t normal = uint64_t(1) << format.fraction_bits;
      const bool tiny = policy.tininess == Policy::BeforeRounding
                            ? scaled < normal * d
                            : compare_precision(precision, normal, format) < 0;
      if (tiny && status != 0)
        status |= underflow;
      if (policy.output_subnormals == Policy::SignedZero && magnitude > 0 &&
          magnitude < static_cast<int>(normal)) {
        magnitude = 0;
        status |= underflow | inexact;
      }
      if (magnitude == 0 && policy.zeros == Policy::Positive)
        negative = false;
      return {magnitude | (negative ? format.sign_bit : 0), status};
    }

    ArithmeticResult evaluate(const TinyFormat& format, const Policy& policy,
                              Arithmetic operation, int a, int b) {
      int a_magnitude = a & (format.sign_bit - 1),
          b_magnitude = b & (format.sign_bit - 1);
      // NaN selection precedes subtraction's numerical sign change.
      if (format.is_nan(static_cast<WordValue>(a)) ||
          format.is_nan(static_cast<WordValue>(b)))
        return {policy.nan == Policy::Canonical    ? format.canonical_nan()
                : format.is_nan(static_cast<WordValue>(a)) ? a
                                                           : b,
                0};
      if (policy.input_subnormals == Policy::SignedZero) {
        if (a_magnitude < (1 << format.fraction_bits))
          a_magnitude = 0;
        if (b_magnitude < (1 << format.fraction_bits))
          b_magnitude = 0;
      }
      const bool a_negative = (a & format.sign_bit) != 0;
      const bool b_negative =
          ((b & format.sign_bit) != 0) != (operation == Arithmetic::Subtract);
      const uint64_t a_units = format.units(a_magnitude),
                     b_units = format.units(b_magnitude);
      switch (operation) {
      case Arithmetic::Add:
      case Arithmetic::Subtract: {
        const int64_t sum = (a_negative ? -static_cast<int64_t>(a_units)
                                        : static_cast<int64_t>(a_units)) +
                            (b_negative ? -static_cast<int64_t>(b_units)
                                        : static_cast<int64_t>(b_units));
        const bool negative = sum == 0 ? (a_negative == b_negative
                                              ? a_negative
                                              : policy.rounding == Policy::RDN)
                                       : sum < 0;
        return round_finite(format, policy,
                            static_cast<uint64_t>(sum < 0 ? -sum : sum),
                            format.units_per_one, negative);
      }
      case Arithmetic::Multiply:
        return round_finite(format, policy, a_units * b_units,
                            format.units_per_one * format.units_per_one,
                            a_negative != b_negative);
      case Arithmetic::Divide:
        if (b_units == 0) {
          if (a_units == 0)
            return {format.has_nan() ? format.canonical_nan()
                                     : format.maximum_code,
                    invalid};
          return fallback(format, policy, a_negative != b_negative,
                          divide_by_zero);
        }
        return round_finite(format, policy, a_units, b_units,
                            a_negative != b_negative);
      }
      GECODE_NEVER;
      return {0, 0};
    }

    using Word::WordValueSet;
    using Word::WordView;
    using Word::Float::failed_change;
    using Word::Float::TinyInterval;
    using Word::Float::TinyOutcomes;
    using Word::Float::TinyRational;
    using Word::Float::TinyRounding;
    using Word::Float::TinySummary;
    using Word::Float::TinyValues;

    /// Return the key of the number \a code: its magnitude code, negated
    /// when the sign bit is set
    int key(const TinyFormat& format, WordValue code) {
      const int magnitude = static_cast<int>(code) & (format.sign_bit - 1);
      return (static_cast<int>(code) & format.sign_bit) != 0 ? -magnitude
                                                             : magnitude;
    }
    /// Return the value of the number with key \a k
    TinyRational key_value(const TinyFormat& format, int k) {
      const int64_t units =
          static_cast<int64_t>(format.units(k < 0 ? -k : k));
      return {k < 0 ? -units : units,
              static_cast<int64_t>(format.units_per_one)};
    }
    /// Return \a numerator / \a denominator with a positive denominator
    TinyRational ratio(int64_t numerator, int64_t denominator) {
      assert(denominator != 0);
      return denominator < 0 ? TinyRational{-numerator, -denominator}
                             : TinyRational{numerator, denominator};
    }

    /// Rounding of one exact real: a number key or overflow to a NaN
    struct RoundedBound {
      WordValue code;
      int key;
      bool is_nan, overflows;
    };
    RoundedBound round_bound(const TinyFormat& format, const Policy& policy,
                             TinyRational exact) {
      const bool negative = exact.numerator < 0;
      const ArithmeticResult r = round_finite(
          format, policy,
          static_cast<uint64_t>(negative ? -exact.numerator
                                         : exact.numerator),
          static_cast<uint64_t>(exact.denominator), negative);
      const WordValue code = static_cast<WordValue>(r.code);
      return {code, key(format, code), format.is_nan(code),
              (r.status & overflow) != 0};
    }

    /// Bit of the flag value \a flags in a set of flag values
    constexpr unsigned int flag_bit(int flags) { return 1U << flags; }

    /// Nonzero divisors of one sign, from least to greatest
    struct DivisorPart {
      bool exists;
      int64_t least, greatest;
    };
    DivisorPart negative_divisors(const TinySummary& s) {
      return {s.has_negative, s.least, s.greatest_negative};
    }
    DivisorPart positive_divisors(const TinySummary& s) {
      return {s.has_positive, s.least_positive, s.greatest};
    }
    /// Return the hull of the quotients of numbers from \a least to
    /// \a greatest by the nonempty divisor part \a part
    TinyInterval quotient_hull(int64_t least, int64_t greatest,
                               DivisorPart part) {
      assert(part.exists);
      const TinyRational corners[] = {
          ratio(least, part.least), ratio(least, part.greatest),
          ratio(greatest, part.least), ratio(greatest, part.greatest)};
      return Word::Float::hull(corners, 4);
    }

    /// Outcomes of division by an effective zero that the result admits
    struct ZeroDivisions {
      /// Whether a nonzero number divided by zero has a kept outcome
      bool nonzero_by_zero = false;
      /// Whether zero divided by zero has a kept outcome
      bool zero_by_zero = false;
    };

    /** \brief Tiny addition, subtraction, multiplication or division
     *
     * The exact outcome evaluates one pair of encodings. The default
     * reasoning reads each operand once (see narrow_bounds) and never
     * combines operand values.
     */
    struct TinyArithmetic {
      TinyFormat format;
      Policy policy;
      Arithmetic kind;
      /// TinyBinary tabulates the outcomes of all operand pairs
      static constexpr bool tabulated = true;

      Word::Float::TinyOutcome operator()(WordValue x, WordValue y) const {
        const ArithmeticResult r = evaluate(
            format, policy, kind, static_cast<int>(x), static_cast<int>(y));
        return {static_cast<WordValue>(r.code), r.status};
      }

      /// Return the hull of the exact results of the numbers, if any
      std::optional<TinyInterval>
      exact_results(const TinySummary& x, const TinySummary& y) const {
        const int64_t one = static_cast<int64_t>(format.units_per_one);
        if (!x.has_number || !y.has_number)
          return std::nullopt;
        switch (kind) {
        case Arithmetic::Add:
          return TinyInterval{{x.least + y.least, one},
                              {x.greatest + y.greatest, one}};
        case Arithmetic::Subtract:
          return TinyInterval{{x.least - y.greatest, one},
                              {x.greatest - y.least, one}};
        case Arithmetic::Multiply: {
          const TinyRational corners[] = {{x.least * y.least, one * one},
                                          {x.least * y.greatest, one * one},
                                          {x.greatest * y.least, one * one},
                                          {x.greatest * y.greatest, one * one}};
          return Word::Float::hull(corners, 4);
        }
        case Arithmetic::Divide: {
          const DivisorPart parts[] = {negative_divisors(y),
                                       positive_divisors(y)};
          std::optional<TinyInterval> results;
          for (const DivisorPart& part : parts) {
            if (!part.exists)
              continue;
            const TinyInterval q = quotient_hull(x.least, x.greatest, part);
            if (results) {
              results->least = std::min(results->least, q.least);
              results->greatest = std::max(results->greatest, q.greatest);
            } else {
              results = q;
            }
          }
          return results;
        }
        }
        GECODE_NEVER;
        return std::nullopt;
      }

      /** \brief Test whether the number \a value of operand \a i combines
       * with a number of the \a other operand into an exact result in
       * \a reals
       */
      bool reaches(unsigned int i, int64_t value, const TinySummary& other,
                   const TinyInterval& reals) const {
        const int64_t one = static_cast<int64_t>(format.units_per_one);
        if (!other.has_number)
          return false;
        switch (kind) {
        case Arithmetic::Add:
          return reals.meets({value + other.least, one},
                             {value + other.greatest, one});
        case Arithmetic::Subtract:
          return i == 0 ? reals.meets({value - other.greatest, one},
                                      {value - other.least, one})
                        : reals.meets({other.least - value, one},
                                      {other.greatest - value, one});
        case Arithmetic::Multiply: {
          const TinyRational ends[] = {{value * other.least, one * one},
                                       {value * other.greatest, one * one}};
          const TinyInterval products = Word::Float::hull(ends, 2);
          return reals.meets(products.least, products.greatest);
        }
        case Arithmetic::Divide:
          if (i == 1) {
            if (value == 0)
              return false;
            const TinyRational ends[] = {ratio(other.least, value),
                                         ratio(other.greatest, value)};
            const TinyInterval quotients = Word::Float::hull(ends, 2);
            return reals.meets(quotients.least, quotients.greatest);
          }
          for (const DivisorPart& part :
               {negative_divisors(other), positive_divisors(other)})
            if (part.exists) {
              const TinyInterval q = quotient_hull(value, value, part);
              if (reals.meets(q.least, q.greatest))
                return true;
            }
          return false;
        }
        GECODE_NEVER;
        return false;
      }

      /// Keep the outcomes of division by an effective zero
      ZeroDivisions keep_zero_divisions(const TinySummary& x,
                                        const TinySummary& y,
                                        TinyOutcomes& outcomes) const {
        ZeroDivisions kept;
        if (kind != Arithmetic::Divide || !y.has_zero)
          return kept;
        if (x.has_negative || x.has_positive) {
          WordValueSet codes = {{0, 0, 0, 0}};
          for (bool negative : {false, true})
            codes = Word::with_value(
                codes, static_cast<WordValue>(
                           fallback(format, policy, negative, divide_by_zero)
                               .code));
          kept.nonzero_by_zero = !Word::is_empty_value_set(
              outcomes.keep(codes, flag_bit(divide_by_zero)));
        }
        if (x.has_zero) {
          const ArithmeticResult r =
              evaluate(format, policy, Arithmetic::Divide, 0, 0);
          kept.zero_by_zero = !Word::is_empty_value_set(
              outcomes.keep(Word::singleton_value_set(
                                static_cast<WordValue>(r.code)),
                            flag_bit(r.status)));
        }
        return kept;
      }

      /// Test whether the number \a value of operand \a i has a kept outcome
      bool keeps_number(unsigned int i, int64_t value,
                        const TinySummary& other,
                        const Word::Float::TinyNans& nans,
                        const std::optional<TinyInterval>& reals,
                        ZeroDivisions zero_divisions) const {
        if (nans.keeps_number(i) || (reals && reaches(i, value, other, *reals)))
          return true;
        if (i == 0)
          return value != 0 ? zero_divisions.nonzero_by_zero
                            : zero_divisions.zero_by_zero;
        return value == 0 &&
               (zero_divisions.nonzero_by_zero || zero_divisions.zero_by_zero);
      }

      /** \brief Narrow to the outcomes that the numerical bounds of the
       * operands allow
       *
       * Reads each operand once: its NaNs and its least and greatest
       * numbers, and the nonzero divisors closest to zero. The result keeps
       * the codes between the rounded least and greatest exact results,
       * the NaNs of NaN operands and of overflow, and for division the
       * outcomes of division by zero. Each operand number is kept when the
       * other operand's bounds can combine it into an exact result that
       * rounds to a kept number or overflows to a kept NaN (see
       * Word::Float::preimage), when it pairs with a kept NaN, or when it
       * takes part in a kept division by zero. A NaN operand is kept when
       * its NaN result is kept. The status keeps the flags of the kept
       * classes of outcomes.
       */
      template<class StatusView>
      ExecStatus narrow_bounds(Space& home, WordView x, WordView y,
                               WordView result, StatusView status) const {
        const TinyValues xs(x), ys(y);
        const TinySummary operands[] = {
            Word::Float::summarise(format, policy, xs),
            Word::Float::summarise(format, policy, ys)};
        TinyOutcomes outcomes(result.values(),
                              Word::Float::admitted_flags(status));
        TinyRounding rounding;
        if (const auto exact = exact_results(operands[0], operands[1]))
          rounding = Word::Float::round_interval(format, policy, *exact);
        const WordValueSet numbers =
            outcomes.keep(rounding.numbers, rounding.number_flags);
        const WordValueSet overflow_nans = outcomes.keep(
            rounding.overflow_nans(), flag_bit(overflow | inexact));
        const Word::Float::TinyNans nans(
            policy, operands, 2,
            outcomes.keep(Word::Float::TinyNans::results(policy, operands, 2),
                          flag_bit(0)));
        const ZeroDivisions zero_divisions =
            keep_zero_divisions(operands[0], operands[1], outcomes);
        if (Word::is_empty_value_set(outcomes.results()))
          return ES_FAILED;
        const std::optional<TinyInterval> reals = Word::Float::preimage(
            format, policy, numbers, rounding, overflow_nans);
        bool changed = false;
        if (failed_change(result.narrow_values(home, outcomes.results()),
                          changed) ||
            failed_change(Word::Float::narrow_flags(home, status,
                                                    outcomes.statuses()),
                          changed))
          return ES_FAILED;
        for (unsigned int i = 0; i < 2; ++i) {
          const ModEvent me = Word::Float::narrow_kept(
              home, i == 0 ? x : y, i == 0 ? xs : ys, [&](WordValue code) {
                return format.is_nan(code)
                           ? nans.keeps_nan(i, code)
                           : keeps_number(i, format.value(code, policy),
                                          operands[1 - i], nans, reals,
                                          zero_divisions);
              });
          if (failed_change(me, changed))
            return ES_FAILED;
        }
        return changed ? ES_NOFIX : ES_FIX;
      }
    };

    using Integer = Word::Arithmetic::Global::Integer;

    WideResult round_wide(const WideFormat& f, const Policy& p,
                          const Integer& exact, bool negative_zero = false) {
      bool negative = exact.zero() ? negative_zero : exact.neg();
      const Integer magnitude = exact.abs();
      if (magnitude.zero())
        return {negative && p.zeros == Policy::Signed ? f.sign : 0, 0};
      // Finite same-format sums are multiples of the minimum subnormal.
      // Therefore every tiny sum is exact before output treatment.
      const unsigned int shift = magnitude.bits() > f.fraction_bits + 1
                                     ? magnitude.bits() - f.fraction_bits - 1
                                     : 1;
      const Integer rounded =
          round_coefficient(magnitude, shift, negative, p.rounding);
      if (rounded > f.magnitude(f.maximum)) {
        return {overflow_code(f, p, negative), overflow | inexact};
      }
      WordValue code = floor_encoding(f, rounded);
      int flags = rounded == magnitude ? 0 : inexact;
      if (code > 0 && code < f.normal &&
          p.output_subnormals == Policy::SignedZero) {
        code = 0;
        flags |= underflow | inexact;
      }
      if (code == 0 && p.zeros == Policy::Positive)
        negative = false;
      return {code | (negative ? f.sign : 0), flags};
    }

    /// The result of a binary operation on \a a and \a b when one is a NaN
    std::optional<WideResult> nan_result(const WideFormat& f, const Policy& p,
                                         WordValue a, WordValue b) {
      const bool a_nan = f.nan(a), b_nan = f.nan(b);
      const int flags = f.signaling(a) || f.signaling(b) ? invalid : 0;
      if (!a_nan && !b_nan)
        return std::nullopt;
      WordValue selected = a_nan ? a : b;
      if (p.nan == Policy::SignalFirst && flags)
        selected = f.signaling(a) ? a : b;
      return WideResult{p.nan == Policy::Canonical ? f.infinity | f.quiet
                                                   : selected | f.quiet,
                        flags};
    }

    /// Replace subnormal operands \a a and \a b by zeros under \a p
    void flush_inputs(const WideFormat& f, const Policy& p, WordValue& a,
                      WordValue& b) {
      if (p.input_subnormals == Policy::SignedZero) {
        if ((a & (f.sign - 1)) < f.normal)
          a &= f.sign;
        if ((b & (f.sign - 1)) < f.normal)
          b &= f.sign;
      }
    }

    WideResult evaluate_multiply(const WideFormat& f, const Policy& p,
                                 WordValue a, WordValue b) {
      if (const std::optional<WideResult> nan = nan_result(f, p, a, b))
        return *nan;
      flush_inputs(f, p, a, b);
      const WordValue am = a & (f.sign - 1), bm = b & (f.sign - 1);
      const WordValue sign = (a ^ b) & f.sign;
      if (am == f.infinity || bm == f.infinity) {
        if (am == 0 || bm == 0)
          return {f.infinity | f.quiet, invalid};
        return {sign | f.infinity, 0};
      }
      return round_dyadic(f, p, exact_product(f, a, b));
    }

    // Compare a coefficient times a signed power of two with an integer.
    // Division can have an exponent below the destination subnormal grid.
    int compare_division_scale(const Integer& coefficient, int exponent,
                               const Integer& other) {
      return exponent >= 0
                 ? compare(coefficient.shift_left(
                               static_cast<unsigned int>(exponent)),
                           other)
                 : compare(coefficient,
                           other.shift_left(
                               static_cast<unsigned int>(-exponent)));
    }

    Integer round_quotient_grid(Integer numerator, Integer denominator,
                                int exponent, bool negative,
                                Policy::Rounding rounding) {
      if (exponent < 0)
        numerator = numerator.shift_left(static_cast<unsigned int>(-exponent));
      else
        denominator =
            denominator.shift_left(static_cast<unsigned int>(exponent));
      Integer quotient, remainder;
      Integer::divide(numerator, denominator, quotient, remainder);
      bool increment = false;
      if (!remainder.zero()) {
        switch (magnitude_rounding(rounding, negative)) {
        case MagnitudeRounding::Nearest:
          increment = remainder + remainder > denominator ||
                      (remainder + remainder == denominator && quotient.bit(0));
          break;
        case MagnitudeRounding::Down:
          break;
        case MagnitudeRounding::Up:
          increment = true;
          break;
        }
      }
      return quotient + Integer(increment ? 1 : 0);
    }

    // Positive numerator/denominator represent the exact real magnitude.
    // Precision rounding decides range flags; destination rounding always
    // starts from the original quotient, including below the subnormal grid.
    WideResult round_quotient(const WideFormat& f, const Policy& p,
                              const Integer& numerator,
                              const Integer& denominator, bool negative) {
      assert(!numerator.zero() && !denominator.zero() && !numerator.neg() &&
             !denominator.neg());
      int exponent = static_cast<int>(numerator.bits()) -
                     static_cast<int>(denominator.bits());
      if (compare_division_scale(denominator, exponent, numerator) > 0)
        --exponent;
      const int precision_grid = exponent - static_cast<int>(f.fraction_bits);
      const int source_scale = 127 + static_cast<int>(f.fraction_bits);
      const Integer precision = round_quotient_grid(
          numerator, denominator, precision_grid, negative, p.rounding);
      if (compare_division_scale(precision, precision_grid + source_scale,
                                 f.magnitude(f.maximum)) > 0) {
        return {overflow_code(f, p, negative), overflow | inexact};
      }
      const int destination_grid = std::max(precision_grid, 1 - source_scale);
      const Integer rounded =
          round_quotient_grid(numerator, denominator, destination_grid,
                              negative, p.rounding)
              .shift_left(
                  static_cast<unsigned int>(destination_grid + source_scale));
      WordValue code = floor_encoding(f, rounded);
      const Integer scaled_numerator =
          numerator.shift_left(static_cast<unsigned int>(source_scale));
      int flags = rounded * denominator == scaled_numerator ? 0 : inexact;
      const Integer normal = f.magnitude(f.normal);
      const bool precision_tiny =
          compare_division_scale(precision, precision_grid + source_scale,
                                 normal) < 0;
      const bool tiny = p.tininess == Policy::BeforeRounding
                            ? scaled_numerator < normal * denominator
                            : precision_tiny;
      if (tiny && flags)
        flags |= underflow;
      if (p.output_subnormals == Policy::SignedZero && code > 0 &&
          code < f.normal) {
        code = 0;
        flags |= underflow | inexact;
      }
      if (code == 0 && p.zeros == Policy::Positive)
        negative = false;
      return {code | (negative ? f.sign : 0), flags};
    }

    WideResult evaluate_divide(const WideFormat& f, const Policy& p,
                               WordValue a, WordValue b) {
      if (const std::optional<WideResult> nan = nan_result(f, p, a, b))
        return *nan;
      flush_inputs(f, p, a, b);
      const WordValue am = a & (f.sign - 1), bm = b & (f.sign - 1);
      const WordValue sign = (a ^ b) & f.sign;
      if ((am == 0 && bm == 0) || (am == f.infinity && bm == f.infinity))
        return {f.infinity | f.quiet, invalid};
      if (am == f.infinity)
        return {sign | f.infinity, 0};
      if (bm == 0)
        return {sign | f.infinity, divide_by_zero};
      if (am == 0 || bm == f.infinity)
        return {p.zeros == Policy::Signed ? sign : 0, 0};
      return round_quotient(f, p, f.magnitude(am), f.magnitude(bm), sign != 0);
    }

    WideResult evaluate_wide(const WideFormat& f, const Policy& p,
                             Arithmetic op, WordValue a, WordValue b) {
      if (const std::optional<WideResult> nan = nan_result(f, p, a, b))
        return *nan;
      const bool negative_a = (a & f.sign) != 0;
      const bool negative_b =
          ((b & f.sign) != 0) != (op == Arithmetic::Subtract);
      WordValue a_magnitude = a & (f.sign - 1), b_magnitude = b & (f.sign - 1);
      if (a_magnitude == f.infinity || b_magnitude == f.infinity) {
        if (a_magnitude == f.infinity && b_magnitude == f.infinity &&
            negative_a != negative_b)
          return {f.infinity | f.quiet, invalid};
        return {f.infinity |
                    ((a_magnitude == f.infinity ? negative_a : negative_b)
                         ? f.sign
                         : 0),
                0};
      }
      if (p.input_subnormals == Policy::SignedZero) {
        if (a_magnitude < f.normal)
          a_magnitude = 0;
        if (b_magnitude < f.normal)
          b_magnitude = 0;
      }
      const Integer av = f.magnitude(a_magnitude),
                    bv = f.magnitude(b_magnitude);
      const Integer sum = (negative_a ? -av : av) + (negative_b ? -bv : bv);
      return round_wide(f, p, sum,
                        negative_a == negative_b ? negative_a
                                                 : p.rounding == Policy::RDN);
    }

    bool retain_inverse_operands(const WideFormat& f, WordView result,
                                 const Word::WordDomain& domain) {
      // Zero flushing and maximum-finite saturation/directed tails can have
      // larger preimages. Keep these branches rather than guessing a cutoff.
      if (admits_nonfinite(f, result, domain))
        return true;
      for (WordValue sign : {WordValue(0), f.sign})
        for (WordValue magnitude : {WordValue(0), f.maximum})
          if (wide_intersect(result, domain, sign | magnitude, sign | magnitude)
                  .is_consistent)
            return true;
      return false;
    }

    /**
     * \brief Steps a lattice search takes before it hands back
     *
     * A step passes a binade of the free operand that holds no candidate or
     * a candidate that fails its exact test, so the bound that the search
     * returns after this many steps lies beyond the bound it started from,
     * and every code it passed over is unsupported. Where that bound is the
     * whole projection of the operand, the pass publishes it and the next
     * pass continues from it. Where it is one part of a union, a cut can
     * hide a narrowing, so a pass that narrows nothing after a cut is
     * repeated with exhaustive searches (LatticeBudget). Either way the
     * limit never changes a final domain. The sum search (SumLattice) counts
     * only failed candidates, as it visits at most 254 binades.
     */
    constexpr unsigned int lattice_search_limit = 64;

    /** \brief Lattice search for the free operand of a sum whose other
     * operand has one value
     *
     * The free operand x has magnitude X*u in a binade with spacing u =
     * 2^a, X its significand, and the other operand is the one value y.
     * Read with the sign of the result part z, the exact sum has magnitude
     * m = e*X*u + Y, with e = +1 or -1 and Y fixed, and a code of z in
     * binade b, with spacing g = 2^b, has magnitude Z*g. Within the cells
     * of the least and the greatest admitted code of that binade, z rounds
     * m to Z = R(m/g), R the floor, the ceiling or the nearest integer with
     * ties to even: inside a binade every cell is that of R, the gap above
     * the greatest code of a binade is g as well, and the cell of its least
     * code lies within the cell of R. So, all spacings being powers of two,
     * x supports z there iff X meets the known bits of x and R(m/g) meets
     * the known fraction bits of z.
     *
     * For e = -1 the reflection X' = ~X = -X - 1 gives m = X'*u + Y + u,
     * with the known bits of X complemented, so let e = +1 and w = Y (+ g-1
     * for the ceiling, + g/2 for the nearest integer). Then floor((X*u +
     * w)/g) is X*2^(a-b) + floor(w/g) for a >= b, and (X + floor(w/u)) >>
     * (b-a) for a < b. Either way the known bits of Z are known bits of X
     * plus a constant, the low a-b bits of Z being fixed for a >= b, and
     * least_with_masks finds the extreme X in a constant number of steps.
     * The sums are taken modulo 2^64, which keeps the bits of Z exact as
     * long as b-a plus the fraction bits stays below 64.
     *
     * Ties to even need care where m/g can be an odd integer plus 1/2. For
     * a >= b either every X ties or none, and Z is then one less for an odd
     * floor, which a constant parity or, for a = b, bit 0 cleared expresses.
     * For a < b exactly the X with X + floor(w/u) a multiple of 2^(b-a)
     * tie: the supports are those whose floor is even, those whose floor
     * minus one is even and meets the bits, and those that do not tie,
     * split by the lowest set bit of X + floor(w/u).
     *
     * Where the clipped significands of x round to at most four
     * significands of z, which always holds once b-a exceeds the fraction
     * bits plus one, the search instead takes, per significand of z that
     * meets its bits, the significands of x in its cell.
     *
     * So the search is exact: it visits each binade of x at most once and,
     * per binade of x, the result binades its sums reach. Every candidate
     * is still checked by evaluate_wide, and the search continues past one
     * that fails.
     */
    struct SumLattice {
      const WideFormat& f;
      const Policy& p;
      Arithmetic op;
      /// The operand that the search varies
      Operand free;
      WordView free_view;
      /// The sign bit of the free part
      WordValue free_sign;
      /// The code and the value of the other operand
      WordValue fixed_code;
      Integer fixed;
      WordView result_view;
      const FinitePart& z;
      LatticeBudget& budget;
    };

    // The low 64 bits of x in two's complement
    std::uint64_t low_word(const Integer& x) {
      const std::uint64_t word = x.abs().low_bits(64).word();
      return x.neg() ? std::uint64_t(0) - word : word;
    }

    // The significand R(m/2^b) of magnitude m
    Integer magnitude_index(const Integer& m, unsigned int b,
                            MagnitudeRounding rounding) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      if (rounding == MagnitudeRounding::Down)
        return floor_shift_right(m, b);
      if (rounding == MagnitudeRounding::Up)
        return ceil_shift_right(m, b);
      const Integer shifted = m + Integer::power_two(b - 1);
      const Integer q = floor_shift_right(shifted, b);
      return shifted.low_bits_zero(b) && q.bit(0) ? q - Integer(1) : q;
    }

    // The magnitudes whose significand R(m/2^b) is z
    RoundingCell index_cell(const Integer& z, unsigned int b,
                            MagnitudeRounding rounding) {
      const Integer center = z.shift_left(b);
      const Integer g = Integer::power_two(b);
      switch (rounding) {
      case MagnitudeRounding::Down:
        return {center, center + g - Integer(1)};
      case MagnitudeRounding::Up:
        return {center - g + Integer(1), center};
      default: {
        const Integer odd(z.bit(0) ? 1 : 0);
        const Integer half = Integer::power_two(b - 1);
        return {center - half + odd, center + half - odd};
      }
      }
    }

    /// One binade of the free operand against one binade of the result
    struct SumBlock {
      /// Significands of the free operand, and its known bits
      std::uint64_t from, to, x_mask, x_value;
      /// Known fraction bits of the result significand
      std::uint64_t z_mask, z_value;
      /// Spacings 2^a of the free operand and 2^b of the result
      unsigned int a, b;
      MagnitudeRounding rounding;
    };

    // Improve x by the extreme candidate of least_with_masks on block with
    // the given sum masks: the least for Up, the greatest for Down.
    void mask_candidate(const SumBlock& block, Direction direction,
                        std::uint64_t addend, std::uint64_t sum_mask,
                        std::uint64_t sum_value, bool& found,
                        std::uint64_t& x) {
      std::uint64_t candidate;
      const bool up = direction == Direction::Up;
      if (!(up ? least_with_masks(block.from, block.to, block.x_mask,
                                  block.x_value, addend, sum_mask, sum_value,
                                  candidate)
               : greatest_with_masks(block.from, block.to, block.x_mask,
                                     block.x_value, addend, sum_mask,
                                     sum_value, candidate)))
        return;
      if (!found || (up ? candidate < x : candidate > x))
        x = candidate;
      found = true;
    }

    // The extreme X of block whose magnitude X*2^a + w rounds to a
    // significand that meets the known bits of the result, X in [from, to]
    // read modulo 2^64. See SumLattice.
    bool block_extreme(const SumBlock& block, const Integer& w,
                       Direction direction, std::uint64_t& x) {
      using Word::Arithmetic::Global::floor_shift_right;
      const unsigned int a = block.a, b = block.b;
      Integer shifted = w;
      if (block.rounding == MagnitudeRounding::Up)
        shifted = shifted + Integer::power_two(b) - Integer(1);
      else if (block.rounding == MagnitudeRounding::Nearest)
        shifted = shifted + Integer::power_two(b - 1);
      const bool nearest = block.rounding == MagnitudeRounding::Nearest;
      bool found = false;
      if (a >= b) {
        // Z = X*2^k + c, so its bits from k on are those of X + (c >> k).
        const unsigned int k = a - b;
        Integer c = floor_shift_right(shifted, b);
        if (nearest && shifted.low_bits_zero(b)) {
          // Every X ties, and an odd floor rounds down to even.
          if (k == 0) {
            if ((block.z_mask & block.z_value & 1) == 0)
              mask_candidate(block, direction, low_word(c),
                             block.z_mask & ~std::uint64_t(1),
                             block.z_value & ~std::uint64_t(1), found, x);
            return found;
          }
          if (c.bit(0))
            c = c - Integer(1);
        }
        const std::uint64_t low = (std::uint64_t(1) << k) - 1;
        if ((low_word(c) & low & block.z_mask) != (block.z_value & low))
          return false;
        mask_candidate(block, direction, low_word(floor_shift_right(c, k)),
                       block.z_mask >> k, block.z_value >> k, found, x);
        return found;
      }
      // Z = (X + c) >> k, so its bits are those of X + c from k on.
      const unsigned int k = b - a;
      const Integer c = floor_shift_right(shifted, a);
      const std::uint64_t mask = block.z_mask << k, value = block.z_value << k;
      if (!nearest || !shifted.low_bits_zero(a)) {
        mask_candidate(block, direction, low_word(c), mask, value, found, x);
        return found;
      }
      // X ties iff X + c is a multiple of 2^k, and then Z is the floor or
      // the floor minus one, whichever is even.
      const std::uint64_t tie = (std::uint64_t(1) << k) - 1;
      if ((block.z_mask & block.z_value & 1) == 0) {
        const std::uint64_t even = mask | tie | (std::uint64_t(1) << k);
        const std::uint64_t even_value = value & ~(std::uint64_t(1) << k);
        mask_candidate(block, direction, low_word(c), even, even_value, found,
                       x);
        mask_candidate(block, direction, low_word(c - Integer(1)),
                       even | tie, (even_value | tie), found, x);
      }
      for (unsigned int j = 0; j < k; ++j)
        mask_candidate(block, direction, low_word(c),
                       mask | ((std::uint64_t(2) << j) - 1),
                       value | (std::uint64_t(1) << j), found, x);
      return found;
    }

    // The extreme significand X in [from, to] of a binade of the free
    // operand, with spacing 2^a and known fraction bits x_mask and x_value,
    // whose sum rounds to a code of binade e of the result part. The sum
    // has magnitude X*2^a + y if increasing, else y - X*2^a. Return false
    // when there is none.
    bool sum_binade_candidate(const SumLattice& ls, std::uint64_t from,
                              std::uint64_t to, std::uint64_t x_mask,
                              std::uint64_t x_value, unsigned int a,
                              bool increasing, const Integer& y,
                              MagnitudeRounding rounding, unsigned int e,
                              Direction direction, std::uint64_t& x) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      const WideFormat& f = ls.f;
      const WordValue sign = ls.z.negative ? f.sign : 0;
      const WordValue base = e * f.normal;
      const auto binade = wide_intersect(
          ls.result_view, ls.z.domain, sign | std::max(WordValue(1), base),
          sign | std::min(f.maximum - 1, base + f.normal - 1));
      if (!binade.is_consistent)
        return false;
      const auto codes = domain_codes(ls.result_view, binade.domain);
      // The magnitudes that round to the codes of the binade
      const RoundingCell first = rounding_cell(f, ls.p, codes.first, 0, CellEnds::Open);
      const RoundingCell last = rounding_cell(f, ls.p, codes.last, 0, CellEnds::Open);
      const Integer low = ls.z.negative ? -first.high : first.low;
      const Integer high = ls.z.negative ? -last.low : last.high;
      // The significands whose magnitudes lie in [low, high]
      Integer x_low = increasing ? ceil_shift_right(low - y, a)
                                 : ceil_shift_right(y - high, a);
      Integer x_high = increasing ? floor_shift_right(high - y, a)
                                  : floor_shift_right(y - low, a);
      x_low = std::max(x_low, Integer(from));
      x_high = std::min(x_high, Integer(to));
      if (x_low > x_high)
        return false;
      const unsigned int b = e == 0 ? 1 : e;
      const std::uint64_t z_mask =
          ~(binade.domain.cube.lo ^ binade.domain.cube.hi) & (f.normal - 1);
      const std::uint64_t z_value = binade.domain.cube.lo & z_mask;
      // The significands of z that the magnitudes round to
      const Integer m_low = increasing ? x_low.shift_left(a) + y
                                       : y - x_high.shift_left(a);
      const Integer m_high = increasing ? x_high.shift_left(a) + y
                                        : y - x_low.shift_left(a);
      const std::uint64_t z_low = magnitude_index(m_low, b, rounding).word();
      const std::uint64_t z_high = magnitude_index(m_high, b, rounding).word();
      if (z_high - z_low < 4) {
        bool found = false;
        for (std::uint64_t z = z_low; z <= z_high; ++z) {
          if ((z & z_mask) != z_value)
            continue;
          const RoundingCell cell = index_cell(Integer(z), b, rounding);
          Integer cell_low = increasing ? ceil_shift_right(cell.low - y, a)
                                        : ceil_shift_right(y - cell.high, a);
          Integer cell_high = increasing ? floor_shift_right(cell.high - y, a)
                                         : floor_shift_right(y - cell.low, a);
          cell_low = std::max(cell_low, x_low);
          cell_high = std::min(cell_high, x_high);
          if (cell_low > cell_high)
            continue;
          const SumBlock block{cell_low.word(), cell_high.word(), x_mask,
                               x_value, 0, 0, a, b, rounding};
          mask_candidate(block, direction, 0, 0, 0, found, x);
        }
        return found;
      }
      if (increasing) {
        const SumBlock block{x_low.word(), x_high.word(), x_mask, x_value,
                             z_mask, z_value, a, b, rounding};
        return block_extreme(block, y, direction, x);
      }
      // Reflect: X' = ~X has magnitude X'*2^a + y + 2^a, in reverse order.
      const SumBlock block{~x_high.word(), ~x_low.word(), x_mask,
                           ~x_value & x_mask, z_mask, z_value, a, b,
                           rounding};
      std::uint64_t reflected;
      if (!block_extreme(block, y + Integer::power_two(a),
                         direction == Direction::Up ? Direction::Down
                                                    : Direction::Up,
                         reflected))
        return false;
      x = ~reflected;
      return true;
    }

    // Whether code of the free operand, admitted, supports the result part
    bool sum_support(const SumLattice& ls, const Word::WordDomain& admitted,
                     WordValue code) {
      if (!wide_intersect(ls.free_view, admitted, code, code).is_consistent)
        return false;
      const WordValue a = ls.free == Operand::First ? code : ls.fixed_code;
      const WordValue b = ls.free == Operand::First ? ls.fixed_code : code;
      const WideResult r = evaluate_wide(ls.f, ls.p, ls.op, a, b);
      return wide_intersect(ls.result_view, ls.z.domain, r.code, r.code)
          .is_consistent;
    }

    // The least (Up) or greatest (Down) code of the free operand within
    // admitted that supports the result part. The search visits the
    // binades of the free operand in order and, in each, the result binades
    // that its sums reach (SumLattice). A candidate that fails its exact
    // test moves the search past it; after lattice_search_limit of them a
    // bounded search returns the code past the last one as a bound and
    // counts the cut in its budget. Return false when no code supports the
    // result part.
    bool sum_endpoint(const SumLattice& ls, const Word::WordDomain& admitted,
                      Direction direction, WordValue& endpoint) {
      const WideFormat& f = ls.f;
      const WordValue mask = f.sign - 1;
      const bool up = direction == Direction::Up;
      const auto codes = domain_codes(ls.free_view, admitted);
      const WordValue lowest = codes.first & mask, highest = codes.last & mask;
      const auto result = domain_codes(ls.result_view, ls.z.domain);
      const unsigned int result_low =
          static_cast<unsigned int>((result.first & mask) >> f.fraction_bits);
      const unsigned int result_high =
          static_cast<unsigned int>((result.last & mask) >> f.fraction_bits);
      // Read with the sign of z, the sum has magnitude e*X*u + y.
      const bool subtrahend = ls.op == Arithmetic::Subtract && ls.free == Operand::Second;
      const bool increasing =
          ((ls.free_sign != 0) != subtrahend) == ls.z.negative;
      const Integer term = ls.op == Arithmetic::Subtract && ls.free == Operand::First
                               ? -ls.fixed
                               : ls.fixed;
      const Integer y = ls.z.negative ? -term : term;
      const MagnitudeRounding rounding =
          magnitude_rounding(ls.p.rounding, ls.z.negative);
      WordValue x = up ? lowest : highest;
      unsigned int failures = 0;
      while (true) {
        const auto next = wide_intersect(
            ls.free_view, admitted, ls.free_sign | (up ? x : lowest),
            ls.free_sign | (up ? highest : x));
        if (!next.is_consistent)
          return false;
        const auto remaining = domain_codes(ls.free_view, next.domain);
        x = (up ? remaining.first : remaining.last) & mask;
        const unsigned int e = static_cast<unsigned int>(x >> f.fraction_bits);
        const WordValue base = e * f.normal;
        const auto binade = wide_intersect(
            ls.free_view, next.domain,
            ls.free_sign | std::max(WordValue(1), base),
            ls.free_sign | (base + f.normal - 1));
        assert(binade.is_consistent);
        const auto range = domain_codes(ls.free_view, binade.domain);
        const WordValue start = range.first & mask, stop = range.last & mask;
        const WordValue offset = e == 0 ? 0 : f.normal;
        const std::uint64_t from = start - base + offset;
        const std::uint64_t to = stop - base + offset;
        const std::uint64_t x_mask =
            ~(binade.domain.cube.lo ^ binade.domain.cube.hi) & (f.normal - 1);
        const std::uint64_t x_value = binade.domain.cube.lo & x_mask;
        const unsigned int a = e == 0 ? 1 : e;
        // The result binades that the magnitudes of from..to reach
        Integer least = increasing ? Integer(from).shift_left(a) + y
                                   : y - Integer(to).shift_left(a);
        const Integer greatest = increasing ? Integer(to).shift_left(a) + y
                                            : y - Integer(from).shift_left(a);
        bool found = false;
        std::uint64_t s = 0;
        if (greatest > Integer()) {
          least = std::max(least, Integer());
          const unsigned int reach_low =
              std::max(result_low, static_cast<unsigned int>(
                                       floor_encoding(f, least) >>
                                       f.fraction_bits));
          const unsigned int reach_high = std::min(
              result_high,
              static_cast<unsigned int>(
                  std::min(f.maximum, floor_encoding(f, greatest) + 1) >>
                  f.fraction_bits));
          for (unsigned int k = reach_low; k <= reach_high; ++k) {
            std::uint64_t candidate;
            if (sum_binade_candidate(ls, from, to, x_mask, x_value, a,
                                     increasing, y, rounding, k, direction,
                                     candidate) &&
                (!found || (up ? candidate < s : candidate > s))) {
              s = candidate;
              found = true;
            }
          }
        }
        WordValue passed;
        if (found) {
          const WordValue candidate = static_cast<WordValue>(s - offset) + base;
          if (sum_support(ls, next.domain, ls.free_sign | candidate)) {
            endpoint = ls.free_sign | candidate;
            return true;
          }
          passed = candidate;
          ++failures;
        } else {
          passed = up ? stop : start;
        }
        if (up ? passed >= highest : passed <= lowest)
          return false;
        x = up ? passed + 1 : passed - 1;
        if (failures == lattice_search_limit &&
            ls.budget.limit == LatticeLimit::Bounded) {
          ++ls.budget.cuts;
          endpoint = ls.free_sign | x;
          return true;
        }
      }
    }

    // Narrow admitted, the encodings of the free operand of a sum, to the
    // hull of its codes that support the result part. Return false when
    // none does.
    bool narrow_sum_lattice(const SumLattice& ls, Word::WordDomain& admitted) {
      WordValue low, high;
      if (!sum_endpoint(ls, admitted, Direction::Up, low) ||
          !sum_endpoint(ls, admitted, Direction::Down, high))
        return false;
      const auto narrowed = wide_intersect(ls.free_view, admitted,
                                           std::min(low, high),
                                           std::max(low, high));
      if (!narrowed.is_consistent)
        return false;
      admitted = narrowed.domain;
      return true;
    }

    /** \brief Search for an operand of a sum whose other operand has
     * several values
     *
     * Read with the sign of the result part z, the exact sum of a code of
     * the free part x and a code of the other part y has magnitude m =
     * e_x*|x| + e_y*|y|, with e_x and e_y +1 or -1 (adds), and a code of z
     * in binade b, with spacing 2^b, is the result iff m lies within the
     * cells of the least and the greatest admitted code of that binade and
     * R(m/2^b) meets the known fraction bits of z (see SumLattice). For one
     * binade of x, one of y and one of z that is a condition on X*2^a and
     * Y*2^c, X and Y the significands, that sum_term_extreme solves
     * exactly: it gives the extreme X with some Y. The search visits the
     * binades of x in order and, in each, the result binades its sums can
     * reach and the binades of y that can reach them, and stops at the
     * first binade of x with a support. So it is exact, without candidate
     * tests.
     *
     * Two observations bound the work. For b >= 3 the condition on m only
     * changes next to multiples of 2^(b-2): the cell bounds and the steps
     * of R lie there, as the spacing below binade b is at least 2^(b-1).
     * So with T = min(a, b-2), all |y| with 0 < |y| < 2^T give the same
     * condition on X: e_x*X*2^a is a multiple of 2^T and adding e_y*|y|
     * stays strictly between it and the next multiple of 2^T in that
     * direction, and every multiple of 2^(b-2) is one of 2^T. The binades
     * of y below 2^T thus take one search, with |y| = 2^(T-1) in place of
     * all of them, if y has a code below 2^T. Symmetrically, when the
     * whole binade of x lies below 2^min(c, b-2), every X has the same
     * status against binade c of y, and one search with |x| = 2^(T-1)
     * decides it for the extreme X of the binade. What remains are binades
     * of x and y at most fraction_bits + 4 binades apart (more where the
     * result is much smaller, but then x and y lie in adjacent binades), so
     * each search spans at most 2*fraction_bits + 6 bit positions.
     */
    struct SumPair {
      const WideFormat& f;
      const Policy& p;
      WordView x_view;
      /// The sign bit of the free part, and whether its magnitude adds
      WordValue x_sign;
      bool x_adds;
      WordView y_view;
      const FinitePart& y;
      bool y_adds;
      WordView z_view;
      const FinitePart& z;
      MagnitudeRounding rounding;
    };

    // Whether operand i of op, with a part of the given sign, adds its
    // magnitude to the magnitude of the result part z
    bool adds_magnitude(Arithmetic op, int i, bool negative,
                        const FinitePart& z) {
      return (negative != (op == Arithmetic::Subtract && i == 1)) ==
             z.negative;
    }

    // Set first and last to the magnitudes of the codes in [low, high].
    // Return false when there are none.
    bool magnitude_codes(const WideFormat& f, const Integer& low,
                         const Integer& high, WordValue& first,
                         WordValue& last) {
      if (high < low || high < Integer())
        return false;
      first = floor_encoding(f, std::max(low, Integer()));
      if (f.magnitude(first) < low)
        ++first;
      last = floor_encoding(f, high);
      return first <= last;
    }

    // Set term to the codes of domain, a part with sign bit sign, whose
    // magnitudes are in [first, last] within binade e. Return false when
    // there are none.
    bool binade_term(const WideFormat& f, WordView view,
                     const Word::WordDomain& domain, WordValue sign,
                     unsigned int e, WordValue first, WordValue last,
                     bool negative, SumTerm& term) {
      const WordValue base = WordValue(e) * f.normal;
      first = std::max(first, std::max(WordValue(1), base));
      last = std::min(last, base + f.normal - 1);
      if (first > last)
        return false;
      const auto binade = wide_intersect(view, domain, sign | first,
                                         sign | last);
      if (!binade.is_consistent)
        return false;
      const auto codes = domain_codes(view, binade.domain);
      const WordValue offset = e == 0 ? 0 : f.normal;
      const std::uint64_t fraction =
          ~(binade.domain.cube.lo ^ binade.domain.cube.hi) & (f.normal - 1);
      term = {(codes.first & (f.sign - 1)) - base + offset,
              (codes.last & (f.sign - 1)) - base + offset,
              fraction | f.normal,
              (binade.domain.cube.lo & fraction) | offset,
              e == 0 ? 1U : e,
              f.fraction_bits + 1,
              negative};
      return true;
    }

    // Set target to binade e of the result part. Return false when the
    // part has no code there.
    bool result_target(const SumPair& sp, unsigned int e, SumTarget& target) {
      const WideFormat& f = sp.f;
      const WordValue sign = sp.z.negative ? f.sign : 0;
      const WordValue base = WordValue(e) * f.normal;
      const auto binade = wide_intersect(
          sp.z_view, sp.z.domain, sign | std::max(WordValue(1), base),
          sign | std::min(f.maximum - 1, base + f.normal - 1));
      if (!binade.is_consistent)
        return false;
      const auto codes = domain_codes(sp.z_view, binade.domain);
      const RoundingCell first = rounding_cell(f, sp.p, codes.first, 0, CellEnds::Open);
      const RoundingCell last = rounding_cell(f, sp.p, codes.last, 0, CellEnds::Open);
      const std::uint64_t fraction =
          ~(binade.domain.cube.lo ^ binade.domain.cube.hi) & (f.normal - 1);
      target = {sp.z.negative ? -first.high : first.low,
                sp.z.negative ? -last.low : last.high,
                e == 0 ? 1U : e,
                fraction,
                binade.domain.cube.lo & fraction,
                sp.rounding};
      return true;
    }

    // The magnitudes of x for which e_x*|x| + e_y*|y|, |y| in [y_low,
    // y_high], can lie in [low, high]
    void needed_magnitudes(bool x_adds, bool y_adds, const Integer& low,
                           const Integer& high, const Integer& y_low,
                           const Integer& y_high, Integer& from,
                           Integer& to) {
      if (x_adds && y_adds) {
        from = low - y_high;
        to = high - y_low;
      } else if (x_adds) {
        from = low + y_low;
        to = high + y_high;
      } else {
        from = y_low - high;
        to = y_high - low;
      }
    }

    // Narrow the significands [from, to] of term, with spacing 2^a, to
    // those whose magnitude lies in [low, high]. Return false when none do.
    bool clip_term(SumTerm& term, const Integer& low, const Integer& high) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      const Integer from = std::max(ceil_shift_right(low, term.a), Integer(term.from));
      const Integer to = std::min(floor_shift_right(high, term.a), Integer(term.to));
      if (from > to)
        return false;
      term.from = from.word();
      term.to = to.word();
      return true;
    }

    // Keep in term only the significands beyond best in direction.
    bool beyond_best(SumTerm& term, Direction direction, bool found,
                     std::uint64_t best) {
      if (!found)
        return true;
      if (direction == Direction::Up) {
        if (best == 0)
          return false;
        term.to = std::min(term.to, best - 1);
      } else {
        term.from = std::max(term.from, best + 1);
      }
      return term.from <= term.to;
    }

    // Improve best by the extreme significand of x, binade e of the free
    // part, supported in target, the result binade, by a code of y whose
    // magnitude lies in [y_low, y_high]. Return true when best is the
    // extreme significand of the binade, limit.
    bool target_extreme(const SumPair& sp, const SumTerm& binade,
                        unsigned int e, std::uint64_t limit,
                        const SumTarget& target, const Integer& y_low,
                        const Integer& y_high, Direction direction,
                        bool& found, std::uint64_t& best) {
      const WideFormat& f = sp.f;
      const unsigned int fb = f.fraction_bits;
      Integer from, to;
      needed_magnitudes(sp.x_adds, sp.y_adds, target.low, target.high, y_low,
                        y_high, from, to);
      SumTerm x = binade;
      if (!clip_term(x, from, to) || !beyond_best(x, direction, found, best))
        return false;
      // The magnitudes of y that these significands need
      const Integer x_low = Integer(x.from).shift_left(x.a);
      const Integer x_high = Integer(x.to).shift_left(x.a);
      Integer need_low, need_high;
      if (sp.x_adds && sp.y_adds) {
        need_low = target.low - x_high;
        need_high = target.high - x_low;
      } else if (sp.x_adds) {
        need_low = x_low - target.high;
        need_high = x_high - target.low;
      } else {
        need_low = target.low + x_low;
        need_high = target.high + x_high;
      }
      need_low = std::max(need_low, y_low);
      need_high = std::min(need_high, y_high);
      WordValue y_first, y_last;
      if (!magnitude_codes(f, need_low, need_high, y_first, y_last))
        return false;
      const WordValue y_sign = sp.y.negative ? f.sign : 0;
      const unsigned int t = target.b >= 3 ? std::min(x.a, target.b - 2) : 0;
      std::uint64_t candidate;
      if (t >= 1 && y_low < Integer::power_two(t)) {
        // The codes of y below 2^t, all alike: |y| = 2^(t-1)
        const Integer v = Integer::power_two(t - 1);
        if (sum_term_extreme(x, nullptr, sp.y_adds ? v : -v, t - 1, target,
                             direction, candidate)) {
          found = true;
          best = candidate;
          if (best == limit)
            return true;
          if (!beyond_best(x, direction, found, best))
            return false;
        }
      }
      for (unsigned int c = static_cast<unsigned int>(y_first >> fb);
           c <= static_cast<unsigned int>(y_last >> fb); ++c) {
        if (t >= 1 && c + fb + 1 <= t)
          continue;
        SumTerm y;
        if (!binade_term(f, sp.y_view, sp.y.domain, y_sign, c, y_first, y_last,
                         !sp.y_adds, y))
          continue;
        SumTerm xc = x;
        needed_magnitudes(sp.x_adds, sp.y_adds, target.low, target.high,
                          Integer(y.from).shift_left(y.a),
                          Integer(y.to).shift_left(y.a), from, to);
        if (!clip_term(xc, from, to))
          continue;
        const unsigned int tx =
            target.b >= 3 ? std::min(y.a, target.b - 2) : 0;
        if (tx >= 1 && e + fb + 1 <= tx) {
          // Every code of the binade of x alike: |x| = 2^(tx-1)
          const Integer v = Integer::power_two(tx - 1);
          if (sum_term_extreme(y, nullptr, sp.x_adds ? v : -v, tx - 1,
                               target, Direction::Up, candidate)) {
            found = true;
            best = limit;
            return true;
          }
          continue;
        }
        if (sum_term_extreme(xc, &y, Integer(), 0, target, direction,
                             candidate)) {
          found = true;
          best = candidate;
          if (best == limit)
            return true;
          if (!beyond_best(x, direction, found, best))
            return false;
        }
      }
      return false;
    }

    // The least (Up) or greatest (Down) magnitude of the free part within
    // admitted that supports the result part, see SumPair. Return false
    // when there is none.
    bool sum_pair_endpoint(const SumPair& sp, const Word::WordDomain& admitted,
                           Direction direction, WordValue& endpoint) {
      const WideFormat& f = sp.f;
      const unsigned int fb = f.fraction_bits;
      const WordValue mask = f.sign - 1;
      const bool up = direction == Direction::Up;
      const auto x_codes = domain_codes(sp.x_view, admitted);
      const unsigned int x_low =
          static_cast<unsigned int>((x_codes.first & mask) >> fb);
      const unsigned int x_high =
          static_cast<unsigned int>((x_codes.last & mask) >> fb);
      const auto y_codes = domain_codes(sp.y_view, sp.y.domain);
      const Integer y_low = f.magnitude(y_codes.first & mask);
      const Integer y_high = f.magnitude(y_codes.last & mask);
      const auto z_codes = domain_codes(sp.z_view, sp.z.domain);
      unsigned int z_low =
          static_cast<unsigned int>((z_codes.first & mask) >> fb);
      const unsigned int z_high =
          static_cast<unsigned int>((z_codes.last & mask) >> fb);
      if (sp.p.output_subnormals == Policy::SignedZero)
        z_low = std::max(z_low, 1U);
      for (unsigned int i = 0; i <= x_high - x_low; ++i) {
        const unsigned int e = up ? x_low + i : x_high - i;
        SumTerm x;
        if (!binade_term(f, sp.x_view, admitted, sp.x_sign, e, 0, f.maximum,
                         !sp.x_adds, x))
          continue;
        std::uint64_t limit;
        if (!(up ? least_with_masks(x.from, x.to, x.mask, x.value, 0, 0, 0,
                                    limit)
                 : greatest_with_masks(x.from, x.to, x.mask, x.value, 0, 0, 0,
                                       limit)))
          continue;
        // The result binades that the sums of the binade reach
        Integer m_low, m_high;
        const Integer x_from = Integer(x.from).shift_left(x.a);
        const Integer x_to = Integer(x.to).shift_left(x.a);
        if (sp.x_adds && sp.y_adds) {
          m_low = x_from + y_low;
          m_high = x_to + y_high;
        } else if (sp.x_adds) {
          m_low = x_from - y_high;
          m_high = x_to - y_low;
        } else {
          m_low = y_low - x_to;
          m_high = y_high - x_from;
        }
        if (m_high < Integer(1))
          continue;
        m_low = std::max(m_low, Integer(1));
        const unsigned int reach_low = std::max(
            z_low, static_cast<unsigned int>(floor_encoding(f, m_low) >> fb));
        const unsigned int reach_high = std::min(
            z_high,
            static_cast<unsigned int>(
                std::min(f.maximum, floor_encoding(f, m_high) + 1) >> fb));
        bool found = false;
        std::uint64_t best = 0;
        for (unsigned int b = reach_low; b <= reach_high; ++b) {
          SumTarget target;
          if (result_target(sp, b, target) &&
              target_extreme(sp, x, e, limit, target, y_low, y_high, direction,
                             found, best))
            break;
        }
        if (found) {
          const WordValue base = WordValue(e) * f.normal;
          const WordValue offset = e == 0 ? 0 : f.normal;
          endpoint = sp.x_sign | (static_cast<WordValue>(best) - offset + base);
          return true;
        }
      }
      return false;
    }

    // Narrow admitted, the encodings of the free part of a sum, to the hull
    // of its codes that support the result part. Return false when none
    // does.
    bool narrow_sum_pair(const SumPair& sp, Word::WordDomain& admitted) {
      WordValue low, high;
      if (!sum_pair_endpoint(sp, admitted, Direction::Up, low) ||
          !sum_pair_endpoint(sp, admitted, Direction::Down, high))
        return false;
      const auto narrowed = wide_intersect(sp.x_view, admitted,
                                           std::min(low, high),
                                           std::max(low, high));
      if (!narrowed.is_consistent)
        return false;
      admitted = narrowed.domain;
      return true;
    }

    // Set result to the codes of domain, a part with sign bit sign, whose
    // magnitude is that of a code of the result part z, and only normal
    // magnitudes when the policy flushes results: the codes x for which x +
    // 0 = x is a code of z, read with its sign. Return false when there are
    // none.
    bool same_magnitudes(const WideFormat& f, const Policy& p, WordView view,
                         const Word::WordDomain& domain, WordValue sign,
                         WordView z_view, const FinitePart& z,
                         Word::WordDomain& result) {
      const WordValue mask = f.sign - 1;
      const auto codes = domain_codes(z_view, z.domain);
      WordValue first = codes.first & mask;
      const WordValue last = codes.last & mask;
      if (p.output_subnormals == Policy::SignedZero)
        first = std::max(first, f.normal);
      if (first > last)
        return false;
      Word::WordDomain d = domain;
      d.cube.lo |= z.domain.cube.lo & mask;
      d.cube.hi &= z.domain.cube.hi | ~mask;
      if (d.cube.lo & ~d.cube.hi)
        return false;
      const auto same = wide_intersect(view, d, sign | first, sign | last);
      if (!same.is_consistent)
        return false;
      result = same.domain;
      return true;
    }

    // The greatest operand magnitude of a sum whose result is a nonzero
    // code of magnitude at most largest, below the greatest finite one: by
    // the cancellation bound (see apply_cancellation_bound) one operand has
    // magnitude at most C(z) = (2*normal-1)*2^ctz(|z|) and the other at most
    // |z| + C(z). Within the binades up to that of largest, ctz(|z|) is at
    // most max(1, binade) + fraction_bits in the exact integer scale.
    Integer cancellation_reach(const WideFormat& f, WordValue largest) {
      assert(largest != 0 && largest < f.maximum);
      const unsigned int binade =
          static_cast<unsigned int>(largest >> f.fraction_bits);
      return f.magnitude(largest) +
             Integer(2 * f.normal - 1)
                 .shift_left(std::max(1U, binade) + f.fraction_bits);
    }

    // Whether the code x, of operand i of op, has a code of the other
    // operand's part y that gives a code of the result part z, whose
    // rounding cells span [cell_low, cell_high]. Only the least and the
    // greatest code of y whose exact sum lies in that span are evaluated,
    // so false means only that neither of them supports x.
    bool supports_code(const WideFormat& f, const Policy& p, Arithmetic op,
                       const WordView views[3], int i, WordValue x,
                       const FinitePart& y, const FinitePart& z,
                       const Integer& cell_low, const Integer& cell_high) {
      const Integer value = f.value(x);
      Integer low, high;
      if (op == Arithmetic::Add) {
        low = cell_low - value;
        high = cell_high - value;
      } else if (i == 0) {
        low = value - cell_high;
        high = value - cell_low;
      } else {
        low = cell_low + value;
        high = cell_high + value;
      }
      Word::WordDomain ys;
      if (!real_domain(f, views[1 - i], y, low, high, ys))
        return false;
      const auto codes = domain_codes(views[1 - i], ys);
      for (WordValue other : {codes.first, codes.last}) {
        const WideResult r = i == 0 ? evaluate_wide(f, p, op, x, other)
                                    : evaluate_wide(f, p, op, other, x);
        if (wide_intersect(views[2], z.domain, r.code, r.code).is_consistent)
          return true;
      }
      return false;
    }

    // Whether both the least and the greatest code of admitted, the codes
    // of part x, operand i of op, support the result part z (supports_code).
    // Then admitted already is the hull of the supports, and a SumPair search
    // would return it unchanged.
    bool pair_endpoints_supported(const WideFormat& f, const Policy& p,
                                  Arithmetic op, const WordView views[3],
                                  int i, const Word::WordDomain& admitted,
                                  const FinitePart& y, const FinitePart& z) {
      const auto z_codes = domain_codes(views[2], z.domain);
      const Integer cell_low =
          rounding_cell(f, p, z.negative ? z_codes.last : z_codes.first, 0, CellEnds::Open).low;
      const Integer cell_high =
          rounding_cell(f, p, z.negative ? z_codes.first : z_codes.last, 0, CellEnds::Open).high;
      const auto x_codes = domain_codes(views[i], admitted);
      return supports_code(f, p, op, views, i, x_codes.first, y, z, cell_low,
                           cell_high) &&
             supports_code(f, p, op, views, i, x_codes.last, y, z, cell_low,
                           cell_high);
    }

    // Whether projection already holds every encoding of d, so including
    // any subset of d leaves it unchanged
    bool covers_domain(const WideProjection& projection,
                       const Word::WordDomain& d) {
      return projection.present && (projection.cube.lo & ~d.cube.lo) == 0 &&
             (d.cube.hi & ~projection.cube.hi) == 0 &&
             projection.interval.minimum <= d.interval.minimum &&
             d.interval.maximum <= projection.interval.maximum;
    }

    // Include in projection the encodings of part x, operand i of a sum,
    // whose values lie in [low, high] and that support the result part z
    // with some code of the other operand's part y. The caller excludes a
    // zero and the greatest finite magnitude from z. A sum with a zero
    // operand, or one read as zero, is exactly the other operand: its sign
    // must be that of z, which also decides the sign of a nonzero sum, and
    // its magnitude that of a code of z, a normal one when the policy
    // flushes results (same_magnitudes). Two zeros sum to zero, which z
    // excludes, and so does a subnormal result when the policy flushes
    // results. Otherwise the lattice searches give the hull of the
    // supports: SumLattice where y has one value, SumPair where it has
    // more. Before searching, x is clipped to the cancellation reach of z
    // (no code beyond it supports z), nothing is done when projection
    // already covers the clipped part, and SumPair is skipped when both
    // ends of the clipped part support z (pair_endpoints_supported).
    void include_sum_operand(const WideFormat& f, const Policy& p,
                             Arithmetic op, const WordView views[3], int i,
                             const FinitePart& x, const FinitePart& y,
                             const FinitePart& z, const Integer& low,
                             const Integer& high, LatticeBudget& budget,
                             WideProjection& projection) {
      const auto z_codes = domain_codes(views[2], z.domain);
      const WordValue least = z_codes.first & (f.sign - 1);
      Word::WordDomain admitted;
      if (least == 0) {
        if (real_domain(f, views[i], x, low, high, admitted))
          projection.include(views[i], admitted);
        return;
      }
      if (p.output_subnormals == Policy::SignedZero && least < f.normal)
        return;
      const Integer reach = cancellation_reach(f, z_codes.last & (f.sign - 1));
      if (!real_domain(f, views[i], x, std::max(low, -reach),
                       std::min(high, reach), admitted) ||
          covers_domain(projection, admitted))
        return;
      const bool x_zero = x.flushed || (x.low.zero() && x.high.zero());
      const bool y_zero = y.flushed || (y.low.zero() && y.high.zero());
      const bool x_adds = adds_magnitude(op, i, x.negative, z);
      const bool y_adds = adds_magnitude(op, 1 - i, y.negative, z);
      const WordValue x_sign = x.negative ? f.sign : 0;
      const WordValue y_sign = y.negative ? f.sign : 0;
      Word::WordDomain same;
      if (x_zero) {
        if (!y_zero && y_adds &&
            same_magnitudes(f, p, views[1 - i], y.domain, y_sign, views[2], z,
                            same))
          projection.include(views[i], admitted);
        return;
      }
      if (y_zero) {
        if (x_adds && same_magnitudes(f, p, views[i], admitted, x_sign,
                                      views[2], z, same))
          projection.include(views[i], same);
        return;
      }
      if (!x_adds && !y_adds)
        return;
      if (y.low == y.high) {
        const SumLattice search{f,
                                p,
                                op,
                                i == 0 ? Operand::First : Operand::Second,
                                views[i],
                                x_sign,
                                domain_codes(views[1 - i], y.domain).first,
                                y.low,
                                views[2],
                                z,
                                budget};
        if (!narrow_sum_lattice(search, admitted))
          return;
      } else if (!pair_endpoints_supported(f, p, op, views, i, admitted, y,
                                           z)) {
        const SumPair search{f,
                             p,
                             views[i],
                             x_sign,
                             x_adds,
                             views[1 - i],
                             y,
                             y_adds,
                             views[2],
                             z,
                             magnitude_rounding(p.rounding, z.negative)};
        if (!narrow_sum_pair(search, admitted))
          return;
      }
      projection.include(views[i], admitted);
    }

    /** \brief Flag values of the status during one projection pass
     *
     * \a admitted holds the flag values that the status admits, one bit
     * per value, and \a produced collects those that the projected pairs
     * of operand classes can give with an admitted result. Every projection
     * adds to \a produced the flags of each pair it keeps, so the status
     * can be narrowed to \a produced.
     */
    struct StatusFlags {
      unsigned int admitted;
      unsigned int produced = 0;
      bool admits(int flags) const { return (admitted & flag_bit(flags)) != 0; }
      void produce(int flags) { produced |= flag_bit(flags); }
      /// Whether some flag value of a rounded finite result is admitted
      bool admits_finite() const {
        return admits(0) || admits(inexact) || admits(underflow | inexact) ||
               admits(overflow | inexact);
      }
    };

    enum class OperandClass { Zero, Finite, Infinity, QuietNaN, SignalingNaN };
    struct OperandPart {
      FinitePart finite;
      OperandClass kind;
      bool nan() const {
        return kind == OperandClass::QuietNaN ||
               kind == OperandClass::SignalingNaN;
      }
      /// Whether the part holds finite values, zeros included
      bool number() const {
        return kind == OperandClass::Zero || kind == OperandClass::Finite;
      }
    };

    // Six finite parts and three special parts per sign.
    using OperandParts = Word::Float::Parts<OperandPart, 12>;
    OperandParts operand_parts(const WideFormat& f, const Policy& p,
                               WordView view, const Word::WordDomain& domain) {
      OperandParts parts;
      for (const auto& part : finite_parts(f, p, view, domain, Role::Input))
        parts.push_back({part, part.low.zero() && part.high.zero()
                                   ? OperandClass::Zero
                                   : OperandClass::Finite});
      for (WordValue sign : {WordValue(0), f.sign}) {
        const struct {
          WordValue first, last;
          OperandClass kind;
        } ranges[] = {
            {f.infinity, f.infinity, OperandClass::Infinity},
            {f.infinity + 1, f.infinity + f.quiet - 1,
             OperandClass::SignalingNaN},
            {f.infinity + f.quiet, f.sign - 1, OperandClass::QuietNaN}};
        for (const auto& range : ranges) {
          const auto admitted = wide_intersect(view, domain, sign | range.first,
                                               sign | range.last);
          if (admitted.is_consistent)
            parts.push_back(
                {{admitted.domain, Integer(), Integer(), sign != 0, false},
                 range.kind});
        }
      }
      return parts;
    }

    /// Whether the outcome of \a op on parts \a a and \a b has a fixed class
    bool special_pair(Arithmetic op, const OperandPart& a,
                      const OperandPart& b) {
      if (op == Arithmetic::Add || op == Arithmetic::Subtract)
        return !a.number() || !b.number();
      return a.kind != OperandClass::Finite || b.kind != OperandClass::Finite;
    }

    /// Result encodings from \a first to \a last, and the flags, of a pair
    /// of classes whose outcome has a fixed class
    struct SpecialOutcome {
      WordValue first, last;
      int flags;
    };

    SpecialOutcome canonical_invalid(const WideFormat& f) {
      return {f.infinity | f.quiet, f.infinity | f.quiet, invalid};
    }

    // A NaN operand gives the selected NaN, quiet and with its sign, or the
    // canonical NaN, with I for a signaling operand. The selection order is
    // known from the classes; payloads are kept conservatively, as every
    // quiet NaN of the selected sign.
    SpecialOutcome nan_outcome(const WideFormat& f, const Policy& p,
                               const OperandPart& a, const OperandPart& b) {
      const int flags = a.kind == OperandClass::SignalingNaN ||
                                b.kind == OperandClass::SignalingNaN
                            ? invalid
                            : 0;
      if (p.nan == Policy::Canonical)
        return {f.infinity | f.quiet, f.infinity | f.quiet, flags};
      const OperandPart& selected =
          p.nan == Policy::SignalFirst && flags
              ? (a.kind == OperandClass::SignalingNaN ? a : b)
              : (a.nan() ? a : b);
      const WordValue sign = selected.finite.negative ? f.sign : 0;
      return {sign | f.infinity | f.quiet, sign | (f.sign - 1), flags};
    }

    // Read with the sign of b negated for subtraction, infinities of
    // opposite signs give the canonical NaN with I, and otherwise an
    // infinite operand gives that infinity without flags, whatever the
    // other number. Input flushing changes no infinity.
    SpecialOutcome sum_outcome(const WideFormat& f, Arithmetic op,
                               const OperandPart& a, const OperandPart& b) {
      const bool a_infinite = a.kind == OperandClass::Infinity;
      const bool b_infinite = b.kind == OperandClass::Infinity;
      const bool negative_a = a.finite.negative;
      const bool negative_b =
          b.finite.negative != (op == Arithmetic::Subtract);
      if (a_infinite && b_infinite && negative_a != negative_b)
        return canonical_invalid(f);
      const WordValue code =
          f.infinity | ((a_infinite ? negative_a : negative_b) ? f.sign : 0);
      return {code, code, 0};
    }

    // Zero times infinity is invalid; otherwise an infinity gives the signed
    // infinity and a zero the signed zero, without flags.
    SpecialOutcome product_outcome(const WideFormat& f, const Policy& p,
                                   const OperandPart& a,
                                   const OperandPart& b) {
      if ((a.kind == OperandClass::Zero && b.kind == OperandClass::Infinity) ||
          (b.kind == OperandClass::Zero && a.kind == OperandClass::Infinity))
        return canonical_invalid(f);
      const bool infinity = a.kind == OperandClass::Infinity ||
                            b.kind == OperandClass::Infinity;
      const bool negative = a.finite.negative != b.finite.negative;
      const WordValue code =
          (infinity ? f.infinity : 0) |
          (negative && (infinity || p.zeros == Policy::Signed) ? f.sign : 0);
      return {code, code, 0};
    }

    // Zero by zero and infinity by infinity are invalid. An infinite
    // dividend or a zero divisor gives the signed infinity, with D for a
    // finite nonzero dividend; otherwise the result is the signed zero.
    SpecialOutcome quotient_outcome(const WideFormat& f, const Policy& p,
                                    const OperandPart& a,
                                    const OperandPart& b) {
      if ((a.kind == OperandClass::Zero && b.kind == OperandClass::Zero) ||
          (a.kind == OperandClass::Infinity &&
           b.kind == OperandClass::Infinity))
        return canonical_invalid(f);
      const bool infinity =
          a.kind == OperandClass::Infinity || b.kind == OperandClass::Zero;
      const int flags =
          b.kind == OperandClass::Zero && a.kind == OperandClass::Finite
              ? divide_by_zero
              : 0;
      const bool negative = a.finite.negative != b.finite.negative;
      const WordValue code =
          (infinity ? f.infinity : 0) |
          (negative && (infinity || p.zeros == Policy::Signed) ? f.sign : 0);
      return {code, code, flags};
    }

    /// Outcome of \a op on parts \a a and \a b, a special_pair()
    SpecialOutcome special_outcome(const WideFormat& f, const Policy& p,
                                   Arithmetic op, const OperandPart& a,
                                   const OperandPart& b) {
      if (a.nan() || b.nan())
        return nan_outcome(f, p, a, b);
      switch (op) {
      case Arithmetic::Add:
      case Arithmetic::Subtract:
        return sum_outcome(f, op, a, b);
      case Arithmetic::Multiply:
        return product_outcome(f, p, a, b);
      case Arithmetic::Divide:
        return quotient_outcome(f, p, a, b);
      }
      GECODE_NEVER;
      return canonical_invalid(f);
    }

    // A pair of classes with a fixed outcome supports its whole parts when
    // the result and the status admit that outcome. NaN payloads are
    // deliberately conservative, but selection order, signaling status, sign
    // and quietness are known from each pair of classes.
    void project_special(const WideFormat& f, const Policy& p, Arithmetic op,
                         const OperandPart& a, const OperandPart& b,
                         const WordView views[3],
                         const Word::WordDomain domains[3],
                         StatusFlags& status, WideProjection projections[3]) {
      const SpecialOutcome outcome = special_outcome(f, p, op, a, b);
      if (!status.admits(outcome.flags))
        return;
      WideProjection result;
      result.include(views[2], domains[2], outcome.first, outcome.last);
      if (!result.present)
        return;
      status.produce(outcome.flags);
      projections[0].include(views[0], a.finite.domain);
      projections[1].include(views[1], b.finite.domain);
      projections[2].unite(result);
    }

    /// Whether \a domain of \a view admits a code of a sign from magnitude
    /// \a first to \a last
    bool admits_magnitudes(const WideFormat& f, WordView view,
                           const Word::WordDomain& domain, WordValue first,
                           WordValue last) {
      return wide_intersect(view, domain, first, last).is_consistent ||
             wide_intersect(view, domain, f.sign | first, f.sign | last)
                 .is_consistent;
    }

    /** \brief Flag values of a rounded finite result admitted by \a domain
     *
     * Rounding a finite nonzero real gives 0, X, O|X or U|X. A result that
     * neither overflows nor underflows is a finite code. Overflow gives the
     * code of overflow_code, and underflow a magnitude at most the least
     * normal one, a zero after flushing.
     */
    unsigned int rounded_flags(const WideFormat& f, const Policy& p,
                               WordView view, const Word::WordDomain& domain) {
      unsigned int flags = 0;
      if (admits_magnitudes(f, view, domain, 0, f.maximum))
        flags |= flag_bit(0) | flag_bit(inexact);
      for (bool negative : {false, true}) {
        const WordValue code = overflow_code(f, p, negative);
        if (wide_intersect(view, domain, code, code).is_consistent)
          flags |= flag_bit(overflow | inexact);
      }
      if (admits_magnitudes(f, view, domain, 0, f.normal))
        flags |= flag_bit(underflow | inexact);
      return flags;
    }

    /// How x + x depends on the magnitude X of x within one piece
    enum class Doubling {
      /// One result for the whole piece
      Constant,
      /// The code 2X: a subnormal x doubled exactly
      Twice,
      /// The code X + normal: a normal x one binade up, exactly
      NextBinade
    };

    // Include in projections the codes of x, with sign bit sign and
    // magnitudes in [first, last], for which x + x is a code of the result
    // with an admitted status, and those results (see project_double).
    void include_double_piece(const WideFormat& f, const Policy& p,
                              const WordView views[3],
                              const Word::WordDomain domains[3],
                              StatusFlags& status, WordValue sign,
                              WordValue first, WordValue last, Doubling kind,
                              WideProjection projections[3]) {
      const auto piece =
          wide_intersect(views[0], domains[0], sign | first, sign | last);
      if (!piece.is_consistent)
        return;
      const WordValue mask = f.sign - 1;
      const auto codes = domain_codes(views[0], piece.domain);
      if (kind == Doubling::Constant) {
        const WideResult r =
            evaluate_wide(f, p, Arithmetic::Add, codes.first, codes.first);
        if (!status.admits(r.status) ||
            !wide_intersect(views[2], domains[2], r.code, r.code).is_consistent)
          return;
        status.produce(r.status);
        projections[0].include(views[0], piece.domain);
        projections[2].include(views[2], domains[2], r.code, r.code);
        return;
      }
      // An exact nonzero sum keeps the sign of x.
      const auto half = wide_intersect(views[2], domains[2], sign, sign | mask);
      if (!half.is_consistent)
        return;
      const auto results = domain_codes(views[2], half.domain);
      const std::uint64_t r_low = results.first & mask;
      const std::uint64_t r_high = results.last & mask;
      const std::uint64_t x_mask =
          ~(piece.domain.cube.lo ^ piece.domain.cube.hi) & mask;
      const std::uint64_t x_value = piece.domain.cube.lo & x_mask;
      const std::uint64_t z_mask =
          ~(half.domain.cube.lo ^ half.domain.cube.hi) & mask;
      const std::uint64_t z_value = half.domain.cube.lo & z_mask;
      std::uint64_t low = codes.first & mask, high = codes.last & mask;
      std::uint64_t least, greatest;
      if (kind == Doubling::NextBinade) {
        // The result X + normal meets the bits and the range of the result.
        if (r_high < f.normal)
          return;
        low = std::max(low, r_low < f.normal ? 0 : r_low - f.normal);
        high = std::min(high, r_high - f.normal);
        if (!least_with_masks(low, high, x_mask, x_value, f.normal, z_mask,
                              z_value, least) ||
            !greatest_with_masks(low, high, x_mask, x_value, f.normal, z_mask,
                                 z_value, greatest))
          return;
      } else {
        // Bit 0 of the result 2X is clear and its bit i + 1 is bit i of X.
        const std::uint64_t shifted_mask = z_mask >> 1;
        const std::uint64_t shifted_value = z_value >> 1;
        if ((z_value & 1) != 0 ||
            ((x_value ^ shifted_value) & x_mask & shifted_mask) != 0)
          return;
        low = std::max(low, (r_low + 1) >> 1);
        high = std::min(high, r_high >> 1);
        if (!least_with_masks(low, high, x_mask | shifted_mask,
                              x_value | shifted_value, 0, 0, 0, least) ||
            !greatest_with_masks(low, high, x_mask | shifted_mask,
                                 x_value | shifted_value, 0, 0, 0, greatest))
          return;
      }
      // Exact sums all have the status of the least one.
      const WideResult lr =
          evaluate_wide(f, p, Arithmetic::Add, sign | least, sign | least);
      const WideResult gr = evaluate_wide(f, p, Arithmetic::Add,
                                          sign | greatest, sign | greatest);
      assert(lr.code == (sign | (kind == Doubling::Twice ? 2 * least
                                                          : least + f.normal)));
      assert(gr.status == lr.status);
      if (!status.admits(lr.status))
        return;
      status.produce(lr.status);
      projections[0].include(views[0], piece.domain, sign | least,
                             sign | greatest);
      projections[2].include(views[2], half.domain, lr.code, gr.code);
    }

    /** \brief Exact projection of x + x for a finite x
     *
     * Within one sign, x + x is a monotone function of the magnitude X of
     * x, given piece by piece: a zero, or a subnormal x read as zero, gives
     * one zero; a subnormal x gives the exact code 2X, which the policy may
     * flush to one zero while it is subnormal; a normal x gives the exact
     * code X + normal one binade up until that overflows; and every x from
     * there gives the one overflow result of its sign. A piece with one
     * result is kept whole when that result and its status are admitted.
     * On the others the known bits of the result are known bits of 2X or X
     * + normal, and least_with_masks gives the extreme supported X. So the
     * projection is the exact hull of the supports, and of their results,
     * and repeating it changes nothing. Each kept piece adds the status of
     * its results to \a status. The special values of x are projected
     * before (project_wide) and are included in both operand projections,
     * so the second operand takes the projection of the first.
     */
    void project_double(const WideFormat& f, const Policy& p,
                        const WordView views[3],
                        const Word::WordDomain domains[3],
                        StatusFlags& status,
                        WideProjection projections[3]) {
      const bool flush_in = p.input_subnormals == Policy::SignedZero;
      const bool flush_out = p.output_subnormals == Policy::SignedZero;
      const WordValue half_normal = f.normal >> 1;
      const WordValue overflowing = f.infinity - f.normal;
      for (WordValue sign : {WordValue(0), f.sign}) {
        include_double_piece(f, p, views, domains, status, sign, 0, 0,
                             Doubling::Constant, projections);
        if (flush_in) {
          include_double_piece(f, p, views, domains, status, sign, 1,
                               f.normal - 1, Doubling::Constant, projections);
        } else {
          include_double_piece(f, p, views, domains, status, sign, 1,
                               half_normal - 1,
                               flush_out ? Doubling::Constant : Doubling::Twice,
                               projections);
          include_double_piece(f, p, views, domains, status, sign, half_normal,
                               f.normal - 1, Doubling::Twice, projections);
        }
        include_double_piece(f, p, views, domains, status, sign, f.normal,
                             overflowing - 1, Doubling::NextBinade,
                             projections);
        include_double_piece(f, p, views, domains, status, sign, overflowing,
                             f.maximum, Doubling::Constant, projections);
      }
      projections[1] = projections[0];
    }

    /** \brief Project a sum or difference on its pairs of operand parts
     *
     * A pair with a NaN or an infinity has a fixed outcome (project_special).
     * Both positions of a repeated operand hold one value, so only the pairs
     * of a part with itself exist there: the finite values of x + x take the
     * closed form of project_double, and x - x is an exact zero. Otherwise
     * the pairs of finite parts take the closed-form sum reasoning: their
     * rounded bounds give the result, and the cell of each finite result
     * part bounds the operands, settled by the lattice searches of
     * include_sum_operand. Where the result admits a zero, the greatest
     * finite magnitude or a nonfinite value, whose preimages are irregular,
     * the operands keep all their finite codes when the rounded sums of
     * some finite pair meet the result. Every pair that is kept adds its
     * flags to \a status: the rounded flags that its result can carry, with
     * O|X only where its sums can overflow and U|X only where flushing can
     * meet a nonzero sum below the least normal magnitude.
     */
    void project_wide(const WideFormat& f, const Policy& p, Arithmetic op,
                      const WordView views[3],
                      const Word::WordDomain domains[3],
                      StatusFlags& status, LatticeBudget& budget,
                      WideProjection projections[3]) {
      const bool repeated = views[0] == views[1];
      const auto a_parts = operand_parts(f, p, views[0], domains[0]);
      const auto b_parts =
          repeated ? a_parts : operand_parts(f, p, views[1], domains[1]);
      for (const auto& a : a_parts) {
        if (repeated) {
          if (!a.number())
            project_special(f, p, op, a, a, views, domains, status,
                            projections);
          continue;
        }
        for (const auto& b : b_parts)
          if (special_pair(op, a, b))
            project_special(f, p, op, a, b, views, domains, status,
                            projections);
      }
      if (!status.admits_finite())
        return;
      if (repeated && op == Arithmetic::Add) {
        project_double(f, p, views, domains, status, projections);
        return;
      }
      const auto as = finite_parts(f, p, views[0], domains[0], Role::Input);
      // Repeated subtraction is exact cancellation, including flushed inputs
      // and the directed zero sign.
      if (repeated) {
        const auto zero =
            round_wide(f, p, Integer(), p.rounding == Policy::RDN);
        if (!status.admits(zero.status))
          return;
        WideProjection result;
        result.include(views[2], domains[2], zero.code, zero.code);
        if (!result.present)
          return;
        status.produce(zero.status);
        projections[2].unite(result);
        for (const auto& a : as) {
          projections[0].include(views[0], a.domain);
          projections[1].include(views[1], a.domain);
        }
        return;
      }
      const auto bs = finite_parts(f, p, views[1], domains[1], Role::Input);
      const Integer normal = f.magnitude(f.normal);
      const bool flushes = p.output_subnormals == Policy::SignedZero;
      const unsigned int rounded = rounded_flags(f, p, views[2], domains[2]);
      // Whether the rounded sums of some pair of finite parts meet the result
      bool met = false;
      for (const auto& a : as)
        for (const auto& b : bs) {
          const Integer low =
              op == Arithmetic::Add ? a.low + b.low : a.low - b.high;
          const Integer high =
              op == Arithmetic::Add ? a.high + b.high : a.high - b.low;
          const WideResult least = round_wide(f, p, low);
          const WideResult greatest = round_wide(f, p, high);
          WideProjection result;
          include_rounded(f, result, views[2], domains[2], least.code,
                          greatest.code);
          if (!result.present)
            continue;
          met = true;
          projections[2].unite(result);
          // Exact sums lie on the subnormal grid, so only flushing a nonzero
          // sum below the least normal magnitude underflows.
          unsigned int flags = rounded;
          if (((least.status | greatest.status) & overflow) == 0)
            flags &= ~flag_bit(overflow | inexact);
          if (!(flushes && low < normal && high > -normal &&
                !(low.zero() && high.zero())))
            flags &= ~flag_bit(underflow | inexact);
          status.produced |= flags;
        }
      if (retain_inverse_operands(f, views[2], domains[2])) {
        // Keep every finite code of both operands: the united hulls of the
        // parts of a finite operand are the hull of its domain.
        if (met) {
          for (const auto& a : as)
            projections[0].include(views[0], a.domain);
          for (const auto& b : bs)
            projections[1].include(views[1], b.domain);
        }
        return;
      }
      const auto zs = finite_parts(f, p, views[2], domains[2], Role::Result);
      for (const auto& z : zs) {
        const Word::WordEncodingRange codes = domain_codes(views[2], z.domain);
        const Integer low =
            rounding_cell(f, p, z.negative ? codes.last : codes.first, 0, CellEnds::Open).low;
        const Integer high =
            rounding_cell(f, p, z.negative ? codes.first : codes.last, 0, CellEnds::Open).high;
        for (const auto& a : as)
          for (const auto& b : bs) {
            include_sum_operand(
                f, p, op, views, 0, a, b, z,
                op == Arithmetic::Add ? low - b.high : low + b.low,
                op == Arithmetic::Add ? high - b.low : high + b.high, budget,
                projections[0]);
            include_sum_operand(
                f, p, op, views, 1, b, a, z,
                op == Arithmetic::Add ? low - a.high : a.low - high,
                op == Arithmetic::Add ? high - a.low : a.high - low, budget,
                projections[1]);
          }
      }
    }

    /// How the free operand of a lattice search meets the result cells
    enum class Lattice { Product, Quotient };

    /** \brief Lattice search for one operand of a product or quotient
     *
     * One operand (the dividend of a quotient) is free and the other is the
     * single nonzero value \a fixed of encoding \a fixed_code. The free
     * operand has magnitude A = s*u in a binade with spacing u, and the
     * result codes of one binade with spacing g have cells that repeat
     * every code. Where the known bits of the result leave runs of 2^j
     * admitted codes in every period of 2^(j+t) codes, s is supported only
     * if s*K mod M lies in the window of one run, where K = u*|fixed| and M
     * = 2^(j+t)*g (products, with cells scaled by the square of the
     * half-subnormal unit), or K = u*2^scale and M = 2^(j+t)*g*|fixed|
     * (quotients A/|fixed|, whose cells are multiplied by |fixed|).
     * first_in_window and last_in_window find the extreme such s directly,
     * where the interval projection would retreat one encoding per pass.
     *
     * The windows come from a run that does not start a binade: the first
     * code of a binade has a narrower cell, as its predecessor is closer,
     * so translating a regular run gives a superset of every cell.
     */
    struct LatticeSearch {
      const WideFormat& f;
      const Policy& p;
      Lattice op;
      /// The operand that the search varies
      Operand free;
      WordView free_view;
      WordValue free_sign;
      WordValue fixed_code;
      Integer fixed;
      WordView result_view;
      const FinitePart& z;
      LatticeBudget& budget;
    };

    // The cell of the result code of magnitude m, as magnitudes in the
    // scale of the search.
    //
    // A quotient's cell is its closed rounding cell times |fixed|. That cell
    // is closed, as a quotient need not lie on the half-subnormal lattice, but
    // in this scale the free operand is the integer A*2^(127+fb), so an
    // open end excludes one unit, as in an open cell: both ends of an odd
    // code under ties to even, the end at the previous code under rounding
    // up, and the end at the next code under rounding down. An exact
    // quotient at the open end of a cell, as from dividing by a power of
    // two, then falls outside the cell.
    RoundingCell search_cell(const LatticeSearch& ls, WordValue m) {
      const WordValue code = (ls.z.negative ? ls.f.sign : 0) | m;
      RoundingCell cell;
      if (ls.op == Lattice::Product) {
        cell = rounding_cell(ls.f, ls.p, code, 127 + ls.f.fraction_bits,
                             CellEnds::Open);
      } else {
        const RoundingCell closed =
            rounding_cell(ls.f, ls.p, code, 0, CellEnds::Closed);
        bool low_open, high_open;
        if (ls.p.rounding == Policy::RNE) {
          low_open = high_open = (m & 1) != 0;
        } else {
          const bool upward = rounds_upward(ls.p.rounding, ls.z.negative);
          low_open = upward;
          high_open = !upward;
        }
        cell = {closed.low * ls.fixed + Integer(low_open ? 1 : 0),
                closed.high * ls.fixed - Integer(high_open ? 1 : 0)};
      }
      if (ls.z.negative)
        cell = {-cell.high, -cell.low};
      return cell;
    }

    // Whether the free operand's code supports the result part
    bool lattice_support(const LatticeSearch& ls,
                         const Word::WordDomain& admitted, WordValue code) {
      if (!wide_intersect(ls.free_view, admitted, code, code).is_consistent)
        return false;
      const WordValue a = ls.free == Operand::First ? code : ls.fixed_code;
      const WordValue b = ls.free == Operand::First ? ls.fixed_code : code;
      const WideResult r = ls.op == Lattice::Product
                               ? evaluate_multiply(ls.f, ls.p, a, b)
                               : evaluate_divide(ls.f, ls.p, a, b);
      return wide_intersect(ls.result_view, ls.z.domain, r.code, r.code)
          .is_consistent;
    }

    // The extreme significand s in [from, to] (with magnitude s*term in the
    // search scale, within the cells of the admitted extremes of runs, the
    // result codes of binade e) whose value can lie in a cell of a code of
    // the lowest run of known bits of the result, with the known fraction
    // bits x_mask and x_value of the free operand (and their lowest run,
    // known). Return false when there is none.
    //
    // The windows rule out unsupported s only, and so does the lowest run
    // of known bits, s = t*2^m + u with the run on u: known_run_candidate.
    // Every s between that bound and the next one with all known fraction
    // bits is not admitted, so the bound moves there (least_with_masks).
    bool window_candidate(const LatticeSearch& ls, const BinadeRuns& runs,
                          unsigned int e, const Integer& term,
                          const Integer& from, const Integer& to,
                          Direction direction, std::uint64_t x_mask,
                          std::uint64_t x_value, const KnownRun& known,
                          Integer& s) {
      const WideFormat& f = ls.f;
      Integer low, high, modulus;
      if (runs.periodic) {
        const Integer spacing = ls.op == Lattice::Product
                                    ? binade_unit(e).shift_left(
                                          127 + f.fraction_bits)
                                    : binade_unit(e) * ls.fixed;
        modulus = spacing * Integer(runs.period);
        low = search_cell(ls, runs.start).low;
        high = search_cell(ls, runs.start + runs.run - 1).high;
      }
      if (!modular_candidate(term, from, to, direction, low, high, modulus,
                             s) ||
          !known_run_candidate(known, term, from, to, direction, low, high,
                               modulus, s))
        return false;
      std::uint64_t masked;
      if (direction == Direction::Up
              ? !least_with_masks(s.word(), to.word(), x_mask, x_value, 0, 0,
                                  0, masked)
              : !greatest_with_masks(from.word(), s.word(), x_mask, x_value,
                                     0, 0, 0, masked))
        return false;
      s = Integer(masked);
      return true;
    }

    /// The result significands Z(s) = R(s*n/d) of one result binade as a
    /// function of the free significand s, R the rounding of the result
    struct ScaledIndex {
      Integer n, d;
      MagnitudeRounding rounding;
    };

    // Z(s) for s >= 0
    Integer scaled_index(const ScaledIndex& q, const Integer& s) {
      Integer quotient, remainder;
      Integer::divide(s * q.n, q.d, quotient, remainder);
      if (remainder.zero() || q.rounding == MagnitudeRounding::Down)
        return quotient;
      if (q.rounding == MagnitudeRounding::Up)
        return quotient + Integer(1);
      const int half = compare(remainder.shift_left(1), q.d);
      return half > 0 || (half == 0 && quotient.bit(0))
                 ? quotient + Integer(1)
                 : quotient;
    }

    // The least s >= 0 with Z(s) >= z, for z >= 1
    Integer least_reaching(const ScaledIndex& q, const Integer& z) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      if (q.rounding == MagnitudeRounding::Down)
        return ceil_div(z * q.d, q.n);
      if (q.rounding == MagnitudeRounding::Up)
        return floor_div((z - Integer(1)) * q.d, q.n) + Integer(1);
      // 2*s*n > (2*z - 1)*d, or equal with the tie going to an even z
      Integer quotient, remainder;
      Integer::divide((z.shift_left(1) - Integer(1)) * q.d, q.n.shift_left(1),
                      quotient, remainder);
      return remainder.zero() && !z.bit(0) ? quotient : quotient + Integer(1);
    }

    // The greatest s >= 0 with Z(s) <= z, for z >= 0
    Integer greatest_reaching(const ScaledIndex& q, const Integer& z) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      if (q.rounding == MagnitudeRounding::Down)
        return ceil_div((z + Integer(1)) * q.d, q.n) - Integer(1);
      if (q.rounding == MagnitudeRounding::Up)
        return floor_div(z * q.d, q.n);
      // 2*s*n < (2*z + 1)*d, or equal with the tie going to an even z
      Integer quotient, remainder;
      Integer::divide((z.shift_left(1) + Integer(1)) * q.d, q.n.shift_left(1),
                      quotient, remainder);
      return remainder.zero() && z.bit(0) ? quotient - Integer(1) : quotient;
    }

    // The run [low, high] of t around s >= 0 on which floor(t*e/d) +
    // ceil(t*e/d) stays constant, for e != 0 and d > 0, and that constant
    void offset_run(const Integer& s, const Integer& e, const Integer& d,
                    Integer& low, Integer& high, Integer& offset) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      const Integer v = floor_div(s * e, d);
      if ((s * e - v * d).zero()) {
        low = high = s;
        offset = v.shift_left(1);
        return;
      }
      // v*d < t*e < (v + 1)*d
      offset = v.shift_left(1) + Integer(1);
      if (e > Integer()) {
        low = floor_div(v * d, e) + Integer(1);
        high = ceil_div((v + Integer(1)) * d, e) - Integer(1);
      } else {
        low = floor_div((v + Integer(1)) * d, e) + Integer(1);
        high = ceil_div(v * d, e) - Integer(1);
      }
    }

    /**
     * \brief The extreme supported significand of one block: one binade
     * of the free operand against one binade e of the result
     *
     * Within the cells of the least and the greatest admitted code of
     * binade e, the result significand is Z(s) = R(s*n/d), R the rounding
     * of its magnitude (see SumLattice), monotone in s: for products n =
     * term and d = 2^b*2^scale, for quotients n = term and d = 2^b*|fixed|,
     * 2^b the spacing of binade e. A significand s is supported iff it
     * meets the known bits of the free operand and Z(s) the known fraction
     * bits of the result, which an exact evaluation still checks.
     *
     * Let 2^p be the power of two nearest to n/d, b' = max(1, -p) and a' =
     * p + b'. Then s*n/d = (s*2^a' + w(s))/2^b' with w(s) = s*E/d and E =
     * n*2^b' - d*2^a', and for an integer m and 0 < f < 1 every rounding R
     * gives R((m + f)/2^b') = R((m + 1/2)/2^b'), as b' >= 1. So Z(s) =
     * R((s*2^(a'+1) + W(s))/2^(b'+1)) with W(s) = floor(w(s)) + ceil(w(s)),
     * which is constant on runs of s (offset_run), and where it is, the
     * condition on s is one that block_extreme solves exactly from the bits
     * of s and of Z, as for a sum. The runs are long where n/d is close to
     * a power of two: there the bits of s and Z stay aligned over many s,
     * as for a product with a power of two, one with a significand next to
     * a power of two or a quotient by one.
     *
     * Where the run at s is short, the search alternates: the windows of
     * window_candidate give the next s that the lowest runs of known bits
     * of s and Z admit; if Z(s) misses the other known bits of Z, the next
     * significand Z' that meets them maps back to the least s' with Z(s')
     * >= Z' (least_reaching), and every s before s' maps to a code between,
     * which no admitted code holds. Either step moves s forward, by at
     * least one, and by a whole run or a whole gap of the known bits of Z.
     */
    bool block_candidate(const LatticeSearch& ls, unsigned int e,
                         const Integer& term, Integer from, Integer to,
                         Direction direction, std::uint64_t x_mask,
                         std::uint64_t x_value, const KnownRun& known,
                         Integer& s) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      const WideFormat& f = ls.f;
      const WordValue sign = ls.z.negative ? f.sign : 0;
      BinadeRuns runs;
      if (!binade_runs(f, ls.result_view, ls.z.domain, sign, e, runs))
        return false;
      from = std::max(from, ceil_div(search_cell(ls, runs.first).low, term));
      to = std::min(to, floor_div(search_cell(ls, runs.last).high, term));
      if (from > to)
        return false;
      const WordValue base = e * f.normal;
      const WordValue offset = e == 0 ? 0 : f.normal;
      const auto binade = wide_intersect(
          ls.result_view, ls.z.domain, sign | runs.first, sign | runs.last);
      assert(binade.is_consistent);
      const std::uint64_t z_mask =
          ~(binade.domain.cube.lo ^ binade.domain.cube.hi) & (f.normal - 1);
      const std::uint64_t z_value = binade.domain.cube.lo & z_mask;
      const std::uint64_t z_first = runs.first - base + offset;
      const std::uint64_t z_last = runs.last - base + offset;
      const ScaledIndex q{term,
                          ls.op == Lattice::Product
                              ? binade_unit(e).shift_left(127 + f.fraction_bits)
                              : binade_unit(e) * ls.fixed,
                          magnitude_rounding(ls.p.rounding, ls.z.negative)};
      // 2^p nearest to n/d: n/d lies in [2^p0, 2^(p0+1))
      int p = static_cast<int>(q.n.bits()) - static_cast<int>(q.d.bits());
      if (p >= 0 ? q.n < q.d.shift_left(static_cast<unsigned int>(p))
                 : q.n.shift_left(static_cast<unsigned int>(-p)) < q.d)
        --p;
      if ((p >= 0 ? q.n : q.n.shift_left(static_cast<unsigned int>(-p)))
              .shift_left(1) >=
          (p >= 0 ? q.d.shift_left(static_cast<unsigned int>(p)) : q.d) *
              Integer(3))
        ++p;
      const unsigned int b = p >= 0 ? 1U : static_cast<unsigned int>(-p);
      const unsigned int a = static_cast<unsigned int>(p + static_cast<int>(b));
      const Integer slope = q.n.shift_left(b) - q.d.shift_left(a);
      // Runs at least this long are solved whole, shorter ones alternate.
      // block_extreme reads the bits of Z modulo 2^64, which holds them
      // exactly while |p| plus the significand bits stays below 64; a
      // larger |p| (deep underflow, or a tiny free operand against a huge
      // factor) gives a block of at most a few result codes, which the
      // alternation settles in as many steps.
      const Integer long_run = Integer::power_two((f.fraction_bits + 1) / 2);
      const bool solvable = p >= -32 && p <= 32;
      const bool up = direction == Direction::Up;
      Integer x = up ? from : to;
      while (up ? x <= to : x >= from) {
        std::uint64_t masked;
        if (up ? !least_with_masks(x.word(), to.word(), x_mask, x_value, 0, 0,
                                   0, masked)
               : !greatest_with_masks(from.word(), x.word(), x_mask, x_value,
                                      0, 0, 0, masked))
          return false;
        x = Integer(masked);
        Integer low = from, high = to, constant;
        if (!slope.zero())
          offset_run(x, slope, q.d, low, high, constant);
        if (solvable && (slope.zero() || high - low + Integer(1) >= long_run)) {
          const Integer& end = up ? std::min(high, to) : std::max(low, from);
          const SumBlock block{(up ? x : end).word(),
                               (up ? end : x).word(),
                               x_mask,
                               x_value,
                               z_mask,
                               z_value,
                               a + 1,
                               b + 1,
                               q.rounding};
          std::uint64_t solved;
          if (block_extreme(block, constant, direction, solved)) {
            s = Integer(solved);
            return true;
          }
          x = up ? end + Integer(1) : end - Integer(1);
          continue;
        }
        Integer candidate;
        if (!(up ? window_candidate(ls, runs, e, term, x, to, direction, x_mask,
                                    x_value, known, candidate)
                 : window_candidate(ls, runs, e, term, from, x, direction,
                                    x_mask, x_value, known, candidate)))
          return false;
        // Within the cells z lies in [z_first, z_last]; the clamps only
        // keep the search moving should a policy narrow a cell.
        const std::uint64_t z = scaled_index(q, candidate).word();
        if (z >= z_first && z <= z_last && (z & z_mask) == z_value) {
          s = candidate;
          return true;
        }
        std::uint64_t next;
        if (up ? !least_with_masks(std::max(z + 1, z_first), z_last, z_mask,
                                   z_value, 0, 0, 0, next)
               : (z == 0 || !greatest_with_masks(z_first, std::min(z - 1, z_last),
                                                 z_mask, z_value, 0, 0, 0,
                                                 next)))
          return false;
        x = up ? std::max(least_reaching(q, Integer(next)),
                          candidate + Integer(1))
               : std::min(greatest_reaching(q, Integer(next)),
                          candidate - Integer(1));
      }
      return false;
    }

    // The least (Up) or greatest (Down) code of the free operand within
    // admitted that supports the result part, or, for a bounded search
    // that lattice_search_limit steps did not settle, a bound below (above)
    // which every code was shown unsupported, counted as a cut in its
    // budget. Return false when no code supports it.
    bool lattice_endpoint(const LatticeSearch& ls,
                          const Word::WordDomain& admitted,
                          Direction direction, WordValue& endpoint) {
      const WideFormat& f = ls.f;
      const WordValue mask = f.sign - 1;
      const bool up = direction == Direction::Up;
      const auto codes = domain_codes(ls.free_view, admitted);
      const WordValue lowest = codes.first & mask, highest = codes.last & mask;
      const auto result = domain_codes(ls.result_view, ls.z.domain);
      const unsigned int result_low =
          static_cast<unsigned int>((result.first & mask) >> f.fraction_bits);
      const unsigned int result_high =
          static_cast<unsigned int>((result.last & mask) >> f.fraction_bits);
      WordValue x = up ? lowest : highest;
      for (unsigned int i = 0; ls.budget.limit == LatticeLimit::Exhaustive ||
                               i < lattice_search_limit;
           ++i) {
        const auto next =
            wide_intersect(ls.free_view, admitted, ls.free_sign | (up ? x : lowest),
                           ls.free_sign | (up ? highest : x));
        if (!next.is_consistent)
          return false;
        const auto range = domain_codes(ls.free_view, next.domain);
        x = (up ? range.first : range.last) & mask;
        const unsigned int e = static_cast<unsigned int>(x >> f.fraction_bits);
        const WordValue base = e * f.normal;
        const WordValue start = std::max(lowest, std::max(WordValue(1), base));
        const WordValue stop = std::min(highest, base + f.normal - 1);
        // Significands of the codes of binade e
        const WordValue offset = e == 0 ? 0 : f.normal;
        const Integer from(offset + (up ? x : start) - base);
        const Integer to(offset + (up ? stop : x) - base);
        const Integer unit = binade_unit(e);
        const Integer term = ls.op == Lattice::Product
                                 ? unit * ls.fixed
                                 : unit.shift_left(127 + f.fraction_bits);
        // Result binades that the values of from..to reach
        Integer least = from * term, greatest = to * term;
        if (ls.op == Lattice::Quotient) {
          least = least / ls.fixed;
          greatest = greatest / ls.fixed;
        } else {
          least = least.shift_right(127 + f.fraction_bits);
          greatest = greatest.shift_right(127 + f.fraction_bits);
        }
        const unsigned int reach_low = std::max(
            result_low, static_cast<unsigned int>(floor_encoding(f, least) >>
                                                  f.fraction_bits));
        const unsigned int reach_high = std::min(
            result_high,
            static_cast<unsigned int>(
                std::min(f.maximum, floor_encoding(f, greatest) + 1) >>
                f.fraction_bits));
        const std::uint64_t x_mask =
            ~(next.domain.cube.lo ^ next.domain.cube.hi) & (f.normal - 1);
        const std::uint64_t x_value = next.domain.cube.lo & x_mask;
        const KnownRun known = known_run(next.domain.cube, f.fraction_bits);
        bool found = false;
        Integer s;
        for (unsigned int k = 0; !found && reach_low + k <= reach_high; ++k)
          found = block_candidate(ls, up ? reach_low + k : reach_high - k,
                                  term, from, to, direction, x_mask, x_value,
                                  known, s);
        WordValue next_code;
        if (found) {
          const WordValue candidate = s.word() - offset + base;
          if (lattice_support(ls, next.domain, ls.free_sign | candidate)) {
            endpoint = ls.free_sign | candidate;
            return true;
          }
          next_code = candidate;
        } else {
          next_code = up ? stop : start;
        }
        if (up ? next_code >= highest : next_code <= lowest)
          return false;
        x = up ? next_code + 1 : next_code - 1;
      }
      ++ls.budget.cuts;
      endpoint = ls.free_sign | x;
      return true;
    }

    // Narrow admitted, the encodings of the free operand, to the hull of
    // its codes that support the result part, up to the bounds of
    // lattice_endpoint. Return false when none does.
    bool narrow_to_lattice(const LatticeSearch& ls,
                           Word::WordDomain& admitted) {
      WordValue low, high;
      if (!lattice_endpoint(ls, admitted, Direction::Up, low) ||
          !lattice_endpoint(ls, admitted, Direction::Down, high))
        return false;
      const auto narrowed = wide_intersect(ls.free_view, admitted,
                                           std::min(low, high),
                                           std::max(low, high));
      if (!narrowed.is_consistent)
        return false;
      admitted = narrowed.domain;
      return true;
    }

    // Whether a lattice search applies to the free part x against the fixed
    // part y and the result part z: y is one nonzero value, x is neither
    // zero nor read as zero, and z holds neither zero, which binade_runs
    // leaves out, nor a flushed subnormal range.
    bool lattice_applies(const WideFormat& f, const Policy& p,
                         const FinitePart& x, const FinitePart& y,
                         WordView result, const FinitePart& z) {
      if (y.flushed || y.low.zero() || y.low != y.high || x.flushed ||
          (x.low.zero() && x.high.zero()))
        return false;
      const WordValue least =
          domain_codes(result, z.domain).first & (f.sign - 1);
      return least != 0 &&
             (p.output_subnormals != Policy::SignedZero || least >= f.normal);
    }

    void project_finite_product(const WideFormat& f, const Policy& p,
                                const FinitePart& a, const FinitePart& b,
                                const WordView views[3],
                                const Word::WordDomain domains[3],
                                LatticeBudget& budget,
                                WideProjection projections[3]) {
      Integer low = a.low * b.low, high = low;
      for (const Integer& av : {a.low, a.high})
        for (const Integer& bv : {b.low, b.high}) {
          const Integer product = av * bv;
          low = std::min(low, product);
          high = std::max(high, product);
        }
      const unsigned int source_scale = 127 + f.fraction_bits;
      const int exponent = -2 * static_cast<int>(source_scale);
      WideProjection forward;
      include_rounded(f, forward, views[2], domains[2],
                      round_dyadic(f, p, {low, exponent, false}).code,
                      round_dyadic(f, p, {high, exponent, false}).code);
      if (!forward.present)
        return;
      const auto candidate = Word::compute_domain_closure(
          views[2].width(), views[2].domain_type(),
          {{domains[2].cube.lo | forward.cube.lo,
            domains[2].cube.hi & forward.cube.hi},
           {std::max(domains[2].interval.minimum, forward.interval.minimum),
            std::min(domains[2].interval.maximum, forward.interval.maximum)}});
      if (!candidate.is_consistent)
        return;
      // Infinity and maximum-finite tails have unbounded/saturation preimages.
      // The forward intersection has already checked branch compatibility.
      bool tail = false;
      for (WordValue sign : {WordValue(0), f.sign})
        for (WordValue magnitude : {f.maximum, f.infinity})
          tail |= wide_intersect(views[2], candidate.domain, sign | magnitude,
                                 sign | magnitude)
                      .is_consistent;
      if (tail) {
        projections[0].include(views[0], a.domain);
        projections[1].include(views[1], b.domain);
        projections[2].include(views[2], candidate.domain);
        return;
      }
      for (const auto& z :
           finite_parts(f, p, views[2], candidate.domain, Role::Result)) {
        const Word::WordEncodingRange codes = domain_codes(views[2], z.domain);
        const WordValue low_code = z.negative ? codes.last : codes.first;
        const WordValue high_code = z.negative ? codes.first : codes.last;
        const RoundingCell cell{
            std::max(low, rounding_cell(f, p, low_code, source_scale, CellEnds::Open).low),
            std::min(high, rounding_cell(f, p, high_code, source_scale, CellEnds::Open).high)};
        if (cell.low > cell.high)
          continue;
        const RoundingCell left = quotient_bounds(cell, b);
        const RoundingCell right = quotient_bounds(cell, a);
        Word::WordDomain admitted[2];
        if (!real_domain(f, views[0], a, left.low, left.high, admitted[0]) ||
            !real_domain(f, views[1], b, right.low, right.high, admitted[1]))
          continue;
        // One factor fixed: the other one's endpoints by the lattice search
        const FinitePart* parts[] = {&a, &b};
        bool supported = true;
        for (int i = 0; i < 2 && supported; ++i)
          if (lattice_applies(f, p, *parts[i], *parts[1 - i], views[2], z)) {
            const LatticeSearch search{
                f, p, Lattice::Product,
                i == 0 ? Operand::First : Operand::Second, views[i],
                parts[i]->negative ? f.sign : 0,
                domain_codes(views[1 - i], parts[1 - i]->domain).first,
                parts[1 - i]->low.abs(), views[2], z, budget};
            supported = narrow_to_lattice(search, admitted[i]);
          }
        if (!supported)
          continue;
        WideProjection sources[2];
        for (int i = 0; i < 2; ++i)
          sources[i].include(views[i], admitted[i]);
        for (int i = 0; i < 2; ++i)
          projections[i].include(views[i],
                                 {sources[i].cube, sources[i].interval});
        projections[2].include(views[2], z.domain);
      }
    }

    void project_multiply(const WideFormat& f, const Policy& p,
                          const WordView views[3],
                          const Word::WordDomain domains[3],
                          StatusFlags& status, LatticeBudget& budget,
                          WideProjection projections[3]) {
      const auto as = operand_parts(f, p, views[0], domains[0]);
      const auto bs = operand_parts(f, p, views[1], domains[1]);
      const unsigned int rounded =
          rounded_flags(f, p, views[2], domains[2]);
      for (const auto& a : as)
        for (const auto& b : bs) {
          if (special_pair(Arithmetic::Multiply, a, b)) {
            project_special(f, p, Arithmetic::Multiply, a, b, views, domains,
                            status, projections);
          } else if (status.admits_finite()) {
            status.produced |= rounded;
            project_finite_product(f, p, a.finite, b.finite, views, domains,
                                   budget, projections);
          }
        }
    }

    struct QuotientEndpoint {
      Integer numerator, denominator;
      QuotientEndpoint(const Integer& a, const Integer& b)
          : numerator(b.neg() ? -a : a), denominator(b.abs()) {}
      bool operator<(const QuotientEndpoint& other) const {
        return numerator * other.denominator < other.numerator * denominator;
      }
      WordValue rounded(const WideFormat& f, const Policy& p) const {
        return round_quotient(f, p, numerator.abs(), denominator,
                              numerator.neg())
            .code;
      }
    };

    // The numerators in cell * denominator, scaled down by 2^scale_bits
    RoundingCell division_numerator_bounds(const RoundingCell& cell,
                                           const FinitePart& denominator,
                                           unsigned int scale_bits) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      Integer low = cell.low * denominator.low, high = low;
      for (const Integer& target : {cell.low, cell.high})
        for (const Integer& divisor : {denominator.low, denominator.high}) {
          const Integer product = target * divisor;
          low = std::min(low, product);
          high = std::max(high, product);
        }
      return {ceil_shift_right(low, scale_bits), floor_shift_right(high, scale_bits)};
    }

    // The denominators in numerator * 2^scale_bits / cell
    RoundingCell division_denominator_bounds(const RoundingCell& cell,
                                             const FinitePart& numerator,
                                             unsigned int scale_bits) {
      assert(cell.low > Integer() || cell.high < Integer());
      const Integer* dividends[] = {&numerator.low, &numerator.high};
      const Integer* targets[] = {&cell.low, &cell.high};
      const int dividend_count = numerator.low == numerator.high ? 1 : 2;
      const int target_count = cell.low == cell.high ? 1 : 2;
      RoundingCell bounds;
      for (int i = 0; i < dividend_count; ++i) {
        const Integer scaled = dividends[i]->shift_left(scale_bits);
        for (int j = 0; j < target_count; ++j) {
          Integer floor, ceiling;
          Word::Float::floor_ceil_div(scaled, *targets[j], floor, ceiling);
          if (i == 0 && j == 0) {
            bounds = {std::move(ceiling), std::move(floor)};
            continue;
          }
          if (ceiling < bounds.low)
            bounds.low = std::move(ceiling);
          if (floor > bounds.high)
            bounds.high = std::move(floor);
        }
      }
      return bounds;
    }

    void project_finite_quotient(const WideFormat& f, const Policy& p,
                                 const FinitePart& a, const FinitePart& b,
                                 const WordView views[3],
                                 const Word::WordDomain domains[3],
                                 LatticeBudget& budget,
                                 WideProjection projections[3]) {
      QuotientEndpoint low(a.low, b.low), high = low;
      for (const Integer& av : {a.low, a.high})
        for (const Integer& bv : {b.low, b.high}) {
          const QuotientEndpoint quotient(av, bv);
          low = std::min(low, quotient);
          high = std::max(high, quotient);
        }
      WideProjection forward;
      include_rounded(f, forward, views[2], domains[2], low.rounded(f, p),
                      high.rounded(f, p));
      if (!forward.present)
        return;
      const auto candidate = Word::compute_domain_closure(
          views[2].width(), views[2].domain_type(),
          {{domains[2].cube.lo | forward.cube.lo,
            domains[2].cube.hi & forward.cube.hi},
           {std::max(domains[2].interval.minimum, forward.interval.minimum),
            std::min(domains[2].interval.maximum, forward.interval.maximum)}});
      if (!candidate.is_consistent)
        return;
      // Maximum finite and infinity may have unbounded rounding preimages.
      bool tail = false;
      for (WordValue sign : {WordValue(0), f.sign})
        for (WordValue magnitude : {f.maximum, f.infinity})
          tail |= wide_intersect(views[2], candidate.domain, sign | magnitude,
                                 sign | magnitude)
                      .is_consistent;
      if (tail) {
        projections[0].include(views[0], a.domain);
        projections[1].include(views[1], b.domain);
        projections[2].include(views[2], candidate.domain);
        return;
      }
      const unsigned int scale_bits = 127 + f.fraction_bits;
      for (const auto& z :
           finite_parts(f, p, views[2], candidate.domain, Role::Result)) {
        const Word::WordEncodingRange codes = domain_codes(views[2], z.domain);
        const RoundingCell cell{
            rounding_cell(f, p, z.negative ? codes.last : codes.first, 0, CellEnds::Closed).low,
            rounding_cell(f, p, z.negative ? codes.first : codes.last, 0, CellEnds::Closed).high};
        const RoundingCell numerator =
            division_numerator_bounds(cell, b, scale_bits);
        WideProjection sources[2];
        Word::WordDomain dividends;
        if (!real_domain(f, views[0], a, numerator.low, numerator.high,
                         dividends))
          continue;
        // A fixed divisor: the dividend's endpoints by the lattice search
        if (lattice_applies(f, p, a, b, views[2], z)) {
          const LatticeSearch search{
              f, p, Lattice::Quotient, Operand::First, views[0],
              a.negative ? f.sign : 0, domain_codes(views[1], b.domain).first,
              b.low.abs(), views[2], z, budget};
          if (!narrow_to_lattice(search, dividends))
            continue;
        }
        sources[0].include(views[0], dividends);
        if (cell.low <= Integer() && cell.high >= Integer()) {
          sources[1].include(views[1], b.domain);
        } else {
          const RoundingCell denominator =
              division_denominator_bounds(cell, a, scale_bits);
          include_real(f, sources[1], views[1], b, denominator.low,
                       denominator.high);
        }
        if (!sources[1].present)
          continue;
        for (int i = 0; i < 2; ++i)
          projections[i].include(views[i],
                                 {sources[i].cube, sources[i].interval});
        projections[2].include(views[2], z.domain);
      }
    }

    void project_divide(const WideFormat& f, const Policy& p,
                        const WordView views[3],
                        const Word::WordDomain domains[3],
                        StatusFlags& status, LatticeBudget& budget,
                        WideProjection projections[3]) {
      const auto as = operand_parts(f, p, views[0], domains[0]);
      if (views[0] == views[1]) {
        for (const auto& a : as) {
          if (special_pair(Arithmetic::Divide, a, a)) {
            project_special(f, p, Arithmetic::Divide, a, a, views, domains,
                            status, projections);
          } else if (status.admits(0)) {
            // Every effective finite nonzero x has the exact quotient x/x=1.
            const WordValue one = 127 * f.normal;
            WideProjection result;
            result.include(views[2], domains[2], one, one);
            if (!result.present)
              continue;
            status.produce(0);
            projections[2].unite(result);
            projections[0].include(views[0], a.finite.domain);
            projections[1].include(views[1], a.finite.domain);
          }
        }
        return;
      }
      const auto bs = operand_parts(f, p, views[1], domains[1]);
      const unsigned int rounded =
          rounded_flags(f, p, views[2], domains[2]);
      for (const auto& a : as)
        for (const auto& b : bs) {
          if (special_pair(Arithmetic::Divide, a, b)) {
            project_special(f, p, Arithmetic::Divide, a, b, views, domains,
                            status, projections);
          } else if (status.admits_finite()) {
            status.produced |= rounded;
            project_finite_quotient(f, p, a.finite, b.finite, views, domains,
                                    budget, projections);
          }
        }
    }

    /*
     * The widened projection of a product or quotient (the step rule)
     *
     * A read of the domains (read_word, read_result in float-wide.hpp) gives
     * each operand a closed range of values and the result a closed range of
     * exact values: the hull read from the least to the greatest code, the
     * widened read from pred to succ, the nearest codes outside the interval
     * that the read cube admits. The pieces are the sign parts of the two
     * operand reads: negative values up to -1 and positive values from 1, as
     * every nonzero value has magnitude at least 2 (in half-subnormal units).
     * The codes of value zero (zeros, flushed subnormals), infinities and NaNs
     * are not pieces: their pairs have a fixed result class and status, which
     * project_special decides exactly on the classes of the domains, as for
     * the interval projections.
     *
     * On a piece the operands x and y have one sign each, and so has every
     * exact value v of the relation: v = x*y (at 2^k over the unit of the
     * values, k = 127 + fraction bits, the scale of an open rounding_cell), or v =
     * x*2^(k+S)/y (at 2^S, S = quotient_scale). The piece projects the real
     * relation exactly, from corners, as the relation is monotone in each
     * argument on a box of one sign:
     *  - x onto X with V/Y (V*Y/2^(k+S) for quotients),
     *  - y onto Y with V/X (X*2^(k+S)/V for quotients),
     *  - v onto V with X*Y (X*2^(k+S)/Y),
     * where V is the result read in the half of the piece's sign. Operand
     * values are integers, so the inward integer hull of the projection keeps
     * exactly the codes in it. Products are integers too; quotients are not,
     * so the projection of v is rounded outward, which only adds cells that
     * meet the real projection's integer hull. The cells of the result codes
     * are those of ResultCells with the piece's sign, closed for quotients.
     * Each word keeps its codes inside the projection of some piece whose
     * three projections all keep a code.
     *
     * The status is a finite case analysis outside the projection: the pieces
     * are projected only when the status admits an outcome of a finite
     * product or quotient (exact, inexact, underflow or overflow), and when
     * one is kept, the status admits the flag values of rounded_flags.
     *
     * Why a widened step is a fixpoint of itself: this is the argument before
     * Read in float-wide.hpp, with the cells of ResultCells, which are
     * nondecreasing in the rank and leave no gap in the half of the piece. A
     * piece kept at the first step keeps, at the second, every code it kept,
     * because its projection at the new read contains the old one; the
     * classes of the special pairs are kept whole.
     * The second step can prune again only when the read of a word changes
     * otherwise: a bit that interval_bits newly fixes (at most once per bit
     * of each word), a special class that the first step removed (at most
     * once per class), or a status value that it removed. This bounds the
     * steps of the step rule (before StepState in float-wide.hpp).
     */

    /// The scale S of the exact quotients of the widened projection. Every
    /// quotient of values of magnitude at least 1 and at most the largest
    /// finite value is at least 2^(S-129) at this scale, above 1, as
    /// ResultCells needs; and the closed cells add only 2^-S units.
    constexpr unsigned int quotient_scale = 136;

    /// The sign parts of a read: values up to -1 and from 1
    void sign_parts(const ValueRange& r, ValueRange parts[2]) {
      parts[0] = {r.low, std::min(r.high, -Integer(1))};
      parts[1] = {std::max(r.low, Integer(1)), r.high};
    }

    /// The inward integer hull of the corner products of [a_low, a_high] x
    /// [b_low, b_high], divided by 2^shift
    ValueRange corner_products(const Integer& a_low, const Integer& a_high,
                               const Integer& b_low, const Integer& b_high,
                               unsigned int shift) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      Integer low = a_low * b_low, high = low;
      for (const Integer& a : {a_low, a_high})
        for (const Integer& b : {b_low, b_high}) {
          const Integer product = a * b;
          low = std::min(low, product);
          high = std::max(high, product);
        }
      return {ceil_shift_right(low, shift), floor_shift_right(high, shift)};
    }

    /// The outward integer hull [floor(min), ceil(max)] of the corner
    /// quotients n/d, d of one sign
    ValueRange outward_quotients(const Integer& n_low, const Integer& n_high,
                                 const Integer& d_low, const Integer& d_high) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      ValueRange q{floor_div(n_low, d_low), ceil_div(n_low, d_low)};
      for (const Integer& n : {n_low, n_high})
        for (const Integer& d : {d_low, d_high}) {
          q.low = std::min(q.low, floor_div(n, d));
          q.high = std::max(q.high, ceil_div(n, d));
        }
      return q;
    }

    /// Intersect r with the range [low, high]
    void clip(ValueRange& r, const Integer& low, const Integer& high) {
      r.low = std::max(r.low, low);
      r.high = std::min(r.high, high);
    }

    /// Project a product or quotient of three distinct views with read onto
    /// projections, and add the flag values of the kept pairs to status
    /// (see above)
    void project_widened(const WideFormat& f, const Policy& p, Arithmetic op,
                         const WordView views[3],
                         const Word::WordDomain domains[3],
                         StatusFlags& status, Read read,
                         WideProjection projections[3]) {
      const bool divide = op == Arithmetic::Divide;
      const auto as = operand_parts(f, p, views[0], domains[0]);
      const auto bs = operand_parts(f, p, views[1], domains[1]);
      bool finite = false;
      for (const auto& a : as)
        for (const auto& b : bs)
          if (special_pair(op, a, b))
            project_special(f, p, op, a, b, views, domains, status,
                            projections);
          else
            finite = true;
      if (!finite || !status.admits_finite())
        return;
      const unsigned int k = 127 + f.fraction_bits;
      const unsigned int scale = divide ? quotient_scale : k;
      const Integer beyond = f.magnitude(f.infinity);
      // Bounds every exact value of a piece and every finite cell end
      const Integer big = divide ? beyond.shift_left(k + scale + 2)
                                 : beyond * beyond * Integer(4);
      const CellEnds ends = divide ? CellEnds::Closed : CellEnds::Open;
      const ResultCells cells[2] = {
          {f, p, scale, big, ResultSign::Positive, ends},
          {f, p, scale, big, ResultSign::Negative, ends}};
      const RoundingCell reads[2] = {
          read_result(views[2], domains[2], cells[0], read),
          read_result(views[2], domains[2], cells[1], read)};
      ValueRange xs[2], ys[2];
      sign_parts(read_word(f, p, views[0], domains[0], read), xs);
      sign_parts(read_word(f, p, views[1], domains[1], read), ys);
      WideProjection wholes[3];
      for (const ValueRange& x : xs)
        for (const ValueRange& y : ys) {
          if (x.empty() || y.empty())
            continue;
          const int negative = (&x == &xs[0]) != (&y == &ys[0]) ? 1 : 0;
          Integer v_low = reads[negative].low, v_high = reads[negative].high;
          if (negative)
            v_high = std::min(v_high, -Integer(1));
          else
            v_low = std::max(v_low, Integer(1));
          if (v_low > v_high)
            continue;
          ValueRange xq, yq, vq;
          if (divide) {
            const unsigned int shift = k + scale;
            xq = corner_products(v_low, v_high, y.low, y.high, shift);
            yq = corner_quotients(x.low.shift_left(shift),
                                  x.high.shift_left(shift), v_low, v_high);
            vq = outward_quotients(x.low.shift_left(shift),
                                   x.high.shift_left(shift), y.low, y.high);
          } else {
            xq = corner_quotients(v_low, v_high, y.low, y.high);
            yq = corner_quotients(v_low, v_high, x.low, x.high);
            vq = corner_products(x.low, x.high, y.low, y.high, 0);
          }
          clip(xq, x.low, x.high);
          clip(yq, y.low, y.high);
          clip(vq, v_low, v_high);
          if (xq.empty() || yq.empty() || vq.empty())
            continue;
          WideProjection piece[3];
          include_values(f, p, piece[0], views[0], domains[0], xq.low, xq.high);
          include_values(f, p, piece[1], views[1], domains[1], yq.low, yq.high);
          include_meeting(piece[2], views[2], domains[2], cells[negative],
                          vq.low, vq.high);
          if (!piece[0].present || !piece[1].present || !piece[2].present)
            continue;
          for (int i = 0; i < 3; ++i)
            merge(wholes[i], views[i], piece[i]);
        }
      if (!wholes[2].present)
        return;
      status.produced |= rounded_flags(f, p, views[2], domains[2]);
      for (int i = 0; i < 3; ++i)
        projections[i].unite(wholes[i]);
    }

    // Narrow domains[i] to the hull of projections[i] for i < n, as
    // publish_projections narrows the views: positions sharing a view take
    // the intersection of their projections, and every position of that
    // view gets the result. Wide words have no value sets, so a domain is
    // exactly its cube and interval, closed as the view would close it.
    bool narrow_domains(const WordView views[3], Word::WordDomain domains[3],
                        const WideProjection projections[], int n) {
      for (int i = 0; i < n; ++i) {
        int earlier = 0;
        while (earlier < i && views[earlier] != views[i])
          ++earlier;
        if (earlier < i)
          continue;
        WideProjection shared = projections[i];
        for (int j = i + 1; j < n; ++j)
          if (views[j] == views[i])
            shared.intersect(projections[j]);
        if (!shared.present)
          return false;
        const Word::WordDomain& d = domains[i];
        const auto narrowed = Word::compute_domain_closure(
            views[i].width(), views[i].domain_type(),
            {{d.cube.lo | shared.cube.lo, d.cube.hi & shared.cube.hi},
             {std::max(d.interval.minimum, shared.interval.minimum),
              std::min(d.interval.maximum, shared.interval.maximum)}});
        if (!narrowed.is_consistent)
          return false;
        for (int j = 0; j < 3; ++j)
          if (views[j] == views[i])
            domains[j] = narrowed.domain;
      }
      return true;
    }

    bool same_domain(const Word::WordDomain& x, const Word::WordDomain& y) {
      return x.cube.lo == y.cube.lo && x.cube.hi == y.cube.hi &&
             x.interval.minimum == y.interval.minimum &&
             x.interval.maximum == y.interval.maximum;
    }

    // Narrow each view to its domain.
    bool publish_domains(Space& home, const WordView views[3],
                         const Word::WordDomain domains[3]) {
      for (int i = 0; i < 3; ++i)
        if (me_failed(Word::narrow_word_domain(home, views[i], domains[i].cube,
                                               domains[i].interval)))
          return false;
      return true;
    }

    // Cancellation bound for finite nonzero sums, in every rounding
    // direction. In the exact integer scale let C(z) = (2*normal-1)*2^ctz(|z|).
    // A positive result z admits operands only within [-C(z), z+C(z)], a
    // negative one within [z-C(z), C(z)]; subtraction negates the enclosure
    // of operand two. Zero, nonfinite and maximum-finite results keep
    // conservative bounds.
    //
    // Why: the cell of z (its rounding preimage) holds no multiple of
    // g = 2^(ctz(|z|)+1). When both operands lie on the grid of g (their
    // spacings are at least g), so does their exact sum. Hence one operand
    // has spacing at most g/2, and so magnitude at most C(z), and the other
    // is the sum minus it. The bound is a closed-form jump: without it the
    // inverse projection retreats one encoding per round where the cell is
    // narrower than both operand spacings.
    //
    // A fixed exponent and trailing-significand-zero count fixes C(z).
    // Intersect the class with the actual cube before taking its endpoints:
    // an interior power of two can have a much larger C than either endpoint.
    bool apply_cancellation_bound(const WideFormat& f, const Policy& p,
                                  Arithmetic op, const WordView views[3],
                                  Word::WordDomain domains[3]) {
      // The bound holds under every policy, given that nonzero
      // non-maximum finite results only arise from their regular cell:
      // - SaturateFinite changes only overflowing results, which are the
      //   maximum finite (retained below); other cells are unchanged.
      // - Output SignedZero replaces subnormal results by signed zeros
      //   (retained below). A normal result keeps its cell; a subnormal
      //   code has no preimage, so its class only widens the bound.
      // - Input SignedZero reads subnormal operands as zero. Zero is on
      //   every grid, so the argument holds for effective values, and
      //   finite_parts gives flushed parts the value zero, which lies in
      //   every enclosure: their encodings are never removed.
      if (op != Arithmetic::Add && op != Arithmetic::Subtract)
        return true;
      const Word::WordDomain& result = domains[2];
      if (retain_inverse_operands(f, views[2], result))
        return true;
      bool present = false;
      Integer low, high;
      for (WordValue sign : {WordValue(0), f.sign})
        for (unsigned int exponent = 0; exponent < 255; ++exponent) {
          const WordValue first = std::max(WordValue(1), exponent * f.normal);
          const WordValue last = (exponent + 1) * f.normal - 1;
          const auto binade =
              wide_intersect(views[2], result, sign | first, sign | last);
          if (!binade.is_consistent)
            continue;
          for (unsigned int trailing = 0; trailing <= f.fraction_bits;
               ++trailing) {
            if (exponent == 0 && trailing == f.fraction_bits)
              continue;
            auto candidate = binade.domain;
            const WordValue bit = WordValue(1) << trailing;
            candidate.cube.hi &= ~(bit - 1);
            if (trailing < f.fraction_bits)
              candidate.cube.lo |= bit;
            if (candidate.cube.lo & ~candidate.cube.hi)
              continue;
            const auto admitted = Word::compute_domain_closure(
                views[2].width(), views[2].domain_type(), candidate);
            if (!admitted.is_consistent)
              continue;
            // The greatest encoding of a sign half has the largest magnitude.
            const WordValue largest = domain_codes(views[2], admitted.domain).last;
            const Integer span = Integer(2 * f.normal - 1)
                                     .shift_left(std::max(1U, exponent) + trailing);
            const Integer z = f.value(largest);
            const Integer lower = sign ? z - span : -span;
            const Integer upper = sign ? span : z + span;
            low = present ? std::min(low, lower) : lower;
            high = present ? std::max(high, upper) : upper;
            present = true;
          }
        }
      if (!present)
        return false;
      WideProjection projections[2];
      for (int column = 0; column < 2; ++column)
        for (const auto& part :
             finite_parts(f, p, views[column], domains[column], Role::Input))
          include_real(f, projections[column], views[column], part,
                       column == 1 && op == Arithmetic::Subtract ? -high : low,
                       column == 1 && op == Arithmetic::Subtract ? -low : high);
      return narrow_domains(views, domains, projections, 2);
    }

    // Result/input aliases need their correlated rounding cell. Treating the
    // input and result as independent intervals can move an endpoint by one
    // encoding per invocation. Within each class below cell(x)-x is constant;
    // the rational residuals have their extrema at the admitted endpoints.
    RoundingCell alias_operand_bounds(const WideFormat& f, const Policy& p,
                                      Arithmetic op, int alias,
                                      const FinitePart& part, WordValue first,
                                      WordValue last) {
      const unsigned int scale_bits = 127 + f.fraction_bits;
      bool present = false;
      RoundingCell bounds;
      for (WordValue code : {first, last}) {
        const Integer x = f.value(code);
        RoundingCell cell;
        if (op == Arithmetic::Multiply) {
          cell = quotient_bounds(rounding_cell(f, p, code, scale_bits, CellEnds::Open),
                                 {part.domain, x, x, part.negative, false});
        } else if (op == Arithmetic::Divide && alias == 0) {
          // x / b rounds to x, so b lies in x / cell(x).
          const auto target = rounding_cell(f, p, code, 0, CellEnds::Closed);
          cell = target.low <= Integer() && target.high >= Integer()
                     ? RoundingCell{-f.magnitude(f.maximum),
                                    f.magnitude(f.maximum)}
                     : division_denominator_bounds(
                           target, {part.domain, x, x, part.negative, false},
                           scale_bits);
        } else if (op == Arithmetic::Divide) {
          // a / x rounds to x, so a lies in x * cell(x).
          cell = division_numerator_bounds(
              rounding_cell(f, p, code, 0, CellEnds::Closed),
              {part.domain, x, x, part.negative, false}, scale_bits);
        } else {
          const auto target = rounding_cell(f, p, code, 0, CellEnds::Open);
          if (op == Arithmetic::Add)
            cell = {target.low - x, target.high - x};
          else if (alias == 0)
            cell = {x - target.high, x - target.low};
          else
            cell = {target.low + x, target.high + x};
        }
        bounds.low = present ? std::min(bounds.low, cell.low) : cell.low;
        bounds.high = present ? std::max(bounds.high, cell.high) : cell.high;
        present = true;
      }
      return bounds;
    }

    // Add to status the flags of every pair of operand classes whose
    // outcome the result and the status admit, reading the operands as
    // independent: an alias only removes pairs.
    void alias_flags(const WideFormat& f, const Policy& p, Arithmetic op,
                     const WordView views[3],
                     const Word::WordDomain domains[3], StatusFlags& status) {
      const bool repeated = views[0] == views[1];
      const auto as = operand_parts(f, p, views[0], domains[0]);
      const auto bs =
          repeated ? as : operand_parts(f, p, views[1], domains[1]);
      const unsigned int rounded =
          rounded_flags(f, p, views[2], domains[2]);
      WideProjection ignored[3];
      for (size_t i = 0; i < as.size(); ++i)
        for (size_t j = repeated ? i : 0; j < (repeated ? i + 1 : bs.size());
             ++j) {
          if (special_pair(op, as[i], bs[j]))
            project_special(f, p, op, as[i], bs[j], views, domains, status,
                            ignored);
          else if (status.admits_finite())
            status.produced |= rounded;
        }
    }

    // Each class of the aliased word is kept whole or dropped, and the
    // other operand keeps the encodings that support a kept class. Keeping
    // a whole class is conservative: a code of it may lack a support of its
    // own, but every supported code is kept. Once the
    // projection of the other operand equals the hull of its whole domain,
    // inclusions no longer change it and a class needs only one support.
    // Special operand values only reach the tails, which are always kept.
    void project_alias(const WideFormat& f, const Policy& p, Arithmetic op,
                       const WordView views[3],
                       const Word::WordDomain domains[3], StatusFlags& status,
                       WideProjection projections[3]) {
      alias_flags(f, p, op, views, domains, status);
      const int alias = views[0] == views[2] ? 0 : 1;
      const int other = 1 - alias;
      const bool shared = views[other] == views[alias];
      const auto others =
          finite_parts(f, p, views[other], domains[other], Role::Input);
      WideProjection all;
      if (!shared)
        all.include(views[other], domains[other]);
      WideProjection kept;
      visit_alias_classes(
          f, p, views[alias], domains[alias], kept,
          [&](const FinitePart& part, WordValue first, WordValue last) {
            const auto bounds =
                alias_operand_bounds(f, p, op, alias, part, first, last);
            Word::WordDomain admitted;
            if (shared)
              return real_domain(f, views[other], part, bounds.low,
                                 bounds.high, admitted);
            bool supported = false;
            for (const auto& operand : others) {
              if (!real_domain(f, views[other], operand, bounds.low,
                               bounds.high, admitted))
                continue;
              supported = true;
              if (projections[other].covers(all))
                break;
              projections[other].include(views[other], admitted);
            }
            return supported;
          },
          [&](const Word::WordDomain&) {
            // Tails retain the other operand; regular classes still get
            // filtered.
            if (!shared)
              projections[other].unite(all);
          });
      projections[alias] = kept;
      projections[2] = kept;
      if (shared)
        projections[other] = kept;
    }

    class WideArithmetic : public Propagator {
      WordView views[3];
      Int::IntView status;
      const bool has_status;
      const WideFormat format;
      const Policy policy;
      const Arithmetic operation;

      /// Outcome of iterating the projections on local domains
      enum class Iteration { Failed, Converged, Assigned, Exhausted };
      /// Passes per execution of the interval projections
      static constexpr unsigned int interval_pass_limit = 64;

      bool first_occurrence(int i) const {
        for (int j = 0; j < i; ++j)
          if (views[i] == views[j])
            return false;
        return true;
      }
      ExecStatus enforce_assigned_inputs(Space& home) {
        WideResult outcome;
        switch (operation) {
        case Arithmetic::Multiply:
          outcome =
              evaluate_multiply(format, policy, views[0].val(), views[1].val());
          break;
        case Arithmetic::Divide:
          outcome =
              evaluate_divide(format, policy, views[0].val(), views[1].val());
          break;
        case Arithmetic::Add:
        case Arithmetic::Subtract:
          outcome = evaluate_wide(format, policy, operation, views[0].val(),
                                  views[1].val());
          break;
        }
        if (has_status)
          GECODE_ME_CHECK(status.eq(home, outcome.status));
        GECODE_ME_CHECK(views[2].eq(home, outcome.code));
        return home.ES_SUBSUMED(*this);
      }

      // A result aliasing an assigned operand holds that one value in both
      // roles, so the independent projections are exact for it.
      bool result_alias() const {
        return (views[2] == views[0] && !views[0].assigned()) ||
               (views[2] == views[1] && !views[1].assigned());
      }
      // Passes per execution, by the cost of a pass. A pass of the interval
      // projections takes microseconds, so one execution can do many. A pass
      // of the alias classes visits every admitted class of the aliased
      // result, so each one hands control back.
      unsigned int pass_limit() const {
        return result_alias() ? 1 : interval_pass_limit;
      }
      // Project domains onto projections: the alias classes for a result
      // aliasing an operand, otherwise the interval projections of the
      // operation.
      void project(const Word::WordDomain domains[3], StatusFlags& flags,
                   LatticeBudget& budget,
                   WideProjection projections[3]) const {
        if (result_alias())
          project_alias(format, policy, operation, views, domains, flags,
                        projections);
        else if (operation == Arithmetic::Multiply)
          project_multiply(format, policy, views, domains, flags, budget,
                           projections);
        else if (operation == Arithmetic::Divide)
          project_divide(format, policy, views, domains, flags, budget,
                         projections);
        else
          project_wide(format, policy, operation, views, domains, flags,
                       budget, projections);
      }
      // Iterate the cancellation bound (sums only) and the projections on
      // domains and on statuses, the flag values the status admits, until
      // they reach a fixpoint, the inputs are assigned, or pass_limit()
      // passes that narrow are done. A pass that narrows nothing after a
      // lattice search was cut is repeated with exhaustive searches
      // (LatticeBudget), and the rest of the execution keeps them. Without a
      // status variable statuses admits every value and stays so.
      Iteration iterate(Word::WordDomain domains[3],
                        unsigned int& statuses) const {
        const unsigned int limit = pass_limit();
        LatticeLimit lattice = LatticeLimit::Bounded;
        for (unsigned int i = 0; i < limit;) {
          Word::WordDomain next[] = {domains[0], domains[1], domains[2]};
          if (!apply_cancellation_bound(format, policy, operation, views,
                                        next))
            return Iteration::Failed;
          // Positions sharing a view take the intersection of their bounds.
          WideProjection projections[3];
          for (int j = 0; j < 3; ++j)
            projections[j].include(views[j], next[j]);
          if (!narrow_domains(views, next, projections, 3))
            return Iteration::Failed;
          for (int j = 0; j < 3; ++j)
            projections[j] = WideProjection();
          LatticeBudget budget{lattice, 0};
          StatusFlags flags{statuses};
          project(next, flags, budget, projections);
          if (!narrow_domains(views, next, projections, 3))
            return Iteration::Failed;
          const unsigned int narrowed =
              has_status ? statuses & flags.produced : statuses;
          if (narrowed == 0)
            return Iteration::Failed;
          bool changed = narrowed != statuses;
          statuses = narrowed;
          for (int j = 0; j < 3; ++j) {
            changed = changed || !same_domain(next[j], domains[j]);
            domains[j] = next[j];
          }
          if (!changed && budget.cuts == 0)
            return Iteration::Converged;
          if (!changed) {
            lattice = LatticeLimit::Exhaustive;
            continue;
          }
          ++i;
          if (domains[0].interval.minimum == domains[0].interval.maximum &&
              domains[1].interval.minimum == domains[1].interval.maximum)
            return Iteration::Assigned;
        }
        return Iteration::Exhausted;
      }
      // Whether the step rule applies: a product or quotient of three
      // distinct views
      bool step_rule() const {
        return (operation == Arithmetic::Multiply ||
                operation == Arithmetic::Divide) &&
               views[0] != views[1] && views[0] != views[2] &&
               views[1] != views[2];
      }
      // Narrow state by one projection of the step rule with read: the
      // widened projection W for Read::Widened, and for Read::Hull the
      // projection H, the intersection of the interval projection (with its
      // constant-cost closed forms, the bounded lattice searches) and the
      // widened projection on the hull read. Both narrow the statuses to the
      // flag values of the pairs they keep. Return false on failure.
      bool project_step(Read read, StepState<3>& state) const {
        StatusFlags widened{state.statuses};
        WideProjection projections[3];
        project_widened(format, policy, operation, views, state.domains,
                        widened, read, projections);
        unsigned int produced = widened.produced;
        if (read == Read::Hull) {
          StatusFlags hull{state.statuses};
          WideProjection interval[3];
          LatticeBudget budget{LatticeLimit::Bounded, 0};
          if (operation == Arithmetic::Multiply)
            project_multiply(format, policy, views, state.domains, hull,
                             budget, interval);
          else
            project_divide(format, policy, views, state.domains, hull, budget,
                           interval);
          for (int i = 0; i < 3; ++i)
            projections[i].intersect(interval[i]);
          produced &= hull.produced;
        }
        if (!narrow_domains(views, state.domains, projections, 3))
          return false;
        if (has_status)
          state.statuses &= produced;
        return state.statuses != 0;
      }
      // Narrow domains and statuses by the step rule (run_step_rule)
      Iteration step(Word::WordDomain domains[3],
                     unsigned int& statuses) const {
        StepState<3> state{{domains[0], domains[1], domains[2]}, statuses};
        if (run_step_rule<3>(*this, &WideArithmetic::project_step, views,
                             state) == StepOutcome::Failed)
          return Iteration::Failed;
        for (int i = 0; i < 3; ++i)
          domains[i] = state.domains[i];
        statuses = state.statuses;
        return Iteration::Converged;
      }

    public:
      WideArithmetic(Home home, WordVar a, WordVar b, WordVar result,
                     IntVar* flags, const Policy& p, Arithmetic op)
          : Propagator(home), has_status(flags != nullptr),
            format(a.domain_type()), policy(p), operation(op) {
        views[0] = a;
        views[1] = b;
        views[2] = result;
        if (flags != nullptr)
          status = *flags;
        for (int i = 0; i < 3; ++i)
          if (first_occurrence(i))
            views[i].subscribe(home, *this, Word::PC_WORD_DOM);
        if (flags != nullptr)
          status.subscribe(home, *this, Int::PC_INT_DOM);
      }
      WideArithmetic(Space& home, WideArithmetic& other)
          : Propagator(home, other), has_status(other.has_status),
            format(other.format), policy(other.policy),
            operation(other.operation) {
        for (int i = 0; i < 3; ++i)
          views[i].update(home, other.views[i]);
        if (has_status)
          status.update(home, other.status);
      }
      Actor* copy(Space& home) override {
        return new (home) WideArithmetic(home, *this);
      }
      size_t dispose(Space& home) override {
        for (int i = 0; i < 3; ++i)
          if (first_occurrence(i))
            views[i].cancel(home, *this, Word::PC_WORD_DOM);
        if (has_status)
          status.cancel(home, *this, Int::PC_INT_DOM);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      /// Cost by path: assigned inputs only check the result. A result
      /// aliasing an unassigned operand visits the alias classes of the
      /// result, whose number grows with its width. The other shapes run the
      /// step rule (products and quotients of distinct words) or the
      /// interval projections with their lattice searches (sums and
      /// differences, also of a word with itself), each a bounded number of
      /// projections over the codes of the format.
      PropCost cost(const Space&, const ModEventDelta&) const override {
        if (views[0].assigned() && views[1].assigned())
          return PropCost::binary(PropCost::LO);
        if (result_alias())
          return PropCost::linear(PropCost::HI, views[2].width());
        return PropCost::ternary(PropCost::HI);
      }
      void reschedule(Space& home) override {
        for (int i = 0; i < 3; ++i)
          if (first_occurrence(i))
            views[i].reschedule(home, *this, Word::PC_WORD_DOM);
        if (has_status)
          status.reschedule(home, *this, Int::PC_INT_DOM);
      }

      /** \brief Propagate by projections on local domains
       *
       * A product or quotient of three distinct words takes the step rule
       * (run_step_rule in float-wide.hpp): one execution publishes a
       * fixpoint g(D) = D of the rule, which is a fixpoint of the widened
       * projection, with the status narrowed alongside, after a number of
       * projections bounded by the word width, the special classes and the
       * status values, not by the domain sizes, and reports ES_FIX. The
       * other shapes iterate as follows.
       *
       * One execution iterates the projections on local domains until they
       * reach a fixpoint, the inputs are assigned, or pass_limit() passes
       * are done. Every pass is sound, and whatever the passes narrowed is
       * published: no narrowing is discarded. The limit decides only when
       * the propagator hands control back to the kernel. Without a
       * fixpoint after pass_limit() passes it reports ES_NOFIX, so cheaper
       * propagators run before it continues. It reports ES_FIX only when
       * the views and the status hold a fixpoint of the projections.
       *
       * The operands are split into parts: zeros, subnormal and normal
       * numbers, infinities, signaling and quiet NaNs, per sign. A pair of
       * parts with a NaN, an infinity or, for products and quotients, a
       * zero has a fixed outcome; the pairs of numbers take the closed-form
       * reasoning below. Each pass also narrows the status to the flags of
       * the pairs it keeps.
       *
       * The jumps are closed-form: the cancellation bound bounds both
       * operands of a sum by the lattice of its result, the lattice
       * searches settle the free operand of a sum whose other operand has
       * one value (SumLattice), of a product with one fixed factor and of a
       * quotient with a fixed divisor against known bits, and the alias
       * classes keep or drop whole classes of an aliased result. What
       * remains is interval reasoning, which rounds every bound outward to
       * the lattice of the operand it narrows. For a sum of distinct words,
       * repeating it can still move bounds a few encodings per pass where
       * the supports form a lattice in the two operands; it converges
       * because every pass that narrows removes encodings.
       *
       * A result aliasing an unassigned operand takes the alias classes
       * alone, one pass per execution, and they do not creep. A pass keeps
       * a class of the aliased word with its supports or drops it whole,
       * and narrows the other operand to the hull of the supports of the
       * kept classes. The next pass therefore finds every kept class
       * supported again and no dropped class supported, and projects the
       * other operand onto the same supports, so it changes nothing unless
       * the closure of a domain fixed a further bit, at most once per bit,
       * or the cancellation bound of a sum moved a bound. The domain
       * projections of the alias classes do not read the status. So one
       * status() takes two executions of such a propagator, the second
       * confirming the fixpoint, plus one for each such change.
       *
       * The step rule does not creep: a single propagator of a product or
       * quotient of distinct words settles in a bounded number of steps, so
       * slow convergence can only come from several propagators sharing
       * words.
       */
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        if (views[0].assigned() && views[1].assigned())
          return enforce_assigned_inputs(home);
        Word::WordDomain domains[] = {wide_snapshot(views[0]),
                                      wide_snapshot(views[1]),
                                      wide_snapshot(views[2])};
        unsigned int statuses =
            has_status ? Word::Float::admitted_flags(status) : ~0U;
        const bool aliased = result_alias();
        const Iteration outcome = step_rule() ? step(domains, statuses)
                                              : iterate(domains, statuses);
        if (outcome == Iteration::Failed ||
            !publish_domains(home, views, domains))
          return ES_FAILED;
        if (has_status)
          GECODE_ME_CHECK(Word::Float::narrow_flags(home, status, statuses));
        if (views[0].assigned() && views[1].assigned())
          return enforce_assigned_inputs(home);
        // Assigning the aliased operand switches to the interval projections.
        if (outcome != Iteration::Converged || aliased != result_alias())
          return ES_NOFIX;
        // Publication closes like narrow_domains, so the views hold the
        // local fixpoint.
        for (int i = 0; i < 3; ++i)
          if (!same_domain(domains[i], wide_snapshot(views[i])))
            return ES_NOFIX;
        if (has_status && Word::Float::admitted_flags(status) != statuses)
          return ES_NOFIX;
        return ES_FIX;
      }
    };
  } // namespace
} // namespace Gecode

// STATISTICS: word-prop
