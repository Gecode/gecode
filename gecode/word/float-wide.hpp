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

#ifndef GECODE_WORD_FLOAT_WIDE_HPP
#define GECODE_WORD_FLOAT_WIDE_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <gecode/float-word.hh>
#include <gecode/word/arithmetic/global-integer.hpp>
#include <gecode/word/float.hpp>
#include <utility>

namespace Gecode {
  namespace Word {
    namespace Float {
      using Integer = Arithmetic::Global::Integer;

      /// The facts of BF16 or FP32 that wide propagation reads
      struct WideFormat {
        unsigned int fraction_bits;
        WordValue sign, normal, infinity, maximum, quiet;
        explicit WideFormat(WordDomainType f)
            : WideFormat(format_facts(f)) {
          assert(f == WDT_BF16 || f == WDT_FP32);
        }
        explicit WideFormat(const FormatFacts& f)
            : fraction_bits(f.fraction_bits), sign(f.sign()),
              normal(f.normal()), infinity(f.infinity), maximum(f.maximum),
              quiet(f.quiet()) {}

        // Half-minimum-subnormal units: 2^-134 (BF16), 2^-150 (FP32).
        // The extra infinity code denotes the next finite binade here only.
        Integer magnitude(WordValue code) const {
          const unsigned int exponent =
              static_cast<unsigned int>(code >> fraction_bits);
          return exponent == 0 ? Integer(code * 2)
                               : Integer(normal + (code & (normal - 1)))
                                     .shift_left(exponent);
        }
        Integer value(WordValue code) const {
          Integer v = magnitude(code & (sign - 1));
          return (code & sign) ? -v : v;
        }
        bool nan(WordValue code) const {
          return (code & (sign - 1)) > infinity;
        }
        bool signaling(WordValue code) const {
          return nan(code) && !(code & quiet);
        }
      };

      struct WideResult {
        WordValue code;
        int status;
      };

      // Encoding of a nonnegative finite floor, with a checked upper clamp.
      // Only the leading precision bits are needed; no encoding-space search.
      inline WordValue floor_encoding(const WideFormat& f,
                                      const Integer& value) {
        if (value >= f.magnitude(f.maximum))
          return f.maximum;
        if (value < f.magnitude(f.normal))
          return value.shift_right(1).word();
        const unsigned int shift = value.bits() - f.fraction_bits - 1;
        WordValue significand = 0;
        for (unsigned int bit = 0; bit <= f.fraction_bits; ++bit)
          if (value.bit(shift + bit))
            significand |= WordValue(1) << bit;
        return (WordValue(shift) << f.fraction_bits) + significand - f.normal;
      }

      struct ExactDyadic {
        Integer coefficient;
        int exponent;
        bool negative_zero;
      };

      // Requires finite, input-treated encodings. This lossless dyadic has no
      // rounding or flags: FMA aligns its addend before rounding.
      // With S=127+fraction_bits, operands need at most 262/278 coefficient
      // bits and their products at most 524/556; Integer grows without
      // truncation.
      inline ExactDyadic exact_product(const WideFormat& f, WordValue a,
                                       WordValue b) {
        assert((a & (f.sign - 1)) <= f.maximum &&
               (b & (f.sign - 1)) <= f.maximum);
        return {f.value(a) * f.value(b),
                -2 * (127 + static_cast<int>(f.fraction_bits)),
                ((a ^ b) & f.sign) != 0};
      }

      /// Code of an overflowing result of sign \a negative
      inline WordValue overflow_code(const WideFormat& f, const Policy& p,
                                     bool negative) {
        return (negative ? f.sign : 0) |
               (overflows_to_infinity(p, negative) ? f.infinity : f.maximum);
      }

      // Round the nonnegative magnitude to a multiple of 2^shift. The
      // remainder is the low shift bits: RNE rounds up when it exceeds half
      // the step (bit shift-1 and a lower bit set) or ties with an odd
      // quotient (bits shift-1 and shift set).
      inline Integer round_coefficient(const Integer& magnitude,
                                       unsigned int shift, bool negative,
                                       Policy::Rounding rounding) {
        bool increment = false;
        if (!magnitude.low_bits_zero(shift)) {
          switch (magnitude_rounding(rounding, negative)) {
          case MagnitudeRounding::Nearest:
            increment = magnitude.bit(shift - 1) &&
                        (!magnitude.low_bits_zero(shift - 1) ||
                         magnitude.bit(shift));
            break;
          case MagnitudeRounding::Down:
            break;
          case MagnitudeRounding::Up:
            increment = true;
            break;
          }
        }
        const Integer quotient = magnitude.shift_right(shift);
        return (increment ? quotient + Integer(1) : quotient).shift_left(shift);
      }

      // Round the original dyadic directly to the destination lattice. The
      // separate precision rounding determines range flags only. The exponent
      // must provide at least the destination's half-subnormal resolution;
      // same-format products use -268/-300, including minimum products 2^-266
      // and 2^-298. Signed exponents are checked before becoming shift counts.
      inline WideResult round_dyadic(const WideFormat& f, const Policy& p,
                                     const ExactDyadic& exact) {
        bool negative = exact.coefficient.zero() ? exact.negative_zero
                                                 : exact.coefficient.neg();
        const Integer magnitude = exact.coefficient.abs();
        if (magnitude.zero())
          return {negative && p.zeros == Policy::Signed ? f.sign : 0, 0};
        const int alignment =
            -exact.exponent - 127 - static_cast<int>(f.fraction_bits);
        assert(alignment >= 0);
        const unsigned int scale_bits = static_cast<unsigned int>(alignment);
        const unsigned int precision_shift =
            magnitude.bits() > f.fraction_bits + 1
                ? magnitude.bits() - f.fraction_bits - 1
                : 0;
        const Integer precision =
            round_coefficient(magnitude, precision_shift, negative, p.rounding);
        if (precision > f.magnitude(f.maximum).shift_left(scale_bits)) {
          return {overflow_code(f, p, negative), overflow | inexact};
        }
        const Integer rounded = round_coefficient(
            magnitude,
            std::max(precision_shift, scale_bits + 1),
            negative, p.rounding);
        WordValue code = floor_encoding(f, rounded.shift_right(scale_bits));
        int flags = rounded == magnitude ? 0 : inexact;
        const Integer normal = f.magnitude(f.normal).shift_left(scale_bits);
        const bool precision_tiny = precision < normal;
        const bool tiny = p.tininess == Policy::BeforeRounding
                              ? magnitude < normal
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

      inline Word::WordDomain wide_snapshot(WordView view) {
        return {{view.lo(), view.hi()},
                {view.bounded() ? view.rank_minimum() : view.lo(),
                 view.bounded() ? view.rank_maximum() : view.hi()}};
      }

      // Intersect d with the encodings first..last of one sign half.
      inline Word::WordDomainResult wide_intersect(WordView view,
                                                   Word::WordDomain d,
                                                   WordValue first,
                                                   WordValue last) {
        const Word::WordRankInterval ranks =
            Word::rank_interval(view.domain_type(), view.width(), first, last);
        d.interval.minimum = std::max(d.interval.minimum, ranks.minimum);
        d.interval.maximum = std::min(d.interval.maximum, ranks.maximum);
        if (d.interval.minimum > d.interval.maximum)
          return {false, {{0, 0}, {0, 0}}};
        return Word::compute_domain_closure(view.width(), view.domain_type(),
                                            d);
      }

      // Least and greatest encoding of a domain within one sign half.
      inline Word::WordEncodingRange domain_codes(WordView view,
                                                  const Word::WordDomain& d) {
        return Word::encoding_range(view.domain_type(), view.width(),
                                    d.interval);
      }

      struct WideProjection {
        bool present = false;
        Word::WordCube cube{~WordValue(0), 0};
        Word::WordRankInterval interval{~WordValue(0), 0};

        void include(WordView view, const Word::WordDomain& d) {
          const WordValue first =
              Word::encode_rank(view.domain_type(), view.width(), d.interval.minimum);
          Word::WordCube hull{first, first};
          for (WordValue bits = d.cube.hi ^ d.cube.lo; bits; bits &= bits - 1) {
            const WordValue bit = bits & (WordValue(0) - bits);
            Word::WordDomain alternative = d;
            if (first & bit)
              alternative.cube.hi &= ~bit;
            else
              alternative.cube.lo |= bit;
            if (Word::compute_domain_closure(view.width(), view.domain_type(),
                                             alternative)
                    .is_consistent) {
              hull.lo &= ~bit;
              hull.hi |= bit;
            }
          }
          present = true;
          cube.lo &= hull.lo;
          cube.hi |= hull.hi;
          interval.minimum = std::min(interval.minimum, d.interval.minimum);
          interval.maximum = std::max(interval.maximum, d.interval.maximum);
        }
        void include(WordView view, const Word::WordDomain& d, WordValue first,
                     WordValue last) {
          const auto admitted = wide_intersect(view, d, first, last);
          if (admitted.is_consistent)
            include(view, admitted.domain);
        }
        // Include every encoding that other includes.
        void unite(const WideProjection& other) {
          if (!other.present)
            return;
          present = true;
          cube.lo &= other.cube.lo;
          cube.hi |= other.cube.hi;
          interval.minimum = std::min(interval.minimum, other.interval.minimum);
          interval.maximum = std::max(interval.maximum, other.interval.maximum);
        }
        // Whether this projection equals all, the projection of a whole
        // domain. Each include adds the exact hull of encodings that the
        // domain admits, so a projection that equals all no longer changes.
        bool covers(const WideProjection& all) const {
          return present && cube.lo == all.cube.lo && cube.hi == all.cube.hi &&
                 interval.minimum == all.interval.minimum &&
                 interval.maximum == all.interval.maximum;
        }
        // Hulls of two projections of one view bound their intersection.
        void intersect(const WideProjection& other) {
          present = present && other.present;
          cube.lo |= other.cube.lo;
          cube.hi &= other.cube.hi;
          interval.minimum = std::max(interval.minimum, other.interval.minimum);
          interval.maximum = std::min(interval.maximum, other.interval.maximum);
        }
        // Narrow view to the hull of the projection.
        bool publish(Space& home, WordView view) const {
          return present && !me_failed(Word::narrow_word_domain(
                                home, view, cube, interval));
        }
      };

      // Publish projections[i] to views[i]. Positions sharing a view publish
      // the intersection of their projections in one tell.
      inline bool publish_projections(Space& home, const WordView views[],
                                      const WideProjection projections[],
                                      int n) {
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
          if (!shared.publish(home, views[i]))
            return false;
        }
        return true;
      }

