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
            if (format == WDT_E4M3FN && (ac == 127 || bc == 127))
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
                return {format == WDT_E2M1 ? 7 : 127, 1};
              const bool nan =
                  format == WDT_E4M3FN && policy.overflow == Policy::Format;
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
                  format == WDT_E4M3FN && policy.overflow == Policy::Format;
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

        void post(Home home, Operation op, const Policy& policy, WordVar a,
                  WordVar b, WordVar result, IntVar* flags,
                  IntPropLevel ipl = IPL_DEF) {
          switch (op) {
          case Operation::Add:
            if (flags)
              fp_add(home, a, b, result, *flags, policy, ipl);
            else
              fp_add(home, a, b, result, policy, ipl);
            break;
          case Operation::Subtract:
            if (flags)
              fp_subtract(home, a, b, result, *flags, policy, ipl);
            else
              fp_subtract(home, a, b, result, policy, ipl);
            break;
          case Operation::Multiply:
            if (flags)
              fp_multiply(home, a, b, result, *flags, policy, ipl);
            else
              fp_multiply(home, a, b, result, policy, ipl);
            break;
          case Operation::Divide:
            if (flags)
              fp_divide(home, a, b, result, *flags, policy, ipl);
            else
              fp_divide(home, a, b, result, policy, ipl);
            break;
          }
        }
        const Operation operations[] = {Operation::Add, Operation::Subtract,
                                        Operation::Multiply,
                                        Operation::Divide};

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
          return std::find(values.begin(), values.end(), value) != values.end();
        }

        class PairSpace : public Space {
        public:
          WordVar x[3];
          IntVar flags;
          PairSpace(WordDomainType format, WordValueSetOption value_set)
              : flags(*this, -4, 40) {
            for (auto& v : x)
              v = WordVar(*this, format, value_set);
          }
          PairSpace(PairSpace& s) : Space(s) {
            for (int i = 0; i < 3; ++i)
              x[i].update(*this, s.x[i]);
            flags.update(*this, s.flags);
          }
          virtual Space* copy(void) { return new PairSpace(*this); }
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
        /// Check \a a op \a b, with the inputs assigned before posting, or
        /// after a first propagation of the unassigned inputs if \a after.
        bool check_pair(PairSpace& source, const Oracle& oracle, Operation op,
                        const Policy& policy, int a, int b,
                        IntPropLevel ipl = IPL_DEF, bool after = false,
                        Exclude exclude = Exclude::None) {
          const Result expected = oracle.evaluate(op, policy, a, b);
          std::unique_ptr<PairSpace> s(static_cast<PairSpace*>(source.clone()));
          const WordVar x = s->x[0], y = s->x[1],
                        result = s->x[2];
          if (exclude == Exclude::Code)
            rel(*s, result, WRT_NQ, WordValue(expected.code));
          if (exclude == Exclude::Flags)
            rel(*s, s->flags, IRT_NQ, expected.flags);
          if (after) {
            // Four candidates per input keep the first IPL_DOM pass small.
            if (ipl == IPL_DOM) {
              dom(*s, x, WordValue(a & ~3), WordValue(a | 3));
              dom(*s, y, WordValue(b & ~3), WordValue(b | 3));
            }
            post(*s, op, policy, s->x[0], s->x[1], s->x[2], &s->flags, ipl);
            (void)s->status();
          }
          dom(*s, x, WordValue(a));
          dom(*s, y, WordValue(b));
          if (!after)
            post(*s, op, policy, s->x[0], s->x[1], s->x[2], &s->flags, ipl);
          const SpaceStatus status = s->status();
          if (exclude != Exclude::None ? status == SS_FAILED
                                       : status != SS_FAILED &&
                                             result.assigned() &&
                                             s->flags.assigned() &&
                                             code(s->x[2]) == expected.code &&
                                             s->flags.val() == expected.flags)
            return true;
          std::cerr << "tiny arithmetic mismatch format=" << oracle.format
                    << " op=" << static_cast<int>(op) << " a=" << a << " b=" << b
                    << " expected=" << expected.code << ',' << expected.flags
                    << " ipl=" << ipl << " after=" << after
                    << " exclude=" << static_cast<int>(exclude)
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
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN}) {
              const Oracle oracle(format);
              PairSpace source(format, WVS_EXACT);
              if (source.status() == SS_FAILED)
                return false;
              for (Operation op : operations)
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
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN}) {
              const Oracle oracle(format);
              PairSpace exact(format, WVS_EXACT), none(format, WVS_NONE);
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
                                  static_cast<Policy::Comparison>(comparison)};
                              for (Operation op : operations)
                                for (const auto& pair : format == WDT_E2M1
                                                            ? fp4_cases
                                                            : fp8_cases) {
                                  // Alternate the level, the value set and
                                  // when the inputs are assigned; a few
                                  // cases exclude the outcome and must fail.
                                  const IntPropLevel ipl =
                                      count % 2 == 0 ? IPL_DEF : IPL_DOM;
                                  const bool after = assigns_after(count);
                                  const Exclude exclude =
                                      count % 97 == 5    ? Exclude::Code
                                      : count % 97 == 50 ? Exclude::Flags
                                                         : Exclude::None;
                                  if (!check_pair(count % 8 >= 4 ? none : exact,
                                                  oracle, op, policy, pair[0],
                                                  pair[1], ipl, after, exclude))
                                    return false;
                                  ++count;
                                }
                            }
            }
            // Pin the contract's noninterchangeable fallback and endpoint
            // witnesses.
            const Oracle fp4(WDT_E2M1), fp8(WDT_E4M3FN);
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
        // alias: repeated inputs (1), a result that is an input (2, 3), or
        // one word for both inputs and the result (6).
        bool aliases(Tuple tuple, int alias) {
          switch (alias) {
          case 1:
            return tuple[0] == tuple[1];
          case 2:
            return tuple[0] == tuple[2];
          case 3:
            return tuple[1] == tuple[2];
          case 6:
            return tuple[0] == tuple[1] && tuple[0] == tuple[2];
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
        /// A posted model and the values its words had before propagation
        struct PairModel {
          std::unique_ptr<PairSpace> space;
          Domains domains;
        };
        PairModel pair_model(const Oracle& oracle, Operation op,
                             const Policy& policy, const Domains& wanted,
                             int alias, bool status,
                             WordValueSetOption value_set, IntPropLevel ipl) {
          PairModel model{
              std::unique_ptr<PairSpace>(new PairSpace(oracle.format, value_set)),
              wanted};
          PairSpace& s = *model.space;
          if (alias == 1 || alias == 6)
            s.x[1] = s.x[0];
          if (alias == 2 || alias == 6)
            s.x[2] = s.x[0];
          if (alias == 3)
            s.x[2] = s.x[1];
          for (int i = 0; i < 3; ++i)
            restrict(s, s.x[i], wanted[i]);
          for (int i = 0; i < 3; ++i)
            model.domains[i] = values(s.x[i]);
          if (status)
            dom(s, s.flags,
                IntSet(wanted[3].data(), static_cast<int>(wanted[3].size())));
          post(s, op, policy, s.x[0], s.x[1], s.x[2],
               status ? &s.flags : nullptr, ipl);
          branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                 WORD_VAL_LSB());
          if (status)
            branch(s, s.flags, INT_VAL_MIN());
          return model;
        }
        /// Test that \a root keeps every supported value, and if \a exact
        /// only those (as hulls without a value set).
        bool root_supports(PairSpace& root, const Tuples& expected, bool status,
                           bool exact) {
          if (root.status() == SS_FAILED)
            return expected.empty();
          if (expected.empty())
            return !exact;
          for (int column = 0; column < (status ? 4 : 3); ++column) {
            std::set<int> supported;
            for (const auto& tuple : expected)
              supported.insert(tuple[column]);
            for (int value : supported)
              if (column == 3 ? !root.flags.in(value)
                              : !root.x[column].in(WordValue(value)))
                return false;
            if (exact && (column == 3
                              ? root.flags.size() != supported.size()
                              : !projected(root.x[column], supported)))
              return false;
          }
          return true;
        }
        Tuples solutions(PairSpace& root, bool status) {
          Tuples tuples;
          DFS<PairSpace> search(&root);
          while (std::unique_ptr<PairSpace> solution{search.next()})
            tuples.insert(Tuple{{code(solution->x[0]), code(solution->x[1]),
                                 code(solution->x[2]),
                                 status ? solution->flags.val() : -1}});
          return tuples;
        }
        class Symbolic : public Base {
          static bool projections(void) {
            const Policy defaults, upward{Policy::RUP};
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE}) {
                const Oracle oracle(format);
                const bool fp4 = format == WDT_E2M1;
                const bool set = value_set == WVS_EXACT;
                // Without a value set, the E4M3FN domains have small hulls:
                // the inputs cover signs, overflow and the NaN.
                const std::vector<int> inputs =
                    fp4   ? range(0, 15)
                    : set ? std::vector<int>{0,   1,   7,   8,   48,  56,
                                             57,  88,  126, 127, 128, 129,
                                             136, 184, 185, 254, 255}
                          : cube(0x36, 0xc9);
                const std::vector<int> results =
                    fp4   ? std::vector<int>{0, 1, 2, 7, 8, 15}
                    : set ? std::vector<int>{0,   1,   8,   56,  126, 127,
                                             128, 129, 184, 254, 255}
                          : std::vector<int>{0x38, 0x3c, 0x3e, 0x7e, 0x7f,
                                             0xb8, 0xfe, 0xff};
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                  for (Operation op : operations)
                    for (const Policy& policy : {defaults, upward})
                      for (int alias : {0, 1, 2, 3, 6}) {
                        Domains wanted{{inputs, inputs,
                                        range(0, 2 * oracle.sign - 1),
                                        range(0, 31)}};
                        // Inverse constraints remove results and correlated
                        // flags together.
                        if (alias == 0) {
                          wanted[2] = results;
                          wanted[3] = {0, 1, 2, 16, 24};
                        }
                        PairModel model = pair_model(oracle, op, policy, wanted,
                                                     alias, true, value_set, ipl);
                        const Tuples expected = expected_pairs(
                            oracle, op, policy, model.domains, alias, true);
                        if (!root_supports(*model.space, expected, true,
                                           ipl == IPL_DOM) ||
                            solutions(*model.space, true) != expected)
                          return false;
                      }
                  for (Operation op : operations) {
                    const Domains wanted{{inputs,
                                          inputs,
                                          {0, 1, oracle.maximum},
                                          range(0, 31)}};
                    PairModel model = pair_model(oracle, op, defaults, wanted,
                                                 0, false, value_set, ipl);
                    const Tuples expected = expected_pairs(
                        oracle, op, defaults, model.domains, 0, false);
                    if (!root_supports(*model.space, expected, false,
                                       ipl == IPL_DOM) ||
                        solutions(*model.space, false) != expected)
                      return false;
                  }
                }
              }
            // Result=6 and flags=0 occur separately for 4+{0,2,3}, but never
            // together.
            const Oracle fp4(WDT_E2M1);
            const Domains correlated{{{6}, {0, 2, 3}, {7}, {0}}};
            if (!expected_pairs(fp4, Operation::Add, defaults, correlated, 0,
                                true)
                     .empty())
              return false;
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                PairModel impossible =
                    pair_model(fp4, Operation::Add, defaults, correlated, 0,
                               true, value_set, ipl);
                if (impossible.space->status() != SS_FAILED)
                  return false;
              }
            return true;
          }
          static bool clone_and_word(void) {
            const Policy defaults;
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                  const Oracle oracle(format);
                  const bool set = value_set == WVS_EXACT;
                  for (Operation op : operations) {
                    Domains wanted{{range(0, 2 * oracle.sign - 1),
                                    {format == WDT_E2M1 ? 2 : 56},
                                    range(0, 2 * oracle.sign - 1),
                                    range(0, 31)}};
                    PairModel model = pair_model(oracle, op, defaults, wanted,
                                                 0, true, value_set, ipl);
                    PairSpace& root = *model.space;
                    if (root.status() == SS_FAILED)
                      return false;
                    std::unique_ptr<PairSpace> clone(
                        static_cast<PairSpace*>(root.clone()));
                    if (clone->x[0].varimp() == root.x[0].varimp())
                      return false;
                    // Remove interior codes where a value set holds them,
                    // then add independent cube and bound restrictions.
                    std::vector<int> kept = range(2, oracle.sign - 1);
                    if (set) {
                      rel(*clone, clone->x[0], WRT_NQ, WordValue(3));
                      rel(*clone, clone->x[0], WRT_NQ,
                          WordValue(3 + oracle.sign));
                      kept.erase(std::find(kept.begin(), kept.end(), 3));
                    }
                    dom(*clone, clone->x[0], WordValue(0),
                        WordValue(oracle.sign - 1));
                    float_bounds(*clone, clone->x[0], WordValue(2),
                                 WordValue(oracle.sign - 1));
                    wanted[0] = kept;
                    PairModel fresh = pair_model(oracle, op, defaults, wanted,
                                                 0, true, value_set, ipl);
                    const Tuples expected = expected_pairs(
                        oracle, op, defaults, fresh.domains, 0, true);
                    // The assigned second operand makes both levels exact.
                    if (!root_supports(*clone, expected, true, true) ||
                        !root_supports(*fresh.space, expected, true, true) ||
                        solutions(*clone, true) != expected ||
                        solutions(*fresh.space, true) != expected)
                      return false;
                    if (set && (!root.x[0].in(WordValue(3)) ||
                                !root.x[0].in(WordValue(3 + oracle.sign))))
                      return false;
                  }
                }
            // A pure interior-hole event after stabilization must reach
            // adjacent shared domains.
            const Oracle fp4(WDT_E2M1);
            const Domains domains{{range(0, 15), {0}, range(0, 15), range(0, 31)}};
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
              PairModel model = pair_model(fp4, Operation::Add, defaults,
                                           domains, 0, true, WVS_EXACT, ipl);
              PairSpace& root = *model.space;
              if (root.status() == SS_FAILED)
                return false;
              rel(root, root.x[0], WRT_NQ, WordValue(3));
              rel(root, root.x[0], WRT_NQ, WordValue(11));
              if (root.status() == SS_FAILED ||
                  root.x[2].in(WordValue(3)) ||
                  root.x[2].in(WordValue(11)))
                return false;
            }
            return true;
          }

        public:
          Symbolic(void) : Base("Word::Float::Arithmetic::Symbolic") {}
          virtual bool run(void) { return projections() && clone_and_word(); }
        };

        using GraphTuple = std::array<int, 12>;
        using GraphTuples = std::set<GraphTuple>;
        class GraphSpace : public Space {
        public:
          WordVar x[8];
          IntVarArray flags;
          GraphSpace(bool fp8, IntPropLevel ipl) : flags(*this, 4, 0, 31) {
            const WordDomainType format = fp8 ? WDT_E4M3FN : WDT_E2M1;
            for (auto& v : x)
              v = WordVar(*this, format, WVS_EXACT);
            if (!fp8) {
              const int lo[] = {3, 0, 0, 0, 0, 0, 2, 15};
              const int hi[] = {15, 13, 14, 15, 15, 15, 2, 15};
              for (int i = 0; i < 8; ++i)
                dom(*this, x[i], WordValue(lo[i]), WordValue(hi[i]));
              fp_add(*this, x[0], x[1], x[4], flags[0], ipl);
              fp_add(*this, x[4], x[2], x[5], flags[1], ipl);
              fp_add(*this, x[5], x[3], x[6], flags[2], ipl);
              fp_add(*this, x[0], x[3], x[7], flags[3], ipl);
            } else {
              for (int i = 0; i < 3; ++i)
                restrict(*this, x[i], {0, 1, 7, 8, 48, 56, 57, 184});
              for (int i = 6; i < 8; ++i)
                rel(*this, x[i], WRT_EQ, WordValue(0));
              rel(*this, flags[3], IRT_EQ, 0);
              fp_multiply(*this, x[0], x[1], x[3], flags[0], ipl);
              fp_add(*this, x[3], x[2], x[4], flags[1], ipl);
              fp_add(*this, x[3], x[2], x[5], flags[2], Policy{Policy::RTZ},
                     ipl);
              rel(*this, x[4], WRT_NQ, x[5]);
            }
            WordVarArgs words(8);
            for (int i = 0; i < 8; ++i)
              words[i] = x[i];
            branch(*this, words, WORD_VAL_LSB());
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
          const Oracle oracle(fp8 ? WDT_E4M3FN : WDT_E2M1);
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
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                GraphSpace root(fp8, ipl);
                if (root.status() == SS_FAILED)
                  return false;
                for (const auto& tuple : expected) {
                  for (int i = 0; i < 8; ++i)
                    if (!root.x[i].in(WordValue(tuple[i])))
                      return false;
                  for (int i = 0; i < 4; ++i)
                    if (!root.flags[i].in(tuple[8 + i]))
                      return false;
                }
                const WordVar x3 = root.x[3];
                if (ipl == IPL_DOM && !fp8 &&
                    (!root.x[0].assigned() || code(root.x[0]) != 15 ||
                     !root.x[1].assigned() || code(root.x[1]) != 5 ||
                     !root.x[2].assigned() || code(root.x[2]) != 6 ||
                     values(x3) != std::vector<int>{0, 8}))
                  return false;
                DFS<GraphSpace> search(&root);
                GraphTuples actual;
                while (std::unique_ptr<GraphSpace> solution{search.next()}) {
                  GraphTuple tuple{};
                  for (int i = 0; i < 8; ++i)
                    tuple[i] = code(solution->x[i]);
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
          /// Plain float-typed words and constants propagate exactly once
          /// their inputs are assigned.
          static bool plain_words(void) {
            const Oracle oracle(WDT_E2M1);
            for (Operation op : operations)
              for (bool status : {false, true})
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                  PairSpace s(WDT_E2M1, WVS_EXACT);
                  s.x[0] = float_code(s, WDT_E2M1, 3);
                  s.x[1] = WordVar(s, WDT_E2M1);
                  s.x[2] = WordVar(s, WDT_E2M1);
                  post(s, op, Policy(), s.x[0], s.x[1], s.x[2],
                       status ? &s.flags : nullptr, ipl);
                  dom(s, s.x[1], WordValue(5));
                  const Result expected = oracle.evaluate(op, Policy(), 3, 5);
                  if (s.status() == SS_FAILED || !s.x[2].assigned() ||
                      code(s.x[2]) != expected.code ||
                      (status && (!s.flags.assigned() ||
                                  s.flags.val() != expected.flags)))
                    return false;
                }
            return true;
          }
          /// FP32 propagation does not depend on the level.
          static bool wide_levels(void) {
            for (Operation op : operations) {
              std::unique_ptr<PairSpace> spaces[2];
              const IntPropLevel levels[] = {IPL_DEF, IPL_DOM};
              for (int i = 0; i < 2; ++i) {
                spaces[i].reset(new PairSpace(WDT_FP32, WVS_NONE));
                PairSpace& s = *spaces[i];
                float_bounds(s, s.x[0], 0x3f800000, 0x40400000);
                float_bounds(s, s.x[1], 0x40000000, 0x40800000);
                post(s, op, Policy(), s.x[0], s.x[1], s.x[2], &s.flags,
                     levels[i]);
                if (s.status() == SS_FAILED)
                  return false;
              }
              for (int i = 0; i < 3; ++i) {
                const WordVar a = spaces[0]->x[i],
                              b = spaces[1]->x[i];
                if (a.lo() != b.lo() || a.hi() != b.hi() ||
                    a.minimum() != b.minimum() || a.maximum() != b.maximum())
                  return false;
              }
              if (values(spaces[0]->flags) != values(spaces[1]->flags))
                return false;
            }
            return true;
          }

        public:
          Posting(void) : Base("Word::Float::Arithmetic::Posting") {}
          virtual bool run(void) {
            int caught = 0;
            for (Operation op : operations) {
              PairSpace s(WDT_E2M1, WVS_EXACT);
              for (bool status : {false, true}) {
                WordVar wide(s, WDT_BINARY16);
                try {
                  post(s, op, Policy(), wide, wide, wide,
                       status ? &s.flags : nullptr);
                } catch (const Gecode::Word::OutOfLimits&) {
                  ++caught;
                }
                WordVar fp8(s, WDT_E4M3FN);
                try {
                  post(s, op, Policy(), s.x[0], fp8, s.x[2],
                       status ? &s.flags : nullptr);
                } catch (const Gecode::Word::FormatMismatch&) {
                  ++caught;
                }
              }
            }
            return caught == 16 && plain_words() && wide_levels();
          }
        };

        /// Focused default-level deductions on E4M3FN words
        class TinyBounds : public Base {
          static constexpr int zero = 0x00, half = 0x30, one = 0x38,
                               one_half = 0x3c, two = 0x40, three = 0x44,
                               three_half = 0x46, four = 0x48, five = 0x4a,
                               six = 0x4c, minus_two = 0xc0,
                               minus_three = 0xc4, nan = 0x7f;
          static PairModel model(Operation op, const std::vector<int>& x,
                                 const std::vector<int>& y,
                                 const std::vector<int>& result,
                                 WordValueSetOption value_set,
                                 IntPropLevel ipl) {
            const Oracle oracle(WDT_E4M3FN);
            return pair_model(oracle, op, Policy(),
                              Domains{{x, y, result, range(0, 31)}}, 0, true,
                              value_set, ipl);
          }
          static bool keeps(const WordVar& x, const std::vector<int>& p) {
            return projected(x, std::set<int>(p.begin(), p.end()));
          }
          /// The result keeps the rounded hull of the exact sums 3 to 6.
          static bool results(WordValueSetOption value_set) {
            PairModel m = model(Operation::Add, {one, one_half, two},
                                {two, three, four}, range(0, 255), value_set,
                                IPL_DEF);
            PairSpace& s = *m.space;
            return s.status() != SS_FAILED && keeps(s.x[2], range(three, six)) &&
                   keeps(s.x[0], {one, one_half, two}) &&
                   keeps(s.x[1], {two, three, four});
          }
          /// Results 3 to 3.5 leave the operands whose bounds can reach them,
          /// and the then assigned operand makes the result exact.
          static bool operands(WordValueSetOption value_set,
                               IntPropLevel ipl) {
            PairModel m = model(Operation::Add, {one, one_half, two},
                                {two, three, four}, {three, 0x45, three_half},
                                value_set, ipl);
            PairSpace& s = *m.space;
            return s.status() != SS_FAILED && keeps(s.x[0], {one, one_half}) &&
                   keeps(s.x[1], {two}) && keeps(s.x[2], {three, three_half});
          }
          /// Only 1 + 4 and 4 + 1 give 5: IPL_DEF keeps 2 for the first
          /// operand, which lies within the bounds, and IPL_DOM removes it.
          static bool levels(WordValueSetOption value_set) {
            PairModel bounds = model(Operation::Add, {one, two, four},
                                     {one, four}, {five}, value_set, IPL_DEF);
            PairModel domain = model(Operation::Add, {one, two, four},
                                     {one, four}, {five}, value_set, IPL_DOM);
            return bounds.space->status() != SS_FAILED &&
                   domain.space->status() != SS_FAILED &&
                   keeps(bounds.space->x[0], {one, two, four}) &&
                   keeps(domain.space->x[0], {one, four});
          }
          /// Exact differences 0.5 to 3.5.
          static bool subtract(WordValueSetOption value_set) {
            PairModel m = model(Operation::Subtract, {two, three, four},
                                {half, one, one_half}, range(0, 255), value_set,
                                IPL_DEF);
            return m.space->status() != SS_FAILED &&
                   keeps(m.space->x[2], range(half, three_half));
          }
          /// Exact products -2 to -6.
          static bool multiply(WordValueSetOption value_set) {
            PairModel m = model(Operation::Multiply, {one, one_half, two},
                                {minus_two, minus_three}, range(0, 255),
                                value_set, IPL_DEF);
            return m.space->status() != SS_FAILED &&
                   keeps(m.space->x[2], range(minus_two, six | 0x80));
          }
          /// Quotients 0.5 to 1 and the NaN of division by zero; IPL_DEF
          /// keeps the division-by-zero results of both signs.
          static bool divide(WordValueSetOption value_set) {
            PairModel bounds = model(Operation::Divide, {one, two},
                                     {zero, two}, range(0, 255), value_set,
                                     IPL_DEF);
            PairModel domain = model(Operation::Divide, {one, two},
                                     {zero, two}, range(0, 255), value_set,
                                     IPL_DOM);
            PairSpace& s = *bounds.space;
            if (s.status() == SS_FAILED ||
                domain.space->status() == SS_FAILED ||
                !keeps(domain.space->x[2], {half, one, nan}) ||
                !s.flags.in(2))
              return false;
            for (int v : {half, one, nan})
              if (!s.x[2].in(WordValue(v)))
                return false;
            if (value_set == WVS_EXACT)
              for (int v : values(s.x[2]))
                if ((v < half || v > one) && v != nan && v != (nan | 0x80))
                  return false;
            return true;
          }

        public:
          TinyBounds(void) : Base("Word::Float::Arithmetic::TinyBounds") {}
          virtual bool run(void) {
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              if (!results(value_set) || !operands(value_set, IPL_DEF) ||
                  !operands(value_set, IPL_DOM) || !levels(value_set) ||
                  !subtract(value_set) || !multiply(value_set) ||
                  !divide(value_set))
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
                               const Policy& policy, Operation op) {
            const Oracle oracle(format);
            PairSpace root(format, value_set);
            const int choices[] = {0, 1, 2, 3, 6};
            const int alias = choices[_rand(5U)];
            if (alias == 1 || alias == 6)
              root.x[1] = root.x[0];
            if (alias == 2 || alias == 6)
              root.x[2] = root.x[0];
            if (alias == 3)
              root.x[2] = root.x[1];
            restrict(root, root.x[0], random_codes(format));
            if (alias != 1 && alias != 6)
              restrict(root, root.x[1], random_codes(format));
            // Results and flags are restricted around reachable outcomes.
            std::vector<Result> outcomes;
            for (int a : values(root.x[0]))
              for (int b : values(root.x[1]))
                if ((alias != 1 && alias != 6) || a == b)
                  outcomes.push_back(oracle.evaluate(op, policy, a, b));
            if (alias == 0 && _rand(2U) == 0) {
              std::vector<int> results;
              for (const Result& r : outcomes)
                if (_rand(2U) == 0)
                  results.push_back(r.code);
              results.push_back(static_cast<int>(_rand(2U * oracle.sign)));
              restrict(root, root.x[2], results);
            }
            const int flags =
                outcomes[_rand(static_cast<unsigned int>(outcomes.size()))].flags;
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
                                   values(root.flags)}};
            const Tuples expected =
                expected_pairs(oracle, op, policy, domains, alias, true);
            post(root, op, policy, root.x[0], root.x[1], root.x[2],
                 &root.flags, ipl);
            if (!root_supports(root, expected, true, ipl == IPL_DOM))
              return false;
            if (root.status() == SS_FAILED)
              return true;
            branch(root,
                   WordVarArgs{root.x[0], root.x[1],
                               root.x[2]},
                   WORD_VAL_LSB());
            branch(root, root.flags, INT_VAL_MIN());
            return solutions(root, true) == expected;
          }

        public:
          TinyRoot(void) : Base("Word::Float::Arithmetic::TinyRoot") {}
          virtual bool run(void) {
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                  for (const Policy& policy : policies())
                    for (Operation op : operations)
                      for (int trial = 0; trial < 3; ++trial)
                        if (!keeps_supported(format, value_set, ipl, policy,
                                             op))
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
          // Operand windows and result window; a cube case gives the first
          // operand the known ones and possible ones a.minimum and a.maximum
          // on its full domain instead.
          struct Case {
            Range a, b, result;
            bool cube;
          };
          static WordVar window(Space& home, WordDomainType format,
                                Range r, WordValueSetOption value_set) {
            return float_window(home, format, WordValue(r.minimum),
                                WordValue(r.maximum), value_set);
          }
          static bool check(const Oracle& oracle, Operation op,
                            const Policy& policy, const Case& c,
                            WordValueSetOption value_set, IntPropLevel ipl) {
            const WordDomainType format = oracle.format;
            const Range all{oracle.sign | (oracle.sign - 1), oracle.sign - 1};
            PairSpace s(format, value_set);
            s.x[0] = window(s, format, c.cube ? all : c.a, value_set);
            if (c.cube)
              dom(s, s.x[0], WordValue(c.a.minimum),
                  WordValue(c.a.maximum));
            s.x[1] = window(s, format, c.b, value_set);
            s.x[2] = window(s, format, c.result, value_set);
            Tuples expected;
            for (int a : values(s.x[0]))
              for (int b : values(s.x[1])) {
                const Result r = oracle.evaluate(op, policy, a, b);
                if (s.x[2].in(WordValue(r.code)))
                  expected.insert({a, b, r.code, r.flags});
              }
            post(s, op, policy, s.x[0], s.x[1], s.x[2], &s.flags, ipl);
            branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                   WORD_VAL_LSB());
            branch(s, s.flags, INT_VAL_MIN());
            if (solutions(s, true) == expected)
              return true;
            if (opt.log)
              olog << "float-typed tiny arithmetic " << format << " op "
                   << static_cast<int>(op) << " rounding " << policy.rounding
                   << " value set " << static_cast<int>(value_set) << " ipl " << ipl
                   << std::hex << " a " << c.a.minimum << ".." << c.a.maximum
                   << " b " << c.b.minimum << ".." << c.b.maximum
                   << std::dec << " cube " << c.cube << '\n';
            return false;
          }
          static bool format_cases(WordDomainType format) {
            const Oracle oracle(format);
            const int s = oracle.sign;
            const Range all{s | (s - 1), s - 1};
            const std::vector<Case> cases =
                format == WDT_E2M1
                    ? std::vector<Case>{{{s | 3, 3}, {s | 5, s | 1}, all, false},
                                        {{s | 7, s | 2},
                                         {s | 2, 2},
                                         {s | 7, s | 1},
                                         false},
                                        {{s | 1, 6}, {4, 6}, {s | 3, 3}, false},
                                        {{0, 0xb}, {s | 4, 4}, all, true}}
                    : std::vector<Case>{
                          {{s | 6, 6}, {s | 0x3c, s | 0x38}, all, false},
                          {{s | 0x40, s | 0x38},
                           {s | 2, 2},
                           {s | 0x50, s | 0x30},
                           false},
                          {{s | 0x3a, 0x3a}, {0x38, 0x3c}, {s | 0x3c, 0x3c},
                           false},
                          {{0x30, 0xb3}, {0x38, 0x3a}, all, true}};
            const Policy policies[] = {
                Policy(), Policy{Policy::RUP},
                Policy{Policy::RDN, Policy::SignedZero, Policy::SignedZero}};
            for (const Case& c : cases)
              for (const Policy& policy : policies)
                for (Operation op : operations)
                  for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
                    for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                      if (!check(oracle, op, policy, c, value_set, ipl))
                        return false;
            return true;
          }

        public:
          FloatTyped(void) : Base("Word::Float::Arithmetic::FloatTyped") {}
          virtual bool run(void) {
            return format_cases(WDT_E2M1) && format_cases(WDT_E4M3FN);
          }
        };
        Exhaustive exhaustive;
        Policies policies;
        Symbolic symbolic;
        Connected connected;
        Posting posting;
        TinyBounds tiny_bounds;
        TinyRoot tiny_root;
        FloatTyped float_typed;
      } // namespace Arithmetic
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
