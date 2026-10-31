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

#include "test/word.hh"
#include <array>
#include <gecode/float-word.hh>
#include <gecode/word/arithmetic/global-integer.hpp>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace Test {
  namespace Word {
    namespace Float {
      namespace {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        using Integer = Gecode::Word::Arithmetic::Global::Integer;

        struct Ratio {
          Integer numerator, denominator;
        };
        Ratio dyadic(WordValue significand, int exponent) {
          return exponent < 0
                     ? Ratio{Integer(significand),
                             Integer::power_two(
                                 static_cast<unsigned int>(-exponent))}
                     : Ratio{Integer(significand) *
                                 Integer::power_two(
                                     static_cast<unsigned int>(exponent)),
                             Integer(1)};
        }
        int compare_ratio(const Ratio& a, const Ratio& b) {
          return compare(a.numerator * b.denominator,
                         b.numerator * a.denominator);
        }
        struct Expected {
          WordValue bits;
          int flags;
        };

        struct Format {
          FloatWordFormat id;
          unsigned int fraction;
          WordValue sign, normal, infinity, maximum, quiet, one;
          explicit Format(FloatWordFormat f)
              : id(f), fraction(f == FWF_BF16 ? 7 : 23),
                sign(WordValue(1) << (fraction + 8)),
                normal(WordValue(1) << fraction), infinity(255 * normal),
                maximum(infinity - 1), quiet(normal / 2), one(127 * normal) {}
          Ratio decode(WordValue magnitude) const {
            const int exponent = static_cast<int>(magnitude / normal);
            return dyadic(exponent ? normal + magnitude % normal : magnitude,
                          (exponent ? exponent - 127 : -126) -
                              static_cast<int>(fraction));
          }
          bool nan(WordValue code) const { return code % sign > infinity; }
        };

        // Decode raw significands/exponents into signed rationals, form the
        // exact product plus addend, and binary-search destination neighbors.
        // Only Integer arithmetic is shared with production.
        struct Reference {
          const Format source, destination;
          Reference(FloatWordFormat s, FloatWordFormat d)
              : source(s), destination(d) {}
          bool choose_high(const Ratio& exact, const Ratio& low,
                           const Ratio& high, WordValue low_code, bool negative,
                           Policy::Rounding rounding) const {
            if (compare_ratio(exact, low) == 0)
              return false;
            if (rounding == Policy::RNE) {
              const Ratio midpoint{low.numerator * high.denominator +
                                       high.numerator * low.denominator,
                                   Integer(2) * low.denominator *
                                       high.denominator};
              const int relation = compare_ratio(exact, midpoint);
              return relation > 0 || (relation == 0 && (low_code & 1));
            }
            return (rounding == Policy::RUP && !negative) ||
                   (rounding == Policy::RDN && negative);
          }
          Expected evaluate(const Policy& p, WordValue a, WordValue b,
                            WordValue c) const {
            WordValue am = a % source.sign, bm = b % source.sign;
            WordValue cm = c % destination.sign;
            assert(am < source.infinity && bm < source.infinity &&
                   cm < destination.infinity);
            if (p.input_subnormals == Policy::SignedZero) {
              if (am < source.normal)
                am = 0;
              if (bm < source.normal)
                bm = 0;
              if (cm < destination.normal)
                cm = 0;
            }
            const bool product_negative =
                (a >= source.sign) != (b >= source.sign);
            const bool addend_negative = c >= destination.sign;
            const Ratio av = source.decode(am), bv = source.decode(bm);
            const Ratio cv = destination.decode(cm);
            Integer product = av.numerator * bv.numerator * cv.denominator;
            Integer addend = cv.numerator * av.denominator * bv.denominator;
            if (product_negative)
              product = -product;
            if (addend_negative)
              addend = -addend;
            const Integer sum = product + addend;
            const bool negative = sum.zero()
                                      ? (product_negative == addend_negative
                                             ? product_negative
                                             : p.rounding == Policy::RDN)
                                      : sum.neg();
            if (sum.zero())
              return {negative && p.zeros == Policy::Signed ? destination.sign
                                                            : 0,
                      0};
            const Ratio exact{sum.abs(),
                              av.denominator * bv.denominator * cv.denominator};
            int low_exponent = -300, high_exponent = 256;
            while (low_exponent < high_exponent) {
              const int middle =
                  low_exponent + (high_exponent - low_exponent + 1) / 2;
              if (compare_ratio(dyadic(1, middle), exact) <= 0)
                low_exponent = middle;
              else
                high_exponent = middle - 1;
            }
            const int grid =
                low_exponent - static_cast<int>(destination.fraction);
            WordValue low = destination.normal,
                      high = destination.normal * 2 - 1;
            while (low < high) {
              const WordValue middle = (low + high + 1) / 2;
              if (compare_ratio(dyadic(middle, grid), exact) <= 0)
                low = middle;
              else
                high = middle - 1;
            }
            const Ratio precision =
                dyadic(low + choose_high(exact, dyadic(low, grid),
                                         dyadic(low + 1, grid), low, negative,
                                         p.rounding),
                       grid);
            if (compare_ratio(precision,
                              destination.decode(destination.maximum)) > 0) {
              const bool infinite = p.overflow == Policy::Format &&
                                    (p.rounding == Policy::RNE ||
                                     (p.rounding == Policy::RUP && !negative) ||
                                     (p.rounding == Policy::RDN && negative));
              return {(infinite ? destination.infinity : destination.maximum) +
                          (negative ? destination.sign : 0),
                      20};
            }
            low = 0;
            high = destination.maximum;
            while (low < high) {
              const WordValue middle = (low + high + 1) / 2;
              if (compare_ratio(destination.decode(middle), exact) <= 0)
                low = middle;
              else
                high = middle - 1;
            }
            WordValue chosen = low;
            if (low < destination.maximum &&
                choose_high(exact, destination.decode(low),
                            destination.decode(low + 1), low, negative,
                            p.rounding))
              ++chosen;
            int flags =
                compare_ratio(exact, destination.decode(chosen)) == 0 ? 0 : 16;
            const bool precision_tiny =
                compare_ratio(precision,
                              destination.decode(destination.normal)) < 0;
            const bool tiny =
                p.tininess == Policy::BeforeRounding
                    ? compare_ratio(exact,
                                    destination.decode(destination.normal)) < 0
                    : precision_tiny;
            if (tiny && flags)
              flags |= 8;
            if (p.output_subnormals == Policy::SignedZero && chosen > 0 &&
                chosen < destination.normal) {
              chosen = 0;
              flags |= 24;
            }
            return {chosen + (negative && (chosen || p.zeros == Policy::Signed)
                                  ? destination.sign
                                  : 0),
                    flags};
          }
        };

        class FmaSpace : public Space {
        public:
          FloatWordVar x[4];
          IntVar flags;
          explicit FmaSpace(const Reference& ref,
                            WordDomainType kind = WDT_UNSIGNED,
                            bool mixed_storage = false)
              : flags(*this, 0, 31) {
            for (int i = 0; i < 4; ++i)
              x[i] = FloatWordVar(*this,
                                  i < 2 ? ref.source.id : ref.destination.id,
                                  mixed_storage && (i & 1) ? WDT_SIGNED : kind);
          }
          FmaSpace(FmaSpace& other) : Space(other) {
            for (int i = 0; i < 4; ++i)
              x[i].update(*this, other.x[i]);
            flags.update(*this, other.flags);
          }
          Space* copy() override { return new FmaSpace(*this); }
          void post(const Policy& p, bool status = true) {
            if (status)
              fp_fma(*this, p, x[0], x[1], x[2], x[3], flags);
            else
              fp_fma(*this, p, x[0], x[1], x[2], x[3]);
          }
        };

        bool check_expected(const Reference& ref, const Policy& p, WordValue a,
                            WordValue b, WordValue c, Expected expected,
                            bool status = true) {
          FmaSpace s(ref);
          const WordValue inputs[] = {a, b, c};
          for (int i = 0; i < 3; ++i)
            dom(s, s.x[i].bits(), inputs[i]);
          s.post(p, status);
          if (s.status() == SS_FAILED)
            return false;
          if (!s.x[3].bits().assigned() ||
              s.x[3].bits().val() != expected.bits ||
              (status &&
               (!s.flags.assigned() || s.flags.val() != expected.flags))) {
            std::cerr << "FMA exact mismatch " << ref.source.id << '/'
                      << ref.destination.id << ' ' << p.rounding << ' '
                      << p.input_subnormals << ' ' << p.output_subnormals << ' '
                      << p.zeros << ' ' << p.tininess << std::hex << " inputs "
                      << a << ' ' << b << ' ' << c << " expected "
                      << expected.bits << std::dec << '/' << expected.flags
                      << '\n';
            return false;
          }
          return true;
        }
        bool check_finite(const Reference& ref, const Policy& p, WordValue a,
                          WordValue b, WordValue c, bool status = true) {
          return check_expected(ref, p, a, b, c, ref.evaluate(p, a, b, c),
                                status);
        }

        const std::array<FloatWordFormat, 2> signatures[] = {
            {FWF_BF16, FWF_BF16}, {FWF_FP32, FWF_FP32}, {FWF_BF16, FWF_FP32}};

        class Exact : public Base {
        public:
          Exact() : Base("Word::Float::Fma::Wide::Exact") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const std::array<WordValue, 3> catalogue[] = {
                  {s.one + 1, s.one - 2, d.sign + d.one},
                  {1, 1, 0},
                  {s.normal, s.one - 1, 0},
                  {s.normal + 1, s.one - 2, d.sign + 1},
                  {s.maximum, s.one + s.normal, d.sign + d.maximum},
                  {s.maximum, s.maximum, 0},
                  {s.sign, s.one, d.sign},
                  {s.sign + 1, s.one - s.normal, 1}};
              for (int r = 0; r < 4; ++r)
                for (int in = 0; in < 2; ++in)
                  for (int out = 0; out < 2; ++out)
                    for (int ov = 0; ov < 2; ++ov)
                      for (int zero = 0; zero < 2; ++zero)
                        for (int nan = 0; nan < 3; ++nan)
                          for (int tiny = 0; tiny < 2; ++tiny)
                            for (int cmp = 0; cmp < 2; ++cmp) {
                              const Policy p{
                                  static_cast<Policy::Rounding>(r),
                                  static_cast<Policy::Subnormals>(in),
                                  static_cast<Policy::Subnormals>(out),
                                  static_cast<Policy::Overflow>(ov),
                                  static_cast<Policy::Zeros>(zero),
                                  static_cast<Policy::Nan>(nan),
                                  static_cast<Policy::Tininess>(tiny),
                                  static_cast<Policy::Comparison>(cmp)};
                              for (const auto& v : catalogue)
                                if (!check_finite(ref, p, v[0], v[1], v[2]))
                                  return false;
                            }
              // Signed midpoint parity, gradual cancellation, rounded/exact
              // zeros and minimum products under all four directions.
              for (int r = 0; r < 4; ++r) {
                const Policy p{static_cast<Policy::Rounding>(r)};
                for (WordValue sign : {WordValue(0), s.sign})
                  for (WordValue small : {WordValue(1), WordValue(3),
                                          s.normal - 1, s.normal, s.normal + 1})
                    for (WordValue c : {WordValue(0), WordValue(1), d.sign + 1,
                                        d.normal, d.sign + d.normal})
                      if (!check_finite(ref, p, sign + small, s.one - s.normal,
                                        c))
                        return false;
                for (const auto& v : catalogue)
                  if (!check_finite(ref, p, v[0], v[1], v[2], false))
                    return false;
              }
              const WordValue witness = signature[0] == FWF_FP32   ? 0xa8800000
                                        : signature[1] == FWF_BF16 ? 0xb880
                                                                   : 0xb8800000;
              const auto exact =
                  ref.evaluate(Policy(), s.one + 1, s.one - 2, d.sign + d.one);
              if (exact.bits != witness || exact.flags != 0 ||
                  !check_expected(ref, Policy(), s.one + 1, s.one - 2,
                                  d.sign + d.one, {witness, 0}))
                return false;
              if (s.id == d.id) {
                for (int r = 0; r < 4; ++r)
                  if (!check_expected(ref,
                                      Policy{static_cast<Policy::Rounding>(r)},
                                      s.maximum, s.one + s.normal,
                                      d.sign + d.maximum, {d.maximum, 0}))
                    return false;
              }
              if (s.id == FWF_FP32) {
                const Policy scalar_flush{Policy::RNE,
                                          Policy::Preserve, Policy::SignedZero};
                if (!check_expected(ref, scalar_flush, 0x00800000, 0x3f7fffff,
                                    0, {0x00800000, 24}))
                  return false;
              }
              if (s.id != d.id)
                for (int tiny = 0; tiny < 2; ++tiny) {
                  const Policy p{
                      Policy::RNE, Policy::Preserve,
                      Policy::Preserve, Policy::Format, Policy::Signed,
                      Policy::SignalFirst, static_cast<Policy::Tininess>(tiny)};
                  if (!check_expected(ref, p, 0x8001, 0x3380, 0x00800000,
                                      {0x00800000, tiny ? 24 : 16}))
                    return false;
                }
            }
            return true;
          }
        };
        class Exceptions : public Base {
        public:
          Exceptions() : Base("Word::Float::Fma::Wide::Exceptions") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const WordValue sq = s.sign + s.infinity + s.quiet + 3;
              const WordValue ss = s.infinity + 5;
              const WordValue dq = d.infinity + d.quiet + 7;
              const WordValue ds = d.sign + d.infinity + 9;
              const unsigned int shift = s.id == d.id ? 0 : 16;
              for (int mode = 0; mode < 3; ++mode) {
                const Policy p{Policy::RNE, Policy::Preserve,
                               Policy::Preserve, Policy::Format, Policy::Signed,
                               static_cast<Policy::Nan>(mode)};
                const WordValue selected =
                    p.nan == Policy::Canonical     ? d.infinity + d.quiet
                    : p.nan == Policy::SignalFirst ? (ss << shift) | d.quiet
                                                   : sq << shift;
                if (!check_expected(ref, p, sq, ss, ds, {selected, 1}) ||
                    !check_expected(
                        ref, p, s.one, s.one, dq,
                        {p.nan == Policy::Canonical ? d.infinity + d.quiet : dq,
                         0}))
                  return false;
                for (int position = 0; position < 3; ++position)
                  for (bool signaling : {false, true}) {
                    WordValue codes[] = {s.one, s.one, d.one};
                    const WordValue code = position == 2
                                               ? (signaling ? ds : dq)
                                               : (signaling ? ss : sq);
                    codes[position] = code;
                    const WordValue result =
                        p.nan == Policy::Canonical
                            ? d.infinity + d.quiet
                            : (position == 2 ? code : code << shift) | d.quiet;
                    if (!check_expected(ref, p, codes[0], codes[1], codes[2],
                                        {result, signaling ? 1 : 0}))
                      return false;
                  }
              }
              for (bool flush : {false, true}) {
                const Policy p{Policy::RNE,
                               flush ? Policy::SignedZero : Policy::Preserve};
                if (!check_expected(ref, p, 0, s.infinity, dq, {dq, 1}) ||
                    !check_expected(ref, p, 1, s.infinity, dq,
                                    {dq, flush ? 1 : 0}) ||
                    !check_expected(ref, p, s.infinity, flush ? 1 : 0, d.one,
                                    {d.infinity + d.quiet, 1}))
                  return false;
              }
              const Policy sat{Policy::RNE, Policy::Preserve,
                               Policy::Preserve, Policy::SaturateFinite};
              if (!check_expected(ref, sat, s.infinity, s.one,
                                  d.sign + d.infinity,
                                  {d.infinity + d.quiet, 1}) ||
                  !check_expected(ref, sat, s.sign + s.infinity, s.one, d.one,
                                  {d.sign + d.infinity, 0}) ||
                  !check_expected(ref, sat, s.one, s.one, d.infinity,
                                  {d.infinity, 0}))
                return false;
            }
            return true;
          }
        };

        void bounds(Space& s, WordVar word, WordValue low, WordValue high) {
          Gecode::Word::WordView view(word);
          if (view.bounded()) {
            if (me_failed(view.narrow_rank_range(
                    s,
                    Gecode::Word::rank(view.domain_type(), view.width(), low),
                    Gecode::Word::rank(view.domain_type(), view.width(),
                                       high))))
              s.fail();
          } else {
            rel(s, word, WRT_UGQ, low);
            rel(s, word, WRT_ULQ, high);
          }
        }
        struct Window {
          WordValue low, high;
        };
        Window fixed_window(WordValue code) {
          return {code, code};
        }
        using Windows = std::array<Window, 4>;
        using Row = std::array<WordValue, 5>;
        using Rows = std::set<Row>;
        enum class Alias {
          Independent,
          RepeatedMultiplicands,
          RepeatedAddend,
          ResultIsFirst,
          ResultIsSecond,
          ResultIsSquare,
          ResultIsFirstAndAddend,
          ResultIsAddend,
          RepeatedInputs
        };
        // The column whose value each column repeats; free columns map to
        // themselves.
        std::array<int, 4> origins(Alias alias) {
          switch (alias) {
          case Alias::Independent:
            return {0, 1, 2, 3};
          case Alias::RepeatedMultiplicands:
            return {0, 0, 2, 3};
          case Alias::RepeatedAddend:
            return {0, 1, 0, 3};
          case Alias::ResultIsFirst:
            return {0, 1, 2, 0};
          case Alias::ResultIsSecond:
            return {0, 1, 2, 1};
          case Alias::ResultIsSquare:
            return {0, 0, 2, 0};
          case Alias::ResultIsFirstAndAddend:
            return {0, 1, 0, 0};
          case Alias::ResultIsAddend:
            return {0, 1, 2, 2};
          case Alias::RepeatedInputs:
            return {0, 0, 0, 3};
          }
          GECODE_NEVER;
          return {};
        }
        // The lowest value of column i in a row: its window bound when free,
        // otherwise the value of the earlier column it repeats.
        WordValue column_low(const Windows& windows,
                             const std::array<int, 4>& origin, const Row& row,
                             int i) {
          return origin[i] == i ? windows[i].low : row[origin[i]];
        }
        // The highest value of column i in a row, as for column_low.
        WordValue column_high(const Windows& windows,
                              const std::array<int, 4>& origin, const Row& row,
                              int i) {
          return origin[i] == i ? windows[i].high : row[origin[i]];
        }
        // Enumerate only free input columns: square alias windows would
        // otherwise iterate the cube of an alias window.
        Rows expected_rows(const Reference& ref, const Policy& p,
                           const Windows& windows, Alias alias, int status) {
          const auto origin = origins(alias);
          Row row{};
          Rows expected;
          for (row[0] = windows[0].low; row[0] <= windows[0].high; ++row[0])
            for (row[1] = column_low(windows, origin, row, 1);
                 row[1] <= column_high(windows, origin, row, 1); ++row[1])
              for (row[2] = column_low(windows, origin, row, 2);
                   row[2] <= column_high(windows, origin, row, 2); ++row[2]) {
                const auto result = ref.evaluate(p, row[0], row[1], row[2]);
                if ((origin[3] == 3 || result.bits == row[origin[3]]) &&
                    result.bits >= windows[3].low &&
                    result.bits <= windows[3].high &&
                    (status < 0 || status == result.flags))
                  expected.insert({row[0], row[1], row[2], result.bits,
                                   WordValue(result.flags)});
              }
          return expected;
        }
        void configure(FmaSpace& s, const Windows& windows, Alias alias,
                       int status = -1) {
          const auto origin = origins(alias);
          for (int i = 1; i < 4; ++i)
            s.x[i] = s.x[origin[i]];
          for (int i = 0; i < 4; ++i)
            if (windows[i].low != 0 || windows[i].high != s.x[i].bits().mask())
              bounds(s, s.x[i].bits(), windows[i].low, windows[i].high);
          if (status >= 0)
            rel(s, s.flags, IRT_EQ, status);
        }
        bool preserves(FmaSpace& s, const Rows& expected) {
          if (s.status() == SS_FAILED)
            return expected.empty();
          for (const auto& row : expected) {
            for (int i = 0; i < 4; ++i)
              if (!Gecode::Word::WordView(s.x[i].bits()).in(row[i]))
                return false;
            if (!s.flags.in(static_cast<int>(row[4])))
              return false;
          }
          return true;
        }
        void branching(FmaSpace& s) {
          branch(s,
                 WordVarArgs{s.x[0].bits(), s.x[1].bits(), s.x[2].bits(),
                             s.x[3].bits()},
                 WORD_VAL_LSB());
          branch(s, s.flags, INT_VAL_MIN());
        }
        Rows solutions(FmaSpace& s, unsigned int recompute = 8) {
          Rows found;
          Search::Options options;
          options.c_d = recompute;
          options.a_d = recompute;
          DFS<FmaSpace> search(&s, options);
          while (std::unique_ptr<FmaSpace> next{search.next()})
            found.insert({next->x[0].bits().val(), next->x[1].bits().val(),
                          next->x[2].bits().val(), next->x[3].bits().val(),
                          WordValue(next->flags.val())});
          return found;
        }
        bool check_window(const Reference& ref, const Policy& p,
                          WordDomainType kind, const Windows& windows,
                          int prune = -1, Alias alias = Alias::Independent,
                          int status = -1, bool mixed_storage = false) {
          const Rows expected = expected_rows(ref, p, windows, alias, status);
          FmaSpace s(ref, kind, mixed_storage);
          configure(s, windows, alias, status);
          for (int i = 0; i < 3; ++i)
            if (windows[i].low != windows[i].high && s.x[i].bits().assigned())
              return false;
          s.post(p);
          if (!preserves(s, expected)) {
            std::cerr << "FMA lost root support " << ref.source.id << '/'
                      << ref.destination.id << '\n';
            return false;
          }
          if (s.status() == SS_FAILED)
            return expected.empty();
          if (prune >= 0) {
            const auto view = Gecode::Word::WordView(s.x[prune].bits());
            if (view.in(windows[prune].low) && view.in(windows[prune].high)) {
              std::cerr << "FMA did not prune " << ref.source.id << '/'
                        << ref.destination.id << ' ' << kind << ' ' << prune
                        << '\n';
              return false;
            }
          }
          branching(s);
          const auto found = solutions(s);
          if (found != expected) {
            std::cerr << "FMA tuples " << ref.source.id << '/'
                      << ref.destination.id << ' ' << kind << ' ' << static_cast<int>(alias)
                      << ' ' << found.size() << '/' << expected.size() << '\n';
            return false;
          }
          return true;
        }
        class Symbolic : public Base {
        public:
          Symbolic() : Base("Word::Float::Fma::Wide::Symbolic") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const bool mixed = s.id != d.id;
              const WordValue witness = s.id == FWF_FP32 ? 0xa8800000
                                        : mixed          ? 0xb8800000
                                                         : 0xb880;
              const WordValue inverse = mixed ? 0x40008000 : d.one + d.normal;
              const Windows cancellation{
                  {{s.one - 1, s.one + 3},
                   {s.one - 4, s.one},
                   {d.sign + d.one - 2, d.sign + d.one + 2},
                   {witness, witness}}};
              const Windows addend{{{s.one, s.one + 3},
                                    {s.one, s.one + 3},
                                    {d.one, d.one + 15},
                                    {inverse, inverse + 2}}};
              const Windows multiplicand{{{s.one, s.one + 31},
                                          {s.one, s.one + 3},
                                          {d.one, d.one + 3},
                                          {inverse + 1, inverse + 2}}};
              const Window all{0, 2 * d.sign - 1};
              for (WordDomainType kind : {WDT_UNSIGNED, WDT_SIGNED, WDT_CUBE}) {
                if (!check_window(ref, Policy(), kind, cancellation,
                                  kind == WDT_CUBE ? -1 : 0) ||
                    !check_window(ref, Policy(), kind, addend, 2) ||
                    !check_window(ref, Policy(), kind, multiplicand, 0) ||
                    !check_window(ref, Policy(), kind,
                                  {{{s.one, s.one + 3},
                                    {s.one, s.one + 3},
                                    {d.one, d.one + 3},
                                    all}},
                                  3) ||
                    !check_window(ref, Policy(), kind, addend, -1, Alias::Independent,
                                  -1, true))
                  return false;
                if (kind == WDT_CUBE &&
                    !check_window(ref, Policy(), kind,
                                  {{{s.one, s.one + 3},
                                    {s.one - 4, s.one - 1},
                                    {d.sign + d.one, d.sign + d.one + 3},
                                    {witness, witness}}},
                                  0))
                  return false;
                for (int r = 0; r < 4; ++r) {
                  const Policy p{static_cast<Policy::Rounding>(r)};
                  for (Alias alias :
                       {Alias::RepeatedMultiplicands, Alias::RepeatedAddend, Alias::ResultIsFirst,
                        Alias::ResultIsAddend, Alias::RepeatedInputs}) {
                    if (mixed && alias != Alias::RepeatedMultiplicands &&
                        alias != Alias::ResultIsAddend)
                      continue;
                    // Small exact cubes exercise alias identity, including
                    // result aliases whose supported code differs by tuple.
                    const Windows q{
                        {{0, 3},
                         {0, 3},
                         {0, 3},
                         alias == Alias::ResultIsFirst || alias == Alias::ResultIsAddend
                             ? Window{0, 3}
                             : all}};
                    if (!check_window(ref, p, kind, q, -1, alias))
                      return false;
                  }
                  for (bool flush : {false, true}) {
                    const Policy pflush{
                        static_cast<Policy::Rounding>(r),
                        flush ? Policy::SignedZero : Policy::Preserve,
                        Policy::SignedZero};
                    if (!check_window(ref, pflush, kind,
                                      {{{s.normal - 1, s.normal + 2},
                                        {s.one - 1, s.one + 1},
                                        {d.sign + 1, d.sign + 3},
                                        all}}))
                      return false;
                  }
                }
              }
              // Restricting result and flags independently must not create a
              // tuple absent from the exact relation.
              const Windows q{{{s.one, s.one + 3},
                               {s.one, s.one + 3},
                               {d.one, d.one + 15},
                               {inverse, inverse + 2}}};
              for (int flags : {0, 16, 24})
                if (!check_window(ref, Policy(), WDT_UNSIGNED, q, -1,
                                  Alias::Independent, flags))
                  return false;
            }
            // Both marginal result/flag choices have support, but their
            // crossed pair does not belong to the relation.
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& a = ref.source;
              const auto& z = ref.destination;
              FmaSpace correlated(ref);
              configure(correlated,
                        {{{a.one, a.one},
                          {a.one, a.one},
                          {z.one + 1, z.one + 2},
                          {z.one + z.normal, z.one + z.normal + 1}}},
                        Alias::Independent);
              correlated.post(Policy());
              if (correlated.status() == SS_FAILED || !correlated.flags.in(0) ||
                  !correlated.flags.in(16) ||
                  !Gecode::Word::WordView(correlated.x[3].bits())
                       .in(z.one + z.normal + 1))
                return false;
              dom(correlated, correlated.x[3].bits(), z.one + z.normal + 1);
              rel(correlated, correlated.flags, IRT_EQ, 16);
              if (correlated.status() != SS_FAILED)
                return false;
            }
            return true;
          }
        };
        std::array<WordValue, 17> snapshot(FmaSpace& s) {
          std::array<WordValue, 17> result{};
          for (int i = 0; i < 4; ++i) {
            const Gecode::Word::WordView view(s.x[i].bits());
            result[4 * i] = view.lo();
            result[4 * i + 1] = view.hi();
            result[4 * i + 2] =
                view.bounded() ? view.rank_minimum() : view.lo();
            result[4 * i + 3] =
                view.bounded() ? view.rank_maximum() : view.hi();
          }
          for (IntVarValues flags(s.flags); flags(); ++flags)
            result[16] |= WordValue(1) << flags.val();
          return result;
        }
        void restrict_later(FmaSpace& s, const Reference& ref) {
          dom(s, s.x[0].bits(), WordValue(0), s.x[0].bits().mask() ^ 1);
          bounds(s, s.x[1].bits(), ref.source.one + 1, ref.source.one + 3);
          rel(s, s.flags, IRT_NQ, 16);
        }
        class Lifecycle : public Base {
        public:
          Lifecycle() : Base("Word::Float::Fma::Wide::Lifecycle") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const WordValue inverse =
                  s.id != d.id ? 0x40008000 : d.one + d.normal;
              const Windows small{{{s.one, s.one + 3},
                                   {s.one, s.one + 3},
                                   {d.one, d.one + 15},
                                   {inverse, inverse + 2}}};
              const Rows expected =
                  expected_rows(ref, Policy(), small, Alias::Independent, -1);
              Rows restricted;
              for (const auto& row : expected)
                if (!(row[0] & 1) && row[1] >= s.one + 1 && row[4] != 16)
                  restricted.insert(row);
              for (WordDomainType kind : {WDT_UNSIGNED, WDT_SIGNED, WDT_CUBE}) {
                FmaSpace original(ref, kind);
                configure(original, small, Alias::Independent);
                original.post(Policy());
                branching(original);
                if (!preserves(original, expected))
                  return false;
                const auto before = snapshot(original);
                std::unique_ptr<FmaSpace> clone(
                    static_cast<FmaSpace*>(original.clone()));
                restrict_later(*clone, ref);
                FmaSpace fresh(ref, kind);
                configure(fresh, small, Alias::Independent);
                restrict_later(fresh, ref);
                fresh.post(Policy());
                branching(fresh);
                if (!preserves(*clone, restricted) ||
                    !preserves(fresh, restricted) ||
                    solutions(*clone, 1) != restricted ||
                    solutions(fresh, 16) != restricted ||
                    original.status() == SS_FAILED ||
                    snapshot(original) != before ||
                    solutions(original) != expected)
                  return false;
              }
              // Above the input-assignment scan cap: endpoint projection must
              // narrow direct and inverse domains without scanning triples.
              for (WordDomainType kind : {WDT_UNSIGNED, WDT_SIGNED}) {
                const Windows broad{{{s.one, s.one + s.normal},
                                     {s.one, s.one + s.normal},
                                     {d.one, d.one + d.normal},
                                     {0, 2 * d.sign - 1}}};
                FmaSpace direct(ref, kind, true);
                configure(direct, broad, Alias::Independent);
                direct.post(Policy());
                if (direct.status() == SS_FAILED ||
                    Gecode::Word::WordView(direct.x[3].bits()).in(0) ||
                    !Gecode::Word::WordView(direct.x[3].bits())
                         .in(d.one + d.normal))
                  return false;
                FmaSpace inverse_model(ref, kind, true);
                configure(inverse_model, broad, Alias::Independent);
                dom(inverse_model, inverse_model.x[3].bits(), d.one + d.normal);
                inverse_model.post(Policy());
                if (inverse_model.status() == SS_FAILED)
                  return false;
                for (int i = 0; i < 3; ++i) {
                  const auto& f = i < 2 ? s : d;
                  const auto view =
                      Gecode::Word::WordView(inverse_model.x[i].bits());
                  if (!view.in(f.one) || view.in(f.one + f.normal))
                    return false;
                }
                // Keep all inputs broad through at least 70 externally
                // scheduled finite projections, then restrict another broad
                // input.
                FmaSpace aged(ref, kind);
                configure(aged, broad, Alias::Independent);
                aged.post(Policy());
                for (WordValue k = 0; k < 70; ++k) {
                  bounds(aged, aged.x[0].bits(), s.one, s.one + s.normal - k);
                  if (aged.status() == SS_FAILED)
                    return false;
                }
                std::unique_ptr<FmaSpace> aged_clone(
                    static_cast<FmaSpace*>(aged.clone()));
                // Both aged actors must still respond above the scan cap.
                // Even BF16 retains 60*65*129 distinct input assignments.
                for (FmaSpace* model : {&aged, aged_clone.get()}) {
                  bounds(*model, model->x[1].bits(), s.one,
                         s.one + s.normal / 2);
                  const auto upper =
                      ref.evaluate(Policy(), s.one + s.normal - 69,
                                   s.one + s.normal / 2, d.one + d.normal);
                  if (model->status() == SS_FAILED ||
                      Gecode::Word::WordView(model->x[3].bits())
                              .rank_maximum() !=
                          Gecode::Word::rank(kind, model->x[3].bits().width(),
                                             upper.bits) ||
                      model->x[0].bits().assigned() ||
                      model->x[1].bits().assigned() ||
                      model->x[2].bits().assigned())
                    return false;
                  const auto stable = snapshot(*model);
                  std::unique_ptr<FmaSpace> closure(
                      static_cast<FmaSpace*>(model->clone()));
                  closure->post(Policy());
                  if (closure->status() == SS_FAILED ||
                      snapshot(*closure) != stable)
                    return false;
                }
                const auto before_scan = snapshot(aged);
                const Windows after{{{s.one, s.one + 3},
                                     {s.one, s.one + 3},
                                     {d.one, d.one + 3},
                                     {d.one + d.normal, d.one + d.normal + 2}}};
                configure(*aged_clone, after, Alias::Independent);
                const Rows supports =
                    expected_rows(ref, Policy(), after, Alias::Independent, -1);
                if (supports.empty() || !preserves(*aged_clone, supports) ||
                    aged.status() == SS_FAILED || snapshot(aged) != before_scan)
                  return false;
                branching(*aged_clone);
                if (solutions(*aged_clone) != supports)
                  return false;
                // Ground enforcement also survives the delayed restrictions.
                std::unique_ptr<FmaSpace> wrong(
                    static_cast<FmaSpace*>(aged.clone()));
                for (int i = 0; i < 3; ++i)
                  dom(*wrong, wrong->x[i].bits(), i < 2 ? s.one : d.one);
                dom(*wrong, wrong->x[3].bits(), d.one + d.normal + 1);
                if (wrong->status() != SS_FAILED)
                  return false;
                for (WordValue as : {WordValue(0), s.sign})
                  for (WordValue bs : {WordValue(0), s.sign})
                    for (WordValue cs : {WordValue(0), d.sign}) {
                      FmaSpace signs(ref, kind);
                      const auto value = ref.evaluate(Policy(), as + s.one,
                                                      bs + s.one, cs + d.one);
                      configure(signs,
                                {{{as + s.one, as + s.one + s.normal},
                                  {bs + s.one, bs + s.one + s.normal},
                                  {cs + d.one, cs + d.one + d.normal},
                                  {value.bits, value.bits}}},
                                Alias::Independent);
                      signs.post(Policy());
                      const Rows known{{as + s.one, bs + s.one, cs + d.one,
                                        value.bits, WordValue(value.flags)}};
                      if (!preserves(signs, known))
                        return false;
                    }
              }
            }
            // Remove the interior status value 16 while preserving min=0
            // and max=24. PC_INT_DOM must wake the actor and prune the result.
            for (FloatWordFormat format : {FWF_BF16, FWF_FP32}) {
              const Reference ref(format, format);
              const auto& f = ref.source;
              const Windows q{{{f.normal, f.normal + 1},
                               {f.one - 1, f.one + 1},
                               {0, 1},
                               {0, 2 * f.sign - 1}}};
              FmaSpace original(ref);
              configure(original, q, Alias::Independent);
              original.post(Policy());
              const Rows expected =
                  expected_rows(ref, Policy(), q, Alias::Independent, -1);
              if (!preserves(original, expected) || original.flags.min() != 0 ||
                  original.flags.max() != 24 || !original.flags.in(16) ||
                  !Gecode::Word::WordView(original.x[3].bits())
                       .in(f.normal + 3))
                return false;
              const auto before = snapshot(original);
              std::unique_ptr<FmaSpace> clone(
                  static_cast<FmaSpace*>(original.clone()));
              rel(*clone, clone->flags, IRT_NQ, 16);
              Rows restricted;
              for (const auto& row : expected)
                if (row[4] != 16)
                  restricted.insert(row);
              if (!preserves(*clone, restricted) || clone->flags.min() != 0 ||
                  clone->flags.max() != 24 ||
                  Gecode::Word::WordView(clone->x[3].bits()).in(f.normal + 3) ||
                  snapshot(original) != before)
                return false;
              branching(*clone);
              if (solutions(*clone) != restricted)
                return false;
            }
            return true;
          }
        };

        class Absorption : public Base {
        public:
          Absorption() : Base("Word::Float::Fma::Wide::Absorption") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& a = ref.source;
              const auto& c = ref.destination;
              const WordValue threshold = (128 + c.fraction) * c.normal;
              FmaSpace diagnostic(ref);
              configure(diagnostic,
                        {{{a.one, a.one},
                          {a.one, a.one},
                          {c.one, threshold},
                          {c.one, threshold}}},
                        Alias::ResultIsAddend);
              diagnostic.post(Policy());
              if (diagnostic.status() == SS_FAILED ||
                  !diagnostic.x[2].bits().assigned() ||
                  diagnostic.x[2].bits().val() != threshold ||
                  !diagnostic.flags.assigned() || diagnostic.flags.val() != 16)
                return false;
              for (int rounding = 0; rounding < 4; ++rounding)
                for (WordValue product_sign : {WordValue(0), a.sign})
                  for (WordValue addend_sign : {WordValue(0), c.sign})
                    for (WordValue boundary :
                         {threshold - c.normal, threshold}) {
                      const Policy policy{
                          static_cast<Policy::Rounding>(rounding)};
                      const Window addend{addend_sign + boundary - 2,
                                          addend_sign + boundary + 2};
                      const Windows small{
                          {{product_sign + a.one, product_sign + a.one + 2},
                           {a.one, a.one + 2},
                           addend,
                           addend}};
                      const Rows expected =
                          expected_rows(ref, policy, small, Alias::ResultIsAddend, -1);
                      Windows broad = small;
                      broad[0].high = product_sign + a.one + a.normal;
                      broad[1].high = a.one + a.normal;
                      FmaSpace original(ref);
                      configure(original, broad, Alias::ResultIsAddend);
                      original.post(policy);
                      // Above the scan cap, this exercises the residual
                      // projector before the complete bounded tuple check.
                      if (!preserves(original, expected))
                        return false;
                      if (original.failed())
                        continue;
                      const auto before = snapshot(original);
                      std::unique_ptr<FmaSpace> clone(
                          static_cast<FmaSpace*>(original.clone()));
                      clone->post(policy);
                      if (clone->status() == SS_FAILED ||
                          snapshot(*clone) != before)
                        return false;
                      configure(*clone, small, Alias::ResultIsAddend);
                      if (!preserves(*clone, expected) ||
                          snapshot(original) != before)
                        return false;
                      if (clone->failed())
                        continue;
                      branching(*clone);
                      if (solutions(*clone) != expected)
                        return false;
                    }
            }
            return true;
          }
        };

        // Outcome of propagating a fresh space to its first fixpoint.
        struct Settled {
          SpaceStatus status;
          // Propagator executions needed to reach the fixpoint.
          unsigned long long int propagations;
        };
        Settled settle(FmaSpace& s) {
          StatusStatistics statistics;
          const SpaceStatus status = s.status(statistics);
          return {status, statistics.propagate};
        }
        class MultiplicandAlias : public Base {
        public:
          MultiplicandAlias()
              : Base("Word::Float::Fma::Wide::MultiplicandAlias") {}
          bool run() override {
            // Above the support scan, convergence must not depend on the
            // number of values the projection has to rule out.
            static constexpr unsigned long long int bound = 8;
            const Reference ref(FWF_FP32, FWF_FP32);
            const WordValue one = ref.source.one, normal = ref.source.normal;
            const WordValue up = one + 1;
              const Window binade{one, one + normal - 1};
            // Rounding x*(1+2^-23) moves every normal x up by one ULP.
            for (Alias alias : {Alias::ResultIsFirst, Alias::ResultIsSecond}) {
              unsigned long long int counts[2];
              int count = 0;
              for (const Window& x : {binade, Window{one, one + 0xffff}}) {
                const Windows w = alias == Alias::ResultIsFirst
                                      ? Windows{{x, fixed_window(up), fixed_window(0), x}}
                                      : Windows{{fixed_window(up), x, fixed_window(0), x}};
                FmaSpace s(ref);
                configure(s, w, alias);
                s.post(Policy());
                const Settled settled = settle(s);
                if (settled.status != SS_FAILED || settled.propagations > bound)
                  return false;
                counts[count++] = settled.propagations;
              }
              if (counts[0] != counts[1])
                return false;
            }
            FmaSpace square(ref);
            configure(square, {{binade, binade, fixed_window(0), binade}},
                      Alias::ResultIsSquare);
            square.post(Policy());
            const Settled squared = settle(square);
            if (squared.status == SS_FAILED || squared.propagations > bound ||
                !square.x[0].bits().assigned() ||
                square.x[0].bits().val() != one)
              return false;
            FmaSpace absorbed(ref);
            configure(absorbed, {{binade, fixed_window(up), binade, binade}},
                      Alias::ResultIsFirstAndAddend);
            absorbed.post(Policy());
            const Settled settled = settle(absorbed);
            if (settled.status != SS_FAILED || settled.propagations > bound)
              return false;
            // Windows just above the 2,048-assignment scan cap, each with a
            // few solutions. Near the smallest normal they mix preserved or
            // flushed subnormals with the tie at the power of two. Under RNE
            // x*(1-2^-24) keeps subnormal x and the smallest normal; under
            // RTZ x*(1+2^-23) keeps subnormal x. x+x*b keeps normal x below
            // 1+2^-19 when b is just below half (RNE) or one (RTZ) ULP of 1.
            // x*x+2^-20 rounds to x only at 1-2^-20, and x*x+2^-126 only at
            // the smallest normal.
            const Policy flush{Policy::RNE, Policy::Preserve,
                               Policy::SignedZero};
            const Window low{normal - 16, normal + 2048};
            const Window near_one{one - 32, one + 2032};
            for (const Policy& p :
                 {Policy(), Policy{Policy::RTZ}, flush}) {
              const bool truncates = p.rounding == Policy::RTZ;
              const Window product = fixed_window(truncates ? up : one - 1);
              const Window absorbed_factor =
                  fixed_window(truncates ? 0x33ffffe0 : 0x337fffe0);
              const Windows square_windows =
                  p.output_subnormals == Policy::SignedZero
                      ? Windows{{low, low, fixed_window(normal), low}}
                      : Windows{
                            {near_one, near_one, fixed_window(0x35800000), near_one}};
              const std::pair<Alias, Windows> cases[] = {
                  {Alias::ResultIsFirst, {{low, product, fixed_window(0), low}}},
                  {Alias::ResultIsSecond, {{product, low, fixed_window(0), low}}},
                  {Alias::ResultIsSquare, square_windows},
                  {Alias::ResultIsFirstAndAddend, {{low, absorbed_factor, low, low}}}};
              for (const auto& c : cases)
                if (!check_window(ref, p, WDT_UNSIGNED, c.second, -1, c.first))
                  return false;
            }
            return true;
          }
        };

        class Posting : public Base {
        public:
          Posting() : Base("Word::Float::Fma::Wide::Posting") {}
          bool run() override {
            for (const auto& signature : signatures)
              for (bool status : {false, true}) {
                const Reference ref(signature[0], signature[1]);
                FmaSpace s(ref);
                s.post(Policy(), status);
                if (s.status() == SS_FAILED)
                  return false;
              }
            const Reference ref(FWF_FP32, FWF_FP32);
            for (int column = 0; column < 4; ++column) {
              FmaSpace s(ref);
              s.x[column] = FloatWordVar(s, FWF_BF16);
              bool caught = false;
              try {
                s.post(Policy());
              } catch (const Gecode::Word::FormatMismatch&) {
                caught = true;
              }
              if (!caught)
                return false;
            }
            FmaSpace status_range(ref);
            status_range.flags = IntVar(status_range, -10, 40);
            status_range.post(Policy());
            if (status_range.flags.min() != 0 || status_range.flags.max() != 31)
              return false;
            return true;
          }
        };
        Exact exact;
        Exceptions exceptions;
        Symbolic symbolic;
        Lifecycle lifecycle;
        Absorption absorption;
        MultiplicandAlias multiplicand_alias;
        Posting posting;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test
// STATISTICS: test-word