      struct FinitePart {
        Word::WordDomain domain;
        Integer low, high;
        bool negative, flushed;
      };
      /// At most \a capacity parts of a domain, stored in place
      template<class Part, int capacity>
      class Parts {
        Part parts[capacity];
        int n = 0;
      public:
        void push_back(const Part& part) {
          assert(n < capacity);
          parts[n++] = part;
        }
        size_t size() const { return static_cast<size_t>(n); }
        const Part& operator[](size_t i) const { return parts[i]; }
        const Part* begin() const { return parts; }
        const Part* end() const { return parts + n; }
      };
      /// Each sign has a zero, subnormal and normal part
      using FiniteParts = Parts<FinitePart, 6>;
      /// How a word is read: input subnormals flush only in an operand
      enum class Role { Input, Result };
      inline FiniteParts
      finite_parts(const WideFormat& f, const Policy& p, WordView view,
                   const Word::WordDomain& domain, Role role) {
        FiniteParts parts;
        for (WordValue sign : {WordValue(0), f.sign})
          for (const auto& range :
               {std::pair<WordValue, WordValue>(0, 0),
                std::pair<WordValue, WordValue>(1, f.normal - 1),
                std::pair<WordValue, WordValue>(f.normal, f.maximum)}) {
            const auto admitted = wide_intersect(
                view, domain, sign | range.first, sign | range.second);
            if (!admitted.is_consistent)
              continue;
            const Word::WordEncodingRange codes =
                domain_codes(view, admitted.domain);
            const bool flushed = role == Role::Input && range.first == 1 &&
                                 p.input_subnormals == Policy::SignedZero;
            Integer low =
                flushed ? Integer() : f.value(sign ? codes.last : codes.first);
            Integer high =
                flushed ? Integer() : f.value(sign ? codes.first : codes.last);
            parts.push_back({admitted.domain, low, high, sign != 0, flushed});
          }
        return parts;
      }

      inline bool admits_nonfinite(const WideFormat& f, WordView view,
                                   const Word::WordDomain& domain) {
        return wide_intersect(view, domain, f.infinity, f.sign - 1)
                   .is_consistent ||
               wide_intersect(view, domain, f.sign | f.infinity,
                              (f.sign << 1) - 1)
                   .is_consistent;
      }

      // Monotone numerical endpoints become at most two raw-encoding ranges.
      // Include both zeros: exact evaluation retains their distinct sign rules.
      inline void include_rounded(const WideFormat& f,
                                  WideProjection& projection, WordView view,
                                  const Word::WordDomain& domain, WordValue low,
                                  WordValue high) {
        const WordValue lm = low & (f.sign - 1), hm = high & (f.sign - 1);
        const bool ln = (low & f.sign) != 0, hn = (high & f.sign) != 0;
        if (ln && hn) {
          projection.include(view, domain, high, low);
        } else if (!ln && !hn) {
          projection.include(view, domain, low, high);
        } else {
          projection.include(view, domain, f.sign, low);
          projection.include(view, domain, 0, high);
        }
        if (lm == 0 || hm == 0 || (ln && !hn)) {
          projection.include(view, domain, 0, 0);
          projection.include(view, domain, f.sign, f.sign);
        }
      }

      // Set admitted to the encodings of part whose values lie in [low,
      // high]. Return false when there are none.
      inline bool real_domain(const WideFormat& f, WordView view,
                              const FinitePart& part, Integer low,
                              Integer high, Word::WordDomain& admitted) {
        low = std::max(low, part.low);
        high = std::min(high, part.high);
        if (low > high)
          return false;
        if (part.flushed) {
          admitted = part.domain;
          return true;
        }
        const Integer minimum = part.negative ? -high : low;
        const Integer maximum = part.negative ? -low : high;
        WordValue first = floor_encoding(f, minimum);
        if (f.magnitude(first) < minimum)
          ++first;
        const WordValue last = floor_encoding(f, maximum);
        if (first > last)
          return false;
        const auto result =
            wide_intersect(view, part.domain, first | (part.negative ? f.sign : 0),
                           last | (part.negative ? f.sign : 0));
        if (!result.is_consistent)
          return false;
        admitted = result.domain;
        return true;
      }

      inline void include_real(const WideFormat& f, WideProjection& projection,
                               WordView view, const FinitePart& part,
                               Integer low, Integer high) {
        Word::WordDomain admitted;
        if (real_domain(f, view, part, low, high, admitted))
          projection.include(view, admitted);
      }

      // Closed bounds on the half-subnormal integer lattice. Strict midpoint
      // and directed endpoints are represented by the adjacent integer unit.
      struct RoundingCell {
        Integer low, high;
      };

      /// Whether a rounding cell excludes the ends that do not round to it
      enum class CellEnds {
        /// Exclude such an end by one unit of the lattice: the cell holds
        /// the integers of the lattice that round to the code
        Open,
        /// Keep every end: the cell encloses the reals that round to the code
        Closed
      };

      /** \brief The values that round to \a code under \a p, scaled by 2^scale
       *
       * The values are in half-subnormal units. Product coefficients use the
       * square of the source half-subnormal scale: midpoints are formed in
       * that finer lattice before open ends are excluded. A quotient need not
       * lie on the lattice, so its enclosure keeps every end (CellEnds::Closed);
       * exact ties and signed zeros are then decided by the evaluation.
       */
      inline RoundingCell rounding_cell(const WideFormat& f, const Policy& p,
                                        WordValue code, unsigned int scale,
                                        CellEnds ends) {
        const WordValue magnitude = code & (f.sign - 1);
        if (magnitude == 0) {
          // Both zero signs can be joined safely. Flushing preimages extend to
          // the normal boundary.
          const Integer radius =
              f.magnitude(p.output_subnormals == Policy::SignedZero ? f.normal
                                                                    : 1)
                  .shift_left(scale);
          return {-radius, radius};
        }
        assert(magnitude < f.maximum);
        const bool negative = (code & f.sign) != 0;
        const Integer center = f.value(code).shift_left(scale);
        const Integer previous = (negative ? -f.magnitude(magnitude + 1)
                                           : f.magnitude(magnitude - 1))
                                     .shift_left(scale);
        const Integer next = (negative ? -f.magnitude(magnitude - 1)
                                       : f.magnitude(magnitude + 1))
                                 .shift_left(scale);
        if (ends == CellEnds::Closed) {
          if (p.rounding == Policy::RNE)
            return {(previous + center).shift_right(1),
                    (center + next).shift_right(1)};
          return rounds_upward(p.rounding, negative)
                     ? RoundingCell{previous, center}
                     : RoundingCell{center, next};
        }
        if (p.rounding == Policy::RNE) {
          const Integer odd(magnitude & 1);
          return {(previous + center).shift_right(1) + odd,
                  (center + next).shift_right(1) - odd};
        }
        return rounds_upward(p.rounding, negative)
                   ? RoundingCell{previous + Integer(1), center}
                   : RoundingCell{center, next - Integer(1)};
      }

      /// Direction of a search for an extreme supported code
      enum class Direction { Up, Down };

      /// Whether a lattice search may stop at its step limit
      enum class LatticeLimit { Bounded, Exhaustive };

      /**
       * \brief The step limit of the lattice searches of one pass
       *
       * A bounded search that reaches its step limit hands back a bound
       * past the codes it showed unsupported, and counts itself in \a cuts.
       * Such a bound narrows only the operand part that the search was
       * for, and a projection is the union over all parts, so a cut search
       * may hide a narrowing. A pass that narrows nothing after a cut is
       * therefore no fixpoint: the propagators repeat it with exhaustive
       * searches, which settle every endpoint.
       */
      struct LatticeBudget {
        LatticeLimit limit;
        /// The searches that handed back a bound at their step limit
        unsigned int cuts;
      };

      // Spacing of the codes of binade e, in half-subnormal units
      inline Integer binade_unit(unsigned int e) {
        return e == 0 ? Integer(2) : Integer::power_two(e);
      }

      /// The admitted codes of one binade of a word, as runs of codes that
      /// share their known fraction bits
      struct BinadeRuns {
        /// The least and the greatest admitted magnitude
        WordValue first, last;
        /// Whether several runs repeat every \a period codes
        bool periodic;
        /// Codes per run and per period
        WordValue run, period;
        /// First magnitude of the reference run of a periodic binade
        WordValue start;
      };

      // The runs of the admitted nonzero codes with sign bit sign in binade
      // e of view in domain. Known fraction bits from bit j up to bit
      // j+t-1, with bit j-1 and below unknown, admit runs of 2^j codes
      // every 2^(j+t) codes. Where j+t reaches the fraction width the
      // binade holds one run, and first and last are narrowed to it.
      // Otherwise the binade is periodic, with a reference run
      // [start, start+run-1] that neither starts the binade nor holds the
      // maximum finite magnitude, so that the cells of its codes are
      // regular: the first code of a binade has a closer predecessor and
      // a narrower cell. Translating the cells of the reference run by
      // whole periods then gives a superset of the cells of every run.
      // Without such a run the binade counts as not periodic. Return false
      // when the binade admits no code.
      inline bool binade_runs(const WideFormat& f, WordView view,
                              const Word::WordDomain& domain, WordValue sign,
                              unsigned int e, BinadeRuns& runs) {
        const WordValue base = e * f.normal;
        const auto binade = wide_intersect(
            view, domain, sign | std::max(WordValue(1), base),
            sign | std::min(f.maximum, base + f.normal - 1));
        if (!binade.is_consistent)
          return false;
        const auto codes = domain_codes(view, binade.domain);
        runs.first = codes.first & (f.sign - 1);
        runs.last = codes.last & (f.sign - 1);
        runs.periodic = false;
        const WordValue known =
            ~(binade.domain.cube.lo ^ binade.domain.cube.hi) & (f.normal - 1);
        if (known == 0)
          return true;
        unsigned int j = 0, t = 0;
        while (!((known >> j) & 1))
          ++j;
        while (j + t < f.fraction_bits && ((known >> (j + t)) & 1))
          ++t;
        runs.run = WordValue(1) << j;
        runs.period = WordValue(1) << (j + t);
        const WordValue pattern =
            binade.domain.cube.lo & (runs.period - 1) & ~(runs.run - 1);
        if (j + t == f.fraction_bits) {
          runs.first = std::max(runs.first, base + pattern);
          runs.last = std::min(runs.last, base + pattern + runs.run - 1);
          return runs.first <= runs.last;
        }
        runs.start = base + pattern + (pattern == 0 ? runs.period : 0);
        runs.periodic = runs.start + runs.run - 1 < f.maximum;
        return true;
      }

