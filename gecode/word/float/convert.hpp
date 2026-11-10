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
#include <gecode/float-word.hh>
#include <gecode/word/float.hpp>

namespace Gecode {
  namespace Word {
    namespace {
      using Rounding = FloatWordPolicy::Rounding;

      WordDomain snapshot(WordView view) {
        return {{view.lo(), view.hi()},
                {view.bounded() ? view.rank_minimum() : view.lo(),
                 view.bounded() ? view.rank_maximum() : view.hi()}};
      }

      // Each encoding range lies within one sign half, so its ranks form one
      // interval.
      WordDomainResult intersect(WordView view, WordDomain domain,
                                 WordEncodingRange range) {
        const WordRankInterval ranks = rank_interval(
            view.domain_type(), view.width(), range.first, range.last);
        domain.interval.minimum =
            std::max(domain.interval.minimum, ranks.minimum);
        domain.interval.maximum =
            std::min(domain.interval.maximum, ranks.maximum);
        if (domain.interval.minimum > domain.interval.maximum)
          return {false, {{0, 0}, {0, 0}}};
        return compute_domain_closure(view.width(), view.domain_type(), domain);
      }

      // Iterates the encodings a cube/interval domain of a view admits, in
      // ascending rank order. Each step costs a constant number of bit
      // operations.
      class AdmittedEncodings {
        WordDomainType domain_type;
        unsigned int width;
        WordCube cube;
        WordValue current, last;
        bool is_valid;

      public:
        AdmittedEncodings(WordView view, WordDomain domain)
            : domain_type(view.domain_type()), width(view.width()),
              cube(domain.cube), last(domain.interval.maximum) {
          is_valid = next_admitted_rank(domain_type, width, cube,
                                        domain.interval.minimum, current) &&
                     current <= last;
        }
        bool operator()() const { return is_valid; }
        void operator++() {
          is_valid = current < last &&
                     next_admitted_rank(domain_type, width, cube, current + 1,
                                        current) &&
                     current <= last;
        }
        WordValue val() const {
          return encode_rank(domain_type, width, current);
        }
      };

      struct Projection {
        bool present = false;
        WordCube cube{~WordValue(0), 0};
        WordRankInterval interval{~WordValue(0), 0};

        // Hulls of two projections of one view bound their intersection.
        void intersect(const Projection& other) {
          present = present && other.present;
          cube.lo |= other.cube.lo;
          cube.hi &= other.cube.hi;
          interval.minimum = std::max(interval.minimum, other.interval.minimum);
          interval.maximum = std::min(interval.maximum, other.interval.maximum);
        }

        void add(WordCube bits, WordRankInterval bounds) {
          present = true;
          cube.lo &= bits.lo;
          cube.hi |= bits.hi;
          interval.minimum = std::min(interval.minimum, bounds.minimum);
          interval.maximum = std::max(interval.maximum, bounds.maximum);
        }

        // Narrow view to the cube and interval of the supports.
        bool publish(Space& home, WordView view) const {
          return present &&
                 !me_failed(narrow_word_domain(home, view, cube, interval));
        }
      };

      using Policy = FloatWordPolicy;

      using Format = Float::FormatFacts;
      using Float::compare;
      using Float::decode;
      using Float::Dyadic;
      using Float::inexact;
      using Float::invalid;
      using Float::MagnitudeRounding;
      using Float::overflow;
      using Float::underflow;

      // Round significand * 2^exponent in units of 2^unit. All nonzero
      // significands have at most 24 bits; a very large right shift rounds 0
      // or 1.
      WordValue round_units(Dyadic value, int unit, bool negative,
                            Rounding mode) {
        const int shift = unit - value.exponent;
        if (shift <= 0) {
          assert(-shift < 64);
          return value.significand << -shift;
        }
        const WordValue quotient = shift >= 64 ? 0 : value.significand >> shift;
        const WordValue remainder =
            shift >= 64 ? value.significand
                        : value.significand & ((WordValue(1) << shift) - 1);
        const MagnitudeRounding rounding =
            Float::magnitude_rounding(mode, negative);
        bool increment = remainder && rounding == MagnitudeRounding::Up;
        if (rounding == MagnitudeRounding::Nearest && shift < 64) {
          const WordValue half = WordValue(1) << (shift - 1);
          increment = remainder > half || (remainder == half && (quotient & 1));
        }
        return quotient + increment;
      }

      struct Conversion {
        WordValue bits;
        int flags;
        bool tiny = false;
      };

      WordValue overflow_bits(bool negative, const Format& to,
                              const Policy& policy) {
        const WordValue sign = negative ? to.sign() : 0;
        if (policy.overflow == Policy::SaturateFinite || !to.has_nan())
          return sign | to.maximum;
        if (!to.has_infinity())
          return (policy.nan == Policy::Canonical ? 0 : sign) |
                 to.canonical_nan();
        return sign |
               (Float::overflows_to_infinity(policy, negative) ? to.infinity
                                                               : to.maximum);
      }

