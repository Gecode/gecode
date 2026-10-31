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

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    using Word::Float::TinyResult;

    // Decode a finite magnitude in minimum-subnormal units.
    int64_t tiny_units(int magnitude, unsigned int fraction_bits) {
      const unsigned int exponent =
          static_cast<unsigned int>(magnitude) >> fraction_bits;
      const unsigned int fraction =
          static_cast<unsigned int>(magnitude) & ((1U << fraction_bits) - 1);
      return exponent == 0 ? fraction
                           : int64_t((1U << fraction_bits) + fraction)
                                 << (exponent - 1);
    }

    TinyResult evaluate_fma(FloatWordFormat format, const Policy& policy,
                            int a, int b, int c) {
      const int sign = format == FWF_E2M1 ? 8 : 128;
      const unsigned int fraction_bits = format == FWF_E2M1 ? 1 : 3;
      int magnitudes[] = {a & (sign - 1), b & (sign - 1), c & (sign - 1)};
      const int operands[] = {a, b, c};
      // Both E4M3FN NaNs are quiet; select from original operand order.
      if (format == FWF_E4M3FN)
        for (int i = 0; i < 3; ++i)
          if (magnitudes[i] == 127)
            return {policy.nan == Policy::Canonical ? 127 : operands[i], 0};
      if (policy.input_subnormals == Policy::SignedZero)
        for (int& magnitude : magnitudes)
          if (magnitude < (1 << fraction_bits))
            magnitude = 0;
      const bool product_negative = ((a ^ b) & sign) != 0;
      const bool addend_negative = (c & sign) != 0;
      const int64_t scale = format == FWF_E2M1 ? 2 : 512;
      const int64_t product = tiny_units(magnitudes[0], fraction_bits) *
                              tiny_units(magnitudes[1], fraction_bits);
      const int64_t addend = tiny_units(magnitudes[2], fraction_bits) * scale;
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

    class TinyFma : public Propagator {
      static const unsigned int assignment_limit = 32768;
      Int::IntView views[5];
      int columns[5];
      int column_count, view_count, input_count;
      const FloatWordFormat format;
      const Policy policy;

    public:
      TinyFma(Home home, const IntVarArgs& variables, FloatWordFormat f,
              const Policy& p)
          : Propagator(home), column_count(variables.size()), view_count(0),
            input_count(0), format(f), policy(p) {
        for (int i = 0; i < column_count; ++i) {
          Int::IntView view(variables[i]);
          int j = 0;
          while (j < view_count && views[j] != view)
            ++j;
          columns[i] = j;
          if (j == view_count)
            views[view_count++] = view;
          if (i == 2)
            input_count = view_count;
        }
        // Subscriptions can call cost(); initialize all mappings first.
        for (int i = 0; i < view_count; ++i)
          views[i].subscribe(home, *this, Int::PC_INT_DOM);
      }
      TinyFma(Space& home, TinyFma& other)
          : Propagator(home, other), column_count(other.column_count),
            view_count(other.view_count), input_count(other.input_count),
            format(other.format), policy(other.policy) {
        for (int i = 0; i < column_count; ++i)
          columns[i] = other.columns[i];
        for (int i = 0; i < view_count; ++i)
          views[i].update(home, other.views[i]);
      }
      virtual Actor* copy(Space& home) {
        return new (home) TinyFma(home, *this);
      }
      virtual size_t dispose(Space& home) {
        for (int i = 0; i < view_count; ++i)
          views[i].cancel(home, *this, Int::PC_INT_DOM);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      virtual PropCost cost(const Space&, const ModEventDelta&) const {
        unsigned int size = 0;
        for (int i = 0; i < input_count; ++i)
          size += views[i].size();
        return PropCost::cubic(PropCost::HI, size);
      }
      virtual void reschedule(Space& home) {
        for (int i = 0; i < view_count; ++i)
          views[i].reschedule(home, *this, Int::PC_INT_DOM);
      }
      virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        unsigned int assignments = 1;
        for (int i = 0; i < input_count; ++i) {
          if (views[i].size() > assignment_limit / assignments)
            return ES_FIX;
          assignments *= views[i].size();
        }
        bool supports[5][256] = {};
        bool found = false;
        Int::ViewValues<Int::IntView> inputs[3];
        for (int i = 0; i < input_count; ++i)
          inputs[i].init(views[i]);
        for (;;) {
          const int a = inputs[columns[0]].val();
          const int b = inputs[columns[1]].val();
          const int c = inputs[columns[2]].val();
          const TinyResult result = evaluate_fma(format, policy, a, b, c);
          const int tuple[] = {a, b, c, result.code, result.status};
          bool allowed = true;
          for (int i = 0; i < column_count && allowed; ++i) {
            allowed = views[columns[i]].in(tuple[i]);
            for (int j = 0; j < i; ++j)
              if (columns[i] == columns[j] && tuple[i] != tuple[j])
                allowed = false;
          }
          if (allowed) {
            found = true;
            for (int i = 0; i < column_count; ++i)
              supports[columns[i]][tuple[i]] = true;
          }
          // The first input_count views are precisely the distinct inputs.
          int i = 0;
          for (; i < input_count; ++i) {
            ++inputs[i];
            if (inputs[i]())
              break;
            inputs[i].init(views[i]);
          }
          if (i == input_count)
            break;
        }
        if (!found)
          return ES_FAILED;
        bool assigned = true;
        for (int i = 0; i < view_count; ++i) {
          int supported[256], count = 0;
          for (Int::ViewValues<Int::IntView> value(views[i]); value();
               ++value)
            if (supports[i][value.val()])
              supported[count++] = value.val();
          Iter::Values::Array values(supported, count);
          GECODE_ME_CHECK(views[i].inter_v(home, values, false));
          assigned &= views[i].assigned();
        }
        // Each retained tuple supports every retained coordinate, so the
        // simultaneous projection is a fixpoint, including aliases.
        return assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
    };

    using Word::WordView;
    using Word::Float::admits_nonfinite;
    using Word::Float::exact_product;
    using Word::Float::ExactDyadic;
    using Word::Float::finite_parts;
    using Word::Float::FinitePart;
    using Word::Float::include_real;
    using Word::Float::include_rounded;
    using Word::Float::Integer;
    using Word::Float::invalid;
    using Word::Float::product_cell;
    using Word::Float::quotient_bounds;
    using Word::Float::round_dyadic;
    using Word::Float::RoundingCell;
    using Word::Float::visit_alias_classes;
    using Word::Float::wide_intersect;
    using Word::Float::wide_snapshot;
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
          destination.value(effective[2]) *
              Integer::power_two(static_cast<unsigned int>(addend_shift));
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

    // Number of encodings a cube/rank domain admits, saturated at limit.
    unsigned int admitted_count(WordView view, const Word::WordDomain& domain,
                                unsigned int limit) {
      const auto ordered =
          Word::order_cube(view.domain_type(), view.width(), domain.cube);
      const auto admitted =
          Word::find_admitted_interval(ordered, domain.interval, view.mask());
      if (!admitted.is_admitted)
        return 0;
      const WordValue free = (ordered.hi & ~ordered.lo) & view.mask();
      const WordValue span = member_index(admitted.interval.maximum, free) -
                             member_index(admitted.interval.minimum, free);
      return span >= limit ? limit : static_cast<unsigned int>(span) + 1;
    }

    // Store the encodings a cube/rank domain admits, in ascending rank order.
    void collect_admitted_encodings(WordView view,
                                    const Word::WordDomain& domain,
                                    WordValue encodings[]) {
      const auto ordered =
          Word::order_cube(view.domain_type(), view.width(), domain.cube);
      WordValue lower = domain.interval.minimum, candidate;
      unsigned int size = 0;
      while (Word::cube_successor(ordered.lo, ordered.hi, lower, view.mask(),
                                  candidate) &&
             candidate <= domain.interval.maximum) {
        encodings[size++] =
          Word::rank(view.domain_type(), view.width(), candidate);
        if (candidate == domain.interval.maximum)
          break;
        lower = candidate + 1;
      }
    }

    // Return false only when the distinct-input product exceeds the budget,
    // which is decided before any enumeration. An empty supported relation is
    // represented by absent projections.
    bool scan_fma(const WideFormat& source, const WideFormat& destination,
                  const Policy& policy, const WordView views[4],
                  const Word::WordDomain domains[4], const Int::IntView* status,
                  WideProjection projections[4], bool flags[32]) {
      static constexpr unsigned int assignment_budget = 2048;
      unsigned int sizes[3];
      int columns[3], inputs[3], input_count = 0;
      unsigned int assignments = 1;
      for (int i = 0; i < 3; ++i) {
        int j = 0;
        while (j < input_count && views[i] != views[inputs[j]])
          ++j;
        columns[i] = j;
        if (j < input_count)
          continue;
        inputs[input_count++] = i;
        const unsigned int limit = assignment_budget / assignments;
        sizes[j] = admitted_count(views[i], domains[i], limit + 1);
        if (sizes[j] > limit)
          return false;
        assert(sizes[j] > 0);
        assignments *= sizes[j];
      }
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
          return true;
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
                            const Integer& addend_scale,
                            WideProjection projections[4]) {
      const WordValue sign = c.negative ? destination.sign : 0;
      WordValue first = Word::rank(views[2].domain_type(), views[2].width(),
                                   c.domain.interval.minimum) &
                        (destination.sign - 1);
      const WordValue last =
          Word::rank(views[2].domain_type(), views[2].width(),
                     c.domain.interval.maximum) &
          (destination.sign - 1);
      while (first <= last) {
        // Saturation and overflow can give maximum finite an unbounded cell.
        if (first == destination.maximum) {
          const auto maximum =
              wide_intersect(views[2], c.domain, sign | first, sign | first);
          const Integer center = destination.value(sign | first) * addend_scale;
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
          const WordValue code =
              Word::rank(views[2].domain_type(), views[2].width(),
                         admitted.domain.interval.minimum);
          auto residual = product_cell(destination, policy, code, addend_scale);
          const Integer center = destination.value(code) * addend_scale;
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

    void project_fma(const WideFormat& source, const WideFormat& destination,
                     const Policy& policy, const WordView views[4],
                     const Word::WordDomain domains[4],
                     WideProjection projections[4]) {
      const auto as = finite_parts(source, policy, views[0], domains[0], true);
      const auto bs = finite_parts(source, policy, views[1], domains[1], true);
      const auto cs =
          finite_parts(destination, policy, views[2], domains[2], true);
      const int sum_exponent =
          -2 * (127 + static_cast<int>(source.fraction_bits));
      const int addend_shift =
          -sum_exponent - 127 - static_cast<int>(destination.fraction_bits);
      assert(addend_shift >= 0);
      const Integer addend_scale =
          Integer::power_two(static_cast<unsigned int>(addend_shift));
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      for (const auto& a : as)
        for (const auto& b : bs) {
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
                                 {product_low, product_high}, addend_scale,
                                 projections);
              continue;
            }
            const Integer low = product_low + addend_scale * c.low;
            const Integer high = product_high + addend_scale * c.high;
            WideProjection forward;
            include_rounded(
                destination, forward, views[3], domains[3],
                round_dyadic(destination, policy, {low, sum_exponent, false})
                    .code,
                round_dyadic(destination, policy, {high, sum_exponent, false})
                    .code);
            if (!forward.present)
              continue;
            const Word::WordDomain candidate{forward.cube, forward.interval};
            // Maximum-finite and infinity tails can have unbounded preimages.
            // Forward compatibility has already been checked against Word.
            bool tail = false;
            for (WordValue sign : {WordValue(0), destination.sign})
              for (WordValue magnitude :
                   {destination.maximum, destination.infinity})
                tail |= wide_intersect(views[3], candidate, sign | magnitude,
                                       sign | magnitude)
                            .is_consistent;
            if (tail) {
              projections[0].include(views[0], a.domain);
              projections[1].include(views[1], b.domain);
              projections[2].include(views[2], c.domain);
              projections[3].include(views[3], candidate);
              continue;
            }
            for (const auto& z : finite_parts(destination, policy, views[3],
                                              candidate, false)) {
              const WordValue first =
                  Word::rank(views[3].domain_type(), views[3].width(),
                             z.domain.interval.minimum);
              const WordValue last =
                  Word::rank(views[3].domain_type(), views[3].width(),
                             z.domain.interval.maximum);
              const RoundingCell cell{
                  std::max(low,
                           product_cell(destination, policy,
                                        z.negative ? last : first, addend_scale)
                               .low),
                  std::min(high,
                           product_cell(destination, policy,
                                        z.negative ? first : last, addend_scale)
                               .high)};
              if (cell.low > cell.high)
                continue;
              WideProjection inputs[3];
              include_real(destination, inputs[2], views[2], c,
                           ceil_div(cell.low - product_high, addend_scale),
                           floor_div(cell.high - product_low, addend_scale));
              const RoundingCell product{cell.low - addend_scale * c.high,
                                         cell.high - addend_scale * c.low};
              project_product_inputs(source, views, a, b, product, inputs);
              if (!inputs[0].present || !inputs[1].present ||
                  !inputs[2].present)
                continue;
              for (int i = 0; i < 3; ++i)
                projections[i].include(views[i],
                                       {inputs[i].cube, inputs[i].interval});
              projections[3].include(views[3], z.domain);
            }
          }
        }
    }

    // Free multiplicands m with v*m + scale*c in target, for v != 0 and c in
    // the addend part (absent when the addend aliases the result).
    RoundingCell multiplicand_bounds(const RoundingCell& target,
                                     const FinitePart& x, const Integer& v,
                                     const FinitePart* addend,
                                     const Integer& scale) {
      const RoundingCell product =
          addend ? RoundingCell{target.low - scale * addend->high,
                                target.high - scale * addend->low}
                 : target;
      return quotient_bounds(product, {x.domain, v, v, x.negative, false});
    }

    // Free addends c with v*m + scale*c in target, for m in the multiplicand
    // part (absent when both multiplicands alias the result).
    RoundingCell addend_bounds(const RoundingCell& target, const Integer& v,
                               const FinitePart* multiplicand,
                               const Integer& scale) {
      using Word::Arithmetic::Global::ceil_div;
      using Word::Arithmetic::Global::floor_div;
      Integer low, high;
      if (multiplicand) {
        low = std::min(v * multiplicand->low, v * multiplicand->high);
        high = std::max(v * multiplicand->low, v * multiplicand->high);
      }
      return {ceil_div(target.low - high, scale),
              floor_div(target.high - low, scale)};
    }

    RoundingCell hull(const RoundingCell& a, const RoundingCell& b) {
      return {std::min(a.low, b.low), std::max(a.high, b.high)};
    }

    // The target T(v) of project_multiplicand_alias for the aliased code:
    // its product cell less the aliased addend and square terms.
    RoundingCell alias_target(const WideFormat& f, const Policy& policy,
                              WordValue code, const Integer& scale,
                              bool aliased_addend, bool square) {
      const Integer v = f.value(code);
      const Integer aliased = (aliased_addend ? v * scale : Integer()) +
                              (square ? v * v : Integer());
      const RoundingCell cell = product_cell(f, policy, code, scale);
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
      const Integer scale = Integer::power_two(127 + f.fraction_bits);
      // Aliased roles iterate once with an absent part.
      const auto multiplicands =
          finite_parts(f, policy, views[other], domains[other], true);
      const auto addends = finite_parts(f, policy, views[2], domains[2], true);
      const size_t multiplicand_count = square ? 1 : multiplicands.size();
      const size_t addend_count = aliased_addend ? 1 : addends.size();
      visit_alias_classes(
          f, policy, views[3], domains[3],
          [&](const FinitePart& x, WordValue first, WordValue last) {
            const Integer first_value = f.value(first),
                          last_value = f.value(last);
            const RoundingCell first_target = alias_target(
                f, policy, first, scale, aliased_addend, square);
            const RoundingCell last_target = alias_target(
                f, policy, last, scale, aliased_addend, square);
            if (square && aliased_addend) {
              const RoundingCell sum = hull(first_target, last_target);
              if (sum.low <= Integer() && sum.high >= Integer())
                projections[3].include(views[3], x.domain);
              return;
            }
            for (size_t i = 0; i < multiplicand_count; ++i)
              for (size_t j = 0; j < addend_count; ++j) {
                const FinitePart* m = square ? nullptr : &multiplicands[i];
                const FinitePart* c = aliased_addend ? nullptr : &addends[j];
                WideProjection m_support, c_support;
                if (m) {
                  const auto bounds =
                      hull(multiplicand_bounds(first_target, x, first_value, c,
                                               scale),
                           multiplicand_bounds(last_target, x, last_value, c,
                                               scale));
                  include_real(f, m_support, views[other], *m, bounds.low,
                               bounds.high);
                  if (!m_support.present)
                    continue;
                }
                if (c) {
                  const auto bounds =
                      hull(addend_bounds(first_target, first_value, m, scale),
                           addend_bounds(last_target, last_value, m, scale));
                  include_real(f, c_support, views[2], *c, bounds.low,
                               bounds.high);
                  if (!c_support.present)
                    continue;
                }
                projections[3].include(views[3], x.domain);
                if (m)
                  projections[other].include(
                      views[other], {m_support.cube, m_support.interval});
                if (c)
                  projections[2].include(views[2],
                                         {c_support.cube, c_support.interval});
              }
          },
          [&](const Word::WordDomain& tail) {
            projections[3].include(views[3], tail);
            if (!square)
              projections[other].include(views[other], domains[other]);
            if (!aliased_addend)
              projections[2].include(views[2], domains[2]);
          });
      for (int i = 0; i < 3; ++i)
        if (views[i] == views[3])
          projections[i] = projections[3];
    }

    class WideFma : public Propagator {
      WordView views[4];
      Int::IntView status;
      const bool has_status;
      const WideFormat source, destination;
      const Policy policy;

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

    public:
      WideFma(Home home, FloatWordVar a, FloatWordVar b, FloatWordVar c,
              FloatWordVar result, IntVar* flags, const Policy& p)
          : Propagator(home), has_status(flags != nullptr), source(a.format()),
            destination(result.format()), policy(p) {
        views[0] = a.bits();
        views[1] = b.bits();
        views[2] = c.bits();
        views[3] = result.bits();
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
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::cubic(PropCost::HI, 3);
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
        const Word::WordDomain domains[] = {
            wide_snapshot(views[0]), wide_snapshot(views[1]),
            wide_snapshot(views[2]), wide_snapshot(views[3])};
        WideProjection projections[4];
        bool flags[32] = {};
        const bool scanned =
            scan_fma(source, destination, policy, views, domains,
                     has_status ? &status : nullptr, projections, flags);
        if (!scanned) {
          if (admits_nonfinite(source, views[0], domains[0]) ||
              admits_nonfinite(source, views[1], domains[1]) ||
              admits_nonfinite(destination, views[2], domains[2]))
            return ES_FIX;
          if (views[3] == views[0] || views[3] == views[1]) {
            // Mixed formats cannot alias the result with a multiplicand.
            assert(source.fraction_bits == destination.fraction_bits);
            project_multiplicand_alias(destination, policy, views, domains,
                                       projections);
          } else {
            project_fma(source, destination, policy, views, domains,
                        projections);
          }
        }
        if (!publish_projections(home, views, projections, 4))
          return ES_FAILED;
        if (scanned && has_status) {
          int supported[32], count = 0;
          for (int flag = 0; flag < 32; ++flag)
            if (flags[flag])
              supported[count++] = flag;
          Iter::Values::Array values(supported, count);
          GECODE_ME_CHECK(status.inter_v(home, values, false));
        }
        if (assigned_inputs())
          return enforce_assigned_inputs(home);
        if (scanned) {
          // Publication keeps every scanned support, so only a skipped move
          // can leave the scan short of its fixpoint.
          for (int i = 0; i < 4; ++i)
            if (projections[i].skipped(views[i]))
              return ES_NOFIX;
          return ES_FIX;
        }
        for (int i = 0; i < 4; ++i) {
          const auto now = wide_snapshot(views[i]);
          if (now.cube.lo != domains[i].cube.lo ||
              now.cube.hi != domains[i].cube.hi ||
              now.interval.minimum != domains[i].interval.minimum ||
              now.interval.maximum != domains[i].interval.maximum)
            return ES_NOFIX;
        }
        return ES_FIX;
      }
    };

    void post_fma(Home home, const Policy& policy, FloatWordVar a,
                  FloatWordVar b, FloatWordVar c, FloatWordVar result,
                  IntVar* status) {
      Word::Float::check(policy, "fp_fma");
      const bool same = a.format() == b.format() && a.format() == c.format() &&
                        a.format() == result.format();
      const bool mixed = a.format() == FWF_BF16 && b.format() == FWF_BF16 &&
                         c.format() == FWF_FP32 && result.format() == FWF_FP32;
      if (!same && !mixed)
        throw Word::FormatMismatch("fp_fma");
      if (a.format() == FWF_BF16 || a.format() == FWF_FP32) {
        GECODE_POST;
        if (status != nullptr) {
          Int::IntView flags(*status);
          GECODE_ME_FAIL(flags.gq(home, 0));
          GECODE_ME_FAIL(flags.lq(home, 31));
        }
        (void)new (home) WideFma(home, a, b, c, result, status, policy);
        return;
      }
      if (a.format() != FWF_E2M1 && a.format() != FWF_E4M3FN)
        throw Word::OutOfLimits("fp_fma");
      IntVarArgs variables(status == nullptr ? 4 : 5);
      variables[0] = a.encoding();
      variables[1] = b.encoding();
      variables[2] = c.encoding();
      variables[3] = result.encoding();
      if (status != nullptr)
        variables[4] = *status;
      GECODE_POST;
      if (status != nullptr) {
        Int::IntView flags(*status);
        GECODE_ME_FAIL(flags.gq(home, 0));
        GECODE_ME_FAIL(flags.lq(home, 31));
      }
      (void)new (home) TinyFma(home, variables, a.format(), policy);
    }
  } // namespace

  void fp_fma(Home home, const FloatWordPolicy& policy, FloatWordVar a,
              FloatWordVar b, FloatWordVar c, FloatWordVar result) {
    post_fma(home, policy, a, b, c, result, nullptr);
  }
  void fp_fma(Home home, const FloatWordPolicy& policy, FloatWordVar a,
              FloatWordVar b, FloatWordVar c, FloatWordVar result,
              IntVar status) {
    post_fma(home, policy, a, b, c, result, &status);
  }
} // namespace Gecode

// STATISTICS: word-post