      // Partition a result/input alias x into classes with correlated
      // rounding cells. Per sign, tail(domain) receives the admitted
      // zero/flushed range and the range from maximum finite to the sign bit:
      // these can have nonstandard or unbounded preimages. Every other code
      // belongs to one regular(part, first, last) class: a power of two
      // (asymmetric predecessor spacing), or the odd or even codes of a binade
      // (strict RNE ties). Within a class cell(x)-x is constant, so residuals
      // that are monotone in x take their extrema at the endpoint codes.
      //
      // regular returns whether to keep its class; tails are always kept.
      // kept receives every kept class and tail. Only the binades between
      // the least and the greatest admitted magnitude of a sign are visited.
      // A run of binades whose admitted classes are all kept is included as
      // one range of encodings. The range admits exactly the encodings of
      // those classes, so kept gets the same hull as from including the
      // classes one by one, at the cost of one inclusion per run.
      template <class Regular, class Tail>
      void visit_alias_classes(const WideFormat& f, const Policy& p,
                               WordView view, const Word::WordDomain& domain,
                               WideProjection& kept, Regular regular,
                               Tail tail) {
        const WordValue first_regular =
            p.input_subnormals == Policy::Preserve &&
                    p.output_subnormals == Policy::Preserve
                ? 1
                : f.normal;
        for (WordValue sign : {WordValue(0), f.sign}) {
          const auto half = wide_intersect(view, domain, sign, sign | (f.sign - 1));
          if (!half.is_consistent)
            continue;
          for (const auto& range :
               {std::pair<WordValue, WordValue>(0, first_regular - 1),
                std::pair<WordValue, WordValue>(f.maximum, f.sign - 1)}) {
            const auto admitted = wide_intersect(
                view, domain, sign | range.first, sign | range.second);
            if (admitted.is_consistent) {
              kept.include(view, admitted.domain);
              tail(admitted.domain);
            }
          }
          const Word::WordEncodingRange extent = domain_codes(view, half.domain);
          const unsigned int least = static_cast<unsigned int>(
              (extent.first & (f.sign - 1)) >> f.fraction_bits);
          const unsigned int greatest = std::min(
              254U, static_cast<unsigned int>((extent.last & (f.sign - 1)) >>
                                              f.fraction_bits));
          // The pending run of kept binades, [run_first, run_last]
          bool run = false;
          WordValue run_first = 0, run_last = 0;
          for (unsigned int exponent = least; exponent <= greatest;
               ++exponent) {
            const WordValue start = exponent * f.normal;
            const WordValue binade_first = std::max(first_regular, start);
            const WordValue binade_last =
                std::min(f.maximum - 1, (exponent + 1) * f.normal - 1);
            if (binade_first > binade_last)
              continue;
            Word::WordDomain classes[3];
            int count = 0;
            bool all = true;
            for (int partition = 0; partition < 3; ++partition) {
              const WordValue first =
                  std::max(first_regular, start + (partition ? 1 : 0));
              const WordValue last =
                  std::min(f.maximum - 1,
                           partition ? (exponent + 1) * f.normal - 1 : start);
              if (first > last)
                continue;
              auto candidate =
                  wide_intersect(view, domain, sign | first, sign | last);
              if (!candidate.is_consistent)
                continue;
              if (partition) {
                if (partition == 1)
                  candidate.domain.cube.lo |= 1;
                else
                  candidate.domain.cube.hi &= ~WordValue(1);
                if (candidate.domain.cube.lo & ~candidate.domain.cube.hi)
                  continue;
                candidate = Word::compute_domain_closure(
                    view.width(), view.domain_type(), candidate.domain);
                if (!candidate.is_consistent)
                  continue;
              }
              const Word::WordEncodingRange codes =
                  domain_codes(view, candidate.domain);
              const FinitePart part{
                  candidate.domain, f.value(sign ? codes.last : codes.first),
                  f.value(sign ? codes.first : codes.last), sign != 0, false};
              if (regular(part, codes.first, codes.last))
                classes[count++] = candidate.domain;
              else
                all = false;
            }
            if (all) {
              if (!run)
                run_first = binade_first;
              run = true;
              run_last = binade_last;
              continue;
            }
            if (run)
              kept.include(view, domain, sign | run_first, sign | run_last);
            run = false;
            for (int i = 0; i < count; ++i)
              kept.include(view, classes[i]);
          }
          if (run)
            kept.include(view, domain, sign | run_first, sign | run_last);
        }
      }

      // Set floor and ceiling to those of x / y for a nonzero y, from one
      // truncating division.
      inline void floor_ceil_div(const Integer& x, const Integer& y,
                                 Integer& floor, Integer& ceiling) {
        Integer quotient, remainder;
        Integer::divide(x, y, quotient, remainder);
        if (remainder.zero()) {
          floor = quotient;
          ceiling = std::move(quotient);
        } else if (x.neg() != y.neg()) {
          ceiling = quotient;
          floor = quotient - Integer(1);
        } else {
          floor = quotient;
          ceiling = quotient + Integer(1);
        }
      }

      // The least t >= 0 with low <= (a*t) mod m <= high, given 0 <= a < m
      // and 0 <= low <= high < m. Return false when there is none.
      //
      // If [low, high] holds a multiple of a, the least one is a*ceil(low/a)
      // <= high < m: t = ceil(low/a), and every smaller t has a*t < low
      // without wrapping. Otherwise high - low < a, and low, high have the
      // same quotient by a. Each t wraps y = floor(a*t/m) times, and (a*t)
      // mod m lies in the window iff a*t lies in [low + m*y, high + m*y].
      // These ranges are disjoint and increase with y, so the least t
      // belongs to the least y whose range holds a multiple of a, and it is
      // ceil((low + m*y)/a). That range holds a multiple of a iff (m*y) mod a
      // lies in [a - high mod a, a - low mod a], a window within [1, a - 1].
      // So y solves the same problem for (m mod a, a), and the recursion
      // follows Euclid's algorithm. When a divides m, a*t mod m is always a
      // multiple of a, and the window holds none.
      inline bool least_in_window(const Integer& a, const Integer& m,
                                  const Integer& low, const Integer& high,
                                  Integer& t) {
        if (low.zero()) {
          t = Integer();
          return true;
        }
        if (a.zero())
          return false;
        Integer q, r;
        Integer::divide(low, a, q, r);
        if (!r.zero())
          q = q + Integer(1);
        if (a * q <= high) {
          t = std::move(q);
          return true;
        }
        Integer y;
        if (!least_in_window(m % a, a, a - high % a, a - low % a, y))
          return false;
        t = Arithmetic::Global::ceil_div(low + m * y, a);
        return true;
      }

      // The least t >= 0 with (a*t + base) mod m <= width, for m > 0. The
      // window [-base, width - base] modulo m wraps around zero, where t =
      // 0 lies, or is one range [low, high] within [1, m - 1]. Dividing a,
      // m and the window by their common power of two keeps the numbers of
      // least_in_window small: a*t mod m is a multiple of it.
      inline bool least_offset(const Integer& a, const Integer& m,
                               const Integer& base, const Integer& width,
                               Integer& t) {
        using Arithmetic::Global::ceil_shift_right;
        using Arithmetic::Global::floor_shift_right;
        using Arithmetic::Global::mod;
        if (width >= m - Integer(1)) {
          t = Integer();
          return true;
        }
        const Integer low = mod(-base, m), high = low + width;
        if (low.zero() || high >= m) {
          t = Integer();
          return true;
        }
        const Integer multiplier = mod(a, m);
        if (multiplier.zero())
          return false;
        unsigned int shift = 0;
        while (!multiplier.bit(shift) && !m.bit(shift))
          ++shift;
        const Integer reduced_low = ceil_shift_right(low, shift);
        const Integer reduced_high = floor_shift_right(high, shift);
        if (reduced_low > reduced_high)
          return false;
        return least_in_window(multiplier.shift_right(shift),
                               m.shift_right(shift), reduced_low,
                               reduced_high, t);
      }

      // The least x in [from, to] with low <= (a*x) mod m <= high, read
      // modulo m, for m > 0 and low <= high. Return false when there is
      // none. With x = from + t this asks for (a*t + a*from - low) mod m <=
      // high - low.
      inline bool first_in_window(const Integer& a, const Integer& m,
                                  const Integer& low, const Integer& high,
                                  const Integer& from, const Integer& to,
                                  Integer& x) {
        if (from > to)
          return false;
        Integer t;
        if (!least_offset(a, m, a * from - low, high - low, t))
          return false;
        x = from + t;
        return x <= to;
      }

      // The greatest x in [from, to] with low <= (a*x) mod m <= high, read
      // modulo m, for m > 0 and low <= high. Return false when there is
      // none. With x = to - t this asks for (-a*t + a*to - low) mod m <=
      // high - low.
      inline bool last_in_window(const Integer& a, const Integer& m,
                                 const Integer& low, const Integer& high,
                                 const Integer& from, const Integer& to,
                                 Integer& x) {
        if (from > to)
          return false;
        Integer t;
        if (!least_offset(-a, m, a * to - low, high - low, t))
          return false;
        x = to - t;
        return x >= from;
      }

      /// The lowest run of known fraction bits of a cube: bits free to bits - 1
      /// are those of value, whose lower bits are zero. Codes and significands
      /// of one binade differ by a multiple of 2^fb, so the run holds for the
      /// significands too. Without known fraction bits, free = bits = 0.
      struct KnownRun {
        unsigned int free;
        unsigned int bits;
        WordValue value;
      };
      inline KnownRun known_run(const Word::WordCube& cube,
                                unsigned int fraction_bits) {
        const WordValue known = ~(cube.lo ^ cube.hi);
        unsigned int free = 0;
        while (free < fraction_bits && ((known >> free) & 1) == 0)
          ++free;
        unsigned int bits = free;
        while (bits < fraction_bits && ((known >> bits) & 1) != 0)
          ++bits;
        if (bits == free)
          return {0, 0, 0};
        return {free, bits, cube.lo & ((WordValue(1) << bits) - 1)};
      }

      // The extreme s in [from, to] with s*term in [low, high] modulo
      // modulus, or the extreme of [from, to] when there is no modulus or
      // the window covers every residue. Return false when there is none.
      inline bool modular_candidate(const Integer& term, const Integer& from,
                                    const Integer& to, Direction direction,
                                    const Integer& low, const Integer& high,
                                    const Integer& modulus, Integer& s) {
        if (from > to)
          return false;
        if (modulus.zero() || high - low + Integer(1) >= modulus) {
          s = direction == Direction::Up ? from : to;
          return true;
        }
        return direction == Direction::Up
                   ? first_in_window(term, modulus, low, high, from, to, s)
                   : last_in_window(term, modulus, low, high, from, to, s);
      }

      // Improve s, the extreme s in [from, to] with s*term in [low, high]
      // modulo modulus (modular_candidate), by the lowest run of known bits
      // of s. With s = t*2^m + u for the m low bits u, of which bits i to
      // m - 1 are known to be v, u lies in [v, v + 2^i - 1], and s*term in
      // the window gives t*(term*2^m) in [low - (v + 2^i - 1)*term, high -
      // v*term], again one modular window, exact for i = 0. Every s of a t
      // that fails it, and every s whose low bits differ from v, is ruled
      // out, so the more extreme of the two candidates is the bound. Return
      // false when no s meets both. The second one alone can be weaker,
      // where i is large, as for the known high bits of a range.
      inline bool known_run_candidate(const KnownRun& known,
                                      const Integer& term, const Integer& from,
                                      const Integer& to, Direction direction,
                                      const Integer& low, const Integer& high,
                                      const Integer& modulus, Integer& s) {
        using Arithmetic::Global::ceil_div;
        using Arithmetic::Global::floor_div;
        if (known.bits == 0)
          return true;
        const Integer step = Integer::power_two(known.bits);
        const Integer value(known.value);
        const Integer spread = Integer::power_two(known.free) - Integer(1);
        Integer t;
        if (!modular_candidate(term * step,
                               ceil_div(from - value - spread, step),
                               floor_div(to - value, step), direction,
                               low - (value + spread) * term,
                               high - value * term, modulus, t))
          return false;
        if (direction == Direction::Up) {
          const Integer other = std::max(t * step + value, from);
          if (other > s)
            s = other;
        } else {
          const Integer other = std::min(t * step + value + spread, to);
          if (other < s)
            s = other;
        }
        return true;
      }