      Conversion convert(WordValue bits, const Format& from, const Format& to,
                         const Policy& policy) {
        const bool negative = (bits & from.sign()) != 0;
        const WordValue sign = negative ? to.sign() : 0;
        const WordValue magnitude = bits & (from.sign() - 1);
        if (from.is_nan(magnitude)) {
          if (!to.has_nan())
            return {to.maximum, invalid};
          const bool signaling =
              from.has_infinity() && !(magnitude & from.quiet());
          if (policy.nan == Policy::Canonical)
            return {to.canonical_nan(), signaling ? invalid : 0};
          WordValue payload =
              from.has_infinity() ? magnitude & (from.quiet() - 1) : 0;
          if (from.fraction_bits > to.fraction_bits)
            payload >>= from.fraction_bits - to.fraction_bits;
          else
            payload <<= to.fraction_bits - from.fraction_bits;
          return {sign | to.canonical_nan() | (to.has_infinity() ? payload : 0),
                  signaling ? invalid : 0};
        }
        if (from.infinity && magnitude == from.infinity)
          return to.infinity ? Conversion{sign | to.infinity, 0}
                             : Conversion{overflow_bits(negative, to, policy),
                                          overflow | inexact};
        if (!magnitude || (policy.input_subnormals == Policy::SignedZero &&
                           magnitude < from.normal()))
          return {policy.zeros == Policy::Positive ? 0 : sign, 0};

        const Dyadic exact = decode(from, magnitude);
        const int leading =
            63 - Support::count_leading_zeros_64(exact.significand);
        const int precision_unit =
            exact.exponent + leading - static_cast<int>(to.fraction_bits);
        const Dyadic precision{
            round_units(exact, precision_unit, negative, policy.rounding),
            precision_unit};
        const bool tiny =
            compare(policy.tininess == Policy::BeforeRounding ? exact
                                                              : precision,
                    {1, 1 - to.bias}) < 0;
        if (compare(precision, decode(to, to.maximum)) > 0)
          return {overflow_bits(negative, to, policy), overflow | inexact,
                  tiny};
        const int unit = std::max(
            precision_unit, 1 - to.bias - static_cast<int>(to.fraction_bits));
        WordValue rounded = round_units(exact, unit, negative, policy.rounding);
        int flags = compare(exact, {rounded, unit}) == 0
                        ? 0
                        : (tiny ? underflow | inexact : inexact);
        WordValue result;
        if (rounded < to.normal()) {
          result = rounded;
        } else {
          int exponent = unit + static_cast<int>(to.fraction_bits);
          if (rounded >= (to.normal() << 1)) {
            rounded >>= 1;
            ++exponent;
          }
          result = (WordValue(exponent + to.bias) << to.fraction_bits) |
                   (rounded - to.normal());
        }
        if (policy.output_subnormals == Policy::SignedZero && result &&
            result < to.normal()) {
          result = 0;
          flags = underflow | inexact;
        }
        return {result |
                    ((!result && policy.zeros == Policy::Positive) ? 0 : sign),
                flags, tiny};
      }

      // Return the first encoding satisfying a monotone predicate, or last+1.
      template <class Predicate>
      WordValue first_true(WordEncodingRange range, Predicate predicate) {
        WordValue lower = range.first, upper = range.last + 1;
        while (lower < upper) {
          const WordValue middle = lower + (upper - lower) / 2;
          if (predicate(middle))
            upper = middle;
          else
            lower = middle + 1;
        }
        return lower;
      }

      class MixedConversion : public BinaryPropagator<WordView, PC_WORD_DOM> {
        using Base = BinaryPropagator<WordView, PC_WORD_DOM>;
        const Format from, to;
        const Policy policy;
        // Optional status; a null view when the posting has none.
        Int::IntView status;
        // The source magnitudes whose results vary lie from first_varying to
        // last_varying (see cost).
        const WordValue first_varying, last_varying;

        bool has_status() const { return status.varimp() != nullptr; }
        bool admits_status(int flags) const {
          return !has_status() || status.in(flags);
        }

        // The hull of the supports of each view, and for a view of width at
        // most eight also their exact values.
        struct Supports {
          Projection source, result;
          WordValueSet source_values = {{0, 0, 0, 0}},
                       result_values = {{0, 0, 0, 0}};
          unsigned int statuses = 0;
        };

        void support_pair(Supports& supports, WordValue source,
                          WordValue result, int flags) const {
          if (!x1.in(result) || !admits_status(flags) ||
              ((x0 == x1) && source != result))
            return;
          const WordValue r = rank(x0.domain_type(), from.width, source);
          supports.source.add({source, source}, {r, r});
          const WordValue t = rank(x1.domain_type(), to.width, result);
          supports.result.add({result, result}, {t, t});
          if (from.width <= 8)
            supports.source_values = with_value(supports.source_values, source);
          if (to.width <= 8)
            supports.result_values = with_value(supports.result_values, result);
          supports.statuses |= 1U << flags;
        }

        void scan_sources(Supports& supports, WordDomain sources) const {
          for (AdmittedEncodings source(x0, sources); source(); ++source) {
            const WordValue code = source.val();
            // A value set can exclude encodings that its cube and interval
            // admit.
            if (x0.has_value_set() && !x0.in(code))
              continue;
            const auto output = convert(code, from, to, policy);
            support_pair(supports, code, output.bits, output.flags);
          }
        }

