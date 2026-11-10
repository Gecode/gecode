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
        // The expected quotient from the shared exact oracle: the special
        // cases of fp_divide(), then one rounding of the exact quotient.
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
            if ((am == 0 && bm == 0) || (am == infinity && bm == infinity))
              return {nan, 1};
            if (am == infinity)
              return {infinity + (negative ? sign : 0), 0};
            if (bm == 0)
              return {infinity + (negative ? sign : 0), 2};
            if (am == 0 || bm == infinity)
              return {negative && p.zeros == Policy::Signed ? sign : 0, 0};
            const ExactRatio av = value(am), bv = value(bm);
            return round_exact(*this,
                               {av.numerator * bv.denominator,
                                av.denominator * bv.numerator},
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
              fp_divide(*this, x[0], x[1], x[2], flags, p);
            else
              fp_divide(*this, x[0], x[1], x[2], p);
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
            olog << ind(2) << "divide exact mismatch " << ref.fraction << ' '
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
        /// The window of the encodings with sign bit \a sign, which may be
        /// zero, and magnitudes from \a low to \a high
        Window magnitudes(WordValue sign, WordValue low, WordValue high) {
          return sign != 0 ? Window{sign | high, sign | low}
                           : Window{low, high};
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
          Pair s(ref.type);
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
              olog << ind(2) << "divide did not prune " << ref.type << ' '
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
            olog << ind(2) << "divide relation mismatch " << ref.type << ' '
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
          Pair s(ref.type);
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
              const FloatResult value = ref.evaluate(p, a, b);
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
              v = WordVar(*this, ref.type);
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
                  olog << ind(2) << "divide network failed " << f << ' ' << r
                            << '\n';
                  return false;
                }
                if (Gecode::Word::WordView(original.x[0])
                        .in(ref.one - 4) &&
                    Gecode::Word::WordView(original.x[0])
                        .in(ref.one + 4)) {
                  olog << ind(2) << "divide network did not prune " << f << ' '
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
              olog << ind(2) << "float-typed divide " << ref.type << ' ' << p.rounding
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
        /// A policy of the lattice regressions: RDN, flushed subnormal
        /// results and saturation, with the given tininess
        Policy lattice_policy(Policy::Tininess tininess) {
          Policy p;
          p.rounding = Policy::RDN;
          p.output_subnormals = Policy::SignedZero;
          p.overflow = Policy::SaturateFinite;
          p.nan = Policy::Canonical;
          p.tininess = tininess;
          return p;
        }

        // A fixed divisor and known bits on the result: the supports of the
        // dividend form a lattice that the interval projection crossed one
        // or two encodings per propagation. One status() takes at most 8
        // propagations and keeps every supported tuple: all of them on BF16,
        // known witnesses on FP32.
        class DivideLattice : public Base {
          struct Case {
            WordDomainType format;
            Policy policy;
            Window free;
            WordValue free_mask, free_known;
            WordValue divisor;
            Window z;
            WordValue mask, known;
            std::vector<WordValue> witnesses;
          };
          static bool supported(const Case& c, WordValue a, const FloatResult& e) {
            return (a & c.free_mask) == c.free_known &&
                   float_order_contains(float_word_width(c.format),
                                        c.z.minimum, c.z.maximum, e.bits) &&
                   (e.bits & c.mask) == c.known;
          }
          static bool check(const Case& c) {
            const Reference ref(c.format);
            const unsigned int width = float_word_width(c.format);
            const WordValue all_bits = (WordValue(1) << width) - 1;
            Pair s(c.format);
            bounds(s, s.x[0], c.free);
            dom(s, s.x[0], c.free_known, c.free_known | (~c.free_mask & all_bits));
            dom(s, s.x[1], c.divisor);
            bounds(s, s.x[2], c.z);
            dom(s, s.x[2], c.known, c.known | (~c.mask & all_bits));
            s.post(c.policy);
            StatusStatistics statistics;
            const bool failed = s.status(statistics) == SS_FAILED;
            if (statistics.propagate > 8) {
              if (opt.log)
                olog << ind(2) << "divide lattice " << width << ": "
                     << statistics.propagate << " propagations\n";
              return false;
            }
            const std::vector<WordValue> all =
                c.witnesses.empty()
                    ? float_order_range(width, c.free.minimum, c.free.maximum)
                    : c.witnesses;
            for (WordValue a : all) {
              const FloatResult e = ref.evaluate(c.policy, a, c.divisor);
              if (!supported(c, a, e)) {
                if (!c.witnesses.empty())
                  return false;
                continue;
              }
              if (failed || !Gecode::Word::WordView(s.x[0]).in(a) ||
                  !Gecode::Word::WordView(s.x[2]).in(e.bits) ||
                  !s.flags.in(e.flags))
                return false;
            }
            return true;
          }

        public:
          DivideLattice()
              : Base("Word::Float::Arithmetic::Wide::Divide::Lattice", TestTags::check()) {}
          bool run() override {
            const Case cases[] = {
                {WDT_BF16, lattice_policy(Policy::BeforeRounding),
                 {0x83c7, 0x0657}, 0, 0, 0x8004, {0x3c2b, 0x5754}, 0x3, 0x1,
                 {}},
                {WDT_FP32, lattice_policy(Policy::BeforeRounding),
                 {0x83c78aef, 0x6573bea}, 0x10052140, 0x52140, 0x80000004,
                 {0x3c2bac2e, 0x5754cb8a}, 0x2000003c, 0x3c,
                 {0x83c77fff, 0x8025214f}}};
            for (const Case& c : cases)
              if (!check(c))
                return false;
            return true;
          }
        };
        // Quotients with a fixed divisor and known bits on the dividend and
        // the result, which a lattice search that tested one candidate per
        // step crossed over 309 and 85 propagations. In the
        // first one the divisor is -2^-125, so every subnormal dividend
        // gives an exact quotient whose low bits are zero: such a quotient
        // lies at the open end of a cell, which the search now excludes.
        // One status() of at most 8 executions reaches a fixpoint whose
        // moved bounds are witnesses, whose results the reference confirms.
        class DivideReplays : public Base {
          static FloatRegression quotient(const char* name, Policy p,
                                          FloatWord a, FloatWord b,
                                          FloatWord z,
                                          std::vector<std::array<WordValue, 4>> w) {
            return {name,
                    {FloatOperation::Divide, WDT_FP32, p, false,
                     {a, b, {0, 0, 0, 0}, z}},
                    {std::nullopt, b.minimum, std::nullopt, std::nullopt},
                    std::move(w)};
          }

        public:
          DivideReplays()
              : Base("Word::Float::Arithmetic::Wide::Divide::Replays") {}
          bool run() override {
            const FloatRegression cases[] = {
                quotient("RDN, divisor -2^-125, subnormal dividends, low result bits known",
                         float_dump_policy(2, 0, 0, 0, 0, 0, 0),
                         {0x9439dfda, 0x255cfb20, 0x80000200, 0x80000000},
                         {0x81000000, 0x81000000, 0, 0},
                         {0x3dca6914, 0x3fc7a400, 0x7, 0x7},
                         {{0x8147a1ff, 0x81000000, 0, 0x3fc7a1ff},
                          {0x80800007, 0x81000000, 0, 0x3f000007}}),
                quotient("RUP, divisor 2^14, dividends across zero, low result bits known",
                         float_dump_policy(3, 0, 0, 0, 0, 1, 0),
                         {0x89fcac82, 0x15a53e84, 0xc, 0},
                         {0x46800000, 0x46800000, 0, 0},
                         {0x8066ccca, 0xcb4693, 0x1f, 0x17},
                         {{0x85ffff73, 0x46800000, 0, 0x800ffff7},
                          {0x67fffb3, 0x46800000, 0, 0x1ffff7},
                          {0x85ffff70, 0x46800000, 0, 0x800ffff7},
                          {0x67fffb2, 0x46800000, 0, 0x1ffff7}}),
                // x / 1 is x, whose known bit 11 the result excludes: the
                // code-by-code lattice search took 304 executions to fail,
                // the interval passes 19452
                {"RNE divide by one without support",
                 {FloatOperation::Divide,
                  WDT_FP32,
                  float_dump_policy(0, 0, 0, 1, 0, 1, 1),
                  false,
                  {{0xff7fffff, 0x7f7fffff, 0x800, 0x800},
                   {0x3f800000, 0x3f800000, 0, 0},
                   {0, 0, 0, 0},
                   {0x817fc7e2, 0x1c884dc4, 0x52000a10, 0x210}}},
                 {},
                 {},
                 true},
                // A divisor of one or two least subnormals, whose widened
                // read reaches down to zero: the hull projections assign
                // both operands, as the interval passes did
                {"RUP divide by a subnormal",
                 {FloatOperation::Divide,
                  WDT_FP32,
                  float_dump_policy(3, 0, 0, 1, 0, 0, 0),
                  false,
                  {{0x8085d1a5, 0x000fabbc, 0, 0},
                   {0x1, 0x2, 0, 0},
                   {0, 0, 0, 0},
                   {0xca7ffffe, 0xca7ffffe, 0, 0}}},
                 {0x807fffff, 0x2, std::nullopt, std::nullopt},
                 {{0x807fffff, 0x2, 0, 0xca7ffffe}}}};
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

        // FP32 quotients with a single or nearly single result and wide
        // operands. Each interval projection moved a bound
        // of the dividend and the divisor a few codes, and the passes took
        // 3,226 to more than 16,000 executions (up to 10 s). The step rule
        // settles each in one execution. The limit of 2 counts the runs of
        // the replay's watcher, after posting and after the execution.
        class DivideStepBudget : public Base {
          static FloatRegression settles(const char* name, Policy p,
                                         bool status, FloatWord a,
                                         FloatWord b, FloatWord z) {
            return {name,
                    {FloatOperation::Divide, WDT_FP32, p, status,
                     {a, b, {0, 0, 0, 0}, z}},
                    {},
                    {},
                    false,
                    FloatMovedBounds::Free};
          }

        public:
          DivideStepBudget()
              : Base("Word::Float::Arithmetic::Wide::Divide::StepBudget") {}
          bool run() override {
            const FloatRegression cases[] = {
                settles("saturating, result near FLT_MAX, operands from NaN", float_dump_policy(0, 0, 0, 1, 1, 1, 1),
                        false, {0xffffffff, 0xf8d455e8, 0, 0},
                        {0xfacc37e5, 0x7fffffff, 0, 0},
                        {0x7f7ffffd, 0x7f7ffffd, 0, 0}),
                settles("result just below 1, divisor near zero with known bits",
                        float_dump_policy(0, 0, 0, 0, 1, 0, 1), false,
                        {0x9496974b, 0x158bc88f, 0, 0},
                        {0x80d6cc20, 0x001a19ca, 0x88b01000, 0x80800000},
                        {0x3f7ffff8, 0x3f7ffff8, 0, 0}),
                settles("RUP, negative divisor, result near -FLT_MAX", float_dump_policy(3, 0, 1, 0, 0, 0, 1),
                        true, {0xbee41fb5, 0x7fffffff, 0, 0},
                        {0xc8c6be0e, 0xb00ddeb7, 0, 0},
                        {0xff7ffffd, 0xff7ffffd, 0, 0}),
                settles("RUP, result just below -1, operands with known bits", float_dump_policy(3, 0, 0, 1, 0, 1, 0),
                        true, {0xffbfffff, 0x7f800000, 0x80400000, 0x80000000},
                        {0xffffffff, 0x7f80001a, 0x40022, 0x40022},
                        {0xbf800001, 0xbf800001, 0xc08084, 0x800000})};
            bool passed = true;
            for (const FloatRegression& c : cases)
              passed &= check_float_regression(c, 2);
            return passed;
          }
        };

        // floor_shift_right and ceil_shift_right, which scale the division
        // bounds, round like floor_div and ceil_div by 2^k, also for negative
        // values and across limbs.
        class DivideShifts : public Base {
        public:
          DivideShifts()
              : Base("Word::Float::Arithmetic::Wide::Divide::Shifts") {}
          bool run() override {
            using namespace Gecode::Word::Arithmetic::Global;
            const Integer wide = Integer::power_two(70) + Integer(5);
            for (int a = -71; a <= 71; ++a)
              for (unsigned int k = 0; k <= 72; ++k) {
                const Integer scale = Integer::power_two(k);
                for (const Integer& x : {Integer::signed_value(a),
                                         wide * Integer::signed_value(a)})
                  if (floor_shift_right(x, k) != floor_div(x, scale) ||
                      ceil_shift_right(x, k) != ceil_div(x, scale))
                    return false;
              }
            return true;
          }
        };

        // A result aliasing the dividend on small BF16 windows: the
        // propagated domains equal the hulls of the brute-force supports of
        // x/y = x, except under RUP. There x = 5 and y = 1.25 give exactly
        // the previous code 4, and the closed division cells keep such a
        // boundary quotient.
        class DivideAliasProjection : public Base {
          static bool check(const Reference& ref, const Policy& p, Window x,
                            Window y, bool exact) {
            Pair s(ref.type);
            s.x[2] = s.x[0];
            bounds(s, s.x[0], x);
            bounds(s, s.x[1], y);
            s.post(p, false);
            std::set<WordValue> xs, ys;
            for (WordValue a : float_order_range(16, x.minimum, x.maximum))
              for (WordValue b : float_order_range(16, y.minimum, y.maximum))
                if (ref.evaluate(p, a, b).bits == a) {
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
            if (!exact)
              return true;
            const auto hx = float_order_extremes(16, xs);
            const auto hy = float_order_extremes(16, ys);
            if (s.x[0].minimum() != hx.first || s.x[0].maximum() != hx.second ||
                s.x[1].minimum() != hy.first || s.x[1].maximum() != hy.second) {
              if (opt.log)
                olog << ind(2) << "divide alias projection: " << std::hex
                     << s.x[0].minimum() << ".." << s.x[0].maximum() << ' '
                     << s.x[1].minimum() << ".." << s.x[1].maximum()
                     << std::dec << '\n';
              return false;
            }
            return true;
          }

        public:
          DivideAliasProjection()
              : Base("Word::Float::Arithmetic::Wide::Divide::AliasProjection") {}
          bool run() override {
            const Reference ref(WDT_BF16);
            for (int r = 0; r < 4; ++r) {
              Policy p;
              p.rounding = static_cast<Policy::Rounding>(r);
              if (!check(ref, p, {0x0005, 0x0040}, {0x3f00, 0x3fc0},
                         p.rounding != Policy::RUP))
                return false;
            }
            // Only x = 0x7f7f is supported, by the positive y, so the
            // negative part of y goes.
            Policy p;
            p.rounding = Policy::RTZ;
            if (!check(ref, p, {0x7f6e, 0x7f7f}, {0x801a, 0x002f}, true))
              return false;
            return true;
          }
        };
        // The step rule on random small BF16 windows over every policy
        // field, with and without a status: status() keeps exactly the
        // solutions of the oracle (check_window compares them by search),
        // settles in one execution, and ends at a fixpoint, which a fresh
        // propagator on a clone leaves unchanged.
        class DivideStepRule : public Base {
        public:
          DivideStepRule()
              : Base("Word::Float::Arithmetic::Wide::Divide::StepRule") {}
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
                olog << ind(2) << "divide step rule " << std::hex
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

        // A BF16 quotient with status by the subnormal divisor -2^-131
        // (0x8004) under RTZ. Dividing by a power of two is exact here, so
        // the result settles at the hull of a/b over the ends of a,
        // [0xc64e..0x679e], in one execution. A narrowing of the status
        // alone must not send the step rule to the widened projection,
        // which keeps the result at [0xc658..0x67a8].
        class DivideStepStatus : public Base {
        public:
          DivideStepStatus()
              : Base("Word::Float::Arithmetic::Wide::Divide::StepStatus") {}
          bool run() override {
            const FloatRegression c{
                "RTZ divide by -2^-131 with a status",
                {FloatOperation::Divide,
                 WDT_BF16,
                 float_dump_policy(1, 0, 1, 1, 0, 1, 1),
                 true,
                 {{0xa622, 0x04cf, 0x1009, 0x8},
                  {0x8004, 0x8004, 0, 0},
                  {0, 0, 0, 0},
                  {0xffff, 0x7fff, 0, 0}}},
                {},
                {{0x04ce, 0x8004, 0, 0xc64e}, {0xa61e, 0x8004, 0, 0x679e}}};
            const Reference ref(WDT_BF16);
            bool passed = check_float_regression(c, 2);
            for (const auto& w : c.witnesses)
              passed &= ref.evaluate(c.posting.policy, w[0], w[1]).bits == w[3];
            return passed;
          }
        };

        DivideExact divide_exact;
        DivideStepRule divide_step_rule;
        DivideStepStatus divide_step_status;
        DivideAliasProjection divide_alias_projection;
        DivideSymbolic divide_symbolic;
        DivideLifecycle divide_lifecycle;
        DividePosting divide_posting;
        DivideFloatTyped divide_float_typed;
        DivideLattice divide_lattice;
        DivideReplays divide_replays;
        DivideStepBudget divide_step_budget;
        DivideShifts divide_shifts;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test
// STATISTICS: test-word