      /**
       * \brief The least x in [\a from, \a to] with (x & \a mask) == \a
       * value
       *
       * Either x is from, or x agrees with from above some bit i, where
       * from has a 0 that x may set, and takes the least bits below it:
       * value. The bits above i keep the mask only if no bit at or above
       * the highest one where from breaks it, h, is kept, so i >= h, and
       * the lowest such i gives the least x. Requires value & ~mask == 0.
       */
      inline bool least_with_mask(std::uint64_t from, std::uint64_t to,
                                  std::uint64_t mask, std::uint64_t value,
                                  std::uint64_t& x) {
        const std::uint64_t broken = (from ^ value) & mask;
        if (broken == 0) {
          x = from;
          return from <= to;
        }
        const unsigned int h =
            63 - static_cast<unsigned int>(__builtin_clzll(broken));
        const std::uint64_t raisable =
            ~from & (~mask | value) & (~std::uint64_t(0) << h);
        if (raisable == 0)
          return false;
        const unsigned int i =
            static_cast<unsigned int>(__builtin_ctzll(raisable));
        const std::uint64_t below = (std::uint64_t(1) << i) - 1;
        x = (from & ~below) | (std::uint64_t(1) << i) | (value & below);
        return x <= to;
      }

      /**
       * \brief The least x in [\a from, \a to] with known bits on x and on
       * x + \a addend
       *
       * Find the least x with from <= x <= to, (x & x_mask) == x_value and
       * ((x + addend) & sum_mask) == sum_value, all read modulo 2^64. Return
       * false when there is none.
       *
       * Bit i of the sum is x_i ^ addend_i ^ k_i, with k_i the carry into
       * bit i, and the carry out of it is the majority of the three. Going
       * up from bit 0, any[i] holds the carries into bit i that some choice
       * of bits 0..i-1 of x meeting both masks produces, and at_least[i]
       * those of choices whose value is at least from modulo 2^i. Then the
       * bits are chosen from bit 63 down, each the least one that leaves a
       * completion: the set of carries into the bits above that a choice
       * still has to produce is carried down, so that every chosen prefix
       * keeps a feasible rest. That gives the least x >= from, in 64 steps
       * of constant work.
       */
      inline bool least_with_masks(std::uint64_t from, std::uint64_t to,
                                   std::uint64_t x_mask, std::uint64_t x_value,
                                   std::uint64_t addend, std::uint64_t sum_mask,
                                   std::uint64_t sum_value, std::uint64_t& x) {
        if (from > to || (x_value & ~x_mask) || (sum_value & ~sum_mask))
          return false;
        if (sum_mask == 0)
          return least_with_mask(from, to, x_mask, x_value, x);
        // Bit k of any[i] and at_least[i]: carry k into bit i is possible
        unsigned int any[65], at_least[65];
        any[0] = at_least[0] = 1;
        for (unsigned int i = 0; i < 64; ++i) {
          any[i + 1] = at_least[i + 1] = 0;
          const unsigned int c = (addend >> i) & 1, low = (from >> i) & 1;
          for (unsigned int bit = 0; bit < 2; ++bit) {
            if (((x_mask >> i) & 1) && bit != ((x_value >> i) & 1))
              continue;
            for (unsigned int k = 0; k < 2; ++k) {
              if (((sum_mask >> i) & 1) &&
                  (bit ^ c ^ k) != ((sum_value >> i) & 1))
                continue;
              const unsigned int out = (bit & c) | (bit & k) | (c & k);
              if ((any[i] >> k) & 1)
                any[i + 1] |= 1U << out;
              if ((bit > low && ((any[i] >> k) & 1)) ||
                  (bit == low && ((at_least[i] >> k) & 1)))
                at_least[i + 1] |= 1U << out;
            }
          }
        }
        if (at_least[64] == 0)
          return false;
        std::uint64_t result = 0;
        unsigned int carries = 3;
        bool tight = true;
        for (unsigned int i = 64; i-- > 0;) {
          const unsigned int c = (addend >> i) & 1, low = (from >> i) & 1;
          bool chosen = false;
          for (unsigned int bit = 0; bit < 2 && !chosen; ++bit) {
            if ((((x_mask >> i) & 1) && bit != ((x_value >> i) & 1)) ||
                (tight && bit < low))
              continue;
            const bool still_tight = tight && bit == low;
            unsigned int next = 0;
            for (unsigned int k = 0; k < 2; ++k) {
              const unsigned int out = (bit & c) | (bit & k) | (c & k);
              if ((((sum_mask >> i) & 1) &&
                   (bit ^ c ^ k) != ((sum_value >> i) & 1)) ||
                  !((carries >> out) & 1))
                continue;
              if (((still_tight ? at_least[i] : any[i]) >> k) & 1)
                next |= 1U << k;
            }
            if (next != 0) {
              result |= std::uint64_t(bit) << i;
              carries = next;
              tight = still_tight;
              chosen = true;
            }
          }
          if (!chosen)
            return false;
        }
        x = result;
        return x <= to;
      }

      /**
       * \brief The greatest x in [\a from, \a to] with known bits on x and
       * on x + \a addend
       *
       * As least_with_masks, for the greatest such x. With x' = ~x, x <= to
       * iff x' >= ~to, and x + addend = ~(x' - addend), so the complemented
       * values of both masks give the least x'.
       */
      inline bool greatest_with_masks(std::uint64_t from, std::uint64_t to,
                                      std::uint64_t x_mask,
                                      std::uint64_t x_value,
                                      std::uint64_t addend,
                                      std::uint64_t sum_mask,
                                      std::uint64_t sum_value,
                                      std::uint64_t& x) {
        if (from > to || (x_value & ~x_mask) || (sum_value & ~sum_mask))
          return false;
        std::uint64_t complement;
        if (!least_with_masks(~to, ~from, x_mask, ~x_value & x_mask,
                              std::uint64_t(0) - addend, sum_mask,
                              ~sum_value & sum_mask, complement))
          return false;
        x = ~complement;
        return true;
      }

      /// An unknown of an aligned sum: an integer V with low <= V <= high,
      /// (V & mask) == value and V < 2^width, placed at bit \a shift
      struct AlignedTerm {
        std::uint64_t low, high, mask, value;
        unsigned int shift, width;
      };

      /**
       * \brief A condition on an aligned sum
       *
       * S = (X << x.shift) + (Y << y.shift) + constant, read modulo
       * 2^bits, must satisfy low <= S <= high, and Q = S >> shift must meet
       * the known bits mask and value. With \a ties a sum S that is a
       * multiple of 2^shift (shift >= 1) needs them on Q with bit 0 cleared
       * instead: that is how rounding to nearest, ties to even, picks the
       * even neighbour. Requires 0 <= constant, low, high < 2^bits.
       */
      struct AlignedSum {
        Integer constant;
        unsigned int bits;
        Integer low, high;
        unsigned int shift;
        std::uint64_t mask, value;
        bool ties;
      };

      /// Bit positions an aligned sum search spans at most
      constexpr unsigned int aligned_positions = 320;

      /** \brief Flags of a state of the aligned sum search
       *
       * See extreme_aligned_sum. A set of flags is an unsigned int of their
       * bits (aligned_bit), below aligned_flag_sets: the search packs it with a
       * carry into one state index and keeps sets of states as bit masks.
       */
      enum class AlignedFlag : unsigned int {
        SumAtLeast = 1,    ///< S is at least its low bound so far
        SumAtMost = 2,     ///< S is at most its high bound so far
        OtherAtLeast = 4,  ///< Y is at least y.low so far
        OtherAtMost = 8,   ///< Y is at most y.high so far
        Zeros = 16,        ///< The bits of S below its shift are all zero
        Bound = 32         ///< X is at least x.low (Up) or at most x.high
      };
      /// The number of sets of AlignedFlag
      constexpr unsigned int aligned_flag_sets = 64;
      /// The bit of flag in a set of flags
      constexpr unsigned int aligned_bit(AlignedFlag flag) {
        return static_cast<unsigned int>(flag);
      }
      /// The flags that a completed search needs: Y and X within range
      constexpr unsigned int aligned_flags_required =
          aligned_bit(AlignedFlag::OtherAtLeast) |
          aligned_bit(AlignedFlag::OtherAtMost) | aligned_bit(AlignedFlag::Bound);

      // Update flag of flags for a digit against the digit of a bound, an
      // end of a range, read from the least significant digit up: a
      // differing digit decides the comparison, an equal one keeps it. The
      // flag holds when the value lies on the inside of the bound.
      inline unsigned int compare_digit(unsigned int flags, AlignedFlag flag,
                                        unsigned int digit, unsigned int bound,
                                        End end) {
        if (digit == bound)
          return flags;
        return (digit > bound) == (end == End::Low)
                   ? flags | aligned_bit(flag)
                   : flags & ~aligned_bit(flag);
      }

      // The digit of word, a value of term, at bit p of the sum, or 2 when
      // p lies outside term
      inline unsigned int aligned_digit(const AlignedTerm& term,
                                        unsigned int p, std::uint64_t word) {
        if (p < term.shift || p - term.shift >= term.width)
          return 2;
        return static_cast<unsigned int>((word >> (p - term.shift)) & 1);
      }

      // The state after bit p of the sum, from state (carry * 64 + flags)
      // and the digits xd of x and yd of y at p, or -1 when the known bits
      // of Q exclude that bit.
      inline int aligned_step(const AlignedSum& s, const AlignedTerm& x,
                              const AlignedTerm* y, Direction direction,
                              unsigned int p, int state, unsigned int xd,
                              unsigned int yd) {
        const unsigned int carry = static_cast<unsigned int>(state) >> 6;
        unsigned int flags =
            static_cast<unsigned int>(state) % aligned_flag_sets;
        const unsigned int total =
            xd + yd + (s.constant.bit(p) ? 1 : 0) + carry;
        const unsigned int bit = total & 1;
        if (p >= s.shift && p - s.shift < 64) {
          const unsigned int q = p - s.shift;
          if (q == 0 && s.ties && (flags & aligned_bit(AlignedFlag::Zeros))) {
            if (s.mask & s.value & 1)
              return -1;
          } else if (((s.mask >> q) & 1) && bit != ((s.value >> q) & 1)) {
            return -1;
          }
        }
        if (p < s.shift && bit)
          flags &= ~aligned_bit(AlignedFlag::Zeros);
        flags = compare_digit(flags, AlignedFlag::SumAtLeast, bit,
                              s.low.bit(p) ? 1 : 0, End::Low);
        flags = compare_digit(flags, AlignedFlag::SumAtMost, bit,
                              s.high.bit(p) ? 1 : 0, End::High);
        if (y != nullptr) {
          const unsigned int low = aligned_digit(*y, p, y->low);
          if (low != 2) {
            flags = compare_digit(flags, AlignedFlag::OtherAtLeast, yd, low,
                                  End::Low);
            flags = compare_digit(flags, AlignedFlag::OtherAtMost, yd,
                                  aligned_digit(*y, p, y->high), End::High);
          }
        }
        const unsigned int low = aligned_digit(x, p, x.low);
        if (low != 2)
          flags = direction == Direction::Up
                      ? compare_digit(flags, AlignedFlag::Bound, xd, low,
                                      End::Low)
                      : compare_digit(flags, AlignedFlag::Bound, xd,
                                      aligned_digit(x, p, x.high), End::High);
        return static_cast<int>((total >> 1) * 64 + flags);
      }

