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
        Rational magnitude(FloatWordFormat format, int code) {
          const int fraction = format == FWF_E2M1 ? 1 : 3;
          const int bias = format == FWF_E2M1 ? 1 : 7;
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
          FloatWordFormat format;
          int sign, maximum, normal;
          std::vector<Neighbor> finite, precision;
          explicit Oracle(FloatWordFormat f)
              : format(f), sign(f == FWF_E2M1 ? 8 : 128),
                maximum(f == FWF_E2M1 ? 7 : 126),
                normal(f == FWF_E2M1 ? 2 : 8) {
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
            if (format == FWF_E4M3FN)
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
                  format == FWF_E4M3FN && policy.overflow == Policy::Format;
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

        class FmaSpace : public Space {
        public:
          FloatWordVar x[4];
          IntVar flags;
          explicit FmaSpace(FloatWordFormat format) : flags(*this, -4, 40) {
            for (auto& v : x)
              v = FloatWordVar(
                  *this, format,
                  WordVar(*this, float_word_width(format), WDT_UNSIGNED),
                  IntVar(*this, 0, (1 << float_word_width(format)) - 1));
          }
          FmaSpace(FmaSpace& s) : Space(s) {
            for (int i = 0; i < 4; ++i)
              x[i].update(*this, s.x[i]);
            flags.update(*this, s.flags);
          }
          virtual Space* copy(void) { return new FmaSpace(*this); }
          void post(const Policy& policy, bool status = true) {
            if (status)
              fp_fma(*this, policy, x[0], x[1], x[2], x[3], flags);
            else
              fp_fma(*this, policy, x[0], x[1], x[2], x[3]);
          }
        };
        bool check_triple(FmaSpace& source, const Oracle& oracle,
                          const Policy& policy, int a, int b, int c) {
          const Result expected = oracle.evaluate(policy, a, b, c);
          std::unique_ptr<FmaSpace> s(static_cast<FmaSpace*>(source.clone()));
          rel(*s, s->x[0].encoding(), IRT_EQ, a);
          rel(*s, s->x[1].encoding(), IRT_EQ, b);
          rel(*s, s->x[2].encoding(), IRT_EQ, c);
          s->post(policy);
          if (s->status() != SS_FAILED && s->x[3].encoding().assigned() &&
              s->flags.assigned() &&
              s->x[3].encoding().val() == expected.code &&
              s->flags.val() == expected.flags && s->x[3].bits().assigned() &&
              s->x[3].bits().val() == static_cast<WordValue>(expected.code))
            return true;
          std::cerr << "tiny FMA mismatch format=" << oracle.format
                    << " inputs=" << a << ',' << b << ',' << c
                    << " expected=" << expected.code << ',' << expected.flags
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
            const Oracle oracle(FWF_E2M1);
            FmaSpace source(FWF_E2M1);
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
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const Oracle oracle(format);
              FmaSpace source(format);
              if (source.status() == SS_FAILED)
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
                              for (const auto& t : format == FWF_E2M1
                                                       ? fp4_cases
                                                       : fp8_cases) {
                                if (!check_triple(source, oracle, policy,
                                                  t[0], t[1], t[2]))
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
            const Oracle oracle(FWF_E4M3FN);
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
        std::vector<int> range(int first, int last) {
          std::vector<int> values;
          for (int i = first; i <= last; ++i)
            values.push_back(i);
          return values;
        }
        bool contains(const std::vector<int>& values, int value) {
          return std::find(values.begin(), values.end(), value) !=
                 values.end();
        }
        enum class Alias {
          Independent,
          RepeatedMultiplicands,
          RepeatedAddend,
          ResultIsFirst,
          ResultIsAddend,
          StatusIsFirst,
          StatusIsResult,
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
          case Alias::StatusIsFirst:
            return t[0] == t[4];
          case Alias::StatusIsResult:
            return t[3] == t[4];
          case Alias::AllShared:
            return t[0] == t[1] && t[0] == t[2] && t[0] == t[3] &&
                   t[0] == t[4];
          case Alias::RepeatedInputs:
            return t[0] == t[1] && t[0] == t[2];
          case Alias::Independent:
            return true;
          }
          GECODE_NEVER;
          return false;
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
        std::unique_ptr<FmaSpace>
        model(const Oracle& oracle, const Policy& policy,
              const Domains& domains, Alias alias = Alias::Independent,
              bool status = true, bool word = false) {
          std::unique_ptr<FmaSpace> s(new FmaSpace(oracle.format));
          if (alias == Alias::RepeatedMultiplicands || alias == Alias::AllShared ||
              alias == Alias::RepeatedInputs)
            s->x[1] = s->x[0];
          if (alias == Alias::RepeatedAddend || alias == Alias::AllShared ||
              alias == Alias::RepeatedInputs)
            s->x[2] = s->x[0];
          if (alias == Alias::ResultIsFirst || alias == Alias::AllShared)
            s->x[3] = s->x[0];
          if (alias == Alias::ResultIsAddend)
            s->x[3] = s->x[2];
          if (alias == Alias::StatusIsFirst || alias == Alias::AllShared)
            s->flags = s->x[0].encoding();
          if (alias == Alias::StatusIsResult)
            s->flags = s->x[3].encoding();
          for (int i = 0; i < 4; ++i)
            dom(*s, s->x[i].encoding(),
                IntSet(domains[i].data(),
                       static_cast<int>(domains[i].size())));
          if (status)
            dom(*s, s->flags,
                IntSet(domains[4].data(),
                       static_cast<int>(domains[4].size())));
          s->post(policy, status);
          if (word)
            branch(*s,
                   WordVarArgs{s->x[0].bits(), s->x[1].bits(), s->x[2].bits(),
                               s->x[3].bits()},
                   WORD_VAL_LSB());
          else
            branch(*s,
                   IntVarArgs{s->x[0].encoding(), s->x[1].encoding(),
                              s->x[2].encoding(), s->x[3].encoding()},
                   INT_VAR_NONE(), INT_VAL_MIN());
          if (status)
            branch(*s, s->flags, INT_VAL_MIN());
          return s;
        }
        bool root_supports(FmaSpace& root, const Tuples& expected,
                           bool status = true) {
          if (root.status() == SS_FAILED)
            return expected.empty();
          if (expected.empty())
            return false;
          for (int column = 0; column < (status ? 5 : 4); ++column) {
            IntVar variable =
                column == 4 ? root.flags : root.x[column].encoding();
            std::set<int> supports;
            for (const Tuple& tuple : expected)
              supports.insert(tuple[column]);
            if (variable.size() != supports.size())
              return false;
            for (int value : supports)
              if (!variable.in(value))
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
              tuple[i] = solution->x[i].encoding().val();
            tuple[4] = status ? solution->flags.val() : -1;
            tuples.insert(tuple);
          }
          return tuples;
        }
        class Symbolic : public Base {
        public:
          Symbolic(void) : Base("Word::Float::Fma::Symbolic") {}
          virtual bool run(void) {
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const Oracle oracle(format);
              const bool fp4 = format == FWF_E2M1;
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
              for (const Domains& domains : workloads)
                for (bool word : {false, true}) {
                  const Tuples expected =
                      expected_tuples(oracle, Policy(), domains);
                  auto root = model(oracle, Policy(), domains, Alias::Independent,
                                    true, word);
                  if (expected.empty() || !root_supports(*root, expected) ||
                      solutions(*root) != expected)
                    return false;
                  if (root->x[0].encoding().size() == domains[0].size() &&
                      root->x[1].encoding().size() == domains[1].size() &&
                      root->x[2].encoding().size() == domains[2].size())
                    return false;
                }
              const auto codes =
                  fp4 ? range(0, 15)
                      : std::vector<int>{0,  1,  7,  8,   16,  20, 24,
                                         48, 56, 57, 128, 184, 255};
              const Domains domains{{codes, codes, codes,
                                     range(0, 2 * oracle.sign - 1),
                                     range(0, 31)}};
              for (Alias alias :
                   {Alias::Independent, Alias::RepeatedMultiplicands, Alias::RepeatedAddend,
                    Alias::ResultIsFirst, Alias::ResultIsAddend, Alias::StatusIsFirst,
                    Alias::StatusIsResult, Alias::AllShared, Alias::RepeatedInputs})
                for (bool word : {false, true}) {
                  const Tuples expected =
                      expected_tuples(oracle, Policy(), domains, alias);
                  auto root =
                      model(oracle, Policy(), domains, alias, true, word);
                  if (!root_supports(*root, expected) ||
                      solutions(*root) != expected)
                    return false;
                }
              const Tuples expected = expected_tuples(
                  oracle, Policy(), domains, Alias::Independent, false);
              auto omitted =
                  model(oracle, Policy(), domains, Alias::Independent, false);
              if (!root_supports(*omitted, expected, false) ||
                  solutions(*omitted, false) != expected)
                return false;
            }
            const Oracle fp4(FWF_E2M1);
            const Domains impossible{{{6}, {2}, {0, 2, 3}, {7}, {0}}};
            auto root = model(fp4, Policy(), impossible);
            return expected_tuples(fp4, Policy(), impossible).empty() &&
                   root->status() == SS_FAILED;
          }
        };
        class Deferred : public Base {
        public:
          Deferred(void) : Base("Word::Float::Fma::Deferred") {}
          virtual bool run(void) {
            const Oracle oracle(FWF_E4M3FN);
            const Policy policy;
            const Domains full{{range(0, 255), range(0, 255), range(0, 255),
                                range(0, 255), range(0, 31)}};
            auto broad = model(oracle, policy, full);
            if (broad->status() == SS_FAILED ||
                broad->x[0].encoding().size() != 256 ||
                broad->x[3].encoding().size() != 256 ||
                broad->flags.size() != 32)
              return false;
            // A fully aliased input has only 256 assignments, despite the
            // apparent 256^3 Cartesian product of operand columns.
            Tuples diagonal;
            for (int a = 0; a < 256; ++a) {
              const Result r = oracle.evaluate(policy, a, a, a);
              diagonal.insert(Tuple{{a, a, a, r.code, r.flags}});
            }
            auto shared = model(oracle, policy, full, Alias::RepeatedInputs);
            if (!root_supports(*shared, diagonal) ||
                solutions(*shared) != diagonal)
              return false;

            const Domains deferred{
                {range(0, 32), range(32, 63), range(160, 191), {0}, {0}}};
            auto source = model(oracle, policy, deferred);
            if (source->status() == SS_FAILED ||
                source->x[0].encoding().size() != 33 ||
                source->x[1].encoding().size() != 32 ||
                source->x[2].encoding().size() != 32)
              return false;
            // 33*32*32 is over the cap. Removing one interior encoding
            // reaches exactly 32,768 and must wake the actor without changing
            // bounds.
            for (bool word : {false, true}) {
              std::unique_ptr<FmaSpace> clone(
                  static_cast<FmaSpace*>(source->clone()));
              if (clone->x[0].encoding().varimp() ==
                  source->x[0].encoding().varimp())
                return false;
              rel(*clone, clone->x[0].encoding(), IRT_NQ, 16);
              Domains restricted = deferred;
              restricted[0].erase(
                  std::find(restricted[0].begin(), restricted[0].end(), 16));
              if (word) {
                dom(*clone, clone->x[0].bits(), WordValue(0), WordValue(31));
                rel(*clone, clone->x[0].bits(), WRT_UGQ, WordValue(4));
                restricted[0] = range(4, 31);
                restricted[0].erase(std::find(restricted[0].begin(),
                                              restricted[0].end(), 16));
              }
              const Tuples expected =
                  expected_tuples(oracle, policy, restricted);
              auto fresh = model(oracle, policy, restricted);
              if (expected.empty() || !root_supports(*clone, expected) ||
                  !root_supports(*fresh, expected) ||
                  solutions(*clone) != expected ||
                  solutions(*fresh) != expected ||
                  clone->x[0].encoding().size() == restricted[0].size() ||
                  source->x[0].encoding().size() != 33 ||
                  !source->x[0].encoding().in(16))
                return false;
            }
            std::unique_ptr<FmaSpace> invalid(
                static_cast<FmaSpace*>(source->clone()));
            rel(*invalid, invalid->x[0].encoding(), IRT_EQ, 0);
            rel(*invalid, invalid->x[1].encoding(), IRT_EQ, 32);
            rel(*invalid, invalid->x[2].encoding(), IRT_EQ, 160);
            return invalid->status() == SS_FAILED;
          }
        };

        using GraphTuple = std::array<int, 9>;
        using GraphTuples = std::set<GraphTuple>;
        class GraphSpace : public Space {
        public:
          FloatWordVar x[6];
          IntVarArray flags;
          GraphSpace(FloatWordFormat format, bool word)
              : flags(*this, 3, 0, 31) {
            const bool fp4 = format == FWF_E2M1;
            for (auto& v : x)
              v = FloatWordVar(
                  *this, format,
                  WordVar(*this, float_word_width(format), WDT_UNSIGNED),
                  IntVar(*this, 0, fp4 ? 15 : 255));
            dom(*this, x[0].encoding(), fp4 ? 2 : 0x37, fp4 ? 4 : 0x3b);
            dom(*this, x[1].encoding(), fp4 ? 3 : 0x34, fp4 ? 5 : 0x38);
            dom(*this, x[2].encoding(), fp4 ? 12 : 0xb6, fp4 ? 15 : 0xba);
            fp_fma(*this, Policy(), x[0], x[1], x[2], x[3], flags[0]);
            fp_multiply(*this, Policy(), x[0], x[1], x[4], flags[1]);
            fp_add(*this, Policy(), x[4], x[2], x[5], flags[2]);
            rel(*this, x[5].encoding(), IRT_EQ, 0);
            if (word) {
              WordVarArgs words(6);
              for (int i = 0; i < 6; ++i)
                words[i] = x[i].bits();
              branch(*this, words, WORD_VAL_LSB());
            } else {
              IntVarArgs codes(6);
              for (int i = 0; i < 6; ++i)
                codes[i] = x[i].encoding();
              branch(*this, codes, INT_VAR_NONE(), INT_VAL_MIN());
            }
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
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const bool fp4 = format == FWF_E2M1;
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
              for (bool word : {false, true}) {
                GraphSpace root(format, word);
                if (root.status() == SS_FAILED ||
                    root.x[0].encoding().assigned())
                  return false;
                // A Word restriction posted after stabilization reaches FMA,
                // its shared inputs, and the separate
                // multiplication/addition.
                dom(root, root.x[3].bits(), WordValue(fused_code),
                    WordValue(fused_code));
                if (root.status() == SS_FAILED)
                  return false;
                for (int i = 0; i < 6; ++i)
                  if (!root.x[i].encoding().assigned() ||
                      root.x[i].encoding().val() != witness[i])
                    return false;
                DFS<GraphSpace> search(&root);
                GraphTuples actual;
                while (std::unique_ptr<GraphSpace> solution{search.next()}) {
                  GraphTuple tuple{};
                  for (int i = 0; i < 6; ++i)
                    tuple[i] = solution->x[i].encoding().val();
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
        public:
          Posting(void) : Base("Word::Float::Fma::Posting") {}
          virtual bool run(void) {
            int caught = 0;
            for (bool status : {false, true}) {
              FmaSpace s(FWF_E2M1);
              for (FloatWordFormat format : {FWF_BINARY16}) {
                FloatWordVar wide(s, format);
                try {
                  if (status)
                    fp_fma(s, Policy(), wide, wide, wide, wide, s.flags);
                  else
                    fp_fma(s, Policy(), wide, wide, wide, wide);
                } catch (const Gecode::Word::OutOfLimits&) {
                  ++caught;
                }
              }
              FloatWordVar fp8(s, FWF_E4M3FN);
              for (int column = 1; column < 4; ++column) {
                try {
                  if (status)
                    fp_fma(s, Policy(), s.x[0], column == 1 ? fp8 : s.x[1],
                           column == 2 ? fp8 : s.x[2],
                           column == 3 ? fp8 : s.x[3], s.flags);
                  else
                    fp_fma(s, Policy(), s.x[0], column == 1 ? fp8 : s.x[1],
                           column == 2 ? fp8 : s.x[2],
                           column == 3 ? fp8 : s.x[3]);
                } catch (const Gecode::Word::FormatMismatch&) {
                  ++caught;
                }
              }
              for (FloatWordVar unbound :
                   {FloatWordVar(s, FWF_E2M1),
                    FloatWordVar::from_bits(s, FWF_E2M1, 0)})
                for (int column = 0; column < 4; ++column) {
                  try {
                    if (status)
                      fp_fma(s, Policy(), column == 0 ? unbound : s.x[0],
                             column == 1 ? unbound : s.x[1],
                             column == 2 ? unbound : s.x[2],
                             column == 3 ? unbound : s.x[3], s.flags);
                    else
                      fp_fma(s, Policy(), column == 0 ? unbound : s.x[0],
                             column == 1 ? unbound : s.x[1],
                             column == 2 ? unbound : s.x[2],
                             column == 3 ? unbound : s.x[3]);
                  } catch (const Gecode::Word::OutOfLimits&) {
                    ++caught;
                  }
                }
            }
            return caught == 24;
          }
        };
        Exhaustive exhaustive;
        Policies policies;
        Symbolic symbolic;
        Deferred deferred;
        Connected connected;
        Posting posting;
      } // namespace Fma
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