        WordValue output(WordValue bits) const {
          return convert(bits, from, to, policy).bits;
        }

        /// How the status flags vary over the sources of one region
        enum class FlagPattern {
          Uniform, ///< Every source has the same flags
          Rounded  ///< Exactly representable sources have flags 0, the others
                   ///< the same
        };

        /// Sources of one class/sign branch with monotone results of one sign
        struct SourceRegion {
          WordEncodingRange sources;
          FlagPattern pattern;
          /// The flags of every source, or of every inexact one when Rounded
          int flags;
        };

        /// The sources of a region that a scan supports
        enum class Selection {
          All,    ///< Every source
          Exact,  ///< Only the exactly representable sources
          Inexact ///< Only the sources that round
        };

        struct Scan {
          Supports& supports;
          WordDomain sources, results;
        };

        // In a binade of FP32 sources, the exactly representable values are the
        // encodings with the returned number of trailing zero bits, as both
        // formats are binary and FP32 is the wider in range and precision. A
        // binade without such values returns from.width.
        unsigned int exact_shift(WordValue bits) const {
          assert(from.bias >= to.bias && from.fraction_bits > to.fraction_bits);
          const int field = static_cast<int>((bits & (from.sign() - 1)) >>
                                             from.fraction_bits);
          const int least = 1 - to.bias;
          const int unit = std::max(field ? field - from.bias : least, least) -
                           static_cast<int>(to.fraction_bits);
          const int shift = unit - (std::max(field, 1) - from.bias -
                                    static_cast<int>(from.fraction_bits));
          return shift <= static_cast<int>(from.fraction_bits)
                     ? static_cast<unsigned int>(shift)
                     : from.width;
        }

        // Last source up to last of the binades from first on that share the
        // exact_shift of first.
        WordValue zone_last(WordValue first, WordValue last) const {
          const unsigned int shift = exact_shift(first);
          WordValue end = first | (from.normal() - 1);
          while (end < last && exact_shift(end + 1) == shift)
            end += from.normal();
          return std::min(end, last);
        }

        static WordValue low_bits(unsigned int shift) {
          return (WordValue(1) << shift) - 1;
        }

        WordDomainResult close(const Scan& scan, WordCube cube,
                               WordEncodingRange range) const {
          return intersect(x0, {cube, scan.sources.interval}, range);
        }

        WordEncodingRange source_codes(WordRankInterval ranks) const {
          return encoding_range(x0.domain_type(), from.width, ranks);
        }

        // Least code from first to last of one sign that the result admits.
        bool first_admitted(const Scan& scan, WordValue first, WordValue last,
                            WordValue& code) const {
          const auto admitted = intersect(x1, scan.results, {first, last});
          if (!admitted.is_consistent)
            return false;
          const auto range = encoding_range(x1.domain_type(), to.width,
                                            admitted.domain.interval);
          for (code = range.first; code <= range.last; ++code)
            if (!x1.has_value_set() || x1.in(code))
              return true;
          return false;
        }

        // Greatest code from first to last of one sign that the result admits.
        bool last_admitted(const Scan& scan, WordValue first, WordValue last,
                           WordValue& code) const {
          const auto admitted = intersect(x1, scan.results, {first, last});
          if (!admitted.is_consistent)
            return false;
          const auto range = encoding_range(x1.domain_type(), to.width,
                                            admitted.domain.interval);
          for (code = range.last; x1.has_value_set() && !x1.in(code); --code)
            if (code == range.first)
              return false;
          return true;
        }

        // Last code of the run of admitted codes from the admitted code up to
        // last, which the result also admits. Below its lowest fixed bit, a
        // cube admits every code between two admitted ones.
        WordValue run_last(const Scan& scan, WordValue code,
                           WordValue last) const {
          if (x1.has_value_set()) {
            while (code < last && x1.in(code + 1))
              ++code;
            return code;
          }
          const WordValue fixed =
              ~(scan.results.cube.hi ^ scan.results.cube.lo) & (to.sign() - 1);
          if (!fixed)
            return last;
          return std::min(last, code | ((fixed & (WordValue(0) - fixed)) - 1));
        }

        // The first source of a rounded region whose result reaches the code
        // threshold, given the exact sources below and at the threshold. The
        // midpoint of these lies half their distance above the lower one, which
        // is in a binade without a boundary up to the threshold's.
        WordValue rounding_boundary(WordValue threshold, WordValue below,
                                    WordValue at) const {
          const bool negative = (threshold & to.sign()) != 0;
          switch (Float::magnitude_rounding(policy.rounding, negative)) {
          case MagnitudeRounding::Nearest:
            return below + (WordValue(1) << (exact_shift(below) - 1)) +
                   (threshold & 1);
          case MagnitudeRounding::Up:
            return below + 1;
          default:
            return at;
          }
        }