      // The digits that term admits at bit p: digits[0..count-1], in the
      // order that a search in direction prefers.
      inline unsigned int aligned_options(const AlignedTerm& term,
                                          unsigned int p, Direction direction,
                                          unsigned int digits[2]) {
        const unsigned int known = aligned_digit(term, p, term.mask);
        if (known == 2) {
          digits[0] = 0;
          return 1;
        }
        if (known == 1) {
          digits[0] = aligned_digit(term, p, term.value);
          return 1;
        }
        digits[0] = direction == Direction::Up ? 0 : 1;
        digits[1] = 1 - digits[0];
        return 2;
      }

      /**
       * \brief The extreme x of an aligned sum
       *
       * Find the least (Up) or greatest (Down) X such that some Y (when \a
       * y is given; otherwise the sum has no Y) meets the condition \a s
       * (see AlignedSum), X and Y meeting their ranges and known bits.
       * Return false when there is none.
       *
       * The sum is added bit by bit from bit 0 up. Bit p of S is the sum of
       * the digits of X, Y and the constant at p and of the carry into p,
       * which is at most 2. A state at bit p holds that carry and six flags
       * about the bits below p: whether S is at least \a low and at most \a
       * high there (a differing higher bit decides a comparison, an equal
       * one keeps it), the same for Y against its range, whether the bits
       * of S below \a shift are all zero (a tie), and whether X is at least
       * x.low (Up) or at most x.high (Down) there. The known bits of Q
       * exclude transitions. So the conditions hold iff some choice of
       * digits leads from the state of the bits below the lowest digit of X
       * and Y, where only the constant contributes, to a state at the bit
       * above their highest digit from which the constant and the carry
       * alone complete them.
       *
       * A forward pass records the states that the digits below each bit
       * can reach. Then the digits of X are chosen from the highest down,
       * each the least (Up) or greatest (Down) that leaves a reachable
       * state with a completion, carrying the set of such states down. That
       * gives the extreme X, with at most 192 states per bit.
       */
      inline bool extreme_aligned_sum(const AlignedSum& s, const AlignedTerm& x,
                                      const AlignedTerm* y, Direction direction,
                                      std::uint64_t& result) {
        assert(x.width <= 64 && (y == nullptr || y->width <= 64));
        if (x.low > x.high || (x.value & ~x.mask) ||
            (y != nullptr && (y->low > y->high || (y->value & ~y->mask))))
          return false;
        if (s.low > s.high)
          return false;
        const unsigned int first =
            y != nullptr ? std::min(x.shift, y->shift) : x.shift;
        const unsigned int last =
            y != nullptr ? std::max(x.shift + x.width, y->shift + y->width)
                         : x.shift + x.width;
        assert(last < s.bits && last - first <= aligned_positions);
        // The bits of S below first are those of the constant.
        unsigned int flags = aligned_flags_required;
        if (s.constant.low_bits_zero(std::min(first, s.shift)))
          flags |= aligned_bit(AlignedFlag::Zeros);
        for (unsigned int q = 0; q < 64 && s.shift + q < first; ++q) {
          const unsigned int bit = s.constant.bit(s.shift + q) ? 1 : 0;
          if (q == 0 && s.ties && (flags & aligned_bit(AlignedFlag::Zeros))) {
            if (s.mask & s.value & 1)
              return false;
          } else if (((s.mask >> q) & 1) && bit != ((s.value >> q) & 1)) {
            return false;
          }
        }
        const Integer below = s.constant.low_bits(first);
        if (below >= s.low.low_bits(first))
          flags |= aligned_bit(AlignedFlag::SumAtLeast);
        if (below <= s.high.low_bits(first))
          flags |= aligned_bit(AlignedFlag::SumAtMost);
        // reach[i]: the states at bit first + i, one word per carry
        std::uint64_t reach[aligned_positions + 1][3];
        reach[0][0] = std::uint64_t(1) << flags;
        reach[0][1] = reach[0][2] = 0;
        for (unsigned int p = first; p < last; ++p) {
          std::uint64_t* next = reach[p - first + 1];
          next[0] = next[1] = next[2] = 0;
          unsigned int xs[2], ys[2] = {0, 0};
          const unsigned int x_count = aligned_options(x, p, direction, xs);
          const unsigned int y_count =
              y != nullptr ? aligned_options(*y, p, direction, ys) : 1;
          for (unsigned int carry = 0; carry < 3; ++carry)
            for (std::uint64_t set = reach[p - first][carry]; set;
                 set &= set - 1) {
              const int state =
                  static_cast<int>(carry * 64) + __builtin_ctzll(set);
              for (unsigned int i = 0; i < x_count; ++i)
                for (unsigned int j = 0; j < y_count; ++j) {
                  const int to =
                      aligned_step(s, x, y, direction, p, state, xs[i], ys[j]);
                  if (to >= 0)
                    next[to >> 6] |= std::uint64_t(1) << (to & 63);
                }
            }
        }
        // need[carry]: the reachable states at bit last from which the
        // constant and the carry complete the conditions
        std::uint64_t need[3] = {0, 0, 0};
        const Integer low = s.low.shift_right(last);
        const Integer high = s.high.shift_right(last);
        for (unsigned int carry = 0; carry < 3; ++carry) {
          const Integer top = (s.constant.shift_right(last) + Integer(carry))
                                  .low_bits(s.bits - last);
          const int against_low = compare(top, low);
          const int against_high = compare(top, high);
          // Whether the known bits of Q hold above last, without and with
          // all bits of S below last zero
          bool known[2] = {true, true};
          for (unsigned int zeros = 0; zeros < 2; ++zeros)
            for (unsigned int p = std::max(last, s.shift);
                 p - s.shift < 64 && (s.mask >> (p - s.shift)) != 0; ++p) {
              const unsigned int q = p - s.shift;
              const unsigned int bit = top.bit(p - last) ? 1 : 0;
              if (q == 0 && s.ties && zeros &&
                  top.low_bits_zero(s.shift - last)) {
                if (s.mask & s.value & 1)
                  known[zeros] = false;
              } else if (((s.mask >> q) & 1) &&
                         bit != ((s.value >> q) & 1)) {
                known[zeros] = false;
              }
            }
          const unsigned int required = aligned_flags_required;
          for (unsigned int f = 0; f < aligned_flag_sets; ++f) {
            if ((f & required) != required ||
                !known[(f & aligned_bit(AlignedFlag::Zeros)) ? 1 : 0])
              continue;
            if (against_low < 0 ||
                (against_low == 0 && !(f & aligned_bit(AlignedFlag::SumAtLeast))))
              continue;
            if (against_high > 0 ||
                (against_high == 0 && !(f & aligned_bit(AlignedFlag::SumAtMost))))
              continue;
            need[carry] |= std::uint64_t(1) << f;
          }
          need[carry] &= reach[last - first][carry];
        }
        if ((need[0] | need[1] | need[2]) == 0)
          return false;
        // Choose the digits of X from the highest down.
        std::uint64_t chosen = 0;
        for (unsigned int p = last; p-- > first;) {
          unsigned int xs[2], ys[2] = {0, 0};
          const unsigned int x_count = aligned_options(x, p, direction, xs);
          const unsigned int y_count =
              y != nullptr ? aligned_options(*y, p, direction, ys) : 1;
          bool found = false;
          for (unsigned int i = 0; i < x_count && !found; ++i) {
            std::uint64_t admitted[3] = {0, 0, 0};
            for (unsigned int carry = 0; carry < 3; ++carry)
              for (std::uint64_t set = reach[p - first][carry]; set;
                   set &= set - 1) {
                const int state =
                    static_cast<int>(carry * 64) + __builtin_ctzll(set);
                for (unsigned int j = 0; j < y_count; ++j) {
                  const int to = aligned_step(s, x, y, direction, p, state,
                                              xs[i], ys[j]);
                  if (to >= 0 && ((need[to >> 6] >> (to & 63)) & 1)) {
                    admitted[carry] |= set & (std::uint64_t(0) - set);
                    break;
                  }
                }
              }
            if ((admitted[0] | admitted[1] | admitted[2]) == 0)
              continue;
            found = true;
            for (unsigned int carry = 0; carry < 3; ++carry)
              need[carry] = admitted[carry];
            if (p >= x.shift && p - x.shift < x.width)
              chosen |= std::uint64_t(xs[i]) << (p - x.shift);
          }
          if (!found)
            return false;
        }
        result = chosen;
        return direction == Direction::Up ? chosen <= x.high
                                          : chosen >= x.low;
      }

      /// A term +X*2^a or -X*2^a (\a negative) of a sum, with X in [from,
      /// to], X < 2^width and (X & mask) == value
      struct SumTerm {
        std::uint64_t from, to, mask, value;
        unsigned int a, width;
        bool negative;
      };

      /// The condition low <= m <= high on a sum m >= 1, and that Q =
      /// R(m/2^b) meets the known bits mask and value, R the floor (Down),
      /// the ceiling (Up) or the nearest integer with ties to even
      struct SumTarget {
        Integer low, high;
        unsigned int b;
        std::uint64_t mask, value;
        MagnitudeRounding rounding;
      };

      // Add term to the aligned sum at scale 2^u: set t to its unknown and
      // subtract the reflection constant from constant. A negative term
      // -X = X' - (2^width - 1), X' = ~X, has its bits and range
      // complemented. Return whether the term was reflected.
      inline bool align_term(const SumTerm& term, unsigned int u,
                             AlignedTerm& t, Integer& constant) {
        const std::uint64_t all =
            term.width == 64 ? ~std::uint64_t(0)
                             : (std::uint64_t(1) << term.width) - 1;
        t = {term.from, term.to, term.mask, term.value, term.a - u,
             term.width};
        if (!term.negative)
          return false;
        t.low = all - term.to;
        t.high = all - term.from;
        t.value = ~term.value & term.mask;
        constant = constant - Integer(all).shift_left(term.a - u);
        return true;
      }

