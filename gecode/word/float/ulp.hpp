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

#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float.hpp>
#include <gecode/word/arithmetic/global-integer.hpp>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>

namespace Gecode {
  namespace {
    using Word::WordView;
    using Word::WordValueSet;
    using Word::Float::TinyOutcome;
    using Word::Float::TinyResult;
    using Word::Float::TinyValues;
    using Word::Float::failed_change;
    using Word::Float::End;
    using Word::Float::Sign;

    using Format = Word::Float::FormatFacts;

    /// Return the layout of \a format, an already validated float format
    Format layout(WordDomainType format) {
      assert(Word::is_float_domain_type(format));
      return Word::Float::format_facts(format);
    }

    /// Binade floor(log2(v)) of the value v of the magnitude code \a m
    int magnitude_binade(Format source, WordValue m) {
      const WordValue e = m >> source.fraction_bits;
      if (e != 0)
        return static_cast<int>(e) - source.bias;
      if (m == 0)
        return -150; // Below every supported minimum normal exponent.
      int exponent = 1 - source.bias - static_cast<int>(source.fraction_bits);
      while (m >>= 1)
        ++exponent;
      return exponent;
    }

    int tiny_binade(Format source, unsigned int code) {
      return magnitude_binade(source, code & ((1U << (source.width - 1)) - 1));
    }

    // Decode a tiny finite value and encode it exactly in the target lattice.
    bool tiny_encoding(Format source, unsigned int code, Format target,
                       WordValue& encoding) {
      const unsigned int sign = code >> (source.width - 1);
      const unsigned int m = code & ((1U << (source.width - 1)) - 1);
      if (m > source.maximum)
        return false;
      if (m == 0) {
        encoding = WordValue(sign) << (target.width - 1);
        return true;
      }
      unsigned int e = m >> source.fraction_bits;
      WordValue significand = m & ((1U << source.fraction_bits) - 1);
      if (e != 0)
        significand |= WordValue(1) << source.fraction_bits;
      const int exponent = (e == 0 ? 1 : static_cast<int>(e)) - source.bias -
                           static_cast<int>(source.fraction_bits);
      const int binade = tiny_binade(source, code);
      const int target_e = std::max(binade + target.bias, 1);
      const int shift = exponent - (target_e - target.bias -
                                    static_cast<int>(target.fraction_bits));
      WordValue scaled = significand;
      if (shift >= 0) {
        if (shift >= 64)
          return false;
        scaled <<= shift;
      } else {
        if (-shift >= 64 || (scaled & ((WordValue(1) << -shift) - 1)) != 0)
          return false;
        scaled >>= -shift;
      }
      const WordValue hidden = WordValue(1) << target.fraction_bits;
      const WordValue magnitude =
          scaled < hidden
              ? scaled
              : (WordValue(target_e) << target.fraction_bits) + scaled - hidden;
      if (magnitude > target.maximum)
        return false;
      encoding = (WordValue(sign) << (target.width - 1)) | magnitude;
      return true;
    }

    /// Test whether \a code encodes a finite value of the tiny \a source
    bool finite_code(Format source, WordValue code) {
      return (code & ((WordValue(1) << (source.width - 1)) - 1)) <=
             source.maximum;
    }

    /// Spacing exponent of the \a reference lattice at a finite tiny value
    int spacing_exponent(Format source, Format reference, WordValue code) {
      return std::max(tiny_binade(source, static_cast<unsigned int>(code)),
                      1 - reference.bias) -
             static_cast<int>(reference.fraction_bits);
    }