        // First source in g whose result reaches the code threshold, or
        // g.last + 1. Results of a rounded region are reached at their exact
        // sources, which bracket the binary search, and a verified rounding
        // boundary avoids it.
        WordValue first_reaching(const SourceRegion& region, WordEncodingRange g,
                                 WordValue threshold) const {
          WordValue lower = g.first, upper = g.last + 1;
          const WordValue magnitude = threshold & (to.sign() - 1);
          if (region.pattern == FlagPattern::Rounded && magnitude >= 1 &&
              magnitude <= to.maximum) {
            const WordValue below =
                convert(threshold - 1, to, from, Policy()).bits;
            const WordValue at = convert(threshold, to, from, Policy()).bits;
            const WordValue guess = rounding_boundary(threshold, below, at);
            if (guess > lower && guess < upper && output(guess) >= threshold &&
                output(guess - 1) < threshold)
              return guess;
            if (at >= lower && at < upper && output(at) >= threshold)
              upper = at;
            if (below >= lower && below < upper && output(below) < threshold)
              lower = below + 1;
          }
          return first_true({lower, upper - 1}, [&](WordValue bits) {
            return output(bits) >= threshold;
          });
        }

        // Add every step-th code from first to last of one sign as supported
        // results, where step is 1 or 2 and the codes differ by a multiple of
        // it.
        void add_results(Scan& scan, WordValue first, WordValue last,
                         WordValue step) const {
          const WordValue spread =
              low_through_highest(first ^ last) & ~(step - 1);
          scan.supports.result.add(
              {first & ~spread, first | spread},
              rank_interval(x1.domain_type(), to.width, first, last));
          if (to.width <= 8)
            for (WordValue code = first; code <= last; code += step)
              scan.supports.result_values =
                  with_value(scan.supports.result_values, code);
        }

        // Whether the members of a closed domain, from range.first to
        // range.last, are dense for unit, a power of two: the cube fixes no bit
        // from unit up below their highest differing bit. Every run of unit
        // codes between them that starts at a multiple of unit then holds the
        // members with the same pattern of low bits, one for each pattern of
        // the free low bits that the cube admits.
        static bool is_dense(const WordDomain& members, WordEncodingRange range,
                             WordValue unit) {
          const WordValue spread =
              low_through_highest(range.first ^ range.last);
          return (spread & ~(members.cube.hi ^ members.cube.lo)) < unit;
        }

        // Add the hull of the members of cube in the zone with a one among the
        // free bits of low, one such bit at a time. These are its inexact
        // members, so their hull is at most the hull of the members from the
        // least to the greatest inexact one: close gives the exact bit hull and
        // least and greatest member, and an exact member at either end gives
        // way to the next. The bits stop when the hull reaches this bound, as
        // no further bit can change it.
        void inexact_zone(const Scan& scan, WordCube cube,
                          WordEncodingRange zone, WordValue low,
                          Projection& inexact_hull) const {
          auto bound = close(scan, cube, zone);
          if (!bound.is_consistent)
            return;
          WordEncodingRange codes = source_codes(bound.domain.interval);
          if (!(codes.first & low)) {
            if (codes.first == codes.last)
              return;
            bound = close(scan, cube, {codes.first + 1, codes.last});
            codes = source_codes(bound.domain.interval);
          }
          if (!(codes.last & low)) {
            if (codes.first == codes.last)
              return;
            bound = close(scan, cube, {codes.first, codes.last - 1});
          }
          Projection part;
          for (WordValue bits = cube.hi & ~cube.lo & low; bits;
               bits &= bits - 1) {
            const auto closed =
                close(scan, {cube.lo | (bits & (WordValue(0) - bits)), cube.hi},
                      zone);
            if (!closed.is_consistent)
              continue;
            part.add(closed.domain.cube, closed.domain.interval);
            if (part.cube.lo == bound.domain.cube.lo &&
                part.cube.hi == bound.domain.cube.hi &&
                part.interval.minimum == bound.domain.interval.minimum &&
                part.interval.maximum == bound.domain.interval.maximum)
              break;
          }
          if (part.present)
            inexact_hull.add(part.cube, part.interval);
        }