      /**
       * \brief The extreme X of a sum term
       *
       * Find the least (Up) or greatest (Down) X of the term \a x such that
       * the sum m = x + y + fixed meets \a z, for some Y of the term \a y
       * (no such term when null), \a fixed a constant that is a multiple of
       * 2^\a scale. Return false when there is none.
       *
       * Let u = min(a_x, a_y, scale, b), so that m = m'*2^u with m' an
       * integer, and k = b - u. Then R(m/2^b) = (m' + w) >> k with w = 0
       * for the floor, 2^k - 1 for the ceiling and 2^(k-1) for the nearest
       * integer, where a tie, (m' + w) a multiple of 2^k, rounds to the
       * even of (m' + w) >> k and the one below it: that clears bit 0. And
       * low <= m <= high iff ceil(low/2^u) <= m' <= floor(high/2^u). So the
       * condition is AlignedSum on S = m' + w, with the negative terms
       * reflected (align_term) and S read modulo 2^bits: bits leaves room
       * above both |S| and the bound, so that a negative S reads as a value
       * above the bound and a sum in range is its own residue.
       */
      inline bool sum_term_extreme(const SumTerm& x, const SumTerm* y,
                                   const Integer& fixed, unsigned int scale,
                                   const SumTarget& z, Direction direction,
                                   std::uint64_t& result) {
        using Arithmetic::Global::ceil_shift_right;
        using Arithmetic::Global::floor_shift_right;
        if (x.from > x.to || (y != nullptr && y->from > y->to))
          return false;
        unsigned int u = std::min(x.a, z.b);
        if (y != nullptr)
          u = std::min(u, y->a);
        if (!fixed.zero())
          u = std::min(u, scale);
        const unsigned int k = z.b - u;
        Integer w;
        if (z.rounding == MagnitudeRounding::Up)
          w = Integer::power_two(k) - Integer(1);
        else if (z.rounding == MagnitudeRounding::Nearest && k > 0)
          w = Integer::power_two(k - 1);
        AlignedSum s;
        s.low = ceil_shift_right(z.low, u) + w;
        s.high = floor_shift_right(z.high, u) + w;
        s.shift = k;
        s.mask = z.mask;
        s.value = z.value;
        s.ties = z.rounding == MagnitudeRounding::Nearest && k > 0;
        Integer constant = fixed.shift_right(u) + w;
        assert(fixed.low_bits_zero(u));
        AlignedTerm xt, yt;
        const bool reflected = align_term(x, u, xt, constant);
        if (y != nullptr)
          align_term(*y, u, yt, constant);
        unsigned int bits =
            std::max(x.a - u + x.width, std::max(k, s.high.bits()));
        if (y != nullptr)
          bits = std::max(bits, y->a - u + y->width);
        bits = std::max(bits, constant.abs().bits()) + 3;
        if (constant.neg())
          constant = constant + Integer::power_two(bits);
        s.constant = constant;
        s.bits = bits;
        const unsigned int first =
            y != nullptr ? std::min(xt.shift, yt.shift) : xt.shift;
        const unsigned int last =
            y != nullptr ? std::max(xt.shift + xt.width, yt.shift + yt.width)
                         : xt.shift + xt.width;
        if (last - first > aligned_positions) {
          // Never reached by the sums of float words (see the callers):
          // keep every X.
          result = direction == Direction::Up ? x.from : x.to;
          return true;
        }
        const Direction search =
            reflected ? (direction == Direction::Up ? Direction::Down
                                                    : Direction::Up)
                      : direction;
        std::uint64_t found;
        if (!extreme_aligned_sum(s, xt, y != nullptr ? &yt : nullptr, search,
                                 found))
          return false;
        if (reflected) {
          const std::uint64_t all =
              x.width == 64 ? ~std::uint64_t(0)
                            : (std::uint64_t(1) << x.width) - 1;
          found = all - found;
        }
        result = found;
        return true;
      }

      // The least ceiling and the greatest floor of the quotients of the
      // endpoints of product and divisor. One division per distinct pair
      // gives both its floor and its ceiling.
      inline RoundingCell quotient_bounds(const RoundingCell& product,
                                          const FinitePart& divisor) {
        assert(!divisor.low.zero() && !divisor.high.zero());
        const Integer* numerators[] = {&product.low, &product.high};
        const Integer* denominators[] = {&divisor.low, &divisor.high};
        const int numerator_count = product.low == product.high ? 1 : 2;
        const int denominator_count = divisor.low == divisor.high ? 1 : 2;
        RoundingCell bounds;
        for (int i = 0; i < numerator_count; ++i)
          for (int j = 0; j < denominator_count; ++j) {
            Integer floor, ceiling;
            floor_ceil_div(*numerators[i], *denominators[j], floor, ceiling);
            if (i == 0 && j == 0) {
              bounds = {std::move(ceiling), std::move(floor)};
              continue;
            }
            if (ceiling < bounds.low)
              bounds.low = std::move(ceiling);
            if (floor > bounds.high)
              bounds.high = std::move(floor);
          }
        return bounds;
      }

      /*
       * Reads of the widened projections (the closed forms of the fma, and
       * the step rule of the wide products, quotients and fmas)
       *
       * Two reads. The hull read takes a word from its least to its greatest
       * code, and the result from the cell of its least to that of its greatest
       * code. The widened read takes a word from the value of the greatest code
       * below its interval that the word's read cube admits (pred) to the value
       * of the least such code above it (succ), and the result over the cells
       * of the codes strictly between its pred and succ. Both contain every
       * value and cell of the domain, so both projections are sound, and the
       * hull read, being narrower, prunes at least as much.
       *
       * Why a widened pass is a fixpoint of itself. Let B be the widened box
       * and P(B) the exact projection of a piece of the relation (such as one
       * sign part of x and of y + 1 for a == c) whose projections all contain
       * an admitted code; other pieces are dropped. Publishing keeps, for each
       * word, every admitted code with a value in the piece's projection. Take
       * u = pred of a published word. If its value exceeded the least value of
       * the projection, u would lie above the old pred (values are monotone in
       * the rank and u's value is above the old read), so u, admitted by the
       * old cube, would be a code of the old domain inside the projection,
       * hence published, hence not below the new interval. So the new read
       * contains the projection, the piece's projection at the new box
       * contains the old one, and a second pass keeps every published code.
       * The argument for the result is the same with cells in place of values;
       * it needs cell bounds that are nondecreasing in the rank and leave no
       * gap (result_code_cell). The hull read has no such argument: a
       * projection can end between two codes, and the hull of the codes kept
       * then reads less than the projection, so repeating it can move the
       * bounds a few codes per pass. That is why a hull step is kept only
       * where it is a fixpoint of itself (WideFma::propagate_closed_form), or
       * where it makes progress (the step rule, below).
       *
       * The widened reads search the stored cube without the bits that the
       * closure derives from the interval alone (interval_bits, read_cube);
       * with them, a domain such as [1, 1.5], which starts a block of ranks,
       * would have no pred. The argument above needs the cube of the second
       * read to admit no code that the cube of the first read excluded, and a
       * code of the old interval's blocks admitted by the first read cube to be
       * admitted by the old stored cube. Both hold while interval_bits stays
       * the same; when it gains a known bit, the second read no longer sees
       * that bit, and the pass can prune again.
       */

      /// How a widened projection reads the domains
      enum class Read {
        /// From the least to the greatest code of each domain
        Hull,
        /// From pred to succ of each domain
        Widened
      };

      /// The value of code as an input reads it: subnormals are zero when
      /// inputs flush, and infinities and NaNs, which only a widened read can
      /// reach beyond the finite domain of an input, read as the largest
      /// finite value. The fixpoint argument holds with that value in place of
      /// the code's: every finite input value lies at or below it, and a
      /// widened read that reaches a non-finite code reaches no finite one.
      inline Integer read_value(const WideFormat& f, const Policy& p, WordValue code) {
        const WordValue magnitude = code & (f.sign - 1);
        Integer v;
        if (magnitude >= f.infinity)
          v = f.magnitude(f.maximum);
        else if (magnitude >= f.normal ||
                 p.input_subnormals == Policy::Preserve)
          v = f.magnitude(magnitude);
        return (code & f.sign) ? -v : v;
      }

      /// The bits that the closure fixes from the interval of d alone: the
      /// common prefix of the ranks of each sign half of the interval, and of
      /// the encodings the bits common to both halves.
      inline WordValue interval_bits(WordView view, const Word::WordDomain& d) {
        const WordValue mask = Word::width_mask(view.width());
        const Word::WordDomainResult closed = Word::compute_domain_closure(
            view.width(), view.domain_type(), {{0, mask}, d.interval});
        return mask & ~(closed.domain.cube.lo ^ closed.domain.cube.hi);
      }

      /// The cube that the widened reads search: the stored cube without the
      /// bits of interval_bits. With them, an interval at the start or end of
      /// its block has no pred or no succ, and its read reaches beyond every
      /// finite value; so does a result {-0, +0}, whose closure knows every
      /// magnitude bit.
      inline Word::WordCube read_cube(WordView view, const Word::WordDomain& d) {
        const WordValue known = interval_bits(view, d);
        return {d.cube.lo & ~known, d.cube.hi | known};
      }

      /// Closed range of read values of a multiplicand
      struct ValueRange {
        Integer low, high;
        bool empty() const { return low > high; }
      };

      /// The read of the multiplicand view with domain d. The hull read spans
      /// the values of the ends of the interval, which are admitted codes, as
      /// every domain here is closed. The widened read spans from the value of
      /// pred to the value of succ, or to the largest finite value where there
      /// is no such code. An assigned word reads as its value under both
      /// reads: its domain cannot change, so the fixpoint argument holds for
      /// it trivially.
      inline ValueRange read_word(const WideFormat& f, const Policy& p, WordView view,
                                  const Word::WordDomain& d, Read read) {
        const WordDomainType type = view.domain_type();
        const unsigned int width = view.width();
        if (read == Read::Hull || d.interval.minimum == d.interval.maximum)
          return {read_value(f, p, Word::encode_rank(type, width, d.interval.minimum)),
                  read_value(f, p, Word::encode_rank(type, width, d.interval.maximum))};
        const Word::WordCube cube = read_cube(view, d);
        const Integer finite = f.magnitude(f.maximum);
        ValueRange range{-finite, finite};
        WordValue r;
        if (d.interval.minimum > 0 &&
            Word::previous_admitted_rank(type, width, cube,
                                         d.interval.minimum - 1, r))
          range.low = read_value(f, p, Word::encode_rank(type, width, r));
        if (d.interval.maximum < (f.sign << 1) - 1 &&
            Word::next_admitted_rank(type, width, cube, d.interval.maximum + 1,
                                     r))
          range.high = read_value(f, p, Word::encode_rank(type, width, r));
        return range;
      }

      /// The sums that round to 0: from just above the cell of the least
      /// negative nonzero result to just below that of the least positive
      /// one, which are the subnormals of least magnitude, or the least
      /// normals when outputs flush. Rounding to the subnormal grid and then
      /// flushing sends every sum between those cells to a zero, and no other
      /// sum, so the cell is exact; tininess only sets flags. Both zeros get
      /// this cell: the sign of a zero is not reasoned about.
      inline RoundingCell zero_cell(const WideFormat& f, const Policy& p,
                                    unsigned int scale) {
        const WordValue least =
            p.output_subnormals == Policy::SignedZero ? f.normal : 1;
        return {rounding_cell(f, p, f.sign | least, scale, CellEnds::Open).high + Integer(1),
                rounding_cell(f, p, least, scale, CellEnds::Open).low - Integer(1)};
      }

