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
#include <gecode/float-word.hh>
#include <gecode/word/arithmetic/global-integer.hpp>
#include <utility>

namespace Gecode {
  namespace Word {
    namespace Float {
      using Integer = Arithmetic::Global::Integer;
      using Policy = FloatWordPolicy;

      /**
       * \name Operation status flags
       *
       * The bits of an operation status in 0..31, as documented in
       * docs/float-word-contracts.md.
       */
      //@{
      /// Invalid operation
      constexpr int invalid = 1;
      /// Division of a nonzero finite operand by zero
      constexpr int divide_by_zero = 2;
      /// Rounded result exceeds the largest finite magnitude
      constexpr int overflow = 4;
      /// Tiny and inexact result
      constexpr int underflow = 8;
      /// Rounded result differs from the exact result
      constexpr int inexact = 16;
      //@}

      struct WideFormat {
        unsigned int fraction_bits;
        WordValue sign, normal, infinity, maximum, quiet;
        explicit WideFormat(FloatWordFormat f)
            : fraction_bits(f == FWF_BF16 ? 7 : 23),
              sign(WordValue(1) << (fraction_bits + 8)),
              normal(WordValue(1) << fraction_bits), infinity(255 * normal),
              maximum(infinity - 1), quiet(normal >> 1) {}