        // Add the supports of the selected members of cube in zone, a zone of a
        // rounded region, when these members are dense for 2^k or have one
        // result (see dense).
        //
        // In a zone with exact sources, these are the multiples of 2^shift, and
        // consecutive ones have consecutive results. The sources strictly
        // between two of them, a gap, lie in one binade, where codes grow
        // linearly with values, and the zone boundaries are powers of two and
        // thus exact, so no gap spans two zones. Under directed rounding all
        // sources of a gap round to the result of its lower end, or all to the
        // result of its upper end. Under RNE those below the midpoint, at 2^
        // (shift-1) into the gap, round down, those above it up, and the
        // midpoint to the even result. A source is exact exactly when its low
        // shift bits are zero, so the cube alone tells whether all members are
        // exact, all inexact (a one among its low bits), or both.
        //
        // Dense for 2^k <= 2^shift (shift exceeds k in zones of subnormal
        // results), the members leave every bit from k up free between their
        // least and greatest. Every gap that lies strictly between two members
        // thus holds a member for each pattern of low bits that the cube
        // admits, and so does the part of a gap between a member and the next
        // exact source. Hence every result strictly between those of the least
        // and greatest selected member has a selected member in its preimage:
        // in the gap below or above its exact source, whichever rounds to it.
        // The one exception is RNE when every selected member is a midpoint:
        // then exactly the even results between are supported. A zone without
        // exact sources lies in one gap and has at most two results, so its
        // results need no such argument. The caller passes only members whose
        // results the result view admits (see process), so holes in the result
        // cube never fall between the least and greatest result.
        void rounded_zone(Scan& scan, const SourceRegion& region, Selection selection,
                          WordCube cube, WordEncodingRange zone) const {
          const auto closed = close(scan, cube, zone);
          if (!closed.is_consistent)
            return;
          const WordCube bits = closed.domain.cube;
          const unsigned int shift = exact_shift(zone.first);
          const WordValue low = shift < from.width ? low_bits(shift) : 0,
                          half = (low + 1) >> 1;
          // As bits is the exact hull of the members, a low bit that it leaves
          // free or fixes to one marks an inexact member.
          const bool has_inexact = !low || (bits.hi & low) != 0;
          Projection selected;
          if (selection == Selection::All || !low || (bits.lo & low))
            selected.add(bits, closed.domain.interval);
          else if (has_inexact)
            inexact_zone(scan, bits, zone, low, selected);
          if (!selected.present)
            return;
          const bool midpoints =
              policy.rounding == Policy::RNE && low &&
              (bits.hi & low) == half &&
              (selection == Selection::Inexact || (bits.lo & low) == half);
          const WordEncodingRange codes = source_codes(selected.interval);
          scan.supports.source.add(selected.cube, selected.interval);
          add_results(scan, output(codes.first), output(codes.last),
                      midpoints ? 2 : 1);
          if (!has_status())
            return;
          if (has_inexact)
            scan.supports.statuses |= 1U << region.flags;
          if (selection == Selection::All && low && !(bits.lo & low) &&
              close(scan, {bits.lo, bits.hi & ~low}, zone).is_consistent)
            scan.supports.statuses |= 1U;
        }

        // Add the supports of a set of members whose results the result view
        // admits, which is dense or has one result (see dense). In a rounded
        // region that selects all or the inexact members, each zone decides
        // which are selected (see rounded_zone). Otherwise every result
        // strictly between those of the least and greatest member has its whole
        // preimage between them, and that preimage holds a member.
        void leaf(Scan& scan, const SourceRegion& region, Selection selection,
                  const WordDomain& members) const {
          const WordEncodingRange range = source_codes(members.interval);
          if (region.pattern == FlagPattern::Rounded &&
              selection != Selection::Exact) {
            for (WordValue first = range.first; first <= range.last;) {
              const WordValue last = zone_last(first, range.last);
              rounded_zone(scan, region, selection, members.cube,
                           {first, last});
              first = last + 1;
            }
            return;
          }
          scan.supports.source.add(members.cube, members.interval);
          add_results(scan, output(range.first), output(range.last), 1);
          if (has_status())
            scan.supports.statuses |=
                1U << (selection == Selection::Exact ? 0 : region.flags);
        }

        /// Whether the members of a range have one result or possibly several
        enum class Results { One, Several };

        // Split the members of cube in g at their highest differing bit until
        // they are dense for unit (see is_dense). Of a uniform region, every
        // result has a preimage of at least 2^(k-1) consecutive sources, and
        // every NaN payload one of 2^k aligned to 2^k, where k is the
        // difference of fraction widths, so dense members for unit 2^(k-1)
        // leave a member in each. Exact sources lie 2^shift apart within a
        // binade zone, so dense members for that unit are all of them. Members
        // of a rounded region that select inexact sources are dense for 2^k
        // (see rounded_zone). Members of g with one result (Results::One) need
        // no split: results grow with the sources, so their closure supports
        // exactly that result. This keeps zero, overflow, flushed and canonical
        // NaN regions whole whatever bits the source cube fixes.
        void dense(Scan& scan, const SourceRegion& region, Selection selection,
                   WordCube cube, WordEncodingRange g, WordValue unit,
                   Results results) const {
          const auto closed = close(scan, cube, g);
          if (!closed.is_consistent)
            return;
          const WordEncodingRange range = source_codes(closed.domain.interval);
          if (results == Results::One || is_dense(closed.domain, range, unit)) {
            leaf(scan, region, selection, closed.domain);
            return;
          }
          const WordValue split =
              range.last &
              ~(low_through_highest(range.first ^ range.last) >> 1);
          dense(scan, region, selection, cube, {range.first, split - 1}, unit,
                Results::Several);
          dense(scan, region, selection, cube, {split, range.last}, unit,
                Results::Several);
        }