      /// The sums, at scale 2^scale over the source unit, that a result code
      /// stands for. Codes below the maximum have their open rounding_cell, the zeros
      /// zero_cell. The bounds are nondecreasing in the rank and consecutive
      /// codes leave no gap, which the fixpoint argument needs; codes that no
      /// finite sum produces get an empty cell only at the ends of the order
      /// (infinity without overflow to it, NaN), or, for subnormals under
      /// output flushing, the empty cell between the cells of their
      /// neighbours. big bounds every sum of the box.
      inline RoundingCell result_code_cell(const WideFormat& f, const Policy& p,
                                           WordValue code, unsigned int scale,
                                           const Integer& big) {
        const WordValue magnitude = code & (f.sign - 1);
        const bool negative = (code & f.sign) != 0;
        if (magnitude > f.infinity)
          return negative ? RoundingCell{-big, -big - Integer(1)}
                          : RoundingCell{big + Integer(1), big};
        if (magnitude == 0)
          return zero_cell(f, p, scale);
        if (magnitude < f.maximum) {
          if (p.output_subnormals == Policy::SignedZero &&
              magnitude < f.normal) {
            // Never a result: empty, between the zero cell and the cell of the
            // least normal of the sign.
            const RoundingCell zero = zero_cell(f, p, scale);
            const RoundingCell least =
                rounding_cell(f, p, (code & f.sign) | f.normal, scale, CellEnds::Open);
            return negative ? RoundingCell{zero.low, least.high}
                            : RoundingCell{least.low, zero.high};
          }
          return rounding_cell(f, p, code, scale, CellEnds::Open);
        }
        // The maximum and the infinity, in magnitudes. A magnitude rounds to
        // nearest, away from zero or toward zero; overflow reaches infinity
        // only under FormatDefault and not toward zero. Rounding to nearest
        // overflows from half an ulp above the maximum (the tie rounds to the
        // even 2^emax+1), rounding away from zero from just above it.
        const bool nearest = p.rounding == Policy::RNE;
        const bool away = magnitude_rounding(p.rounding, negative) ==
                          MagnitudeRounding::Up;
        const bool to_infinity = overflows_to_infinity(p, negative);
        const Integer top = f.magnitude(f.maximum).shift_left(scale);
        const Integer below = f.magnitude(f.maximum - 1).shift_left(scale);
        const Integer threshold =
            nearest ? top + (top - below).shift_right(1) : top + Integer(1);
        Integer low, high;
        if (magnitude == f.maximum) {
          // The maximum has an odd significand: a tie below it rounds down.
          low = nearest ? (below + top).shift_right(1) + Integer(1)
                        : away ? below + Integer(1) : top;
          high = to_infinity ? threshold - Integer(1) : big;
        } else {
          if (!to_infinity)
            return negative ? RoundingCell{-big, -big - Integer(1)}
                            : RoundingCell{big + Integer(1), big};
          low = threshold;
          high = big;
        }
        return negative ? RoundingCell{-high, -low} : RoundingCell{low, high};
      }

      /// The finite ends of the two cells that reach beyond every finite sum:
      /// the cell of the maximum, or of the infinity where sums overflow to
      /// it, of each sign. low is the upper end of the negative one, high the
      /// lower end of the positive one.
      inline RoundingCell unbounded_cell_ends(const WideFormat& f, const Policy& p,
                                              unsigned int scale, const Integer& big) {
        const RoundingCell negative =
            result_code_cell(f, p, f.sign | f.maximum, scale, big);
        const RoundingCell positive =
            result_code_cell(f, p, f.maximum, scale, big);
        return {negative.low == -big
                    ? negative.high
                    : result_code_cell(f, p, f.sign | f.infinity, scale, big)
                          .high,
                positive.high == big
                    ? positive.low
                    : result_code_cell(f, p, f.infinity, scale, big).low};
      }

      /// Whether no sum rounds to the result code: a subnormal when outputs
      /// flush
      inline bool never_result(const WideFormat& f, const Policy& p, WordValue code) {
        const WordValue magnitude = code & (f.sign - 1);
        return p.output_subnormals == Policy::SignedZero && magnitude > 0 &&
               magnitude < f.normal;
      }

      /// The rank of code in the order of view
      inline WordValue rank_of(WordView view, WordValue code) {
        return Word::rank_interval(view.domain_type(), view.width(), code, code)
            .minimum;
      }

      /// The signs of the results that one piece of a projection produces
      enum class ResultSign {
        /// Both signs (sums)
        Any,
        /// Positive results only (a product or quotient of one sign)
        Positive,
        /// Negative results only
        Negative
      };

      /**
       * \brief The cells of the result codes of a projection
       *
       * The cell of a code is result_code_cell at 2^scale over the unit of
       * the exact values (sums, products or quotients), with these variants:
       *  - ends Closed: every finite end is moved out by one unit. Quotients are
       *    not on the integer lattice of the scale, so the one-unit exclusion
       *    of an open end is unsound for them; the closed cell holds the real
       *    cell.
       *  - sign: a piece whose exact values have one sign gives the codes of
       *    the other sign the empty cell at the far end of the order, and
       *    clips the cells of its own sign and of both zeros to its half
       *    [1, big] or [-big, -1]. Such a piece never has an exact value of
       *    magnitude below 1 (products of values of magnitude at least 1,
       *    and quotients at a scale where the least one is above 1), so the
       *    clipping removes no value, and both zeros keep the same cell.
       * Every variant keeps what the fixpoint argument of the widened read
       * needs: cell bounds that are nondecreasing in the rank, and no gap
       * between the cells of consecutive codes inside the half of the
       * piece. Codes that are never results (never) keep an empty cell
       * between those of their neighbours.
       */
      struct ResultCells {
        const WideFormat& f;
        const Policy& p;
        unsigned int scale;
        /// Bounds the magnitude of every exact value of the projection
        Integer big;
        ResultSign sign;
        CellEnds ends;
        RoundingCell cell(WordValue code) const {
          RoundingCell c = result_code_cell(f, p, code, scale, big);
          if (ends == CellEnds::Closed && c.low <= c.high) {
            if (c.low != -big)
              c.low = c.low - Integer(1);
            if (c.high != big)
              c.high = c.high + Integer(1);
          }
          const bool negative = (code & f.sign) != 0;
          const bool zero = (code & (f.sign - 1)) == 0;
          if (sign == ResultSign::Positive) {
            if (negative && !zero)
              return {-big, -big - Integer(1)};
            if (c.low < Integer(1))
              c.low = Integer(1);
          } else if (sign == ResultSign::Negative) {
            if (!negative && !zero)
              return {big + Integer(1), big};
            if (c.high > -Integer(1))
              c.high = -Integer(1);
          }
          return c;
        }
        bool never(WordValue code) const { return never_result(f, p, code); }
        /// The finite ends of the cells that reach beyond every finite value
        RoundingCell cut() const {
          RoundingCell c = unbounded_cell_ends(f, p, scale, big);
          if (ends == CellEnds::Closed) {
            c.low = c.low + Integer(1);
            c.high = c.high - Integer(1);
          }
          return c;
        }
      };

      /// The read of the result view with domain d. The hull read spans the
      /// cells of the ends of the interval. The widened read spans the cells
      /// of the codes strictly between pred and succ (the ends of the order
      /// where there is none), with two refinements:
      ///  - Pred and succ skip the codes that are never results, which
      ///    publication removes (include_results), so the read is the same with
      ///    and without them.
      ///  - A code beyond the interval whose cell reaches beyond every finite
      ///    sum (the maximum under saturation or toward zero, the infinity
      ///    where sums overflow to it, the NaNs) contributes only up to the
      ///    finite end of that cell (unbounded_cell_ends), and the read never
      ///    ends inside the cells of the ends of the interval. So it holds
      ///    every cell of the domain and is sound. It keeps the fixpoint
      ///    argument: the cut is the same at the next pass while pred stays, a
      ///    pred that moves into the old interval reads no unbounded cell, and
      ///    every bounded cell lies inside the cut.
      /// An assigned result reads as the cell of its code under both reads.
      inline RoundingCell read_result(WordView view, const Word::WordDomain& d,
                                      const ResultCells& cells, Read read) {
        const WideFormat& f = cells.f;
        const WordDomainType type = view.domain_type();
        const unsigned int width = view.width();
        const RoundingCell first =
            cells.cell(Word::encode_rank(type, width, d.interval.minimum));
        const RoundingCell last =
            cells.cell(Word::encode_rank(type, width, d.interval.maximum));
        if (read == Read::Hull || d.interval.minimum == d.interval.maximum)
          return {first.low, last.high};
        const WordValue end = (f.sign << 1) - 1;
        WordValue first_rank = 0, last_rank = end, r;
        const Word::WordCube cube = read_cube(view, d);
        // Below the negative subnormals lie the negative normals, below the
        // positive ones the zeros; above them the zeros and the positive
        // normals.
        WordValue from = d.interval.minimum;
        while (from > 0 &&
               Word::previous_admitted_rank(type, width, cube, from - 1, r)) {
          const WordValue code = Word::encode_rank(type, width, r);
          if (!cells.never(code)) {
            first_rank = r + 1;
            break;
          }
          from = (code & f.sign) ? rank_of(view, f.sign | f.normal) + 1
                                 : rank_of(view, 0) + 1;
        }
        from = d.interval.maximum;
        while (from < end &&
               Word::next_admitted_rank(type, width, cube, from + 1, r)) {
          const WordValue code = Word::encode_rank(type, width, r);
          if (!cells.never(code)) {
            last_rank = r - 1;
            break;
          }
          from = (code & f.sign) ? rank_of(view, f.sign) - 1
                                 : rank_of(view, f.normal) - 1;
        }
        const RoundingCell cut = cells.cut();
        return {std::max(cells.cell(Word::encode_rank(type, width, first_rank)).low,
                         std::min(cut.low, first.low)),
                std::min(cells.cell(Word::encode_rank(type, width, last_rank)).high,
                         std::max(cut.high, last.high))};
      }

      /// read_result for the cells of sums at 2^scale (ResultSign::Any)
      inline RoundingCell read_result(const WideFormat& f, const Policy& p,
                                      WordView view, const Word::WordDomain& d,
                                      unsigned int scale, const Integer& big,
                                      Read read) {
        return read_result(
            view, d,
            ResultCells{f, p, scale, big, ResultSign::Any, CellEnds::Open},
            read);
      }

      /// Include in projection the codes of the domain d of view whose read
      /// value lies in [low, high].
      inline void include_values(const WideFormat& f, const Policy& p,
                                 WideProjection& projection, WordView view,
                                 const Word::WordDomain& d, const Integer& low,
                                 const Integer& high) {
        if (low > high)
          return;
        // The least magnitude code with a nonzero read value
        const WordValue least =
            p.input_subnormals == Policy::SignedZero ? f.normal : 1;
        if (low <= Integer() && Integer() <= high)
          for (WordValue sign : {WordValue(0), f.sign})
            projection.include(view, d, sign, sign | (least - 1));
        for (WordValue sign : {WordValue(0), f.sign}) {
          // The magnitudes of this sign in [minimum, maximum]
          Integer minimum = sign ? -high : low;
          const Integer maximum = sign ? -low : high;
          if (minimum < Integer(1))
            minimum = Integer(1);
          if (minimum > maximum)
            continue;
          WordValue first = floor_encoding(f, minimum);
          if (f.magnitude(first) < minimum)
            ++first;
          first = std::max(first, least);
          const WordValue last = floor_encoding(f, maximum);
          if (first <= last)
            projection.include(view, d, sign | first, sign | last);
        }
      }

      /// Include in projection the result codes of d between the codes first
      /// and last of one sign, except, when outputs flush, the subnormals: no
      /// value rounds to them, and their cells lie between those of their
      /// neighbours, so the read of the result is the same without them.
      inline void include_results(const ResultCells& cells,
                                  WideProjection& projection, WordView view,
                                  const Word::WordDomain& d, WordValue first,
                                  WordValue last) {
        const WideFormat& f = cells.f;
        if (cells.p.output_subnormals != Policy::SignedZero) {
          projection.include(view, d, first, last);
          return;
        }
        const WordValue sign = first & f.sign;
        WordValue low = std::min(first, last) & (f.sign - 1);
        const WordValue high = std::max(first, last) & (f.sign - 1);
        if (low == 0)
          projection.include(view, d, sign, sign);
        low = std::max(low, f.normal);
        if (low <= high)
          projection.include(view, d, sign | low, sign | high);
      }

