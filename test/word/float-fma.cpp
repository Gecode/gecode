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
#include <cstdint>
#include <gecode/float-word.hh>
#include <gecode/search.hh>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

namespace Test {
  namespace Word {
    namespace Float {
      namespace Fma {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        struct Result {
          int code, flags;
        };

        // This oracle uses rational neighbors enumerated from the
        // specification. It shares no decoding, precision rounding, or
        // evaluation code with production.
        struct Rational {
          int64_t numerator, denominator;
        };
        bool less(Rational a, Rational b) {
          return a.numerator * b.denominator < b.numerator * a.denominator;
        }
        bool equal(Rational a, Rational b) {
          return a.numerator * b.denominator == b.numerator * a.denominator;
        }
        Rational magnitude(WordDomainType format, int code) {
          const int fraction = format == WDT_E2M1 ? 1 : 3;
          const int bias = format == WDT_E2M1 ? 1 : 7;
          const int field = code >> fraction;
          Rational value{
              field == 0 ? code : (1 << fraction) + (code % (1 << fraction)),
              1};
          const int exponent = (field == 0 ? 1 : field) - bias - fraction;
          for (int i = 0; i < exponent; ++i)
            value.numerator *= 2;
          for (int i = 0; i < -exponent; ++i)
            value.denominator *= 2;
          return value;
        }
        struct Neighbor {
          Rational value;
          bool even;
        };
        struct Oracle {
          WordDomainType format;
          int sign, maximum, normal;
          std::vector<Neighbor> finite, precision;
          explicit Oracle(WordDomainType f)
              : format(f), sign(f == WDT_E2M1 ? 8 : 128),
                maximum(f == WDT_E2M1 ? 7 : 126),
                normal(f == WDT_E2M1 ? 2 : 8) {
            for (int i = 0; i <= maximum; ++i)
              finite.push_back({magnitude(format, i), i % 2 == 0});
            // This range strictly encloses every nonzero exact tiny FMA
            // result.
            for (int exponent = -24; exponent <= 18; ++exponent)
              for (int significand = normal; significand < 2 * normal;
                   ++significand) {
                Rational value{significand, 1};
                for (int i = 0; i < exponent; ++i)
                  value.numerator *= 2;
                for (int i = 0; i < -exponent; ++i)
                  value.denominator *= 2;
                precision.push_back({value, significand % 2 == 0});
              }
          }
          static size_t nearest(const std::vector<Neighbor>& values,
                                Rational exact, bool negative,
                                Policy::Rounding rounding) {
            const auto upper = std::lower_bound(
                values.begin(), values.end(), exact,
                [](const Neighbor& candidate, Rational value) {
                  return less(candidate.value, value);
                });
            if (upper == values.end())
              return values.size() - 1;
            const size_t hi = static_cast<size_t>(upper - values.begin());
            if (equal(upper->value, exact) || hi == 0)
              return hi;
            const size_t lo = hi - 1;
            if (rounding != Policy::RNE)
              return (rounding == Policy::RUP && !negative) ||
                             (rounding == Policy::RDN && negative)
                         ? hi
                         : lo;
            const Rational low = values[lo].value, high = upper->value;
            const int64_t denominator =
                std::max(low.denominator, high.denominator);
            const Rational midpoint{
                low.numerator * (denominator / low.denominator) +
                    high.numerator * (denominator / high.denominator),
                2 * denominator};
            if (less(exact, midpoint))
              return lo;
            if (less(midpoint, exact))
              return hi;
            return values[lo].even ? lo : hi;
          }
          Result evaluate(const Policy& policy, int a, int b, int c) const {
            int codes[] = {a % sign, b % sign, c % sign};
            const int operands[] = {a, b, c};
            if (format == WDT_E4M3FN)
              for (int i = 0; i < 3; ++i)
                if (codes[i] == 127)
                  return {policy.nan == Policy::Canonical ? 127 : operands[i],
                          0};
            if (policy.input_subnormals == Policy::SignedZero)
              for (int& code : codes)
                if (code < normal)
                  code = 0;
            const Rational av = finite[codes[0]].value;
            const Rational bv = finite[codes[1]].value;
            const Rational cv = finite[codes[2]].value;
            const bool product_negative = (a >= sign) != (b >= sign);
            const bool addend_negative = c >= sign;
            const Rational product{av.numerator * bv.numerator,
                                   av.denominator * bv.denominator};
            // All denominators are powers of two. Align instead of
            // multiplying them: abs(n)<=52,730,789,888, d<=2^18. Neighbor and
            // midpoint comparisons (whose denominators are <=2^25) stay below
            // 2^61.
            const int64_t denominator =
                std::max(product.denominator, cv.denominator);
            Rational exact{
                (product_negative ? -product.numerator : product.numerator) *
                        (denominator / product.denominator) +
                    (addend_negative ? -cv.numerator : cv.numerator) *
                        (denominator / cv.denominator),
                denominator};
            bool negative = exact.numerator < 0 ||
                            (exact.numerator == 0 &&
                             (product_negative == addend_negative
                                  ? product_negative
                                  : policy.rounding == Policy::RDN));
            if (exact.numerator < 0)
              exact.numerator = -exact.numerator;
            if (exact.numerator == 0)
              return {negative && policy.zeros == Policy::Signed ? sign : 0,
                      0};
            const Rational unbounded =
                precision[nearest(precision, exact, negative,
                                  policy.rounding)]
                    .value;
            if (less(finite[maximum].value, unbounded)) {
              const bool nan =
                  format == WDT_E4M3FN && policy.overflow == Policy::Format;
              return {
                  (nan ? 127 : maximum) +
                      (negative && !(nan && policy.nan == Policy::Canonical)
                           ? sign
                           : 0),
                  20};
            }
            int result = static_cast<int>(
                nearest(finite, exact, negative, policy.rounding));
            int flags = equal(finite[result].value, exact) ? 0 : 16;
            const bool tiny = less(
                policy.tininess == Policy::BeforeRounding ? exact : unbounded,
                finite[normal].value);
            if (tiny && flags != 0)
              flags |= 8;
            if (result > 0 && result < normal &&
                policy.output_subnormals == Policy::SignedZero) {
              result = 0;
              flags |= 24;
            }
            if (result == 0 && policy.zeros == Policy::Positive)
              negative = false;
            return {result + (negative ? sign : 0), flags};
          }
        };