    /** \brief Spacing exponent q of a tiny reference x
     *
     * One pass over the at most 256 remaining encodings keeps the finite
     * encodings whose spacing exponent remains, exactly on a value-set word
     * and as their tightest cube and bounds otherwise, and narrows q
     * exactly to the exponents of the kept encodings.
     */
    class TinySpacing
        : public MixBinaryPropagator<WordView, Word::PC_WORD_DOM,
                                     Int::IntView, Int::PC_INT_DOM> {
      using Base = MixBinaryPropagator<WordView, Word::PC_WORD_DOM,
                                       Int::IntView, Int::PC_INT_DOM>;
      const Format source, reference;

      TinySpacing(Home home, WordView x, Int::IntView q, Format s, Format r)
          : Base(home, x, q), source(s), reference(r) {}
      TinySpacing(Space& home, TinySpacing& other)
          : Base(home, other), source(other.source),
            reference(other.reference) {}

      /// Keep the encodings and exponents that support each other.
      static ExecStatus narrow_to_supports(Space& home, WordView x,
                                           Int::IntView q, Format source,
                                           Format reference) {
        const TinyValues codes(x);
        WordValueSet kept = {{0, 0, 0, 0}};
        int exponents[256];
        int count = 0;
        for (unsigned int i = 0; i < codes.size(); ++i) {
          if (!finite_code(source, codes[i]))
            continue;
          const int exponent = spacing_exponent(source, reference, codes[i]);
          if (!q.in(exponent))
            continue;
          kept = Word::with_value(kept, codes[i]);
          exponents[count++] = exponent;
        }
        if (count == 0)
          return ES_FAILED;
        GECODE_ME_CHECK(x.narrow_values(home, kept));
        std::sort(exponents, exponents + count);
        count = static_cast<int>(std::unique(exponents, exponents + count) -
                                 exponents);
        Iter::Values::Array supported(exponents, count);
        GECODE_ME_CHECK(q.inter_v(home, supported, false));
        return ES_OK;
      }

    public:
      /// Post unless an assigned reference decides the exponent.
      static ExecStatus post(Home home, WordView x, Int::IntView q,
                             Format source, Format reference) {
        if (x.assigned())
          return narrow_to_supports(home, x, q, source, reference);
        (void)new (home) TinySpacing(home, x, q, source, reference);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) TinySpacing(home, *this);
      }
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::binary(PropCost::HI);
      }
      size_t dispose(Space& home) override {
        (void)Base::dispose(home);
        return sizeof(*this);
      }
      // One pass is a fixpoint: the kept encodings K and exponents E support
      // each other, narrow_values never shrinks x below K and q keeps
      // exactly E. A second pass finds K and E again and nothing outside the
      // published domains, so it changes nothing.
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        GECODE_ES_CHECK(narrow_to_supports(home, x0, x1, source, reference));
        return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
    };

    /// FP32 magnitude of a finite tiny encoding; nonfinite codes have none
    struct Fp32Magnitude {
      Format source;
      TinyResult operator()(WordValue code) const {
        const Format fp32 = layout(WDT_FP32);
        WordValue encoded;
        if (!tiny_encoding(source, static_cast<unsigned int>(code), fp32,
                           encoded))
          return {0, 1};
        return {static_cast<int>(encoded & (fp32.sign() - 1)), 0};
      }
    };

    /// Signed rank of a tiny encoding in the \a reference lattice, if any
    std::optional<int64_t> reference_rank(Format source, Format reference,
                                          WordValue code) {
      WordValue encoded;
      if (!tiny_encoding(source, static_cast<unsigned int>(code), reference,
                         encoded))
        return std::nullopt;
      const WordValue sign = WordValue(1) << (reference.width - 1);
      const int64_t magnitude = static_cast<int64_t>(encoded & (sign - 1));
      return (encoded & sign) != 0 ? -magnitude : magnitude;
    }

    /// Least and greatest reference rank of the representable values
    struct RankRange {
      bool is_empty;
      int64_t least, greatest;
    };

    /** \brief fp_ulp_steps between tiny endpoints in a reference lattice
     *
     * The distance of two representable endpoints is the absolute
     * difference of their signed ranks. The default reasoning narrows the
     * distance to the interval that the endpoints' rank ranges span, and
     * keeps each representable endpoint encoding whose rank lies within the
     * distance's unsigned interval of the other endpoint's rank range.
     */
    struct TinySteps {
      Format first, second, reference;

      TinyOutcome operator()(WordValue x, WordValue y) const {
        const std::optional<int64_t> rx = reference_rank(first, reference, x);
        const std::optional<int64_t> ry = reference_rank(second, reference, y);
        if (!rx || !ry)
          return {0, 1};
        return {static_cast<WordValue>(*rx >= *ry ? *rx - *ry : *ry - *rx),
                0};
      }
      RankRange rank_range(Format source, const TinyValues& codes) const {
        RankRange range = {true, 0, 0};
        for (unsigned int i = 0; i < codes.size(); ++i) {
          const std::optional<int64_t> rank =
              reference_rank(source, reference, codes[i]);
          if (!rank)
            continue;
          range.least = range.is_empty ? *rank : std::min(range.least, *rank);
          range.greatest =
              range.is_empty ? *rank : std::max(range.greatest, *rank);
          range.is_empty = false;
        }
        return range;
      }
      /// Keep the representable endpoint encodings near \a other.
      ModEvent narrow_endpoint(Space& home, WordView x, Format source,
                               const TinyValues& codes, RankRange other,
                               int64_t least, int64_t greatest) const {
        WordValueSet kept = {{0, 0, 0, 0}};
        for (unsigned int i = 0; i < codes.size(); ++i) {
          const std::optional<int64_t> rank =
              reference_rank(source, reference, codes[i]);
          if (!rank)
            continue;
          const int64_t r = *rank;
          if ((other.least <= r - least && r - greatest <= other.greatest) ||
              (other.least <= r + greatest && r + least <= other.greatest))
            kept = Word::with_value(kept, codes[i]);
        }
        return x.narrow_values(home, kept);
      }
      ExecStatus narrow_bounds(Space& home, WordView x, WordView y,
                               WordView distance, Int::ConstIntView) const {
        const TinyValues xs(x), ys(y);
        const RankRange rx = rank_range(first, xs), ry = rank_range(second, ys);
        if (rx.is_empty || ry.is_empty)
          return ES_FAILED;
        bool changed = false;
        const int64_t closest =
            std::max({int64_t(0), rx.least - ry.greatest,
                      ry.least - rx.greatest});
        const int64_t farthest =
            std::max(rx.greatest - ry.least, ry.greatest - rx.least);
        if (failed_change(Word::Float::narrow_unsigned_interval(
                              home, distance, static_cast<WordValue>(closest),
                              static_cast<WordValue>(farthest)),
                          changed))
          return ES_FAILED;
        const Word::Float::UnsignedInterval d =
            Word::Float::unsigned_interval(distance);
        const int64_t least = static_cast<int64_t>(d.least),
                      greatest = static_cast<int64_t>(d.greatest);
        if (failed_change(
                narrow_endpoint(home, x, first, xs, ry, least, greatest),
                changed) ||
            failed_change(
                narrow_endpoint(home, y, second, ys, rx, least, greatest),
                changed))
          return ES_FAILED;
        return changed ? ES_NOFIX : ES_FIX;
      }
    };

    /// Numerator digits of fp_ulp_error and its fixed scale S.
    /// With A = a*2^149, B = b*2^149 and the spacing exponent q of the
    /// reference, Q = abs(A-B)*2^(qmax-q) and S = 149+qmax. qmax = 127-(p-1)
    /// is the spacing exponent of the largest finite FP32 reference, so the
    /// shift is never negative and the digits hold every finite input.
    struct ErrorLayout {
      int qmin, qmax;
      int digits, leading_bits;
      int scale;
    };

    /// A finite tiny value as a signed multiple of 2^-9
    int64_t scaled_value(Format source, WordValue code) {
      const WordValue m = code & ((WordValue(1) << (source.width - 1)) - 1);
      const WordValue e = m >> source.fraction_bits;
      int64_t significand =
          static_cast<int64_t>(m & ((WordValue(1) << source.fraction_bits) - 1));
      if (e != 0)
        significand |= int64_t(1) << source.fraction_bits;
      const int shift = (e == 0 ? 1 : static_cast<int>(e)) - source.bias -
                        static_cast<int>(source.fraction_bits) + 9;
      assert(shift >= 0);
      const int64_t value = significand << shift;
      return (code >> (source.width - 1)) != 0 ? -value : value;
    }

    /// Formats and digit layout of a tiny fp_ulp_error
    struct ErrorFormats {
      Format a, b, r, reference;
      ErrorLayout error;
    };

    /// Digits of Q = K * 2^s for one spacing exponent of the reference
    struct ErrorGroup {
      int low;             ///< Position of the digit holding bit s
      unsigned int offset; ///< s modulo 30
      bool others_zero;    ///< Every digit outside low and low-1 admits 0
    };

    /// Digit of K * 2^offset at the low position
    int low_digit(WordValue k, unsigned int offset) {
      return static_cast<int>((k << offset) & ((WordValue(1) << 30) - 1));
    }
    /// Digit of K * 2^offset at the position above the low position
    int high_digit(WordValue k, unsigned int offset) {
      return static_cast<int>(k >> (30 - offset));
    }

    /// Absolute difference K of two scaled tiny values
    WordValue difference(int64_t x, int64_t y) {
      return static_cast<WordValue>(x >= y ? x - y : y - x);
    }

    /// Number of set bits in \a bits
    int ones(uint64_t bits) {
      bits -= (bits >> 1) & 0x5555555555555555ULL;
      bits = (bits & 0x3333333333333333ULL) +
             ((bits >> 2) & 0x3333333333333333ULL);
      bits = (bits + (bits >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
      return static_cast<int>((bits * 0x0101010101010101ULL) >> 56);
    }

    /// Index of the marked difference \a k in increasing order
    int difference_index(const uint64_t* marked, const int* below,
                         WordValue k) {
      return below[k >> 6] +
             ones(marked[k >> 6] & ((uint64_t(1) << (k & 63)) - 1));
    }

    /** \brief Domain-consistent fp_ulp_error on tiny a, b and reference r
     *
     * With tiny values a = Ka*2^-9 and b = Kb*2^-9 and the spacing exponent
     * q of r, the numerator is Q = K * 2^s with K = abs(Ka-Kb) < 2^19 and
     * s = 140+qmax-q, as A = a*2^149 = Ka*2^140. Its nonzero bits lie in
     * the base-2^30 digit holding bit s and the digit above it.
     *
     * Each propagation groups the remaining finite values of r by spacing
     * exponent, marks the distinct K of the remaining pairs in a bitmap
     * indexed by K, decides for every distinct K and group whether the
     * digits admit Q, and enumerates the pairs of a and b again. It keeps
     * exactly the encodings of a, b and r and the digit values of the
     * supported tuples, and the tightest cube and bounds of the supported
     * encodings on a word without a value set. Every alias among a, b, r
     * and among the digits is checked per tuple.
     */
    class TinyError : public Propagator {
      WordView a, b, r;
      ViewArray<Int::IntView> digits;
      const ErrorFormats formats;

      TinyError(Home home, WordView x, WordView y, WordView z,
                ViewArray<Int::IntView>& d, const ErrorFormats& f)
          : Propagator(home), a(x), b(y), r(z), digits(d), formats(f) {
        a.subscribe(home, *this, Word::PC_WORD_DOM);
        b.subscribe(home, *this, Word::PC_WORD_DOM);
        r.subscribe(home, *this, Word::PC_WORD_DOM);
        digits.subscribe(home, *this, Int::PC_INT_DOM);
      }
      TinyError(Space& home, TinyError& other)
          : Propagator(home, other), formats(other.formats) {
        a.update(home, other.a);
        b.update(home, other.b);
        r.update(home, other.r);
        digits.update(home, other.digits);
      }

      /// Test whether digits \a low and \a high at \a group respect aliases
      static bool consistent_aliases(const int* first, const int* size,
                                     const ErrorGroup& group, int low,
                                     int high) {
        const int l = group.low, h = group.low - 1;
        if (first[l] == first[h])
          return size[first[l]] == 2 ? low == high : low == 0 && high == 0;
        return (size[first[l]] == 1 || low == 0) &&
               (size[first[h]] == 1 || high == 0);
      }

      /// Keep the values of the tuples with a supported numerator.
      static ExecStatus narrow_to_supports(Space& home, WordView a, WordView b,
                                           WordView r,
                                           ViewArray<Int::IntView>& digits,
                                           const ErrorFormats& formats) {
        Region region;
        const TinyValues as(a), bs(b), rs(r);
        const int n = digits.size();
        // Group the finite references by spacing exponent.
        int exponents[32];
        ErrorGroup groups[32];
        int group_of[256];
        int group_count = 0;
        for (unsigned int i = 0; i < rs.size(); ++i) {
          group_of[i] = -1;
          if (!finite_code(formats.r, rs[i]))
            continue;
          const int q = spacing_exponent(formats.r, formats.reference, rs[i]);
          int g = 0;
          while (g < group_count && exponents[g] != q)
            ++g;
          if (g == group_count) {
            assert(group_count < 32);
            exponents[group_count++] = q;
          }
          group_of[i] = g;
        }
        if (group_count == 0)
          return ES_FAILED;
        for (int g = 0; g < group_count; ++g) {
          const int shift = 140 + formats.error.qmax - exponents[g];
          groups[g].low = n - 1 - shift / 30;
          groups[g].offset = static_cast<unsigned int>(shift % 30);
          assert(groups[g].low >= 1);
          groups[g].others_zero = true;
          for (int p = 0; p < n; ++p)
            if (p != groups[g].low && p != groups[g].low - 1 &&
                !digits[p].in(0))
              groups[g].others_zero = false;
        }
        // Classes of repeated digit variables: first position and size.
        int* first = region.alloc<int>(n);
        int* size = region.alloc<int>(n);
        for (int p = 0; p < n; ++p) {
          first[p] = p;
          for (int j = 0; j < p; ++j)
            if (digits[j] == digits[p]) {
              first[p] = first[j];
              break;
            }
          size[p] = 0;
        }
        for (int p = 0; p < n; ++p)
          ++size[first[p]];
        // Scaled values of the finite operands.
        const bool same_operands = a == b;
        const bool r_is_a = r == a, r_is_b = r == b;
        int64_t* ka = region.alloc<int64_t>(as.size());
        int64_t* kb = region.alloc<int64_t>(bs.size());
        bool* finite_a = region.alloc<bool>(as.size());
        bool* finite_b = region.alloc<bool>(bs.size());
        for (unsigned int i = 0; i < as.size(); ++i) {
          finite_a[i] = finite_code(formats.a, as[i]);
          ka[i] = finite_a[i] ? scaled_value(formats.a, as[i]) : 0;
        }
        for (unsigned int j = 0; j < bs.size(); ++j) {
          finite_b[j] = finite_code(formats.b, bs[j]);
          kb[j] = finite_b[j] ? scaled_value(formats.b, bs[j]) : 0;
        }
        // Distinct differences K, marked in a bitmap over 0..largest. Every
        // K is below 2^19, so the bitmap has at most 2^13 words; the index
        // of K in increasing order counts the marked differences below it.
        int64_t largest = 0;
        for (unsigned int i = 0; i < as.size(); ++i)
          if (finite_a[i])
            largest = std::max(largest, ka[i] < 0 ? -ka[i] : ka[i]);
        int64_t largest_b = 0;
        for (unsigned int j = 0; j < bs.size(); ++j)
          if (finite_b[j])
            largest_b = std::max(largest_b, kb[j] < 0 ? -kb[j] : kb[j]);
        largest += largest_b;
        assert(largest < (int64_t(1) << 19));
        const int words = static_cast<int>(largest >> 6) + 1;
        uint64_t* marked = region.alloc<uint64_t>(words);
        for (int w = 0; w < words; ++w)
          marked[w] = 0;
        for (unsigned int i = 0; i < as.size(); ++i)
          for (unsigned int j = same_operands ? i : 0;
               j < (same_operands ? i + 1 : bs.size()); ++j)
            if (finite_a[i] && finite_b[j]) {
              const WordValue k = difference(ka[i], kb[j]);
              marked[k >> 6] |= uint64_t(1) << (k & 63);
            }
        int* below = region.alloc<int>(words);
        int k_count = 0;
        for (int w = 0; w < words; ++w) {
          below[w] = k_count;
          k_count += ones(marked[w]);
        }
        WordValue* ks = region.alloc<WordValue>(std::max(1, k_count));
        for (int w = 0, k = 0; w < words; ++w)
          for (uint64_t bits = marked[w]; bits != 0; bits &= bits - 1)
            ks[k++] = (WordValue(w) << 6) +
                      Support::count_trailing_zeros_64(bits);
        uint32_t* feasible = region.alloc<uint32_t>(std::max(1, k_count));
        uint32_t* used = region.alloc<uint32_t>(std::max(1, k_count));
        for (int k = 0; k < k_count; ++k) {
          feasible[k] = 0;
          used[k] = 0;
          for (int g = 0; g < group_count; ++g) {
            const int low = low_digit(ks[k], groups[g].offset);
            const int high = high_digit(ks[k], groups[g].offset);
            if (groups[g].others_zero && digits[groups[g].low].in(low) &&
                digits[groups[g].low - 1].in(high) &&
                consistent_aliases(first, size, groups[g], low, high))
              feasible[k] |= uint32_t(1) << g;
          }
        }
        // Supported pairs.
        Word::Float::WordSupports a_supports(a), b_supports(b),
            r_supports(r);
        uint32_t used_groups = 0;
        for (unsigned int i = 0; i < as.size(); ++i)
          for (unsigned int j = same_operands ? i : 0;
               j < (same_operands ? i + 1 : bs.size()); ++j) {
            if (!finite_a[i] || !finite_b[j])
              continue;
            const int index =
                difference_index(marked, below, difference(ka[i], kb[j]));
            uint32_t mask = feasible[index];
            if (r_is_a)
              mask &= group_of[i] < 0 ? 0 : uint32_t(1) << group_of[i];
            if (r_is_b)
              mask &= group_of[j] < 0 ? 0 : uint32_t(1) << group_of[j];
            if (mask == 0)
              continue;
            a_supports.add(as[i]);
            b_supports.add(bs[j]);
            used[index] |= mask;
            used_groups |= mask;
          }
        if (a_supports.empty())
          return ES_FAILED;
        for (unsigned int i = 0; i < rs.size(); ++i)
          if (group_of[i] >= 0 &&
              (used_groups & (uint32_t(1) << group_of[i])) != 0)
            r_supports.add(rs[i]);
        GECODE_ME_CHECK(a_supports.publish(home, a));
        GECODE_ME_CHECK(b_supports.publish(home, b));
        GECODE_ME_CHECK(r_supports.publish(home, r));
        return narrow_digits(home, digits, groups, group_count, ks, k_count,
                             used, used_groups);
      }

      /// Keep exactly the digit values of the supported numerators.
      static ExecStatus narrow_digits(Space& home,
                                      ViewArray<Int::IntView>& digits,
                                      const ErrorGroup* groups,
                                      int group_count, const WordValue* ks,
                                      int k_count, const uint32_t* used,
                                      uint32_t used_groups) {
        Region region;
        const int n = digits.size();
        int* count = region.alloc<int>(n);
        bool* zero = region.alloc<bool>(n);
        for (int p = 0; p < n; ++p) {
          count[p] = 0;
          zero[p] = false;
        }
        int used_count[32];
        for (int g = 0; g < group_count; ++g) {
          used_count[g] = 0;
          if ((used_groups & (uint32_t(1) << g)) == 0)
            continue;
          for (int k = 0; k < k_count; ++k)
            if ((used[k] & (uint32_t(1) << g)) != 0)
              ++used_count[g];
          count[groups[g].low] += used_count[g];
          count[groups[g].low - 1] += used_count[g];
          for (int p = 0; p < n; ++p)
            if (p != groups[g].low && p != groups[g].low - 1)
              zero[p] = true;
        }
        for (int p = 0; p < n; ++p) {
          int* values = region.alloc<int>(count[p] + 1);
          int size = 0;
          if (zero[p])
            values[size++] = 0;
          for (int g = 0; g < group_count; ++g) {
            const bool low = p == groups[g].low;
            if (used_count[g] == 0 || (!low && p != groups[g].low - 1))
              continue;
            for (int k = 0; k < k_count; ++k)
              if ((used[k] & (uint32_t(1) << g)) != 0)
                values[size++] = low ? low_digit(ks[k], groups[g].offset)
                                     : high_digit(ks[k], groups[g].offset);
          }
          std::sort(values, values + size);
          size = static_cast<int>(std::unique(values, values + size) - values);
          Iter::Values::Array supported(values, size);
          GECODE_ME_CHECK(digits[p].inter_v(home, supported, false));
          region.free<int>(values, count[p] + 1);
        }
        return ES_OK;
      }

    public:
      /// Post unless assigned values decide the digits.
      static ExecStatus post(Home home, WordView a, WordView b, WordView r,
                             ViewArray<Int::IntView>& digits,
                             const ErrorFormats& formats) {
        if (a.assigned() && b.assigned() && r.assigned())
          return narrow_to_supports(home, a, b, r, digits, formats);
        (void)new (home) TinyError(home, a, b, r, digits, formats);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) TinyError(home, *this);
      }
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::quadratic(PropCost::HI, 256);
      }
      void reschedule(Space& home) override {
        a.reschedule(home, *this, Word::PC_WORD_DOM);
        b.reschedule(home, *this, Word::PC_WORD_DOM);
        r.reschedule(home, *this, Word::PC_WORD_DOM);
        digits.reschedule(home, *this, Int::PC_INT_DOM);
      }
      size_t dispose(Space& home) override {
        a.cancel(home, *this, Word::PC_WORD_DOM);
        b.cancel(home, *this, Word::PC_WORD_DOM);
        r.cancel(home, *this, Word::PC_WORD_DOM);
        digits.cancel(home, *this, Int::PC_INT_DOM);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      // One pass is a fixpoint. The supported tuples use only values that
      // remain after publishing: a, b and r keep their supported encodings
      // (WordSupports never shrinks a word below them), every digit keeps
      // the digit values of the used numerators, and the zero digits of a
      // used group stay admitted. A second pass therefore finds every
      // supported tuple again, with the same groups and feasible
      // differences, and no tuple outside the published domains, so it
      // publishes nothing new. Alias checks hold per tuple.
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        GECODE_ES_CHECK(narrow_to_supports(home, a, b, r, digits, formats));
        return (a.assigned() && b.assigned() && r.assigned())
                   ? home.ES_SUBSUMED(*this)
                   : ES_FIX;
      }
    };

    using Integer = Word::Arithmetic::Global::Integer;

    /// Sign bit of the encodings of \a f
    WordValue sign_of(Format f) {
      return WordValue(1) << (f.width - 1);
    }

    /// Value of the finite magnitude code \a m of \a f in units of 2^-149
    Integer lattice_value(Format f, WordValue m) {
      const WordValue hidden = WordValue(1) << f.fraction_bits;
      const WordValue e = m >> f.fraction_bits;
      const WordValue significand = e == 0 ? m : (m & (hidden - 1)) | hidden;
      // At least 0: FP32 subnormals are multiples of 2^-149.
      const int shift = (e == 0 ? 1 : static_cast<int>(e)) - f.bias -
                        static_cast<int>(f.fraction_bits) + 149;
      return Integer(significand).shift_left(static_cast<unsigned int>(shift));
    }

    /// Spacing exponent of \a reference at the magnitude code \a m of \a f
    int magnitude_spacing(Format f, Format reference, WordValue m) {
      return std::max(magnitude_binade(f, m), 1 - reference.bias) -
             static_cast<int>(reference.fraction_bits);
    }

    /// Exponent of the spacing of \a f at the magnitude code \a m, in units
    /// of 2^-149
    int lattice_spacing(Format f, WordValue m) {
      return magnitude_spacing(f, f, m) + 149;
    }

    /// Least finite magnitude code of \a f that satisfies the monotone
    /// \a holds, or f.maximum+1 if none does
    template<class Predicate>
    WordValue first_magnitude(Format f, Predicate holds) {
      WordValue low = 0, high = f.maximum + 1;
      while (low < high) {
        const WordValue middle = low + (high - low) / 2;
        if (holds(middle))
          high = middle;
        else
          low = middle + 1;
      }
      return low;
    }

    /// Greatest finite magnitude code whose value is at most \a v >= 0
    WordValue floor_magnitude(Format f, const Integer& v) {
      // The subnormals and the least normal binade are multiples of
      // 2^low, and a normal v with its leading bit at bits-1 lies in the
      // binade with exponent field bits-150+bias, as multiples of
      // 2^(bits-1-fraction).
      const unsigned int low = static_cast<unsigned int>(1 - f.bias - static_cast<int>(f.fraction_bits) + 149);
      const unsigned int bits = v.bits();
      if (bits <= f.fraction_bits + low)
        return v.shift_right(low).word();
      const WordValue e = static_cast<WordValue>(static_cast<int>(bits) - 150 + f.bias);
      if (e > (f.maximum >> f.fraction_bits))
        return f.maximum;
      const WordValue fraction =
          v.shift_right(bits - 1 - f.fraction_bits).word() & ((WordValue(1) << f.fraction_bits) - 1);
      return std::min((e << f.fraction_bits) | fraction, f.maximum);
    }
    /// Least finite magnitude code whose value is at least \a v, or
    /// f.maximum+1 if there is none
    WordValue ceil_magnitude(Format f, const Integer& v) {
      if (!(v > Integer()))
        return 0;
      const WordValue m = floor_magnitude(f, v);
      return lattice_value(f, m) == v ? m : m + 1;
    }

    /// An interval of finite magnitude codes of one sign, if present
    struct MagnitudeRange {
      bool present;
      WordValue least, greatest;
      /// Intersect with \a low..high
      void restrict(WordValue low, WordValue high) {
        least = std::max(least, low);
        greatest = std::min(greatest, high);
        present = present && least <= greatest;
      }
    };

    /// Keep the magnitudes of \a side, of the sign \a sign, whose values
    /// lie in \a low..high, in units of 2^-149
    void restrict_side_values(Format f, MagnitudeRange& side, Sign sign, const Integer& low, const Integer& high) {
      const bool negative = sign == Sign::Negative;
      const Integer zero;
      if (negative ? low > zero : high < zero)
        side.present = false;
      else if (side.present && negative)
        side.restrict(ceil_magnitude(f, std::max(-high, zero)), floor_magnitude(f, -low));
      else if (side.present)
        side.restrict(ceil_magnitude(f, std::max(low, zero)), floor_magnitude(f, high));
    }

    /** \brief Finite magnitude codes of each sign of a float word
     *
     * A tiny word reads its exact values. A wide word combines its cube
     * with its interval in the IEEE total order, in which positive
     * encodings rank from the sign bit up and a negative encoding with
     * magnitude m has rank sign-1-m.
     */
    struct SignedRanges {
      MagnitudeRange positive, negative;
      /// Least nonzero finite magnitude code of the word as read, or 0
      WordValue nonzero;
      /// Magnitude bits that are set, and that are clear, in every value
      WordValue ones, zeros;

      SignedRanges(WordView x, Format f)
          : positive{false, 0, 0}, negative{false, 0, 0}, nonzero(0) {
        const WordValue sign = sign_of(f), mask = sign - 1;
        if (f.width <= 8) {
          const TinyValues codes(x);
          ones = mask;
          zeros = mask;
          for (unsigned int i = 0; i < codes.size(); ++i) {
            const WordValue m = codes[i] & mask;
            if (m > f.maximum)
              continue;
            MagnitudeRange& side =
                (codes[i] & sign) != 0 ? negative : positive;
            side.least = side.present ? std::min(side.least, m) : m;
            side.greatest = side.present ? std::max(side.greatest, m) : m;
            side.present = true;
            if (m != 0 && (nonzero == 0 || m < nonzero))
              nonzero = m;
            ones &= m;
            zeros &= ~m;
          }
          return;
        }
        ones = x.lo() & mask;
        zeros = ~x.hi() & mask;
        const WordValue least = x.lo() & mask;
        const WordValue greatest = std::min(x.hi() & mask, f.maximum);
        const WordValue minimum = x.bounded() ? x.rank_minimum() : 0;
        const WordValue maximum =
            x.bounded() ? x.rank_maximum() : Word::width_mask(f.width);
        if ((x.lo() & sign) == 0 && maximum >= sign) {
          positive = {true, least, greatest};
          positive.restrict(minimum >= sign ? minimum - sign : 0,
                            maximum - sign);
        }
        if ((x.hi() & sign) != 0 && minimum < sign) {
          negative = {true, least, greatest};
          negative.restrict(maximum < sign ? mask - maximum : 0,
                            mask - minimum);
        }
        // Code 1 bounds a nonzero magnitude in a range starting at zero.
        for (const MagnitudeRange* side : {&positive, &negative})
          if (side->present && side->greatest != 0) {
            const WordValue m = std::max<WordValue>(side->least, 1);
            if (nonzero == 0 || m < nonzero)
              nonzero = m;
          }
      }
      bool empty(void) const {
        return !positive.present && !negative.present;
      }
      /// Least magnitude code of a nonempty range
      WordValue least(void) const {
        return !negative.present   ? positive.least
               : !positive.present ? negative.least
                                   : std::min(positive.least, negative.least);
      }
      /// Greatest magnitude code of a nonempty range
      WordValue greatest(void) const {
        return !negative.present ? positive.greatest
               : !positive.present
                   ? negative.greatest
                   : std::max(positive.greatest, negative.greatest);
      }
      /// Intersect both signs with the magnitude codes \a low..high
      void restrict(WordValue low, WordValue high) {
        positive.restrict(low, high);
        negative.restrict(low, high);
      }
      /// Whether both signs have the magnitude codes of \a other
      bool same(const SignedRanges& other) const {
        return positive.present == other.positive.present && negative.present == other.negative.present &&
               (!positive.present ||
                (positive.least == other.positive.least && positive.greatest == other.positive.greatest)) &&
               (!negative.present ||
                (negative.least == other.negative.least && negative.greatest == other.negative.greatest));
      }
      /// Keep the magnitude codes of each sign that \a other also has
      void intersect(const SignedRanges& other) {
        positive.restrict(other.positive.least, other.positive.greatest);
        positive.present = positive.present && other.positive.present;
        negative.restrict(other.negative.least, other.negative.greatest);
        negative.present = negative.present && other.negative.present;
      }
      /// Keep the values in \a low..high, in units of 2^-149
      void restrict_values(Format f, const Integer& low, const Integer& high) {
        restrict_side_values(f, positive, Sign::Positive, low, high);
        restrict_side_values(f, negative, Sign::Negative, low, high);
      }
      /** \brief Narrow \a x to these values, or on a wide word to the
       * tightest cube and rank interval that hold them
       *
       * A wide word holds a cube and an interval in the IEEE total order.
       * The members of the cube within one sign's magnitudes m_lo..m_hi,
       * from the least to the greatest member, share the bits above the
       * highest bit d in which m_lo and m_hi differ, and no other bit
       * outside the cube: setting the free bits below d to one under the
       * common prefix with bit d clear gives a member at most m_hi with
       * each such bit set, and clearing them under bit d set one with each
       * such bit clear. So a bit is fixed in every value of the word
       * exactly when it is fixed in both signs to the same value, and the
       * sign bit when one sign remains. This keeps what a sign's raised
       * least magnitude implies when the rank interval, which runs from
       * the greatest negative to the greatest positive magnitude, cannot
       * show it. A sign without members of the cube goes.
       */
      ModEvent publish(Space& home, WordView x, Format f) const {
        if (empty())
          return Word::ME_WORD_FAILED;
        const WordValue sign = sign_of(f), mask = sign - 1;
        if (f.width <= 8) {
          const TinyValues codes(x);
          WordValueSet kept = {{0, 0, 0, 0}};
          bool is_empty = true;
          for (unsigned int i = 0; i < codes.size(); ++i) {
            const WordValue m = codes[i] & mask;
            const MagnitudeRange& side =
                (codes[i] & sign) != 0 ? negative : positive;
            if (side.present && side.least <= m && m <= side.greatest) {
              kept = Word::with_value(kept, codes[i]);
              is_empty = false;
            }
          }
          return is_empty ? Word::ME_WORD_FAILED : x.narrow_values(home, kept);
        }
        MagnitudeRange held[2] = {positive, negative};
        WordValue known_ones = mask, known_zeros = mask;
        for (MagnitudeRange& side : held) {
          if (side.present && !(Word::cube_successor(ones, ~zeros & mask, side.least, mask, side.least) &&
                                side.least <= side.greatest &&
                                Word::cube_predecessor(ones, ~zeros & mask, side.greatest, mask, side.greatest)))
            side.present = false;
          if (!side.present)
            continue;
          const WordValue prefix = mask & ~Word::low_through_highest(side.least ^ side.greatest);
          known_ones &= ones | (side.least & prefix);
          known_zeros &= zeros | (~side.least & prefix);
        }
        const MagnitudeRange& p = held[0];
        const MagnitudeRange& n = held[1];
        if (!p.present && !n.present)
          return Word::ME_WORD_FAILED;
        if (!n.present)
          known_zeros |= sign;
        if (!p.present)
          known_ones |= sign;
        const ModEvent bits = x.narrow(home, known_ones, ~known_zeros & (sign | mask));
        if (me_failed(bits))
          return bits;
        const WordValue minimum = n.present ? mask - n.greatest : sign | p.least;
        const WordValue maximum = p.present ? sign | p.greatest : mask - n.least;
        const ModEvent range = x.narrow_rank_range(home, minimum, maximum);
        return me_failed(range) || me_modified(range) ? range : bits;
      }
    };

    /// Closed interval of values in units of 2^-149
    struct ValueInterval {
      Integer least, greatest;
    };

    /// The value intervals of the signs of \a s; returns their number
    int value_intervals(Format f, const SignedRanges& s, ValueInterval* out) {
      int n = 0;
      if (s.positive.present)
        out[n++] = {lattice_value(f, s.positive.least),
                    lattice_value(f, s.positive.greatest)};
      if (s.negative.present)
        out[n++] = {-lattice_value(f, s.negative.greatest),
                    -lattice_value(f, s.negative.least)};
      return n;
    }

    /// Least and greatest value of a nonempty \a s
    ValueInterval value_hull(Format f, const SignedRanges& s) {
      ValueInterval parts[2];
      const int n = value_intervals(f, s, parts);
      return n == 1 ? parts[0] : ValueInterval{parts[1].least,
                                               parts[0].greatest};
    }

    /// Least and greatest abs(x-y) of x in \a sx and y in \a sy
    ValueInterval distances(Format fx, const SignedRanges& sx, Format fy,
                            const SignedRanges& sy) {
      ValueInterval xs[2], ys[2];
      const int nx = value_intervals(fx, sx, xs),
                ny = value_intervals(fy, sy, ys);
      ValueInterval d;
      for (int i = 0; i < nx; ++i)
        for (int j = 0; j < ny; ++j) {
          const Integer farthest = std::max(xs[i].greatest - ys[j].least,
                                            ys[j].greatest - xs[i].least);
          const Integer closest =
              std::max({Integer(), xs[i].least - ys[j].greatest,
                        ys[j].least - xs[i].greatest});
          const bool first = i == 0 && j == 0;
          d.least = first ? closest : std::min(d.least, closest);
          d.greatest = first ? farthest : std::max(d.greatest, farthest);
        }
      return d;
    }

    /// Greatest k with x*2^k <= bound, for 0 < x <= bound
    unsigned int widest_shift(const Integer& x, const Integer& bound) {
      unsigned int k = bound.bits() - x.bits();
      if (x.shift_left(k) > bound)
        --k;
      return k;
    }
    /// Least k with x*2^k >= bound, for x > 0
    unsigned int narrowest_shift(const Integer& x, const Integer& bound) {
      if (x >= bound)
        return 0;
      unsigned int k = bound.bits() - x.bits();
      if (x.shift_left(k) < bound)
        ++k;
      return k;
    }

    /// Least multiple of 2^\a k that is at least \a x >= 0
    Integer round_up(const Integer& x, unsigned int k) {
      return (x + Integer::power_two(k) - Integer(1)).shift_right(k).shift_left(k);
    }

    /** \brief Narrow the digits and the numerator interval to each other
     *
     * The numerator is the sum of digit i times 2^(30*(n-1-i)) and a
     * multiple of 2^\a zeros. Each round tightens \a least and \a greatest
     * to the sums of the digit bounds and to multiples of 2^\a zeros, and
     * then bounds every digit by the interval minus what the other digits
     * can add, rounded to the multiples of 2^(zeros-30*(n-1-i)) that its
     * position allows, until no digit bound moves. Repeated digit variables
     * are read as separate digits, which is sound.
     */
    ExecStatus narrow_digits(Space& home, ViewArray<Int::IntView>& digits,
                             Integer& least, Integer& greatest,
                             unsigned int zeros) {
      const int n = digits.size();
      for (bool moved = true; moved;) {
        moved = false;
        Integer low, high;
        for (int i = 0; i < n; ++i) {
          const unsigned int weight = 30 * static_cast<unsigned int>(n - 1 - i);
          low = low + Integer(static_cast<uint64_t>(digits[i].min()))
                          .shift_left(weight);
          high = high + Integer(static_cast<uint64_t>(digits[i].max()))
                            .shift_left(weight);
        }
        least = round_up(std::max(least, low), zeros);
        greatest = std::min(greatest, high).shift_right(zeros).shift_left(zeros);
        if (least > greatest)
          return ES_FAILED;
        for (int i = 0; i < n; ++i) {
          const unsigned int weight = 30 * static_cast<unsigned int>(n - 1 - i);
          const int64_t minimum = digits[i].min(), maximum = digits[i].max();
          int64_t lower = minimum, upper = maximum;
          // The other digits add at most high-maximum*2^weight.
          const Integer missing =
              least - (high - Integer(static_cast<uint64_t>(maximum))
                                  .shift_left(weight));
          if (missing > Integer()) {
            const Integer bound =
                (missing + Integer::power_two(weight) - Integer(1))
                    .shift_right(weight);
            if (bound.bits() < 32)
              lower = std::max(lower, static_cast<int64_t>(bound.word()));
            else
              return ES_FAILED;
          }
          // The other digits add at least low-minimum*2^weight.
          const Integer bound =
              (greatest - (low - Integer(static_cast<uint64_t>(minimum))
                                     .shift_left(weight)))
                  .shift_right(weight);
          if (bound.bits() < 32)
            upper = std::min(upper, static_cast<int64_t>(bound.word()));
          if (weight < zeros) {
            // Only multiples of 2^step remain at this position.
            const unsigned int step = std::min(zeros - weight, 31U);
            const int64_t unit = int64_t(1) << step;
            lower = (lower + unit - 1) / unit * unit;
            upper = upper / unit * unit;
          }
          if (lower > minimum) {
            GECODE_ME_CHECK(digits[i].gq(home, static_cast<long long int>(lower)));
            moved = true;
          }
          if (upper < maximum) {
            GECODE_ME_CHECK(digits[i].lq(home, static_cast<long long int>(upper)));
            moved = true;
          }
        }
      }
      return ES_OK;
    }

    /** \brief Exponent of a spacing that divides every value of \a s
     * whose magnitude code is at least \a from > 0, in units of 2^-149,
     * and the greatest exponent when there is none
     *
     * A finite value of \a f with magnitude code m is +-M*2^g(m) for
     * g(m) = lattice_spacing(f, m), with the significand M = m below the
     * least normal binade and M = 2^fraction + (m modulo 2^fraction) in it
     * and above. g(m) grows with m, also across binades, and the
     * subnormals have the g of the least normal binade, so g(m) >= g(m0)
     * for the least code m0 >= \a from of the cube of \a s, which every
     * such m is at least. When the z lowest bits of every code are clear,
     * for z <= fraction, the z lowest bits of M are those of m, so M is a
     * multiple of 2^z and the value one of 2^(g(m0)+z). The sign changes
     * no magnitude.
     */
    unsigned int spacing_from(Format f, const SignedRanges& s, WordValue from) {
      const WordValue mask = sign_of(f) - 1;
      WordValue m0;
      if (!Word::cube_successor(s.ones, ~s.zeros & mask, from, mask, m0) || m0 > f.maximum)
        return std::numeric_limits<unsigned int>::max();
      const unsigned int z = std::min(Support::count_trailing_zeros_64(~s.zeros), f.fraction_bits);
      return static_cast<unsigned int>(lattice_spacing(f, m0)) + z;
    }

    /// Exponent of a spacing that divides every value of \a s, in units of
    /// 2^-149, and the greatest exponent when \a s can only be zero
    unsigned int value_spacing(Format f, const SignedRanges& s) {
      // Every nonzero value has a magnitude of at least s.nonzero (see
      // spacing_from()), and zero is a multiple of every spacing.
      return s.nonzero == 0 ? std::numeric_limits<unsigned int>::max() : spacing_from(f, s, s.nonzero);
    }

    /// Exponent of a spacing that divides abs(x-y) for all x in \a sa and
    /// y in \a sb, in units of 2^-149
    int finest_spacing(const ErrorFormats& f, const SignedRanges& sa,
                       const SignedRanges& sb) {
      const unsigned int g =
          std::min(value_spacing(f.a, sa), value_spacing(f.b, sb));
      return g == std::numeric_limits<unsigned int>::max()
                 ? 0
                 : static_cast<int>(g);
    }

    /// \a v modulo 2^\a k, in 0..2^k-1
    Integer residue(const Integer& v, unsigned int k) {
      const Integer low = v.abs().low_bits(k);
      return v.neg() && !low.zero() ? Integer::power_two(k) - low : low;
    }

    /// Least value of at least \a x in the cyclic interval \a lo..hi of
    /// 0..mask, which wraps around when lo > hi, or -1 if there is none
    int field_at_least(int x, int lo, int hi, int mask) {
      if (lo <= hi)
        return x <= lo ? lo : x <= hi ? x : -1;
      if (x <= hi || x >= lo)
        return x <= mask ? x : -1;
      return lo;
    }

    /** \brief Whether a value in \a low..high has its low fields in bounds
     *
     * The values lie below 2^\a bits, and bits is at most 30 times the
     * number of digits. Their fields are the last m = floor(bits/30) digits
     * and, when c = bits-30*m is positive, the c low bits of the digit
     * above, which admit the residues modulo 2^c of the values within the
     * bounds of that digit. Finds the least value of at least \a low with
     * every field in bounds, as the lexicographic successor of the fields
     * of \a low, and compares it with \a high.
     */
    bool low_fields_meet(const ViewArray<Int::IntView>& digits,
                         unsigned int bits, const Integer& low,
                         const Integer& high) {
      const int m = static_cast<int>(bits / 30);
      const int c = static_cast<int>(bits) - 30 * m;
      const int count = m + (c > 0 ? 1 : 0);
      const int first = digits.size() - count;
      Region region;
      int* lo = region.alloc<int>(count);
      int* hi = region.alloc<int>(count);
      int* mask = region.alloc<int>(count);
      int* x = region.alloc<int>(count);
      int* y = region.alloc<int>(count);
      for (int j = 0; j < count; ++j) {
        lo[j] = digits[first + j].min();
        hi[j] = digits[first + j].max();
        mask[j] = (1 << 30) - 1;
        if (j == 0 && c > 0) {
          mask[j] = (1 << c) - 1;
          if (hi[j] - lo[j] >= mask[j]) {
            lo[j] = 0;
            hi[j] = mask[j];
          } else {
            lo[j] &= mask[j];
            hi[j] &= mask[j];
          }
        }
        const unsigned int weight =
            30 * static_cast<unsigned int>(count - 1 - j);
        x[j] = static_cast<int>(low.shift_right(weight).low_bits(30).word());
        y[j] = static_cast<int>(high.shift_right(weight).low_bits(30).word());
      }
      int j = 0;
      while (j < count && field_at_least(x[j], lo[j], hi[j], mask[j]) == x[j])
        ++j;
      if (j < count) {
        int next = field_at_least(x[j], lo[j], hi[j], mask[j]);
        while (next < 0) {
          // Increment an earlier field instead.
          if (--j < 0)
            return false;
          next = field_at_least(x[j] + 1, lo[j], hi[j], mask[j]);
        }
        x[j] = next;
        for (int k = j + 1; k < count; ++k)
          x[k] = lo[k] <= hi[k] ? lo[k] : 0;
      }
      for (int k = 0; k < count; ++k)
        if (x[k] != y[k])
          return x[k] < y[k];
      return true;
    }

    /// Whether the residue modulo 2^\a t of some value in \a low..high,
    /// times 2^\a s, has its low fields below 2^(s+t) in bounds
    bool residues_meet(const ViewArray<Int::IntView>& digits, unsigned int s,
                       unsigned int t, const Integer& low,
                       const Integer& high) {
      const Integer modulus = Integer::power_two(t);
      const Integer top = (modulus - Integer(1)).shift_left(s);
      if (high - low >= modulus - Integer(1))
        return low_fields_meet(digits, s + t, Integer(), top);
      const Integer first = residue(low, t);
      const Integer last = first + (high - low);
      if (last < modulus)
        return low_fields_meet(digits, s + t, first.shift_left(s),
                               last.shift_left(s));
      return low_fields_meet(digits, s + t, first.shift_left(s), top) ||
             low_fields_meet(digits, s + t, Integer(),
                             (last - modulus).shift_left(s));
    }

    /// Spacing exponent of the coarsest spacing group of \a s
    int coarsest_spacing(Format f, const SignedRanges& s) {
      int g = 0;
      for (const MagnitudeRange* side : {&s.positive, &s.negative})
        if (side->present)
          g = std::max(g, lattice_spacing(f, side->greatest));
      return g;
    }

    /** \brief Number of low bits of Q whose digits admit every multiple
     * of 2^\a lattice
     *
     * Counts whole digits from the last one: a digit admits every multiple
     * of 2^lattice within its bits when its bounds include 0 and the
     * greatest such multiple. The c low bits of the first digit that does
     * not take every pattern when its bounds span at least 2^c values, as
     * low_fields_meet() reads them.
     */
    unsigned int free_low_bits(const ViewArray<Int::IntView>& digits,
                               unsigned int lattice) {
      unsigned int free_bits = 0;
      for (int i = digits.size(); i-- > 0; free_bits += 30) {
        const unsigned int step = lattice - std::min(lattice, free_bits);
        const int greatest = step >= 30 ? 0 : (1 << 30) - (1 << step);
        if (digits[i].min() != 0 || digits[i].max() < greatest) {
          const int span = digits[i].max() - digits[i].min();
          unsigned int c = 0;
          while (c < 30 && (1 << (c + 1)) - 1 <= span)
            ++c;
          return free_bits + c;
        }
      }
      return free_bits;
    }

    /** \brief Greatest k such that the digits admit a numerator that is a
     * multiple of 2^k
     *
     * Scans from the last digit. A digit whose bounds include 0 admits
     * every further multiple; otherwise k ends at the greatest c such
     * that its bounds contain a multiple of 2^c. The result is the width
     * of the digits when they admit 0.
     */
    unsigned int zero_low_bits(const ViewArray<Int::IntView>& digits) {
      unsigned int k = 0;
      for (int i = digits.size(); i-- > 0; k += 30) {
        const int low = digits[i].min(), high = digits[i].max();
        if (low != 0) {
          unsigned int c = 0;
          while (c < 29 && ((low + (1 << (c + 1)) - 1) >> (c + 1) << (c + 1)) <= high)
            ++c;
          return k + c;
        }
      }
      return k;
    }

    /// floor(\a x / 2^\a k)
    Integer floor_shift(const Integer& x, unsigned int k) {
      if (!x.neg())
        return x.shift_right(k);
      const Integer q = x.abs().shift_right(k);
      return x.abs().low_bits_zero(k) ? -q : -(q + Integer(1));
    }

    /// ceil(\a x / 2^\a k)
    Integer ceil_shift(const Integer& x, unsigned int k) {
      return -floor_shift(-x, k);
    }

    /** \brief The codes of one spacing group of a word
     *
     * The values are sign*M*2^\a g, in units of 2^-149, for the significands
     * M in \a least..greatest. Below the fraction width the bits of M are
     * the code bits, so every M has the bits of \a ones set and the bits of
     * \a zeros clear that the cube of the word fixes there.
     */
    struct GroupCodes {
      bool negative;
      WordValue least, greatest;
      unsigned int g;
      WordValue ones, zeros;
    };

    /// Significand M of the finite magnitude code \a m of \a f, with value
    /// M times the spacing of the group of \a m
    WordValue significand(Format f, WordValue m) {
      const WordValue hidden = WordValue(1) << f.fraction_bits;
      return (m >> f.fraction_bits) <= 1 ? m : (m & (hidden - 1)) | hidden;
    }

    /// Whether some M of \a c has M modulo 2^\a u in \a low..high, where
    /// low <= high < 2^u
    bool residues_match(const GroupCodes& c, unsigned int u, WordValue low,
                        WordValue high) {
      const WordValue all = ~WordValue(0), unit = WordValue(1) << u;
      const WordValue first = c.least >> u, last = c.greatest >> u;
      WordValue m;
      // The units of 2^u that hold the least and the greatest M are cut by
      // the range: search them for a matching M directly.
      for (WordValue k : {first, last}) {
        const WordValue from = std::max(c.least, k * unit + low);
        const WordValue to = std::min(c.greatest, k * unit + high);
        if (from <= to && Word::cube_successor(c.ones, ~c.zeros, from, all, m) && m <= to)
          return true;
      }
      if (last <= first + 1)
        return false;
      // In the units between, the bits of M below u and the unit number
      // match independently.
      WordValue l, k;
      return Word::cube_successor(c.ones & (unit - 1), ~c.zeros & (unit - 1), low, unit - 1, l) && l <= high &&
             Word::cube_successor(c.ones >> u, ~c.zeros >> u, first + 1, all, k) && k < last;
    }

    /// The side of a value v of the tested word that a value y of the other
    /// word lies on
    enum class Side {
      Below, ///< y <= v
      Above  ///< y >= v
    };

    /// The sides on which a value of the other word can lie for some value
    /// of a group
    enum class Sides { None, Below, Above, Both };

    /// The sides of which \a below and \a above say whether they admit one
    Sides sides(bool below, bool above) {
      if (below)
        return above ? Sides::Both : Sides::Below;
      return above ? Sides::Above : Sides::None;
    }

    /// Whether some M of \a c has M modulo 2^\a u congruent to an integer
    /// in \a first..last, where u <= 30
    bool residues_in(const GroupCodes& c, unsigned int u, const Integer& first, const Integer& last) {
      if (first > last)
        return false;
      const WordValue unit = WordValue(1) << u;
      if (last - first >= Integer(unit - 1)) {
        WordValue m;
        return Word::cube_successor(c.ones, ~c.zeros, c.least, ~WordValue(0), m) && m <= c.greatest;
      }
      const WordValue l = residue(first, u).word();
      const WordValue count = (last - first).word() + 1;
      if (l + count <= unit)
        return residues_match(c, u, l, l + count - 1);
      return residues_match(c, u, l, unit - 1) || residues_match(c, u, 0, l + count - 1 - unit);
    }

    /** \brief Whether the digits from the one holding bit g+s down admit Q
     * for some M of \a c and y in \a y_low..y_high on \a side, where y is
     * a multiple of 2^\a h
     *
     * Let t = g+s, digit p hold bit t, and u be the number of its bits at
     * or above t, so that R = Q modulo 2^(t+u) is the value of digits p to
     * n-1. With Y = y*2^s and sigma the sign of the group, Q is
     * sigma*M*2^t-Y below and Y-sigma*M*2^t above, so L*2^t for
     * L = M modulo 2^u is congruent modulo 2^(t+u) to sigma*(R+Y) or
     * sigma*(Y-R). R lies between the sums of the lower and of the upper
     * bounds of digits p to n-1, and Y between Y_low and Y_high, so L is
     * congruent to an integer of a range that these bound. Tests whether
     * some M of \a c has its residue in that range.
     *
     * When h > g, Y is moreover a multiple of 2^(t+d) for d = h-g. Then R
     * is a multiple of 2^t, and Y vanishes modulo 2^(t+k) for
     * k = min(d, u): L is congruent modulo 2^k to sigma*R/2^t below and to
     * -sigma*R/2^t above, whatever y is. Also tests whether some M of \a c
     * has its residue modulo 2^k in that range of R/2^t. This matters when
     * y is far larger than the values of the group, so that the range of
     * Y spans every residue modulo 2^(t+u).
     *
     * The range is the same for every unit of 2^u significands in the
     * group, and the propagator lowers digit p-1 by one unit, together
     * with the greatest M, exactly when the greatest M of a unit misses it.
     * Rejecting the whole group at once replaces stepping through it one
     * unit per pass.
     */
    bool units_meet(const ViewArray<Int::IntView>& digits, const GroupCodes& c,
                    unsigned int s, unsigned int h, const Integer& y_low, const Integer& y_high,
                    Side side) {
      const int n = digits.size();
      const unsigned int t = c.g + s;
      if (t >= 30 * static_cast<unsigned int>(n))
        return true;
      const int p = n - 1 - static_cast<int>(t / 30);
      const unsigned int u = 30 * (t / 30 + 1) - t;
      Integer r_low, r_high;
      for (int i = p; i < n; ++i) {
        r_low = r_low.shift_left(30) + Integer(static_cast<uint64_t>(digits[i].min()));
        r_high = r_high.shift_left(30) + Integer(static_cast<uint64_t>(digits[i].max()));
      }
      if (h > c.g) {
        const Integer rho_low = ceil_shift(r_low, t), rho_high = floor_shift(r_high, t);
        const bool plus = (side == Side::Below) != c.negative;
        if (!residues_in(c, std::min(h - c.g, u), plus ? rho_low : -rho_high, plus ? rho_high : -rho_low))
          return false;
      }
      const Integer big_y_low = y_low.shift_left(s), big_y_high = y_high.shift_left(s);
      Integer low = side == Side::Below ? r_low + big_y_low : big_y_low - r_high;
      Integer high = side == Side::Below ? r_high + big_y_high : big_y_high - r_low;
      if (c.negative) {
        const Integer swap = low;
        low = -high;
        high = -swap;
      }
      return residues_in(c, u, ceil_shift(low, t), floor_shift(high, t));
    }

    /** \brief The values of \a so, whose hull is \a hull, in \a low..high,
     * in units of 2^-149; returns whether one remains
     *
     * Sets \a window to their hull and \a h to the exponent of a spacing
     * that divides them all, from their least nonzero magnitude and the
     * clear low bits of the cube (see spacing_from()), or the greatest
     * exponent when only zero remains.
     */
    bool value_window(Format fo, const SignedRanges& so, const ValueInterval& hull, const Integer& low,
                      const Integer& high, ValueInterval& window, unsigned int& h) {
      if (high < hull.least || low > hull.greatest)
        return false;
      SignedRanges w = so;
      if (low > hull.least || high < hull.greatest) {
        w.restrict_values(fo, low, high);
        if (w.empty())
          return false;
        window = value_hull(fo, w);
      } else {
        window = hull;
      }
      WordValue m = 0;
      for (const MagnitudeRange* side : {&w.positive, &w.negative})
        if (side->present && side->greatest != 0) {
          const WordValue least = std::max<WordValue>(side->least, 1);
          m = m == 0 ? least : std::min(m, least);
        }
      // The ranges may have narrowed since so.nonzero was read.
      h = m == 0 ? std::numeric_limits<unsigned int>::max() : spacing_from(fo, so, std::max(m, so.nonzero));
      return true;
    }

    /** \brief Whether the digits admit Q = abs(v-y)*2^s for some v of
     * \a c and y in \a y_low..y_high on \a side, all multiples of 2^h
     *
     * abs(v-y) is a multiple of 2^e for e = min(g, h), and it is congruent
     * to -y modulo 2^g when y <= v and to y when y >= v. For
     * k = min(g+s, 30*digits), Q modulo 2^k is 0 when k <= e+s, and
     * otherwise (j modulo 2^(k-e-s))*2^(e+s) for j = -y/2^e or y/2^e, which
     * depends on y alone. Tests whether such a value has its fields below
     * 2^k in bounds (see low_fields_meet()) and its fields below 2^(e+s)
     * admitting zero, and whether the digit holding bit g+s meets the
     * residues of the significands of \a c (see units_meet()). Every such
     * Q is a multiple of 2^lattice for the lattice of free_low_bits(), so
     * it meets the lowest \a free_bits bits.
     */
    bool side_admits(const ViewArray<Int::IntView>& digits, unsigned int free_bits, const GroupCodes& c,
                     unsigned int s, unsigned int h, const Integer& y_low, const Integer& y_high, Side side) {
      const unsigned int g = c.g, e = std::min(g, h);
      const unsigned int width = 30 * static_cast<unsigned int>(digits.size());
      const unsigned int bits = std::min(g + s, width);
      if (bits <= free_bits) {
        // The fields below 2^bits admit every such Q.
      } else if (bits <= e + s) {
        // Q is a multiple of 2^bits.
        if (!low_fields_meet(digits, bits, Integer(), Integer()))
          return false;
      } else {
        // Q is a multiple of 2^(e+s): its lowest e+s bits must admit zero.
        if (e + s > free_bits && !low_fields_meet(digits, e + s, Integer(), Integer()))
          return false;
        // The multiples j*2^e of y as a range of -j below and of j above.
        const Integer j_low = side == Side::Below ? -floor_shift(y_high, e) : ceil_shift(y_low, e);
        const Integer j_high = side == Side::Below ? -ceil_shift(y_low, e) : floor_shift(y_high, e);
        if (j_low > j_high || !residues_meet(digits, e + s, bits - e - s, j_low, j_high))
          return false;
      }
      return units_meet(digits, c, s, h, y_low, y_high, side);
    }

    /** \brief The sides on which the digits admit some abs(v-y) for v of
     * \a c and y of the other word \a so of format \a fo and hull \a other
     *
     * With D = Q/2^s in Dmin..Dmax for Q in \a least..\a greatest and s in
     * \a s_low..s_high, the values y that pair with v in v_low..v_high
     * lie in v_low-Dmax..v_high-Dmin below v and in v_low+Dmin..v_high+Dmax
     * above it. Each side tests the values of \a so in its window, with
     * their own spacing (see value_window()), for some s in
     * \a s_low..s_high (see side_admits()). The windows keep a group that
     * pairs only with values near one end of the other word from being
     * tested against the finer spacing or the farther end of the whole
     * word, which let a and b narrow each other by one code per pass.
     */
    Sides low_digits_admit(const ViewArray<Int::IntView>& digits, unsigned int free_bits, const GroupCodes& c,
                           Format fo, const SignedRanges& so, const ValueInterval& other, const Integer& least,
                           const Integer& greatest, unsigned int s_low, unsigned int s_high) {
      const unsigned int g = c.g;
      Integer v_low = Integer(c.least).shift_left(g), v_high = Integer(c.greatest).shift_left(g);
      if (c.negative) {
        const Integer swap = v_low;
        v_low = -v_high;
        v_high = -swap;
      }
      const Integer d_low = ceil_shift(least, s_high), d_high = floor_shift(greatest, s_low);
      if (d_low > d_high)
        return Sides::None;
      ValueInterval below_y, above_y;
      unsigned int below_h = 0, above_h = 0;
      const bool below = value_window(fo, so, other, v_low - d_high, v_high - d_low, below_y, below_h);
      const bool above = value_window(fo, so, other, v_low + d_low, v_high + d_high, above_y, above_h);
      bool below_met = false, above_met = false;
      for (unsigned int s = s_low; s <= s_high && ((below && !below_met) || (above && !above_met)); ++s) {
        below_met = below_met ||
                    (below && side_admits(digits, free_bits, c, s, below_h, below_y.least, below_y.greatest,
                                          Side::Below));
        above_met = above_met ||
                    (above && side_admits(digits, free_bits, c, s, above_h, above_y.least, above_y.greatest,
                                          Side::Above));
      }
      return sides(below_met, above_met);
    }

    /** \brief Narrow the shifts \a s_low..s_high to those with which the
     * magnitudes \a first..\a last of one sign of \a f admit a numerator
     * in \a least..\a greatest; returns whether one remains
     *
     * Every Q = abs(v-y)*2^s for v of the group and y in \a other has
     * abs(v-y) between the least and greatest distance D_lo and D_hi of
     * the group to that hull. So Q >= least needs D_hi*2^s >= least, and
     * Q <= greatest needs D_lo*2^s <= greatest.
     */
    bool group_shifts(Format f, bool negative, WordValue first, WordValue last, const ValueInterval& other,
                      const Integer& least, const Integer& greatest, unsigned int& s_low, unsigned int& s_high) {
      Integer v_low = lattice_value(f, first), v_high = lattice_value(f, last);
      if (negative) {
        const Integer swap = v_low;
        v_low = -v_high;
        v_high = -swap;
      }
      const Integer d_high = std::max(v_high - other.least, other.greatest - v_low);
      const Integer d_low = std::max({Integer(), other.least - v_high, v_low - other.greatest});
      if (!least.zero()) {
        if (d_high.zero())
          return false;
        s_low = std::max(s_low, narrowest_shift(d_high, least));
      }
      if (!d_low.zero()) {
        if (d_low > greatest)
          return false;
        s_high = std::min(s_high, widest_shift(d_low, greatest));
      }
      return s_low <= s_high;
    }

    /** \brief Narrow the magnitudes \a first..\a last of one sign of \a f
     * to those whose values are within reach of \a other; returns whether
     * one remains
     *
     * A value v pairs with a y of \a other at a distance D = Q/2^s for Q in
     * \a least..\a greatest and s in \a s_low..s_high, so D lies in
     * Dmin..Dmax for Dmin = ceil(least/2^s_high) and
     * Dmax = floor(greatest/2^s_low). With y <= v, v lies in
     * other.least+Dmin..other.greatest+Dmax, and with y >= v in
     * other.least-Dmax..other.greatest-Dmin. The magnitudes keep the hull
     * of the values of the group in these two windows. On the binades
     * that keep the same relative distance to the other word, such as the
     * binades of a reference that is also a or b, the windows cut each
     * group at the same significand, below the residues that the digits
     * admit, so that the group fails only once it is tested on the values
     * within reach. Before that, a pass cut one group and the next pass
     * removed it, one binade per pass.
     */
    bool within_reach(Format f, bool negative, const ValueInterval& other, const Integer& least,
                      const Integer& greatest, unsigned int s_low, unsigned int s_high, WordValue& first,
                      WordValue& last) {
      const Integer d_low = ceil_shift(least, s_high), d_high = floor_shift(greatest, s_low);
      if (d_low > d_high)
        return false;
      Integer v_low = lattice_value(f, first), v_high = lattice_value(f, last);
      if (negative) {
        const Integer swap = v_low;
        v_low = -v_high;
        v_high = -swap;
      }
      // The values with y <= v, then those with y >= v.
      const ValueInterval windows[2] = {
          {std::max(v_low, other.least + d_low), std::min(v_high, other.greatest + d_high)},
          {std::max(v_low, other.least - d_high), std::min(v_high, other.greatest - d_low)}};
      bool any = false;
      ValueInterval hull;
      for (const ValueInterval& w : windows)
        if (!(w.least > w.greatest)) {
          hull = any ? ValueInterval{std::min(hull.least, w.least), std::max(hull.greatest, w.greatest)} : w;
          any = true;
        }
      if (!any)
        return false;
      MagnitudeRange side = {true, first, last};
      restrict_side_values(f, side, negative ? Sign::Negative : Sign::Positive, hull.least, hull.greatest);
      first = side.least;
      last = side.greatest;
      return side.present;
    }

    /** \brief The integers below 2^\a width with known bits, within
     * \a least..\a greatest
     *
     * A member has the bits of \a ones set and the bits of \a zeros clear;
     * both lie below 2^width.
     */
    struct KnownBits {
      WordValue least, greatest, ones, zeros;
      unsigned int width;
    };

    /// The bits below 2^\a width
    WordValue low_mask(unsigned int width) {
      return width >= 64 ? ~WordValue(0) : (WordValue(1) << width) - 1;
    }

    /// Least member \a m of \a s that is at least \a v; returns whether
    /// there is one
    bool member_at_least(const KnownBits& s, WordValue v, WordValue& m) {
      const WordValue mask = low_mask(s.width);
      return v <= s.greatest && Word::cube_successor(s.ones, ~s.zeros & mask, std::max(v, s.least), mask, m) &&
             m <= s.greatest;
    }
    /// Greatest member \a m of \a s that is at most \a v; returns whether
    /// there is one
    bool member_at_most(const KnownBits& s, WordValue v, WordValue& m) {
      const WordValue mask = low_mask(s.width);
      return v >= s.least && Word::cube_predecessor(s.ones, ~s.zeros & mask, std::min(v, s.greatest), mask, m) &&
             m >= s.least;
    }
    /// What trim_groups() tests the groups of a word against
    struct TrimBounds {
      /// The digits of Q
      const ViewArray<Int::IntView>& digits;
      /// Whether to test the low digits, and the bits that admit everything
      bool low_digits;
      unsigned int free_bits;
      /// The shifts qmax-q of the remaining references
      unsigned int s_low, s_high;
      /// The interval of Q
      Integer least, greatest;
      /// The reference format and qmax, when the word is also the reference
      const Format* reference;
      int qmax;
    };

    /** \brief Narrow the interval of Q and the shifts in \a ta and \a tb
     * to the words \a sa and \a sb
     *
     * Q = D*2^(qmax-q) for the distances D between \a sa and \a sb, and
     * for the spacing exponents q of \a sr, the reference when it is a or
     * b (otherwise \a sr is null and q keeps its range).
     */
    void narrow_trim_bounds(const ErrorFormats& f, const SignedRanges* sr, const SignedRanges& sa,
                            const SignedRanges& sb, TrimBounds& ta, TrimBounds& tb) {
      if (sr != nullptr) {
        const int q_low = magnitude_spacing(f.r, f.reference, sr->least());
        const int q_high = magnitude_spacing(f.r, f.reference, sr->greatest());
        ta.s_low = std::max(ta.s_low, static_cast<unsigned int>(ta.qmax - q_high));
        ta.s_high = std::min(ta.s_high, static_cast<unsigned int>(ta.qmax - q_low));
        tb.s_low = ta.s_low;
        tb.s_high = ta.s_high;
      }
      const ValueInterval d = distances(f.a, sa, f.b, sb);
      ta.least = std::max(ta.least, d.least.shift_left(ta.s_low));
      ta.greatest = std::min(ta.greatest, d.greatest.shift_left(ta.s_high));
      tb.least = ta.least;
      tb.greatest = ta.greatest;
    }

    /// The ends of the magnitudes of one sign of a word that a group is at
    enum class GroupEnds {
      Top,    ///< The greatest magnitudes
      Bottom, ///< The least magnitudes
      Both    ///< The group holds every magnitude of the sign
    };

    /** \brief Remove the values of the group \a first..\a last at an end
     * of \a side whose side of \a other admits no value
     *
     * A value above every value of the other word needs Sides::Below, and
     * a value below every one needs Sides::Above. When \a met lacks one,
     * the values of the group that need it go, at the end of the
     * magnitudes of the group that is also an end of \a side, as \a ends
     * gives. The values of the other groups were not tested, so they stay.
     */
    void keep_met_side(Format f, MagnitudeRange& side, bool negative, GroupEnds ends, WordValue first,
                       WordValue last, Sides met, const ValueInterval& other) {
      const Integer zero;
      const bool top = ends != GroupEnds::Bottom, bottom = ends != GroupEnds::Top;
      // Values v <= other.greatest when Sides::Above is all that is met,
      // and v >= other.least when Sides::Below is.
      if (met == Sides::Above) {
        if (!negative && top)
          side.restrict(side.least,
                        other.greatest < zero ? first : std::max(first, floor_magnitude(f, other.greatest)));
        if (negative && bottom && other.greatest < zero)
          side.restrict(std::min(last, ceil_magnitude(f, -other.greatest)), side.greatest);
      } else if (met == Sides::Below) {
        if (negative && top)
          side.restrict(side.least, other.least > zero ? first : std::max(first, floor_magnitude(f, -other.least)));
        if (!negative && bottom && other.least > zero)
          side.restrict(std::min(last, ceil_magnitude(f, other.least)), side.greatest);
      }
    }

    /** \brief Remove spacing groups at the ends of \a s that the digits
     * rule out
     *
     * A spacing group is a binade, with the subnormals joined to the least
     * normal binade, so all its values are multiples of one spacing. At
     * both ends of the magnitudes of each sign, removes groups until one
     * passes: a group fails when its distances to the value hull of the
     * other word \a so admit no shift (see group_shifts()), or when
     * low_digits_admit() fails against \a so. A group is tested on its
     * members of the cube within reach of \a so (see within_reach()); its
     * members beyond that reach go, and so does a group without members
     * within reach. When the word is also the reference, a
     * group admits only the shifts of its own magnitudes. A group that
     * passes on one side of the hull only loses its values on the other
     * side, at the end (see keep_met_side()). Removing a whole group, or
     * the part of it beyond the hull, at once is what keeps the propagator
     * from stepping through a group one code or one unit of a digit per
     * pass when the digits exclude it.
     */
    void trim_groups(Format f, SignedRanges& s, Format fo, const SignedRanges& so, const TrimBounds& t) {
      const ValueInterval other = value_hull(fo, so);
      const WordValue fraction = (WordValue(1) << f.fraction_bits) - 1;
      for (MagnitudeRange* side : {&s.positive, &s.negative}) {
        const bool negative = side == &s.negative;
        bool top = true;
        while (side->present) {
          const WordValue e_least = side->least >> f.fraction_bits;
          const WordValue e_greatest = side->greatest >> f.fraction_bits;
          // The subnormals share the spacing of the least normal binade.
          const bool single = std::max<WordValue>(e_least, 1) ==
                              std::max<WordValue>(e_greatest, 1);
          if (!top && single)
            break;
          WordValue group_first, group_last;
          if (top) {
            group_first = e_greatest <= 1 ? side->least : std::max(e_greatest << f.fraction_bits, side->least);
            group_last = side->greatest;
          } else {
            group_first = side->least;
            group_last = std::min(((std::max<WordValue>(e_least, 1) + 1) << f.fraction_bits) - 1, side->greatest);
          }
          // The group is tested on the magnitudes from its least to its
          // greatest member of the cube. An end that the cube excludes,
          // left by removing the neighbouring group, could let the group
          // pass on a value the word does not hold, and the group would
          // then go one pass later, one group per pass.
          const KnownBits held = {group_first, group_last, s.ones, s.zeros, f.width - 1};
          WordValue first, last;
          if (!member_at_least(held, group_first, first) || !member_at_most(held, group_last, last)) {
            if (top ? group_first == side->least : group_last == side->greatest)
              side->present = false;
            else if (top)
              side->restrict(side->least, group_first - 1);
            else
              side->restrict(group_last + 1, side->greatest);
            continue;
          }
          unsigned int group_low = t.s_low, group_high = t.s_high;
          if (t.reference != nullptr) {
            group_low = std::max(group_low,
                                 static_cast<unsigned int>(t.qmax - magnitude_spacing(f, *t.reference, last)));
            group_high = std::min(group_high,
                                  static_cast<unsigned int>(t.qmax - magnitude_spacing(f, *t.reference, first)));
          }
          // The group is tested on its members within reach of the other
          // word (see within_reach()); the members beyond go with it.
          Sides met = Sides::None;
          WordValue reach_first = first, reach_last = last;
          if (group_low <= group_high &&
              group_shifts(f, negative, first, last, other, t.least, t.greatest, group_low, group_high) &&
              within_reach(f, negative, other, t.least, t.greatest, group_low, group_high, reach_first,
                           reach_last) &&
              member_at_least(held, reach_first, reach_first) && member_at_most(held, reach_last, reach_last) &&
              reach_first <= reach_last) {
            const GroupCodes codes = {negative,
                                      significand(f, reach_first),
                                      significand(f, reach_last),
                                      static_cast<unsigned int>(lattice_spacing(f, first)),
                                      s.ones & fraction,
                                      s.zeros & fraction};
            met = t.low_digits ? low_digits_admit(t.digits, t.free_bits, codes, fo, so, other, t.least,
                                                  t.greatest, group_low, group_high)
                               : Sides::Both;
          }
          if (met != Sides::None) {
            const GroupEnds ends = single ? GroupEnds::Both : top ? GroupEnds::Top : GroupEnds::Bottom;
            keep_met_side(f, *side, negative, ends, first, last, met, other);
            if (top || single)
              side->restrict(side->least, reach_last);
            if (!top || single)
              side->restrict(reach_first, side->greatest);
            if (!top || single)
              break;
            top = false;
          } else if (top ? group_first == side->least : group_last == side->greatest) {
            side->present = false;
          } else if (top) {
            side->restrict(side->least, group_first - 1);
          } else {
            side->restrict(group_last + 1, side->greatest);
          }
        }
      }
    }

    /// Last magnitude code of the binade of \a m in \a f
    WordValue binade_last(Format f, WordValue m) {
      if ((m >> f.fraction_bits) != 0 || m == 0)
        return m == 0 ? 0 : (((m >> f.fraction_bits) + 1) << f.fraction_bits) - 1;
      WordValue top = 1;
      while ((top << 1) <= m)
        top <<= 1;
      return 2 * top - 1;
    }
    /// First magnitude code of the binade of \a m in \a f
    WordValue binade_first(Format f, WordValue m) {
      if ((m >> f.fraction_bits) != 0 || m == 0)
        return m == 0 ? 0 : (m >> f.fraction_bits) << f.fraction_bits;
      WordValue top = 1;
      while ((top << 1) <= m)
        top <<= 1;
      return top;
    }

    /** \brief Whether the reference magnitudes \a first..\a last of one
     * binade and one sign admit a numerator in \a least..\a greatest
     *
     * Here the reference is also one of a and b, and \a sy holds the
     * other. The binade fixes q, so Q = D*2^(qmax-q) with D between the
     * least and greatest distance to \a sy. The lowest set bit that the
     * digits leave Q needs no test here: trim_groups() tests it on each
     * binade of a spacing group of its own with the q of that binade (see
     * side_admits()), and the bound on q from zero_low_bits() covers the
     * binades that share the spacing group of the subnormals.
     */
    bool reference_binade_admits(const ErrorFormats& f, Format fx, bool negative, WordValue first,
                                 WordValue last, Format fy, const SignedRanges& sy, const Integer& least,
                                 const Integer& greatest) {
      const int shift = f.error.qmax - magnitude_spacing(fx, f.reference, first);
      SignedRanges sx = sy;
      sx.positive = {!negative, first, last};
      sx.negative = {negative, first, last};
      const ValueInterval d = distances(fx, sx, fy, sy);
      const unsigned int k = static_cast<unsigned int>(shift);
      return d.greatest.shift_left(k) >= least && d.least.shift_left(k) <= greatest;
    }

    /** \brief Remove the binades at the ends of the reference magnitudes
     * \a x that admit no numerator in \a least..\a greatest
     *
     * For a reference that is also one of a and b, with \a sy the other
     * word; see reference_binade_admits(). Each pass tests at most the
     * binades it removes plus two per sign, which is O(#binades).
     */
    void trim_reference_binades(const ErrorFormats& f, Format fx, SignedRanges& x, Format fy,
                                const SignedRanges& sy, const Integer& least, const Integer& greatest) {
      for (MagnitudeRange* side : {&x.positive, &x.negative}) {
        const bool negative = side == &x.negative;
        while (side->present) {
          const WordValue last = std::min(binade_last(fx, side->least), side->greatest);
          if (reference_binade_admits(f, fx, negative, side->least, last, fy, sy, least, greatest))
            break;
          if (last == side->greatest)
            side->present = false;
          else
            side->restrict(last + 1, side->greatest);
        }
        while (side->present) {
          const WordValue first = std::max(binade_first(fx, side->greatest), side->least);
          if (reference_binade_admits(f, fx, negative, first, side->greatest, fy, sy, least, greatest))
            break;
          if (first == side->least)
            side->present = false;
          else
            side->restrict(side->least, first - 1);
        }
      }
    }

    /// The integers 2^width-1-v for the members v of \a s
    KnownBits complement(const KnownBits& s) {
      const WordValue mask = low_mask(s.width);
      return {mask - s.greatest, mask - s.least, s.zeros, s.ones, s.width};
    }
    /// The integers v*2^\a k for the members v of \a s
    KnownBits shifted(const KnownBits& s, unsigned int k) {
      return {s.least << k, s.greatest << k, s.ones << k, (s.zeros << k) | low_mask(k), s.width + k};
    }

    /** \brief Greatest x of a set X for which some c of a set C has x+c in
     * s1..s2
     *
     * X and C hold integers below 2^n with known bits and bounds (see
     * KnownBits), and s1 <= s2 <= 2^(n+1)-2. The bits of x and c are chosen
     * from bit n down to bit 0. After the bits from p up, the sum is
     * P*2^p plus at most 2^(p+1)-2 from the bits below, for the sum P of
     * the chosen prefixes. A state holds whether the prefixes of x and c
     * still equal those of their least and greatest bounds, and the
     * differences l = P-floor(s1/2^p) and h = P-floor(s2/2^p): with
     * l <= -2 or h >= 1 the sum misses s1..s2 whatever follows, and with
     * l >= 1 and h <= -2 it meets that side whatever follows, so l takes
     * the values -1, 0 and at least 1, h at most -2, -1 and 0, which makes
     * 144 states. At bit 0 a state meets the interval exactly when l >= 0.
     * Whether a state can be completed by the bits below p is memoized per
     * bit and state. Choosing the bits of x from the top, 1 before 0, and
     * keeping the set of states that some bits of c reach, gives the
     * greatest x in O(n*144) steps.
     */
    class GreatestSum {
      static const int states = 144;
      const KnownBits& x;
      const KnownBits& c;
      const WordValue s1, s2;
      const int n;
      /// Per number of remaining bits and state: -1 unknown, 0 no, 1 yes
      signed char memo[64][states];

      static int encode(int xl, int xg, int cl, int cg, int l, int h) {
        return ((((xl * 2 + xg) * 2 + cl) * 2 + cg) * 3 + (l + 1)) * 3 + (h + 2);
      }
      static int bit(WordValue v, int p) {
        return static_cast<int>((v >> p) & 1);
      }
      /// Whether bit \a p of a member of \a s can be \a b
      static bool allowed(const KnownBits& s, int p, int b) {
        if (p >= static_cast<int>(s.width))
          return b == 0;
        if (bit(s.ones, p) != 0)
          return b == 1;
        return bit(s.zeros, p) == 0 || b == 0;
      }
      /// Narrow the tightness \a tight to a prefix of \a bound, the end
      /// \a end of the range, by bit \a b
      static bool tighten(int& tight, WordValue bound, int p, int b, End end) {
        if (tight == 0)
          return true;
        const int d = bit(bound, p);
        if (end == End::Low ? b < d : b > d)
          return false;
        tight = b == d ? 1 : 0;
        return true;
      }
      /// The state after bits \a xb of x and \a cb of c at bit \a p, or -1
      int next(int state, int p, int xb, int cb) const {
        int h = state % 3 - 2;
        int l = (state / 3) % 3 - 1;
        int cg = (state / 9) % 2, cl = (state / 18) % 2, xg = (state / 36) % 2, xl = state / 72;
        if (!tighten(xl, x.least, p, xb, End::Low) || !tighten(xg, x.greatest, p, xb, End::High) ||
            !tighten(cl, c.least, p, cb, End::Low) || !tighten(cg, c.greatest, p, cb, End::High))
          return -1;
        l = 2 * l + xb + cb - bit(s1, p);
        h = 2 * h + xb + cb - bit(s2, p);
        if (l <= -2 || h >= 1)
          return -1;
        return encode(xl, xg, cl, cg, std::min(l, 1), std::max(h, -2));
      }
      /// Whether the bits below \a p complete \a state
      bool feasible(int p, int state) {
        if (p == 0)
          return (state / 3) % 3 >= 1;
        signed char& known = memo[p][state];
        if (known < 0) {
          known = 0;
          for (int xb = 0; xb < 2 && known == 0; ++xb)
            for (int cb = 0; cb < 2 && known == 0; ++cb)
              if (allowed(x, p - 1, xb) && allowed(c, p - 1, cb)) {
                const int after = next(state, p - 1, xb, cb);
                if (after >= 0 && feasible(p - 1, after))
                  known = 1;
              }
        }
        return known == 1;
      }

    public:
      GreatestSum(const KnownBits& x0, const KnownBits& c0, WordValue s10, WordValue s20, int n0)
          : x(x0), c(c0), s1(s10), s2(s20), n(n0) {
        assert(n < 63);
        for (int p = 0; p <= n + 1; ++p)
          for (int s = 0; s < states; ++s)
            memo[p][s] = -1;
      }
      /// Find the greatest x as \a result; returns whether there is one
      bool greatest(WordValue& result) {
        const int start = encode(1, 1, 1, 1, 0, 0);
        if (!feasible(n + 1, start))
          return false;
        uint64_t current[3] = {0, 0, 0}, following[3];
        current[start / 64] |= uint64_t(1) << (start % 64);
        result = 0;
        for (int p = n; p >= 0; --p)
          for (int xb = 1; xb >= 0; --xb) {
            if (!allowed(x, p, xb))
              continue;
            following[0] = following[1] = following[2] = 0;
            for (int s = 0; s < states; ++s)
              if ((current[s / 64] >> (s % 64)) & 1)
                for (int cb = 0; cb < 2; ++cb)
                  if (allowed(c, p, cb)) {
                    const int after = next(s, p, xb, cb);
                    if (after >= 0 && feasible(p, after))
                      following[after / 64] |= uint64_t(1) << (after % 64);
                  }
            if ((following[0] | following[1] | following[2]) != 0) {
              result |= WordValue(xb) << p;
              for (int w = 0; w < 3; ++w)
                current[w] = following[w];
              break;
            }
          }
        return true;
      }
    };

    /** \brief Greatest x of \a x for which some t of \a t has x-t in
     * \a low..\a high; returns whether there is one
     *
     * With n bits for both and c = 2^n-1-t, x-t = x+c-(2^n-1), so this is
     * the greatest x with x+c in low+2^n-1..high+2^n-1 (see GreatestSum).
     */
    bool greatest_difference(const KnownBits& x, const KnownBits& t, const Integer& low, const Integer& high,
                             WordValue& result) {
      const Integer least = std::max(low, Integer(x.least) - Integer(t.greatest));
      const Integer greatest = std::min(high, Integer(x.greatest) - Integer(t.least));
      if (least > greatest)
        return false;
      const unsigned int n = std::max(x.width, t.width);
      const Integer all(low_mask(n));
      const KnownBits c = complement({t.least, t.greatest, t.ones, t.zeros, n});
      GreatestSum search(x, c, (least + all).word(), (greatest + all).word(), static_cast<int>(n));
      return search.greatest(result);
    }

    /// Which term of greatest_scaled() carries the factor 2^k
    enum class Coarse {
      First, ///< 2^k*x-y
      Second ///< x-2^k*y
    };

    /** \brief Greatest x of \a x for which some y of \a y has 2^\a k*x-y
     * (Coarse::First) or x-2^k*y (Coarse::Second) in \a low..\a high;
     * returns whether there is one
     *
     * When the scaled term spans at most two bits more than the other, this
     * is greatest_difference() on the scaled set, with at most 49 bits.
     * Otherwise 2^k is more than twice the bound 2^w of the unscaled term,
     * and each value v of the scaled term admits the unscaled values in a
     * window v-high..v-low or low+v..high+v. The windows of successive v
     * lie at least 2^k apart, so at most one of them has its lower end and
     * at most one its upper end within the range of the unscaled term,
     * which is shorter than 2^w. Every other window holds that whole range,
     * and with it a member, or misses it. Going down from the greatest v
     * whose window reaches the range, at most two candidates fail before
     * one succeeds or the windows pass below the range: the search takes at
     * most three steps, whatever the size of the sets.
     */
    bool greatest_scaled(const KnownBits& x, const KnownBits& y, unsigned int k, Coarse coarse, const Integer& low,
                         const Integer& high, WordValue& result) {
      WordValue v, m;
      if (coarse == Coarse::First) {
        if (k <= y.width + 1) {
          if (!greatest_difference(shifted(x, k), y, low, high, v))
            return false;
          result = v >> k;
          return true;
        }
        // The greatest x whose window starts at most at y.greatest.
        const Integer top = floor_shift(high + Integer(y.greatest), k);
        if (top.neg() || !member_at_most(x, top.bits() > 63 ? x.greatest : std::min(top.word(), x.greatest), v))
          return false;
        for (;;) {
          const Integer base = Integer(v).shift_left(k);
          const Integer window_low = base - high, window_high = base - low;
          if (window_high < Integer(y.least))
            return false;
          if (member_at_least(y, window_low < Integer(y.least) ? y.least : window_low.word(), m) &&
              Integer(m) <= window_high) {
            result = v;
            return true;
          }
          if (v == 0 || !member_at_most(x, v - 1, v))
            return false;
        }
      }
      if (k <= x.width + 1)
        return greatest_difference(x, shifted(y, k), low, high, result);
      // The greatest y whose window starts at most at x.greatest.
      const Integer top = floor_shift(Integer(x.greatest) - low, k);
      if (top.neg() || !member_at_most(y, top.bits() > 63 ? y.greatest : std::min(top.word(), y.greatest), v))
        return false;
      for (;;) {
        const Integer base = Integer(v).shift_left(k);
        const Integer window_low = base + low, window_high = base + high;
        if (window_high < Integer(x.least))
          return false;
        if (member_at_most(x, window_high > Integer(x.greatest) ? x.greatest : window_high.word(), m) &&
            Integer(m) >= window_low) {
          result = m;
          return true;
        }
        if (v == 0 || !member_at_most(y, v - 1, v))
          return false;
      }
    }

    /** \brief The values sign*(hidden+F)*2^\a g, in units of 2^-149, of the
     * fractions F of \a fraction
     *
     * A piece is one binade of one sign of a wide word, whose values are
     * affine in the fraction bits of their codes, or one code of a tiny
     * word. Its fraction bounds are members. When the word is also the
     * reference, its values admit only the shifts qmax-q of their own
     * magnitudes, s_low..s_high; otherwise every shift.
     */
    struct Piece {
      bool negative;
      unsigned int g;
      WordValue hidden;
      KnownBits fraction;
      unsigned int s_low, s_high;
    };

    /// The interval of Q and the shifts qmax-q that the references admit
    struct NumeratorBounds {
      Integer least, greatest;
      unsigned int s_low, s_high;
      /// Every distance is a multiple of 2^fine
      unsigned int fine;
    };

    /** \brief The distances \a closest..\a farthest, multiples of 2^fine,
     * with Q = D*2^s in the interval of \a n for a shift s in both
     * \a s_low..\a s_high and the shifts of \a n; returns whether there
     * is one
     */
    bool distance_interval(const NumeratorBounds& n, unsigned int s_low, unsigned int s_high, Integer& closest,
                           Integer& farthest) {
      s_low = std::max(s_low, n.s_low);
      s_high = std::min(s_high, n.s_high);
      if (s_low > s_high)
        return false;
      farthest = n.greatest.shift_right(s_low + n.fine).shift_left(n.fine);
      closest = round_up((n.least + Integer::power_two(s_high) - Integer(1)).shift_right(s_high), n.fine);
      return closest <= farthest;
    }

    /// Value of the fraction \a f of \a p
    Integer piece_value(const Piece& p, WordValue f) {
      const Integer v = Integer(p.hidden + f).shift_left(p.g);
      return p.negative ? -v : v;
    }
    /// Least value of \a p
    Integer piece_least(const Piece& p) {
      return piece_value(p, p.negative ? p.fraction.greatest : p.fraction.least);
    }
    /// Greatest value of \a p
    Integer piece_greatest(const Piece& p) {
      return piece_value(p, p.negative ? p.fraction.least : p.fraction.greatest);
    }

    /** \brief Greatest value \a value of \a px for which some value of
     * \a py lies at a difference in \a low..\a high below it; returns
     * whether there is one
     *
     * With v = sx*(hx+Fx)*2^gx and w = sy*(hy+Fy)*2^gy, v-w is
     * sx*2^gx*Fx-sy*2^gy*Fy plus a constant. Negating the condition when
     * sx < 0, and writing Fy as 2^wy-1 minus its complement when the signs
     * differ, gives 2^gx*Fx-2^gy*Fy in an interval. The greatest v has the
     * greatest Fx when px is positive, and otherwise the least, which is
     * the greatest complement of Fx when both fractions are complemented.
     * Dividing by 2^min(gx,gy) leaves the condition of greatest_scaled().
     */
    bool piece_pair_greatest(const Piece& px, const Piece& py, const Integer& low, const Integer& high,
                             Integer& value) {
      KnownBits x = px.fraction, y = py.fraction;
      const Integer hx = Integer(px.hidden).shift_left(px.g), hy = Integer(py.hidden).shift_left(py.g);
      const Integer base = (px.negative ? -hx : hx) - (py.negative ? -hy : hy);
      Integer l = low - base, h = high - base;
      if (px.negative) {
        const Integer swap = l;
        l = -h;
        h = -swap;
      }
      if (px.negative != py.negative) {
        const Integer shift = Integer(low_mask(y.width)).shift_left(py.g);
        l = l - shift;
        h = h - shift;
        y = complement(y);
      }
      if (px.negative) {
        const Integer k =
            Integer(low_mask(x.width)).shift_left(px.g) - Integer(low_mask(y.width)).shift_left(py.g);
        const Integer swap = l;
        l = k - h;
        h = k - swap;
        x = complement(x);
        y = complement(y);
      }
      const unsigned int g = std::min(px.g, py.g);
      const Integer scaled_low = ceil_shift(l, g), scaled_high = floor_shift(h, g);
      if (scaled_low > scaled_high)
        return false;
      WordValue f;
      if (!(px.g >= py.g ? greatest_scaled(x, y, px.g - g, Coarse::First, scaled_low, scaled_high, f)
                         : greatest_scaled(x, y, py.g - g, Coarse::Second, scaled_low, scaled_high, f)))
        return false;
      value = piece_value(px, px.negative ? low_mask(x.width) - f : f);
      return true;
    }

    /// Room for the pieces of a word of format \a f
    int piece_capacity(Format f) {
      return f.width <= 8 ? 256 : 2 * static_cast<int>((f.maximum >> f.fraction_bits) + 1);
    }

    /// The reference format and qmax when a word is also the reference
    struct PieceReference {
      const Format* reference;
      int qmax;
    };

    /// Set the shifts of \a p, whose magnitudes are \a first..\a last
    void set_shifts(Format f, const PieceReference& r, WordValue first, WordValue last, Piece& p) {
      if (r.reference == nullptr) {
        p.s_low = 0;
        p.s_high = ~0U;
      } else {
        p.s_low = static_cast<unsigned int>(r.qmax - magnitude_spacing(f, *r.reference, last));
        p.s_high = static_cast<unsigned int>(r.qmax - magnitude_spacing(f, *r.reference, first));
      }
    }

    /// Add the codes \a first..\a last of the binade \a e of one sign to
    /// \a pieces, if they hold a member of the cube of \a s
    void add_binade(Format f, const SignedRanges& s, const PieceReference& r, bool negative, WordValue e,
                    WordValue first, WordValue last, Piece* pieces, int& n) {
      const WordValue fraction = (WordValue(1) << f.fraction_bits) - 1;
      const WordValue exponent_ones = s.ones >> f.fraction_bits, exponent_zeros = s.zeros >> f.fraction_bits;
      if ((e & exponent_ones) != exponent_ones || (e & exponent_zeros) != 0)
        return;
      Piece p = {negative, static_cast<unsigned int>(lattice_spacing(f, e << f.fraction_bits)),
                 e == 0 ? 0 : WordValue(1) << f.fraction_bits,
                 {first & fraction, last & fraction, s.ones & fraction, s.zeros & fraction, f.fraction_bits}, 0, 0};
      if (!member_at_least(p.fraction, p.fraction.least, p.fraction.least) ||
          !member_at_most(p.fraction, p.fraction.greatest, p.fraction.greatest))
        return;
      const WordValue binade = e << f.fraction_bits;
      set_shifts(f, r, binade | p.fraction.least, binade | p.fraction.greatest, p);
      pieces[n++] = p;
    }

    /// Add the binades of the magnitudes of the sign \a sign of \a s to
    /// \a pieces, in increasing order of value
    void add_side(Format f, const SignedRanges& s, const PieceReference& r, Sign sign, Piece* pieces, int& n) {
      const bool negative = sign == Sign::Negative;
      const MagnitudeRange& side = negative ? s.negative : s.positive;
      if (!side.present)
        return;
      const WordValue low = side.least >> f.fraction_bits, high = side.greatest >> f.fraction_bits;
      for (WordValue i = 0; i <= high - low; ++i) {
        const WordValue e = negative ? high - i : low + i;
        add_binade(f, s, r, negative, e, std::max(e << f.fraction_bits, side.least),
                   std::min(((e + 1) << f.fraction_bits) - 1, side.greatest), pieces, n);
      }
    }

    /// Add the code \a c of a tiny word to \a pieces, if \a s holds it
    void add_code(Format f, const SignedRanges& s, const PieceReference& r, WordValue c, Piece* pieces, int& n) {
      const WordValue sign = sign_of(f), fraction = (WordValue(1) << f.fraction_bits) - 1;
      const bool negative = (c & sign) != 0;
      const WordValue m = c & (sign - 1);
      const MagnitudeRange& side = negative ? s.negative : s.positive;
      if (!side.present || m < side.least || m > side.greatest)
        return;
      const WordValue e = m >> f.fraction_bits;
      const WordValue fm = m & fraction;
      Piece p = {negative, static_cast<unsigned int>(lattice_spacing(f, m)),
                 e == 0 ? 0 : WordValue(1) << f.fraction_bits, {fm, fm, fm, ~fm & fraction, f.fraction_bits}, 0, 0};
      set_shifts(f, r, m, m, p);
      pieces[n++] = p;
    }

    /// The pieces of the values of \a s of the word \a x, in increasing
    /// order of value; returns their number
    int word_pieces(WordView x, Format f, const SignedRanges& s, const PieceReference& r, Piece* pieces) {
      int n = 0;
      if (f.width > 8) {
        add_side(f, s, r, Sign::Negative, pieces, n);
        add_side(f, s, r, Sign::Positive, pieces, n);
        return n;
      }
      // The codes increase, so the negative ones come last, with
      // increasing magnitude: backwards they come in increasing value.
      const TinyValues codes(x);
      unsigned int positive = 0;
      while (positive < codes.size() && (codes[positive] & sign_of(f)) == 0)
        ++positive;
      for (unsigned int i = codes.size(); i-- > positive;)
        add_code(f, s, r, codes[i], pieces, n);
      for (unsigned int i = 0; i < positive; ++i)
        add_code(f, s, r, codes[i], pieces, n);
      return n;
    }

    /// The pieces of a word in increasing order of value, or those of its
    /// negation (sign Negative), and whether the word is also the reference
    struct PieceOrder {
      const Piece* pieces;
      int count;
      Sign sign;
      bool reference;
      Piece operator[](int i) const {
        if (sign == Sign::Positive)
          return pieces[i];
        Piece p = pieces[count - 1 - i];
        p.negative = !p.negative;
        return p;
      }
    };

    /// Index of the first piece of \a o whose greatest value is at least
    /// \a v, or o.count
    int first_reaching(const PieceOrder& o, const Integer& v) {
      int low = 0, high = o.count;
      while (low < high) {
        const int middle = low + (high - low) / 2;
        if (piece_greatest(o[middle]) >= v)
          high = middle;
        else
          low = middle + 1;
      }
      return low;
    }

    /// Whether \a p has a value in \a low..\a high
    bool piece_has_value(const Piece& p, const Integer& low, const Integer& high) {
      if (piece_greatest(p) < low || piece_least(p) > high)
        return false;
      // The least value of the piece that is at least low, which exists.
      WordValue f;
      if (!p.negative) {
        const Integer need = ceil_shift(low, p.g) - Integer(p.hidden);
        (void)member_at_least(p.fraction, need <= Integer(p.fraction.least) ? p.fraction.least : need.word(), f);
      } else {
        const Integer cap = floor_shift(-low, p.g) - Integer(p.hidden);
        (void)member_at_most(p.fraction, cap >= Integer(p.fraction.greatest) ? p.fraction.greatest : cap.word(),
                             f);
      }
      return piece_value(p, f) <= high;
    }

    /// Whether \a o has a value in \a low..\a high
    bool has_value(const PieceOrder& o, const Integer& low, const Integer& high) {
      // The pieces are disjoint and ordered, so the least value of o that
      // is at least low lies in the first piece reaching low.
      const int i = first_reaching(o, low);
      return i < o.count && piece_has_value(o[i], low, high);
    }

    /** \brief A piece of the word that greatest_supported() pairs with,
     * with what does not depend on the other piece of a pair
     */
    struct PieceReach {
      /// The least and greatest value of the piece
      Integer least, greatest;
      /// Whether the piece's own shifts admit a distance (for the reference)
      bool admits = false;
      /// The distances that the piece's own shifts admit
      Integer closest, farthest;
      /** \brief For each side, the values least+low..greatest+high that
       * the piece pairs with at those distances (low..high is
       * closest..farthest or -farthest..-closest)
       */
      Integer from[2], to[2];
    };

    /// Fill \a r for the piece \a q; \a reference tells whether its word
    /// is the reference
    void piece_reach(const Piece& q, bool reference, const NumeratorBounds& n, PieceReach& r) {
      r.least = piece_least(q);
      r.greatest = piece_greatest(q);
      r.admits = reference && distance_interval(n, q.s_low, q.s_high, r.closest, r.farthest);
      if (r.admits) {
        r.from[0] = r.least + r.closest;
        r.to[0] = r.greatest + r.farthest;
        r.from[1] = r.least - r.farthest;
        r.to[1] = r.greatest - r.closest;
      }
    }

    /** \brief The PieceReach of each piece of a word, computed when first
     * read
     *
     * The search for a greatest supported value usually stops at the
     * first pieces it reads, so the pieces are not all filled in advance.
     */
    class ReachCache {
      Region& region;
      const PieceOrder& ys;
      const NumeratorBounds& n;
      PieceReach* reach;
      bool* filled;

    public:
      /// The cache for the pieces of \a ys0 under \a n0, in \a r
      ReachCache(Region& r, const PieceOrder& ys0, const NumeratorBounds& n0)
          : region(r), ys(ys0), n(n0), reach(static_cast<PieceReach*>(r.ralloc(sizeof(PieceReach) * ys0.count))),
            filled(r.alloc<bool>(ys0.count)) {
        for (int j = 0; j < ys.count; ++j)
          filled[j] = false;
      }
      ReachCache(const ReachCache&) = delete;
      ReachCache& operator=(const ReachCache&) = delete;
      /// Destroy the filled entries
      ~ReachCache() {
        for (int j = 0; j < ys.count; ++j)
          if (filled[j])
            reach[j].~PieceReach();
        region.free<bool>(filled, ys.count);
        region.rfree(reach, sizeof(PieceReach) * ys.count);
      }
      /// The reach of piece \a j
      const PieceReach& operator[](int j) {
        if (!filled[j]) {
          new (&reach[j]) PieceReach();
          filled[j] = true;
          piece_reach(ys[j], ys.reference, n, reach[j]);
        }
        return reach[j];
      }
    };

    /** \brief Greatest value \a value of \a xs that has a value of \a ys,
     * whose pieces \a reach describes, at a distance that \a n admits;
     * returns whether there is one
     *
     * A pair of values admits the distances of the shifts that both their
     * pieces admit (see distance_interval()). Goes down the pieces of
     * \a xs. A piece whose greatest value has a partner gives that value.
     * When every piece of \a ys admits every shift, that test reads \a ys
     * as a whole; otherwise it runs on each piece of \a ys within reach,
     * with the distances of the pair. Then each side (v-w in
     * closest..farthest or in -farthest..-closest) tests the pieces of
     * \a ys within reach of the piece with piece_pair_greatest(), which
     * computes the greatest supported value of a pair of pieces directly,
     * unless the pair's value hulls are too far apart. The first piece
     * with a supported value holds the greatest one. When the shifts of
     * a pair are those of its piece of \a ys, which holds when \a ys is
     * the reference and \a xs is not, the hull test reads the values that
     * piece pairs with from \a reach.
     */
    bool greatest_reached(const PieceOrder& xs, const PieceOrder& ys, ReachCache& reach, const NumeratorBounds& n,
                          Integer& value) {
      for (int i = xs.count; i-- > 0;) {
        const Piece p = xs[i];
        Integer closest, farthest;
        if (!distance_interval(n, p.s_low, p.s_high, closest, farthest))
          continue;
        const Integer top = piece_greatest(p);
        if (!ys.reference &&
            (has_value(ys, top - farthest, top - closest) || has_value(ys, top + closest, top + farthest))) {
          value = top;
          return true;
        }
        const Integer bottom = piece_least(p);
        bool found = false;
        for (int side = 0; side < 2; ++side) {
          const Integer low = side == 0 ? closest : -farthest, high = side == 0 ? farthest : -closest;
          const Integer last = top - low;
          for (int j = first_reaching(ys, bottom - high); j < ys.count; ++j) {
            const PieceReach& h = reach[j];
            if (h.least > last)
              break;
            const Piece q = ys[j];
            const bool own = ys.reference && p.s_low <= q.s_low && p.s_high >= q.s_high;
            if (own && (!h.admits || h.to[side] < bottom || h.from[side] > top))
              continue;
            Integer pair_closest = own ? h.closest : closest, pair_farthest = own ? h.farthest : farthest;
            if (ys.reference && !own &&
                !distance_interval(n, std::max(p.s_low, q.s_low), std::min(p.s_high, q.s_high), pair_closest,
                                   pair_farthest))
              continue;
            const Integer pair_low = side == 0 ? pair_closest : -pair_farthest;
            const Integer pair_high = side == 0 ? pair_farthest : -pair_closest;
            if (h.greatest < bottom - pair_high || h.least > top - pair_low)
              continue;
            if (ys.reference && piece_has_value(q, top - pair_high, top - pair_low)) {
              value = top;
              return true;
            }
            Integer v;
            if (piece_pair_greatest(p, q, pair_low, pair_high, v) && (!found || v > value)) {
              value = v;
              found = true;
            }
          }
        }
        if (found)
          return true;
      }
      return false;
    }

    /// Greatest value \a value of \a xs that has a value of \a ys at a
    /// distance that \a n admits; returns whether there is one (see
    /// greatest_reached())
    bool greatest_supported(const PieceOrder& xs, const PieceOrder& ys, const NumeratorBounds& n, Integer& value) {
      Region region;
      ReachCache reach(region, ys, n);
      return greatest_reached(xs, ys, reach, n, value);
    }

    /// The word among a and b whose own binades fix the shift of the
    /// reference, as the reference is that word
    enum class ShiftWord { Neither, A, B };

    /** \brief Keep the values of each of \a sa and \a sb between the least
     * and greatest value with a value of the other word at a distance that
     * \a n admits; returns whether both keep one
     *
     * Every distance abs(a-b) is one that \a n admits with the shift of
     * the reference, so this is sound. When the reference is a or b
     * (\a shift_word), the shift follows the magnitude of that word, so
     * each of its binades admits only its own distances. The words are
     * read with their cubes, so the bounds are members, and a pair of
     * bounds has partners within the other word's new bounds. Computing
     * the supported bounds directly, rather than narrowing each word to
     * the other's hull plus or minus the distances, reaches them in one
     * call: hull narrowing moves one code per pass when the distances are
     * narrow and the cubes or spacings differ, and one binade per pass of
     * the word that is also the reference.
     */
    bool keep_supported_values(const ErrorFormats& f, WordView a, WordView b, SignedRanges& sa, SignedRanges& sb,
                               const NumeratorBounds& n, ShiftWord shift_word) {
      const bool reference_a = shift_word == ShiftWord::A, reference_b = shift_word == ShiftWord::B;
      Region region;
      Piece* pa = region.alloc<Piece>(piece_capacity(f.a));
      Piece* pb = region.alloc<Piece>(piece_capacity(f.b));
      const PieceReference ra = {reference_a ? &f.reference : nullptr, f.error.qmax};
      const PieceReference rb = {reference_b ? &f.reference : nullptr, f.error.qmax};
      const int na = word_pieces(a, f.a, sa, ra, pa), nb = word_pieces(b, f.b, sb, rb, pb);
      const PieceOrder up_a = {pa, na, Sign::Positive, reference_a}, up_b = {pb, nb, Sign::Positive, reference_b};
      const PieceOrder down_a = {pa, na, Sign::Negative, reference_a}, down_b = {pb, nb, Sign::Negative, reference_b};
      Integer a_greatest, a_least, b_greatest, b_least;
      if (!greatest_supported(up_a, up_b, n, a_greatest) || !greatest_supported(down_a, down_b, n, a_least) ||
          !greatest_supported(up_b, up_a, n, b_greatest) || !greatest_supported(down_b, down_a, n, b_least))
        return false;
      sa.restrict_values(f.a, -a_least, a_greatest);
      sb.restrict_values(f.b, -b_least, b_greatest);
      return !sa.empty() && !sb.empty();
    }

    /** \brief Bounds reasoning for fp_ulp_error
     *
     * With D = abs(a-b)*2^149 and the spacing exponent q of the reference,
     * the numerator is Q = D*2^(qmax-q). Each pass reads the finite
     * magnitude codes of each sign of a, b and the reference, which also
     * removes every nonfinite value, and computes:
     *  - the least and greatest D from the value intervals of a and b, and
     *    the least and greatest q from the least and greatest reference
     *    magnitude, which bound Q and so zero the digits above its bit
     *    length;
     *  - digit bounds and the numerator interval against each other, with
     *    Q a multiple of 2^(qmax-q) times the finest spacing of a and b
     *    (see finest_spacing()), which zeroes its low digits (see
     *    narrow_digits());
     *  - the spacing exponents that admit a numerator in the interval with
     *    some D, and, when the digits force a nonzero Q whose lowest set
     *    bit is at most L, those with finest+qmax-q <= L (see
     *    zero_low_bits()), which bound the reference magnitudes;
     *  - for a reference that is also a or b, the binades at the ends of
     *    that word whose own q admits no numerator in the interval (see
     *    trim_reference_binades());
     *  - the distances D that the interval admits with some q, which keep
     *    each of a and b between its least and greatest value with a value
     *    of the other word at such a distance, reading both words with
     *    their cubes and, when the reference is a or b, with the q of each
     *    binade of that word (see keep_supported_values()), again after
     *    each round of the trims below that changes a or b;
     *  - the binades at the ends of a and b whose low digits no value of
     *    the other word within the reach of D admits, or whose significand
     *    bits the cube of the word rules out for every residue that the
     *    digits leave (see trim_groups(), low_digits_admit() and
     *    units_meet()), and the values of an end binade
     *    beyond the other word's interval when only the values within it
     *    admit a numerator (see keep_met_side()), repeated with the
     *    interval of Q narrowed to the remaining words until neither a nor
     *    b changes (see narrow_trim_bounds()).
     *
     * A wide word keeps the rank hull of its narrowed signs and the bits
     * that both fix to the same value (see SignedRanges::publish()). The
     * propagator repeats passes until one changes none of a, b and r,
     * which is its fixpoint, and so reports ES_FIX. Aliases among a, b and
     * the reference are read as separate words, except for the reference
     * binades above and that a == b never reaches this propagator. Once
     * a, b and the reference are assigned, the digits are computed
     * exactly. With tiny a, b and reference, an assigned one rewrites the
     * propagator into TinyError.
     */
    class ErrorBounds : public Propagator {
      WordView a, b, r;
      ViewArray<Int::IntView> digits;
      const ErrorFormats formats;

      ErrorBounds(Home home, WordView x, WordView y, WordView z,
                  ViewArray<Int::IntView>& d, const ErrorFormats& f)
          : Propagator(home), a(x), b(y), r(z), digits(d), formats(f) {
        a.subscribe(home, *this, Word::PC_WORD_DOM);
        b.subscribe(home, *this, Word::PC_WORD_DOM);
        r.subscribe(home, *this, Word::PC_WORD_DOM);
        digits.subscribe(home, *this, Int::PC_INT_BND);
      }
      ErrorBounds(Space& home, ErrorBounds& other)
          : Propagator(home, other), formats(other.formats) {
        a.update(home, other.a);
        b.update(home, other.b);
        r.update(home, other.r);
        digits.update(home, other.digits);
      }

      static bool tiny(const ErrorFormats& f) {
        return f.a.width <= 8 && f.b.width <= 8 && f.r.width <= 8;
      }

      /// Assign the digits of the assigned \a a, \a b and \a r
      static ExecStatus assign_digits(Space& home, WordView a, WordView b,
                                      WordView r,
                                      ViewArray<Int::IntView>& digits,
                                      const ErrorFormats& f) {
        const WordValue ma = a.val() & (sign_of(f.a) - 1),
                        mb = b.val() & (sign_of(f.b) - 1),
                        mr = r.val() & (sign_of(f.r) - 1);
        if (ma > f.a.maximum || mb > f.b.maximum || mr > f.r.maximum)
          return ES_FAILED;
        Integer va = lattice_value(f.a, ma), vb = lattice_value(f.b, mb);
        if ((a.val() & sign_of(f.a)) != 0)
          va = -va;
        if ((b.val() & sign_of(f.b)) != 0)
          vb = -vb;
        Integer q = (va - vb).abs().shift_left(static_cast<unsigned int>(
            f.error.qmax - magnitude_spacing(f.r, f.reference, mr)));
        for (int i = digits.size(); i--;) {
          GECODE_ME_CHECK(
              digits[i].eq(home, static_cast<int>(q.low_bits(30).word())));
          q = q.shift_right(30);
        }
        assert(q.zero());
        return ES_OK;
      }

      /// One bounds pass; \a changed tells whether a, b or r changed
      static ExecStatus filter(Space& home, WordView a, WordView b,
                               WordView r, ViewArray<Int::IntView>& digits,
                               const ErrorFormats& f, bool& changed) {
        SignedRanges sa(a, f.a), sb(b, f.b), sr(r, f.r);
        if (sa.empty() || sb.empty() || sr.empty())
          return ES_FAILED;
        const int qmax = f.error.qmax;
        const ValueInterval d = distances(f.a, sa, f.b, sb);
        int q_low = magnitude_spacing(f.r, f.reference, sr.least());
        int q_high = magnitude_spacing(f.r, f.reference, sr.greatest());
        Integer least = d.least.shift_left(qmax - q_high);
        Integer greatest = d.greatest.shift_left(qmax - q_low);
        const int finest = finest_spacing(f, sa, sb);
        GECODE_ES_CHECK(
            narrow_digits(home, digits, least, greatest,
                          static_cast<unsigned int>(finest + qmax - q_high)));
        const unsigned int width =
            30 * static_cast<unsigned int>(digits.size());
        // Spacing exponents with Q = D*2^(qmax-q) in least..greatest.
        int q_least = d.least.zero() ? q_low : qmax - static_cast<int>(widest_shift(d.least, greatest));
        // When the digits make Q nonzero with its lowest set bit at or
        // below zero_bits, a multiple of 2^(finest+qmax-q), then
        // finest+qmax-q <= zero_bits. This also covers the bound that
        // a != b gives when least is positive but d.least is zero, with
        // D >= 2^finest and so 2^(finest+qmax-q) <= greatest: least is then
        // a sum of digit minima, so the digits exclude Q = 0 and zero_bits
        // < width. Every digit is at most greatest over its weight, so
        // 2^zero_bits <= greatest, and q >= finest+qmax-zero_bits gives
        // 2^(finest+qmax-q) <= 2^zero_bits <= greatest.
        const unsigned int zero_bits = zero_low_bits(digits);
        if (zero_bits < width)
          q_least = std::max(q_least, finest + qmax - static_cast<int>(zero_bits));
        const int q_greatest =
            d.greatest.zero()
                ? q_high
                : qmax - static_cast<int>(narrowest_shift(d.greatest, least));
        if (q_least > q_low || q_greatest < q_high) {
          q_low = std::max(q_low, q_least);
          q_high = std::min(q_high, q_greatest);
          if (q_low > q_high)
            return ES_FAILED;
          const WordValue first = first_magnitude(f.r, [&](WordValue m) {
            return magnitude_spacing(f.r, f.reference, m) >= q_low;
          });
          const WordValue beyond = first_magnitude(f.r, [&](WordValue m) {
            return magnitude_spacing(f.r, f.reference, m) > q_high;
          });
          if (beyond == 0)
            return ES_FAILED;
          sr.restrict(first, beyond - 1);
        }
        if (r == a || r == b) {
          // The reference is also a or b, so q follows its magnitude.
          const bool is_a = r == a;
          SignedRanges& x = is_a ? sa : sb;
          x.intersect(sr);
          trim_reference_binades(f, is_a ? f.a : f.b, x, is_a ? f.b : f.a, is_a ? sb : sa, least, greatest);
          if (x.empty())
            return ES_FAILED;
          sr.intersect(x);
        }
        // Distances D with Q in least..greatest for some remaining q, as
        // multiples of 2^finest.
        const unsigned int fine = static_cast<unsigned int>(finest);
        const unsigned int s_low = static_cast<unsigned int>(qmax - q_high);
        const unsigned int s_high = static_cast<unsigned int>(qmax - q_low);
        NumeratorBounds numerators = {least, greatest, s_low, s_high, fine};
        Integer closest, farthest;
        if (!distance_interval(numerators, s_low, s_high, closest, farthest))
          return ES_FAILED;
        // Every distance of the words lies in d, so the words keep all their
        // values when d lies within closest..farthest, unless the reference
        // is a or b with more than one q, so that each of its binades
        // admits only some of the distances.
        const bool own_shifts = (r == a || r == b) && q_low < q_high;
        bool support = farthest < d.greatest || closest > d.least || own_shifts;
        // Every Q is a multiple of 2^(finest+s_low). A group tests the
        // digits up to the one that holds the lowest bit of its significand
        // in Q (see units_meet()), which is at bit coarsest+s_high at most.
        // When the low digits admit every multiple there, every group
        // passes low_digits_admit(), which is then skipped.
        const unsigned int free_bits = free_low_bits(digits, fine + s_low);
        const unsigned int coarsest = static_cast<unsigned int>(
            std::max(coarsest_spacing(f.a, sa), coarsest_spacing(f.b, sb)));
        const unsigned int reach = std::min(30 * ((coarsest + s_high) / 30 + 1), width);
        TrimBounds bounds_a = {digits, reach > free_bits, free_bits, s_low, s_high, least, greatest, nullptr, qmax};
        TrimBounds bounds_b = bounds_a;
        bounds_a.reference = r == a ? &f.reference : nullptr;
        bounds_b.reference = r == b ? &f.reference : nullptr;
        // Removing groups of one word moves the values that the other is
        // tested against, and narrows D, and q when the reference is a or
        // b, so the interval of Q. Repeat until neither word changes, which
        // tests at most two groups per end and round beyond those removed.
        // The supported values of each word move with the other word too,
        // so they are computed again on the narrowed words and interval of
        // Q. A word that is also the reference loses a binade at the end
        // of its magnitudes that no supported value reaches, and the other
        // word then the values that only that binade supported; repeating
        // within this loop removes these in one execution instead of one
        // binade per pass.
        const SignedRanges* sr_word = r == a ? &sa : r == b ? &sb : nullptr;
        const ShiftWord shift_word = !own_shifts ? ShiftWord::Neither : r == a ? ShiftWord::A : ShiftWord::B;
        for (bool narrowed = true; narrowed;) {
          const SignedRanges start_a = sa, start_b = sb;
          if (support && !keep_supported_values(f, a, b, sa, sb, numerators, shift_word))
            return ES_FAILED;
          if (!sa.same(start_a) || !sb.same(start_b))
            narrow_trim_bounds(f, sr_word, sa, sb, bounds_a, bounds_b);
          for (bool again = true; again;) {
            const SignedRanges old_a = sa, old_b = sb;
            trim_groups(f.a, sa, f.b, sb, bounds_a);
            if (sa.empty())
              return ES_FAILED;
            trim_groups(f.b, sb, f.a, sa, bounds_b);
            if (sb.empty())
              return ES_FAILED;
            again = !sa.same(old_a) || !sb.same(old_b);
            if (again)
              narrow_trim_bounds(f, sr_word, sa, sb, bounds_a, bounds_b);
          }
          narrowed = !sa.same(start_a) || !sb.same(start_b);
          if (narrowed) {
            numerators = {bounds_a.least, bounds_a.greatest, bounds_a.s_low, bounds_a.s_high, fine};
            if (!distance_interval(numerators, bounds_a.s_low, bounds_a.s_high, closest, farthest))
              return ES_FAILED;
            const ValueInterval dn = distances(f.a, sa, f.b, sb);
            support = farthest < dn.greatest || closest > dn.least || own_shifts;
          }
        }
        if (r == a || r == b)
          sr.intersect(r == a ? sa : sb);
        if (failed_change(sa.publish(home, a, f.a), changed) ||
            failed_change(sb.publish(home, b, f.b), changed) ||
            failed_change(sr.publish(home, r, f.r), changed))
          return ES_FAILED;
        return ES_OK;
      }

    public:
      /// Post the propagator for fp_ulp_error, or decide the posting
      static ExecStatus post(Home home, WordView a, WordView b, WordView r,
                             ViewArray<Int::IntView>& digits,
                             const ErrorFormats& f,
                             Word::Float::Consistency level) {
        if (a == b) {
          // abs(a-a) = 0 for every finite a.
          for (int i = 0; i < digits.size(); ++i)
            GECODE_ME_CHECK(digits[i].eq(home, 0));
          GECODE_ME_CHECK(SignedRanges(a, f.a).publish(home, a, f.a));
          GECODE_ME_CHECK(SignedRanges(r, f.r).publish(home, r, f.r));
          return ES_OK;
        }
        if (tiny(f) &&
            (level == Word::Float::Consistency::Domain || a.assigned() ||
             b.assigned() || r.assigned()))
          return TinyError::post(home, a, b, r, digits, f);
        if (a.assigned() && b.assigned() && r.assigned())
          return assign_digits(home, a, b, r, digits, f);
        (void)new (home) ErrorBounds(home, a, b, r, digits, f);
        return ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) ErrorBounds(home, *this);
      }
      PropCost cost(const Space&, const ModEventDelta&) const override {
        return PropCost::linear(PropCost::HI, digits.size());
      }
      void reschedule(Space& home) override {
        a.reschedule(home, *this, Word::PC_WORD_DOM);
        b.reschedule(home, *this, Word::PC_WORD_DOM);
        r.reschedule(home, *this, Word::PC_WORD_DOM);
        digits.reschedule(home, *this, Int::PC_INT_BND);
      }
      size_t dispose(Space& home) override {
        a.cancel(home, *this, Word::PC_WORD_DOM);
        b.cancel(home, *this, Word::PC_WORD_DOM);
        r.cancel(home, *this, Word::PC_WORD_DOM);
        digits.cancel(home, *this, Int::PC_INT_BND);
        (void)Propagator::dispose(home);
        return sizeof(*this);
      }
      // Passes repeat until one changes none of a, b and r. Such a pass is
      // a fixpoint: narrow_digits() is idempotent, so a second pass starts
      // from the same interval and digit bounds, recomputes the same
      // values and publishes nothing. ES_FIX is therefore honest, also for
      // the digit and word changes of this execution.
      ExecStatus propagate(Space& home, const ModEventDelta&) override {
        // The number of passes can grow with the size of the domains, by
        // the bits of the digits and the binades of the words rather than
        // by their codes: when the reference is a or b, a digit that is
        // not fixed can halve in each pass while that word loses its least
        // binade.
        for (;;) {
          if (a.assigned() && b.assigned() && r.assigned()) {
            GECODE_ES_CHECK(assign_digits(home, a, b, r, digits, formats));
            return home.ES_SUBSUMED(*this);
          }
          if (tiny(formats) && (a.assigned() || b.assigned() || r.assigned()))
            GECODE_REWRITE(*this, TinyError::post(home(*this), a, b, r, digits,
                                                  formats));
          bool changed = false;
          GECODE_ES_CHECK(filter(home, a, b, r, digits, formats, changed));
          if (!changed)
            return ES_FIX;
        }
      }
    };
  } // namespace
} // namespace Gecode

// STATISTICS: word-prop