      /// Include in projection the codes of the result domain d of view whose
      /// cell meets [low, high]. The cell bounds are monotone in the rank, so
      /// these codes form one rank range, found by bisection.
      inline void include_meeting(WideProjection& projection, WordView view,
                                  const Word::WordDomain& d,
                                  const ResultCells& cells, const Integer& low,
                                  const Integer& high) {
        if (low > high)
          return;
        const WideFormat& f = cells.f;
        const WordDomainType type = view.domain_type();
        const unsigned int width = view.width();
        // The least rank whose cell reaches low
        WordValue a = 0, b = (f.sign << 1);
        while (a < b) {
          const WordValue middle = a + (b - a) / 2;
          if (cells.cell(Word::encode_rank(type, width, middle)).high >= low)
            b = middle;
          else
            a = middle + 1;
        }
        const WordValue first = a;
        // One past the greatest rank whose cell starts at or below high
        a = first;
        b = (f.sign << 1);
        while (a < b) {
          const WordValue middle = a + (b - a) / 2;
          if (cells.cell(Word::encode_rank(type, width, middle)).low > high)
            b = middle;
          else
            a = middle + 1;
        }
        if (a == first)
          return;
        const WordValue last = a - 1;
        // Negative codes have the ranks below the sign bit.
        if (first < f.sign)
          include_results(cells, projection, view, d,
                          Word::encode_rank(type, width, first),
                          Word::encode_rank(type, width,
                                            std::min(last, f.sign - 1)));
        if (last >= f.sign)
          include_results(cells, projection, view, d,
                          Word::encode_rank(type, width, std::max(first, f.sign)),
                          Word::encode_rank(type, width, last));
      }

      /// include_meeting for the cells of sums at 2^scale (ResultSign::Any)
      inline void include_meeting(const WideFormat& f, const Policy& p,
                                  WideProjection& projection, WordView view,
                                  const Word::WordDomain& d, unsigned int scale,
                                  const Integer& big, const Integer& low,
                                  const Integer& high) {
        include_meeting(
            projection, view, d,
            ResultCells{f, p, scale, big, ResultSign::Any, CellEnds::Open},
            low, high);
      }

      /// Merge the projection part of one piece into the projection whole
      inline void merge(WideProjection& whole, WordView view, const WideProjection& part) {
        if (part.present)
          whole.include(view, {part.cube, part.interval});
      }

      /// Integer hull [ceil(min), floor(max)] of the quotients n/d over the
      /// corners of [n_low, n_high] x [d_low, d_high], where d keeps one sign:
      /// the integers of the real set {n/d}, whose ends are at corners because
      /// n/d is monotone in each argument on such a box.
      inline ValueRange corner_quotients(const Integer& n_low, const Integer& n_high,
                                         const Integer& d_low, const Integer& d_high) {
        using Word::Arithmetic::Global::ceil_div;
        using Word::Arithmetic::Global::floor_div;
        ValueRange q{ceil_div(n_low, d_low), floor_div(n_low, d_low)};
        for (const Integer& n : {n_low, n_high})
          for (const Integer& d : {d_low, d_high}) {
            q.low = std::min(q.low, ceil_div(n, d));
            q.high = std::max(q.high, floor_div(n, d));
          }
        return q;
      }

      /*
       * The step rule of the wide products and quotients of distinct words
       * and of the wide fmas but the closed forms (WideArithmetic and
       * WideFma)
       *
       * Two projections narrow the domains D of the words. W reads every
       * word widened, from pred to succ, and projects the real relation on
       * that box (project_widened, project_widened_fma); a word that plays
       * several roles of an fma is read once for all of them and takes the
       * union of their projections (WideFma::project_step). H is the interval
       * projection of the operation, with its constant-cost closed forms,
       * intersected with W on the hull read. For a product or quotient with
       * a status, both also read S, the flag values that the status admits,
       * dropping the pairs of parts whose flags S excludes, and narrow S to
       * the flags of the pairs they keep. A run of the step rule g holds one
       * S: every projection of the run reads the current S and narrows it.
       * A run computes, from D, comparing domains only:
       *  1. E := H(D); fail if E is empty; stop at D if E = D.
       *  2. If H(E) = E, stop at E.
       *  3. If E progresses over D, D := E and repeat from 2 with H(E) as
       *     the next E. E progresses when for some word 2*|E_i| <= |D_i|
       *     (rank intervals), or interval_bits of E_i has a bit that the
       *     one of D_i has not.
       *  4. Otherwise D' := W(D); fail if D' is empty; stop at D if D' = D,
       *     else D := D' and repeat from 1.
       * Runs repeat, each from the domains and S where the last one
       * stopped, until a run ends with the S it started with.
       *
       * The status takes no part in the comparisons: a projection that
       * narrows only S is no step of the domains, so it never sends a run
       * to W(D). The narrower S then reaches the domains through the
       * projections that follow and through the next run.
       *
       * Soundness: H and W keep every solution in D and its flags in S, so
       * every state of every run does.
       *
       * Honesty: g ends with a run from (D0, S) to D* in which S did not
       * change. Every projection of that run read S, so a run from (D*, S)
       * makes the same deterministic choices and stops at once:
       *  - at the exit of step 1, D* = D and H(D*) = D*;
       *  - at the exit of step 2, D* = E with H(E) = E, so the next run
       *    computes H(D*) = D* at step 1;
       *  - at the exit of step 4, W(D*) = D*. D* is where the run began the
       *    repetition (D0, the result of a W step, or the D of a progress
       *    step, whose next E was H(D*)), so the next run repeats it: the
       *    same E = H(D*), the same tests, and W(D*) = D* again.
       * So g(D*) = D*, and reporting ES_FIX after publishing D* and S is
       * honest. D* is also a fixpoint of W. At step 4 that is the exit
       * test. At steps 1 and 2, H(D*) = D*, and H(D) is contained in W(D),
       * as the hull read is contained in the widened read and W is monotone
       * in the read, so W(D*) = D*. That is bounds consistency of the real
       * relation on the widened box.
       *
       * The number of projections is bounded by the widths of the words,
       * the special classes and the status values, not by the domain sizes.
       * Domains only shrink, also from one run to the next, so interval_bits
       * only grows: a word halves its rank interval at most width times and
       * gains at most width bits, which bounds the progress steps of all
       * runs together by 2 * (sum of the widths). Every run but the last
       * removes a value from S, so there are at most 33 runs (the status
       * has 32 values). Between two progress steps of a run, D changes only
       * by W steps, and a W step on the result of a W step changes nothing
       * unless the read of some word changed beyond shrinking or S lost a
       * value: a bit newly fixed by interval_bits, a special class or a
       * status value removed (the argument before Read above). So the W
       * steps that change D number at most one at the start of each run and
       * after each progress step, plus one per bit, special class and status
       * value, and every repetition does at most three projections. A
       * single wide propagator settles this way in a bounded number of
       * steps; slow convergence can only come from several propagators
       * sharing words.
       */

      /// The state that the step rule narrows
      template<int n>
      struct StepState {
        Word::WordDomain domains[n];
        /// The flag values that the status admits, one bit per value
        unsigned int statuses;
      };

      /// Outcome of the step rule
      enum class StepOutcome { Failed, Settled };

      /// A projection of the step rule: it narrows the state by H for
      /// Read::Hull and by W for Read::Widened, and returns false when it
      /// fails
      template<int n, class Projector>
      using StepProjection = bool (Projector::*)(Read, StepState<n>&) const;

      /// Whether \a x and \a y have the same domains (their statuses aside)
      template<int n>
      bool same_domains(const StepState<n>& x, const StepState<n>& y) {
        for (int i = 0; i < n; ++i) {
          const Word::WordDomain& a = x.domains[i];
          const Word::WordDomain& b = y.domains[i];
          if (a.cube.lo != b.cube.lo || a.cube.hi != b.cube.hi ||
              a.interval.minimum != b.interval.minimum ||
              a.interval.maximum != b.interval.maximum)
            return false;
        }
        return true;
      }

      /// Whether \a e progresses over \a d: for some word, \a e halves the
      /// rank interval or fixes a bit of interval_bits that \a d does not
      template<int n>
      bool progresses(const WordView views[n], const StepState<n>& e,
                      const StepState<n>& d) {
        for (int i = 0; i < n; ++i) {
          const auto& ei = e.domains[i].interval;
          const auto& di = d.domains[i].interval;
          // Wide words have at most 32 bits, so the sizes do not overflow.
          if (2 * (ei.maximum - ei.minimum + 1) <= di.maximum - di.minimum + 1)
            return true;
          if ((interval_bits(views[i], e.domains[i]) &
               ~interval_bits(views[i], d.domains[i])) != 0)
            return true;
        }
        return false;
      }

      /// Narrow \a state by \a project with \a read, from the flag values
      /// \a statuses of the run, and narrow \a statuses to the result
      template<int n, class Projector>
      bool project_in_run(const Projector& projector,
                          StepProjection<n, Projector> project, Read read,
                          StepState<n>& state, unsigned int& statuses) {
        state.statuses = statuses;
        if (!(projector.*project)(read, state))
          return false;
        statuses = state.statuses;
        return true;
      }

      /// One run of the step rule (see above) from the domains of \a d,
      /// with the flag values \a statuses, which every projection narrows
      template<int n, class Projector>
      StepOutcome run_step_once(const Projector& projector,
                                StepProjection<n, Projector> project,
                                const WordView views[n], StepState<n>& d,
                                unsigned int& statuses) {
        StepState<n> e = d;
        if (!project_in_run(projector, project, Read::Hull, e, statuses))
          return StepOutcome::Failed;
        for (;;) {
          if (same_domains(e, d))
            return StepOutcome::Settled;
          StepState<n> again = e;
          if (!project_in_run(projector, project, Read::Hull, again, statuses))
            return StepOutcome::Failed;
          if (same_domains(again, e)) {
            d = e;
            return StepOutcome::Settled;
          }
          if (progresses(views, e, d)) {
            d = e;
            e = again;
            continue;
          }
          StepState<n> next = d;
          if (!project_in_run(projector, project, Read::Widened, next,
                              statuses))
            return StepOutcome::Failed;
          if (same_domains(next, d))
            return StepOutcome::Settled;
          d = next;
          e = d;
          if (!project_in_run(projector, project, Read::Hull, e, statuses))
            return StepOutcome::Failed;
        }
      }

      /// Narrow \a d by the step rule g (see above): runs until one ends
      /// with the statuses it started with
      template<int n, class Projector>
      StepOutcome run_step_rule(const Projector& projector,
                                StepProjection<n, Projector> project,
                                const WordView views[n], StepState<n>& d) {
        for (;;) {
          const unsigned int start = d.statuses;
          unsigned int statuses = start;
          if (run_step_once(projector, project, views, d, statuses) ==
              StepOutcome::Failed)
            return StepOutcome::Failed;
          d.statuses = statuses;
          if (statuses == start)
            return StepOutcome::Settled;
        }
      }

    } // namespace Float
  } // namespace Word
} // namespace Gecode

#endif

// STATISTICS: word-other