        /// The values of a word of width at most 8, in increasing order
        std::vector<int> values(const WordVar& x) {
          std::vector<int> found;
          for (int v = 0; v < (1 << x.width()); ++v)
            if (x.in(WordValue(v)))
              found.push_back(v);
          return found;
        }
        std::vector<int> values(const IntVar& x) {
          std::vector<int> found;
          for (IntVarValues v(x); v(); ++v)
            found.push_back(v.val());
          return found;
        }
        /// Restrict \a x to \a codes, or without a value set to their cube
        /// hull and total-order hull
        void restrict(Space& home, WordVar x, const std::vector<int>& codes) {
          if (x.has_value_set()) {
            std::vector<WordValue> list(codes.begin(), codes.end());
            dom(home, x, WordValArgs(list));
            return;
          }
          // A list would post a membership constraint beyond the domain.
          WordValue ones = ~WordValue(0), any = 0;
          for (int v : codes) {
            ones &= WordValue(v);
            any |= WordValue(v);
          }
          dom(home, x, ones, any);
          const auto bounds = float_order_extremes(
              x.width(), std::set<WordValue>(codes.begin(), codes.end()));
          float_bounds(home, x, bounds.first, bounds.second);
        }
        /// Test whether \a x keeps exactly, or the hulls of, \a p
        bool projected(const WordVar& x, const std::set<int>& p) {
          if (x.has_value_set())
            return values(x) == std::vector<int>(p.begin(), p.end());
          WordValue ones = ~WordValue(0), any = 0;
          for (int v : p) {
            ones &= WordValue(v);
            any |= WordValue(v);
          }
          const auto bounds = float_order_extremes(
              x.width(), std::set<WordValue>(p.begin(), p.end()));
          return x.lo() == ones && x.hi() == any &&
                 x.minimum() == bounds.first && x.maximum() == bounds.second;
        }
        int code(const WordVar& x) {
          return static_cast<int>(x.val());
        }
        std::vector<int> range(int first, int last) {
          std::vector<int> values;
          for (int i = first; i <= last; ++i)
            values.push_back(i);
          return values;
        }
        /// The codes that differ from \a base only in the bits of \a free
        std::vector<int> cube(int base, int free) {
          std::vector<int> codes;
          for (int v = 0; v <= free; ++v)
            if ((v & ~free) == 0)
              codes.push_back(base ^ v);
          std::sort(codes.begin(), codes.end());
          return codes;
        }
        bool contains(const std::vector<int>& values, int value) {
          return std::find(values.begin(), values.end(), value) !=
                 values.end();
        }

