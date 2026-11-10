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
#include <algorithm>
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

        // Independent rational neighbor oracle. Decode raw significands and
        // signed exponents; binary-search binades and neighboring encodings.
        // It shares only Integer's arithmetic with production, not decoding,
        // coefficient bit lengths, rounding grids or projection helpers.
        struct Reference {
          WordDomainType format;
          unsigned int fraction;
          WordValue sign, normal, infinity, maximum, quiet, one;
          explicit Reference(WordDomainType f)
              : format(f), fraction(f == WDT_BF16 ? 7 : 23),
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
          Expected evaluate(const Policy& p, WordValue a, WordValue b) const {
            const bool negative = (a >= sign) != (b >= sign);
            if (nan(a) || nan(b)) {
              const bool as = nan(a) && !(a & quiet),
                         bs = nan(b) && !(b & quiet);
              WordValue selected = nan(a) ? a : b;
              if (p.nan == Policy::SignalFirst && (as || bs))
                selected = as ? a : b;
              return {p.nan == Policy::Canonical ? infinity + quiet
                                                 : selected | quiet,
                      as || bs ? 1 : 0};
            }
            WordValue am = a % sign, bm = b % sign;
            if (p.input_subnormals == Policy::SignedZero) {
              if (am < normal)
                am = 0;
              if (bm < normal)
                bm = 0;
            }
            if ((am == 0 && bm == 0) || (am == infinity && bm == infinity))
              return {infinity + quiet,
                      1};
            if (am == infinity)
              return {infinity + (negative ? sign : 0), 0};
            if (bm == 0)
              return {infinity + (negative ? sign : 0), 2};
            if (am == 0 || bm == infinity)
              return {negative && p.zeros == Policy::Signed ? sign : 0, 0};
            const Ratio av = decode(am), bv = decode(bm);
            const Ratio exact{av.numerator * bv.denominator,
                              av.denominator * bv.numerator};
            int low_exponent = -300, high_exponent = 300;
            while (low_exponent < high_exponent) {
              const int middle =
                  low_exponent + (high_exponent - low_exponent + 1) / 2;
              if (compare_ratio(dyadic(1, middle), exact) <= 0)
                low_exponent = middle;
              else
                high_exponent = middle - 1;
            }
            const int grid = low_exponent - static_cast<int>(fraction);
            WordValue low = normal, high = normal * 2 - 1;
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
            if (compare_ratio(precision, decode(maximum)) > 0) {
              const bool infinite = p.overflow == Policy::Format &&
                                    (p.rounding == Policy::RNE ||
                                     (p.rounding == Policy::RUP && !negative) ||
                                     (p.rounding == Policy::RDN && negative));
              return {(infinite ? infinity : maximum) + (negative ? sign : 0),
                      20};
            }
            low = 0;
            high = maximum;
            while (low < high) {
              const WordValue middle = (low + high + 1) / 2;
              if (compare_ratio(decode(middle), exact) <= 0)
                low = middle;
              else
                high = middle - 1;
            }
            WordValue chosen = low;
            if (low < maximum &&
                choose_high(exact, decode(low), decode(low + 1), low, negative,
                            p.rounding))
              ++chosen;
            int flags = compare_ratio(exact, decode(chosen)) == 0 ? 0 : 16;
            const bool precision_tiny =
                compare_ratio(precision, decode(normal)) < 0;
            const bool tiny = p.tininess == Policy::BeforeRounding
                                  ? compare_ratio(exact, decode(normal)) < 0
                                  : precision_tiny;
            if (tiny && flags)
              flags |= 8;
            if (p.output_subnormals == Policy::SignedZero && chosen > 0 &&
                chosen < normal) {
              chosen = 0;
              flags |= 24;
            }
            return {chosen + (negative && (chosen || p.zeros == Policy::Signed)
                                  ? sign
                                  : 0),
                    flags};
          }
        };

        class Pair : public Space {
        public:
          WordVar x[3];
          IntVar flags;
          explicit Pair(WordDomainType f) : flags(*this, 0, 31) {
            for (auto& v : x)
              v = WordVar(*this, f);
          }
          Pair(Pair& other) : Space(other) {
            for (int i = 0; i < 3; ++i)
              x[i].update(*this, other.x[i]);
            flags.update(*this, other.flags);
          }
          Space* copy() override { return new Pair(*this); }
          void post(const Policy& p, bool with_status = true) {
            if (with_status)
              fp_divide(*this, x[0], x[1], x[2], flags, p);
            else
              fp_divide(*this, x[0], x[1], x[2], p);
          }
        };

        bool check_exact(const Reference& ref, const Policy& p, WordValue a,
                         WordValue b, bool with_status = true) {
          const Expected expected = ref.evaluate(p, a, b);
          Pair s(ref.format);
          dom(s, s.x[0], a);
          dom(s, s.x[1], b);
          s.post(p, with_status);
          if (s.status() == SS_FAILED)
            return false;
          if (!s.x[2].assigned() ||
              s.x[2].val() != expected.bits ||
              (with_status &&
               (!s.flags.assigned() || s.flags.val() != expected.flags))) {
            std::cerr << "divide exact mismatch " << ref.fraction << ' '
                      << p.rounding << ' ' << p.input_subnormals << ' '
                      << p.output_subnormals << ' ' << p.tininess << ' '
                      << std::hex << a << ' ' << b << " expected "
                      << expected.bits << std::dec << '/' << expected.flags
                      << '\n';
            return false;
          }
          return true;
        }

        class DivideExact : public Base {
        public:
          DivideExact()
              : Base("Word::Float::Arithmetic::Wide::Divide::Exact") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              const std::vector<WordValue> values{
                  0,
                  ref.sign,
                  1,
                  ref.sign + 1,
                  ref.normal - 1,
                  ref.normal,
                  ref.normal + 1,
                  2 * ref.normal - 1,
                  ref.one + ref.normal,
                  ref.one + ref.normal + ref.normal / 2,
                  ref.one - 2,
                  ref.one - 1,
                  ref.one,
                  ref.one + 1,
                  ref.one + ref.normal / 2,
                  ref.sign + ref.one,
                  ref.maximum - 1,
                  ref.maximum,
                  ref.sign + ref.maximum,
                  ref.infinity,
                  ref.sign + ref.infinity,
                  ref.infinity + 1,
                  ref.infinity + ref.quiet + 3,
                  ref.sign + ref.infinity + 5,
                  ref.sign + ref.infinity + ref.quiet + 7};
              for (int r = 0; r < 4; ++r)
                for (WordValue a : values)
                  for (WordValue b : values)
                    if (!check_exact(ref,
                                     Policy{static_cast<Policy::Rounding>(r)},
                                     a, b))
                      return false;
              const std::pair<WordValue, WordValue> catalogue[] = {
                  {1, ref.one + ref.normal},
                  {2 * ref.normal - 1, ref.one + ref.normal},
                  {ref.normal, ref.one - 1},
                  {ref.maximum, 1},
                  {ref.one, 0},
                  {0, 1},
                  {ref.sign, ref.one},
                  {ref.infinity + ref.quiet + 3, ref.sign + ref.infinity + 5}};
              for (int r = 0; r < 4; ++r)
                for (int in = 0; in < 2; ++in)
                  for (int out = 0; out < 2; ++out)
                    for (int overflow = 0; overflow < 2; ++overflow)
                      for (int zeros = 0; zeros < 2; ++zeros)
                        for (int nan = 0; nan < 3; ++nan)
                          for (int tiny = 0; tiny < 2; ++tiny)
                            for (int comparison = 0; comparison < 2;
                                 ++comparison) {
                              const Policy p{
                                  static_cast<Policy::Rounding>(r),
                                  static_cast<Policy::Subnormals>(in),
                                  static_cast<Policy::Subnormals>(out),
                                  static_cast<Policy::Overflow>(overflow),
                                  static_cast<Policy::Zeros>(zeros),
                                  static_cast<Policy::Nan>(nan),
                                  static_cast<Policy::Tininess>(tiny),
                                  static_cast<Policy::Comparison>(comparison)};
                              for (const auto& pair : catalogue)
                                if (!check_exact(ref, p, pair.first,
                                                 pair.second) ||
                                    !check_exact(ref, p, pair.first,
                                                 pair.second, false))
                                  return false;
                            }
              // Both parity midpoints at the subnormal boundary, both signs.
              for (WordValue small : {WordValue(1), WordValue(3)})
                for (WordValue sign : {WordValue(0), ref.sign})
                  for (int r = 0; r < 4; ++r)
                    if (!check_exact(ref,
                                     Policy{static_cast<Policy::Rounding>(r)},
                                     sign + small, ref.one + ref.normal))
                      return false;
              // Literal witnesses anchor the independent oracle's range rules.
              const struct {
                WordValue a, b, result;
                int flags;
              } literals[] = {
                  {1, ref.one + ref.normal, 0, 24},
                  {ref.sign + 1, ref.one + ref.normal, ref.sign, 24},
                  {3, ref.one + ref.normal, 2, 24},
                  {ref.normal, ref.one + ref.normal, ref.normal / 2, 0},
                  {2 * ref.normal - 1, ref.one + ref.normal, ref.normal, 24},
                  {ref.one, ref.one + ref.normal + ref.normal / 2,
                   ref.one - 2 * ref.normal + (ref.normal + 1) / 3, 16},
                  {ref.maximum, 1, ref.infinity, 20},
                  {1, ref.maximum, 0, 24},
                  {ref.one, 0, ref.infinity, 2},
                  {ref.infinity, 0, ref.infinity, 0},
                  {0, 0, ref.infinity + ref.quiet, 1},
                  {ref.infinity, ref.infinity, ref.infinity + ref.quiet, 1},
                  {0, ref.infinity, 0, 0},
                  {ref.sign, ref.one, ref.sign, 0}};
              for (const auto& e : literals) {
                const Expected expected = ref.evaluate(Policy(), e.a, e.b);
                if (expected.bits != e.result || expected.flags != e.flags)
                  return false;
                for (bool wrong_status : {false, true}) {
                  Pair s(f);
                  s.post(Policy());
                  dom(s, s.x[0], e.a);
                  dom(s, s.x[1], e.b);
                  dom(s, s.x[2], e.result ^ (wrong_status ? 0 : 1));
                  rel(s, s.flags, IRT_EQ, e.flags ^ (wrong_status ? 16 : 0));
                  if (s.status() != SS_FAILED)
                    return false;
                }
              }
            }
            return true;
          }
        };

        using Row = std::array<WordValue, 4>;
        /// Total-order window of encodings
        struct Window {
          WordValue minimum, maximum;
        };
        void bounds(Space& s, WordVar word, Window w) {
          float_bounds(s, word, w.minimum, w.maximum);
        }
        /// The lowest code of \a w, which lies within one sign half
        WordValue lowest_code(Window w) {
          return std::min(w.minimum, w.maximum);
        }
        /// The window of the encodings with sign bit \a sign, which may be
        /// zero, and magnitudes from \a low to \a high
        Window magnitudes(WordValue sign, WordValue low, WordValue high) {
          return sign != 0 ? Window{sign | high, sign | low}
                           : Window{low, high};
        }

        std::set<Row> expected_pairs(const Reference& ref, const Policy& p,
                                     Window a, Window b, Window z, int status,
                                     int alias) {
          const unsigned int width = float_word_width(ref.format);
          std::set<Row> expected;
          for (WordValue av : float_order_range(width, a.minimum, a.maximum))
            for (WordValue bv :
                 float_order_range(width, b.minimum, b.maximum)) {
              if ((alias == 1 || alias == 3) && av != bv)
                continue;
              const Expected value = ref.evaluate(p, av, bv);
              if (status >= 0 && status != value.flags)
                continue;
              if (float_order_contains(width, z.minimum, z.maximum,
                                       value.bits) &&
                  (alias < 2 || value.bits == (alias == 4 ? bv : av)))
                expected.insert({av, bv, value.bits, WordValue(value.flags)});
            }
          return expected;
        }
        std::set<Row> pair_solutions(Pair& s) {
          std::set<Row> found;
          DFS<Pair> search(&s);
          while (std::unique_ptr<Pair> next{search.next()})
            found.insert({next->x[0].val(), next->x[1].val(),
                          next->x[2].val(),
                          WordValue(next->flags.val())});
          return found;
        }
        bool check_window(const Reference& ref, const Policy& p, Window a,
                          Window b, Window z, int prune = -1, int status = -1,
                          int alias = 0) {
          const auto expected = expected_pairs(ref, p, a, b, z, status, alias);
          Pair s(ref.format);
          if (alias == 1 || alias == 3)
            s.x[1] = s.x[0];
          if (alias >= 2)
            s.x[2] = s.x[alias == 4 ? 1 : 0];
          bounds(s, s.x[0], a);
          bounds(s, s.x[1], b);
          bounds(s, s.x[2], z);
          if (status >= 0)
            rel(s, s.flags, IRT_EQ, status);
          s.post(p);
          if (s.status() == SS_FAILED)
            return expected.empty();
          for (const auto& row : expected) {
            for (int i = 0; i < 3; ++i)
              if (!Gecode::Word::WordView(s.x[i]).in(row[i]))
                return false;
            if (!s.flags.in(static_cast<int>(row[3])))
              return false;
          }
          if (prune >= 0) {
            const Window original[] = {a, b, z};
            const auto view = Gecode::Word::WordView(s.x[prune]);
            if (view.in(original[prune].minimum) &&
                view.in(original[prune].maximum)) {
              std::cerr << "divide did not prune " << ref.format << ' '
                        << prune << ' ' << std::hex << a.minimum << ' '
                        << a.maximum << ' ' << b.minimum << ' ' << b.maximum
                        << ' ' << z.minimum << ' ' << z.maximum << std::dec
                        << '\n';
              return false;
            }
          }
          branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                 WORD_VAL_LSB());
          const auto found = pair_solutions(s);
          if (found != expected) {
            std::cerr << "divide relation mismatch " << ref.format << ' '
                      << p.rounding << ' ' << p.input_subnormals << ' '
                      << p.output_subnormals << ' ' << alias << ' ' << std::hex
                      << a.minimum << ' ' << a.maximum << ' ' << b.minimum
                      << ' ' << b.maximum << std::dec << " tuples "
                      << found.size() << '/' << expected.size() << '\n';
            return false;
          }
          return true;
        }

        // These exact cubes span both signs and zero without enumerating an
        // interval across the raw sign bit. They also leave interior bit holes.
        bool check_zero_alternatives(const Reference& ref, const Policy& p,
                                     WordValue numerator, WordValue denominator,
                                     int status) {
          Pair s(ref.format);
          dom(s, s.x[0], WordValue(0), ref.sign | numerator);
          dom(s, s.x[1], WordValue(0), ref.sign | denominator);
          if (status >= 0)
            rel(s, s.flags, IRT_EQ, status);
          s.post(p);
          std::set<Row> expected;
          for (WordValue a :
               {WordValue(0), numerator, ref.sign, ref.sign | numerator})
            for (WordValue b : {WordValue(0), denominator, ref.sign,
                                ref.sign | denominator}) {
              const Expected value = ref.evaluate(p, a, b);
              if (status < 0 || value.flags == status)
                expected.insert({a, b, value.bits, WordValue(value.flags)});
            }
          if (s.status() == SS_FAILED)
            return expected.empty();
          for (const auto& row : expected)
            for (int i = 0; i < 3; ++i)
              if (!Gecode::Word::WordView(s.x[i]).in(row[i]))
                return false;
          branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                 WORD_VAL_LSB());
          return pair_solutions(s) == expected;
        }

        class DivideSymbolic : public Base {
        public:
          DivideSymbolic()
              : Base("Word::Float::Arithmetic::Wide::Divide::Symbolic") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              const Window all{ref.sign | (ref.sign - 1), ref.sign - 1};
              for (bool flush : {false, true})
                for (int status : {-1, 0, 1, 2, 24})
                  for (WordValue a : {WordValue(1), ref.one + ref.normal})
                    for (WordValue b : {WordValue(1), ref.one + ref.normal})
                      if (!check_zero_alternatives(
                              ref,
                              Policy{Policy::RNE, flush ? Policy::SignedZero
                                                        : Policy::Preserve},
                              a, b, status))
                        return false;

              for (int r = 0; r < 4; ++r) {
                const Policy p{static_cast<Policy::Rounding>(r)};
                // Forward and each inverse direction, across the one binade.
                if (!check_window(ref, p, {ref.one - 4, ref.one + 4},
                                  {ref.one, ref.one + 1}, all, 2) ||
                    !check_window(ref, p, {ref.one - 4, ref.one + 4},
                                  {ref.one, ref.one + 1}, {ref.one, ref.one},
                                  0) ||
                    !check_window(ref, p, {ref.one, ref.one + 1},
                                  {ref.one - 4, ref.one + 4},
                                  {ref.one, ref.one}, 1))
                  return false;
                // Windows that are exact bit cubes, pruned in each inverse
                // direction.
                if (!check_window(ref, p, {ref.one, ref.one + 7},
                                  {ref.one, ref.one + 1}, {ref.one, ref.one},
                                  0) ||
                    !check_window(ref, p, {ref.one, ref.one + 1},
                                  {ref.one, ref.one + 7}, {ref.one, ref.one},
                                  1))
                  return false;
                const struct {
                  Window a, b;
                } boundaries[] = {
                    {{0, 3}, {ref.one - 1, ref.one + 1}},
                    {{ref.normal - 2, ref.normal + 2},
                     {ref.one - 2, ref.one + 1}},
                    {{ref.maximum - 2, ref.maximum},
                     {ref.one - 1, ref.one + 1}},
                    {{ref.infinity - 2, ref.infinity + 2}, {0, 2}},
                    {{ref.infinity + ref.quiet, ref.infinity + ref.quiet + 2},
                     {ref.sign + ref.infinity + 3,
                      ref.sign + ref.infinity + 1}},
                    {{ref.sign + ref.one + 2, ref.sign + ref.one - 2},
                     {ref.one - 1, ref.one + 1}}};
                for (const auto& q : boundaries)
                  for (bool inverse : {false, true}) {
                    const WordValue selected =
                        ref.evaluate(p, lowest_code(q.a), lowest_code(q.b))
                            .bits;
                    if (!check_window(ref, p, q.a, q.b,
                                      inverse ? Window{selected, selected}
                                              : all))
                      return false;
                  }
                for (int in = 0; in < 2; ++in)
                  for (int out = 0; out < 2; ++out) {
                    const Policy flush{static_cast<Policy::Rounding>(r),
                                       static_cast<Policy::Subnormals>(in),
                                       static_cast<Policy::Subnormals>(out)};
                    if (!check_window(ref, flush,
                                      {ref.normal - 2, ref.normal + 2},
                                      {ref.one - ref.normal - 1,
                                       ref.one - ref.normal + 1},
                                      all) ||
                        !check_window(ref, flush, {0, 3},
                                      {ref.infinity - 1, ref.infinity + 1},
                                      all) ||
                        !check_window(ref, flush,
                                      {ref.normal - 2, ref.normal + 2},
                                      {ref.one - 1, ref.one + 1}, {0, 0}))
                      return false;
                  }
                for (WordValue as : {WordValue(0), ref.sign})
                  for (WordValue bs : {WordValue(0), ref.sign})
                    if (!check_window(ref, p,
                                      magnitudes(as, ref.one - 2, ref.one + 2),
                                      magnitudes(bs, ref.one - 1, ref.one + 1),
                                      all))
                      return false;
                for (bool flush_inputs : {false, true}) {
                  const Policy denominator_policy{
                      static_cast<Policy::Rounding>(r),
                      flush_inputs ? Policy::SignedZero : Policy::Preserve};
                  if (!check_window(ref, denominator_policy,
                                    {ref.one, ref.one + 2}, {0, 3}, all) ||
                      !check_window(ref, denominator_policy, {0, 3}, {0, 3},
                                    all))
                    return false;
                }
                const Policy positive{static_cast<Policy::Rounding>(r),
                                      Policy::Preserve, Policy::Preserve,
                                      Policy::Format, Policy::Positive};
                if (!check_window(
                        ref, positive, {ref.sign + 3, ref.sign},
                        {ref.one - ref.normal, ref.one - ref.normal + 1},
                        all))
                  return false;
                const Policy positive_flush{
                    static_cast<Policy::Rounding>(r),
                    Policy::Preserve, Policy::SignedZero, Policy::Format,
                    Policy::Positive};
                if (!check_window(ref, positive_flush,
                                  {ref.sign + ref.normal + 2,
                                   ref.sign + ref.normal - 2},
                                  {ref.one - 1, ref.one + 1}, all))
                  return false;
                if (!check_window(ref, p, {ref.one - 2, ref.one + 2},
                                  {ref.one - 2, ref.one + 2}, all, -1, 0))
                  return false;
              }
              // Inverse exceptional classes: finite result excludes infinity
              // and NaNs even though those classes occur in the operands.
              if (!check_window(ref, Policy(),
                                {ref.infinity - 2, ref.infinity + 2},
                                {ref.one, ref.one + 1},
                                {ref.maximum - 2, ref.maximum}, 0) ||
                  !check_window(ref, Policy(),
                                {ref.infinity - 2, ref.infinity + 2}, {0, 1},
                                {ref.infinity + ref.quiet,
                                 ref.infinity + ref.quiet + 2},
                                0, 1) ||
                  !check_window(ref, Policy(),
                                {ref.infinity - 2, ref.infinity + 2},
                                {ref.one, ref.one + 1},
                                {ref.infinity, ref.infinity}, 0) ||
                  !check_window(ref, Policy(),
                                {ref.infinity + 1, ref.infinity + 3},
                                {ref.one, ref.one + 1},
                                {ref.infinity + ref.quiet,
                                 ref.infinity + ref.quiet + 3},
                                -1, 0))
                return false;
              // Exact class cubes make exceptional inverse pruning visible.
              if (!check_window(ref, Policy(),
                                {ref.infinity, ref.infinity + 3},
                                {ref.one, ref.one + 1},
                                {ref.infinity, ref.infinity}, 0) ||
                  !check_window(
                      ref, Policy(), {0, 3}, {0, 0},
                      {ref.infinity + ref.quiet, ref.infinity + ref.quiet},
                      0))
                return false;
              if (f == WDT_FP32) {
                for (int r = 0; r < 4; ++r)
                  for (int flush = 0; flush < 2; ++flush) {
                    const Policy p{static_cast<Policy::Rounding>(r),
                                   static_cast<Policy::Subnormals>(flush),
                                   static_cast<Policy::Subnormals>(flush)};
                    if (!check_window(ref, p,
                                      {2 * ref.normal - 2, 2 * ref.normal + 2},
                                      {ref.one + ref.normal - 1,
                                       ref.one + ref.normal + 1},
                                      all) ||
                        !check_window(ref, p,
                                      {2 * ref.normal - 2, 2 * ref.normal + 2},
                                      {ref.one + ref.normal - 1,
                                       ref.one + ref.normal + 1},
                                      {0, 0}) ||
                        !check_window(ref, p, {ref.one, ref.one + 2}, {0, 3},
                                      all))
                      return false;
                  }
              }
              for (int rounding = 0; rounding < 4; ++rounding)
                for (int alias = 1; alias <= 4; ++alias)
                  for (const Window& a :
                       {Window{ref.one - 2, ref.one + 2}, Window{0, 3},
                        Window{ref.sign + 3, ref.sign},
                        Window{ref.infinity - 1, ref.infinity + 2},
                        Window{ref.infinity + ref.quiet,
                               ref.infinity + ref.quiet + 2}}) {
                    const Window b = (alias == 2 || alias == 4)
                                         ? Window{ref.one, ref.one + 1}
                                         : a;
                    if (!check_window(
                            ref,
                            Policy{static_cast<Policy::Rounding>(rounding)}, a,
                            b, alias == 4 ? b : (alias >= 2 ? a : all), -1,
                            -1, alias))
                      return false;
                  }
            }
            return true;
          }
        };

        using Tuple = std::array<WordValue, 7>;
        class Network : public Space {
        public:
          WordVar x[5];
          IntVar flags[2];
          explicit Network(const Reference& ref, const Policy& p,
                           bool later = false) {
            for (auto& v : x)
              v = WordVar(*this, ref.format);
            for (auto& v : flags)
              v = IntVar(*this, 0, 31);
            bounds(*this, x[0], {ref.one - 4, ref.one + 4});
            bounds(*this, x[1], {ref.one - 1, ref.one + 1});
            bounds(*this, x[3], {ref.one - 1, ref.one + 1});
            dom(*this, x[4], ref.one);
            fp_divide(*this, x[0], x[1], x[2], flags[0], p);
            fp_divide(*this, x[2], x[3], x[4], flags[1], p);
            if (later)
              restrict_later(ref);
            WordVarArgs words(5);
            for (int i = 0; i < 5; ++i)
              words[i] = x[i];
            branch(*this, words, WORD_VAR_NONE(), WORD_VAL_LSB());
          }
          void restrict_later(const Reference& ref) {
            dom(*this, x[0], WordValue(0), (2 * ref.sign - 1) ^ 1);
            bounds(*this, x[1], {ref.one, ref.one + 1});
            rel(*this, flags[0], IRT_EQ, 0);
          }
          Network(Network& other) : Space(other) {
            for (int i = 0; i < 5; ++i)
              x[i].update(*this, other.x[i]);
            for (int i = 0; i < 2; ++i)
              flags[i].update(*this, other.flags[i]);
          }
          Space* copy() override { return new Network(*this); }
        };
        std::set<Tuple> expected_network(const Reference& ref, const Policy& p,
                                         bool later) {
          std::set<Tuple> expected;
          for (WordValue a = ref.one - 4; a <= ref.one + 4; ++a)
            for (WordValue b = ref.one - 1; b <= ref.one + 1; ++b)
              for (WordValue c = ref.one - 1; c <= ref.one + 1; ++c) {
                if (later && ((a & 1) || b < ref.one))
                  continue;
                const auto first = ref.evaluate(p, a, b);
                const auto second = ref.evaluate(p, first.bits, c);
                if (later && first.flags != 0)
                  continue;
                if (second.bits == ref.one)
                  expected.insert({a, b, first.bits, c, second.bits,
                                   WordValue(first.flags),
                                   WordValue(second.flags)});
              }
          return expected;
        }
        bool check_network(Network& s, const std::set<Tuple>& expected,
                           unsigned int recompute) {
          if (s.status() == SS_FAILED)
            return expected.empty();
          for (const auto& tuple : expected) {
            for (int i = 0; i < 5; ++i)
              if (!Gecode::Word::WordView(s.x[i]).in(tuple[i]))
                return false;
            for (int i = 0; i < 2; ++i)
              if (!s.flags[i].in(static_cast<int>(tuple[5 + i])))
                return false;
          }
          Search::Options options;
          options.c_d = recompute;
          options.a_d = recompute;
          DFS<Network> search(&s, options);
          std::set<Tuple> found;
          while (std::unique_ptr<Network> next{search.next()})
            found.insert({next->x[0].val(), next->x[1].val(),
                          next->x[2].val(), next->x[3].val(),
                          next->x[4].val(),
                          WordValue(next->flags[0].val()),
                          WordValue(next->flags[1].val())});
          return found == expected;
        }
        class DivideLifecycle : public Base {
        public:
          DivideLifecycle()
              : Base("Word::Float::Arithmetic::Wide::Divide::Lifecycle") {}
          bool run() override {
            // For every normal x, this factor moves the exact value by more
            // than half an ulp. No normal fixed point exists.
            Pair impossible(WDT_FP32);
            impossible.x[2] = impossible.x[0];
            bounds(impossible, impossible.x[0],
                   {0x3f800000, 0x4b800000});
            dom(impossible, impossible.x[1], WordValue(0x3f800001));
            impossible.post(Policy());
            if (impossible.status() != SS_FAILED)
              return false;
            Pair repeated(WDT_FP32);
            repeated.x[1] = repeated.x[0];
            bounds(repeated, repeated.x[0], {0x3f800000, 0x4b800000});
            dom(repeated, repeated.x[2], WordValue(0x3f800001));
            repeated.post(Policy());
            if (repeated.status() != SS_FAILED)
              return false;
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              for (int r = 0; r < 4; ++r) {
                const Policy p{static_cast<Policy::Rounding>(r)};
                Network original(ref, p);
                if (original.status() == SS_FAILED) {
                  std::cerr << "divide network failed " << f << ' ' << r
                            << '\n';
                  return false;
                }
                if (Gecode::Word::WordView(original.x[0])
                        .in(ref.one - 4) &&
                    Gecode::Word::WordView(original.x[0])
                        .in(ref.one + 4)) {
                  std::cerr << "divide network did not prune " << f << ' '
                            << r << '\n';
                  return false;
                }
                std::unique_ptr<Network> clone(
                    static_cast<Network*>(original.clone()));
                fp_divide(*clone, clone->x[0], clone->x[1], clone->x[2],
                          clone->flags[0], p);
                fp_divide(*clone, clone->x[2], clone->x[3], clone->x[4],
                          clone->flags[1], p);
                if (clone->status() == SS_FAILED)
                  return false;
                for (int i = 0; i < 5; ++i) {
                  const WordVar a = original.x[i],
                                b = clone->x[i];
                  if (a.lo() != b.lo() || a.hi() != b.hi() ||
                      (a.bounded() && (a.minimum() != b.minimum() ||
                                       a.maximum() != b.maximum())))
                    return false;
                }
                for (int i = 0; i < 2; ++i)
                  for (int flag = 0; flag < 32; ++flag)
                    if (original.flags[i].in(flag) !=
                        clone->flags[i].in(flag))
                      return false;
                clone->restrict_later(ref);
                Network fresh(ref, p, true);
                const auto restricted = expected_network(ref, p, true);
                if (!check_network(*clone, restricted, 1) ||
                    !check_network(fresh, restricted, 16) ||
                    !check_network(original, expected_network(ref, p, false),
                                   8))
                  return false;
              }
            }
            return true;
          }
        };

        class DividePosting : public Base {
        public:
          DividePosting()
              : Base("Word::Float::Arithmetic::Wide::Divide::Posting") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32, WDT_BINARY16})
              for (bool status : {false, true}) {
                Pair s(f);
                bool caught = false;
                try {
                  if (status)
                    fp_divide(s, s.x[0], s.x[1], s.x[2], s.flags);
                  else
                    fp_divide(s, s.x[0], s.x[1], s.x[2]);
                } catch (const Gecode::Word::OutOfLimits&) {
                  caught = true;
                }
                if (caught != (f == WDT_BINARY16))
                  return false;
              }
            Pair mismatch(WDT_BF16);
            WordVar binary16 = WordVar(mismatch, WDT_BINARY16);
            try {
              fp_divide(mismatch, mismatch.x[0], binary16,
                        mismatch.x[2]);
            } catch (const Gecode::Word::FormatMismatch&) {
              return true;
            }
            return false;
          }
        };
        // Quotients on float-typed words, whose bounds are in IEEE total order,
        // over negative-only and cross-zero windows.
        class DivideFloatTyped : public Base {
          // Total-order window
          struct Range {
            WordValue minimum, maximum;
          };
          static bool check(const Reference& ref, const Policy& p, Range a,
                            Range b, Range z) {
            const unsigned int width = float_word_width(ref.format);
            Pair s(ref.format);
            s.x[0] = float_window(s, ref.format, a.minimum, a.maximum);
            s.x[1] = float_window(s, ref.format, b.minimum, b.maximum);
            s.x[2] = float_window(s, ref.format, z.minimum, z.maximum);
            std::set<Row> expected;
            for (WordValue av : float_order_range(width, a.minimum, a.maximum))
              for (WordValue bv :
                   float_order_range(width, b.minimum, b.maximum)) {
                const Expected e = ref.evaluate(p, av, bv);
                if (s.x[2].in(e.bits))
                  expected.insert({av, bv, e.bits, WordValue(e.flags)});
              }
            s.post(p);
            if (s.status() == SS_FAILED)
              return expected.empty();
            branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                   WORD_VAL_LSB());
            branch(s, s.flags, INT_VAL_MIN());
            if (pair_solutions(s) == expected)
              return true;
            if (opt.log)
              olog << "float-typed divide " << ref.format << ' ' << p.rounding
                   << ' ' << std::hex << a.minimum << ".." << a.maximum << ' '
                   << b.minimum << ".." << b.maximum << ' ' << z.minimum
                   << ".." << z.maximum << std::dec << '\n';
            return false;
          }
          static bool windows(const Reference& ref) {
            const WordValue sign = ref.sign, one = ref.one,
                            normal = ref.normal;
            const Range all{sign | ref.infinity, ref.infinity};
            // Operand windows in total order, with the result window: all
            // values, or a negative window that drops the zeros and tails.
            const Range cases[][3] = {
                {{sign | (one + 2), sign | one}, {one, one + 2}, all},
                {{sign | 3, 3}, {sign | (one + 1), sign | (one - 1)}, all},
                {{sign | (normal + 2), sign | (normal - 2)},
                 {one - normal, one - normal + 2},
                 all},
                {{sign | (one + 8), sign | (one - 8)},
                 {one - 1, one + 1},
                 {sign | (one + 2), sign | (one - 2)}},
                {{one - 4, one + 4},
                 {sign | (one + 1), sign | (one - 1)},
                 {sign | (one + 2), sign | (one - 2)}},
                {{sign | 2, 2}, {sign | 1, 1}, all}};
            for (int r = 0; r < 4; ++r)
              for (bool flush : {false, true}) {
                const auto mode = static_cast<Policy::Rounding>(r);
                const Policy p = flush ? Policy{mode, Policy::SignedZero,
                                                Policy::SignedZero}
                                       : Policy{mode};
                for (const auto& c : cases)
                  if (!check(ref, p, c[0], c[1], c[2]))
                    return false;
              }
            return true;
          }
          // Dividing the float-typed window of the three codes up to -1 by 1
          // gives the total-order bounds -1 - 2 ulp and -1.
          static bool bound_deduction() {
            Pair s(WDT_FP32);
            s.x[0] = float_window(s, WDT_FP32, 0xbf800002, 0xbf800000);
            s.x[1] = float_window(s, WDT_FP32, 0x3f800000, 0x3f800000);
            s.post(Policy());
            return s.status() != SS_FAILED && s.x[2].minimum() == 0xbf800002 &&
                   s.x[2].maximum() == 0xbf800000;
          }

        public:
          DivideFloatTyped()
              : Base("Word::Float::Arithmetic::Wide::Divide::FloatTyped") {}
          bool run() override {
            for (WordDomainType format : {WDT_BF16, WDT_FP32})
              if (!windows(Reference(format)))
                return false;
            return bound_deduction();
          }
        };
        DivideExact divide_exact;
        DivideSymbolic divide_symbolic;
        DivideLifecycle divide_lifecycle;
        DividePosting divide_posting;
        DivideFloatTyped divide_float_typed;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test
// STATISTICS: test-word
