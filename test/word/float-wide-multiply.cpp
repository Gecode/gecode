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

#include "test/word/float-oracle.hh"
#include <algorithm>
#include <array>
#include <gecode/float-word.hh>
#include <gecode/word/arithmetic/global-integer.hpp>
#include <gecode/word/float-wide.hpp>
#include <memory>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace Test {
  namespace Word {
    namespace Float {
      namespace {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        // The expected product from the shared exact oracle: the special
        // cases of fp_multiply(), then one rounding of the exact product.
        struct Reference : FloatFormat {
          explicit Reference(WordDomainType f) : FloatFormat(float_format(f)) {}
          FloatResult evaluate(const Policy& p, WordValue a, WordValue b) const {
            const bool negative = (a >= sign) != (b >= sign);
            if (is_nan(a) || is_nan(b))
              return nan_result(*this, p, {a, b});
            WordValue am = magnitude(a), bm = magnitude(b);
            if (p.input_subnormals == Policy::SignedZero) {
              if (am < normal)
                am = 0;
              if (bm < normal)
                bm = 0;
            }
            if (am == infinity || bm == infinity) {
              if (am == 0 || bm == 0)
                return {nan, 1};
              return {infinity + (negative ? sign : 0), 0};
            }
            if (am == 0 || bm == 0)
              return {negative && p.zeros == Policy::Signed ? sign : 0, 0};
            const ExactRatio av = value(am), bv = value(bm);
            return round_exact(*this,
                               {av.numerator * bv.numerator,
                                av.denominator * bv.denominator},
                               negative, p);
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
              fp_multiply(*this, x[0], x[1], x[2], flags, p);
            else
              fp_multiply(*this, x[0], x[1], x[2], p);
          }
        };

        bool check_exact(const Reference& ref, const Policy& p, WordValue a,
                         WordValue b, bool with_status = true) {
          const FloatResult expected = ref.evaluate(p, a, b);
          Pair s(ref.type);
          dom(s, s.x[0], a);
          dom(s, s.x[1], b);
          s.post(p, with_status);
          if (s.status() == SS_FAILED)
            return false;
          if (!s.x[2].assigned() ||
              s.x[2].val() != expected.bits ||
              (with_status &&
               (!s.flags.assigned() || s.flags.val() != expected.flags))) {
            olog << ind(2) << "multiply exact mismatch " << ref.fraction << ' '
                      << p.rounding << ' ' << p.input_subnormals << ' '
                      << p.output_subnormals << ' ' << p.tininess << ' '
                      << std::hex << a << ' ' << b << " expected "
                      << expected.bits << std::dec << '/' << expected.flags
                      << '\n';
            return false;
          }
          return (expected.flags & 2) == 0;
        }

        class MultiplyExact : public Base {
        public:
          MultiplyExact()
              : Base("Word::Float::Arithmetic::Wide::Multiply::Exact") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              const std::vector<WordValue> values{0,
                                                  ref.sign,
                                                  1,
                                                  ref.sign + 1,
                                                  ref.normal - 1,
                                                  ref.normal,
                                                  ref.normal + 1,
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
                                                  ref.sign + ref.infinity +
                                                      ref.quiet + 7};
              for (int r = 0; r < 4; ++r)
                for (WordValue a : values)
                  for (WordValue b : values)
                    if (!check_exact(ref,
                                     Policy{static_cast<Policy::Rounding>(r)},
                                     a, b))
                      return false;
              const std::pair<WordValue, WordValue> catalogue[] = {
                  {1, 1},
                  {ref.normal, ref.one - ref.normal},
                  {ref.normal, ref.one - 1},
                  {ref.normal + 1, ref.one - 2},
                  {ref.maximum - 1, ref.one + 1},
                  {ref.sign, ref.one},
                  {ref.infinity, 1},
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
                                     sign + small, ref.one - ref.normal))
                      return false;
              // Literal witnesses anchor the independent oracle's range rules.
              const struct {
                WordValue a, b, result;
                int flags;
              } literals[] = {
                  {1, 1, 0, 24},
                  {ref.normal, ref.one - ref.normal, ref.normal / 2, 0},
                  {ref.normal, ref.one - 1, ref.normal, 24},
                  {ref.normal + 1, ref.one - 2, ref.normal, 16},
                  {ref.maximum, ref.maximum, ref.infinity, 20},
                  {ref.maximum - 1, ref.one + 1, ref.infinity, 20},
                  {0, ref.infinity, ref.infinity + ref.quiet, 1},
                  {ref.sign, ref.one, ref.sign, 0}};
              for (const auto& e : literals) {
                const FloatResult expected = ref.evaluate(Policy(), e.a, e.b);
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

        std::set<Row> expected_pairs(const Reference& ref, const Policy& p,
                                     Window a, Window b, Window z, int status,
                                     int alias) {
          const unsigned int width = float_word_width(ref.type);
          std::set<Row> expected;
          for (WordValue av : float_order_range(width, a.minimum, a.maximum))
            for (WordValue bv :
                 float_order_range(width, b.minimum, b.maximum)) {
              if ((alias == 1 || alias == 3) && av != bv)
                continue;
              const FloatResult value = ref.evaluate(p, av, bv);
              if (status >= 0 && status != value.flags)
                continue;
              if (float_order_contains(width, z.minimum, z.maximum,
                                       value.bits) &&
                  (alias < 2 || value.bits == av))
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
          Pair s(ref.type);
          if (alias == 1 || alias == 3)
            s.x[1] = s.x[0];
          if (alias >= 2)
            s.x[2] = s.x[0];
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
              olog << ind(2) << "multiply did not prune " << ref.type << ' '
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
            olog << ind(2) << "multiply relation mismatch " << ref.type << ' '
                      << p.rounding << ' ' << p.input_subnormals << ' '
                      << p.output_subnormals << ' ' << alias << ' ' << std::hex
                      << a.minimum << ' ' << a.maximum << ' ' << b.minimum
                      << ' ' << b.maximum << std::dec << " tuples "
                      << found.size() << '/' << expected.size() << '\n';
            return false;
          }
          return true;
        }

        class MultiplySymbolic : public Base {
        public:
          MultiplySymbolic()
              : Base("Word::Float::Arithmetic::Wide::Multiply::Symbolic") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              const Window all{ref.sign | (ref.sign - 1), ref.sign - 1};
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
                const Policy positive{static_cast<Policy::Rounding>(r),
                                      Policy::Preserve, Policy::Preserve,
                                      Policy::FormatDefault, Policy::Positive};
                if (!check_window(
                        ref, positive, {ref.sign + 3, ref.sign},
                        {ref.one - ref.normal, ref.one - ref.normal + 1},
                        all))
                  return false;
                const Policy positive_flush{
                    static_cast<Policy::Rounding>(r),
                    Policy::Preserve, Policy::SignedZero, Policy::FormatDefault,
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
                      ref, Policy(), {0, 3}, {ref.infinity, ref.infinity},
                      {ref.infinity + ref.quiet, ref.infinity + ref.quiet},
                      0))
                return false;
              for (int rounding = 0; rounding < 4; ++rounding)
                for (int alias = 1; alias <= 3; ++alias)
                  for (const Window& a :
                       {Window{ref.one - 2, ref.one + 2}, Window{0, 3},
                        Window{ref.sign + 3, ref.sign},
                        Window{ref.infinity - 1, ref.infinity + 2},
                        Window{ref.infinity + ref.quiet,
                               ref.infinity + ref.quiet + 2}}) {
                    const Window b =
                        alias == 2 ? Window{ref.one, ref.one + 1} : a;
                    if (!check_window(
                            ref,
                            Policy{static_cast<Policy::Rounding>(rounding)},
                            a, b, alias >= 2 ? a : all, -1, -1, alias))
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
              v = WordVar(*this, ref.type);
            for (auto& v : flags)
              v = IntVar(*this, 0, 31);
            bounds(*this, x[0], {ref.one - 4, ref.one + 4});
            bounds(*this, x[1], {ref.one - 1, ref.one + 1});
            bounds(*this, x[3], {ref.one - 1, ref.one + 1});
            dom(*this, x[4], ref.one);
            fp_multiply(*this, x[0], x[1], x[2], flags[0], p);
            fp_multiply(*this, x[2], x[3], x[4], flags[1], p);
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
        class MultiplyLifecycle : public Base {
        public:
          MultiplyLifecycle()
              : Base("Word::Float::Arithmetic::Wide::Multiply::Lifecycle") {}
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
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              for (int r = 0; r < 4; ++r) {
                const Policy p{static_cast<Policy::Rounding>(r)};
                Network original(ref, p);
                if (original.status() == SS_FAILED) {
                  olog << ind(2) << "multiply network failed " << f << ' ' << r
                            << '\n';
                  return false;
                }
                if (Gecode::Word::WordView(original.x[0])
                        .in(ref.one - 4) &&
                    Gecode::Word::WordView(original.x[0])
                        .in(ref.one + 4)) {
                  olog << ind(2) << "multiply network did not prune " << f << ' '
                            << r << '\n';
                  return false;
                }
                std::unique_ptr<Network> clone(
                    static_cast<Network*>(original.clone()));
                fp_multiply(*clone, clone->x[0], clone->x[1],
                            clone->x[2], clone->flags[0], p);
                fp_multiply(*clone, clone->x[2], clone->x[3],
                            clone->x[4], clone->flags[1], p);
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

        class MultiplyPosting : public Base {
        public:
          MultiplyPosting()
              : Base("Word::Float::Arithmetic::Wide::Multiply::Posting") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32, WDT_BINARY16})
              for (bool status : {false, true}) {
                Pair s(f);
                bool caught = false;
                try {
                  if (status)
                    fp_multiply(s, s.x[0], s.x[1], s.x[2], s.flags);
                  else
                    fp_multiply(s, s.x[0], s.x[1], s.x[2]);
                } catch (const Gecode::Word::OutOfLimits&) {
                  caught = true;
                }
                if (caught != (f == WDT_BINARY16))
                  return false;
              }
            Pair mismatch(WDT_BF16);
            WordVar binary16 = WordVar(mismatch, WDT_BINARY16);
            try {
              fp_multiply(mismatch, mismatch.x[0], binary16,
                          mismatch.x[2]);
            } catch (const Gecode::Word::FormatMismatch&) {
              return true;
            }
            return false;
          }
        };
        // Products on float-typed words, whose bounds are in IEEE total order,
        // over negative-only and cross-zero windows.
        class MultiplyFloatTyped : public Base {
          // Total-order window
          struct Range {
            WordValue minimum, maximum;
          };
          static bool check(const Reference& ref, const Policy& p, Range a,
                            Range b, Range z) {
            const unsigned int width = float_word_width(ref.type);
            Pair s(ref.type);
            s.x[0] = float_window(s, ref.type, a.minimum, a.maximum);
            s.x[1] = float_window(s, ref.type, b.minimum, b.maximum);
            s.x[2] = float_window(s, ref.type, z.minimum, z.maximum);
            std::set<Row> expected;
            for (WordValue av : float_order_range(width, a.minimum, a.maximum))
              for (WordValue bv :
                   float_order_range(width, b.minimum, b.maximum)) {
                const FloatResult e = ref.evaluate(p, av, bv);
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
              olog << ind(2) << "float-typed multiply " << ref.type << ' ' << p.rounding
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
          // Multiplying the float-typed window of the three codes up to -1 by 1
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
          MultiplyFloatTyped()
              : Base("Word::Float::Arithmetic::Wide::Multiply::FloatTyped") {}
          bool run() override {
            for (WordDomainType format : {WDT_BF16, WDT_FP32})
              if (!windows(Reference(format)))
                return false;
            return bound_deduction();
          }
        };
        /// A policy of the lattice regressions: RDN, flushed subnormal
        /// results and saturation, with the given zeros and tininess
        Policy lattice_policy(Policy::Zeros zeros, Policy::Tininess tininess) {
          Policy p;
          p.rounding = Policy::RDN;
          p.output_subnormals = Policy::SignedZero;
          p.overflow = Policy::SaturateFinite;
          p.zeros = zeros;
          p.nan = Policy::Canonical;
          p.tininess = tininess;
          return p;
        }

        // One factor fixed and known bits on the result: the supports of the
        // free factor form a lattice that the interval projection crossed
        // one encoding per propagation. One status() takes at most 8
        // propagations and keeps every supported tuple: all of them on BF16,
        // known witnesses on FP32.
        class MultiplyLattice : public Base {
          struct Case {
            WordDomainType format;
            Policy policy;
            WordValue fixed;
            Window free, z;
            WordValue mask, known;
            std::vector<WordValue> witnesses;
          };
          static bool supported(const Case& c, const FloatResult& e) {
            return float_order_contains(float_word_width(c.format),
                                        c.z.minimum, c.z.maximum, e.bits) &&
                   (e.bits & c.mask) == c.known;
          }
          static bool check(const Case& c) {
            const Reference ref(c.format);
            const unsigned int width = float_word_width(c.format);
            Pair s(c.format);
            dom(s, s.x[0], c.fixed);
            bounds(s, s.x[1], c.free);
            bounds(s, s.x[2], c.z);
            dom(s, s.x[2], c.known,
                c.known | (~c.mask & ((WordValue(1) << width) - 1)));
            s.post(c.policy, false);
            StatusStatistics statistics;
            const bool failed = s.status(statistics) == SS_FAILED;
            if (statistics.propagate > 8) {
              if (opt.log)
                olog << ind(2) << "multiply lattice " << width << ": "
                     << statistics.propagate << " propagations\n";
              return false;
            }
            const std::vector<WordValue> all =
                c.witnesses.empty()
                    ? float_order_range(width, c.free.minimum, c.free.maximum)
                    : c.witnesses;
            for (WordValue b : all) {
              const FloatResult e = ref.evaluate(c.policy, c.fixed, b);
              if (!supported(c, e)) {
                if (!c.witnesses.empty())
                  return false;
                continue;
              }
              if (failed || !Gecode::Word::WordView(s.x[1]).in(b) ||
                  !Gecode::Word::WordView(s.x[2]).in(e.bits))
                return false;
            }
            return true;
          }

        public:
          MultiplyLattice()
              : Base("Word::Float::Arithmetic::Wide::Multiply::Lattice", TestTags::check()) {}
          bool run() override {
            const Policy p =
                lattice_policy(Policy::Positive, Policy::AfterPrecision);
            const Case cases[] = {
                {WDT_BF16, p, 0xff7f, {0x807f, 0x0001}, {0x3000, 0x5800},
                 0x3, 0x1, {}},
                {WDT_FP32, p, 0xff7fffff, {0x800d4eb0, 0x4839f},
                 {0x378e5a97, 0x5826dbb5}, 0x8208, 0x208,
                 {0x800d4800, 0x80000101}}};
            for (const Case& c : cases)
              if (!check(c))
                return false;
            return true;
          }
        };
        // Products with one factor fixed and known bits on the free one. In
        // the first, a lattice search that tests one candidate per step
        // walks the free factor's bounds over 131 propagations; the
        // known-run substitution settles them. In the second the factor
        // 1 - 2^-24 maps every subnormal to a flushed zero, which interval
        // projections reach one code per execution (7944 executions) and a
        // code-by-code lattice search in 63. One status() of at most 8
        // executions ends at a fixpoint whose moved bounds are witnesses,
        // whose results the reference confirms.
        class MultiplyReplays : public Base {
          static FloatRegression product(const char* name, Policy p,
                                         FloatWord a, FloatWord b,
                                         FloatWord z,
                                         std::vector<std::array<WordValue, 4>> w) {
            return {name,
                    {FloatOperation::Multiply, WDT_FP32, p, false,
                     {a, b, {0, 0, 0, 0}, z}},
                    {point(a), point(b), std::nullopt, std::nullopt},
                    std::move(w)};
          }
          static std::optional<WordValue> point(const FloatWord& x) {
            return x.minimum == x.maximum ? std::optional<WordValue>(x.minimum)
                                          : std::nullopt;
          }

        public:
          MultiplyReplays()
              : Base("Word::Float::Arithmetic::Wide::Multiply::Replays") {}
          bool run() override {
            const FloatRegression cases[] = {
                product("RTZ times -1, free factor with known low bits",
                        float_dump_policy(1, 1, 1, 0, 0, 1, 1),
                        {0xbf800000, 0xbf800000, 0, 0},
                        {0xff7fffff, 0x7f7fffff, 0xe, 0x4},
                        {0x81880a1e, 0xe6b17, 0x1f, 0},
                        {{0xbf800000, 0x807ffff5, 0, 0},
                         {0xbf800000, 0x7ffff5, 0, 0x80000000},
                         {0xbf800000, 0x4, 0, 0x80000000},
                         {0xbf800000, 0x80000004, 0, 0}}),
                product("RNE times 1 - 2^-24, flushed subnormal results",
                        float_dump_policy(0, 0, 1, 0, 0, 1, 0),
                        {0xb463b5b9, 0x91f7734, 0x10604604, 0x4604},
                        {0x3f7fffff, 0x3f7fffff, 0, 0},
                        {0xff7fffff, 0x7f7fffff, 0x2080204, 0},
                        {{0x801fffff, 0x3f7fffff, 0, 0x80000000},
                         {0x1fffff, 0x3f7fffff, 0, 0}})};
            // Every case runs, so that a log names each one that fails
            const Reference ref(WDT_FP32);
            bool passed = true;
            for (const FloatRegression& c : cases) {
              passed &= check_float_regression(c, 8);
              for (const auto& w : c.witnesses)
                passed &= ref.evaluate(c.posting.policy, w[0], w[1]).bits == w[3];
            }
            return passed;
          }
        };

        // A result aliasing an operand on small BF16 windows: the
        // propagated domains equal the hulls of the brute-force supports of
        // x*y = y, so the alias classes keep every support and no more here.
        class MultiplyAliasProjection : public Base {
          static bool check(const Reference& ref, const Policy& p, Window x,
                            Window y) {
            Pair s(ref.type);
            s.x[2] = s.x[1];
            bounds(s, s.x[0], x);
            bounds(s, s.x[1], y);
            s.post(p, false);
            std::set<WordValue> xs, ys;
            for (WordValue a : float_order_range(16, x.minimum, x.maximum))
              for (WordValue b : float_order_range(16, y.minimum, y.maximum))
                if (ref.evaluate(p, a, b).bits == b) {
                  xs.insert(a);
                  ys.insert(b);
                }
            if (s.status() == SS_FAILED)
              return xs.empty();
            if (xs.empty())
              return false;
            for (WordValue a : xs)
              if (!Gecode::Word::WordView(s.x[0]).in(a))
                return false;
            for (WordValue b : ys)
              if (!Gecode::Word::WordView(s.x[1]).in(b))
                return false;
            const auto hx = float_order_extremes(16, xs);
            const auto hy = float_order_extremes(16, ys);
            if (s.x[0].minimum() != hx.first || s.x[0].maximum() != hx.second ||
                s.x[1].minimum() != hy.first || s.x[1].maximum() != hy.second) {
              if (opt.log)
                olog << ind(2) << "multiply alias projection: " << std::hex
                     << s.x[0].minimum() << ".." << s.x[0].maximum() << ' '
                     << s.x[1].minimum() << ".." << s.x[1].maximum()
                     << std::dec << '\n';
              return false;
            }
            return true;
          }

        public:
          MultiplyAliasProjection()
              : Base("Word::Float::Arithmetic::Wide::Multiply::AliasProjection") {}
          bool run() override {
            const Reference ref(WDT_BF16);
            for (int r = 0; r < 4; ++r) {
              Policy p;
              p.rounding = static_cast<Policy::Rounding>(r);
              if (!check(ref, p, {0x3f00, 0x3fc0}, {0x0005, 0x0040}))
                return false;
            }
            // No support: x*y = y needs x = 1 for these finite y, and x lies
            // near 2^-15, so the space fails; y = 0x7f7f, the largest finite
            // code, is not a support either.
            if (!check(ref, Policy(), {0x381e, 0x3866}, {0x7f44, 0x7f7f}))
              return false;
            return true;
          }
        };
        // The modular-window helpers of the lattice searches against direct
        // enumeration: every modulus up to a bound with every window, and
        // BF16-sized arguments from a fixed generator.
        class WindowHelpers : public Base {
          using Integer = Gecode::Word::Arithmetic::Global::Integer;
          static Integer integer(long long v) {
            return v < 0 ? -Integer(static_cast<std::uint64_t>(-v))
                         : Integer(static_cast<std::uint64_t>(v));
          }
          static long long value(const Integer& x) {
            const long long v = static_cast<long long>(x.abs().word());
            return x.neg() ? -v : v;
          }
          static long long modulo(long long x, long long m) {
            return ((x % m) + m) % m;
          }
          bool report(const char* helper, long long a, long long m,
                      long long low, long long high) const {
            if (opt.log)
              olog << ind(2) << "window helper " << helper << ": a=" << a << " m=" << m
                   << ' ' << low << ' ' << high << '\n';
            return false;
          }
          // least t >= 0 with low <= (a*t) mod m <= high
          bool least(long long a, long long m, long long low,
                     long long high) const {
            long long expected = -1;
            for (long long t = 0; t < m && expected < 0; ++t)
              if (modulo(a * t, m) >= low && modulo(a * t, m) <= high)
                expected = t;
            Integer t;
            const bool found = Gecode::Word::Float::least_in_window(
                integer(a), integer(m), integer(low), integer(high), t);
            return (found ? expected >= 0 && value(t) == expected
                          : expected < 0) ||
                   report("least_in_window", a, m, low, high);
          }
          // least t >= 0 with (a*t + base) mod m <= width
          bool offset(long long a, long long m, long long base,
                      long long width) const {
            long long expected = -1;
            for (long long t = 0; t < m && expected < 0; ++t)
              if (modulo(a * t + base, m) <= width)
                expected = t;
            Integer t;
            const bool found = Gecode::Word::Float::least_offset(
                integer(a), integer(m), integer(base), integer(width), t);
            return (found ? expected >= 0 && value(t) == expected
                          : expected < 0) ||
                   report("least_offset", a, m, base, width);
          }
          // first and last x in [from, to] with (a*x - low) mod m <= high - low
          bool window(long long a, long long m, long long low, long long high,
                      long long from, long long to) const {
            bool any = false;
            long long first = 0, last = 0;
            for (long long x = from; x <= to; ++x)
              if (modulo(a * x - low, m) <= high - low) {
                if (!any)
                  first = x;
                last = x;
                any = true;
              }
            Integer x;
            if (Gecode::Word::Float::first_in_window(
                    integer(a), integer(m), integer(low), integer(high),
                    integer(from), integer(to), x)
                    ? !any || value(x) != first
                    : any)
              return report("first_in_window", a, m, from, to);
            if (Gecode::Word::Float::last_in_window(
                    integer(a), integer(m), integer(low), integer(high),
                    integer(from), integer(to), x)
                    ? !any || value(x) != last
                    : any)
              return report("last_in_window", a, m, from, to);
            return true;
          }
          // least and greatest x in [from, to] with known bits on x and on
          // x + addend, modulo 2^64
          bool masks(std::uint64_t from, std::uint64_t to, std::uint64_t x_mask,
                     std::uint64_t x_value, std::uint64_t addend,
                     std::uint64_t sum_mask, std::uint64_t sum_value) const {
            bool any = false;
            std::uint64_t first = 0, last = 0;
            for (std::uint64_t x = from;; ++x) {
              if ((x & x_mask) == x_value &&
                  ((x + addend) & sum_mask) == sum_value) {
                if (!any)
                  first = x;
                last = x;
                any = true;
              }
              if (x == to)
                break;
            }
            std::uint64_t x;
            if (Gecode::Word::Float::least_with_masks(from, to, x_mask, x_value,
                                                      addend, sum_mask,
                                                      sum_value, x)
                    ? !any || x != first
                    : any)
              return report("least_with_masks", static_cast<long long>(addend),
                            0, static_cast<long long>(from),
                            static_cast<long long>(to));
            if (Gecode::Word::Float::greatest_with_masks(
                    from, to, x_mask, x_value, addend, sum_mask, sum_value, x)
                    ? !any || x != last
                    : any)
              return report("greatest_with_masks",
                            static_cast<long long>(addend), 0,
                            static_cast<long long>(from),
                            static_cast<long long>(to));
            return true;
          }

        public:
          WindowHelpers()
              : Base("Word::Float::Arithmetic::Wide::WindowHelpers") {}
          bool run() override {
            for (long long m = 1; m <= 40; ++m)
              for (long long a = 0; a < m; ++a)
                for (long long low = 0; low < m; ++low)
                  for (long long high = low; high < m; ++high)
                    if (!least(a, m, low, high))
                      return false;
            for (long long m = 1; m <= 16; ++m)
              for (long long a = -m; a <= m; ++a)
                for (long long base = -m; base < 2 * m; ++base)
                  for (long long width = 0; width <= m; ++width)
                    if (!offset(a, m, base, width))
                      return false;
            for (long long m = 1; m <= 8; ++m)
              for (long long a = -m; a <= m; ++a)
                for (long long low = 0; low < m; ++low)
                  for (long long high = low; high <= low + m; ++high)
                    for (long long from = -2; from <= m; ++from)
                      for (long long to = from - 1; to <= from + m + 1; ++to)
                        if (!window(a, m, low, high, from, to))
                          return false;
            // BF16-sized: significands below 2^9, moduli up to 2^10
            std::mt19937_64 random(7);
            for (int i = 0; i < 20000; ++i) {
              const long long m = i % 2 ? 1LL << (random() % 11)
                                        : 1 + static_cast<long long>(
                                                  random() % 1024);
              const long long a =
                  static_cast<long long>(random() % 1025) - 512;
              const long long low =
                  static_cast<long long>(random() % 2048) - 1024;
              const long long high =
                  low + static_cast<long long>(random() % (2 * m + 1));
              const long long from =
                  static_cast<long long>(random() % 600) - 100;
              const long long to =
                  from + static_cast<long long>(random() % 600) - 10;
              if (!window(a, m, low, high, from, to))
                return false;
            }
            const std::uint64_t starts[] = {0, 1000, std::uint64_t(1) << 32,
                                            std::uint64_t(0) - 600};
            for (int i = 0; i < 20000; ++i) {
              const std::uint64_t from = starts[i % 4] + random() % 256;
              const std::uint64_t to = from + random() % 300;
              if (to < from)
                continue;
              const std::uint64_t addend =
                  i % 3 == 0   ? random() % 2048
                  : i % 3 == 1 ? std::uint64_t(0) - random() % 2048
                               : random();
              const std::uint64_t x_mask =
                  random() & random() & ((std::uint64_t(1) << 12) - 1);
              std::uint64_t sum_mask =
                  random() & random() & ((std::uint64_t(1) << 12) - 1);
              if (i % 5 == 0)
                sum_mask |= std::uint64_t(1) << (12 + random() % 52);
              // Masks of a code in [from, to] mostly, to have solutions
              const std::uint64_t witness = from + random() % (to - from + 1);
              const bool fit = i % 4 != 0;
              const std::uint64_t x_value =
                  (fit ? witness : random()) & x_mask;
              const std::uint64_t sum_value =
                  (fit ? witness + addend : random()) & sum_mask;
              if (!masks(from, to, x_mask, x_value, addend, sum_mask,
                         sum_value))
                return false;
            }
            return true;
          }
        };
        // The step rule on random small BF16 windows over every policy
        // field, with and without a status: status() keeps exactly the
        // solutions of the oracle (check_window compares them by search),
        // settles in one execution, and ends at a fixpoint, which a fresh
        // propagator on a clone leaves unchanged.
        class MultiplyStepRule : public Base {
        public:
          MultiplyStepRule()
              : Base("Word::Float::Arithmetic::Wide::Multiply::StepRule") {}
          bool run() override {
            const Reference ref(WDT_BF16);
            for (int n = 0; n < 300; ++n) {
              const Policy p = float_dump_policy(
                  static_cast<int>(_rand(4)), static_cast<int>(_rand(2)),
                  static_cast<int>(_rand(2)), static_cast<int>(_rand(2)),
                  static_cast<int>(_rand(2)), static_cast<int>(_rand(3)),
                  static_cast<int>(_rand(2)));
              Window w[3];
              for (int i = 0; i < 3; ++i)
                draw_float_window(_rand, WDT_BF16,
                                  i == 2 && _rand(3) == 0 ? 1U << 16 : 24,
                                  w[i].minimum, w[i].maximum);
              const int status =
                  _rand(4) == 0 ? static_cast<int>(_rand(32)) : -1;
              Pair s(ref.type);
              bounds(s, s.x[0], w[0]);
              bounds(s, s.x[1], w[1]);
              bounds(s, s.x[2], w[2]);
              if (status >= 0)
                rel(s, s.flags, IRT_EQ, status);
              s.post(p);
              if (!check_window(ref, p, w[0], w[1], w[2], -1, status) ||
                  !settles_in_one_execution(s, p)) {
                olog << ind(2) << "multiply step rule " << std::hex
                     << w[0].minimum << ' ' << w[0].maximum << ' '
                     << w[1].minimum << ' ' << w[1].maximum << ' '
                     << w[2].minimum << ' ' << w[2].maximum << std::dec << ' '
                     << status << '\n';
                return false;
              }
            }
            return true;
          }
        };

        // x * 1.0 on BF16 under RDN with flushed input subnormals, with and
        // without a status. x admits every non-NaN code whose bits 2 and 8
        // are clear, so the products are exact and the result settles at
        // the hull of x, [0xfefb..0x7efb], in one execution. A narrowing of
        // the status alone must not send the step rule to the widened
        // projection, which keeps the result at [0xff7f..0x7f7f].
        class MultiplyStepStatus : public Base {
          static FloatRegression times_one(const char* name, bool status) {
            return {name,
                    {FloatOperation::Multiply, WDT_BF16,
                     float_dump_policy(2, 1, 0, 0, 0, 1, 1), status,
                     {{0x3f80, 0x3f80, 0, 0},
                      {0xff80, 0x7f80, 0x104, 0},
                      {0, 0, 0, 0},
                      {0xffff, 0x7fff, 0, 0}}},
                    {},
                    {{0x3f80, 0xfefb, 0, 0xfefb}, {0x3f80, 0x7efb, 0, 0x7efb}}};
          }

        public:
          MultiplyStepStatus()
              : Base("Word::Float::Arithmetic::Wide::Multiply::StepStatus") {}
          bool run() override {
            const FloatRegression cases[] = {
                times_one("x * 1.0 with a status", true),
                times_one("x * 1.0 without a status", false)};
            const Reference ref(WDT_BF16);
            bool passed = true;
            for (const FloatRegression& c : cases) {
              passed &= check_float_regression(c, 2);
              for (const auto& w : c.witnesses)
                passed &= ref.evaluate(c.posting.policy, w[0], w[1]).bits == w[3];
            }
            return passed;
          }
        };

        MultiplyExact multiply_exact;
        MultiplyStepRule multiply_step_rule;
        MultiplyStepStatus multiply_step_status;
        MultiplyAliasProjection multiply_alias_projection;
        MultiplySymbolic multiply_symbolic;
        MultiplyLifecycle multiply_lifecycle;
        MultiplyPosting multiply_posting;
        MultiplyFloatTyped multiply_float_typed;
        MultiplyLattice multiply_lattice;
        MultiplyReplays multiply_replays;
        WindowHelpers window_helpers;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test
// STATISTICS: test-word