        // Project the members of cube in g of region, one run of admitted
        // results at a time. Results grow with the sources, so each run has a
        // contiguous preimage, and the sources between runs map to results that
        // the result view excludes.
        void process(Scan& scan, const SourceRegion& region, Selection selection,
                     WordCube cube, WordEncodingRange g, WordValue unit) const {
          while (true) {
            const auto closed = close(scan, cube, g);
            if (!closed.is_consistent)
              return;
            g = source_codes(closed.domain.interval);
            const WordValue low = output(g.first), high = output(g.last);
            WordValue code;
            if (!first_admitted(scan, low, high, code))
              return;
            if (code != low) {
              g.first = first_reaching(region, g, code);
              continue;
            }
            if (last_admitted(scan, low, high, code) && code != high) {
              g.last = first_reaching(region, g, code + 1) - 1;
              continue;
            }
            const WordValue run = run_last(scan, low, high);
            if (run == high) {
              dense(scan, region, selection, cube, g, unit,
                    low == high ? Results::One : Results::Several);
              return;
            }
            const WordValue cut = first_reaching(region, g, run + 1);
            dense(scan, region, selection, cube, {g.first, cut - 1}, unit,
                  low == run ? Results::One : Results::Several);
            g.first = cut;
          }
        }

        void scan_region(Scan& scan, const SourceRegion& region) const {
          const unsigned int k = from.fraction_bits - to.fraction_bits;
          const WordCube cube = scan.sources.cube;
          if (region.pattern == FlagPattern::Uniform) {
            if (admits_status(region.flags))
              process(scan, region, Selection::All, cube, region.sources,
                      WordValue(1) << (k - 1));
            return;
          }
          const bool exact_admitted = admits_status(0),
                     inexact_admitted = admits_status(region.flags);
          if (exact_admitted && inexact_admitted) {
            process(scan, region, Selection::All, cube, region.sources,
                    WordValue(1) << k);
          } else if (inexact_admitted) {
            process(scan, region, Selection::Inexact, cube, region.sources,
                    WordValue(1) << k);
          } else if (exact_admitted) {
            for (WordValue first = region.sources.first;
                 first <= region.sources.last;) {
              const WordValue last = zone_last(first, region.sources.last);
              const unsigned int shift = exact_shift(first);
              if (shift < from.width && !(cube.lo & low_bits(shift)))
                process(scan, region, Selection::Exact,
                        {cube.lo, cube.hi & ~low_bits(shift)}, {first, last},
                        WordValue(1) << shift);
              first = last + 1;
            }
          }
        }

        // Split a class/sign branch where its results become nonzero, its
        // sources stop being tiny and start to overflow. Each of these is
        // monotone in the source, and so is the result magnitude.
        void scan_branch(Scan& scan, WordEncodingRange range,
                         bool rounds) const {
          if (!rounds) {
            scan_region(scan, {range, FlagPattern::Uniform,
                               convert(range.first, from, to, policy).flags});
            return;
          }
          WordValue cuts[] = {
              range.first, range.last + 1,
              first_true(range,
                         [&](WordValue bits) {
                           return (output(bits) & (to.sign() - 1)) != 0;
                         }),
              first_true(range,
                         [&](WordValue bits) {
                           return !convert(bits, from, to, policy).tiny;
                         }),
              first_true(range, [&](WordValue bits) {
                return (convert(bits, from, to, policy).flags & 4) != 0;
              })};
          const unsigned int n = sizeof(cuts) / sizeof(cuts[0]);
          for (unsigned int i = 1; i < n; ++i)
            for (unsigned int j = i; j > 0 && cuts[j] < cuts[j - 1]; --j)
              std::swap(cuts[j], cuts[j - 1]);
          for (unsigned int i = 1; i < n; ++i) {
            if (cuts[i] == cuts[i - 1])
              continue;
            const Conversion first = convert(cuts[i - 1], from, to, policy);
            const WordEncodingRange sources{cuts[i - 1], cuts[i] - 1};
            // Results of zero and overflow are constant within a branch. A tiny
            // source that does not round to zero rounds to the least normal
            // when the policy flushes subnormal results.
            if ((first.flags & 4) || !(first.bits & (to.sign() - 1)) ||
                (first.tiny && policy.output_subnormals == Policy::SignedZero))
              scan_region(scan, {sources, FlagPattern::Uniform, first.flags});
            else
              scan_region(scan, {sources, FlagPattern::Rounded,
                                 first.tiny ? underflow | inexact : inexact});
          }
        }

        void scan_preimages(Supports& supports, WordDomain sources) const {
          Scan scan{supports, sources, snapshot(x1)};
          // Zero, input subnormals, finite normals, infinity, sNaN and qNaN.
          const WordValue boundaries[] = {0,
                                          1,
                                          from.normal(),
                                          from.infinity,
                                          from.infinity + 1,
                                          from.infinity | from.quiet(),
                                          from.sign()};
          for (WordValue sign : {WordValue(0), from.sign()})
            for (unsigned int part = 0; part < 6; ++part) {
              const auto available = intersect(
                  x0, sources,
                  {sign | boundaries[part], sign | (boundaries[part + 1] - 1)});
              if (!available.is_consistent)
                continue;
              const bool rounds =
                  part == 2 ||
                  (part == 1 && policy.input_subnormals == Policy::Preserve);
              scan_branch(scan, source_codes(available.domain.interval),
                          rounds);
            }
        }

