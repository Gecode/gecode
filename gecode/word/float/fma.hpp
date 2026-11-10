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

#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float-wide.hpp>
#include <gecode/word/float.hpp>

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    using Word::Float::TinyResult;

    using Word::Float::TinyFormat;

    TinyResult evaluate_fma(const TinyFormat& format, const Policy& policy,
                            int a, int b, int c) {
      const int sign = format.sign_bit;
      int magnitudes[] = {a & (sign - 1), b & (sign - 1), c & (sign - 1)};
      const int operands[] = {a, b, c};
      // Both E4M3FN NaNs are quiet; select from original operand order.
      for (int operand : operands)
        if (format.is_nan(static_cast<WordValue>(operand)))
          return {policy.nan == Policy::Canonical ? format.canonical_nan()
                                                  : operand,
                  0};
      if (policy.input_subnormals == Policy::SignedZero)
        for (int& magnitude : magnitudes)
          if (magnitude < format.normal())
            magnitude = 0;
      const bool product_negative = ((a ^ b) & sign) != 0;
      const bool addend_negative = (c & sign) != 0;
      const int64_t scale = static_cast<int64_t>(format.units_per_one);
      const int64_t product =
          static_cast<int64_t>(format.units(magnitudes[0])) *
          static_cast<int64_t>(format.units(magnitudes[1]));
      const int64_t addend =
          static_cast<int64_t>(format.units(magnitudes[2])) * scale;
      // E4M3FN magnitudes are <=229376 units. Thus abs(sum)<=52,730,789,888
      // (<2^36) and scale^2<=2^18, within the private rounder's bounds.
      const int64_t sum = (product_negative ? -product : product) +
                          (addend_negative ? -addend : addend);
      const bool negative = sum == 0 ? (product_negative == addend_negative
                                            ? product_negative
                                            : policy.rounding == Policy::RDN)
                                     : sum < 0;
      return Word::Float::round_tiny(
          format, policy, static_cast<uint64_t>(sum < 0 ? -sum : sum),
          static_cast<uint64_t>(scale * scale), negative);
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

    /** \brief Tiny fused multiply-add a*b+c
     *
     * The exact outcome evaluates one triple of encodings. The default
     * reasoning reads each operand once (see narrow_bounds) and never
     * combines operand values.
     */
    struct TinyFma {
      TinyFormat format;
      Policy policy;

      Word::Float::TinyOutcome operator()(WordValue a, WordValue b,
                                          WordValue c) const {
        const TinyResult r =
            evaluate_fma(format, policy, static_cast<int>(a),
                         static_cast<int>(b), static_cast<int>(c));
        return {static_cast<WordValue>(r.code), r.status};
      }

      /// Return the hull of the products of numbers of \a a and \a b
      static TinyInterval products(const TinySummary& a,
                                   const TinySummary& b, int64_t scale) {
        const int64_t d = scale * scale;
        const TinyRational corners[] = {{a.least * b.least, d},
                                        {a.least * b.greatest, d},
                                        {a.greatest * b.least, d},
                                        {a.greatest * b.greatest, d}};
        return Word::Float::hull(corners, 4);
      }
      /// Return the interval \a p plus the numbers of \a c
      static TinyInterval plus_addend(const TinyInterval& p,
                                      const TinySummary& c, int64_t scale) {
        return {{p.least.numerator + c.least * scale, p.least.denominator},
                {p.greatest.numerator + c.greatest * scale,
                 p.greatest.denominator}};
      }

      /** \brief Test whether the number \a value of operand \a i combines
       * with numbers of the other operands into an exact result in
       * \a reals
       */
      bool reaches(unsigned int i, int64_t value, const TinySummary* s,
                   const TinyInterval& reals) const {
        const int64_t scale = static_cast<int64_t>(format.units_per_one);
        for (unsigned int j = 0; j < 3; ++j)
          if (j != i && !s[j].has_number)
            return false;
        TinySummary point;
        point.has_number = true;
        point.least = point.greatest = value;
        const TinyInterval sum =
            i == 2 ? plus_addend(products(s[0], s[1], scale), point, scale)
                   : plus_addend(products(point, s[1 - i], scale), s[2],
                                 scale);
        return reals.meets(sum.least, sum.greatest);
      }

      /** \brief Narrow to the outcomes that the numerical bounds of the
       * operands allow
       *
       * Reads each operand once: its NaNs and its least and greatest
       * numbers. The result keeps the codes between the rounded least and
       * greatest exact results and the NaNs of NaN operands and of
       * overflow. Each operand number is kept when the other operands'
       * bounds can combine it into an exact result that rounds to a kept
       * number or overflows to a kept NaN (see Word::Float::preimage), or
       * when it pairs with a kept NaN. A NaN operand is kept when its NaN
       * result is kept. The status keeps the flags of the kept classes of
       * outcomes; I and D never arise.
       */
      template<class StatusView>
      ExecStatus narrow_bounds(Space& home, WordView a, WordView b,
                               WordView c, WordView result,
                               StatusView status) const {
        const int64_t scale = static_cast<int64_t>(format.units_per_one);
        const WordView views[] = {a, b, c};
        const TinyValues values[] = {TinyValues(a), TinyValues(b),
                                     TinyValues(c)};
        const TinySummary operands[] = {
            Word::Float::summarise(format, policy, values[0]),
            Word::Float::summarise(format, policy, values[1]),
            Word::Float::summarise(format, policy, values[2])};
        TinyOutcomes outcomes(result.values(),
                              Word::Float::admitted_flags(status));
        TinyRounding rounding;
        if (operands[0].has_number && operands[1].has_number &&
            operands[2].has_number)
          rounding = Word::Float::round_interval(
              format, policy,
              plus_addend(products(operands[0], operands[1], scale),
                          operands[2], scale));
        const WordValueSet numbers =
            outcomes.keep(rounding.numbers, rounding.number_flags);
        const WordValueSet overflow_nans =
            outcomes.keep(rounding.overflow_nans(),
                          1U << (Word::Float::overflow | Word::Float::inexact));
        const Word::Float::TinyNans nans(
            policy, operands, 3,
            outcomes.keep(Word::Float::TinyNans::results(policy, operands, 3),
                          1U << 0));
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
        for (unsigned int i = 0; i < 3; ++i) {
          const ModEvent me = Word::Float::narrow_kept(
              home, views[i], values[i], [&](WordValue code) {
                if (format.is_nan(code))
                  return nans.keeps_nan(i, code);
                return nans.keeps_number(i) ||
                       (reals && reaches(i, format.value(code, policy),
                                         operands, *reals));
              });
          if (failed_change(me, changed))
            return ES_FAILED;
        }
        return changed ? ES_NOFIX : ES_FIX;
      }
    };

    using Word::Float::admits_nonfinite;
    using Word::Float::Direction;
    using Word::Float::domain_codes;
    using Word::Float::exact_product;
    using Word::Float::ExactDyadic;
    using Word::Float::finite_parts;
    using Word::Float::Role;
    using Word::Float::End;
    using Word::Float::FinitePart;
    using Word::Float::floor_encoding;
    using Word::Float::greatest_with_masks;
    using Word::Float::include_real;
    using Word::Float::include_rounded;
    using Word::Float::Integer;
    using Word::Float::invalid;
    using Word::Float::least_in_window;
    using Word::Float::least_with_masks;
    using Word::Float::magnitude_rounding;
    using Word::Float::MagnitudeRounding;
    using Word::Float::sum_term_extreme;
    using Word::Float::SumTarget;
    using Word::Float::SumTerm;
    using Word::Float::CellEnds;
    using Word::Float::rounding_cell;
    using Word::Float::quotient_bounds;
    using Word::Float::real_domain;
    using Word::Float::round_dyadic;
    using Word::Float::RoundingCell;
    using Word::Float::visit_alias_classes;
    using Word::Float::wide_intersect;
    using Word::Float::wide_snapshot;
    using Word::Float::corner_quotients;
    using Word::Float::include_meeting;
    using Word::Float::include_values;
    using Word::Float::interval_bits;
    using Word::Float::merge;
    using Word::Float::Read;
    using Word::Float::read_cube;
    using Word::Float::read_result;
    using Word::Float::read_value;
    using Word::Float::read_word;
    using Word::Float::result_code_cell;
    using Word::Float::ResultCells;
    using Word::Float::ResultSign;
    using Word::Float::run_step_rule;
    using Word::Float::StepOutcome;
    using Word::Float::StepState;
    using Word::Float::ValueRange;
    using Word::Float::WideFormat;
    using Word::Float::WideProjection;
    using Word::Float::WideResult;

    WideResult evaluate_wide_fma(const WideFormat& source,
                                 const WideFormat& destination,
                                 const Policy& policy, WordValue a, WordValue b,
                                 WordValue c) {
      const WideFormat* formats[] = {&source, &source, &destination};
      const WordValue original[] = {a, b, c};
      WordValue effective[] = {a, b, c};
      int first_nan = -1, first_signaling = -1;
      for (int i = 0; i < 3; ++i) {
        if (formats[i]->nan(original[i]) && first_nan < 0)
          first_nan = i;
        if (formats[i]->signaling(original[i]) && first_signaling < 0)
          first_signaling = i;
        if (policy.input_subnormals == Policy::SignedZero &&
            (effective[i] & (formats[i]->sign - 1)) < formats[i]->normal)
          effective[i] &= formats[i]->sign;
      }
      const WordValue am = effective[0] & (source.sign - 1);
      const WordValue bm = effective[1] & (source.sign - 1);
      const WordValue cm = effective[2] & (destination.sign - 1);
      const bool invalid_product = (am == 0 && bm == source.infinity) ||
                                   (bm == 0 && am == source.infinity);
      if (first_nan >= 0) {
        const int selected =
            policy.nan == Policy::SignalFirst && first_signaling >= 0
                ? first_signaling
                : first_nan;
        const unsigned int payload_shift =
            destination.fraction_bits - formats[selected]->fraction_bits;
        const WordValue code =
            policy.nan == Policy::Canonical
                ? destination.infinity | destination.quiet
                : (original[selected] << payload_shift) | destination.quiet;
        return {code, first_signaling >= 0 || invalid_product ? invalid : 0};
      }
      const bool product_negative = ((a ^ b) & source.sign) != 0;
      const bool addend_negative = (c & destination.sign) != 0;
      const bool infinite_product =
          am == source.infinity || bm == source.infinity;
      if (invalid_product || (infinite_product && cm == destination.infinity &&
                              product_negative != addend_negative))
        return {destination.infinity | destination.quiet, invalid};
      if (infinite_product)
        return {destination.infinity |
                    (product_negative ? destination.sign : 0),
                0};
      if (cm == destination.infinity)
        return {c, 0};
      ExactDyadic sum = exact_product(source, effective[0], effective[1]);
      const int addend_shift =
          -sum.exponent - 127 - static_cast<int>(destination.fraction_bits);
      assert(addend_shift >= 0);
      sum.coefficient =
          sum.coefficient +
          destination.value(effective[2])
              .shift_left(static_cast<unsigned int>(addend_shift));
      sum.negative_zero = product_negative == addend_negative
                              ? product_negative
                              : policy.rounding == Policy::RDN;
      return round_dyadic(destination, policy, sum);
    }

    // Position of a cube member among all members in ascending order: the
    // free bits of value, compacted.
    WordValue member_index(WordValue value, WordValue free) {
      WordValue index = 0, weight = 1;
      for (; free; free &= free - 1, weight <<= 1)
        if (value & free & (WordValue(0) - free))
          index |= weight;
      return index;
    }

    // Number of ranks in interval that the rank cube ordered admits,
    // saturated at limit.
    unsigned int admitted_rank_count(Word::WordCube ordered,
                                     Word::WordRankInterval interval,
                                     WordValue mask, unsigned int limit) {
      const auto admitted =
          Word::find_admitted_interval(ordered, interval, mask);
      if (!admitted.is_admitted)
        return 0;
      const WordValue free = (ordered.hi & ~ordered.lo) & mask;
      const WordValue span = member_index(admitted.interval.maximum, free) -
                             member_index(admitted.interval.minimum, free);
      return span >= limit ? limit : static_cast<unsigned int>(span) + 1;
    }

    // Number of encodings a cube/rank domain admits, saturated at limit. A
    // float cube with an unknown sign bit is counted one sign half at a time,
    // as its rank cube is exact only within a half.
    unsigned int admitted_count(WordView view, const Word::WordDomain& domain,
                                unsigned int limit) {
      const WordDomainType type = view.domain_type();
      const unsigned int width = view.width();
      if (!Word::splits_by_sign(type, width, domain.cube))
        return admitted_rank_count(Word::order_cube(type, width, domain.cube),
                                   domain.interval, view.mask(), limit);
      // Negative encodings have the ranks below the sign bit.
      const WordValue s = Word::sign_bit(width);
      const Word::SignHalves halves = Word::sign_halves(domain.cube, s);
      const Word::WordRankInterval interval = domain.interval;
      unsigned int count = 0;
      if (interval.minimum < s)
        count = admitted_rank_count(
            Word::order_cube(type, width, halves.with_sign),
            {interval.minimum, std::min(interval.maximum, s - 1)}, view.mask(),
            limit);
      if (interval.maximum >= s)
        count += admitted_rank_count(
            Word::order_cube(type, width, halves.without_sign),
            {std::max(interval.minimum, s), interval.maximum}, view.mask(),
            limit - count);
      return count;
    }

    // Store the encodings a cube/rank domain admits, in ascending rank order.
    void collect_admitted_encodings(WordView view,
                                    const Word::WordDomain& domain,
                                    WordValue encodings[]) {
      WordValue lower = domain.interval.minimum, candidate;
      unsigned int size = 0;
      while (Word::next_admitted_rank(view.domain_type(), view.width(),
                                      domain.cube, lower, candidate) &&
             candidate <= domain.interval.maximum) {
        encodings[size++] =
          Word::encode_rank(view.domain_type(), view.width(), candidate);
        if (candidate == domain.interval.maximum)
          break;
        lower = candidate + 1;
      }
    }

    // The distinct inputs of a scan: input i reads column columns[i] of the
    // inputs[0..count-1], which admit sizes[0..count-1] encodings.
    struct ScanPlan {
      unsigned int sizes[3];
      int columns[3], inputs[3], count;
    };

    // Plan the scan of every assignment of the distinct inputs. Return false
    // when their product exceeds the budget, decided before any enumeration.
    bool plan_scan(const WordView views[4], const Word::WordDomain domains[4],
                   ScanPlan& plan) {
      static constexpr unsigned int assignment_budget = 2048;
      unsigned int assignments = 1;
      plan.count = 0;
      for (int i = 0; i < 3; ++i) {
        int j = 0;
        while (j < plan.count && views[i] != views[plan.inputs[j]])
          ++j;
        plan.columns[i] = j;
        if (j < plan.count)
          continue;
        plan.inputs[plan.count++] = i;
        const unsigned int limit = assignment_budget / assignments;
        plan.sizes[j] = admitted_count(views[i], domains[i], limit + 1);
        if (plan.sizes[j] > limit)
          return false;
        assert(plan.sizes[j] > 0);
        assignments *= plan.sizes[j];
      }
      return true;
    }

    // Scan every assignment of the distinct inputs of plan and keep the
    // supported tuples. The scan is extra work on top of the basic wide
    // propagation, which checks assigned inputs exactly and, once no input
    // admits a NaN or an infinity, applies the finite interval projections
    // of project_fma at any domain size. The budget of plan_scan bounds only
    // this scan. An empty supported relation is represented by absent
    // projections.
    void scan_fma(const WideFormat& source, const WideFormat& destination,
                  const Policy& policy, const WordView views[4],
                  const Word::WordDomain domains[4], const ScanPlan& plan,
                  const Int::IntView* status, WideProjection projections[4],
                  bool flags[32]) {
      const unsigned int* sizes = plan.sizes;
      const int* columns = plan.columns;
      const int* inputs = plan.inputs;
      const int input_count = plan.count;
      Region r;
      WordValue* candidates[3];
      for (int j = 0; j < input_count; ++j) {
        candidates[j] = r.alloc<WordValue>(sizes[j]);
        collect_admitted_encodings(views[inputs[j]], domains[inputs[j]],
                                   candidates[j]);
      }
      unsigned int indices[3] = {};
      for (;;) {
        const WordValue a = candidates[columns[0]][indices[columns[0]]];
        const WordValue b = candidates[columns[1]][indices[columns[1]]];
        const WordValue c = candidates[columns[2]][indices[columns[2]]];
        const WideResult result =
            evaluate_wide_fma(source, destination, policy, a, b, c);
        const WordValue tuple[] = {a, b, c, result.code};
        bool allowed = (!status || status->in(result.status)) &&
                       views[3].in(result.code);
        for (int i = 0; i < 3; ++i)
          if (views[i] == views[3])
            allowed &= tuple[i] == result.code;
        if (allowed) {
          for (int i = 0; i < 4; ++i)
            projections[i].include(views[i], domains[i], tuple[i], tuple[i]);
          flags[result.status] = true;
        }
        int i = 0;
        for (; i < input_count; ++i) {
          if (++indices[i] < sizes[i])
            break;
          indices[i] = 0;
        }
        if (i == input_count)
          return;
      }
    }

    void project_product_inputs(const WideFormat& source,
                                const WordView views[4], const FinitePart& a,
                                const FinitePart& b,
                                const RoundingCell& product,
                                WideProjection inputs[2]) {
      if (b.low.zero() || b.high.zero()) {
        inputs[0].include(views[0], a.domain);
      } else {
        const auto inverse = quotient_bounds(product, b);
        include_real(source, inputs[0], views[0], a, inverse.low, inverse.high);
      }
      if (a.low.zero() || a.high.zero()) {
        inputs[1].include(views[1], b.domain);
      } else {
        const auto inverse = quotient_bounds(product, a);
        include_real(source, inputs[1], views[1], b, inverse.low, inverse.high);
      }
    }

    // For result == addend, a*b must belong to cell(c)-c. Within a binade
    // this residual is constant apart from tie parity and its first value,
    // whose predecessor has smaller spacing. Keep that boundary separate.
    void project_absorption(const WideFormat& source,
                            const WideFormat& destination, const Policy& policy,
                            const WordView views[4], const FinitePart& a,
                            const FinitePart& b, const FinitePart& c,
                            const RoundingCell& product,
                            unsigned int addend_shift,
                            WideProjection projections[4]) {
      const WordValue sign = c.negative ? destination.sign : 0;
      const Word::WordEncodingRange codes = domain_codes(views[2], c.domain);
      WordValue first = codes.first & (destination.sign - 1);
      const WordValue last = codes.last & (destination.sign - 1);
      while (first <= last) {
        // Saturation and overflow can give maximum finite an unbounded cell.
        if (first == destination.maximum) {
          const auto maximum =
              wide_intersect(views[2], c.domain, sign | first, sign | first);
          const Integer center =
              destination.value(sign | first).shift_left(addend_shift);
          const int exponent =
              -2 * (127 + static_cast<int>(source.fraction_bits));
          WideProjection forward;
          include_rounded(destination, forward, views[3], maximum.domain,
                          round_dyadic(destination, policy,
                                       {product.low + center, exponent, false})
                              .code,
                          round_dyadic(destination, policy,
                                       {product.high + center, exponent, false})
                              .code);
          if (forward.present) {
            projections[0].include(views[0], a.domain);
            projections[1].include(views[1], b.domain);
            for (int i : {2, 3})
              projections[i].include(views[i], maximum.domain);
          }
          break;
        }
        WordValue end;
        if (first < destination.normal)
          end = destination.normal - 1;
        else if (first % destination.normal == 0)
          end = first;
        else
          end = (first / destination.normal + 1) * destination.normal - 1;
        end = std::min(end, std::min(last, destination.maximum - 1));
        const auto admitted =
            wide_intersect(views[2], c.domain, sign | first, sign | end);
        if (admitted.is_consistent) {
          const WordValue code = domain_codes(views[2], admitted.domain).first;
          auto residual = rounding_cell(destination, policy, code, addend_shift, CellEnds::Open);
          const Integer center =
              destination.value(code).shift_left(addend_shift);
          residual.low = residual.low - center;
          residual.high = residual.high - center;
          if (policy.rounding == Policy::RNE &&
              admitted.domain.interval.minimum !=
                  admitted.domain.interval.maximum) {
            // A range can contain even encodings between odd endpoints.
            // Including midpoint ties is an outward parity relaxation.
            const Integer odd(code & 1);
            residual.low = residual.low - odd;
            residual.high = residual.high + odd;
          }
          residual.low = std::max(residual.low, product.low);
          residual.high = std::min(residual.high, product.high);
          if (residual.low <= residual.high) {
            WideProjection inputs[2];
            project_product_inputs(source, views, a, b, residual, inputs);
            if (inputs[0].present && inputs[1].present) {
              for (int i : {0, 1})
                projections[i].include(views[i],
                                       {inputs[i].cube, inputs[i].interval});
              for (int i : {2, 3})
                projections[i].include(views[i], admitted.domain);
            }
          }
        }
        first = end + 1;
      }
    }

    // The lattice of the nonzero numbers of a finite part: each one is
    // odd * 2^t with t >= shift and odd <= this odd bound.
    struct Granularity {
      // Whether the part holds only zeros, flushed subnormals included
      bool zero;
      unsigned int shift;
      Integer odd;
    };

    // In the half-subnormal units of WideFormat with p fraction bits, a
    // number with biased exponent e > 0 is (2^p + fraction) * 2^e and a
    // subnormal is 2 * fraction. A nonzero number is thus odd * 2^t with
    // t >= max(e,1) and odd < 2^(p+1). Exponents grow with magnitude, so the
    // least magnitude of a part bounds t from below; a single number gives
    // t and odd exactly.
    Granularity granularity(const WideFormat& f, const FinitePart& x) {
      if (x.low.zero() && x.high.zero())
        return {true, 0, Integer()};
      const Integer low = std::min(x.low.abs(), x.high.abs());
      const Integer high = std::max(x.low.abs(), x.high.abs());
      unsigned int shift = 0;
      if (low == high)
        while (!low.bit(shift))
          ++shift;
      else
        shift = low.bits() > f.fraction_bits + 1
                    ? low.bits() - f.fraction_bits - 1
                    : 1;
      const Integer odd = Integer::power_two(f.fraction_bits + 1) - Integer(1);
      return {false, shift, std::min(odd, high.shift_right(shift))};
    }

    // The largest j such that [low, high], with 0 < low <= high, contains a
    // multiple of 2^j: the highest bit in which low - 1 and high differ.
    // There high has a one, so its prefix down to that bit is a multiple of
    // 2^j in the range, and the multiples of 2^(j+1) around it lie outside.
    unsigned int largest_multiple_exponent(const Integer& low,
                                           const Integer& high) {
      const Integer below = low - Integer(1);
      unsigned int j = high.bits() - 1;
      while (high.bit(j) == below.bit(j))
        --j;
      return j;
    }

    // Bounds |a*b| <= product and |c*2^k| <= addend on every support of a
    // cell that excludes zero, or no support at all
    struct TermBounds {
      bool supported;
      Integer product, addend;
    };

    // Cancellation bound for the exact sum s = a*b + c*2^k in a cell [low,
    // high] that excludes zero, with granularities ga, gb of the multiplicand
    // parts and gc of the addend part. By Granularity, s is a nonzero
    // multiple of 2^m with m = min(t(a)+t(b), t(c)+k), taking t(0) as
    // infinite, so m <= g for the largest g with a multiple of 2^g in the
    // cell. Either t(a)+t(b) <= g and |a*b| <= odd(a)*odd(b)*2^g, or
    // t(c)+k <= g and |c*2^k| <= odd(c)*2^g; in each case the other term
    // differs from the bounded one by |s| <= h = max(|low|, |high|).
    //
    // The interval projections cannot see this lattice: where |s| is far
    // below the spacing of b and c, they move b <= c + |s| down to c and
    // c <= b - |s| down to the predecessor of b, one encoding at a time
    // until that spacing reaches the scale of s. The bound jumps to that
    // limit directly. Where a multiplicand part holds a single value,
    // project_fma uses the exact extremes of linear_extreme instead: there
    // the bound is a limit only for one of a*b and c, and the projections
    // would walk the other one encoding per pass.
    TermBounds cancellation_bounds(const Granularity& ga, const Granularity& gb,
                                   const Granularity& gc, unsigned int k,
                                   const Integer& low, const Integer& high) {
      assert(low.neg() == high.neg() && !low.zero() && !high.zero());
      const Integer least = std::min(low.abs(), high.abs());
      const Integer greatest = std::max(low.abs(), high.abs());
      const unsigned int g = largest_multiple_exponent(least, greatest);
      const bool by_product =
          !ga.zero && !gb.zero && ga.shift + gb.shift <= g;
      const bool by_addend = !gc.zero && gc.shift + k <= g;
      if (!by_product && !by_addend)
        return {false, Integer(), Integer()};
      const Integer product = (ga.odd * gb.odd).shift_left(g);
      const Integer addend = gc.odd.shift_left(g);
      Integer product_bound, addend_bound;
      if (by_product) {
        product_bound = product;
        addend_bound = product + greatest;
      }
      if (by_addend) {
        product_bound = std::max(product_bound, addend + greatest);
        addend_bound = std::max(addend_bound, addend);
      }
      return {true, product_bound, addend_bound};
    }

    // The codes of one binade of a finite part, as the values first + i*step
    // for 0 <= i < count: within a binade the values are equally spaced.
    struct Progression {
      Integer first, step, count;
    };

    // The progression of the magnitude codes [low, high] of one binade, with
    // sign
    Progression progression(const WideFormat& f, WordValue sign, WordValue low,
                            WordValue high) {
      const unsigned int exponent =
          static_cast<unsigned int>(low >> f.fraction_bits);
      const Integer spacing = Integer::power_two(std::max(exponent, 1U));
      return {f.value(sign | low), sign != 0 ? -spacing : spacing,
              Integer(high - low + 1)};
    }

    // The values of a finite part, from the magnitude code low to high, each
    // times coefficient: one term of a sum
    struct LinearTerm {
      const WideFormat* format;
      WordValue sign, low, high;
      Integer coefficient;
    };

    LinearTerm linear_term(const WideFormat& f, WordView view,
                           const FinitePart& part, const Integer& coefficient) {
      const Word::WordEncodingRange codes = domain_codes(view, part.domain);
      return {&f, part.negative ? f.sign : WordValue(0),
              codes.first & (f.sign - 1), codes.last & (f.sign - 1),
              coefficient};
    }

    // The magnitude codes [first, last] of term y whose values lie in [low,
    // high]; false when there are none
    bool magnitude_codes(const LinearTerm& y, const Integer& low,
                         const Integer& high, WordValue& first,
                         WordValue& last) {
      const WideFormat& f = *y.format;
      const Integer least = y.sign != 0 ? -high : low;
      const Integer most = y.sign != 0 ? -low : high;
      if (most.neg())
        return false;
      first = y.low;
      if (!least.neg() && !least.zero()) {
        WordValue code = floor_encoding(f, least);
        if (f.magnitude(code) < least)
          ++code;
        first = std::max(first, code);
      }
      last = std::min(y.high, floor_encoding(f, most));
      return first <= last;
    }

    // The largest (or least) i in [0, x.count) with a j in [0, y.count) such
    // that (x.first + i*x.step)*cx + (y.first + j*y.step)*cy lies in [low,
    // high]; false when there is none. With a = x.step*cx and b = |y.step*cy|
    // the sum is base + i*a + j*b after counting j from the end where y.step*cy
    // is negative, and i has such a j exactly when the window [low - base -
    // i*a, high - base - i*a] meets [0, (y.count-1)*b], which holds for i in
    // one range, and holds a multiple of b. A window meeting that range but
    // not inside it holds one of its ends, both multiples of b, so on the
    // range the condition is that (high - base - i*a) mod b is at most the
    // width high - low. Stepping i from the end of the range adds a constant
    // to that residue modulo b, so the first i that passes is a least_in_window
    // problem. bound is set to the end of that range on the side sought,
    // clamped on that side to the indices of x: no i beyond it pairs with y,
    // also when the range is empty.
    bool extreme_index(const Progression& x, const Integer& cx,
                       const Progression& y, const Integer& cy,
                       const Integer& low, const Integer& high, bool largest,
                       Integer& index, Integer& bound) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      using Word::Arithmetic::Global::mod;
      const Integer a = x.step * cx;
      Integer b = y.step * cy;
      const Integer last_j = y.count - Integer(1);
      Integer base = x.first * cx + y.first * cy;
      if (b.neg()) {
        base = base + last_j * b;
        b = -b;
      }
      const Integer l = low - base, h = high - base;
      const Integer top = l - last_j * b;
      Integer first = a.neg() ? ceil_div(h, a) : ceil_div(top, a);
      Integer last = a.neg() ? floor_div(top, a) : floor_div(h, a);
      first = std::max(first, Integer());
      last = std::min(last, x.count - Integer(1));
      bound = largest ? last : first;
      if (first > last)
        return false;
      const Integer width = high - low;
      const Integer r = mod(h - (largest ? last : first) * a, b);
      Integer t;
      if (r > width &&
          !least_in_window(mod(largest ? a : -a, b), b, b - r, b - r + width,
                           t))
        return false;
      if (t > last - first)
        return false;
      index = largest ? last - t : first + t;
      return true;
    }

    // The greatest (end High) or least (end Low) value v of term x for which
    // some value w of term y puts v*x.coefficient + w*y.coefficient in
    // [low, high]; false when there is none. Every value of the binades
    // between the bounds of each part counts: known bits are not used. The
    // binades of x are tried from the extreme end, and within one binade the
    // binades of y that can pair with it, so the cost is bounded by the
    // binades of the formats. It is not sum_term_extreme (float-wide.hpp):
    // that search walks the known bits of terms whose scales are powers of
    // two, while the coefficient of x here is an arbitrary integer (the
    // fixed factor of the product), so each binade of x is an arithmetic
    // progression and is searched with extreme_index instead.
    // The scan starts within the codes of x that the real relaxation leaves
    // (every value of y between its ends), since the binades beyond them
    // pair with no binade of y.
    //
    // With a = step*x.coefficient of the binade of x, its index i pairs with
    // a binade of y whose values span [y_min, y_max] only if i*a lies in
    // [low - y_max*cy, high - y_min*cy] less the first term (cy > 0 after
    // the flip below). The binades of y are disjoint and ordered by value,
    // so the end of that index range on the side sought (the bound of
    // extreme_index) moves monotonically from binade to binade: for the
    // largest i with a > 0 it falls as the values of y rise. The binades are
    // visited from the one with the most extreme bound, and the visit ends
    // once a bound reaches the best index found: no later binade passes it.
    bool linear_extreme(const LinearTerm& x, const LinearTerm& y,
                        const Integer& low, const Integer& high, End end,
                        Integer& value) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      // Make the coefficient of y positive.
      const bool flip = y.coefficient.neg();
      const Integer cx = flip ? -x.coefficient : x.coefficient;
      const Integer cy = flip ? -y.coefficient : y.coefficient;
      const Integer sum_low = flip ? -high : low;
      const Integer sum_high = flip ? -low : high;
      const unsigned int p = x.format->fraction_bits;
      const unsigned int q = y.format->fraction_bits;
      // The values of x grow with the index of a progression when x is
      // positive, so the greatest value has the largest index.
      const bool largest = (end == End::High) != (x.sign != 0);
      const Integer y_first = y.format->value(y.sign | y.low);
      const Integer y_last = y.format->value(y.sign | y.high);
      const Integer term_low = sum_low - cy * std::max(y_first, y_last);
      const Integer term_high = sum_high - cy * std::min(y_first, y_last);
      WordValue x_low, x_high;
      if (!magnitude_codes(
              x, cx.neg() ? ceil_div(term_high, cx) : ceil_div(term_low, cx),
              cx.neg() ? floor_div(term_low, cx) : floor_div(term_high, cx),
              x_low, x_high))
        return false;
      // The index in a binade of x grows with the magnitude code, so the
      // extreme code of the relaxation is x_high or x_low. When it pairs with
      // a value of y it is the extreme value; otherwise the binades are
      // searched.
      const Integer extreme =
          x.format->value(x.sign | (largest ? x_high : x_low));
      WordValue y_low, y_high;
      if (magnitude_codes(y, ceil_div(sum_low - extreme * cx, cy),
                          floor_div(sum_high - extreme * cx, cy), y_low,
                          y_high)) {
        value = extreme;
        return true;
      }
      const WordValue lowest = x_low >> p, highest = x_high >> p;
      for (WordValue k = 0; k <= highest - lowest; ++k) {
        const WordValue e = largest ? highest - k : lowest + k;
        const Progression xs =
            progression(*x.format, x.sign, std::max(x_low, e << p),
                        std::min(x_high, ((e + 1) << p) - 1));
        const Integer last_value =
            xs.first + (xs.count - Integer(1)) * xs.step;
        const Integer first_term = xs.first * cx, end_term = last_value * cx;
        WordValue first, last;
        if (!magnitude_codes(
                y,
                ceil_div(sum_low - std::max(first_term, end_term), cy),
                floor_div(sum_high - std::min(first_term, end_term), cy),
                first, last))
          continue;
        // The binades of y in the order of the bounds they give (see above),
        // from the one that admits the most extreme index.
        const bool small_y_first = largest != (xs.step.neg() != cx.neg());
        const bool ascending = small_y_first == (y.sign == 0);
        const WordValue g_low = first >> q, g_high = last >> q;
        bool found = false;
        Integer best = largest ? -Integer(1) : xs.count;
        for (WordValue n = 0; n <= g_high - g_low; ++n) {
          const WordValue g = ascending ? g_low + n : g_high - n;
          const Progression ys =
              progression(*y.format, y.sign, std::max(first, g << q),
                          std::min(last, ((g + 1) << q) - 1));
          Integer i, bound;
          if (extreme_index(xs, cx, ys, cy, sum_low, sum_high, largest, i,
                            bound) &&
              (largest ? i > best : i < best)) {
            best = i;
            found = true;
          }
          // No later binade of y gives an index beyond bound.
          if (largest ? bound <= best : bound >= best)
            break;
        }
        if (found) {
          value = xs.first + best * xs.step;
          return true;
        }
      }
      return false;
    }

    // Round the exact sums [low, high], scaled by the square of the source
    // half-subnormal unit, onto the result domain. Return false when no
    // result code remains; otherwise candidate holds the reachable result
    // codes and tail whether they include a maximum-finite or infinity
    // code, whose preimages can be unbounded. Forward compatibility has
    // then already been checked against Word.
    bool forward_result(const WideFormat& source,
                        const WideFormat& destination, const Policy& policy,
                        WordView result, const Word::WordDomain& domain,
                        const Integer& low, const Integer& high,
                        Word::WordDomain& candidate, bool& tail) {
      const int exponent = -2 * (127 + static_cast<int>(source.fraction_bits));
      WideProjection forward;
      include_rounded(
          destination, forward, result, domain,
          round_dyadic(destination, policy, {low, exponent, false}).code,
          round_dyadic(destination, policy, {high, exponent, false}).code);
      if (!forward.present)
        return false;
      candidate = {forward.cube, forward.interval};
      tail = false;
      for (WordValue sign : {WordValue(0), destination.sign})
        for (WordValue magnitude : {destination.maximum, destination.infinity})
          tail |= wide_intersect(result, candidate, sign | magnitude,
                                 sign | magnitude)
                      .is_consistent;
      return true;
    }

    // The exact sums within [low, high] that round to a code of the finite
    // result part z, at the scale of forward_result
    RoundingCell result_cell(const WideFormat& destination,
                             const Policy& policy, WordView result,
                             const FinitePart& z, unsigned int scale,
                             const Integer& low, const Integer& high) {
      const Word::WordEncodingRange codes = domain_codes(result, z.domain);
      return {std::max(low, rounding_cell(destination, policy,
                                         z.negative ? codes.last : codes.first,
                                         scale, CellEnds::Open)
                                .low),
              std::min(high, rounding_cell(destination, policy,
                                          z.negative ? codes.first
                                                     : codes.last,
                                          scale, CellEnds::Open)
                                 .high)};
    }

    // Whether the values of a finite part exclude zero
    bool nonzero(const FinitePart& part) {
      return !part.low.zero() && !part.high.zero();
    }

    /** \brief The addend of an fma whose product is one value
     *
     * Read with the sign of the result part z, the exact sum of the fixed
     * product P and an addend code c of binade e has magnitude m = fixed +-
     * X*2^a, with fixed = +-P, X the significand of c and a = max(e, 1) +
     * k, k the addend scale; the term is negative when c and z differ in
     * sign. A code of z in binade f, with significand Q, has magnitude
     * Q*2^b, b = max(f, 1) + k, and m rounds to the admitted codes of that
     * binade iff m lies within the cells of its least and greatest admitted
     * codes and R(m/2^b) has the known fraction bits of z, R the rounding
     * of magnitudes (see SumTarget). The cell of a nonzero code below the
     * greatest finite magnitude holds all sums that round to it and, where
     * subnormal results flush, perhaps more; the greatest finite magnitude
     * itself never gets here (forward_result reports it as the tail). So
     * sum_term_extreme over the binades of c and, in each, those of z
     * finds every addend code without support, and the extreme code it
     * returns is the extreme supported one when its evaluation lands in z.
     */
    struct FixedProduct {
      const WideFormat& source;
      const WideFormat& destination;
      const Policy& policy;
      const WordView* views;
      WordValue a_code, b_code;
      const FinitePart& c;
      const FinitePart& z;
      /// The product read with the sign of z
      Integer fixed;
      unsigned int addend_bits;
      /// The known bits of the addend and of the result magnitudes
      std::uint64_t c_mask, c_value, z_mask, z_value;
      /// The magnitudes of the least and greatest codes of z
      WordValue z_first, z_last;
    };

    /// Whether a fixed product decides the support of an fma cell
    enum class FixedProductStatus { Supported, Unsupported, Undecided };

    // The magnitude of the cell of code (with the sign of z) at the scale of
    // the sums: its low end or its high end
    Integer cell_magnitude(const FixedProduct& fp, WordValue code, End end) {
      const bool low = end == End::Low;
      const WideFormat& d = fp.destination;
      const RoundingCell cell =
          rounding_cell(d, fp.policy, (fp.z.negative ? d.sign : 0) | code,
                       fp.addend_bits, CellEnds::Open);
      if (fp.z.negative)
        return low ? -cell.high : -cell.low;
      return low ? cell.low : cell.high;
    }

    // The extreme significand of term, an addend binade, whose sums reach
    // an admitted code of z (FixedProduct). Return false when there is
    // none.
    bool fixed_product_binade(const FixedProduct& fp, SumTerm term,
                              Direction direction, std::uint64_t& best) {
      const WideFormat& d = fp.destination;
      const WordValue fraction = d.normal - 1;
      const Integer least = Integer(term.from).shift_left(term.a);
      const Integer greatest = Integer(term.to).shift_left(term.a);
      const Integer m_low = term.negative ? fp.fixed - greatest
                                          : fp.fixed + least;
      const Integer m_high = term.negative ? fp.fixed - least
                                           : fp.fixed + greatest;
      if (m_high < Integer(1))
        return false;
      const MagnitudeRounding rounding =
          magnitude_rounding(fp.policy.rounding, fp.z.negative);
      bool found = false;
      const WordValue first_binade = fp.z_first >> d.fraction_bits;
      const WordValue last_binade = fp.z_last >> d.fraction_bits;
      for (WordValue f = first_binade; f <= last_binade; ++f) {
        const WordValue base = f * d.normal;
        if ((base ^ fp.z_value) & fp.z_mask & ~fraction)
          continue;
        std::uint64_t z_low, z_high;
        if (!least_with_masks(std::max(fp.z_first, base),
                              std::min(fp.z_last, base + fraction), fp.z_mask,
                              fp.z_value, 0, 0, 0, z_low) ||
            !greatest_with_masks(std::max(fp.z_first, base),
                                 std::min(fp.z_last, base + fraction),
                                 fp.z_mask, fp.z_value, 0, 0, 0, z_high))
          continue;
        const Integer low = cell_magnitude(fp, z_low, End::Low);
        if (low > m_high)
          break;
        const Integer high = cell_magnitude(fp, z_high, End::High);
        if (high < m_low)
          continue;
        const SumTarget target{low,
                               high,
                               static_cast<unsigned int>(std::max(
                                   f, WordValue(1))) +
                                   fp.addend_bits,
                               fp.z_mask & fraction,
                               fp.z_value & fraction,
                               rounding};
        std::uint64_t x;
        if (!sum_term_extreme(term, nullptr, fp.fixed, 0, target, direction,
                              x))
          continue;
        found = true;
        best = x;
        // Later binades of z only matter for significands beyond best.
        if (direction == Direction::Up) {
          if (x == term.from)
            return true;
          term.to = x - 1;
        } else {
          if (x == term.to)
            return true;
          term.from = x + 1;
        }
      }
      return found;
    }

    // The least (Up) or greatest (Down) addend magnitude in [low, high]
    // whose sums reach z (FixedProduct). Return false when there is none.
    bool fixed_product_end(const FixedProduct& fp, WordValue low,
                           WordValue high, Direction direction,
                           WordValue& end) {
      const WideFormat& d = fp.destination;
      const WordValue fraction = d.normal - 1;
      const bool up = direction == Direction::Up;
      const WordValue first = low >> d.fraction_bits;
      const WordValue last = high >> d.fraction_bits;
      for (WordValue i = 0; i <= last - first; ++i) {
        const WordValue e = up ? first + i : last - i;
        const WordValue base = e * d.normal;
        if ((base ^ fp.c_value) & fp.c_mask & ~fraction)
          continue;
        const WordValue offset = e == 0 ? 0 : d.normal;
        const WordValue from = std::max(low, base);
        const WordValue to = std::min(high, base + fraction);
        const SumTerm term{from - base + offset,
                           to - base + offset,
                           fp.c_mask & fraction,
                           fp.c_value & fraction,
                           static_cast<unsigned int>(std::max(e, WordValue(1))) +
                               fp.addend_bits,
                           d.fraction_bits + 1,
                           fp.c.negative != fp.z.negative};
        std::uint64_t best;
        if (fixed_product_binade(fp, term, direction, best)) {
          end = base + static_cast<WordValue>(best) - offset;
          return true;
        }
      }
      return false;
    }

    // Whether the addend code (magnitude) x of the part c gives, with the
    // fixed multiplicands, a result in z
    bool fixed_product_supports(const FixedProduct& fp, WordValue x) {
      const WordValue code = (fp.c.negative ? fp.destination.sign : 0) | x;
      const WideResult r = evaluate_wide_fma(fp.source, fp.destination,
                                             fp.policy, fp.a_code, fp.b_code,
                                             code);
      return wide_intersect(fp.views[3], fp.z.domain, r.code, r.code)
          .is_consistent;
    }

    // Decide the cell of the parts a, b, c and z where both multiplicands
    // are one code each, no two operands are aliased and z holds no zero
    // (nor subnormal codes when the policy flushes results), by
    // FixedProduct. When the cell is supported, narrow addend, the addend
    // projection, to the hull of its supported codes.
    FixedProductStatus fixed_product(const WideFormat& source,
                                     const WideFormat& destination,
                                     const Policy& policy,
                                     const WordView views[4],
                                     const FinitePart& a, const FinitePart& b,
                                     const FinitePart& c, const FinitePart& z,
                                     unsigned int addend_bits,
                                     WideProjection& addend) {
      for (int i = 0; i < 4; ++i)
        for (int j = i + 1; j < 4; ++j)
          if (views[i] == views[j])
            return FixedProductStatus::Undecided;
      const auto as = domain_codes(views[0], a.domain);
      const auto bs = domain_codes(views[1], b.domain);
      if (as.first != as.last || bs.first != bs.last || !addend.present ||
          c.flushed || a.low != a.high || b.low != b.high)
        return FixedProductStatus::Undecided;
      const WordValue mask = destination.sign - 1;
      const auto zs = domain_codes(views[3], z.domain);
      const WordValue z_first = zs.first & mask;
      if (z_first == 0 || (z_first < destination.normal &&
                           policy.output_subnormals != Policy::Preserve))
        return FixedProductStatus::Undecided;
      const Word::WordDomain admitted{addend.cube, addend.interval};
      const auto cs = domain_codes(views[2], admitted);
      const std::uint64_t c_mask = ~(addend.cube.lo ^ addend.cube.hi) & mask;
      const std::uint64_t z_mask =
          ~(z.domain.cube.lo ^ z.domain.cube.hi) & mask;
      const Integer product = a.low * b.low;
      const FixedProduct fp{source,
                            destination,
                            policy,
                            views,
                            as.first,
                            bs.first,
                            c,
                            z,
                            z.negative ? -product : product,
                            addend_bits,
                            c_mask,
                            addend.cube.lo & c_mask,
                            z_mask,
                            z.domain.cube.lo & z_mask,
                            z_first,
                            zs.last & mask};
      WordValue low, high;
      if (!fixed_product_end(fp, cs.first & mask, cs.last & mask,
                             Direction::Up, low))
        return FixedProductStatus::Unsupported;
      if (!fixed_product_end(fp, low, cs.last & mask, Direction::Down, high) ||
          !fixed_product_supports(fp, low) || !fixed_product_supports(fp, high))
        return FixedProductStatus::Undecided;
      const WordValue sign = c.negative ? destination.sign : 0;
      const auto narrowed =
          wide_intersect(views[2], admitted, sign | low, sign | high);
      if (!narrowed.is_consistent)
        return FixedProductStatus::Undecided;
      addend = WideProjection();
      addend.include(views[2], narrowed.domain);
      return FixedProductStatus::Supported;
    }

    /** \brief The ends of the addend of an fma, by addend binade
     *
     * Read with the sign of an addend c in a normal binade with spacing U
     * (in the scale of the sums), the exact sum has magnitude X*U + q, X
     * the significand of c and q in [qlow, qhigh], the products a*b with
     * the sign of c taken as positive. Where X + q/U lies in [normal,
     * 2*normal] for every such q (and the sum stays at most the maximum
     * finite magnitude in the top binade), the sum rounds on the grid of
     * the binade: its magnitude rounds to (X + R(q/U))*U, R the floor, the
     * ceiling or the nearest integer (either neighbour at a tie), and
     * 2*normal*U is the value of the next code, the first one of the next
     * binade. So the result code is c + delta with delta in [delta_low,
     * delta_high], the range of R over [qlow/U, qhigh/U], and it is normal,
     * finite and of the sign of c. These codes are the interior of the
     * binade.
     *
     * An interior code is then supported only if some c + delta has the
     * known bits of the result part z and lies in its range, and c has the
     * known bits of the addend: an aligned sum of the addend's code, a free
     * term delta - delta_low and the constant delta_low, without rounding,
     * whose extreme c sum_term_extreme finds directly. The codes below the
     * interior and those above it form at most two zones per binade. As
     * rounding is monotone, the result codes of a zone lie between those
     * of its least sum and of its greatest one; when no code of z lies
     * between them, the zone holds no support. The subnormal binade is
     * one zone. A zone whose sums may cancel to zero or change sign counts
     * as supported.
     *
     * The search for an end visits these zones in its direction and stops
     * at the first possible support: at most three per binade.
     */
    struct AddendEnds {
      const WideFormat& source;
      const WideFormat& destination;
      const Policy& policy;
      WordView addend, result;
      const FinitePart& c;
      const FinitePart& z;
      /// The products, read with the sign of the addend
      Integer low, high;
      unsigned int addend_bits;
      /// The known bits of the addend magnitude
      std::uint64_t mask, value;
    };

    // The magnitude code of rounding the exact sum magnitude m with the
    // sign of the addend
    WordValue rounded_magnitude(const AddendEnds& ae, const Integer& m) {
      const int exponent =
          -2 * (127 + static_cast<int>(ae.source.fraction_bits));
      return round_dyadic(ae.destination, ae.policy,
                          {ae.c.negative ? -m : m, exponent, false})
                 .code &
             (ae.destination.sign - 1);
    }

    // Whether some code of z of the sign of the addend has a magnitude in
    // [low, high]. A zero of z of the other sign counts where low is zero,
    // as a flushed sum may take either sign of zero.
    bool result_between(const AddendEnds& ae, WordValue low, WordValue high) {
      const WideFormat& d = ae.destination;
      if (ae.z.negative != ae.c.negative)
        return low == 0;
      const WordValue mask = d.sign - 1;
      const auto codes = domain_codes(ae.result, ae.z.domain);
      low = std::max(low, codes.first & mask);
      high = std::min(high, codes.last & mask);
      const std::uint64_t z_mask =
          ~(ae.z.domain.cube.lo ^ ae.z.domain.cube.hi) & mask;
      std::uint64_t found;
      return low <= high &&
             least_with_masks(low, high, z_mask, ae.z.domain.cube.lo & z_mask,
                              0, 0, 0, found);
    }

    // Whether the sums of the one addend code (magnitude) x may round to a
    // code of z: the zone test of AddendEnds on the zone [x, x]
    bool code_may_reach(const AddendEnds& ae, WordValue x) {
      const Integer m =
          ae.destination.magnitude(x).shift_left(ae.addend_bits);
      const Integer least = m + ae.low;
      return least <= Integer() ||
             result_between(ae, rounded_magnitude(ae, least),
                            rounded_magnitude(ae, m + ae.high));
    }

    // The extreme addend code (magnitude) of the zone [from, to] of binade
    // e with the known bits of the addend, if the zone may hold a support
    // (AddendEnds). Return false when it does not.
    bool zone_extreme(const AddendEnds& ae, WordValue from, WordValue to,
                      Direction direction, WordValue& extreme) {
      const WideFormat& d = ae.destination;
      const Integer least =
          d.magnitude(from).shift_left(ae.addend_bits) + ae.low;
      if (least > Integer()) {
        const Integer greatest =
            d.magnitude(to).shift_left(ae.addend_bits) + ae.high;
        if (!result_between(ae, rounded_magnitude(ae, least),
                            rounded_magnitude(ae, greatest)))
          return false;
      }
      std::uint64_t x;
      if (direction == Direction::Up
              ? !least_with_masks(from, to, ae.mask, ae.value, 0, 0, 0, x)
              : !greatest_with_masks(from, to, ae.mask, ae.value, 0, 0, 0,
                                     x))
        return false;
      extreme = x;
      return true;
    }

    // The extreme interior addend code in [from, to] of binade e, with the
    // deltas [delta_low, delta_high] (AddendEnds). Return false when there
    // is none.
    bool interior_extreme(const AddendEnds& ae, WordValue from, WordValue to,
                          const Integer& delta_low, const Integer& delta_high,
                          Direction direction, WordValue& extreme) {
      const WideFormat& d = ae.destination;
      if (ae.z.negative != ae.c.negative)
        return false;
      const WordValue mask = d.sign - 1;
      const auto codes = domain_codes(ae.result, ae.z.domain);
      const WordValue z_first = std::max(WordValue(1), codes.first & mask);
      const WordValue z_last = codes.last & mask;
      if (z_first > z_last)
        return false;
      const std::uint64_t z_mask =
          ~(ae.z.domain.cube.lo ^ ae.z.domain.cube.hi) & mask;
      unsigned int width = 1;
      while ((mask >> width) != 0)
        ++width;
      const Integer spread = delta_high - delta_low;
      const SumTerm x{from, to, ae.mask, ae.value, 0, width, false};
      const SumTerm y{0, spread.word(), 0, 0, 0,
                      std::max(1U, spread.bits()), false};
      const SumTarget target{Integer(z_first),
                             Integer(z_last),
                             0,
                             z_mask,
                             ae.z.domain.cube.lo & z_mask,
                             MagnitudeRounding::Down};
      std::uint64_t found;
      if (!sum_term_extreme(x, &y, delta_low, 0, target, direction, found))
        return false;
      extreme = found;
      return true;
    }

    // The least (Up) or greatest (Down) addend code (magnitude) in [low,
    // high] that may be supported, by AddendEnds. Return false when there
    // is none.
    bool addend_end(const AddendEnds& ae, WordValue low, WordValue high,
                    Direction direction, WordValue& end) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      const WideFormat& d = ae.destination;
      const bool up = direction == Direction::Up;
      const WordValue top = d.maximum >> d.fraction_bits;
      const Integer normal(d.normal);
      // The first code x with the known bits of the addend in the direction
      // of the search. The zones and interiors before it hold no such code,
      // so their tests fail. When the sums of x alone may reach z, the test
      // of its zone or interior passes and returns x: the rounded sums of x
      // lie between those of the least and the greatest sum of its zone,
      // and in an interior they are c + R(q/U), within c + [delta_low,
      // delta_high]. So that test decides, without the walk, the searches
      // that narrow nothing.
      std::uint64_t x;
      if (up ? !least_with_masks(low, high, ae.mask, ae.value, 0, 0, 0, x)
             : !greatest_with_masks(low, high, ae.mask, ae.value, 0, 0, 0, x))
        return false;
      if (code_may_reach(ae, x)) {
        end = x;
        return true;
      }
      while (true) {
        const WordValue e = x >> d.fraction_bits;
        const WordValue base = e * d.normal;
        // The interior of binade e as significands [first, last], none in
        // the subnormal binade
        const unsigned int shift =
            static_cast<unsigned int>(e) + ae.addend_bits;
        Integer first = normal - floor_shift_right(ae.low, shift);
        Integer last = normal.shift_left(1) - ceil_shift_right(ae.high, shift) -
                       Integer(e == top ? 1 : 0);
        first = std::max(first, normal);
        last = std::min(last, normal.shift_left(1) - Integer(1));
        if (e == 0)
          last = Integer();
        const WordValue start = std::max(low, base);
        const WordValue stop = std::min(high, base + d.normal - 1);
        // Zones of binade e in [start, stop], in the order of the search
        WordValue zones[3][2];
        bool interior[3] = {false, false, false};
        int count = 0;
        if (first > last) {
          zones[count][0] = start;
          zones[count++][1] = stop;
        } else {
          const WordValue i_first = base + first.word() - d.normal;
          const WordValue i_last = base + last.word() - d.normal;
          if (start < i_first) {
            zones[count][0] = start;
            zones[count++][1] = std::min(stop, i_first - 1);
          }
          if (std::max(start, i_first) <= std::min(stop, i_last)) {
            zones[count][0] = std::max(start, i_first);
            zones[count][1] = std::min(stop, i_last);
            interior[count++] = true;
          }
          if (stop > i_last) {
            zones[count][0] = std::max(start, i_last + 1);
            zones[count++][1] = stop;
          }
        }
        Integer delta_low, delta_high;
        if (first <= last) {
          const MagnitudeRounding rounding =
              magnitude_rounding(ae.policy.rounding, ae.c.negative);
          const Integer spacing = Integer::power_two(shift);
          if (rounding == MagnitudeRounding::Down) {
            delta_low = floor_shift_right(ae.low, shift);
            delta_high = floor_shift_right(ae.high, shift);
          } else if (rounding == MagnitudeRounding::Up) {
            delta_low = ceil_shift_right(ae.low, shift);
            delta_high = ceil_shift_right(ae.high, shift);
          } else {
            delta_low =
                ceil_shift_right(ae.low.shift_left(1) - spacing, shift + 1);
            delta_high =
                floor_shift_right(ae.high.shift_left(1) + spacing, shift + 1);
          }
        }
        for (int k = 0; k < count; ++k) {
          const int i = up ? k : count - 1 - k;
          const WordValue from = zones[i][0];
          const WordValue to = zones[i][1];
          if (from > to)
            continue;
          if (up ? to < x : from > x)
            continue;
          const WordValue lo = up ? std::max(from, x) : from;
          const WordValue hi = up ? to : std::min(to, x);
          if (interior[i] ? interior_extreme(ae, lo, hi, delta_low,
                                             delta_high, direction, end)
                          : zone_extreme(ae, lo, hi, direction, end))
            return true;
        }
        if (up ? stop >= high : start <= low)
          return false;
        x = up ? stop + 1 : start - 1;
      }
    }

    // Whether every code in [low, high] has the known bits mask, given that
    // low and high have them: the known bits then lie above the highest bit
    // where low and high differ.
    bool known_bits_hold(WordValue low, WordValue high, std::uint64_t mask) {
      const std::uint64_t differ = low ^ high;
      return differ == 0 ||
             (mask & (~std::uint64_t(0) >> __builtin_clzll(differ))) == 0;
    }

    /*
     * Whether addend_end returns the ends low and high of the addend
     * projection unchanged, decided without the search. The ends come from
     * the real projection of the cell of z (include_real): the sums of an
     * end x, |x|*2^k + [ae.low, ae.high] read with the sign of the addend,
     * meet the cell. Where an end has the known bits of the addend, the
     * search in either direction starts at that end. Let z have the sign
     * of the addend and every code of its range [z_first, z_last]. The
     * least sum of x is at most the high end of the cell. Where z_last is
     * a normal code, or a subnormal one kept by the policy, its cell is its
     * preimage under rounding, so that sum rounds to at most z_last. The
     * greatest sum is at least the low end of the cell, and rounds to at
     * least z_first where z_first is zero or a code of the same kind.
     * Rounding is monotone, so the rounded sums of x then reach z, and
     * code_may_reach(x) holds.
     */
    bool ends_supported(const AddendEnds& ae, WordValue low, WordValue high) {
      const WideFormat& d = ae.destination;
      if (ae.z.negative != ae.c.negative || (low & ae.mask) != ae.value ||
          (high & ae.mask) != ae.value)
        return false;
      const WordValue mask = d.sign - 1;
      const auto codes = domain_codes(ae.result, ae.z.domain);
      const WordValue z_first = codes.first & mask;
      const WordValue z_last = codes.last & mask;
      const std::uint64_t z_mask =
          ~(ae.z.domain.cube.lo ^ ae.z.domain.cube.hi) & mask;
      const bool preserve = ae.policy.output_subnormals == Policy::Preserve;
      return z_last > 0 && (z_last >= d.normal || preserve) &&
             (z_first == 0 || z_first >= d.normal || preserve) &&
             known_bits_hold(z_first, z_last, z_mask);
    }

    // Narrow the addend projection input by AddendEnds, for the products
    // in products, the addend part c and the result part z. Return false
    // when no addend code remains.
    bool narrow_addend_ends(const WideFormat& source,
                            const WideFormat& destination,
                            const Policy& policy, const WordView views[4],
                            const FinitePart& c, const FinitePart& z,
                            const RoundingCell& products,
                            unsigned int addend_bits, WideProjection& input) {
      if (!input.present)
        return false;
      if (c.flushed)
        return true;
      const Word::WordDomain admitted{input.cube, input.interval};
      const WordValue mask = destination.sign - 1;
      const std::uint64_t known = ~(input.cube.lo ^ input.cube.hi) & mask;
      const AddendEnds ae{source,
                          destination,
                          policy,
                          views[2],
                          views[3],
                          c,
                          z,
                          c.negative ? -products.high : products.low,
                          c.negative ? -products.low : products.high,
                          addend_bits,
                          known,
                          input.cube.lo & known};
      const auto codes = domain_codes(views[2], admitted);
      if (ends_supported(ae, codes.first & mask, codes.last & mask))
        return true;
      WordValue low, high;
      if (!addend_end(ae, codes.first & mask, codes.last & mask,
                      Direction::Up, low) ||
          !addend_end(ae, low, codes.last & mask, Direction::Down, high))
        return false;
      const WordValue sign = c.negative ? destination.sign : 0;
      const auto narrowed =
          wide_intersect(views[2], admitted, sign | low, sign | high);
      if (!narrowed.is_consistent)
        return false;
      input = WideProjection();
      input.include(views[2], narrowed.domain);
      return true;
    }

    void project_fma(const WideFormat& source, const WideFormat& destination,
                     const Policy& policy, const WordView views[4],
                     const Word::WordDomain domains[4],
                     WideProjection projections[4]) {
      const auto as =
          finite_parts(source, policy, views[0], domains[0], Role::Input);
      const auto bs =
          finite_parts(source, policy, views[1], domains[1], Role::Input);
      const auto cs =
          finite_parts(destination, policy, views[2], domains[2], Role::Input);
      const int sum_exponent =
          -2 * (127 + static_cast<int>(source.fraction_bits));
      const int addend_shift =
          -sum_exponent - 127 - static_cast<int>(destination.fraction_bits);
      assert(addend_shift >= 0);
      const unsigned int addend_bits = static_cast<unsigned int>(addend_shift);
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      for (const auto& a : as)
        for (const auto& b : bs) {
          const Granularity ga = granularity(source, a);
          const Granularity gb = granularity(source, b);
          Integer product_low = a.low * b.low, product_high = product_low;
          for (const Integer& av : {a.low, a.high})
            for (const Integer& bv : {b.low, b.high}) {
              const Integer product = av * bv;
              product_low = std::min(product_low, product);
              product_high = std::max(product_high, product);
            }
          for (const auto& c : cs) {
            const bool subnormal =
                c.low.abs() < destination.magnitude(destination.normal) &&
                c.high.abs() < destination.magnitude(destination.normal);
            if (views[2] == views[3] && !c.flushed && !c.low.zero() &&
                !c.high.zero() &&
                (!subnormal ||
                 policy.output_subnormals == Policy::Preserve)) {
              project_absorption(source, destination, policy, views, a, b, c,
                                 {product_low, product_high}, addend_bits,
                                 projections);
              continue;
            }
            const Integer low = product_low + c.low.shift_left(addend_bits);
            const Integer high = product_high + c.high.shift_left(addend_bits);
            Word::WordDomain candidate;
            bool tail;
            if (!forward_result(source, destination, policy, views[3],
                                domains[3], low, high, candidate, tail))
              continue;
            if (tail) {
              projections[0].include(views[0], a.domain);
              projections[1].include(views[1], b.domain);
              projections[2].include(views[2], c.domain);
              projections[3].include(views[3], candidate);
              continue;
            }
            for (const auto& z : finite_parts(destination, policy, views[3],
                                              candidate, Role::Result)) {
              const RoundingCell cell = result_cell(
                  destination, policy, views[3], z, addend_bits, low, high);
              if (cell.low > cell.high)
                continue;
              // With a multiplicand part holding a single value m, the sum
              // x*m + c*2^k is linear in the other multiplicand x and in c,
              // and linear_extreme finds the extremes of both exactly. The
              // interval projections below would instead move the bounds of
              // x and c towards them one encoding per pass where the cell
              // is narrower than the spacing of both terms.
              if ((a.low == a.high || b.low == b.high) && nonzero(a) &&
                  nonzero(b) && nonzero(c)) {
                const int fixed = b.low == b.high ? 1 : 0;
                const int other = 1 - fixed;
                const FinitePart& m = fixed == 1 ? b : a;
                const FinitePart& x = fixed == 1 ? a : b;
                const LinearTerm xt =
                    linear_term(source, views[other], x, m.low);
                const LinearTerm ct = linear_term(destination, views[2], c,
                                                  Integer::power_two(addend_bits));
                Integer x_low, x_high, c_low, c_high;
                if (!linear_extreme(xt, ct, cell.low, cell.high, End::Low, x_low))
                  continue;
                // A solution exists, so the other extremes exist too.
                (void)linear_extreme(xt, ct, cell.low, cell.high, End::High, x_high);
                (void)linear_extreme(ct, xt, cell.low, cell.high, End::Low, c_low);
                (void)linear_extreme(ct, xt, cell.low, cell.high, End::High, c_high);
                WideProjection inputs[3];
                include_real(source, inputs[other], views[other], x, x_low,
                             x_high);
                inputs[fixed].include(views[fixed], m.domain);
                include_real(destination, inputs[2], views[2], c, c_low,
                             c_high);
                if (!inputs[0].present || !inputs[1].present ||
                    !inputs[2].present)
                  continue;
                // linear_extreme reads every code between the bounds of c
                // and z. The addend ends, and with both multiplicands single
                // codes FixedProduct, also read the known bits of c and z.
                const Integer ends[2] = {m.low * x_low, m.low * x_high};
                if (!narrow_addend_ends(source, destination, policy, views, c,
                                        z,
                                        {std::min(ends[0], ends[1]),
                                         std::max(ends[0], ends[1])},
                                        addend_bits, inputs[2]) ||
                    fixed_product(source, destination, policy, views, a, b, c,
                                  z, addend_bits, inputs[2]) ==
                        FixedProductStatus::Unsupported)
                  continue;
                for (int i = 0; i < 3; ++i)
                  projections[i].include(views[i],
                                         {inputs[i].cube, inputs[i].interval});
                projections[3].include(views[3], z.domain);
                continue;
              }
              // The terms a*b and c*2^k, narrowed by the cancellation bound
              // when the cell excludes zero.
              RoundingCell product_terms{product_low, product_high};
              RoundingCell addend_terms{c.low.shift_left(addend_bits),
                                        c.high.shift_left(addend_bits)};
              const bool cancellation = cell.low.neg() == cell.high.neg() &&
                                        !cell.low.zero() && !cell.high.zero();
              if (cancellation) {
                const TermBounds bounds = cancellation_bounds(
                    ga, gb, granularity(destination, c), addend_bits,
                    cell.low, cell.high);
                if (!bounds.supported)
                  continue;
                product_terms.low = std::max(product_terms.low, -bounds.product);
                product_terms.high = std::min(product_terms.high, bounds.product);
                addend_terms.low = std::max(addend_terms.low, -bounds.addend);
                addend_terms.high = std::min(addend_terms.high, bounds.addend);
                if (product_terms.low > product_terms.high ||
                    addend_terms.low > addend_terms.high)
                  continue;
              }
              WideProjection inputs[3];
              include_real(
                  destination, inputs[2], views[2], c,
                  ceil_shift_right(std::max(cell.low - product_terms.high,
                                            addend_terms.low),
                                   addend_bits),
                  floor_shift_right(std::min(cell.high - product_terms.low,
                                             addend_terms.high),
                                    addend_bits));
              if (!narrow_addend_ends(source, destination, policy, views, c,
                                      z, product_terms, addend_bits,
                                      inputs[2]))
                continue;
              RoundingCell product{cell.low - addend_terms.high,
                                   cell.high - addend_terms.low};
              if (cancellation) {
                product.low = std::max(product.low, product_terms.low);
                product.high = std::min(product.high, product_terms.high);
                if (product.low > product.high)
                  continue;
              }
              project_product_inputs(source, views, a, b, product, inputs);
              if (!inputs[0].present || !inputs[1].present ||
                  !inputs[2].present ||
                  fixed_product(source, destination, policy, views, a, b, c,
                                z, addend_bits, inputs[2]) ==
                      FixedProductStatus::Unsupported)
                continue;
              for (int i = 0; i < 3; ++i)
                projections[i].include(views[i],
                                       {inputs[i].cube, inputs[i].interval});
              projections[3].include(views[3], z.domain);
            }
          }
        }
    }

    // x*(x + offset) for the value x of code
    Integer shifted_square(const WideFormat& f, WordValue code,
                           const Integer& offset) {
      const Integer x = f.value(code);
      return x * (x + offset);
    }

    /// How x*(x + offset) moves with the code on a run of codes
    enum class Slope { Rising, Falling };

    // The first code in [first, last] whose x*(x + offset) has passed bound
    // in the direction of slope: lies above it when rising, below it when
    // falling, or last + 1 if there is none. The codes must form a run on
    // which that test turns true only once.
    WordValue passing_code(const WideFormat& f, WordValue first,
                           WordValue last, const Integer& offset,
                           const Integer& bound, Slope slope) {
      WordValue low = first, high = last + 1;
      while (low < high) {
        const WordValue middle = low + (high - low) / 2;
        const Integer v = shifted_square(f, middle, offset);
        if (slope == Slope::Rising ? v > bound : v < bound)
          high = middle;
        else
          low = middle + 1;
      }
      return low;
    }

    // The codes [low, high] of the run [first, last] whose x*(x + offset)
    // lies in target, where x*(x + offset) moves with the code on the run
    // as slope says. Return false when there are none.
    bool square_run(const WideFormat& f, WordValue first, WordValue last,
                    const Integer& offset, const RoundingCell& target,
                    Slope slope, WordValue& low, WordValue& high) {
      if (first > last)
        return false;
      const bool rising = slope == Slope::Rising;
      low = passing_code(f, first, last, offset,
                         rising ? target.low - Integer(1)
                                : target.high + Integer(1),
                         slope);
      const WordValue end = passing_code(
          f, first, last, offset, rising ? target.high : target.low, slope);
      if (low >= end)
        return false;
      high = end - 1;
      return true;
    }

    // For a == b the product x*x rises with the code on the codes of either
    // sign, so the codes of a part whose squares lie in a cell less the
    // addend form one range, found by bisection, and the addend follows
    // from the squares of that range. This projects x exactly where
    // project_fma bounds one x by the other and converges by one encoding
    // per pass. It serves only a result that aliases the assigned addend;
    // every other a == b shape takes the closed form (project_square_sum).
    void project_square(const WideFormat& source,
                        const WideFormat& destination, const Policy& policy,
                        const WordView views[4],
                        const Word::WordDomain domains[4],
                        WideProjection projections[4]) {
      const int sum_exponent =
          -2 * (127 + static_cast<int>(source.fraction_bits));
      const unsigned int addend_bits = static_cast<unsigned int>(
          -sum_exponent - 127 - static_cast<int>(destination.fraction_bits));
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      const auto cs =
          finite_parts(destination, policy, views[2], domains[2], Role::Input);
      for (const auto& x :
           finite_parts(source, policy, views[0], domains[0], Role::Input)) {
        const Word::WordEncodingRange codes = domain_codes(views[0], x.domain);
        const bool zero = x.flushed || (x.low.zero() && x.high.zero());
        // The squares rise with the magnitude, so with the code.
        const Integer square_low =
            zero ? Integer() : shifted_square(source, codes.first, Integer());
        const Integer square_high =
            zero ? Integer() : shifted_square(source, codes.last, Integer());
        for (const auto& c : cs) {
          const Integer low = square_low + c.low.shift_left(addend_bits);
          const Integer high = square_high + c.high.shift_left(addend_bits);
          Word::WordDomain candidate;
          bool tail;
          if (!forward_result(source, destination, policy, views[3],
                              domains[3], low, high, candidate, tail))
            continue;
          if (tail) {
            for (int i : {0, 1})
              projections[i].include(views[i], x.domain);
            projections[2].include(views[2], c.domain);
            projections[3].include(views[3], candidate);
            continue;
          }
          for (const auto& z : finite_parts(destination, policy, views[3],
                                            candidate, Role::Result)) {
            const RoundingCell cell = result_cell(
                destination, policy, views[3], z, addend_bits, low, high);
            if (cell.low > cell.high)
              continue;
            const RoundingCell target{
                std::max(square_low, cell.low - c.high.shift_left(addend_bits)),
                std::min(square_high,
                         cell.high - c.low.shift_left(addend_bits))};
            if (target.low > target.high)
              continue;
            WideProjection inputs[2];
            Integer least = square_low, greatest = square_high;
            if (zero) {
              inputs[0].include(views[0], x.domain);
            } else {
              WordValue first, last;
              if (!square_run(source, codes.first, codes.last, Integer(),
                              target, Slope::Rising, first, last))
                continue;
              inputs[0].include(views[0], x.domain, first, last);
              least = shifted_square(source, first, Integer());
              greatest = shifted_square(source, last, Integer());
            }
            include_real(destination, inputs[1], views[2], c,
                         ceil_shift_right(cell.low - greatest, addend_bits),
                         floor_shift_right(cell.high - least, addend_bits));
            if (!inputs[0].present || !inputs[1].present)
              continue;
            for (int i : {0, 1})
              projections[i].include(views[i],
                                     {inputs[0].cube, inputs[0].interval});
            projections[2].include(views[2],
                                   {inputs[1].cube, inputs[1].interval});
            projections[3].include(views[3], z.domain);
          }
        }
      }
    }

    /*
     * Closed-form projection of the addend-alias shapes
     *
     * For a == c the exact sum is x*y + x = x*(y + 1) = x*K, for b == c it
     * is the same with the roles of a and b swapped, and for a == b == c it
     * is x*(x + 1). A projection reads every word as a range of real values
     * (the result as a range of sums), projects the real relaxation of the
     * relation on that box exactly, and keeps the codes of each domain whose
     * value (for the result, whose rounding cell) meets the projection. The
     * codes are treated as real numbers; only the rounding cells of the
     * result keep their exact shape. Known bits act only on each word on its
     * own: through the codes that a read reaches and the codes that are kept.
     *
     * The reads, and why a widened pass is a fixpoint of itself, are
     * described before Read in float-wide.hpp.
     */

    /// Whether the domain d of view admits a code of [first, last], one sign
    bool admits_codes(WordView view, const Word::WordDomain& d,
                      WordValue first, WordValue last) {
      return wide_intersect(view, d, first, last).is_consistent;
    }

    // For an addend aliasing multiplicand m, the exact sum a*b + c is x*K
    // with x the aliased word and K = y + 1 for the other multiplicand y
    // (a == c gives a*(b + 1), b == c gives b*(a + 1)); r is the result. The
    // pieces are the sign parts of x (negative, the zero-valued codes,
    // positive) times those of K. Every finite y other than -1 lies at least
    // 2^-(p+1) above -1 or 2^-p below it, so K keeps out of the hole
    // (-2^-p, 2^-(p+1)) except K = 0, which is y = -1 and gives the exact
    // sum 0 for every x. A piece with x and K of one sign each projects
    // exactly: x onto X with V/K, K onto K with V/X, the sum onto V with
    // X*K, all from the one box and by corners. The zero pieces (x zero, or
    // y = -1 in the domain) contribute the zero sum, which needs a result
    // code whose cell holds 0 (the cells of both zeros hold it; the sign of
    // the zero is not reasoned about). The cell bounds and the code
    // conversions are exact on the integer lattice of the units, so the
    // projection is the exact hull of the real relaxation of the reads
    // (read_word, read_result), rounded inward to codes.
    void project_addend_alias(const WideFormat& f, const Policy& policy,
                              const WordView views[4],
                              const Word::WordDomain domains[4], int m,
                              Read read, WideProjection projections[4]) {
      const int o = 1 - m;
      const unsigned int p = f.fraction_bits;
      const unsigned int k = 127 + p;
      const Integer one = Integer::power_two(k);
      const Integer beyond = f.magnitude(f.infinity);
      // Bounds |x*K| for every read value x and y
      const Integer big = beyond * beyond * Integer(4);
      const ValueRange xr = read_word(f, policy, views[m], domains[m], read);
      const ValueRange yr = read_word(f, policy, views[o], domains[o], read);
      const RoundingCell v =
          read_result(f, policy, views[3], domains[3], k, big, read);
      const ValueRange x_parts[] = {{xr.low, std::min(xr.high, -Integer(1))},
                                    {std::max(xr.low, Integer(1)), xr.high}};
      const ValueRange k_parts[] = {
          {yr.low + one, std::min(yr.high + one, -Integer::power_two(k - p))},
          {std::max(yr.low + one, Integer::power_two(k - p - 1)),
           yr.high + one}};
      WideProjection x_whole, y_whole, r_whole;
      for (const ValueRange& x : x_parts) {
        if (x.empty())
          continue;
        for (const ValueRange& kp : k_parts) {
          if (kp.empty())
            continue;
          ValueRange xq = corner_quotients(v.low, v.high, kp.low, kp.high);
          xq.low = std::max(xq.low, x.low);
          xq.high = std::min(xq.high, x.high);
          ValueRange kq = corner_quotients(v.low, v.high, x.low, x.high);
          kq.low = std::max(kq.low, kp.low);
          kq.high = std::min(kq.high, kp.high);
          Integer s_low = x.low * kp.low, s_high = s_low;
          for (const Integer& a : {x.low, x.high})
            for (const Integer& b : {kp.low, kp.high}) {
              s_low = std::min(s_low, a * b);
              s_high = std::max(s_high, a * b);
            }
          s_low = std::max(s_low, v.low);
          s_high = std::min(s_high, v.high);
          if (xq.empty() || kq.empty() || s_low > s_high)
            continue;
          WideProjection px, py, pr;
          include_values(f, policy, px, views[m], domains[m], xq.low, xq.high);
          include_values(f, policy, py, views[o], domains[o], kq.low - one,
                         kq.high - one);
          include_meeting(f, policy, pr, views[3], domains[3], k, big, s_low,
                          s_high);
          if (!px.present || !py.present || !pr.present)
            continue;
          merge(x_whole, views[m], px);
          merge(y_whole, views[o], py);
          merge(r_whole, views[3], pr);
        }
      }
      // The zero sum: x zero with any y, or y = -1 with any x
      WideProjection zero_result;
      include_meeting(f, policy, zero_result, views[3], domains[3], k, big,
                      Integer(), Integer());
      if (zero_result.present) {
        WideProjection px;
        include_values(f, policy, px, views[m], domains[m], Integer(),
                       Integer());
        if (px.present) {
          merge(x_whole, views[m], px);
          y_whole.include(views[o], domains[o]);
          merge(r_whole, views[3], zero_result);
        }
        const WordValue minus_one = f.sign | 127 * f.normal;
        if (admits_codes(views[o], domains[o], minus_one, minus_one)) {
          WideProjection nonzero;
          for (const ValueRange& x : x_parts)
            include_values(f, policy, nonzero, views[m], domains[m], x.low,
                           x.high);
          if (nonzero.present) {
            merge(x_whole, views[m], nonzero);
            y_whole.include(views[o], domains[o], minus_one, minus_one);
            merge(r_whole, views[3], zero_result);
          }
        }
      }
      for (int i : {m, 2})
        projections[i] = x_whole;
      projections[o] = y_whole;
      projections[3] = r_whole;
    }

    // For a == b == c the exact sum is x*(x + 1) of the one word x. It falls
    // as the value rises below -1/2 and rises above it, so the nonzero codes
    // of each sign split into runs on which it is monotone in the code:
    // positive codes (rising), negative codes down to -1/2 (falling as the
    // code, the magnitude, rises) and from -1/2 on (rising). On a run the
    // codes whose sum lies in the read V of the result form one range, found
    // by bisection with the exact sums (square_run), and the domain's codes
    // in it are kept. The result is projected from the sums of the kept
    // codes: V intersected with the sums between the least and greatest kept
    // code of each run. x is read by its own codes under both reads; only
    // the read of the result differs. Under the widened read the pass is
    // still a fixpoint of itself: a second pass keeps every kept code, since
    // its sum lies in that intersection, which the new read of the result
    // contains (the argument of the closed form), so the kept range of each
    // run and its sums can only grow. A run whose sums meet no result code
    // is dropped. The zero-valued codes have the exact sum 0.
    void project_square_addend(const WideFormat& f, const Policy& policy,
                               const WordView views[4],
                               const Word::WordDomain domains[4], Read read,
                               WideProjection projections[4]) {
      const unsigned int k = 127 + f.fraction_bits;
      const Integer one = Integer::power_two(k);
      const Integer beyond = f.magnitude(f.infinity);
      const Integer big = beyond * beyond * Integer(4);
      const RoundingCell v =
          read_result(f, policy, views[3], domains[3], k, big, read);
      const WordView x = views[0];
      const Word::WordDomain& d = domains[0];
      WideProjection x_whole, r_whole;
      {
        WideProjection px, pr;
        include_values(f, policy, px, x, d, Integer(), Integer());
        include_meeting(f, policy, pr, views[3], domains[3], k, big,
                        Integer(), Integer());
        if (px.present && pr.present) {
          merge(x_whole, x, px);
          merge(r_whole, views[3], pr);
        }
      }
      const WordValue least =
          policy.input_subnormals == Policy::SignedZero ? f.normal : 1;
      // The magnitude code of 1/2
      const WordValue half = 126 * f.normal;
      struct Run {
        WordValue first, last;
        Slope slope;
      };
      const Run runs[] = {{least, f.maximum, Slope::Rising},
                          {f.sign | least, f.sign | half, Slope::Falling},
                          {f.sign | half, f.sign | f.maximum, Slope::Rising}};
      for (const Run& run : runs) {
        const auto admitted = wide_intersect(x, d, run.first, run.last);
        if (!admitted.is_consistent)
          continue;
        const Word::WordEncodingRange codes = domain_codes(x, admitted.domain);
        WordValue low, high;
        if (!square_run(f, codes.first, codes.last, one, v, run.slope, low,
                        high))
          continue;
        WideProjection px;
        px.include(x, d, low, high);
        if (!px.present)
          continue;
        const Word::WordEncodingRange kept = domain_codes(x, {px.cube, px.interval});
        const Integer a = shifted_square(f, kept.first, one);
        const Integer b = shifted_square(f, kept.last, one);
        WideProjection pr;
        include_meeting(f, policy, pr, views[3], domains[3], k, big,
                        std::max(std::min(a, b), v.low),
                        std::min(std::max(a, b), v.high));
        if (!pr.present)
          continue;
        merge(x_whole, x, px);
        merge(r_whole, views[3], pr);
      }
      for (int i = 0; i < 3; ++i)
        projections[i] = x_whole;
      projections[3] = r_whole;
    }

    // For a == b with the result aliasing no input the exact sum is x*x + c,
    // in units of the square of the source unit, with c scaled from the
    // destination unit. c is read as a word (read_word; an assigned addend
    // reads as its value) and x by its own codes. The pieces are the
    // zero-valued codes of x, with the square 0, and the nonzero codes of
    // either sign, on which x*x rises with the code; there the codes whose
    // square q has q + c in the read V of the result for some c in the read
    // C, that is q in [V.low - C.high, V.high - C.low], form one range, found
    // by bisection (square_run), and are kept. With [a, b] the squares from
    // the least to the greatest kept code, the piece projects c onto C and
    // [V.low - b, V.high - a], and the result onto V and
    // [a + C.low, b + C.high]: the real relaxation with x*x relaxed to [a, b].
    // A piece is dropped unless every word keeps a code of it.
    //
    // The widened pass is a fixpoint of itself. For a kept code x take c* in
    // C with x*x + c* in V. Then c* lies in the projection of c and x*x + c*
    // in that of the result, which the new reads contain (the argument of
    // the closed form), so a second pass keeps x, and [a, b] can only grow.
    // A published c has some q in [a, b] with q + c in V, so q + c lies in
    // the projection of the result, hence in the new V; a published result
    // code meets some q + c with q in [a, b] and c in C, so c lies in the
    // projection of c, hence in the new C. So the second pass keeps every
    // published code.
    void project_square_sum(const WideFormat& source,
                            const WideFormat& destination,
                            const Policy& policy, const WordView views[4],
                            const Word::WordDomain domains[4], Read read,
                            WideProjection projections[4]) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      const unsigned int k = 127 + source.fraction_bits;
      const unsigned int scale = 2 * k - 127 - destination.fraction_bits;
      const ValueRange cr =
          read_word(destination, policy, views[2], domains[2], read);
      const RoundingCell c{cr.low.shift_left(scale), cr.high.shift_left(scale)};
      const Integer beyond = source.magnitude(source.infinity);
      // Bounds |x*x + c| for every read value x and c
      const Integer big =
          (beyond * beyond +
           destination.magnitude(destination.infinity).shift_left(scale)) *
          Integer(4);
      const RoundingCell v =
          read_result(destination, policy, views[3], domains[3], scale, big,
                      read);
      // The squares q with q + c in V for some c in C
      const RoundingCell squares{v.low - c.high, v.high - c.low};
      const WordView x = views[0];
      const Word::WordDomain& d = domains[0];
      const WordValue least =
          policy.input_subnormals == Policy::SignedZero ? source.normal : 1;
      WideProjection x_whole, c_whole, r_whole;
      // The zero-valued codes, then the nonzero codes of each sign
      for (int piece = 0; piece < 3; ++piece) {
        WideProjection px;
        Integer a, b;
        if (piece == 0) {
          include_values(source, policy, px, x, d, Integer(), Integer());
        } else {
          const WordValue sign = piece == 1 ? 0 : source.sign;
          const auto admitted =
              wide_intersect(x, d, sign | least, sign | source.maximum);
          if (!admitted.is_consistent)
            continue;
          const Word::WordEncodingRange codes =
              domain_codes(x, admitted.domain);
          WordValue low, high;
          if (!square_run(source, codes.first, codes.last, Integer(), squares,
                          Slope::Rising, low, high))
            continue;
          px.include(x, d, low, high);
          if (!px.present)
            continue;
          const Word::WordEncodingRange kept =
              domain_codes(x, {px.cube, px.interval});
          a = shifted_square(source, kept.first, Integer());
          b = shifted_square(source, kept.last, Integer());
          if (a > b)
            std::swap(a, b);
        }
        if (!px.present)
          continue;
        WideProjection pc, pr;
        include_values(destination, policy, pc, views[2], domains[2],
                       ceil_shift_right(std::max(v.low - b, c.low), scale),
                       floor_shift_right(std::min(v.high - a, c.high), scale));
        include_meeting(destination, policy, pr, views[3], domains[3], scale,
                        big, std::max(a + c.low, v.low),
                        std::min(b + c.high, v.high));
        if (!pc.present || !pr.present)
          continue;
        merge(x_whole, x, px);
        merge(c_whole, views[2], pc);
        merge(r_whole, views[3], pr);
      }
      projections[0] = projections[1] = x_whole;
      projections[2] = c_whole;
      projections[3] = r_whole;
    }

    // The widened projection of an fma (the step rule;
    // WideFma::propagate_step), reading every position independently; the
    // positions of an aliased word read its one domain
    // (WideFma::project_step). The exact sum is s = a*b + c' with c' =
    // c*2^scale, in units of the square of the source unit. Every word is
    // read (read_word, read_result with the cells of sums); the inputs are
    // finite here, as no projection runs while an input admits a NaN or an
    // infinity. The pieces are the sign parts of the reads of a and b
    // (values up to -1 and from 1), and the zero-valued codes of a or b,
    // whose exact product is 0. On a sign piece the products form the real
    // interval P = A*B (corners), and the piece projects the real relation
    // exactly:
    //  - the products onto P' = P with V - C',
    //  - a onto A with P'/B and b onto B with P'/A (corners, B and A of one
    //    sign),
    //  - c onto C with (V - P)/2^scale,
    //  - the sum onto V with P + C',
    // where {a : a*b in P' for some b in B} is P'/B because the products
    // that meet P' are in P anyway. On the zero piece s = c': c onto C with
    // V/2^scale, the sum onto V with C', the zero-valued codes of the one
    // operand and every code of the other. Values are integers, so the
    // inward integer hulls keep exactly the codes in the projections; the
    // sums are integers too. A piece is dropped unless each word keeps a code
    // of it. The widened pass is a fixpoint of itself by the argument of the
    // closed form (before Read in float-wide.hpp): the projection of each
    // kept piece at the new reads contains the old one. The zero piece is a
    // box with a coordinate fixed to 0, a side condition that does not depend
    // on the box. Lattice reasoning (cancellation bound, linear_extreme,
    // narrow_addend_ends, FixedProduct) is not part of this projection; it
    // belongs to the hull step (project_fma).
    void project_widened_fma(const WideFormat& source,
                             const WideFormat& destination,
                             const Policy& policy, const WordView views[4],
                             const Word::WordDomain domains[4], Read read,
                             WideProjection projections[4]) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      const unsigned int k = 127 + source.fraction_bits;
      const unsigned int scale = 2 * k - 127 - destination.fraction_bits;
      const ValueRange cr =
          read_word(destination, policy, views[2], domains[2], read);
      const RoundingCell c{cr.low.shift_left(scale), cr.high.shift_left(scale)};
      const Integer beyond = source.magnitude(source.infinity);
      // Bounds |a*b + c'| for every read value
      const Integer big =
          (beyond * beyond +
           destination.magnitude(destination.infinity).shift_left(scale)) *
          Integer(4);
      const RoundingCell v = read_result(destination, policy, views[3],
                                         domains[3], scale, big, read);
      const ValueRange ar = read_word(source, policy, views[0], domains[0], read);
      const ValueRange br = read_word(source, policy, views[1], domains[1], read);
      const ValueRange as[] = {{ar.low, std::min(ar.high, -Integer(1))},
                               {std::max(ar.low, Integer(1)), ar.high}};
      const ValueRange bs[] = {{br.low, std::min(br.high, -Integer(1))},
                               {std::max(br.low, Integer(1)), br.high}};
      WideProjection wholes[4];
      for (const ValueRange& a : as)
        for (const ValueRange& b : bs) {
          if (a.empty() || b.empty())
            continue;
          Integer p_low = a.low * b.low, p_high = p_low;
          for (const Integer& x : {a.low, a.high})
            for (const Integer& y : {b.low, b.high}) {
              p_low = std::min(p_low, x * y);
              p_high = std::max(p_high, x * y);
            }
          const Integer q_low = std::max(p_low, v.low - c.high);
          const Integer q_high = std::min(p_high, v.high - c.low);
          if (q_low > q_high)
            continue;
          ValueRange aq = corner_quotients(q_low, q_high, b.low, b.high);
          ValueRange bq = corner_quotients(q_low, q_high, a.low, a.high);
          aq = {std::max(aq.low, a.low), std::min(aq.high, a.high)};
          bq = {std::max(bq.low, b.low), std::min(bq.high, b.high)};
          if (aq.empty() || bq.empty())
            continue;
          WideProjection piece[4];
          include_values(source, policy, piece[0], views[0], domains[0],
                         aq.low, aq.high);
          include_values(source, policy, piece[1], views[1], domains[1],
                         bq.low, bq.high);
          include_values(destination, policy, piece[2], views[2], domains[2],
                         ceil_shift_right(std::max(v.low - p_high, c.low), scale),
                         floor_shift_right(std::min(v.high - p_low, c.high),
                                           scale));
          include_meeting(destination, policy, piece[3], views[3], domains[3],
                          scale, big, std::max(p_low + c.low, v.low),
                          std::min(p_high + c.high, v.high));
          if (!piece[0].present || !piece[1].present || !piece[2].present ||
              !piece[3].present)
            continue;
          for (int i = 0; i < 4; ++i)
            merge(wholes[i], views[i], piece[i]);
        }
      // The zero products: a zero-valued code of a with every b, or of b
      // with every a
      WideProjection zeros[2];
      include_values(source, policy, zeros[0], views[0], domains[0], Integer(),
                     Integer());
      include_values(source, policy, zeros[1], views[1], domains[1], Integer(),
                     Integer());
      if (zeros[0].present || zeros[1].present) {
        WideProjection pc, pr;
        include_values(destination, policy, pc, views[2], domains[2],
                       ceil_shift_right(std::max(v.low, c.low), scale),
                       floor_shift_right(std::min(v.high, c.high), scale));
        include_meeting(destination, policy, pr, views[3], domains[3], scale,
                        big, std::max(c.low, v.low), std::min(c.high, v.high));
        if (pc.present && pr.present)
          for (int i = 0; i < 2; ++i)
            if (zeros[i].present) {
              merge(wholes[i], views[i], zeros[i]);
              wholes[1 - i].include(views[1 - i], domains[1 - i]);
              merge(wholes[2], views[2], pc);
              merge(wholes[3], views[3], pr);
            }
      }
      for (int i = 0; i < 4; ++i)
        projections[i].unite(wholes[i]);
    }

    // Free multiplicands m with v*m + 2^scale_bits*c in target, for v != 0
    // and c in the addend part (absent when the addend aliases the result).
    RoundingCell multiplicand_bounds(const RoundingCell& target,
                                     const FinitePart& x, const Integer& v,
                                     const FinitePart* addend,
                                     unsigned int scale_bits) {
      const RoundingCell product =
          addend ? RoundingCell{target.low - addend->high.shift_left(scale_bits),
                                target.high - addend->low.shift_left(scale_bits)}
                 : target;
      return quotient_bounds(product, {x.domain, v, v, x.negative, false});
    }

    // Free addends c with v*m + 2^scale_bits*c in target, for m in the
    // multiplicand part (absent when both multiplicands alias the result).
    RoundingCell addend_bounds(const RoundingCell& target, const Integer& v,
                               const FinitePart* multiplicand,
                               unsigned int scale_bits) {
      using Word::Arithmetic::Global::ceil_shift_right;
      using Word::Arithmetic::Global::floor_shift_right;
      Integer low, high;
      if (multiplicand) {
        low = std::min(v * multiplicand->low, v * multiplicand->high);
        high = std::max(v * multiplicand->low, v * multiplicand->high);
      }
      return {ceil_shift_right(target.low - high, scale_bits),
              floor_shift_right(target.high - low, scale_bits)};
    }

    RoundingCell hull(const RoundingCell& a, const RoundingCell& b) {
      return {std::min(a.low, b.low), std::max(a.high, b.high)};
    }

    // The target T(v) of project_multiplicand_alias for the aliased code:
    // its product cell less the aliased addend and square terms.
    RoundingCell alias_target(const WideFormat& f, const Policy& policy,
                              WordValue code, unsigned int scale,
                              bool aliased_addend, bool square) {
      const Integer v = f.value(code);
      const Integer aliased =
          (aliased_addend ? v.shift_left(scale) : Integer()) +
          (square ? v * v : Integer());
      const RoundingCell cell = rounding_cell(f, policy, code, scale, CellEnds::Open);
      return {cell.low - aliased, cell.high - aliased};
    }

    // For result == a or b, round(a*b+c) must reproduce the aliased
    // multiplicand x itself. Projecting x and the result as independent
    // intervals moves x by one encoding per invocation, so x is split into
    // the classes of visit_alias_classes and each class is kept whole or
    // dropped. With scale = 2^(127+fraction_bits), the aliased terms move to
    // the target T(v) = cell(x) - [c aliased]*v*scale - [square]*v*v, leaving
    // the free terms v*m + scale*c in T(v). The cell offsets are constant
    // within a class, so each free bound is linear, bilinear or
    // scale + d/v in v. The square term v*scale - v*v peaks at x = 0.5, a
    // power of two with its own class, so it is monotone in every class. The
    // endpoint codes therefore bound the class. A free multiplicand and a free
    // addend sharing a view get separate projections whose publication
    // intersects them; this is sound but uncorrelated. Status is left to the
    // assigned-input enforcement, as in project_fma.
    void project_multiplicand_alias(const WideFormat& f, const Policy& policy,
                                    const WordView views[4],
                                    const Word::WordDomain domains[4],
                                    WideProjection projections[4]) {
      const int other = views[0] == views[3] ? 1 : 0;
      const bool square = views[other] == views[3];
      const bool aliased_addend = views[2] == views[3];
      const unsigned int scale_bits = 127 + f.fraction_bits;
      // Aliased roles iterate once with an absent part.
      const auto multiplicands =
          finite_parts(f, policy, views[other], domains[other], Role::Input);
      const auto addends =
          finite_parts(f, policy, views[2], domains[2], Role::Input);
      const size_t multiplicand_count = square ? 1 : multiplicands.size();
      const size_t addend_count = aliased_addend ? 1 : addends.size();
      // Once a free projection equals the hull of its whole domain,
      // inclusions no longer change it.
      WideProjection all[2];
      if (!square)
        all[0].include(views[other], domains[other]);
      if (!aliased_addend)
        all[1].include(views[2], domains[2]);
      WideProjection kept;
      visit_alias_classes(
          f, policy, views[3], domains[3], kept,
          [&](const FinitePart& x, WordValue first, WordValue last) {
            const Integer first_value = f.value(first),
                          last_value = f.value(last);
            const RoundingCell first_target = alias_target(
                f, policy, first, scale_bits, aliased_addend, square);
            const RoundingCell last_target = alias_target(
                f, policy, last, scale_bits, aliased_addend, square);
            if (square && aliased_addend) {
              const RoundingCell sum = hull(first_target, last_target);
              return sum.low <= Integer() && sum.high >= Integer();
            }
            // The addend bounds depend on the multiplicand part only and
            // the multiplicand bounds on the addend part only, so each is
            // computed once per class, when a pair first needs it.
            RoundingCell addend_range[6], multiplicand_range[6];
            bool addend_ready[6] = {}, multiplicand_ready[6] = {};
            bool supported = false;
            for (size_t i = 0; i < multiplicand_count; ++i)
              for (size_t j = 0; j < addend_count; ++j) {
                const FinitePart* m = square ? nullptr : &multiplicands[i];
                const FinitePart* c = aliased_addend ? nullptr : &addends[j];
                if (supported && (!m || projections[other].covers(all[0])) &&
                    (!c || projections[2].covers(all[1])))
                  return true;
                Word::WordDomain m_support, c_support;
                if (c) {
                  if (!addend_ready[i]) {
                    addend_range[i] = hull(
                        addend_bounds(first_target, first_value, m, scale_bits),
                        addend_bounds(last_target, last_value, m, scale_bits));
                    addend_ready[i] = true;
                  }
                  if (!real_domain(f, views[2], *c, addend_range[i].low,
                                   addend_range[i].high, c_support))
                    continue;
                }
                if (m) {
                  if (!multiplicand_ready[j]) {
                    multiplicand_range[j] =
                        hull(multiplicand_bounds(first_target, x, first_value,
                                                 c, scale_bits),
                             multiplicand_bounds(last_target, x, last_value, c,
                                                 scale_bits));
                    multiplicand_ready[j] = true;
                  }
                  if (!real_domain(f, views[other], *m,
                                   multiplicand_range[j].low,
                                   multiplicand_range[j].high, m_support))
                    continue;
                }
                supported = true;
                if (m && !projections[other].covers(all[0]))
                  projections[other].include(views[other], m_support);
                if (c && !projections[2].covers(all[1]))
                  projections[2].include(views[2], c_support);
              }
            return supported;
          },
          [&](const Word::WordDomain&) {
            if (!square)
              projections[other].unite(all[0]);
            if (!aliased_addend)
              projections[2].unite(all[1]);
          });
      projections[3] = kept;
      for (int i = 0; i < 3; ++i)
        if (views[i] == views[3])
          projections[i] = projections[3];
    }

    bool same_domain(const Word::WordDomain& x, const Word::WordDomain& y) {
      return x.cube.lo == y.cube.lo && x.cube.hi == y.cube.hi &&
             x.interval.minimum == y.interval.minimum &&
             x.interval.maximum == y.interval.maximum;
    }

    // Narrow the local domains to the hulls of projections exactly as
    // publish_projections narrows the views: positions sharing a view take
    // the intersection of their projections, closed as by
    // narrow_word_domain. Return false on failure; set changed when a domain
    // changes.
    bool narrow_locally(const WordView views[4],
                        const WideProjection projections[4],
                        Word::WordDomain domains[4], bool& changed) {
      for (int i = 0; i < 4; ++i) {
        int earlier = 0;
        while (earlier < i && views[earlier] != views[i])
          ++earlier;
        if (earlier < i) {
          domains[i] = domains[earlier];
          continue;
        }
        WideProjection shared = projections[i];
        for (int j = i + 1; j < 4; ++j)
          if (views[j] == views[i])
            shared.intersect(projections[j]);
        if (!shared.present)
          return false;
        const Word::WordDomain next = Word::intersect_word_domains(
            domains[i], {shared.cube, shared.interval});
        if ((next.cube.lo & ~next.cube.hi) != 0 ||
            next.interval.minimum > next.interval.maximum)
          return false;
        if (same_domain(next, domains[i]))
          continue;
        const Word::WordDomainResult closed = Word::compute_domain_closure(
            views[i].width(), views[i].domain_type(), next);
        if (!closed.is_consistent)
          return false;
        changed |= !same_domain(closed.domain, domains[i]);
        domains[i] = closed.domain;
      }
      return true;
    }

    /** \brief Wide fused multiply-add a*b+c, BF16 or FP32 or mixed
     *
     * Small domains are scanned exactly (scan_fma). Otherwise assigned inputs
     * are checked exactly and, while no input admits a NaN or an infinity,
     * the domains are projected.
     *
     * An addend aliasing a multiplicand (a == c, b == c, a == b == c), and
     * a == b, each with a result that aliases no input, are projected by
     * the closed form (project_addend_alias,
     * project_square_addend, project_square_sum): the exact hull of the
     * real relaxation of x*(y + 1), x*(x + 1) or x*x + c over a read of the
     * words, rounded inward to the admitted codes (x*(x + 1) and x*x + c
     * read x by its own codes). Known bits act only within each word. The
     * hull read takes each word from its least to its greatest code and is
     * used where its projection is a fixpoint of itself; elsewhere the
     * widened read takes each word one admitted code beyond its bounds,
     * whose projection is a fixpoint of itself while no word gains a bit
     * fixed by its interval (propagate_closed_form). The execution reports
     * ES_FIX after the last step, unless the inputs became assigned or the
     * scan now applies. Neither the cost of a step nor the number of steps
     * depends on the domains: a step is a constant number of pieces, each
     * with a constant number of products, quotients and bisections over the
     * codes of the format. Lattice reasoning across words (parities of
     * products, residues of the result) is not part of it.
     *
     * Every other shape takes the step rule (run_step_rule in
     * float-wide.hpp, propagate_step): four distinct words, a result that
     * aliases a multiplicand, the addend or several inputs, and a == b with
     * a result that aliases the addend. H is the hull projection of the
     * shape: project_fma for every operand, intersected for a == b with an
     * assigned addend that the result aliases with the bisection of
     * project_square, or, for a result aliasing an unassigned multiplicand,
     * the alias classes alone (project_multiplicand_alias), visiting only
     * the classes and part pairs that can hold a support. Where a
     * multiplicand part holds the single value m, project_fma takes the
     * exact extremes of the linear sum x*m + c (linear_extreme); otherwise
     * it bounds the terms by the cancellation bound on the lattice of the
     * exact sum. It then narrows the addend by its ends, binade by binade
     * with the known bits of the addend and the result
     * (narrow_addend_ends), and where both multiplicands are single codes,
     * to the hull of its supported codes (FixedProduct). Known bits of the
     * multiplicands narrow only by moving their bounds to admitted codes. W
     * is the widened projection of the real relation a*b + c
     * (project_widened_fma), with every role of an aliased word read from
     * the one domain of that word, widened once (see project_step). The
     * interval projections are taken where they reach a fixpoint of
     * themselves or make progress (halve the rank interval of a word or fix
     * a further bit of its interval). Interval reasoning on rounded bounds
     * is not idempotent: iterated on its own, it moves bounds slowly where
     * the supports form a lattice in the free words, and with a result
     * aliasing a multiplicand the free multiplicand and the addend would
     * narrow each other by one binade per pass. The step rule takes such
     * steps only while they make progress, so one execution settles in a
     * number of projections that the word widths and the special classes
     * bound, and reports ES_FIX at a fixpoint of the rule. A single propagator
     * therefore settles in a bounded number of steps, so slow convergence
     * can only come from several propagators sharing words.
     */
    class WideFma : public Propagator {
      WordView views[4];
      Int::IntView status;
      const bool has_status;
      const WideFormat source, destination;
      const Policy policy;

      // Whether the result aliases an unassigned multiplicand. A result
      // aliasing an assigned multiplicand holds that one value in both
      // roles, so the independent projections are exact for it.
      bool multiplicand_alias() const {
        return (views[3] == views[0] && !views[0].assigned()) ||
               (views[3] == views[1] && !views[1].assigned());
      }
      // Whether the result aliases a multiplicand whose local domain holds
      // more than one code, the domain version of multiplicand_alias, so
      // that H depends on the local domains alone.
      bool multiplicand_alias(const Word::WordDomain domains[4]) const {
        for (int i = 0; i < 2; ++i)
          if (views[3] == views[i] &&
              domains[i].interval.minimum != domains[i].interval.maximum)
            return true;
        return false;
      }

      bool first_occurrence(int i) const {
        for (int j = 0; j < i; ++j)
          if (views[i] == views[j])
            return false;
        return true;
      }
      bool assigned_inputs() const {
        return views[0].assigned() && views[1].assigned() &&
               views[2].assigned();
      }
      ExecStatus enforce_assigned_inputs(Space& home) {
        const auto outcome =
            evaluate_wide_fma(source, destination, policy, views[0].val(),
                              views[1].val(), views[2].val());
        if (has_status)
          GECODE_ME_CHECK(status.eq(home, outcome.status));
        GECODE_ME_CHECK(views[3].eq(home, outcome.code));
        return home.ES_SUBSUMED(*this);
      }
      ExecStatus scan(Space& home, const Word::WordDomain domains[4],
                      const ScanPlan& plan) {
        WideProjection projections[4];
        bool flags[32] = {};
        scan_fma(source, destination, policy, views, domains, plan,
                 has_status ? &status : nullptr, projections, flags);
        if (!publish_projections(home, views, projections, 4))
          return ES_FAILED;
        if (has_status) {
          int supported[32], count = 0;
          for (int flag = 0; flag < 32; ++flag)
            if (flags[flag])
              supported[count++] = flag;
          Iter::Values::Array values(supported, count);
          GECODE_ME_CHECK(status.inter_v(home, values, false));
        }
        if (assigned_inputs())
          return enforce_assigned_inputs(home);
        // Publication keeps every scanned support, so a scan is a fixpoint.
        return ES_FIX;
      }
      // Whether the closed form applies: the addend aliases a multiplicand
      // and the result neither, or a == b and the result aliases no input.
      // A result aliasing a multiplicand excludes it also where that
      // multiplicand is assigned.
      bool closed_form() const {
        if (views[3] == views[0] || views[3] == views[1])
          return false;
        if (views[2] == views[0] || views[2] == views[1])
          return true;
        return views[0] == views[1] && views[3] != views[2];
      }
      // Project domains onto projections by the hull projection of the step
      // rule, for every shape but those of the closed form. A result
      // aliasing a multiplicand with several codes in domains uses the
      // alias classes alone: project_fma treats the aliased operand and the
      // result as independent and would move them one encoding per pass.
      // Aliasing a multiplicand with one code, the result takes
      // project_fma, as that one value fills both roles. A == b with an
      // assigned addend that the result aliases (so that the result is
      // assigned too) intersects project_fma with project_square.
      void project(const Word::WordDomain domains[4],
                   WideProjection projections[4]) const {
        if (multiplicand_alias(domains)) {
          // Mixed formats cannot alias the result with a multiplicand.
          assert(source.fraction_bits == destination.fraction_bits);
          project_multiplicand_alias(destination, policy, views, domains,
                                     projections);
          return;
        }
        project_fma(source, destination, policy, views, domains, projections);
        if (views[0] == views[1] &&
            domains[2].interval.minimum == domains[2].interval.maximum) {
          WideProjection aliased[4];
          project_square(source, destination, policy, views, domains, aliased);
          for (int i = 0; i < 4; ++i)
            projections[i].intersect(aliased[i]);
        }
      }
      // Project a shape of the closed form with read onto domains, narrowed
      // as publication would narrow the views. Return false on failure; set
      // changed when a domain changes.
      bool project_closed_form(Read read, Word::WordDomain domains[4],
                               bool& changed) const {
        WideProjection projections[4];
        if (views[2] != views[0] && views[2] != views[1]) {
          project_square_sum(source, destination, policy, views, domains,
                             read, projections);
        } else {
          // Mixed formats cannot alias a multiplicand with the addend.
          assert(source.fraction_bits == destination.fraction_bits);
          if (views[0] == views[1])
            project_square_addend(destination, policy, views, domains, read,
                                  projections);
          else
            project_addend_alias(destination, policy, views, domains,
                                 views[2] == views[0] ? 0 : 1, read,
                                 projections);
        }
        return narrow_locally(views, projections, domains, changed);
      }
      // Give every position of a view the union of the projections of
      // all positions of that view.
      void unite_shared(WideProjection projections[4]) const {
        for (int i = 0; i < 4; ++i) {
          WideProjection shared = projections[i];
          for (int j = i + 1; j < 4; ++j)
            if (views[j] == views[i])
              shared.unite(projections[j]);
          for (int j = i; j < 4; ++j)
            if (views[j] == views[i])
              projections[j] = shared;
        }
      }
      // Narrow state by one projection of the step rule with read: the
      // widened projection W (project_widened_fma) for Read::Widened, and
      // for Read::Hull the projection H (project) intersected with W on the
      // hull read. The status is left to the scan and to assigned inputs.
      // Return false on failure.
      //
      // W reads every position from its local domain, and positions of one
      // word hold the same domain, so an aliased word is read once and that
      // one range serves every role it plays: a relaxation of the aliased
      // relation, in which the roles are independent words over the same
      // read, so W keeps every solution. The aliased word takes the union
      // of the projections of its roles, not their intersection. Each role
      // keeps the codes of its kept pieces, so every role reads them again
      // in the next widened read, and the widened pass stays a fixpoint of
      // itself until a word gains a bit of interval_bits or loses a special
      // class (the argument before Read in float-wide.hpp). An intersection
      // could drop codes that the pieces of another role keep, so that a
      // further pass would shrink those pieces again, step after step, like
      // the alias classes iterated alone. Tying the roles together is the
      // work of H. Hence the bound of the step rule in float-wide.hpp holds
      // for the aliased shapes as for four distinct words: at most
      // 2 * (sum of the widths) progress steps, and between two of them one
      // changing W step plus one per bit and special class, each followed
      // by at most three projections. H(D) is contained in W(D), as H is
      // intersected with the hull read of W and that read is contained in
      // the widened one. So a fixpoint D* of H is a fixpoint of W as well.
      bool project_step(Read read, StepState<4>& state) const {
        WideProjection projections[4];
        project_widened_fma(source, destination, policy, views, state.domains,
                            read, projections);
        unite_shared(projections);
        if (read == Read::Hull) {
          WideProjection hull[4];
          project(state.domains, hull);
          for (int i = 0; i < 4; ++i)
            projections[i].intersect(hull[i]);
        }
        bool changed = false;
        return narrow_locally(views, projections, state.domains, changed);
      }
      // Publish domains, settled by propagate_closed_form or the step rule,
      // and report ES_FIX unless the inputs are assigned, publication
      // changed the domains, or the scan now applies.
      ExecStatus publish_settled(Space& home, Word::WordDomain domains[4]) {
        for (int i = 0; i < 4; ++i)
          if (first_occurrence(i))
            GECODE_ME_CHECK(Word::narrow_word_domain(
                home, views[i], domains[i].cube, domains[i].interval));
        if (assigned_inputs())
          return enforce_assigned_inputs(home);
        // Publication closes like narrow_locally, so the views hold the
        // local fixpoint.
        bool same = true;
        for (int i = 0; i < 4; ++i) {
          const Word::WordDomain now = wide_snapshot(views[i]);
          same &= same_domain(now, domains[i]);
          domains[i] = now;
        }
        ScanPlan plan;
        if (plan_scan(views, domains, plan))
          return scan(home, domains, plan);
        return same ? ES_FIX : ES_NOFIX;
      }
      // The step rule (run_step_rule) on every shape but the closed forms.
      // H depends on the local domains alone (multiplicand_alias(domains)),
      // so a run from the published domains repeats the last one and
      // ES_FIX is honest also where the step rule assigns the aliased
      // multiplicand.
      ExecStatus propagate_step(Space& home, Word::WordDomain domains[4]) {
        StepState<4> state{{domains[0], domains[1], domains[2], domains[3]},
                           ~0U};
        if (run_step_rule<4>(*this, &WideFma::project_step, views, state) ==
            StepOutcome::Failed)
          return ES_FAILED;
        for (int i = 0; i < 4; ++i)
          domains[i] = state.domains[i];
        return publish_settled(home, domains);
      }
      // The closed form, on local domains D. A step
      // applies the hull projection H when H(D) is a fixpoint of H, and the
      // widened projection W otherwise; the steps repeat until one changes
      // nothing. Both projections are sound, and H(D) is contained in W(D).
      // The last step is a fixpoint of the step itself, so the execution
      // ends with ES_FIX: when D = H(D) or D = W(D) with H(D) != H(H(D)), the
      // step leaves D, and when D = H(D') is a fixpoint of H the next step
      // sees D = H(D). The number of steps does not depend on the domains: a
      // hull step that changes D ends the repetition after one more step,
      // and a widened step is a fixpoint of itself unless it added a bit to
      // interval_bits of some word (see the argument of the closed form),
      // which happens at most once per bit of the words.
      ExecStatus propagate_closed_form(Space& home,
                                       Word::WordDomain domains[4]) {
        for (;;) {
          Word::WordDomain hull[4];
          for (int i = 0; i < 4; ++i)
            hull[i] = domains[i];
          bool changed = false;
          if (!project_closed_form(Read::Hull, hull, changed))
            return ES_FAILED;
          if (!changed)
            break;
          Word::WordDomain again[4];
          for (int i = 0; i < 4; ++i)
            again[i] = hull[i];
          bool moved = false;
          if (!project_closed_form(Read::Hull, again, moved))
            return ES_FAILED;
          if (!moved) {
            for (int i = 0; i < 4; ++i)
              domains[i] = hull[i];
            break;
          }
          changed = false;
          if (!project_closed_form(Read::Widened, domains, changed))
            return ES_FAILED;
          if (!changed)
            break;
        }
        return publish_settled(home, domains);
      }

    public:
      WideFma(Home home, WordVar a, WordVar b, WordVar c, WordVar result,
              IntVar* flags, const Policy& p)
          : Propagator(home), has_status(flags != nullptr),
            source(a.domain_type()), destination(result.domain_type()),
            policy(p) {
        views[0] = a;
        views[1] = b;
        views[2] = c;
        views[3] = result;
        if (has_status)
          status = *flags;
        for (int i = 0; i < 4; ++i)
          if (first_occurrence(i))
            views[i].subscribe(home, *this, Word::PC_WORD_DOM);
        if (has_status)
          status.subscribe(home, *this, Int::PC_INT_DOM);
      }
      WideFma(Space& home, WideFma& other)
          : Propagator(home, other), has_status(other.has_status),
            source(other.source), destination(other.destination),
            policy(other.policy) {
        for (int i = 0; i < 4; ++i)
          views[i].update(home, other.views[i]);
        if (has_status)
          status.update(home, other.status);
      }
      Actor* copy(Space& home) override {
        return new (home) WideFma(home, *this);
      }
      size_t dispose(Space& home) override {
        for (int i = 0; i < 4; ++i)
          if (first_occurrence(i))
            views[i].cancel(home, *this, Word::PC_WORD_DOM);
        if (has_status)
          status.cancel(home, *this, Int::PC_INT_DOM);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      // Cost by path: assigned inputs only check the result. Every other
      // path does a number of projections that the widths bound. A result
      // aliasing an unassigned multiplicand visits, in each H, the alias
      // classes of the result for each pair of parts of the free words,
      // which grows with the square of the width. A result aliasing the
      // addend narrows the addend in each H binade by binade with the known
      // bits of the addend and the result, the most work per projection.
      // The closed forms (an addend aliasing a multiplicand, a == b) and
      // the step rule on the other shapes project over the codes of the
      // format.
      PropCost cost(const Space&, const ModEventDelta&) const override {
        if (assigned_inputs())
          return PropCost::binary(PropCost::LO);
        if (multiplicand_alias())
          return PropCost::quadratic(PropCost::HI, views[3].width());
        if (views[2] == views[3])
          return PropCost::cubic(PropCost::HI, views[3].width());
        return PropCost::linear(PropCost::HI, views[3].width());
      }
      void reschedule(Space& home) override {
        for (int i = 0; i < 4; ++i)
          if (first_occurrence(i))
            views[i].reschedule(home, *this, Word::PC_WORD_DOM);
        if (has_status)
          status.reschedule(home, *this, Int::PC_INT_DOM);
      }
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        if (assigned_inputs())
          return enforce_assigned_inputs(home);
        Word::WordDomain domains[] = {
            wide_snapshot(views[0]), wide_snapshot(views[1]),
            wide_snapshot(views[2]), wide_snapshot(views[3])};
        ScanPlan plan;
        if (plan_scan(views, domains, plan))
          return scan(home, domains, plan);
        // Without a scan the basic projections apply, and none while an
        // input admits a NaN or an infinity.
        if (admits_nonfinite(source, views[0], domains[0]) ||
            admits_nonfinite(source, views[1], domains[1]) ||
            admits_nonfinite(destination, views[2], domains[2]))
          return ES_FIX;
        if (closed_form())
          return propagate_closed_form(home, domains);
        return propagate_step(home, domains);
      }
    };
  } // namespace
} // namespace Gecode

// STATISTICS: word-prop
