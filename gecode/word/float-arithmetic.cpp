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
#include <utility>

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    using Word::Float::admits_nonfinite;
    using Word::Float::divide_by_zero;
    using Word::Float::exact_product;
    using Word::Float::ExactDyadic;
    using Word::Float::finite_parts;
    using Word::Float::FinitePart;
    using Word::Float::floor_encoding;
    using Word::Float::include_real;
    using Word::Float::include_rounded;
    using Word::Float::inexact;
    using Word::Float::invalid;
    using Word::Float::overflow;
    using Word::Float::product_cell;
    using Word::Float::quotient_bounds;
    using Word::Float::round_coefficient;
    using Word::Float::round_dyadic;
    using Word::Float::RoundingCell;
    using Word::Float::underflow;
    using Word::Float::visit_alias_classes;
    using Word::Float::wide_intersect;
    using Word::Float::wide_snapshot;
    using Word::Float::WideFormat;
    using Word::Float::WideProjection;
    using Word::Float::WideResult;

    enum class Arithmetic { Add, Subtract, Multiply, Divide };

    struct TinyFormat {
      unsigned int fraction_bits;
      int sign_bit, maximum_code;
      uint64_t units_per_one;
      explicit TinyFormat(FloatWordFormat format)
          : fraction_bits(format == FWF_E2M1 ? 1 : 3),
            sign_bit(format == FWF_E2M1 ? 8 : 128),
            maximum_code(format == FWF_E2M1 ? 7 : 126),
            units_per_one(format == FWF_E2M1 ? 2 : 512) {}

      // Finite magnitudes in units of the minimum subnormal.
      uint64_t units(int magnitude) const {
        const unsigned int exponent =
            static_cast<unsigned int>(magnitude) >> fraction_bits;
        const unsigned int trailing =
            static_cast<unsigned int>(magnitude) & ((1U << fraction_bits) - 1);
        return exponent == 0 ? trailing
                             : uint64_t((1U << fraction_bits) + trailing)
                                   << (exponent - 1);
      }
    };
    using ArithmeticResult = Word::Float::TinyResult;

    bool rounds_up(uint64_t quotient, uint64_t remainder, uint64_t denominator,
                   bool negative, Policy::Rounding rounding) {
      if (remainder == 0)
        return false;
      switch (rounding) {
      case Policy::RNE:
        return remainder * 2 > denominator ||
               (remainder * 2 == denominator && (quotient & 1) != 0);
      case Policy::RTZ:
        return false;
      case Policy::RDN:
        return negative;
      case Policy::RUP:
        return !negative;
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
          format.sign_bit == 128 && policy.overflow == Policy::Format;
      const int magnitude = nan ? 127 : format.maximum_code;
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
      if (format.sign_bit == 128 && (a_magnitude == 127 || b_magnitude == 127))
        return {policy.nan == Policy::Canonical ? 127
                : a_magnitude == 127            ? a
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
            return {format.sign_bit == 8 ? 7 : 127, invalid};
          return fallback(format, policy, a_negative != b_negative,
                          divide_by_zero);
        }
        return round_finite(format, policy, a_units, b_units,
                            a_negative != b_negative);
      }
      GECODE_NEVER;
      return {0, 0};
    }

    class TinyArithmetic : public Propagator {
      Int::IntView views[4];
      int columns[4];
      int column_count, view_count;
      const TinyFormat format;
      const Policy policy;
      const Arithmetic operation;

    public:
      TinyArithmetic(Home home, const IntVarArgs& variables, FloatWordFormat f,
                     const Policy& p, Arithmetic op)
          : Propagator(home), column_count(variables.size()), view_count(0),
            format(f), policy(p), operation(op) {
        for (int i = 0; i < column_count; ++i) {
          Int::IntView view(variables[i]);
          int j = 0;
          while (j < view_count && views[j] != view)
            ++j;
          columns[i] = j;
          if (j == view_count) {
            views[view_count++] = view;
          }
        }
        // Subscribing can call cost(), so initialize every column first.
        for (int i = 0; i < view_count; ++i)
          views[i].subscribe(home, *this, Int::PC_INT_DOM);
      }
      TinyArithmetic(Space& home, TinyArithmetic& other)
          : Propagator(home, other), column_count(other.column_count),
            view_count(other.view_count), format(other.format),
            policy(other.policy), operation(other.operation) {
        for (int i = 0; i < column_count; ++i)
          columns[i] = other.columns[i];
        for (int i = 0; i < view_count; ++i)
          views[i].update(home, other.views[i]);
      }
      virtual Actor* copy(Space& home) {
        return new (home) TinyArithmetic(home, *this);
      }
      virtual size_t dispose(Space& home) {
        for (int i = 0; i < view_count; ++i)
          views[i].cancel(home, *this, Int::PC_INT_DOM);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::HI, views[columns[0]].size() +
                                                     views[columns[1]].size());
      }
      virtual void reschedule(Space& home) {
        for (int i = 0; i < view_count; ++i)
          views[i].reschedule(home, *this, Int::PC_INT_DOM);
      }
      virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        bool supports[4][256] = {};
        bool found = false;
        for (Int::ViewValues<Int::IntView> a(views[columns[0]]); a(); ++a)
          for (Int::ViewValues<Int::IntView> b(views[columns[1]]); b(); ++b) {
            if (columns[0] == columns[1] && a.val() != b.val())
              continue;
            const ArithmeticResult result =
                evaluate(format, policy, operation, a.val(), b.val());
            const int tuple[] = {a.val(), b.val(), result.code, result.status};
            bool allowed = true;
            for (int i = 0; i < column_count && allowed; ++i) {
              allowed = views[columns[i]].in(tuple[i]);
              for (int j = 0; j < i; ++j)
                if (columns[i] == columns[j] && tuple[i] != tuple[j])
                  allowed = false;
            }
            if (!allowed)
              continue;
            found = true;
            for (int i = 0; i < column_count; ++i)
              supports[columns[i]][tuple[i]] = true;
          }
        if (!found)
          return ES_FAILED;
        bool assigned = true;
        for (int i = 0; i < view_count; ++i) {
          int supported[256], count = 0;
          for (Int::ViewValues<Int::IntView> value(views[i]); value(); ++value)
            if (supports[i][value.val()])
              supported[count++] = value.val();
          Iter::Values::Array values(supported, count);
          GECODE_ME_CHECK(views[i].inter_v(home, values, false));
          assigned &= views[i].assigned();
        }
        // Every retained tuple supports all its coordinates, including aliases.
        // Their simultaneous projection therefore reaches this actor's
        // fixpoint.
        return assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
    };

    using Integer = Word::Arithmetic::Global::Integer;
    using Word::WordView;

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
      const Integer step = Integer::power_two(shift);
      Integer quotient, remainder;
      Integer::divide(magnitude, step, quotient, remainder);
      bool increment = false;
      if (!remainder.zero()) {
        switch (p.rounding) {
        case Policy::RNE:
          increment = remainder + remainder > step ||
                      (remainder + remainder == step && quotient.bit(0));
          break;
        case Policy::RTZ:
          break;
        case Policy::RDN:
          increment = negative;
          break;
        case Policy::RUP:
          increment = !negative;
          break;
        }
      }
      const Integer rounded = (quotient + Integer(increment ? 1 : 0)) * step;
      if (rounded > f.magnitude(f.maximum)) {
        const bool to_infinity = p.overflow == Policy::Format &&
                                 (p.rounding == Policy::RNE ||
                                  (p.rounding == Policy::RUP && !negative) ||
                                  (p.rounding == Policy::RDN && negative));
        return {(negative ? f.sign : 0) |
                    (to_infinity ? f.infinity : f.maximum),
                overflow | inexact};
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

    WideResult evaluate_multiply(const WideFormat& f, const Policy& p,
                                 WordValue a, WordValue b) {
      const bool a_nan = f.nan(a), b_nan = f.nan(b);
      const int flags = f.signaling(a) || f.signaling(b) ? invalid : 0;
      if (a_nan || b_nan) {
        WordValue selected = a_nan ? a : b;
        if (p.nan == Policy::SignalFirst && flags)
          selected = f.signaling(a) ? a : b;
        return {p.nan == Policy::Canonical ? f.infinity | f.quiet
                                           : selected | f.quiet,
                flags};
      }
      if (p.input_subnormals == Policy::SignedZero) {
        if ((a & (f.sign - 1)) < f.normal)
          a &= f.sign;
        if ((b & (f.sign - 1)) < f.normal)
          b &= f.sign;
      }
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
                 ? compare(coefficient *
                               Integer::power_two(
                                   static_cast<unsigned int>(exponent)),
                           other)
                 : compare(coefficient,
                           other * Integer::power_two(
                                       static_cast<unsigned int>(-exponent)));
    }

    Integer round_quotient_grid(Integer numerator, Integer denominator,
                                int exponent, bool negative,
                                Policy::Rounding rounding) {
      if (exponent < 0)
        numerator = numerator *
                    Integer::power_two(static_cast<unsigned int>(-exponent));
      else
        denominator = denominator *
                      Integer::power_two(static_cast<unsigned int>(exponent));
      Integer quotient, remainder;
      Integer::divide(numerator, denominator, quotient, remainder);
      bool increment = false;
      if (!remainder.zero()) {
        switch (rounding) {
        case Policy::RNE:
          increment = remainder + remainder > denominator ||
                      (remainder + remainder == denominator && quotient.bit(0));
          break;
        case Policy::RTZ:
          break;
        case Policy::RDN:
          increment = negative;
          break;
        case Policy::RUP:
          increment = !negative;
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
        const bool to_infinity = p.overflow == Policy::Format &&
                                 (p.rounding == Policy::RNE ||
                                  (p.rounding == Policy::RUP && !negative) ||
                                  (p.rounding == Policy::RDN && negative));
        return {(negative ? f.sign : 0) |
                    (to_infinity ? f.infinity : f.maximum),
                overflow | inexact};
      }
      const int destination_grid = std::max(precision_grid, 1 - source_scale);
      const Integer rounded =
          round_quotient_grid(numerator, denominator, destination_grid,
                              negative, p.rounding) *
          Integer::power_two(
              static_cast<unsigned int>(destination_grid + source_scale));
      WordValue code = floor_encoding(f, rounded);
      const Integer scaled_numerator =
          numerator *
          Integer::power_two(static_cast<unsigned int>(source_scale));
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
      const bool a_nan = f.nan(a), b_nan = f.nan(b);
      const int flags = f.signaling(a) || f.signaling(b) ? invalid : 0;
      if (a_nan || b_nan) {
        WordValue selected = a_nan ? a : b;
        if (p.nan == Policy::SignalFirst && flags)
          selected = f.signaling(a) ? a : b;
        return {p.nan == Policy::Canonical ? f.infinity | f.quiet
                                           : selected | f.quiet,
                flags};
      }
      if (p.input_subnormals == Policy::SignedZero) {
        if ((a & (f.sign - 1)) < f.normal)
          a &= f.sign;
        if ((b & (f.sign - 1)) < f.normal)
          b &= f.sign;
      }
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
      const bool a_nan = f.nan(a), b_nan = f.nan(b);
      const int flags = f.signaling(a) || f.signaling(b) ? invalid : 0;
      if (a_nan || b_nan) {
        WordValue selected = a_nan ? a : b;
        if (p.nan == Policy::SignalFirst && flags)
          selected = f.signaling(a) ? a : b;
        return {p.nan == Policy::Canonical ? f.infinity | f.quiet
                                           : selected | f.quiet,
                flags};
      }
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

    RoundingCell rounding_cell(const WideFormat& f, const Policy& p,
                               WordValue code) {
      const WordValue magnitude = code & (f.sign - 1);
      const bool negative = (code & f.sign) != 0;
      assert(magnitude > 0 && magnitude < f.maximum);
      const Integer center = f.value(code);
      const Integer previous =
          negative ? -f.magnitude(magnitude + 1) : f.magnitude(magnitude - 1);
      const Integer next =
          negative ? -f.magnitude(magnitude - 1) : f.magnitude(magnitude + 1);
      if (p.rounding == Policy::RNE) {
        const Integer odd(magnitude & 1);
        return {(previous + center) / Integer(2) + odd,
                (center + next) / Integer(2) - odd};
      }
      const bool upward =
          p.rounding == Policy::RUP || (p.rounding == Policy::RTZ && negative);
      return upward ? RoundingCell{previous + Integer(1), center}
                    : RoundingCell{center, next - Integer(1)};
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

    void project_wide(const WideFormat& f, const Policy& p, Arithmetic op,
                      const WordView views[3],
                      const Word::WordDomain domains[3],
                      const Int::IntView* status,
                      WideProjection projections[3]) {
      // The caller excludes nonfinite operands. Repeated subtraction is exact
      // cancellation, including flushed inputs and the directed zero sign.
      if (views[0] == views[1] && op == Arithmetic::Subtract) {
        const auto zero =
            round_wide(f, p, Integer(), p.rounding == Policy::RDN);
        if (status && !status->in(zero.status))
          return;
        projections[2].include(views[2], domains[2], zero.code, zero.code);
        projections[0].include(views[0], domains[0]);
        projections[1].include(views[1], domains[1]);
        return;
      }
      const auto as = finite_parts(f, p, views[0], domains[0], true);
      const auto bs = finite_parts(f, p, views[1], domains[1], true);
      for (const auto& a : as)
        for (const auto& b : bs) {
          const Integer low =
              op == Arithmetic::Add ? a.low + b.low : a.low - b.high;
          const Integer high =
              op == Arithmetic::Add ? a.high + b.high : a.high - b.low;
          include_rounded(f, projections[2], views[2], domains[2],
                          round_wide(f, p, low).code,
                          round_wide(f, p, high).code);
        }
      if (retain_inverse_operands(f, views[2], domains[2])) {
        projections[0].include(views[0], domains[0]);
        projections[1].include(views[1], domains[1]);
        return;
      }
      const auto zs = finite_parts(f, p, views[2], domains[2], false);
      for (const auto& z : zs) {
        const WordValue first =
            Word::rank(views[2].domain_type(), views[2].width(),
                       z.domain.interval.minimum);
        const WordValue last =
            Word::rank(views[2].domain_type(), views[2].width(),
                       z.domain.interval.maximum);
        const Integer low = rounding_cell(f, p, z.negative ? last : first).low;
        const Integer high =
            rounding_cell(f, p, z.negative ? first : last).high;
        if (views[0] == views[1]) {
          using Word::Arithmetic::Global::ceil_div;
          using Word::Arithmetic::Global::floor_div;
          for (const auto& a : as)
            include_real(f, projections[0], views[0], a,
                         ceil_div(low, Integer(2)),
                         floor_div(high, Integer(2)));
          projections[1] = projections[0];
          continue;
        }
        for (const auto& a : as)
          for (const auto& b : bs) {
            include_real(f, projections[0], views[0], a,
                         op == Arithmetic::Add ? low - b.high : low + b.low,
                         op == Arithmetic::Add ? high - b.low : high + b.high);
            include_real(f, projections[1], views[1], b,
                         op == Arithmetic::Add ? low - a.high : a.low - high,
                         op == Arithmetic::Add ? high - a.low : a.high - low);
          }
      }
    }

    enum class ProductClass { Zero, Finite, Infinity, QuietNaN, SignalingNaN };
    struct ProductPart {
      FinitePart finite;
      ProductClass kind;
      bool nan() const {
        return kind == ProductClass::QuietNaN ||
               kind == ProductClass::SignalingNaN;
      }
    };

    // Six finite parts and three special parts per sign.
    using ProductParts = Word::Float::Parts<ProductPart, 12>;
    ProductParts product_parts(const WideFormat& f, const Policy& p,
                               WordView view, const Word::WordDomain& domain) {
      ProductParts parts;
      for (const auto& part : finite_parts(f, p, view, domain, true))
        parts.push_back({part, part.low.zero() && part.high.zero()
                                   ? ProductClass::Zero
                                   : ProductClass::Finite});
      for (WordValue sign : {WordValue(0), f.sign}) {
        const struct {
          WordValue first, last;
          ProductClass kind;
        } ranges[] = {
            {f.infinity, f.infinity, ProductClass::Infinity},
            {f.infinity + 1, f.infinity + f.quiet - 1,
             ProductClass::SignalingNaN},
            {f.infinity + f.quiet, f.sign - 1, ProductClass::QuietNaN}};
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

    // Exceptional branches have a fixed status. NaN payloads are deliberately
    // conservative, but selection order, signaling status, sign and quietness
    // are known from each pair of classes.
    void project_special_product(const WideFormat& f, const Policy& p,
                                 const ProductPart& a, const ProductPart& b,
                                 const WordView views[3],
                                 const Word::WordDomain domains[3],
                                 const Int::IntView* status,
                                 WideProjection projections[3]) {
      WideProjection result;
      WordValue code = 0;
      int flags = 0;
      if (a.nan() || b.nan()) {
        flags = a.kind == ProductClass::SignalingNaN ||
                        b.kind == ProductClass::SignalingNaN
                    ? invalid
                    : 0;
        if (p.nan != Policy::Canonical) {
          const ProductPart& selected =
              p.nan == Policy::SignalFirst && flags
                  ? (a.kind == ProductClass::SignalingNaN ? a : b)
                  : (a.nan() ? a : b);
          const WordValue sign = selected.finite.negative ? f.sign : 0;
          result.include(views[2], domains[2], sign | f.infinity | f.quiet,
                         sign | (f.sign - 1));
        } else {
          code = f.infinity | f.quiet;
          result.include(views[2], domains[2], code, code);
        }
      } else if ((a.kind == ProductClass::Zero &&
                  b.kind == ProductClass::Infinity) ||
                 (b.kind == ProductClass::Zero &&
                  a.kind == ProductClass::Infinity)) {
        flags = invalid;
        code = f.infinity | f.quiet;
        result.include(views[2], domains[2], code, code);
      } else {
        const bool infinity = a.kind == ProductClass::Infinity ||
                              b.kind == ProductClass::Infinity;
        const bool negative = a.finite.negative != b.finite.negative;
        code =
            (infinity ? f.infinity : 0) |
            (negative && (infinity || p.zeros == Policy::Signed) ? f.sign : 0);
        result.include(views[2], domains[2], code, code);
      }
      if (!result.present || (status && !status->in(flags)))
        return;
      projections[0].include(views[0], a.finite.domain);
      projections[1].include(views[1], b.finite.domain);
      projections[2].include(views[2], {result.cube, result.interval});
    }

    void project_finite_product(const WideFormat& f, const Policy& p,
                                const FinitePart& a, const FinitePart& b,
                                const WordView views[3],
                                const Word::WordDomain domains[3],
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
      const Integer scale = Integer::power_two(source_scale);
      for (const auto& z :
           finite_parts(f, p, views[2], candidate.domain, false)) {
        const WordValue first =
            Word::rank(views[2].domain_type(), views[2].width(),
                       z.domain.interval.minimum);
        const WordValue last =
            Word::rank(views[2].domain_type(), views[2].width(),
                       z.domain.interval.maximum);
        const RoundingCell cell{
            std::max(low,
                     product_cell(f, p, z.negative ? last : first, scale).low),
            std::min(
                high,
                product_cell(f, p, z.negative ? first : last, scale).high)};
        if (cell.low > cell.high)
          continue;
        const RoundingCell left = quotient_bounds(cell, b);
        const RoundingCell right = quotient_bounds(cell, a);
        WideProjection sources[2];
        include_real(f, sources[0], views[0], a, left.low, left.high);
        include_real(f, sources[1], views[1], b, right.low, right.high);
        if (!sources[0].present || !sources[1].present)
          continue;
        for (int i = 0; i < 2; ++i)
          projections[i].include(views[i],
                                 {sources[i].cube, sources[i].interval});
        projections[2].include(views[2], z.domain);
      }
    }

    void project_multiply(const WideFormat& f, const Policy& p,
                          const WordView views[3],
                          const Word::WordDomain domains[3],
                          const Int::IntView* status,
                          WideProjection projections[3]) {
      const auto as = product_parts(f, p, views[0], domains[0]);
      const auto bs = product_parts(f, p, views[1], domains[1]);
      for (const auto& a : as)
        for (const auto& b : bs) {
          if (a.kind != ProductClass::Finite ||
              b.kind != ProductClass::Finite) {
            project_special_product(f, p, a, b, views, domains, status,
                                    projections);
          } else if (!status || status->in(0) || status->in(inexact) ||
                     status->in(underflow | inexact) ||
                     status->in(overflow | inexact)) {
            project_finite_product(f, p, a.finite, b.finite, views, domains,
                                   projections);
          }
        }
    }

    // Every exceptional division class pair has a fixed result class/status.
    // Payload projection may retain unsupported encodings; assignment checks
    // the original operands.
    void project_special_quotient(const WideFormat& f, const Policy& p,
                                  const ProductPart& a, const ProductPart& b,
                                  const WordView views[3],
                                  const Word::WordDomain domains[3],
                                  const Int::IntView* status,
                                  WideProjection projections[3]) {
      WideProjection result;
      int flags = 0;
      if (a.nan() || b.nan()) {
        flags = a.kind == ProductClass::SignalingNaN ||
                        b.kind == ProductClass::SignalingNaN
                    ? invalid
                    : 0;
        if (p.nan != Policy::Canonical) {
          const ProductPart& selected =
              p.nan == Policy::SignalFirst && flags
                  ? (a.kind == ProductClass::SignalingNaN ? a : b)
                  : (a.nan() ? a : b);
          const WordValue sign = selected.finite.negative ? f.sign : 0;
          result.include(views[2], domains[2], sign | f.infinity | f.quiet,
                         sign | (f.sign - 1));
        } else {
          const WordValue code = f.infinity | f.quiet;
          result.include(views[2], domains[2], code, code);
        }
      } else if ((a.kind == ProductClass::Zero &&
                  b.kind == ProductClass::Zero) ||
                 (a.kind == ProductClass::Infinity &&
                  b.kind == ProductClass::Infinity)) {
        flags = invalid;
        const WordValue code = f.infinity | f.quiet;
        result.include(views[2], domains[2], code, code);
      } else {
        const bool infinity =
            a.kind == ProductClass::Infinity || b.kind == ProductClass::Zero;
        if (b.kind == ProductClass::Zero && a.kind == ProductClass::Finite)
          flags = divide_by_zero;
        const bool negative = a.finite.negative != b.finite.negative;
        const WordValue code =
            (infinity ? f.infinity : 0) |
            (negative && (infinity || p.zeros == Policy::Signed) ? f.sign : 0);
        result.include(views[2], domains[2], code, code);
      }
      if (!result.present || (status && !status->in(flags)))
        return;
      projections[0].include(views[0], a.finite.domain);
      projections[1].include(views[1], b.finite.domain);
      projections[2].include(views[2], {result.cube, result.interval});
    }

    // Closed enclosures in half-subnormal units. Quotients need not lie on
    // this integer lattice, so no one-unit exclusion of strict endpoints is
    // sound here. Exact tie and signed-zero checks remain in evaluate_divide.
    RoundingCell division_cell(const WideFormat& f, const Policy& p,
                               WordValue code) {
      const WordValue magnitude = code & (f.sign - 1);
      if (magnitude == 0) {
        const Integer radius = f.magnitude(
            p.output_subnormals == Policy::SignedZero ? f.normal : 1);
        return {-radius, radius};
      }
      assert(magnitude < f.maximum);
      const bool negative = (code & f.sign) != 0;
      const Integer center = f.value(code);
      const Integer previous =
          negative ? -f.magnitude(magnitude + 1) : f.magnitude(magnitude - 1);
      const Integer next =
          negative ? -f.magnitude(magnitude - 1) : f.magnitude(magnitude + 1);
      if (p.rounding == Policy::RNE)
        return {(previous + center) / Integer(2), (center + next) / Integer(2)};
      const bool upward =
          p.rounding == Policy::RUP || (p.rounding == Policy::RTZ && negative);
      return upward ? RoundingCell{previous, center}
                    : RoundingCell{center, next};
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

    RoundingCell division_numerator_bounds(const RoundingCell& cell,
                                           const FinitePart& denominator,
                                           const Integer& scale) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      Integer low = cell.low * denominator.low, high = low;
      for (const Integer& target : {cell.low, cell.high})
        for (const Integer& divisor : {denominator.low, denominator.high}) {
          const Integer product = target * divisor;
          low = std::min(low, product);
          high = std::max(high, product);
        }
      return {ceil_div(low, scale), floor_div(high, scale)};
    }

    RoundingCell division_denominator_bounds(const RoundingCell& cell,
                                             const FinitePart& numerator,
                                             const Integer& scale) {
      assert(cell.low > Integer() || cell.high < Integer());
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      Integer low = ceil_div(numerator.low * scale, cell.low);
      Integer high = floor_div(numerator.low * scale, cell.low);
      for (const Integer& dividend : {numerator.low, numerator.high})
        for (const Integer& target : {cell.low, cell.high}) {
          low = std::min(low, ceil_div(dividend * scale, target));
          high = std::max(high, floor_div(dividend * scale, target));
        }
      return {low, high};
    }

    void project_finite_quotient(const WideFormat& f, const Policy& p,
                                 const FinitePart& a, const FinitePart& b,
                                 const WordView views[3],
                                 const Word::WordDomain domains[3],
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
      const Integer scale = Integer::power_two(127 + f.fraction_bits);
      for (const auto& z :
           finite_parts(f, p, views[2], candidate.domain, false)) {
        const WordValue first =
            Word::rank(views[2].domain_type(), views[2].width(),
                       z.domain.interval.minimum);
        const WordValue last =
            Word::rank(views[2].domain_type(), views[2].width(),
                       z.domain.interval.maximum);
        const RoundingCell cell{
            division_cell(f, p, z.negative ? last : first).low,
            division_cell(f, p, z.negative ? first : last).high};
        const RoundingCell numerator =
            division_numerator_bounds(cell, b, scale);
        WideProjection sources[2];
        include_real(f, sources[0], views[0], a, numerator.low, numerator.high);
        if (cell.low <= Integer() && cell.high >= Integer()) {
          sources[1].include(views[1], b.domain);
        } else {
          const RoundingCell denominator =
              division_denominator_bounds(cell, a, scale);
          include_real(f, sources[1], views[1], b, denominator.low,
                       denominator.high);
        }
        if (!sources[0].present || !sources[1].present)
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
                        const Int::IntView* status,
                        WideProjection projections[3]) {
      const auto as = product_parts(f, p, views[0], domains[0]);
      if (views[0] == views[1]) {
        for (const auto& a : as) {
          if (a.kind != ProductClass::Finite) {
            project_special_quotient(f, p, a, a, views, domains, status,
                                     projections);
          } else if (!status || status->in(0)) {
            // Every effective finite nonzero x has the exact quotient x/x=1.
            const WordValue one = 127 * f.normal;
            projections[2].include(views[2], domains[2], one, one);
            projections[0].include(views[0], a.finite.domain);
            projections[1].include(views[1], a.finite.domain);
          }
        }
        return;
      }
      const auto bs = product_parts(f, p, views[1], domains[1]);
      for (const auto& a : as)
        for (const auto& b : bs) {
          if (a.kind != ProductClass::Finite ||
              b.kind != ProductClass::Finite) {
            project_special_quotient(f, p, a, b, views, domains, status,
                                     projections);
          } else if (!status || status->in(0) || status->in(inexact) ||
                     status->in(underflow | inexact) ||
                     status->in(overflow | inexact)) {
            project_finite_quotient(f, p, a.finite, b.finite, views, domains,
                                    projections);
          }
        }
    }

    // A fixed exponent and trailing-significand-zero count fixes C(z).
    // Intersect the class with the actual cube before taking its endpoints:
    // an interior power of two can have a much larger C than either endpoint.
    bool apply_cancellation_bound(const WideFormat& f, const Policy& p,
                                  Arithmetic op, Space& home,
                                  const WordView views[3]) {
      if ((op != Arithmetic::Add && op != Arithmetic::Subtract) ||
          p.input_subnormals != Policy::Preserve ||
          p.output_subnormals != Policy::Preserve ||
          p.overflow != Policy::Format)
        return true;
      const auto result = wide_snapshot(views[2]);
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
            const WordValue maximum =
                Word::rank(views[2].domain_type(), views[2].width(),
                           admitted.domain.interval.maximum);
            const Integer span =
                Integer(2 * f.normal - 1) *
                Integer::power_two(std::max(1U, exponent) + trailing);
            const Integer z = f.value(maximum);
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
      for (int column = 0; column < 2; ++column) {
        const auto domain = wide_snapshot(views[column]);
        for (const auto& part : finite_parts(f, p, views[column], domain, true))
          include_real(f, projections[column], views[column], part,
                       column == 1 && op == Arithmetic::Subtract ? -high : low,
                       column == 1 && op == Arithmetic::Subtract ? -low : high);
      }
      return publish_projections(home, views, projections, 2);
    }

    // Result/input aliases need their correlated rounding cell. Treating the
    // input and result as independent intervals can move an endpoint by one
    // encoding per invocation. Within each class below cell(x)-x is constant;
    // the rational residuals have their extrema at the admitted endpoints.
    RoundingCell alias_operand_bounds(const WideFormat& f, const Policy& p,
                                      Arithmetic op, int alias,
                                      const FinitePart& part, WordValue first,
                                      WordValue last) {
      const Integer scale = Integer::power_two(127 + f.fraction_bits);
      bool present = false;
      RoundingCell bounds;
      for (WordValue code : {first, last}) {
        const Integer x = f.value(code);
        RoundingCell cell;
        if (op == Arithmetic::Multiply) {
          cell = quotient_bounds(product_cell(f, p, code, scale),
                                 {part.domain, x, x, part.negative, false});
        } else if (op == Arithmetic::Divide && alias == 0) {
          // x / b rounds to x, so b lies in x / cell(x).
          const auto target = division_cell(f, p, code);
          cell = target.low <= Integer() && target.high >= Integer()
                     ? RoundingCell{-f.magnitude(f.maximum),
                                    f.magnitude(f.maximum)}
                     : division_denominator_bounds(
                           target, {part.domain, x, x, part.negative, false},
                           scale);
        } else if (op == Arithmetic::Divide) {
          // a / x rounds to x, so a lies in x * cell(x).
          cell = division_numerator_bounds(
              division_cell(f, p, code),
              {part.domain, x, x, part.negative, false}, scale);
        } else {
          const auto target = rounding_cell(f, p, code);
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

    void project_alias(const WideFormat& f, const Policy& p, Arithmetic op,
                       const WordView views[3],
                       const Word::WordDomain domains[3],
                       WideProjection projections[3]) {
      const int alias = views[0] == views[2] ? 0 : 1;
      const int other = 1 - alias;
      const auto others =
          finite_parts(f, p, views[other], domains[other], true);
      visit_alias_classes(
          f, p, views[alias], domains[alias],
          [&](const FinitePart& part, WordValue first, WordValue last) {
            const auto bounds =
                alias_operand_bounds(f, p, op, alias, part, first, last);
            WideProjection support;
            if (views[other] == views[alias]) {
              include_real(f, support, views[other], part, bounds.low,
                           bounds.high);
            } else {
              for (const auto& operand : others)
                include_real(f, support, views[other], operand, bounds.low,
                             bounds.high);
            }
            if (!support.present)
              return;
            // Retaining a whole admitted class is conservative. It avoids
            // feeding an independent forward bound back through the alias.
            projections[alias].include(views[alias], part.domain);
            projections[2].include(views[2], part.domain);
            if (views[other] != views[alias])
              projections[other].include(views[other],
                                         {support.cube, support.interval});
          },
          [&](const Word::WordDomain& tail) {
            // Retain tails locally; regular classes still get filtered.
            projections[alias].include(views[alias], tail);
            projections[2].include(views[2], tail);
            if (views[other] != views[alias])
              projections[other].include(views[other], domains[other]);
          });
      if (views[other] == views[alias])
        projections[other] = projections[alias];
    }

    class WideArithmetic : public Propagator {
      WordView views[3];
      Int::IntView status;
      const bool has_status;
      const WideFormat format;
      const Policy policy;
      const Arithmetic operation;

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

    public:
      WideArithmetic(Home home, FloatWordVar a, FloatWordVar b,
                     FloatWordVar result, IntVar* flags, const Policy& p,
                     Arithmetic op)
          : Propagator(home), has_status(flags != nullptr),
            format(a.format()), policy(p), operation(op) {
        views[0] = a.bits();
        views[1] = b.bits();
        views[2] = result.bits();
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
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::ternary(PropCost::HI);
      }
      void reschedule(Space& home) override {
        for (int i = 0; i < 3; ++i)
          if (first_occurrence(i))
            views[i].reschedule(home, *this, Word::PC_WORD_DOM);
        if (has_status)
          status.reschedule(home, *this, Int::PC_INT_DOM);
      }
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        if (views[0].assigned() && views[1].assigned())
          return enforce_assigned_inputs(home);
        const Word::WordDomain before[] = {wide_snapshot(views[0]),
                                           wide_snapshot(views[1]),
                                           wide_snapshot(views[2])};
        // Exceptional operands retain the posting until assignment. No
        // algebraic alias rewrite changes NaN order or signed-zero behavior.
        if ((operation == Arithmetic::Add ||
             operation == Arithmetic::Subtract) &&
            (admits_nonfinite(format, views[0], before[0]) ||
             admits_nonfinite(format, views[1], before[1])))
          return ES_FIX;
        if (!apply_cancellation_bound(format, policy, operation, home, views))
          return ES_FAILED;
        const Word::WordDomain current[] = {wide_snapshot(views[0]),
                                            wide_snapshot(views[1]),
                                            wide_snapshot(views[2])};
        WideProjection projections[3];
        if (views[0] == views[2] || views[1] == views[2])
          project_alias(format, policy, operation, views, current,
                        projections);
        else if (operation == Arithmetic::Multiply)
          project_multiply(format, policy, views, current,
                           has_status ? &status : nullptr, projections);
        else if (operation == Arithmetic::Divide)
          project_divide(format, policy, views, current,
                         has_status ? &status : nullptr, projections);
        else
          project_wide(format, policy, operation, views, current,
                       has_status ? &status : nullptr, projections);
        if (!publish_projections(home, views, projections, 3))
          return ES_FAILED;
        if (views[0].assigned() && views[1].assigned())
          return enforce_assigned_inputs(home);
        for (int i = 0; i < 3; ++i) {
          const auto now = wide_snapshot(views[i]);
          if (now.cube.lo != before[i].cube.lo ||
              now.cube.hi != before[i].cube.hi ||
              now.interval.minimum != before[i].interval.minimum ||
              now.interval.maximum != before[i].interval.maximum)
            return ES_NOFIX;
        }
        return ES_FIX;
      }
    };

    void post_arithmetic(Home home, const Policy& policy, FloatWordVar a,
                         FloatWordVar b, FloatWordVar result, IntVar* status,
                         Arithmetic operation, const char* name) {
      Word::Float::check(policy, name);
      if (a.format() != b.format() || a.format() != result.format())
        throw Word::FormatMismatch(name);
      if (a.format() == FWF_BF16 || a.format() == FWF_FP32) {
        GECODE_POST;
        if (status != nullptr) {
          Int::IntView flags(*status);
          GECODE_ME_FAIL(flags.gq(home, 0));
          GECODE_ME_FAIL(flags.lq(home, 31));
        }
        (void)new (home)
            WideArithmetic(home, a, b, result, status, policy, operation);
        return;
      }
      if (a.format() != FWF_E2M1 && a.format() != FWF_E4M3FN)
        throw Word::OutOfLimits(name);
      IntVarArgs variables(status == nullptr ? 3 : 4);
      variables[0] = a.encoding();
      variables[1] = b.encoding();
      variables[2] = result.encoding();
      if (status != nullptr)
        variables[3] = *status;
      GECODE_POST;
      if (status != nullptr) {
        Int::IntView flags(*status);
        GECODE_ME_FAIL(flags.gq(home, 0));
        GECODE_ME_FAIL(flags.lq(home, 31));
      }
      (void)new (home)
          TinyArithmetic(home, variables, a.format(), policy, operation);
    }
  } // namespace

  namespace Word {
    namespace Float {
      TinyResult round_tiny(FloatWordFormat format,
                            const FloatWordPolicy& policy, uint64_t numerator,
                            uint64_t denominator, bool negative) {
        return round_finite(TinyFormat(format), policy, numerator,
                            denominator, negative);
      }
    } // namespace Float
  } // namespace Word

  void fp_add(Home home, const FloatWordPolicy& policy, FloatWordVar a,
              FloatWordVar b, FloatWordVar result) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Add,
                    "fp_add");
  }

  void fp_add(Home home, const FloatWordPolicy& policy, FloatWordVar a,
              FloatWordVar b, FloatWordVar result, IntVar status) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Add,
                    "fp_add");
  }

  void fp_subtract(Home home, const FloatWordPolicy& policy, FloatWordVar a,
                   FloatWordVar b, FloatWordVar result) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Subtract,
                    "fp_subtract");
  }

  void fp_subtract(Home home, const FloatWordPolicy& policy, FloatWordVar a,
                   FloatWordVar b, FloatWordVar result, IntVar status) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Subtract,
                    "fp_subtract");
  }

  void fp_multiply(Home home, const FloatWordPolicy& policy, FloatWordVar a,
                   FloatWordVar b, FloatWordVar result) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Multiply,
                    "fp_multiply");
  }

  void fp_multiply(Home home, const FloatWordPolicy& policy, FloatWordVar a,
                   FloatWordVar b, FloatWordVar result, IntVar status) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Multiply,
                    "fp_multiply");
  }

  void fp_divide(Home home, const FloatWordPolicy& policy, FloatWordVar a,
                 FloatWordVar b, FloatWordVar result) {
    post_arithmetic(home, policy, a, b, result, nullptr, Arithmetic::Divide,
                    "fp_divide");
  }

  void fp_divide(Home home, const FloatWordPolicy& policy, FloatWordVar a,
                 FloatWordVar b, FloatWordVar result, IntVar status) {
    post_arithmetic(home, policy, a, b, result, &status, Arithmetic::Divide,
                    "fp_divide");
  }
} // namespace Gecode

// STATISTICS: word-post