        class FmaSpace : public Space {
        public:
          WordVar x[4];
          IntVar flags;
          FmaSpace(WordDomainType format, WordValueSetOption value_set)
              : flags(*this, -4, 40) {
            for (auto& v : x)
              v = WordVar(*this, format, value_set);
          }
          FmaSpace(FmaSpace& s) : Space(s) {
            for (int i = 0; i < 4; ++i)
              x[i].update(*this, s.x[i]);
            flags.update(*this, s.flags);
          }
          virtual Space* copy(void) { return new FmaSpace(*this); }
          void post(const Policy& policy, bool status = true,
                    IntPropLevel ipl = IPL_DEF) {
            if (status)
              fp_fma(*this, x[0], x[1], x[2], x[3], flags, policy, ipl);
            else
              fp_fma(*this, x[0], x[1], x[2], x[3], policy, ipl);
          }
        };
        /// A code or flags restriction that excludes the expected outcome
        enum class Exclude { None, Code, Flags };
        /// Whether Policies case \a count assigns its inputs after a first
        /// propagation: a quarter of the cases, covering each level and
        /// value set
        bool assigns_after(size_t count) {
          const size_t r = count % 16;
          return r == 2 || r == 7 || r == 11 || r == 14;
        }
        /// Check the FMA of \a a, \a b and \a c, with the inputs assigned
        /// before posting, or after a first propagation if \a after.
        bool check_triple(FmaSpace& source, const Oracle& oracle,
                          const Policy& policy, int a, int b, int c,
                          IntPropLevel ipl = IPL_DEF, bool after = false,
                          Exclude exclude = Exclude::None) {
          const Result expected = oracle.evaluate(policy, a, b, c);
          std::unique_ptr<FmaSpace> s(static_cast<FmaSpace*>(source.clone()));
          const int inputs[] = {a, b, c};
          if (exclude == Exclude::Code)
            rel(*s, s->x[3], WRT_NQ, WordValue(expected.code));
          if (exclude == Exclude::Flags)
            rel(*s, s->flags, IRT_NQ, expected.flags);
          if (after) {
            // Four candidates per input keep the first IPL_DOM pass small.
            if (ipl == IPL_DOM)
              for (int i = 0; i < 3; ++i)
                dom(*s, s->x[i], WordValue(inputs[i] & ~3),
                    WordValue(inputs[i] | 3));
            s->post(policy, true, ipl);
            (void)s->status();
          }
          for (int i = 0; i < 3; ++i)
            dom(*s, s->x[i], WordValue(inputs[i]));
          if (!after)
            s->post(policy, true, ipl);
          const SpaceStatus status = s->status();
          if (exclude != Exclude::None ? status == SS_FAILED
                                       : status != SS_FAILED &&
                                             s->x[3].assigned() &&
                                             s->flags.assigned() &&
                                             code(s->x[3]) == expected.code &&
                                             s->flags.val() == expected.flags)
            return true;
          std::cerr << "tiny FMA mismatch format=" << oracle.format
                    << " inputs=" << a << ',' << b << ',' << c
                    << " expected=" << expected.code << ',' << expected.flags
                    << " ipl=" << ipl << " after=" << after
                    << " exclude=" << static_cast<int>(exclude)
                    << " rounding=" << policy.rounding << '\n';
          return false;
        }
        class Exhaustive : public Base {
        public:
          Exhaustive(void) : Base("Word::Float::Fma::Exhaustive") {}
          virtual bool run(void) {
            // Four rounding directions and five individual policy overrides.
            const Policy policies[] = {
                Policy(),
                Policy{Policy::RTZ},
                Policy{Policy::RDN},
                Policy{Policy::RUP},
                Policy{Policy::RNE, Policy::SignedZero},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::SignedZero},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::Preserve, Policy::SaturateFinite},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::Preserve, Policy::Format, Policy::Positive},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::Preserve, Policy::Format, Policy::Signed,
                       Policy::SignalFirst, Policy::BeforeRounding}};
            const Oracle oracle(WDT_E2M1);
            FmaSpace source(WDT_E2M1, WVS_EXACT);
            if (source.status() == SS_FAILED)
              return false;
            size_t count = 0;
            for (const Policy& policy : policies)
              for (int a = 0; a < 16; ++a)
                for (int b = 0; b < 16; ++b)
                  for (int c = 0; c < 16; ++c) {
                    if (!check_triple(source, oracle, policy, a, b, c))
                      return false;
                    ++count;
                  }
            return count == 36864;
          }
        };
        // Frozen catalogue: signed zeros, exact cancellation,
        // positive/negative ties of both parities, normal/subnormal edges,
        // flushing, endpoint overflow and a separately overflowing product
        // with finite fused sum.
        const std::array<std::array<int, 3>, 32> fp4_cases = {
            {{0, 2, 0},   {8, 2, 8},  {0, 10, 0}, {8, 10, 8},  {2, 2, 10},
             {10, 2, 2},  {3, 5, 14}, {11, 5, 6}, {1, 1, 0},   {9, 1, 0},
             {1, 1, 1},   {9, 1, 9},  {1, 1, 2},  {9, 1, 10},  {1, 3, 0},
             {9, 3, 0},   {1, 3, 1},  {9, 3, 9},  {1, 2, 0},   {9, 2, 0},
             {1, 2, 1},   {1, 2, 9},  {7, 2, 2},  {15, 2, 10}, {7, 2, 3},
             {15, 2, 11}, {7, 4, 15}, {15, 4, 7}, {7, 7, 7},   {15, 7, 15},
             {0, 7, 1},   {8, 7, 9}}};
        const std::array<std::array<int, 3>, 32> fp8_cases = {
            {{0, 56, 0},     {128, 56, 128}, {0, 184, 0},    {128, 184, 128},
             {56, 56, 184},  {184, 56, 56},  {57, 54, 184},  {185, 54, 56},
             {1, 48, 0},     {129, 48, 0},   {1, 48, 1},     {129, 48, 129},
             {1, 48, 2},     {129, 48, 130}, {7, 56, 0},     {7, 56, 1},
             {1, 48, 7},     {129, 48, 135}, {8, 55, 0},     {136, 55, 0},
             {126, 56, 88},  {126, 56, 89},  {254, 56, 216}, {254, 56, 217},
             {126, 64, 254}, {254, 64, 126}, {127, 255, 56}, {255, 127, 56},
             {56, 255, 127}, {56, 56, 255},  {1, 52, 7},     {1, 1, 128}}};
        class Policies : public Base {
        public:
          Policies(void) : Base("Word::Float::Fma::Policies") {}
          virtual bool run(void) {
            size_t count = 0;
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN}) {
              const Oracle oracle(format);
              FmaSpace exact(format, WVS_EXACT), none(format, WVS_NONE);
              if (exact.status() == SS_FAILED || none.status() == SS_FAILED)
                return false;
              for (int r = 0; r < 4; ++r)
                for (int input = 0; input < 2; ++input)
                  for (int output = 0; output < 2; ++output)
                    for (int overflow = 0; overflow < 2; ++overflow)
                      for (int zeros = 0; zeros < 2; ++zeros)
                        for (int nan = 0; nan < 3; ++nan)
                          for (int tininess = 0; tininess < 2; ++tininess)
                            for (int comparison = 0; comparison < 2;
                                 ++comparison) {
                              const Policy policy{
                                  static_cast<Policy::Rounding>(r),
                                  static_cast<Policy::Subnormals>(input),
                                  static_cast<Policy::Subnormals>(output),
                                  static_cast<Policy::Overflow>(overflow),
                                  static_cast<Policy::Zeros>(zeros),
                                  static_cast<Policy::Nan>(nan),
                                  static_cast<Policy::Tininess>(tininess),
                                  static_cast<Policy::Comparison>(
                                      comparison)};
                              for (const auto& t : format == WDT_E2M1
                                                       ? fp4_cases
                                                       : fp8_cases) {
                                // Alternate the level, the value set and
                                // when the inputs are assigned; a few cases
                                // exclude the outcome and must fail.
                                const IntPropLevel ipl =
                                    count % 2 == 0 ? IPL_DEF : IPL_DOM;
                                const bool after = assigns_after(count);
                                const Exclude exclude =
                                    count % 97 == 5    ? Exclude::Code
                                    : count % 97 == 50 ? Exclude::Flags
                                                       : Exclude::None;
                                if (!check_triple(count % 8 >= 4 ? none : exact,
                                                  oracle, policy, t[0], t[1],
                                                  t[2], ipl, after, exclude))
                                  return false;
                                ++count;
                              }
                            }
            }
            // Pin exact zero separately from a nonzero result rounded to
            // zero, and distinguish the precision and exact tininess tests.
            struct Witness {
              Policy::Rounding rounding;
              Policy::Tininess tininess;
              int a, b, c, code, flags;
            };
            const Witness witnesses[] = {
                {Policy::RNE, Policy::AfterPrecision, 128, 56, 128, 128, 0},
                {Policy::RNE, Policy::AfterPrecision, 0, 184, 0, 0, 0},
                {Policy::RDN, Policy::AfterPrecision, 0, 184, 0, 128, 0},
                {Policy::RNE, Policy::AfterPrecision, 1, 48, 0, 0, 24},
                {Policy::RNE, Policy::AfterPrecision, 129, 48, 0, 128, 24},
                {Policy::RNE, Policy::AfterPrecision, 1, 52, 7, 8, 16},
                {Policy::RNE, Policy::BeforeRounding, 1, 52, 7, 8, 24},
                {Policy::RNE, Policy::AfterPrecision, 126, 64, 254, 126, 0},
                {Policy::RNE, Policy::AfterPrecision, 126, 56, 88, 126, 16},
                {Policy::RNE, Policy::AfterPrecision, 126, 56, 89, 127, 20}};
            const Oracle oracle(WDT_E4M3FN);
            for (const Witness& w : witnesses) {
              const Policy policy{w.rounding,
                                  Policy::Preserve, Policy::Preserve,
                                  Policy::Format, Policy::Signed,
                                  Policy::SignalFirst, w.tininess};
              const Result r = oracle.evaluate(policy, w.a, w.b, w.c);
              if (r.code != w.code || r.flags != w.flags)
                return false;
            }
            return count == 49152;
          }
        };

        using Tuple = std::array<int, 5>;
        using Tuples = std::set<Tuple>;
        using Domains = std::array<std::vector<int>, 5>;
        enum class Alias {
          Independent,
          RepeatedMultiplicands,
          RepeatedAddend,
          ResultIsFirst,
          ResultIsAddend,
          AllShared,
          RepeatedInputs
        };
        bool aliases(const Tuple& t, Alias alias) {
          switch (alias) {
          case Alias::RepeatedMultiplicands:
            return t[0] == t[1];
          case Alias::RepeatedAddend:
            return t[0] == t[2];
          case Alias::ResultIsFirst:
            return t[0] == t[3];
          case Alias::ResultIsAddend:
            return t[2] == t[3];
          case Alias::AllShared:
            return t[0] == t[1] && t[0] == t[2] && t[0] == t[3];
          case Alias::RepeatedInputs:
            return t[0] == t[1] && t[0] == t[2];
          case Alias::Independent:
            return true;
          }
          GECODE_NEVER;
          return false;
        }
        /// Make the words of \a s share as \a alias says
        void share(FmaSpace& s, Alias alias) {
          if (alias == Alias::RepeatedMultiplicands ||
              alias == Alias::AllShared || alias == Alias::RepeatedInputs)
            s.x[1] = s.x[0];
          if (alias == Alias::RepeatedAddend || alias == Alias::AllShared ||
              alias == Alias::RepeatedInputs)
            s.x[2] = s.x[0];
          if (alias == Alias::ResultIsFirst || alias == Alias::AllShared)
            s.x[3] = s.x[0];
          if (alias == Alias::ResultIsAddend)
            s.x[3] = s.x[2];
        }
        Tuples expected_tuples(const Oracle& oracle, const Policy& policy,
                               const Domains& domains,
                               Alias alias = Alias::Independent,
                               bool status = true) {
          Tuples tuples;
          for (int a : domains[0])
            for (int b : domains[1])
              for (int c : domains[2]) {
                const Result result = oracle.evaluate(policy, a, b, c);
                Tuple t{{a, b, c, result.code, result.flags}};
                if (contains(domains[3], result.code) &&
                    (!status || contains(domains[4], result.flags)) &&
                    aliases(t, alias)) {
                  if (!status)
                    t[4] = -1;
                  tuples.insert(t);
                }
              }
          return tuples;
        }
        /// A posted model and the values its words had before propagation
        struct Model {
          std::unique_ptr<FmaSpace> space;
          Domains domains;
        };
        Model model(const Oracle& oracle, const Policy& policy,
                    const Domains& wanted, Alias alias, bool status,
                    WordValueSetOption value_set, IntPropLevel ipl) {
          Model m{std::unique_ptr<FmaSpace>(new FmaSpace(oracle.format, value_set)),
                  wanted};
          FmaSpace& s = *m.space;
          share(s, alias);
          for (int i = 0; i < 4; ++i)
            restrict(s, s.x[i], wanted[i]);
          for (int i = 0; i < 4; ++i)
            m.domains[i] = values(s.x[i]);
          if (status)
            dom(s, s.flags,
                IntSet(wanted[4].data(), static_cast<int>(wanted[4].size())));
          s.post(policy, status, ipl);
          branch(s,
                 WordVarArgs{s.x[0], s.x[1], s.x[2],
                             s.x[3]},
                 WORD_VAL_LSB());
          if (status)
            branch(s, s.flags, INT_VAL_MIN());
          return m;
        }
        /// Test that \a root keeps every supported value, and if \a exact
        /// only those (as hulls without a value set).
        bool root_supports(FmaSpace& root, const Tuples& expected, bool exact,
                           bool status = true) {
          if (root.status() == SS_FAILED)
            return expected.empty();
          if (expected.empty())
            return !exact;
          for (int column = 0; column < (status ? 5 : 4); ++column) {
            std::set<int> supports;
            for (const Tuple& tuple : expected)
              supports.insert(tuple[column]);
            for (int value : supports)
              if (column == 4 ? !root.flags.in(value)
                              : !root.x[column].in(WordValue(value)))
                return false;
            if (exact && (column == 4
                              ? root.flags.size() != supports.size()
                              : !projected(root.x[column], supports)))
              return false;
          }
          return true;
        }
        Tuples solutions(FmaSpace& root, bool status = true) {
          DFS<FmaSpace> search(&root);
          Tuples tuples;
          while (std::unique_ptr<FmaSpace> solution{search.next()}) {
            Tuple tuple{};
            for (int i = 0; i < 4; ++i)
              tuple[i] = code(solution->x[i]);
            tuple[4] = status ? solution->flags.val() : -1;
            tuples.insert(tuple);
          }
          return tuples;
        }
        class Symbolic : public Base {
        public:
          Symbolic(void) : Base("Word::Float::Fma::Symbolic") {}
          virtual bool run(void) {
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                  const Oracle oracle(format);
                  const bool fp4 = format == WDT_E2M1;
                  const bool exact = ipl == IPL_DOM;
                  const Domains workloads[] = {
                      fp4 ? Domains{{range(2, 4),
                                     range(3, 5),
                                     range(12, 15),
                                     {1},
                                     range(0, 31)}}
                          : Domains{{range(0x37, 0x3b),
                                     range(0x34, 0x38),
                                     range(0xb6, 0xba),
                                     {0x88},
                                     range(0, 31)}},
                      fp4 ? Domains{{range(2, 4),
                                     range(2, 3),
                                     range(0, 7),
                                     {4},
                                     {0}}}
                          : Domains{{range(0x38, 0x3b), range(0x38, 0x3b),
                                     range(0x38, 0x47), range(0x40, 0x42),
                                     range(0, 31)}},
                      fp4 ? Domains{{range(0, 7),
                                     range(2, 3),
                                     range(0, 2),
                                     {4},
                                     {0}}}
                          : Domains{{range(0x38, 0x57), range(0x38, 0x3b),
                                     range(0x38, 0x3b), range(0x41, 0x42),
                                     range(0, 31)}}};
                  for (const Domains& wanted : workloads) {
                    Model m = model(oracle, Policy(), wanted,
                                    Alias::Independent, true, value_set, ipl);
                    const Tuples expected =
                        expected_tuples(oracle, Policy(), m.domains);
                    FmaSpace& root = *m.space;
                    if (expected.empty() ||
                        !root_supports(root, expected, exact) ||
                        solutions(root) != expected)
                      return false;
                    if (exact && values(root.x[0]) == m.domains[0] &&
                        values(root.x[1]) == m.domains[1] &&
                        values(root.x[2]) == m.domains[2])
                      return false;
                  }
                  // Without a value set, the E4M3FN codes have small hulls
                  // that cover signs, overflow and the NaN.
                  const auto codes =
                      fp4 ? range(0, 15)
                      : value_set == WVS_EXACT
                          ? std::vector<int>{0,  1,  7,  8,   16,  20, 24,
                                             48, 56, 57, 128, 184, 255}
                          : cube(0x36, 0xc9);
                  const Domains wanted{{codes, codes, codes,
                                        range(0, 2 * oracle.sign - 1),
                                        range(0, 31)}};
                  for (Alias alias :
                       {Alias::Independent, Alias::RepeatedMultiplicands,
                        Alias::RepeatedAddend, Alias::ResultIsFirst,
                        Alias::ResultIsAddend, Alias::AllShared,
                        Alias::RepeatedInputs}) {
                    Model m = model(oracle, Policy(), wanted, alias, true,
                                    value_set, ipl);
                    const Tuples expected =
                        expected_tuples(oracle, Policy(), m.domains, alias);
                    if (!root_supports(*m.space, expected, exact) ||
                        solutions(*m.space) != expected)
                      return false;
                  }
                  Model omitted = model(oracle, Policy(), wanted,
                                        Alias::Independent, false, value_set,
                                        ipl);
                  const Tuples expected =
                      expected_tuples(oracle, Policy(), omitted.domains,
                                      Alias::Independent, false);
                  if (!root_supports(*omitted.space, expected, exact, false) ||
                      solutions(*omitted.space, false) != expected)
                    return false;
                }
            const Oracle fp4(WDT_E2M1);
            const Domains impossible{{{6}, {2}, {0, 2, 3}, {7}, {0}}};
            if (!expected_tuples(fp4, Policy(), impossible).empty())
              return false;
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                Model m = model(fp4, Policy(), impossible, Alias::Independent,
                                true, value_set, ipl);
                if (m.space->status() != SS_FAILED)
                  return false;
              }
            return true;
          }
        };
        /// IPL_DOM enumerates every tuple, however many there are.
        class Unlimited : public Base {
        public:
          Unlimited(void) : Base("Word::Float::Fma::Unlimited") {}
          virtual bool run(void) {
            const Oracle oracle(WDT_E4M3FN);
            const Policy policy;
            // A fully aliased input has only 256 assignments, despite the
            // apparent 256^3 Cartesian product of operand columns.
            const Domains full{{range(0, 255), range(0, 255), range(0, 255),
                                range(0, 255), range(0, 31)}};
            Tuples diagonal;
            for (int a = 0; a < 256; ++a) {
              const Result r = oracle.evaluate(policy, a, a, a);
              diagonal.insert(Tuple{{a, a, a, r.code, r.flags}});
            }
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
              Model shared = model(oracle, policy, full, Alias::RepeatedInputs,
                                   true, WVS_EXACT, ipl);
              if (!root_supports(*shared.space, diagonal, true) ||
                  solutions(*shared.space) != diagonal)
                return false;
            }
            // 33*32*32 tuples, more than the 32,768 that once deferred
            // propagation. IPL_DOM prunes exactly, the default only by
            // bounds.
            const Domains wide{
                {range(0, 32), range(32, 63), range(160, 191), {0}, {0}}};
            const Tuples expected = expected_tuples(oracle, policy, wide);
            Model domain = model(oracle, policy, wide, Alias::Independent,
                                 true, WVS_EXACT, IPL_DOM);
            Model bounds = model(oracle, policy, wide, Alias::Independent,
                                 true, WVS_EXACT, IPL_DEF);
            if (expected.empty() ||
                !root_supports(*domain.space, expected, true) ||
                !root_supports(*bounds.space, expected, false) ||
                values(bounds.space->x[0]).size() <=
                    values(domain.space->x[0]).size())
              return false;
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
              for (bool word : {false, true}) {
                FmaSpace& source = ipl == IPL_DOM ? *domain.space : *bounds.space;
                const std::vector<int> before = values(source.x[0]);
                std::unique_ptr<FmaSpace> clone(
                    static_cast<FmaSpace*>(source.clone()));
                if (clone->x[0].varimp() == source.x[0].varimp())
                  return false;
                // Removing one interior code wakes the propagator without
                // changing bounds; the Word restrictions change them too.
                rel(*clone, clone->x[0], WRT_NQ, WordValue(16));
                Domains restricted = wide;
                restricted[0].erase(std::find(restricted[0].begin(),
                                              restricted[0].end(), 16));
                if (word) {
                  dom(*clone, clone->x[0], WordValue(0), WordValue(31));
                  float_bounds(*clone, clone->x[0], WordValue(4),
                               WordValue(31));
                  restricted[0] = range(4, 31);
                  restricted[0].erase(std::find(restricted[0].begin(),
                                                restricted[0].end(), 16));
                }
                const Tuples kept = expected_tuples(oracle, policy, restricted);
                Model fresh = model(oracle, policy, restricted,
                                    Alias::Independent, true, WVS_EXACT, ipl);
                if (kept.empty() ||
                    !root_supports(*clone, kept, ipl == IPL_DOM) ||
                    !root_supports(*fresh.space, kept, ipl == IPL_DOM) ||
                    solutions(*clone) != kept ||
                    solutions(*fresh.space) != kept ||
                    values(source.x[0]) != before)
                  return false;
                std::unique_ptr<FmaSpace> invalid(
                    static_cast<FmaSpace*>(source.clone()));
                dom(*invalid, invalid->x[0], WordValue(0));
                dom(*invalid, invalid->x[1], WordValue(32));
                dom(*invalid, invalid->x[2], WordValue(160));
                if (invalid->status() != SS_FAILED)
                  return false;
              }
            return true;
          }
        };

        using GraphTuple = std::array<int, 9>;
        using GraphTuples = std::set<GraphTuple>;
        class GraphSpace : public Space {
        public:
          WordVar x[6];
          IntVarArray flags;
          GraphSpace(WordDomainType format, IntPropLevel ipl)
              : flags(*this, 3, 0, 31) {
            const bool fp4 = format == WDT_E2M1;
            for (auto& v : x)
              v = WordVar(*this, format, WVS_EXACT);
            restrict(*this, x[0], range(fp4 ? 2 : 0x37, fp4 ? 4 : 0x3b));
            restrict(*this, x[1], range(fp4 ? 3 : 0x34, fp4 ? 5 : 0x38));
            restrict(*this, x[2],
                     range(fp4 ? 12 : 0xb6, fp4 ? 15 : 0xba));
            fp_fma(*this, x[0], x[1], x[2], x[3], flags[0], ipl);
            fp_multiply(*this, x[0], x[1], x[4], flags[1], ipl);
            fp_add(*this, x[4], x[2], x[5], flags[2], ipl);
            rel(*this, x[5], WRT_EQ, WordValue(0));
            WordVarArgs words(6);
            for (int i = 0; i < 6; ++i)
              words[i] = x[i];
            branch(*this, words, WORD_VAL_LSB());
            branch(*this, flags, INT_VAR_NONE(), INT_VAL_MIN());
          }
          GraphSpace(GraphSpace& s) : Space(s) {
            for (int i = 0; i < 6; ++i)
              x[i].update(*this, s.x[i]);
            flags.update(*this, s.flags);
          }
          virtual Space* copy(void) { return new GraphSpace(*this); }
        };
        class Connected : public Base {
        public:
          Connected(void) : Base("Word::Float::Fma::Connected") {}
          virtual bool run(void) {
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN}) {
              const bool fp4 = format == WDT_E2M1;
              const Oracle oracle(format);
              const int fused_code = fp4 ? 1 : 0x88;
              GraphTuples expected;
              for (int a : range(fp4 ? 2 : 0x37, fp4 ? 4 : 0x3b))
                for (int b : range(fp4 ? 3 : 0x34, fp4 ? 5 : 0x38))
                  for (int c : range(fp4 ? 12 : 0xb6, fp4 ? 15 : 0xba)) {
                    const Result fused = oracle.evaluate(Policy(), a, b, c);
                    // Zero with the product's sign preserves multiplication's
                    // zero sign; multiplying by one implements exact
                    // addition.
                    const Result product = oracle.evaluate(
                        Policy(), a, b, ((a ^ b) & oracle.sign));
                    const Result separate = oracle.evaluate(
                        Policy(), fp4 ? 2 : 56, product.code, c);
                    if (fused.code == fused_code && separate.code == 0)
                      expected.insert(GraphTuple{
                          {a, b, c, fused.code, product.code, separate.code,
                           fused.flags, product.flags, separate.flags}});
                  }
              if (expected.size() != 1 || (*expected.begin())[6] != 0 ||
                  (*expected.begin())[7] != 16)
                return false;
              const GraphTuple& witness = *expected.begin();
              if (witness[0] != (fp4 ? 3 : 0x39) ||
                  witness[1] != (fp4 ? 5 : 0x36) ||
                  witness[2] != (fp4 ? 14 : 0xb8))
                return false;
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                GraphSpace root(format, ipl);
                if (root.status() == SS_FAILED ||
                    root.x[0].assigned())
                  return false;
                // A Word restriction posted after stabilization reaches FMA,
                // its shared inputs, and the separate
                // multiplication/addition.
                dom(root, root.x[3], WordValue(fused_code));
                if (root.status() == SS_FAILED)
                  return false;
                for (int i = 0; i < 6; ++i)
                  if (!root.x[i].in(WordValue(witness[i])) ||
                      (ipl == IPL_DOM && !root.x[i].assigned()))
                    return false;
                DFS<GraphSpace> search(&root);
                GraphTuples actual;
                while (std::unique_ptr<GraphSpace> solution{search.next()}) {
                  GraphTuple tuple{};
                  for (int i = 0; i < 6; ++i)
                    tuple[i] = code(solution->x[i]);
                  for (int i = 0; i < 3; ++i)
                    tuple[6 + i] = solution->flags[i].val();
                  actual.insert(tuple);
                }
                if (actual != expected)
                  return false;
              }
            }
            return true;
          }
        };
        class Posting : public Base {
          /// Plain float-typed words and constants propagate exactly once
          /// their inputs are assigned.
          static bool plain_words(void) {
            const Oracle oracle(WDT_E2M1);
            for (bool status : {false, true})
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                FmaSpace s(WDT_E2M1, WVS_EXACT);
                s.x[0] = float_code(s, WDT_E2M1, 3);
                s.x[1] = WordVar(s, WDT_E2M1);
                s.x[2] = float_code(s, WDT_E2M1, 10);
                s.x[3] = WordVar(s, WDT_E2M1);
                s.post(Policy(), status, ipl);
                dom(s, s.x[1], WordValue(5));
                const Result expected = oracle.evaluate(Policy(), 3, 5, 10);
                if (s.status() == SS_FAILED || !s.x[3].assigned() ||
                    code(s.x[3]) != expected.code ||
                    (status && (!s.flags.assigned() ||
                                s.flags.val() != expected.flags)))
                  return false;
              }
            return true;
          }

        public:
          Posting(void) : Base("Word::Float::Fma::Posting") {}
          virtual bool run(void) {
            int caught = 0;
            for (bool status : {false, true}) {
              FmaSpace s(WDT_E2M1, WVS_EXACT);
              WordVar wide(s, WDT_BINARY16);
              try {
                if (status)
                  fp_fma(s, wide, wide, wide, wide, s.flags);
                else
                  fp_fma(s, wide, wide, wide, wide);
              } catch (const Gecode::Word::OutOfLimits&) {
                ++caught;
              }
              WordVar fp8(s, WDT_E4M3FN);
              for (int column = 1; column < 4; ++column) {
                try {
                  if (status)
                    fp_fma(s, s.x[0], column == 1 ? fp8 : s.x[1],
                           column == 2 ? fp8 : s.x[2],
                           column == 3 ? fp8 : s.x[3], s.flags);
                  else
                    fp_fma(s, s.x[0], column == 1 ? fp8 : s.x[1],
                           column == 2 ? fp8 : s.x[2],
                           column == 3 ? fp8 : s.x[3]);
                } catch (const Gecode::Word::FormatMismatch&) {
                  ++caught;
                }
              }
            }
            return caught == 8 && plain_words();
          }
        };

        /// Focused default-level deductions on E4M3FN words
        class TinyBounds : public Base {
          static constexpr int half = 0x30, one = 0x38, one_half = 0x3c,
                               two = 0x40, two_half = 0x42, three = 0x44,
                               three_half = 0x46, five = 0x4a, nan = 0x7f;
          static Model bounds_model(const std::vector<int>& a,
                                    const std::vector<int>& b,
                                    const std::vector<int>& c,
                                    const std::vector<int>& result,
                                    WordValueSetOption value_set,
                                    IntPropLevel ipl) {
            const Oracle oracle(WDT_E4M3FN);
            return model(oracle, Policy(),
                         Domains{{a, b, c, result, range(0, 31)}},
                         Alias::Independent, true, value_set, ipl);
          }
          static bool keeps(const WordVar& x, const std::vector<int>& p) {
            return projected(x, std::set<int>(p.begin(), p.end()));
          }
          /// The result keeps the rounded hull of the exact results 2.5 to 5.
          static bool results(WordValueSetOption value_set) {
            Model m = bounds_model({one, one_half}, {two, three}, {half},
                                   range(0, 255), value_set, IPL_DEF);
            return m.space->status() != SS_FAILED &&
                   keeps(m.space->x[3], range(two_half, five));
          }
          /// Results 2.5 to 3 leave the multiplicands whose bounds can reach
          /// them; IPL_DOM also removes 1.375, whose least exact result 3.25
          /// rounds above 3.
          static bool operands(WordValueSetOption value_set) {
            Model bounds = bounds_model(range(0x38, 0x3f), range(0x40, 0x47),
                                        range(0x30, 0x37),
                                        range(two_half, three), value_set,
                                        IPL_DEF);
            Model domain = bounds_model(range(0x38, 0x3f), range(0x40, 0x47),
                                        range(0x30, 0x37),
                                        range(two_half, three), value_set,
                                        IPL_DOM);
            FmaSpace& s = *bounds.space;
            return s.status() != SS_FAILED &&
                   domain.space->status() != SS_FAILED &&
                   keeps(s.x[0], range(0x38, 0x3b)) &&
                   keeps(s.x[1], range(0x40, 0x43)) &&
                   keeps(s.x[2], range(0x30, 0x37)) &&
                   keeps(domain.space->x[0], range(0x38, 0x3a));
          }
          /// A NaN operand keeps its NaN result beside the numbers.
          static bool nans(WordValueSetOption value_set) {
            Model m = bounds_model({one, nan}, {two, three}, {half},
                                   range(0, 255), value_set, IPL_DEF);
            FmaSpace& s = *m.space;
            if (s.status() == SS_FAILED)
              return false;
            for (int v : {two_half, three_half, nan})
              if (!s.x[3].in(WordValue(v)))
                return false;
            if (value_set == WVS_EXACT)
              return keeps(s.x[3], {two_half, 0x43, three, 0x45, three_half,
                                    nan});
            return true;
          }

        public:
          TinyBounds(void) : Base("Word::Float::Fma::TinyBounds") {}
          virtual bool run(void) {
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              if (!results(value_set) || !operands(value_set) ||
                  !nans(value_set))
                return false;
            return true;
          }
        };

        /// Random restrictions, aliases and policies on both levels
        class TinyRoot : public Base {
          /// Codes that differ from a random base in up to four random bits
          std::vector<int> random_codes(WordDomainType format) {
            const unsigned int width = float_word_width(format);
            const int base = static_cast<int>(_rand(1U << width));
            int free = 0;
            for (unsigned int i = 0, n = 1 + _rand(4U); i < n; ++i)
              free |= 1 << _rand(width);
            std::vector<int> codes;
            while (codes.empty())
              for (int v : cube(0, free))
                if (_rand(2U) == 0)
                  codes.push_back(base ^ v);
            return codes;
          }
          static std::array<Policy, 4> policies(void) {
            return {Policy(),
                    Policy{Policy::RUP, Policy::SignedZero, Policy::SignedZero,
                           Policy::SaturateFinite, Policy::Positive,
                           Policy::Canonical, Policy::BeforeRounding,
                           Policy::Signaling},
                    Policy{Policy::RTZ, Policy::Preserve, Policy::SignedZero,
                           Policy::Format, Policy::Signed,
                           Policy::OperandFirst},
                    Policy{Policy::RDN, Policy::SignedZero, Policy::Preserve,
                           Policy::Format, Policy::Signed, Policy::SignalFirst,
                           Policy::BeforeRounding}};
          }
          // Propagation keeps every tuple that the oracle supports, search
          // finds exactly those tuples, and IPL_DOM keeps their projections.
          bool keeps_supported(WordDomainType format,
                               WordValueSetOption value_set, IntPropLevel ipl,
                               const Policy& policy) {
            const Oracle oracle(format);
            FmaSpace root(format, value_set);
            const Alias choices[] = {
                Alias::Independent,   Alias::RepeatedMultiplicands,
                Alias::RepeatedAddend, Alias::ResultIsFirst,
                Alias::ResultIsAddend, Alias::AllShared,
                Alias::RepeatedInputs};
            const Alias alias = choices[_rand(7U)];
            share(root, alias);
            const bool same_multiplicands =
                alias == Alias::RepeatedMultiplicands ||
                alias == Alias::AllShared || alias == Alias::RepeatedInputs;
            const bool same_addend = alias == Alias::RepeatedAddend ||
                                     alias == Alias::AllShared ||
                                     alias == Alias::RepeatedInputs;
            restrict(root, root.x[0], random_codes(format));
            if (!same_multiplicands)
              restrict(root, root.x[1], random_codes(format));
            if (!same_addend)
              restrict(root, root.x[2], random_codes(format));
            // Results and flags are restricted around reachable outcomes.
            std::vector<Result> outcomes;
            for (int a : values(root.x[0]))
              for (int b : values(root.x[1]))
                for (int c : values(root.x[2]))
                  if ((!same_multiplicands || a == b) &&
                      (!same_addend || a == c))
                    outcomes.push_back(oracle.evaluate(policy, a, b, c));
            if (alias == Alias::Independent && _rand(2U) == 0) {
              std::vector<int> results;
              for (const Result& r : outcomes)
                if (_rand(2U) == 0)
                  results.push_back(r.code);
              results.push_back(static_cast<int>(_rand(2U * oracle.sign)));
              restrict(root, root.x[3], results);
            }
            const int flags =
                outcomes[_rand(static_cast<unsigned int>(outcomes.size()))]
                    .flags;
            switch (_rand(3U)) {
            case 0:
              rel(root, root.flags, IRT_EQ, flags);
              break;
            case 1:
              rel(root, root.flags, IRT_NQ, flags);
              break;
            default:
              break;
            }
            const Domains domains{{values(root.x[0]),
                                   values(root.x[1]),
                                   values(root.x[2]),
                                   values(root.x[3]),
                                   values(root.flags)}};
            const Tuples expected =
                expected_tuples(oracle, policy, domains, alias);
            root.post(policy, true, ipl);
            if (!root_supports(root, expected, ipl == IPL_DOM))
              return false;
            if (root.status() == SS_FAILED)
              return true;
            branch(root,
                   WordVarArgs{root.x[0], root.x[1],
                               root.x[2], root.x[3]},
                   WORD_VAL_LSB());
            branch(root, root.flags, INT_VAL_MIN());
            return solutions(root) == expected;
          }

        public:
          TinyRoot(void) : Base("Word::Float::Fma::TinyRoot") {}
          virtual bool run(void) {
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                  for (const Policy& policy : policies())
                    for (int trial = 0; trial < 3; ++trial)
                      if (!keeps_supported(format, value_set, ipl, policy))
                        return false;
            return true;
          }
        };
        /// Complete solutions on float-typed words whose windows are negative
        /// or cross zero, or whose cube leaves the sign unknown
        class FloatTyped : public Base {
          // Total-order window of codes
          struct Range {
            int minimum, maximum;
          };
          // Operand windows and result window; a cube case gives the addend
          // the known ones and possible ones c.minimum and c.maximum on its
          // full domain instead.
          struct Case {
            Range a, b, c, result;
            bool cube;
          };
          static WordVar window(Space& home, WordDomainType format,
                                Range r, WordValueSetOption value_set) {
            return float_window(home, format, WordValue(r.minimum),
                                WordValue(r.maximum), value_set);
          }
          static bool check(const Oracle& oracle, const Policy& policy,
                            const Case& k, WordValueSetOption value_set,
                            IntPropLevel ipl) {
            const WordDomainType format = oracle.format;
            const Range all{oracle.sign | (oracle.sign - 1), oracle.sign - 1};
            FmaSpace s(format, value_set);
            s.x[0] = window(s, format, k.a, value_set);
            s.x[1] = window(s, format, k.b, value_set);
            s.x[2] = window(s, format, k.cube ? all : k.c, value_set);
            if (k.cube)
              dom(s, s.x[2], WordValue(k.c.minimum),
                  WordValue(k.c.maximum));
            s.x[3] = window(s, format, k.result, value_set);
            Tuples expected;
            for (int a : values(s.x[0]))
              for (int b : values(s.x[1]))
                for (int c : values(s.x[2])) {
                  const Result r = oracle.evaluate(policy, a, b, c);
                  if (s.x[3].in(WordValue(r.code)))
                    expected.insert({a, b, c, r.code, r.flags});
                }
            s.post(policy, true, ipl);
            branch(s,
                   WordVarArgs{s.x[0], s.x[1], s.x[2],
                               s.x[3]},
                   WORD_VAL_LSB());
            branch(s, s.flags, INT_VAL_MIN());
            if (solutions(s) == expected)
              return true;
            if (opt.log)
              olog << "float-typed tiny FMA " << format << " rounding "
                   << policy.rounding << " value set "
                   << static_cast<int>(value_set) << " ipl " << ipl << std::hex
                   << " a " << k.a.minimum << ".." << k.a.maximum << " b "
                   << k.b.minimum << ".." << k.b.maximum << " c "
                   << k.c.minimum << ".." << k.c.maximum << std::dec
                   << " cube " << k.cube << '\n';
            return false;
          }
          static bool format_cases(WordDomainType format) {
            const Oracle oracle(format);
            const int s = oracle.sign;
            const Range all{s | (s - 1), s - 1};
            const std::vector<Case> cases =
                format == WDT_E2M1
                    ? std::vector<Case>{{{s | 3, 3},
                                         {s | 5, s | 2},
                                         {s | 2, 2},
                                         all,
                                         false},
                                        {{s | 6, s | 3},
                                         {2, 4},
                                         {s | 4, s | 1},
                                         {s | 7, s | 3},
                                         false},
                                        {{s | 2, 2},
                                         {s | 2, 2},
                                         {0, 0xb},
                                         all,
                                         true}}
                    : std::vector<Case>{{{s | 3, 3},
                                         {s | 0x3a, s | 0x38},
                                         {s | 2, 2},
                                         all,
                                         false},
                                        {{s | 0x3a, s | 0x38},
                                         {0x38, 0x3a},
                                         {s | 0x3c, s | 0x38},
                                         {s | 0x48, s | 0x40},
                                         false},
                                        {{s | 0x39, 0x39},
                                         {0x38, 0x39},
                                         {0x30, 0xb3},
                                         all,
                                         true}};
            const Policy policies[] = {
                Policy(), Policy{Policy::RUP},
                Policy{Policy::RDN, Policy::SignedZero, Policy::SignedZero}};
            for (const Case& k : cases)
              for (const Policy& policy : policies)
                for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
                  for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                    if (!check(oracle, policy, k, value_set, ipl))
                      return false;
            return true;
          }

        public:
          FloatTyped(void) : Base("Word::Float::Fma::FloatTyped") {}
          virtual bool run(void) {
            return format_cases(WDT_E2M1) && format_cases(WDT_E4M3FN);
          }
        };
        Exhaustive exhaustive;
        Policies policies;
        Symbolic symbolic;
        Unlimited unlimited;
        Connected connected;
        Posting posting;
        TinyBounds tiny_bounds;
        TinyRoot tiny_root;
        FloatTyped float_typed;
      } // namespace Fma
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