        // Half-minimum-subnormal units: 2^-134 (BF16), 2^-150 (FP32).
        // The extra infinity code denotes the next finite binade here only.
        Integer magnitude(WordValue code) const {
          const unsigned int exponent =
              static_cast<unsigned int>(code >> fraction_bits);
          return exponent == 0 ? Integer(code * 2)
                               : Integer(normal + (code & (normal - 1))) *
                                     Integer::power_two(exponent);
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
          return (value / Integer(2)).word();
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

      inline Integer round_coefficient(const Integer& magnitude,
                                       unsigned int shift, bool negative,
                                       Policy::Rounding rounding) {
        const Integer step = Integer::power_two(shift);
        Integer quotient, remainder;
        Integer::divide(magnitude, step, quotient, remainder);
        bool increment = false;
        if (!remainder.zero()) {
          switch (rounding) {
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
        return (quotient + Integer(increment ? 1 : 0)) * step;
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
        const Integer scale =
            Integer::power_two(static_cast<unsigned int>(alignment));
        const unsigned int precision_shift =
            magnitude.bits() > f.fraction_bits + 1
                ? magnitude.bits() - f.fraction_bits - 1
                : 0;
        const Integer precision =
            round_coefficient(magnitude, precision_shift, negative, p.rounding);
        if (precision > f.magnitude(f.maximum) * scale) {
          const bool to_infinity = p.overflow == Policy::Format &&
                                   (p.rounding == Policy::RNE ||
                                    (p.rounding == Policy::RUP && !negative) ||
                                    (p.rounding == Policy::RDN && negative));
          return {(negative ? f.sign : 0) |
                      (to_infinity ? f.infinity : f.maximum),
                  overflow | inexact};
        }
        const Integer rounded = round_coefficient(
            magnitude,
            std::max(precision_shift, static_cast<unsigned int>(alignment) + 1),
            negative, p.rounding);
        WordValue code = floor_encoding(f, rounded / scale);
        int flags = rounded == magnitude ? 0 : inexact;
        const Integer normal = f.magnitude(f.normal) * scale;
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

      inline Word::WordDomainResult wide_intersect(WordView view,
                                                   Word::WordDomain d,
                                                   WordValue first,
                                                   WordValue last) {
        d.interval.minimum =
            std::max(d.interval.minimum,
                     Word::rank(view.domain_type(), view.width(), first));
        d.interval.maximum =
            std::min(d.interval.maximum,
                     Word::rank(view.domain_type(), view.width(), last));
        if (d.interval.minimum > d.interval.maximum)
          return {false, {{0, 0}, {0, 0}}};
        return Word::compute_domain_closure(view.width(), view.domain_type(),
                                            d);
      }

      struct WideProjection {
        bool present = false;
        Word::WordCube cube{~WordValue(0), 0};
        Word::WordRankInterval interval{~WordValue(0), 0};

        void include(WordView view, const Word::WordDomain& d) {
          const WordValue first =
              Word::rank(view.domain_type(), view.width(), d.interval.minimum);
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
        // Hulls of two projections of one view bound their intersection.
        void intersect(const WideProjection& other) {
          present = present && other.present;
          cube.lo |= other.cube.lo;
          cube.hi &= other.cube.hi;
          interval.minimum = std::max(interval.minimum, other.interval.minimum);
          interval.maximum = std::min(interval.maximum, other.interval.maximum);
        }
        // Bound moves follow the Word significance rule.
        bool publish(Space& home, WordView view) const {
          return present && !me_failed(Word::narrow_significant_domain(
                                home, view, cube, interval));
        }
        // Whether publication to view skipped an endpoint move.
        bool skipped(WordView view) const {
          return view.bounded() && (view.rank_minimum() < interval.minimum ||
                                    view.rank_maximum() > interval.maximum);
        }
      };

      // Publish projections[i] to views[i]. Positions sharing a view publish
      // the intersection of their projections in one tell, so the
      // significance rule judges the interval after both endpoint moves.
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
      inline FiniteParts
      finite_parts(const WideFormat& f, const Policy& p, WordView view,
                   const Word::WordDomain& domain, bool input) {
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
            const WordValue first =
                Word::rank(view.domain_type(), view.width(),
                           admitted.domain.interval.minimum);
            const WordValue last = Word::rank(view.domain_type(), view.width(),
                                              admitted.domain.interval.maximum);
            const bool flushed = input && range.first == 1 &&
                                 p.input_subnormals == Policy::SignedZero;
            Integer low = flushed ? Integer() : f.value(sign ? last : first);
            Integer high = flushed ? Integer() : f.value(sign ? first : last);
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

      inline void include_real(const WideFormat& f, WideProjection& projection,
                               WordView view, const FinitePart& part,
                               Integer low, Integer high) {
        low = std::max(low, part.low);
        high = std::min(high, part.high);
        if (low > high)
          return;
        if (part.flushed) {
          projection.include(view, part.domain);
          return;
        }
        const Integer minimum = part.negative ? -high : low;
        const Integer maximum = part.negative ? -low : high;
        WordValue first = floor_encoding(f, minimum);
        if (f.magnitude(first) < minimum)
          ++first;
        const WordValue last = floor_encoding(f, maximum);
        if (first <= last)
          projection.include(view, part.domain,
                             first | (part.negative ? f.sign : 0),
                             last | (part.negative ? f.sign : 0));
      }

      // Closed bounds on the half-subnormal integer lattice. Strict midpoint
      // and directed endpoints are represented by the adjacent integer unit.
      struct RoundingCell {
        Integer low, high;
      };

      // Product coefficients use the square of the source half-subnormal scale.
      // Form midpoints in that finer lattice before excluding open endpoints.
      inline RoundingCell product_cell(const WideFormat& f, const Policy& p,
                                       WordValue code, const Integer& scale) {
        const WordValue magnitude = code & (f.sign - 1);
        if (magnitude == 0) {
          // Both zero signs can be joined safely. Flushing preimages extend to
          // the normal boundary.
          const Integer radius =
              f.magnitude(p.output_subnormals == Policy::SignedZero ? f.normal
                                                                    : 1) *
              scale;
          return {-radius, radius};
        }
        assert(magnitude < f.maximum);
        const bool negative = (code & f.sign) != 0;
        const Integer center = f.value(code) * scale;
        const Integer previous = (negative ? -f.magnitude(magnitude + 1)
                                           : f.magnitude(magnitude - 1)) *
                                 scale;
        const Integer next = (negative ? -f.magnitude(magnitude - 1)
                                       : f.magnitude(magnitude + 1)) *
                             scale;
        if (p.rounding == Policy::RNE) {
          const Integer odd(magnitude & 1);
          return {(previous + center) / Integer(2) + odd,
                  (center + next) / Integer(2) - odd};
        }
        const bool upward = p.rounding == Policy::RUP ||
                            (p.rounding == Policy::RTZ && negative);
        return upward ? RoundingCell{previous + Integer(1), center}
                      : RoundingCell{center, next - Integer(1)};
      }

      // Partition a result/input alias x into classes with correlated
      // rounding cells. Per sign, tail(domain) receives the admitted
      // zero/flushed range and the range from maximum finite to the sign bit:
      // these can have nonstandard or unbounded preimages. Every other code
      // belongs to one regular(part, first, last) class: a power of two
      // (asymmetric predecessor spacing), or the odd or even codes of a binade
      // (strict RNE ties). Within a class cell(x)-x is constant, so residuals
      // that are monotone in x take their extrema at the endpoint codes.
      template <class Regular, class Tail>
      void visit_alias_classes(const WideFormat& f, const Policy& p,
                               WordView view, const Word::WordDomain& domain,
                               Regular regular, Tail tail) {
        const WordValue first_regular =
            p.input_subnormals == Policy::Preserve &&
                    p.output_subnormals == Policy::Preserve
                ? 1
                : f.normal;
        for (WordValue sign : {WordValue(0), f.sign}) {
          for (const auto& range :
               {std::pair<WordValue, WordValue>(0, first_regular - 1),
                std::pair<WordValue, WordValue>(f.maximum, f.sign - 1)}) {
            const auto admitted = wide_intersect(
                view, domain, sign | range.first, sign | range.second);
            if (admitted.is_consistent)
              tail(admitted.domain);
          }
          for (unsigned int exponent = 0; exponent < 255; ++exponent) {
            const WordValue start = exponent * f.normal;
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
              const WordValue low_code =
                  Word::rank(view.domain_type(), view.width(),
                             candidate.domain.interval.minimum);
              const WordValue high_code =
                  Word::rank(view.domain_type(), view.width(),
                             candidate.domain.interval.maximum);
              const FinitePart part{
                  candidate.domain, f.value(sign ? high_code : low_code),
                  f.value(sign ? low_code : high_code), sign != 0, false};
              regular(part, low_code, high_code);
            }
          }
        }
      }

      inline RoundingCell quotient_bounds(const RoundingCell& product,
                                          const FinitePart& divisor) {
        assert(!divisor.low.zero() && !divisor.high.zero());
        using Word::Arithmetic::Global::ceil_div;
        using Word::Arithmetic::Global::floor_div;
        Integer low = ceil_div(product.low, divisor.low);
        Integer high = floor_div(product.low, divisor.low);
        for (const Integer& numerator : {product.low, product.high})
          for (const Integer& denominator : {divisor.low, divisor.high}) {
            low = std::min(low, ceil_div(numerator, denominator));
            high = std::max(high, floor_div(numerator, denominator));
          }
        return {low, high};
      }

    } // namespace Float
  } // namespace Word
} // namespace Gecode

#endif

// STATISTICS: word-other
