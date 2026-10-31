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
      namespace Arithmetic {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        enum class Operation { Add, Subtract, Multiply, Divide };
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
            // This range strictly encloses every nonzero exact tiny binary
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
            const auto upper =
                std::lower_bound(values.begin(), values.end(), exact,
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
          Result evaluate(Operation op, const Policy& policy, int a,
                          int b) const {
            int ac = a % sign, bc = b % sign;
            bool an = a >= sign, bn = b >= sign;
            if (format == FWF_E4M3FN && (ac == 127 || bc == 127))
              return {policy.nan == Policy::Canonical ? 127
                      : ac == 127                     ? a
                                                      : b,
                      0};
            if (policy.input_subnormals == Policy::SignedZero) {
              if (ac < normal)
                ac = 0;
              if (bc < normal)
                bc = 0;
            }
            if (op == Operation::Subtract)
              bn = !bn;
            const Rational av = finite[ac].value, bv = finite[bc].value;
            bool negative = an != bn;
            if (op == Operation::Divide && bc == 0) {
              if (ac == 0)
                return {format == FWF_E2M1 ? 7 : 127, 1};
              const bool nan =
                  format == FWF_E4M3FN && policy.overflow == Policy::Format;
              return {(nan ? 127 : maximum) +
                          (negative && !(nan && policy.nan == Policy::Canonical)
                               ? sign
                               : 0),
                      2};
            }
            Rational exact{};
            switch (op) {
            case Operation::Add:
            case Operation::Subtract:
              exact = {(an ? -av.numerator : av.numerator) * bv.denominator +
                           (bn ? -bv.numerator : bv.numerator) * av.denominator,
                       av.denominator * bv.denominator};
              negative = exact.numerator < 0 ||
                         (exact.numerator == 0 &&
                          (an == bn ? an : policy.rounding == Policy::RDN));
              if (exact.numerator < 0)
                exact.numerator = -exact.numerator;
              break;
            case Operation::Multiply:
              exact = {av.numerator * bv.numerator,
                       av.denominator * bv.denominator};
              break;
            case Operation::Divide:
              exact = {av.numerator * bv.denominator,
                       av.denominator * bv.numerator};
              break;
            }
            if (exact.numerator == 0)
              return {negative && policy.zeros == Policy::Signed ? sign : 0, 0};
            const Rational unbounded =
                precision[nearest(precision, exact, negative, policy.rounding)]
                    .value;
            if (less(finite[maximum].value, unbounded)) {
              const bool nan =
                  format == FWF_E4M3FN && policy.overflow == Policy::Format;
              return {(nan ? 127 : maximum) +
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

        void post(Home home, Operation op, const Policy& policy, FloatWordVar a,
                  FloatWordVar b, FloatWordVar result, IntVar* flags) {
          switch (op) {
          case Operation::Add:
            if (flags)
              fp_add(home, policy, a, b, result, *flags);
            else
              fp_add(home, policy, a, b, result);
            break;
          case Operation::Subtract:
            if (flags)
              fp_subtract(home, policy, a, b, result, *flags);
            else
              fp_subtract(home, policy, a, b, result);
            break;
          case Operation::Multiply:
            if (flags)
              fp_multiply(home, policy, a, b, result, *flags);
            else
              fp_multiply(home, policy, a, b, result);
            break;
          case Operation::Divide:
            if (flags)
              fp_divide(home, policy, a, b, result, *flags);
            else
              fp_divide(home, policy, a, b, result);
            break;
          }
        }
        class PairSpace : public Space {
        public:
          FloatWordVar x[3];
          IntVar flags;
          explicit PairSpace(FloatWordFormat format) : flags(*this, -4, 40) {
            for (auto& v : x)
              v = FloatWordVar(
                  *this, format,
                  WordVar(*this, float_word_width(format), WDT_UNSIGNED),
                  IntVar(*this, 0, (1 << float_word_width(format)) - 1));
          }
          PairSpace(PairSpace& s) : Space(s) {
            for (int i = 0; i < 3; ++i)
              x[i].update(*this, s.x[i]);
            flags.update(*this, s.flags);
          }
          virtual Space* copy(void) { return new PairSpace(*this); }
        };
        bool check_pair(PairSpace& source, const Oracle& oracle, Operation op,
                        const Policy& policy, int a, int b) {
          const Result expected = oracle.evaluate(op, policy, a, b);
          std::unique_ptr<PairSpace> s(static_cast<PairSpace*>(source.clone()));
          rel(*s, s->x[0].encoding(), IRT_EQ, a);
          rel(*s, s->x[1].encoding(), IRT_EQ, b);
          post(*s, op, policy, s->x[0], s->x[1], s->x[2], &s->flags);
          if (s->status() != SS_FAILED && s->x[2].encoding().assigned() &&
              s->flags.assigned() &&
              s->x[2].encoding().val() == expected.code &&
              s->flags.val() == expected.flags && s->x[2].bits().assigned() &&
              s->x[2].bits().val() == static_cast<WordValue>(expected.code))
            return true;
          std::cerr << "tiny arithmetic mismatch format=" << oracle.format
                    << " op=" << static_cast<int>(op) << " a=" << a << " b=" << b
                    << " expected=" << expected.code << ',' << expected.flags
                    << " policy=" << policy.rounding << ','
                    << policy.input_subnormals << ','
                    << policy.output_subnormals << ',' << policy.overflow << ','
                    << policy.zeros << ',' << policy.nan << ','
                    << policy.tininess << ',' << policy.comparison << '\n';
          return false;
        }
        class Exhaustive : public Base {
        public:
          Exhaustive(void) : Base("Word::Float::Arithmetic::Exhaustive",TestTags::sweep()) {}
          virtual bool run(void) {
            // Twelve named single-field configurations; this is not a Cartesian
            // policy check.
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
                       Policy::OperandFirst},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::Preserve, Policy::Format, Policy::Signed,
                       Policy::Canonical},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::Preserve, Policy::Format, Policy::Signed,
                       Policy::SignalFirst, Policy::BeforeRounding},
                Policy{Policy::RNE, Policy::Preserve,
                       Policy::Preserve, Policy::Format, Policy::Signed,
                       Policy::SignalFirst, Policy::AfterPrecision,
                       Policy::Signaling}};
            size_t count = 0;
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const Oracle oracle(format);
              PairSpace source(format);
              if (source.status() == SS_FAILED)
                return false;
              for (Operation op : {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide})
                for (const Policy& policy : policies)
                  for (int a = 0; a < 2 * oracle.sign; ++a)
                    for (int b = 0; b < 2 * oracle.sign; ++b) {
                      if (!check_pair(source, oracle, op, policy, a, b))
                        return false;
                      ++count;
                    }
            }
            return count == 3158016;
          }
        };

        // Frozen before execution: zeros, cancellation, both tie parities,
        // subnormals, normal boundaries, endpoint ties, overflow, NaN orders,
        // and signed divisors.
        const std::array<std::array<int, 2>, 64> fp4_cases = {
            {{0, 0},   {0, 8},   {8, 0},   {8, 8},   {2, 8},  {10, 0}, {2, 0},
             {10, 8},  {1, 1},   {1, 2},   {1, 3},   {1, 4},  {1, 5},  {1, 6},
             {1, 7},   {1, 9},   {2, 1},   {3, 1},   {4, 1},  {5, 1},  {6, 1},
             {7, 1},   {9, 1},   {9, 9},   {7, 2},   {7, 3},  {7, 4},  {7, 7},
             {15, 10}, {15, 11}, {15, 12}, {15, 15}, {2, 10}, {3, 11}, {4, 12},
             {5, 13},  {6, 14},  {7, 15},  {15, 7},  {10, 2}, {3, 2},  {5, 2},
             {7, 2},   {2, 3},   {2, 5},   {2, 7},   {3, 4},  {5, 4},  {9, 2},
             {1, 10},  {9, 10},  {2, 9},   {10, 1},  {10, 9}, {11, 4}, {3, 12},
             {0, 1},   {8, 1},   {0, 9},   {8, 9},   {1, 0},  {1, 8},  {9, 0},
             {9, 8}}};
        const std::array<std::array<int, 2>, 64> fp8_cases = {
            {{0, 0},     {0, 128},   {128, 0},   {128, 128}, {56, 128},
             {184, 0},   {56, 0},    {184, 128}, {1, 1},     {1, 2},
             {1, 7},     {7, 8},     {7, 48},    {8, 48},    {9, 48},
             {15, 48},   {1, 56},    {7, 56},    {8, 56},    {9, 56},
             {7, 57},    {8, 57},    {9, 57},    {15, 57},   {126, 88},
             {126, 89},  {126, 90},  {126, 126}, {254, 216}, {254, 217},
             {254, 218}, {254, 254}, {56, 184},  {57, 185},  {8, 136},
             {1, 129},   {126, 254}, {254, 126}, {184, 56},  {129, 1},
             {56, 40},   {57, 40},   {58, 40},   {59, 40},   {56, 57},
             {57, 56},   {1, 126},   {126, 1},   {127, 255}, {255, 127},
             {255, 56},  {56, 255},  {255, 0},   {0, 255},   {127, 1},
             {1, 127},   {1, 0},     {1, 128},   {129, 0},   {129, 128},
             {7, 1},     {1, 7},     {129, 48},  {1, 176}}};
        class Policies : public Base {
        public:
          Policies(void) : Base("Word::Float::Arithmetic::Policies") {}
          virtual bool run(void) {
            size_t count = 0;
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const Oracle oracle(format);
              PairSpace source(format);
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
                                  static_cast<Policy::Comparison>(comparison)};
                              for (Operation op :
                                   {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide})
                                for (const auto& pair : format == FWF_E2M1
                                                            ? fp4_cases
                                                            : fp8_cases) {
                                  if (!check_pair(source, oracle, op, policy,
                                                  pair[0], pair[1]))
                                    return false;
                                  ++count;
                                }
                            }
            }
            // Pin the contract's noninterchangeable fallback and endpoint
            // witnesses.
            const Oracle fp4(FWF_E2M1), fp8(FWF_E4M3FN);
            const Policy defaults, rtz{Policy::RTZ};
            const Policy saturate{Policy::RNE,
                                  Policy::Preserve, Policy::Preserve,
                                  Policy::SaturateFinite};
            struct Witness {
              const Oracle* oracle;
              Operation op;
              const Policy* policy;
              int a, b, code, flags;
            };
            const Witness witnesses[] = {
                {&fp4, Operation::Divide, &defaults, 0, 0, 7, 1},
                {&fp4, Operation::Divide, &defaults, 2, 8, 15, 2},
                {&fp8, Operation::Divide, &defaults, 0x38, 0x80, 0xff, 2},
                {&fp8, Operation::Divide, &saturate, 0x38, 0x80, 0xfe, 2},
                {&fp4, Operation::Add, &defaults, 7, 2, 7, 20},
                {&fp4, Operation::Add, &rtz, 7, 2, 7, 16},
                {&fp8, Operation::Add, &defaults, 0x7e, 0x58, 0x7e, 16},
                {&fp8, Operation::Add, &defaults, 0x7e, 0x59, 0x7f, 20},
                {&fp8, Operation::Subtract, &defaults, 56, 255, 255, 0}};
            for (const auto& w : witnesses) {
              const Result actual =
                  w.oracle->evaluate(w.op, *w.policy, w.a, w.b);
              if (actual.code != w.code || actual.flags != w.flags)
                return false;
            }
            return count == 393216;
          }
        };

        using Tuple = std::array<int, 4>;
        using Tuples = std::set<Tuple>;
        using Domains = std::array<std::vector<int>, 4>;
        bool contains(const std::vector<int>& values, int value) {
          return std::find(values.begin(), values.end(), value) != values.end();
        }
        std::vector<int> range(int first, int last) {
          std::vector<int> values;
          for (int i = first; i <= last; ++i)
            values.push_back(i);
          return values;
        }
        // alias: repeated inputs, result/input, or a status/encoding identity.
        bool aliases(Tuple tuple, int alias) {
          switch (alias) {
          case 1:
            return tuple[0] == tuple[1];
          case 2:
            return tuple[0] == tuple[2];
          case 3:
            return tuple[1] == tuple[2];
          case 4:
            return tuple[0] == tuple[3];
          case 5:
            return tuple[2] == tuple[3];
          case 6:
            return tuple[0] == tuple[1] && tuple[0] == tuple[2] &&
                   tuple[0] == tuple[3];
          default:
            return true;
          }
        }
        Tuples expected_pairs(const Oracle& oracle, Operation op,
                              const Policy& policy, const Domains& domains,
                              int alias, bool status) {
          Tuples tuples;
          for (int a : domains[0])
            for (int b : domains[1]) {
              const Result r = oracle.evaluate(op, policy, a, b);
              Tuple tuple{{a, b, r.code, r.flags}};
              if (contains(domains[2], r.code) &&
                  (!status || contains(domains[3], r.flags)) &&
                  aliases(tuple, alias)) {
                if (!status)
                  tuple[3] = -1;
                tuples.insert(tuple);
              }
            }
          return tuples;
        }
        std::unique_ptr<PairSpace>
        pair_model(const Oracle& oracle, Operation op, const Policy& policy,
                   const Domains& domains, int alias, bool status, bool word) {
          std::unique_ptr<PairSpace> s(new PairSpace(oracle.format));
          if (alias == 1 || alias == 6)
            s->x[1] = s->x[0];
          if (alias == 2 || alias == 6)
            s->x[2] = s->x[0];
          if (alias == 3)
            s->x[2] = s->x[1];
          if (alias == 4 || alias == 6)
            s->flags = s->x[0].encoding();
          if (alias == 5)
            s->flags = s->x[2].encoding();
          for (int i = 0; i < 3; ++i)
            dom(*s, s->x[i].encoding(),
                IntSet(domains[i].data(), static_cast<int>(domains[i].size())));
          if (status)
            dom(*s, s->flags,
                IntSet(domains[3].data(), static_cast<int>(domains[3].size())));
          post(*s, op, policy, s->x[0], s->x[1], s->x[2],
               status ? &s->flags : nullptr);
          if (word)
            branch(*s,
                   WordVarArgs{s->x[0].bits(), s->x[1].bits(), s->x[2].bits()},
                   WORD_VAL_LSB());
          else
            branch(*s,
                   IntVarArgs{s->x[0].encoding(), s->x[1].encoding(),
                              s->x[2].encoding()},
                   INT_VAR_NONE(), INT_VAL_MIN());
          if (status)
            branch(*s, s->flags, INT_VAL_MIN());
          return s;
        }
        bool root_supports(PairSpace& root, const Tuples& expected, bool status,
                           bool exact) {
          if (root.status() == SS_FAILED)
            return expected.empty();
          if (expected.empty())
            return !exact;
          for (int column = 0; column < (status ? 4 : 3); ++column) {
            IntVar variable =
                column == 3 ? root.flags : root.x[column].encoding();
            std::set<int> supported;
            for (const auto& tuple : expected)
              supported.insert(tuple[column]);
            for (int value : supported)
              if (!variable.in(value))
                return false;
            if (exact && variable.size() != supported.size())
              return false;
          }
          return true;
        }
        Tuples solutions(PairSpace& root, bool status) {
          Tuples tuples;
          DFS<PairSpace> search(&root);
          while (std::unique_ptr<PairSpace> solution{search.next()})
            tuples.insert(Tuple{{solution->x[0].encoding().val(),
                                 solution->x[1].encoding().val(),
                                 solution->x[2].encoding().val(),
                                 status ? solution->flags.val() : -1}});
          return tuples;
        }
        class Symbolic : public Base {
          static bool projections(void) {
            const Policy defaults, upward{Policy::RUP};
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const Oracle oracle(format);
              const std::vector<int> inputs =
                  format == FWF_E2M1
                      ? range(0, 15)
                      : std::vector<int>{0,   1,   7,   8,   48,  56,
                                         57,  88,  126, 127, 128, 129,
                                         136, 184, 185, 254, 255};
              for (Operation op : {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide})
                for (const Policy& policy : {defaults, upward})
                  for (int alias = 0; alias <= 6; ++alias)
                    for (bool word : {false, true}) {
                      Domains domains{{inputs, inputs,
                                       range(0, 2 * oracle.sign - 1),
                                       range(0, 31)}};
                      // Inverse constraints remove results and correlated flags
                      // together.
                      if (alias == 0) {
                        domains[2] =
                            format == FWF_E2M1
                                ? std::vector<int>{0, 1, 2, 7, 8, 15}
                                : std::vector<int>{0,   1,   8,   56,  126, 127,
                                                   128, 129, 184, 254, 255};
                        domains[3] = {0, 1, 2, 16, 24};
                      }
                      const Tuples expected = expected_pairs(
                          oracle, op, policy, domains, alias, true);
                      auto root = pair_model(oracle, op, policy, domains, alias,
                                             true, word);
                      if (!root_supports(*root, expected, true, true) ||
                          solutions(*root, true) != expected)
                        return false;
                    }
              for (Operation op : {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide}) {
                Domains domains{
                    {inputs, inputs, {0, 1, oracle.maximum}, range(0, 31)}};
                const Tuples expected =
                    expected_pairs(oracle, op, defaults, domains, 0, false);
                auto root =
                    pair_model(oracle, op, defaults, domains, 0, false, true);
                if (!root_supports(*root, expected, false, true) ||
                    solutions(*root, false) != expected)
                  return false;
              }
            }
            // Result=6 and flags=0 occur separately for 4+{0,2,3}, but never
            // together.
            const Oracle fp4(FWF_E2M1);
            const Domains correlated{{{6}, {0, 2, 3}, {7}, {0}}};
            auto impossible =
                pair_model(fp4, Operation::Add, defaults, correlated, 0, true, false);
            return expected_pairs(fp4, Operation::Add, defaults, correlated, 0, true)
                       .empty() &&
                   impossible->status() == SS_FAILED;
          }
          static bool clone_and_word(void) {
            const Policy defaults;
            for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
              const Oracle oracle(format);
              for (Operation op : {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide}) {
                Domains domains{{range(0, 2 * oracle.sign - 1),
                                 {format == FWF_E2M1 ? 2 : 56},
                                 range(0, 2 * oracle.sign - 1),
                                 range(0, 31)}};
                auto root =
                    pair_model(oracle, op, defaults, domains, 0, true, false);
                if (root->status() == SS_FAILED)
                  return false;
                std::unique_ptr<PairSpace> clone(
                    static_cast<PairSpace*>(root->clone()));
                if (clone->x[0].encoding().varimp() ==
                    root->x[0].encoding().varimp())
                  return false;
                // Remove interior encodings, then add independent Word bit and
                // bound restrictions.
                rel(*clone, clone->x[0].encoding(), IRT_NQ, 3);
                rel(*clone, clone->x[0].encoding(), IRT_NQ, 3 + oracle.sign);
                dom(*clone, clone->x[0].bits(), WordValue(0),
                    WordValue(oracle.sign - 1));
                rel(*clone, clone->x[0].bits(), WRT_UGQ, WordValue(2));
                domains[0] = range(2, oracle.sign - 1);
                domains[0].erase(
                    std::remove(domains[0].begin(), domains[0].end(), 3),
                    domains[0].end());
                auto fresh =
                    pair_model(oracle, op, defaults, domains, 0, true, false);
                const Tuples expected =
                    expected_pairs(oracle, op, defaults, domains, 0, true);
                if (!root_supports(*clone, expected, true, true) ||
                    !root_supports(*fresh, expected, true, true) ||
                    solutions(*clone, true) != expected ||
                    solutions(*fresh, true) != expected ||
                    !root->x[0].encoding().in(3) ||
                    !root->x[0].encoding().in(3 + oracle.sign))
                  return false;
              }
            }
            // A pure interior-hole event after stabilization must reach
            // adjacent shared domains.
            const Oracle fp4(FWF_E2M1);
            Domains domains{{range(0, 15), {0}, range(0, 15), range(0, 31)}};
            auto root = pair_model(fp4, Operation::Add, defaults, domains, 0, true, false);
            if (root->status() == SS_FAILED)
              return false;
            rel(*root, root->x[0].encoding(), IRT_NQ, 3);
            rel(*root, root->x[0].encoding(), IRT_NQ, 11);
            return root->status() != SS_FAILED &&
                   !root->x[2].encoding().in(3) &&
                   !root->x[2].encoding().in(11);
          }

        public:
          Symbolic(void) : Base("Word::Float::Arithmetic::Symbolic") {}
          virtual bool run(void) { return projections() && clone_and_word(); }
        };

        using GraphTuple = std::array<int, 12>;
        using GraphTuples = std::set<GraphTuple>;
        class GraphSpace : public Space {
        public:
          FloatWordVar x[8];
          IntVarArray flags;
          explicit GraphSpace(bool fp8, bool word) : flags(*this, 4, 0, 31) {
            const FloatWordFormat format = fp8 ? FWF_E4M3FN : FWF_E2M1;
            for (auto& v : x)
              v = FloatWordVar(
                  *this, format,
                  WordVar(*this, float_word_width(format), WDT_UNSIGNED),
                  IntVar(*this, 0, fp8 ? 255 : 15));
            if (!fp8) {
              const int lo[] = {3, 0, 0, 0, 0, 0, 2, 15};
              const int hi[] = {15, 13, 14, 15, 15, 15, 2, 15};
              for (int i = 0; i < 8; ++i)
                dom(*this, x[i].bits(), WordValue(lo[i]), WordValue(hi[i]));
              fp_add(*this, Policy(), x[0], x[1], x[4], flags[0]);
              fp_add(*this, Policy(), x[4], x[2], x[5], flags[1]);
              fp_add(*this, Policy(), x[5], x[3], x[6], flags[2]);
              fp_add(*this, Policy(), x[0], x[3], x[7], flags[3]);
            } else {
              const int values[] = {0, 1, 7, 8, 48, 56, 57, 184};
              for (int i = 0; i < 3; ++i)
                dom(*this, x[i].encoding(), IntSet(values, 8));
              for (int i = 6; i < 8; ++i)
                rel(*this, x[i].encoding(), IRT_EQ, 0);
              rel(*this, flags[3], IRT_EQ, 0);
              fp_multiply(*this, Policy(), x[0], x[1], x[3], flags[0]);
              fp_add(*this, Policy(), x[3], x[2], x[4], flags[1]);
              fp_add(*this, Policy{Policy::RTZ}, x[3], x[2],
                     x[5], flags[2]);
              rel(*this, x[4].encoding(), IRT_NQ, x[5].encoding());
            }
            if (word) {
              WordVarArgs words(8);
              for (int i = 0; i < 8; ++i)
                words[i] = x[i].bits();
              branch(*this, words, WORD_VAL_LSB());
            } else {
              IntVarArgs codes(8);
              for (int i = 0; i < 8; ++i)
                codes[i] = x[i].encoding();
              branch(*this, codes, INT_VAR_NONE(), INT_VAL_MIN());
            }
            branch(*this, flags, INT_VAR_NONE(), INT_VAL_MIN());
          }
          GraphSpace(GraphSpace& s) : Space(s) {
            for (int i = 0; i < 8; ++i)
              x[i].update(*this, s.x[i]);
            flags.update(*this, s.flags);
          }
          virtual Space* copy(void) { return new GraphSpace(*this); }
        };
        GraphTuples graph_expected(bool fp8) {
          const Oracle oracle(fp8 ? FWF_E4M3FN : FWF_E2M1);
          const Policy defaults, rtz{Policy::RTZ};
          GraphTuples expected;
          const std::vector<int> inputs =
              fp8 ? std::vector<int>{0, 1, 7, 8, 48, 56, 57, 184}
                  : range(0, 15);
          for (int a : inputs)
            for (int b : inputs)
              for (int c : inputs) {
                if (fp8) {
                  const Result product =
                      oracle.evaluate(Operation::Multiply, defaults, a, b);
                  const Result nearest =
                      oracle.evaluate(Operation::Add, defaults, product.code, c);
                  const Result truncated =
                      oracle.evaluate(Operation::Add, rtz, product.code, c);
                  if (nearest.code != truncated.code)
                    expected.insert(GraphTuple{
                        {a, b, c, product.code, nearest.code, truncated.code, 0,
                         0, product.flags, nearest.flags, truncated.flags, 0}});
                } else {
                  if ((a & 3) != 3 || (b | 13) != 13 || (c | 14) != 14)
                    continue;
                  for (int d : inputs) {
                    const Result u = oracle.evaluate(Operation::Add, defaults, a, b);
                    const Result v = oracle.evaluate(Operation::Add, defaults, u.code, c);
                    const Result r = oracle.evaluate(Operation::Add, defaults, v.code, d);
                    const Result q = oracle.evaluate(Operation::Add, defaults, a, d);
                    if (r.code == 2 && q.code == 15)
                      expected.insert(
                          GraphTuple{{a, b, c, d, u.code, v.code, 2, 15,
                                      u.flags, v.flags, r.flags, q.flags}});
                  }
                }
              }
          return expected;
        }
        class Connected : public Base {
        public:
          Connected(void) : Base("Word::Float::Arithmetic::Connected") {}
          virtual bool run(void) {
            for (bool fp8 : {false, true}) {
              const GraphTuples expected = graph_expected(fp8);
              if (expected.empty() || (!fp8 && expected.size() != 2))
                return false;
              for (bool word : {false, true}) {
                GraphSpace root(fp8, word);
                if (root.status() == SS_FAILED)
                  return false;
                for (const auto& tuple : expected) {
                  for (int i = 0; i < 8; ++i)
                    if (!root.x[i].encoding().in(tuple[i]))
                      return false;
                  for (int i = 0; i < 4; ++i)
                    if (!root.flags[i].in(tuple[8 + i]))
                      return false;
                }
                if (!fp8 && (root.x[0].encoding().val() != 15 ||
                             root.x[1].encoding().val() != 5 ||
                             root.x[2].encoding().val() != 6 ||
                             root.x[3].encoding().size() != 2 ||
                             !root.x[3].encoding().in(0) ||
                             !root.x[3].encoding().in(8)))
                  return false;
                DFS<GraphSpace> search(&root);
                GraphTuples actual;
                while (std::unique_ptr<GraphSpace> solution{search.next()}) {
                  GraphTuple tuple{};
                  for (int i = 0; i < 8; ++i)
                    tuple[i] = solution->x[i].encoding().val();
                  for (int i = 0; i < 4; ++i)
                    tuple[8 + i] = solution->flags[i].val();
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
          Posting(void) : Base("Word::Float::Arithmetic::Posting") {}
          virtual bool run(void) {
            int caught = 0;
            for (Operation op : {Operation::Add, Operation::Subtract, Operation::Multiply, Operation::Divide}) {
              PairSpace s(FWF_E2M1);
              for (bool status : {false, true}) {
                for (FloatWordFormat format :
                     {FWF_FP32, FWF_BF16, FWF_BINARY16}) {
                  if (format != FWF_BINARY16)
                    continue;
                  FloatWordVar wide(s, format);
                  try {
                    post(s, op, Policy(), wide, wide, wide,
                         status ? &s.flags : nullptr);
                  } catch (const Gecode::Word::OutOfLimits&) {
                    ++caught;
                  }
                }
                FloatWordVar fp8(s, FWF_E4M3FN);
                try {
                  post(s, op, Policy(), s.x[0], fp8, s.x[2],
                       status ? &s.flags : nullptr);
                } catch (const Gecode::Word::FormatMismatch&) {
                  ++caught;
                }
                for (FloatWordVar unbound :
                     {FloatWordVar(s, FWF_E2M1),
                      FloatWordVar::from_bits(s, FWF_E2M1, 0)})
                  for (int column = 0; column < 3; ++column) {
                    try {
                      post(s, op, Policy(), column == 0 ? unbound : s.x[0],
                           column == 1 ? unbound : s.x[1],
                           column == 2 ? unbound : s.x[2],
                           status ? &s.flags : nullptr);
                    } catch (const Gecode::Word::OutOfLimits&) {
                      ++caught;
                    }
                  }
              }
            }
            return caught == 64;
          }
        };
        Exhaustive exhaustive;
        Policies policies;
        Symbolic symbolic;
        Connected connected;
        Posting posting;
      } // namespace Arithmetic
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
