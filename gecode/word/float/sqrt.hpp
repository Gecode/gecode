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
#include <cmath>
#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float.hpp>

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    using Word::WordView;

    using Format = Word::Float::FormatFacts;
    using Word::Float::compare;
    using Word::Float::decode;
    using Word::Float::Dyadic;
    using Word::Float::inexact;
    using Word::Float::invalid;
    using Word::Float::underflow;

    // A decoded significand uses at most 24 bits, an adjacent midpoint 25,
    // and its square 50. No operation here needs a multiword integer.
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
    // Greatest encoding whose value is at most the positive dyadic, 0 below
    // the least subnormal, and at most the maximum. It counts units of the
    // encodings in the binade of the dyadic, or of the subnormals below the
    // least normal; the largest shift left is f.fraction_bits.
    WordValue floor_code(const Format& f, Dyadic value) {
      if (!value.significand)
        return 0;
      const int top = value.exponent + 63 -
                      Support::count_leading_zeros_64(value.significand);
      const int least = 1 - f.bias;
      const int unit = std::max(top, least) - static_cast<int>(f.fraction_bits);
      const int shift = unit - value.exponent;
      const WordValue units = shift >= 64 ? 0
                              : shift >= 0
                                  ? value.significand >> shift
                                  : value.significand << -shift;
      const WordValue code =
          (WordValue(std::max(top, least) - least) << f.fraction_bits) + units;
      return std::min(code, f.maximum);
    }
    // Integer square root of n < 2^52, where the double root is within one.
    WordValue integer_root(WordValue n) {
      WordValue root = static_cast<WordValue>(std::sqrt(static_cast<double>(n)));
      while (root * root > n)
        --root;
      while ((root + 1) * (root + 1) <= n)
        ++root;
      return root;
    }
    // Round the exact root of a positive finite input without computing it:
    // low is the largest encoding whose square is at most the input, and
    // RNE compares the input with the square of the midpoint to low + 1.
    // With an even exponent and a significand of at least 2 * (fraction +
    // 1) bits, the integer root of the significand has at least fraction +
    // 1 bits, so encodings at the root are whole multiples of its unit and
    // the floor of the integer root has the floor encoding of the root.
    WordValue rounded_root(const Format& f, Policy::Rounding rounding,
                           WordValue input) {
      const Dyadic value = decode(f, input);
      Dyadic even = value;
      if (even.exponent & 1) {
        even.significand <<= 1;
        --even.exponent;
      }
      const int bits = 64 - Support::count_leading_zeros_64(even.significand);
      int shift = std::max(0, 2 * static_cast<int>(f.fraction_bits) + 2 - bits);
      shift += shift & 1;
      const WordValue low = floor_code(
          f, {integer_root(even.significand << shift), (even.exponent - shift) / 2});
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
      WordValue magnitude = source & (f.sign() - 1);
      if (f.is_nan(magnitude))
        return {p.nan == Policy::Canonical ? f.canonical_nan()
                                           : source | f.quiet(),
                f.quiet() && !(source & f.quiet()) ? invalid : 0};
      if (p.input_subnormals == Policy::SignedZero && magnitude < f.normal()) {
        source &= f.sign();
        magnitude = 0;
      }
      if (!magnitude)
        return {p.zeros == Policy::Positive ? 0 : source, 0};
      if (source & f.sign())
        return {f.has_nan() ? f.canonical_nan() : f.maximum, invalid};
      if (magnitude == f.infinity)
        return {source, 0};
      WordValue result = rounded_root(f, p.rounding, source);
      int flags =
          compare(square(decode(f, result)), decode(f, source)) ? inexact : 0;
      // A root is tiny when the source lies below the square of the least
      // normal. Every positive finite root is normal in FP32, BF16 and
      // E4M3FN. E2M1 sqrt(0.5) is tiny under both tininess definitions, even
      // when RUP rounds the destination to 1: unbounded two-bit rounding is
      // 0.75. No same-format finite square root can overflow.
      if (compare(decode(f, source), square(decode(f, f.normal()))) < 0)
        flags |= underflow;
      if (p.output_subnormals == Policy::SignedZero && result < f.normal()) {
        result = 0;
        flags |= underflow | inexact;
      }
      return {result, flags};
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
    // First encoding greater than the boundary, or equal when requested,
    // from 1 on. maximum+1 is an empty-range sentinel, never decoded.
    WordValue first_above(const Format& f, Boundary boundary, bool equal) {
      const WordValue code = floor_code(f, boundary.value);
      if (equal && code && compare(decode(f, code), boundary.value) == 0)
        return code;
      return code + 1;
    }

    /// Whether an operand NaN is signaling or quiet
    enum class NanKind { Signaling, Quiet };

    // Temporary case projections deliberately retain interior holes. The actor
    // keeps the tuple relation alive until a concrete source decides its result
    // and status, so a union of projections never becomes a false solution.
    class SqrtProjection {
      const Format& f;
      const Policy& p;
      WordView source, result;
      // The flag values the status admits, one bit per value.
      unsigned int admitted;
      bool admits(int flags) const {
        return (admitted & (1U << flags)) != 0;
      }

    public:
      Projection sources, results;
      unsigned int statuses = 0;
      SqrtProjection(const Format& format, const Policy& policy, WordView x,
                     WordView y, unsigned int flags)
          : f(format), p(policy), source(x), result(y), admitted(flags) {}
      void add_constant(WordValue low, WordValue high, WordValue output,
                        int flags) {
        if (!admits(flags))
          return;
        const auto x = intersect(source, low, high),
                   y = intersect(result, output, output);
        if (x.is_consistent && y.is_consistent) {
          sources.add(x.domain);
          results.add(y.domain);
          statuses |= 1U << flags;
        }
      }
      void add_operand_nan(WordValue low, WordValue high, NanKind kind) {
        const bool signaling = kind == NanKind::Signaling;
        const int flags = signaling ? invalid : 0;
        if (p.nan == Policy::Canonical) {
          add_constant(low, high, f.canonical_nan(), flags);
          return;
        }
        if (!admits(flags))
          return;
        const WordValue quiet = signaling ? f.quiet() : 0;
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
        if (!admits(0) && !admits(inexact))
          return;
        auto x = intersect(
            source, p.input_subnormals == Policy::SignedZero ? f.normal() : 1,
            f.maximum);
        if (!x.is_consistent)
          return;
        // The cubes move the bounds of each closure, which move the bounds
        // of the other side in turn, until neither moves. An aliased source
        // equals its root, so it also lies among the results. Each round
        // narrows x or ends the loop.
        const bool aliased = source == result;
        Word::WordDomainResult y;
        while (true) {
          y = intersect(
              result, rounded_root(f, p.rounding, first_code(source, x.domain)),
              rounded_root(f, p.rounding, last_code(source, x.domain)));
          if (!y.is_consistent)
            return;
          const Boundary low = lower_boundary(f, p.rounding,
                                              first_code(result, y.domain)),
                         high = upper_boundary(f, p.rounding,
                                               last_code(result, y.domain));
          WordValue first = std::max(first_code(source, x.domain),
                                     first_above(f, low, low.included));
          WordValue last = std::min(last_code(source, x.domain),
                                    first_above(f, high, !high.included) - 1);
          Word::WordCube cube{0, source.mask()};
          if (aliased) {
            first = std::max(first, first_code(result, y.domain));
            last = std::min(last, last_code(result, y.domain));
            cube = y.domain.cube;
          }
          const auto next = intersect(source, first, last, cube);
          if (!next.is_consistent)
            return;
          const bool moved =
              next.domain.interval.minimum != x.domain.interval.minimum ||
              next.domain.interval.maximum != x.domain.interval.maximum;
          x = next;
          if (!moved)
            break;
        }
        sources.add(x.domain);
        results.add(y.domain);
        if (admits(0))
          statuses |= 1U;
        if (admits(inexact))
          statuses |= 1U << inexact;
      }
      void collect() {
        add_positive_finite();
        const WordValue zero_end =
            p.input_subnormals == Policy::SignedZero ? f.normal() - 1 : 0;
        add_constant(0, zero_end, 0, 0);
        add_constant(f.sign(), f.sign() + zero_end,
                     p.zeros == Policy::Positive ? 0 : f.sign(), 0);
        add_constant(f.sign() + zero_end + 1, f.sign() + f.infinity,
                     f.canonical_nan(), invalid);
        add_constant(f.infinity, f.infinity, f.infinity, 0);
        for (WordValue sign : {WordValue(0), f.sign()}) {
          add_operand_nan(sign + f.infinity + 1,
                          sign + f.infinity + f.quiet() - 1, NanKind::Signaling);
          add_operand_nan(sign + f.infinity + f.quiet(), sign + f.sign() - 1,
                          NanKind::Quiet);
        }
      }
    };

    // The domains of the source and result views, and the flag values the
    // status admits. Posting narrows a status to 0..31, so the admitted
    // flags are its whole domain.
    struct Snapshot {
      WordValue lo[2], hi[2], minimum[2], maximum[2];
      unsigned int statuses;
      Snapshot(WordView x, WordView y, unsigned int admitted)
          : statuses(admitted) {
        const WordView views[] = {x, y};
        for (int i = 0; i < 2; ++i) {
          lo[i] = views[i].lo();
          hi[i] = views[i].hi();
          minimum[i] = views[i].bounded() ? views[i].rank_minimum() : 0;
          maximum[i] = views[i].bounded() ? views[i].rank_maximum() : 0;
        }
      }
      bool operator==(const Snapshot& other) const {
        for (int i = 0; i < 2; ++i)
          if (lo[i] != other.lo[i] || hi[i] != other.hi[i] ||
              minimum[i] != other.minimum[i] || maximum[i] != other.maximum[i])
            return false;
        return statuses == other.statuses;
      }
    };

    template<class StatusView>
    using SqrtBase =
        MixTernaryPropagator<WordView, Word::PC_WORD_DOM, WordView,
                             Word::PC_WORD_DOM, StatusView, Int::PC_INT_DOM>;
    /** \brief Square root of a wide word
     *
     * StatusView is Int::IntView for a posted status, and
     * Word::Float::OmittedStatus without one: that view admits every flag,
     * subscribes to nothing and ignores updates, so the propagator prunes
     * source and result as with a free status and publishes no status.
     */
    template<class StatusView>
    class Sqrt : public SqrtBase<StatusView> {
      using Base = SqrtBase<StatusView>;
      using Base::x0;
      using Base::x1;
      using Base::x2;
      const Format format;
      const Policy policy;

    public:
      Sqrt(Home home, const Format& f, const Policy& p, WordView source,
           WordView result, StatusView status)
          : Base(home, source, result, status), format(f), policy(p) {}
      Sqrt(Space& home, Sqrt& other)
          : Base(home, other), format(other.format), policy(other.policy) {}
      Actor* copy(Space& home) override { return new (home) Sqrt(home, *this); }
      // Whether the cube of x fixes a bit below its highest free bit, so
      // that it excludes members inside the bounds of x.
      static bool cuts(WordView x) {
        const WordValue free = x.hi() ^ x.lo();
        return (~free & Word::low_through_highest(free)) != 0;
      }
      // Cost by path, from the CPU time of one FP32 propagation: an assigned
      // source about 1 us, a projection without cube holes 1-3 us, and an
      // aliased one 3-4 us. Without aliasing, cube holes can make the root
      // walk past many excluded members (up to about 0.4 ms measured).
      PropCost cost(const Space&, const ModEventDelta&) const override {
        if (x0.assigned())
          return PropCost::unary(PropCost::LO);
        return PropCost::ternary(x0 == x1 || cuts(x0) || cuts(x1)
                                     ? PropCost::HI
                                     : PropCost::LO);
      }
      size_t dispose(Space& home) override {
        (void)Base::dispose(home);
        return sizeof(*this);
      }
      // Narrow every view to the hull of its supports in the current
      // domains, where the status admits the flags in admitted.
      ExecStatus pass(Space& home, unsigned int admitted) {
        SqrtProjection projection(format, policy, x0, x1, admitted);
        projection.collect();
        if (x0 == x1) {
          projection.sources.intersect(projection.results);
          if (!projection.sources.supported)
            return ES_FAILED;
          projection.results = projection.sources;
        }
        GECODE_ES_CHECK(projection.sources.publish(home, x0));
        GECODE_ES_CHECK(projection.results.publish(home, x1));
        GECODE_ME_CHECK(
            Word::Float::narrow_flags(home, x2, projection.statuses));
        return ES_OK;
      }
      // Publication can move bounds to a cube, a cube to the bounds, or
      // remove a status or aliased support, and so expose further pruning.
      // Passes repeat until one changes no domain; every other pass
      // narrows one, so the fixpoint is reached and ES_FIX is honest.
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        unsigned int admitted = Word::Float::admitted_flags(x2);
        while (!x0.assigned()) {
          const Snapshot before(x0, x1, admitted);
          GECODE_ES_CHECK(pass(home, admitted));
          admitted = Word::Float::admitted_flags(x2);
          if (!x0.assigned() && before == Snapshot(x0, x1, admitted))
            return ES_FIX;
        }
        const Evaluation e = evaluate(format, policy, x0.val());
        GECODE_ME_CHECK(Word::Float::narrow_flags(home, x2, 1U << e.flags));
        GECODE_ME_CHECK(x1.eq(home, e.result));
        return home.ES_SUBSUMED(*this);
      }
    };

    /// Square root of a tiny word posted without status: flags projected out
    struct TinySqrtResult {
      TinySqrt sqrt;
      Word::Float::TinyResult operator()(WordValue code) const {
        return {sqrt(code).code, 0};
      }
    };
  } // namespace
} // namespace Gecode

// STATISTICS: word-prop
