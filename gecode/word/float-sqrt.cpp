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
#include <cassert>
#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    using Word::WordView;

    struct Format {
      unsigned int fraction;
      int bias;
      WordValue sign, maximum, infinity, quiet, nan;
      explicit Format(WordDomainType f)
          : fraction(f == WDT_FP32     ? 23
                     : f == WDT_BF16   ? 7
                     : f == WDT_E4M3FN ? 3
                                       : 1),
            bias(f == WDT_E2M1     ? 1
                 : f == WDT_E4M3FN ? 7
                                   : 127),
            sign(WordValue(1) << (float_word_width(f) - 1)),
            maximum(f == WDT_FP32     ? 0x7f7fffff
                    : f == WDT_BF16   ? 0x7f7f
                    : f == WDT_E4M3FN ? 0x7e
                                      : 7),
            infinity(f == WDT_FP32   ? 0x7f800000
                     : f == WDT_BF16 ? 0x7f80
                                     : 0),
            quiet(infinity ? WordValue(1) << (fraction - 1) : 0),
            nan(infinity ? infinity | quiet : maximum + 1) {}
      WordValue normal() const { return WordValue(1) << fraction; }
      bool is_nan(WordValue magnitude) const {
        return infinity ? magnitude > infinity
                        : sign == 128 && magnitude == 127;
      }
    };

    // A decoded significand uses at most 24 bits, an adjacent midpoint 25,
    // and its square 50. No operation here needs a multiword integer.
    struct Dyadic {
      WordValue significand;
      int exponent;
    };
    Dyadic decode(const Format& f, WordValue code) {
      const WordValue exponent = code >> f.fraction;
      const WordValue fraction = code & (f.normal() - 1);
      return {exponent ? f.normal() + fraction : fraction,
              static_cast<int>(exponent ? exponent : 1) - f.bias -
                  static_cast<int>(f.fraction)};
    }
    int compare(Dyadic a, Dyadic b) {
      if (!a.significand || !b.significand)
        return a.significand ? 1 : b.significand ? -1 : 0;
      const int abits = 64 - Support::count_leading_zeros_64(a.significand);
      const int bbits = 64 - Support::count_leading_zeros_64(b.significand);
      const int atop = abits + a.exponent, btop = bbits + b.exponent;
      if (atop != btop)
        return atop < btop ? -1 : 1;
      // Equal leading exponents bound alignment by the significand widths.
      // In particular, neither shift can lose a bit or reach 64.
      if (a.exponent > b.exponent) {
        assert(a.exponent - b.exponent < 64 &&
               abits + a.exponent - b.exponent <= 64);
        a.significand <<= a.exponent - b.exponent;
      } else {
        assert(b.exponent - a.exponent < 64 &&
               bbits + b.exponent - a.exponent <= 64);
        b.significand <<= b.exponent - a.exponent;
      }
      return a.significand < b.significand   ? -1
             : a.significand > b.significand ? 1
                                             : 0;
    }
    Dyadic square(Dyadic x) {
      assert(x.significand < (WordValue(1) << 25));
      return {x.significand * x.significand, 2 * x.exponent};
    }
    Dyadic midpoint(const Format& f, WordValue low, WordValue high) {
      const Dyadic a = decode(f, low), b = decode(f, high);
      const int exponent = std::min(a.exponent, b.exponent);
      assert(a.exponent - exponent <= 1 && b.exponent - exponent <= 1);
      return {(a.significand << (a.exponent - exponent)) +
                  (b.significand << (b.exponent - exponent)),
              exponent - 1};
    }
    // Round the exact root of a positive finite input without computing it:
    // low is the largest encoding whose square is at most the input, and
    // RNE compares the input with the square of the midpoint to low + 1.
    WordValue rounded_root(const Format& f, Policy::Rounding rounding,
                           WordValue input) {
      const Dyadic value = decode(f, input);
      WordValue low = 0, high = f.maximum;
      while (low < high) {
        const WordValue middle = low + (high - low + 1) / 2;
        if (compare(square(decode(f, middle)), value) <= 0)
          low = middle;
        else
          high = middle - 1;
      }
      if (compare(square(decode(f, low)), value) == 0 ||
          rounding == Policy::RTZ || rounding == Policy::RDN)
        return low;
      if (rounding == Policy::RUP)
        return low + 1;
      const int order = compare(value, square(midpoint(f, low, low + 1)));
      return order < 0 || (order == 0 && (low & 1) == 0) ? low : low + 1;
    }

    struct Evaluation {
      WordValue result;
      int flags;
    };
    Evaluation evaluate(const Format& f, const Policy& p, WordValue source) {
      WordValue magnitude = source & (f.sign - 1);
      if (f.is_nan(magnitude))
        return {p.nan == Policy::Canonical ? f.nan : source | f.quiet,
                f.quiet && !(source & f.quiet) ? 1 : 0};
      if (p.input_subnormals == Policy::SignedZero && magnitude < f.normal()) {
        source &= f.sign;
        magnitude = 0;
      }
      if (!magnitude)
        return {p.zeros == Policy::Positive ? 0 : source, 0};
      if (source & f.sign)
        return {f.sign == 8 ? f.maximum : f.nan, 1};
      if (magnitude == f.infinity)
        return {source, 0};
      WordValue result = rounded_root(f, p.rounding, source);
      int flags =
          compare(square(decode(f, result)), decode(f, source)) ? 16 : 0;
      // Every positive finite root is normal in FP32, BF16 and E4M3FN.
      // E2M1 sqrt(0.5) is tiny under both tininess definitions, even when
      // RUP rounds the destination to 1: unbounded two-bit rounding is 0.75.
      // No same-format finite square root can overflow.
      if (f.sign == 8 && source == 1)
        flags |= 8;
      if (p.output_subnormals == Policy::SignedZero && result < f.normal()) {
        result = 0;
        flags |= 24;
      }
      return {result, flags};
    }

    // Return the common format of source and result after checking it and
    // the policy.
    WordDomainType validate(const Policy& p, WordVar source, WordVar result) {
      const WordDomainType format =
          Word::Float::float_format(source, "fp_sqrt format");
      const WordDomainType result_format =
          Word::Float::float_format(result, "fp_sqrt format");
      Word::Float::check(p, "fp_sqrt policy");
      if (result_format != format)
        throw Word::FormatMismatch("fp_sqrt");
      if (format == WDT_BINARY16)
        throw Word::OutOfLimits("fp_sqrt format");
      return format;
    }

    struct TinySqrt {
      Format format;
      Policy policy;
      Word::Float::TinyResult operator()(WordValue code) const {
        const Evaluation e = evaluate(format, policy, code);
        return {static_cast<int>(e.result), e.flags};
      }
    };

    // All ranges passed to this helper have a fixed sign, hence their ranks
    // form one interval.
    Word::WordDomainResult intersect(WordView x, WordValue first,
                                     WordValue last, Word::WordCube cube) {
      const WordDomainType kind = x.domain_type();
      if (first > last)
        return {false, {{0, 0}, {0, 0}}};
      const Word::WordRankInterval ranks =
          Word::rank_interval(kind, x.width(), first, last);
      WordValue minimum = ranks.minimum, maximum = ranks.maximum;
      if (x.bounded()) {
        minimum = std::max(minimum, x.rank_minimum());
        maximum = std::min(maximum, x.rank_maximum());
      }
      if (minimum > maximum)
        return {false, {{0, 0}, {0, 0}}};
      const Word::WordCube common{x.lo() | cube.lo, x.hi() & cube.hi};
      if (common.lo & ~common.hi)
        return {false, {{0, 0}, {0, 0}}};
      return Word::compute_domain_closure(x.width(), kind,
                                          {common, {minimum, maximum}});
    }
    Word::WordDomainResult intersect(WordView x, WordValue first,
                                     WordValue last) {
      return intersect(x, first, last, {0, x.mask()});
    }
    // Least and greatest encoding of a domain within one sign half.
    WordValue first_code(WordView x, const Word::WordDomain& d) {
      return Word::encoding_range(x.domain_type(), x.width(), d.interval).first;
    }
    WordValue last_code(WordView x, const Word::WordDomain& d) {
      return Word::encoding_range(x.domain_type(), x.width(), d.interval).last;
    }
    struct Projection {
      bool supported = false;
      Word::WordDomain domain;
      void add(const Word::WordDomain& d) {
        if (!supported) {
          domain = d;
          supported = true;
          return;
        }
        domain.cube.lo &= d.cube.lo;
        domain.cube.hi |= d.cube.hi;
        domain.interval.minimum =
            std::min(domain.interval.minimum, d.interval.minimum);
        domain.interval.maximum =
            std::max(domain.interval.maximum, d.interval.maximum);
      }
      // Hulls of two projections of one view bound their intersection.
      void intersect(const Projection& other) {
        supported = supported && other.supported;
        if (!supported)
          return;
        domain.cube.lo |= other.domain.cube.lo;
        domain.cube.hi &= other.domain.cube.hi;
        domain.interval.minimum =
            std::max(domain.interval.minimum, other.domain.interval.minimum);
        domain.interval.maximum =
            std::min(domain.interval.maximum, other.domain.interval.maximum);
      }
      // Narrow x to the cube and interval of the supports.
      ExecStatus publish(Space& home, WordView x) const {
        if (!supported)
          return ES_FAILED;
        GECODE_ME_CHECK(Word::narrow_word_domain(home, x, domain.cube,
                                                 domain.interval));
        return ES_OK;
      }
    };

    struct Boundary {
      Dyadic value;
      bool included;
    };
    Boundary lower_boundary(const Format& f, Policy::Rounding rounding,
                            WordValue result) {
      if (rounding == Policy::RNE)
        return {square(midpoint(f, result - 1, result)), (result & 1) == 0};
      if (rounding == Policy::RUP)
        return {square(decode(f, result - 1)), false};
      return {square(decode(f, result)), true};
    }
    Boundary upper_boundary(const Format& f, Policy::Rounding rounding,
                            WordValue result) {
      if (rounding == Policy::RNE)
        return {square(midpoint(f, result, result + 1)), (result & 1) == 0};
      if (rounding == Policy::RUP)
        return {square(decode(f, result)), true};
      return {square(decode(f, result + 1)), false};
    }
    // First encoding greater than the boundary, or equal when requested.
    // maximum+1 is an empty-range sentinel, never decoded.
    WordValue first_above(const Format& f, Boundary boundary, bool equal) {
      WordValue low = 1, high = f.maximum + 1;
      while (low < high) {
        const WordValue middle = low + (high - low) / 2;
        const int order = compare(decode(f, middle), boundary.value);
        if (order > 0 || (equal && order == 0))
          high = middle;
        else
          low = middle + 1;
      }
      return low;
    }

    // Temporary case projections deliberately retain interior holes. The actor
    // keeps the tuple relation alive until a concrete source decides its result
    // and status, so a union of projections never becomes a false solution.
    class SqrtProjection {
      const Format& f;
      const Policy& p;
      WordView source, result;
      Int::IntView status;

    public:
      Projection sources, results;
      unsigned int statuses = 0;
      SqrtProjection(const Format& format, const Policy& policy, WordView x,
                     WordView y, Int::IntView flags)
          : f(format), p(policy), source(x), result(y), status(flags) {}
      void add_constant(WordValue low, WordValue high, WordValue output,
                        int flags) {
        if (!status.in(flags))
          return;
        const auto x = intersect(source, low, high),
                   y = intersect(result, output, output);
        if (x.is_consistent && y.is_consistent) {
          sources.add(x.domain);
          results.add(y.domain);
          statuses |= 1U << flags;
        }
      }
      void add_operand_nan(WordValue low, WordValue high, bool signaling) {
        const int flags = signaling ? 1 : 0;
        if (p.nan == Policy::Canonical) {
          add_constant(low, high, f.nan, flags);
          return;
        }
        if (!status.in(flags))
          return;
        const WordValue quiet = signaling ? f.quiet : 0;
        const auto y = intersect(result, low + quiet, high + quiet);
        if (!y.is_consistent)
          return;
        const auto x =
            intersect(source, first_code(result, y.domain) - quiet,
                      last_code(result, y.domain) - quiet,
                      {y.domain.cube.lo & ~quiet, y.domain.cube.hi & ~quiet});
        if (!x.is_consistent)
          return;
        const auto narrowed =
            intersect(result, first_code(source, x.domain) + quiet,
                      last_code(source, x.domain) + quiet,
                      {x.domain.cube.lo | quiet, x.domain.cube.hi | quiet});
        if (narrowed.is_consistent) {
          sources.add(x.domain);
          results.add(narrowed.domain);
          statuses |= 1U << flags;
        }
      }
      void add_positive_finite() {
        if (!status.in(0) && !status.in(16))
          return;
        auto x = intersect(
            source, p.input_subnormals == Policy::SignedZero ? f.normal() : 1,
            f.maximum);
        if (!x.is_consistent)
          return;
        auto y = intersect(
            result, rounded_root(f, p.rounding, first_code(source, x.domain)),
            rounded_root(f, p.rounding, last_code(source, x.domain)));
        if (!y.is_consistent)
          return;
        const Boundary low = lower_boundary(f, p.rounding,
                                            first_code(result, y.domain)),
                       high = upper_boundary(f, p.rounding,
                                             last_code(result, y.domain));
        x = intersect(source,
                      std::max(first_code(source, x.domain),
                               first_above(f, low, low.included)),
                      std::min(last_code(source, x.domain),
                               first_above(f, high, !high.included) - 1));
        if (!x.is_consistent)
          return;
        sources.add(x.domain);
        results.add(y.domain);
        if (status.in(0))
          statuses |= 1U;
        if (status.in(16))
          statuses |= 1U << 16;
      }
      void collect() {
        add_positive_finite();
        const WordValue zero_end =
            p.input_subnormals == Policy::SignedZero ? f.normal() - 1 : 0;
        add_constant(0, zero_end, 0, 0);
        add_constant(f.sign, f.sign + zero_end,
                     p.zeros == Policy::Positive ? 0 : f.sign, 0);
        add_constant(f.sign + zero_end + 1, f.sign + f.infinity, f.nan, 1);
        add_constant(f.infinity, f.infinity, f.infinity, 0);
        for (WordValue sign : {WordValue(0), f.sign}) {
          add_operand_nan(sign + f.infinity + 1,
                          sign + f.infinity + f.quiet - 1, true);
          add_operand_nan(sign + f.infinity + f.quiet, sign + f.sign - 1,
                          false);
        }
      }
    };

    using SqrtBase =
        MixTernaryPropagator<WordView, Word::PC_WORD_DOM, WordView,
                             Word::PC_WORD_DOM, Int::IntView, Int::PC_INT_DOM>;
    class Sqrt : public SqrtBase {
      const Format format;
      const Policy policy;

    public:
      Sqrt(Home home, const Format& f, const Policy& p, WordView source,
           WordView result, Int::IntView status)
          : SqrtBase(home, source, result, status), format(f), policy(p) {}
      Sqrt(Space& home, Sqrt& other)
          : SqrtBase(home, other), format(other.format), policy(other.policy) {}
      Actor* copy(Space& home) override { return new (home) Sqrt(home, *this); }
      size_t dispose(Space& home) override {
        (void)SqrtBase::dispose(home);
        return sizeof(*this);
      }
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        if (x0.assigned()) {
          const Evaluation e = evaluate(format, policy, x0.val());
          GECODE_ME_CHECK(x2.eq(home, e.flags));
          GECODE_ME_CHECK(x1.eq(home, e.result));
          return home.ES_SUBSUMED(*this);
        }
        SqrtProjection projection(format, policy, x0, x1, x2);
        projection.collect();
        if (x0 == x1) {
          projection.sources.intersect(projection.results);
          if (!projection.sources.supported)
            return ES_FAILED;
          projection.results = projection.sources;
        }
        GECODE_ES_CHECK(projection.sources.publish(home, x0));
        GECODE_ES_CHECK(projection.results.publish(home, x1));
        for (int flag = 0; flag <= 31; ++flag)
          if ((projection.statuses & (1U << flag)) == 0)
            GECODE_ME_CHECK(x2.nq(home, flag));
        // Publication can change aliases, cube endpoints, status cases or
        // skip a bound move.
        return ES_NOFIX;
      }
    };

    void post_sqrt(Home home, const Policy& policy, WordVar source,
                   WordVar result, IntVar status) {
      const Format format(validate(policy, source, result));
      GECODE_POST;
      dom(home, status, 0, 31);
      if (home.failed())
        return;
      if (source.width() <= 8) {
        GECODE_ES_FAIL((Word::Float::TinyUnary<TinySqrt, Int::IntView>::post(
            home, source, result, status, TinySqrt{format, policy})));
        return;
      }
      (void)new (home) Sqrt(home, format, policy, source, result, status);
    }
  } // namespace

  void fp_sqrt(Home home, WordVar source, WordVar result,
               const FloatWordPolicy& policy) {
    post_sqrt(home, policy, source, result, IntVar(home, 0, 31));
  }
  void fp_sqrt(Home home, WordVar source, WordVar result, IntVar status,
               const FloatWordPolicy& policy) {
    post_sqrt(home, policy, source, result, status);
  }
} // namespace Gecode

// STATISTICS: word-post