        // A view of width at most eight receives its exact supported values,
        // a wider view their hull.
        static bool publish(Space& home, WordView view, const Projection& hull,
                            const WordValueSet& values) {
          if (view.width() > 8)
            return hull.publish(home, view);
          return hull.present && !me_failed(view.narrow_values(home, values));
        }

        ExecStatus publish_status(Space& home, unsigned int statuses) {
          if (!has_status())
            return ES_OK;
          for (int flag = 0; flag <= 31; ++flag)
            if ((statuses & (1U << flag)) == 0)
              GECODE_ME_CHECK(status.nq(home, flag));
          return ES_OK;
        }

        MixedConversion(Home home, WordVar source, WordVar result,
                        const Policy& p, Int::IntView flags)
            : Base(home, source, result),
              from(Float::format_facts(source.domain_type())),
              to(Float::format_facts(result.domain_type())), policy(p),
              status(flags), first_varying(least_varying_source(from, to, p)),
              last_varying(greatest_varying_source(from, to)) {
          if (has_status())
            status.subscribe(home, *this, Int::PC_INT_DOM);
        }
        MixedConversion(Space& home, MixedConversion& other)
            : Base(home, other), from(other.from), to(other.to),
              policy(other.policy), first_varying(other.first_varying),
              last_varying(other.last_varying) {
          if (other.has_status())
            status.update(home, other.status);
        }

      public:
        Actor* copy(Space& home) override {
          return new (home) MixedConversion(home, *this);
        }
        void reschedule(Space& home) override {
          Base::reschedule(home);
          if (has_status())
            status.reschedule(home, *this, Int::PC_INT_DOM);
        }
        size_t dispose(Space& home) override {
          if (has_status())
            status.cancel(home, *this, Int::PC_INT_DOM);
          Base::dispose(home);
          return sizeof(*this);
        }
        // Log2 of the number of pieces into which the cube cuts the magnitudes
        // first..last: the lowest bit from floor up that the cube fixes below
        // its highest free bit cuts them into pieces of twice its size. Zero
        // when no bit cuts.
        static unsigned int pieces_log(WordCube cube, WordValue first,
                                       WordValue last, unsigned int floor) {
          const WordValue free =
              (cube.hi ^ cube.lo) & low_through_highest(first ^ last);
          const WordValue cuts = ~free & low_through_highest(free) &
                                 ~((WordValue(1) << floor) - 1);
          if (!cuts || first >= last)
            return 0;
          const unsigned int top =
              64U - Support::count_leading_zeros_64(last - first);
          const unsigned int piece = Support::count_trailing_zeros_64(cuts) + 1;
          return top > piece ? top - piece : 0;
        }
        // Half the least result as a source magnitude: one binade lower, or
        // half the code below the least normal source. Below it, every result
        // is zero, or the least normal when subnormal results flush. Only wide
        // sources need it (see cost).
        static WordValue least_varying_source(const Format& from,
                                              const Format& to,
                                              const Policy& policy) {
          if (from.width <= 16)
            return 0;
          const WordValue least =
              convert(policy.output_subnormals == Policy::SignedZero
                          ? to.normal()
                          : 1,
                      to, from, Policy())
                  .bits;
          return least >= (from.normal() << 1) ? least - from.normal()
                                               : least >> 1;
        }
        // The greatest finite result as a source magnitude, plus 2^k: above
        // it, every number overflows. Only wide sources need it (see cost).
        static WordValue greatest_varying_source(const Format& from,
                                                 const Format& to) {
          if (from.width <= 16)
            return 0;
          return convert(to.maximum, to, from, Policy()).bits +
                 (WordValue(1) << (from.fraction_bits - to.fraction_bits));
        }
        // Log2 of the number of leaves of the density split (see dense) over
        // the source magnitudes first..last, counted as leaves that select all
        // or the inexact sources of a zone. Their unit is 2^k (see
        // scan_region), so a source bit fixed from k up cuts. When the status
        // admits only exact sources of rounded regions, the unit of a zone is
        // 2^shift (see exact_shift): 2^k for normal results and for subnormal
        // sources, and one bit more for each binade of normal sources below the
        // least normal result. There the cube cuts the binade of shift s into
        // at most 2^(fraction-1-s) pieces, so the zones from the top one, of
        // shift t, down hold at most 2^(fraction-t) leaves. A leaf of exact
        // sources only adds its closure (see leaf); 2^9 of them in [1, 2) took
        // 36 us against 52 us for leaves of all sources, so these count half.
        unsigned int leaves_log(WordValue first, WordValue last,
                                unsigned int k) const {
          const WordCube cube{x0.lo(), x0.hi()};
          if (!has_status() || !status.in(0) || status.in(16) || status.in(24))
            return pieces_log(cube, first, last, k);
          const WordValue normal =
              static_cast<WordValue>(from.bias - to.bias + 1)
              << from.fraction_bits;
          unsigned int n =
              last >= normal
                  ? pieces_log(cube, std::max(first, normal), last, k)
                  : 0;
          if (first < normal) {
            const WordValue top = std::min(last, normal - 1);
            const unsigned int shift = exact_shift(top);
            if (shift < from.fraction_bits)
              n = std::max(n, std::min(pieces_log(cube, first, top, shift),
                                       from.fraction_bits - shift));
          }
          return n > 0 ? n - 1 : 0;
        }
        // A source of at most 16 bits is scanned value by value: about 1 ms at
        // 16 bits. An FP32 source costs a few binary searches per branch, per
        // run of admitted results, and per leaf of the density split (see
        // dense), about 10 us with free cubes. Only sources whose results vary
        // count: below half the least result (the least normal when subnormal
        // results flush) and above the greatest finite result, the results of
        // numbers are constant and no run or leaf splits them. NaN sources are
        // not counted: where the policy keeps their payloads they took up to
        // 0.4 ms with a fixed result bit. Over the counted sources, a source
        // bit fixed from k up makes leaves (lower bits never split, see
        // rounded_zone and leaves_log). Only their results that the result
        // view's bounds admit make runs, as a binary search cuts off the others
        // (see process); among those, a result bit fixed at j makes runs of 2^j
        // codes. With 2^n runs or leaves, one FP32 propagation took about 50 us
        // at n = 7, 0.2 ms at n = 10, 0.6 ms at n = 11, 3-4 ms at n = 14 and
        // 10-12 ms at n = 15.
        PropCost cost(const Space&, const ModEventDelta&) const override {
          if (x0.assigned())
            return PropCost::binary(PropCost::LO);
          if (from.width <= 16)
            return PropCost::linear(from.width <= 8 ? PropCost::LO
                                                    : PropCost::HI,
                                    1U << from.width);
          const unsigned int k = from.fraction_bits - to.fraction_bits;
          const WordValue magnitude = from.sign() - 1;
          // Magnitudes of the sources, clipped to those whose results vary. A
          // window across zero holds the magnitudes from zero up on one side.
          const WordValue minimum = x0.minimum() & magnitude,
                          maximum = x0.maximum() & magnitude;
          const bool across =
              ((x0.minimum() ^ x0.maximum()) & from.sign()) != 0;
          const WordValue first =
              std::max({across ? 0 : std::min(minimum, maximum),
                        x0.lo() & magnitude, first_varying});
          const WordValue last = std::min(across ? x0.hi() & magnitude
                                                 : std::max(minimum, maximum),
                                          last_varying);
          if (first >= last)
            return PropCost::linear(PropCost::LO, from.width);
          // Magnitudes of their results, clipped to those of the result view
          // in the same way.
          const WordValue results = to.sign() - 1;
          const WordValue result_minimum = x1.minimum() & results,
                          result_maximum = x1.maximum() & results;
          const bool results_across =
              ((x1.minimum() ^ x1.maximum()) & to.sign()) != 0;
          const WordValue low = std::max(
              {output(first) & results,
               results_across ? 0 : std::min(result_minimum, result_maximum),
               x1.lo() & results});
          const WordValue high = std::min(
              {output(last) & results, std::max(result_minimum, result_maximum),
               x1.hi() & results});
          const unsigned int n =
              std::max(leaves_log(first, last, k),
                       pieces_log({x1.lo(), x1.hi()}, low, high, 0));
          if (n <= 8)
            return PropCost::linear(PropCost::LO, from.width);
          if (n <= 12)
            return PropCost::linear(PropCost::HI, from.width);
          return PropCost::quadratic(PropCost::HI, from.width);
        }
        /// Post the conversion; a null \a status means the posting has none.
        static void post(Home home, const Policy& policy, WordVar source,
                         WordVar result, IntVar status) {
          (void)new (home)
              MixedConversion(home, source, result, policy, status);
        }
        ExecStatus propagate(Space& home, const ModEventDelta&) override {
          Supports supports;
          const WordDomain sources = snapshot(x0);
          if (from.width <= 16 || x0.assigned())
            scan_sources(supports, sources);
          else
            scan_preimages(supports, sources);
          if (x0 == x1) {
            supports.source.intersect(supports.result);
            supports.result = supports.source;
            supports.source_values = intersect_value_sets(
                supports.source_values, supports.result_values);
            supports.result_values = supports.source_values;
          }
          if (!publish(home, x0, supports.source, supports.source_values) ||
              !publish(home, x1, supports.result, supports.result_values))
            return ES_FAILED;
          GECODE_ES_CHECK(publish_status(home, supports.statuses));
          if (x0.assigned() && x1.assigned() &&
              (!has_status() || status.assigned()))
            return home.ES_SUBSUMED(*this);
          // Supports retain every admitted tuple, including equality of aliased
          // views. The FP32 scan finds exactly the supported sources, results
          // and statuses because each leaf is dense or has one result (see
          // dense): every result between those of its least and greatest member
          // then has a selected member in its preimage. A narrower view keeps
          // exactly its supported values, or the smallest cube and bounds
          // containing them, and a wider view the hull of its supports. A
          // second pass finds these supports again within the published
          // domains, so publication is idempotent.
          return ES_FIX;
        }
      };
    } // namespace
  } // namespace Word
} // namespace Gecode

// STATISTICS: word-prop
