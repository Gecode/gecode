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

namespace Test {
  namespace Word {
    namespace Float {
      namespace {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        // The expected sum from the shared exact oracle: the special cases of
        // fp_add() and fp_subtract(), then one rounding of the exact sum.
        struct Reference : FloatFormat {
          explicit Reference(WordDomainType f) : FloatFormat(float_format(f)) {}
          /// The value of effective operand magnitude \a m under \a p
          ExactRatio operand(const Policy& p, WordValue m) const {
            return p.input_subnormals == Policy::SignedZero && m < normal
                       ? dyadic(0, 0)
                       : value(m);
          }
          FloatResult evaluate(bool subtract, const Policy& p, WordValue a,
                               WordValue b) const {
            const WordValue am = magnitude(a), bm = magnitude(b);
            const bool an = a >= sign, bn = (b >= sign) != subtract;
            if (is_nan(a) || is_nan(b))
              return nan_result(*this, p, {a, b});
            if (am == infinity && bm == infinity && an != bn)
              return {nan, 1};
            if (am == infinity || bm == infinity)
              return {infinity + ((am == infinity ? an : bn) ? sign : 0), 0};
            const ExactRatio av = operand(p, am), bv = operand(p, bm);
            ExactInteger x = av.numerator * bv.denominator;
            ExactInteger y = bv.numerator * av.denominator;
            if (an)
              x = -x;
            if (bn)
              y = -y;
            const ExactInteger sum = x + y;
            if (sum.zero()) {
              const bool negative = an == bn ? an : p.rounding == Policy::RDN;
              return {negative && p.zeros == Policy::Signed ? sign : 0, 0};
            }
            return round_exact(*this,
                               {sum.abs(), av.denominator * bv.denominator},
                               sum.neg(), p);
          }
        };

        void post_relation(Space& s, bool subtract, const Policy& p,
                           WordVar a, WordVar b, WordVar result,
                           IntVar* flags) {
          if (subtract) {
            if (flags)
              fp_subtract(s, a, b, result, *flags, p);
            else
              fp_subtract(s, a, b, result, p);
          } else {
            if (flags)
              fp_add(s, a, b, result, *flags, p);
            else
              fp_add(s, a, b, result, p);
          }
        }

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
        };

        bool same_domains(const Pair& a, const Pair& b) {
          for (int i = 0; i < 3; ++i) {
            const WordVar x = a.x[i], y = b.x[i];
            if (x.lo() != y.lo() || x.hi() != y.hi() ||
                (x.bounded() &&
                 (x.minimum() != y.minimum() || x.maximum() != y.maximum())))
              return false;
          }
          for (int flag = 0; flag < 32; ++flag)
            if (a.flags.in(flag) != b.flags.in(flag))
              return false;
          return true;
        }

        bool closed_relation(Pair& original, bool sub, const Policy& p) {
          std::unique_ptr<Pair> clone(static_cast<Pair*>(original.clone()));
          post_relation(*clone, sub, p, clone->x[0], clone->x[1], clone->x[2],
                        &clone->flags);
          return clone->status() != SS_FAILED &&
                 same_domains(original, *clone);
        }

        bool check_exact(const Reference& ref, bool subtract, const Policy& p,
                         WordValue a, WordValue b) {
          const FloatResult expected = ref.evaluate(subtract, p, a, b);
          Pair s(ref.type);
          dom(s, s.x[0], a);
          dom(s, s.x[1], b);
          post_relation(s, subtract, p, s.x[0], s.x[1], s.x[2],
                        &s.flags);
          if (s.status() == SS_FAILED)
            return false;
          if (!s.x[2].assigned() ||
              s.x[2].val() != expected.bits || !s.flags.assigned() ||
              s.flags.val() != expected.flags) {
            olog << ind(2) << "wide exact mismatch " << ref.fraction << ' '
                      << subtract << ' ' << p.rounding << ' ' << std::hex << a
                      << ' ' << b << " expected " << expected.bits << std::dec
                      << '/' << expected.flags << '\n';
            return false;
          }
          return true;
        }

        class WideExact : public Base {
        public:
          WideExact() : Base("Word::Float::Arithmetic::Wide::Exact") {}
          bool run() override {
            for (WordDomainType format : {WDT_BF16, WDT_FP32}) {
              const Reference ref(format);
              const std::vector<WordValue> values{
                  0,
                  ref.sign,
                  1,
                  ref.sign + 1,
                  ref.normal - 1,
                  ref.normal,
                  ref.normal + 1,
                  ref.one - 1,
                  ref.one,
                  ref.one + 1,
                  ref.one + ref.sign,
                  ref.one - ref.fraction * ref.normal,
                  ref.maximum,
                  ref.maximum + ref.sign,
                  ref.infinity,
                  ref.infinity + ref.sign,
                  ref.infinity + 1,
                  ref.infinity + ref.quiet + 3,
                  ref.sign + ref.infinity + 5,
                  ref.sign + ref.infinity + ref.quiet + 7};
              for (bool subtract : {false, true})
                for (int rounding = 0; rounding < 4; ++rounding)
                  for (WordValue a : values)
                    for (WordValue b : values)
                      if (!check_exact(
                              ref, subtract,
                              Policy{static_cast<Policy::Rounding>(rounding)},
                              a, b))
                        return false;
              // The overflow midpoint lies half a top-binade ULP above M.
              // Check both adjacent addends, signs and subtraction's raw sign.
              const WordValue half_ulp = (253 - ref.fraction) * ref.normal;
              for (int r = 0; r < 4; ++r)
                for (WordValue sign : {WordValue(0), ref.sign})
                  for (bool sub : {false, true})
                    for (WordValue b = half_ulp - 1; b <= half_ulp + 1; ++b)
                      if (!check_exact(ref, sub,
                                       Policy{static_cast<Policy::Rounding>(r)},
                                       sign | ref.maximum,
                                       b | (sub ? sign ^ ref.sign : sign)))
                        return false;
              // Every policy combination on a fixed semantic catalogue,
              // independent of the broader boundary cross product above.
              const std::pair<WordValue, WordValue> catalogue[] = {
                  {ref.one, ref.one - (ref.fraction + 1) * ref.normal},
                  {ref.one + 1, ref.one - (ref.fraction + 1) * ref.normal},
                  {ref.maximum, ref.one},
                  {ref.maximum, ref.maximum},
                  {ref.normal, ref.sign + ref.normal - 1},
                  {ref.sign, 0},
                  {ref.infinity + ref.quiet + 1, ref.sign + ref.infinity + 2},
                  {ref.infinity, ref.sign + ref.infinity}};
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
                              for (bool subtract : {false, true})
                                for (const auto& pair : catalogue)
                                  if (!check_exact(ref, subtract, p, pair.first,
                                                   pair.second))
                                    return false;
                            }
            }
            // Literal contract examples anchor the independent oracle and
            // flags.
            const Reference bf(WDT_BF16), fp(WDT_FP32);
            const struct {
              WordDomainType format;
              bool sub;
              WordValue a, b, result;
              int flags;
            } examples[] = {
                {WDT_BF16, false, 0x3f80, 0x3b80, 0x3f80, 16},
                {WDT_BF16, false, 0x3f81, 0x3b80, 0x3f82, 16},
                {WDT_BF16, true, 0x0080, 0x007f, 0x0001, 0},
                {WDT_FP32, false, 0x3f800000, 0x33800000, 0x3f800000, 16},
                {WDT_FP32, false, 0x3f800001, 0x33800000, 0x3f800002, 16},
                {WDT_FP32, false, 0x7f7fffff, 0x3f800000, 0x7f7fffff, 16},
                {WDT_FP32, false, 0x7f7fffff, 0x7f7fffff, 0x7f800000, 20},
                {WDT_FP32, true, 0x00800000, 0x007fffff, 0x00000001, 0},
                {WDT_FP32, false, 0x7f800000, 0xff800000, 0x7fc00000, 1},
                {WDT_FP32, true, 0x7fc00003, 0xff800002, 0xffc00002, 1}};
            for (const auto& e : examples) {
              const Reference& ref = e.format == WDT_BF16 ? bf : fp;
              const FloatResult value = ref.evaluate(e.sub, Policy(), e.a, e.b);
              if (value.bits != e.result || value.flags != e.flags ||
                  !check_exact(ref, e.sub, Policy(), e.a, e.b))
                return false;
              // Assigned wrong bits/status must fail, before and after posting.
              for (bool delayed : {false, true}) {
                Pair s(e.format);
                if (!delayed)
                  dom(s, s.x[2], e.result ^ 1);
                post_relation(s, e.sub, Policy(), s.x[0], s.x[1], s.x[2],
                              &s.flags);
                dom(s, s.x[0], e.a);
                dom(s, s.x[1], e.b);
                if (delayed)
                  dom(s, s.x[2], e.result ^ 1);
                if (s.status() != SS_FAILED)
                  return false;
                Pair t(e.format);
                post_relation(t, e.sub, Policy(), t.x[0], t.x[1], t.x[2],
                              &t.flags);
                dom(t, t.x[0], e.a);
                dom(t, t.x[1], e.b);
                rel(t, t.flags, IRT_EQ, e.flags ^ 16);
                if (t.status() != SS_FAILED)
                  return false;
              }
            }
            return true;
          }
        };

        /// Total-order window of encodings
        struct Window {
          WordValue minimum, maximum;
        };
        /// The window of the encodings with sign bit \a sign, which may be
        /// zero, and magnitudes from \a low to \a high
        Window magnitudes(WordValue sign, WordValue low, WordValue high) {
          return sign != 0 ? Window{sign | high, sign | low}
                           : Window{low, high};
        }
        void bounds(Space& s, WordVar word, Window w) {
          float_bounds(s, word, w.minimum, w.maximum);
        }
        using Tuple = std::array<WordValue, 7>;

        class Network : public Space {
        public:
          WordVar x[5];
          IntVar flags[2];
          Network(const Reference& ref, bool subtract, const Policy& p,
                  int direction, int later = 0, bool holes = false,
                  bool exact_status = false) {
            for (auto& v : x)
              v = WordVar(*this, ref.type);
            for (auto& f : flags)
              f = IntVar(*this, 0, 31);
            const WordValue a =
                subtract ? ref.one + ref.normal + ref.normal / 2 : ref.one;
            const int ar = direction == 2 ? 1 : 8, br = direction == 2 ? 8 : 1;
            bounds(*this, x[0], {a - ar, a + ar});
            bounds(*this, x[1], {ref.one - br, ref.one + br});
            bounds(*this, x[2], {0, ref.maximum});
            if (holes)
              dom(*this, x[0], WordValue(0), (ref.sign * 2 - 1) ^ 2);
            if (direction == 1 || direction == 2)
              dom(*this, x[2], ref.one + ref.normal);
            if (direction == 3) {
              bounds(*this, x[3], {ref.one - 1, ref.one + 1});
              dom(*this, x[4],
                  subtract ? ref.one : ref.one + ref.normal + ref.normal / 2);
              post_relation(*this, subtract, p, x[2], x[3], x[4], &flags[1]);
            } else {
              dom(*this, x[3], 0);
              dom(*this, x[4], 0);
              rel(*this, flags[1], IRT_EQ, 0);
            }
            post_relation(*this, subtract, p, x[0], x[1], x[2], &flags[0]);
            if (exact_status)
              rel(*this, flags[0], IRT_EQ, 0);
            restrict_later(later, ref);
            WordVarArgs words(5);
            for (int i = 0; i < 5; ++i)
              words[i] = x[i];
            branch(*this, words, WORD_VAR_NONE(), WORD_VAL_LSB());
            branch(*this, IntVarArgs{flags[0], flags[1]}, INT_VAR_NONE(),
                   INT_VAL_MIN());
          }
          void restrict_later(int later, const Reference& ref) {
            if (later == 1)
              dom(*this, x[0], WordValue(1), ref.sign * 2 - 1);
            if (later == 2)
              bounds(*this, x[1], {ref.one, ref.one + 1});
            if (later == 3)
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

        std::set<Tuple> expected_network(const Reference& ref, bool subtract,
                                         const Policy& p, int direction,
                                         int later = 0, bool holes = false,
                                         bool exact_status = false) {
          std::set<Tuple> tuples;
          const WordValue center =
              subtract ? ref.one + ref.normal + ref.normal / 2 : ref.one;
          const int ar = direction == 2 ? 1 : 8, br = direction == 2 ? 8 : 1;
          for (WordValue a = center - ar; a <= center + ar; ++a)
            for (WordValue b = ref.one - br; b <= ref.one + br; ++b) {
              if ((holes && (a & 2)) || (later == 1 && !(a & 1)) ||
                  (later == 2 && b < ref.one))
                continue;
              const FloatResult z = ref.evaluate(subtract, p, a, b);
              if ((exact_status || later == 3) && z.flags != 0)
                continue;
              if ((direction == 1 || direction == 2) &&
                  z.bits != ref.one + ref.normal)
                continue;
              if (direction != 3) {
                tuples.insert({a, b, z.bits, 0, 0, WordValue(z.flags), 0});
              } else {
                for (WordValue c = ref.one - 1; c <= ref.one + 1; ++c) {
                  const FloatResult out = ref.evaluate(subtract, p, z.bits, c);
                  if (out.bits == (subtract
                                       ? ref.one
                                       : ref.one + ref.normal + ref.normal / 2))
                    tuples.insert({a, b, z.bits, c, out.bits,
                                   WordValue(z.flags), WordValue(out.flags)});
                }
              }
            }
          return tuples;
        }

        bool preserves(Network& s, const std::set<Tuple>& expected) {
          for (const auto& tuple : expected) {
            for (int i = 0; i < 5; ++i)
              if (!Gecode::Word::WordView(s.x[i]).in(tuple[i]))
                return false;
            if (!s.flags[0].in(static_cast<int>(tuple[5])) ||
                !s.flags[1].in(static_cast<int>(tuple[6])))
              return false;
          }
          return true;
        }
        std::set<Tuple> solutions(Network& s, unsigned int recompute = 8) {
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
          return found;
        }
        bool same_domains(Network& a, Network& b) {
          for (int i = 0; i < 5; ++i) {
            const WordVar x = a.x[i], y = b.x[i];
            if (x.lo() != y.lo() || x.hi() != y.hi() ||
                (x.bounded() &&
                 (x.minimum() != y.minimum() || x.maximum() != y.maximum())))
              return false;
          }
          return true;
        }

        class WideSymbolic : public Base {
        public:
          WideSymbolic() : Base("Word::Float::Arithmetic::Wide::Symbolic") {}
          bool run() override {
            for (WordDomainType format : {WDT_BF16, WDT_FP32}) {
              const Reference ref(format);
              for (bool sub : {false, true})
                for (int r = 0; r < 4; ++r)
                  for (int direction = 0; direction < 4; ++direction)
                    for (bool holes : {false, true}) {
                      const Policy p{static_cast<Policy::Rounding>(r)};
                      Network s(ref, sub, p, direction, 0, holes);
                      const auto expected =
                          expected_network(ref, sub, p, direction, 0, holes);
                      if (s.status() == SS_FAILED) {
                        if (!expected.empty())
                          return false;
                        continue;
                      }
                      if (!preserves(s, expected))
                        return false;
                      if (!holes) {
                        if (direction == 0 &&
                            s.x[2].maximum() >= ref.maximum)
                          return false;
                        if (direction == 1 || direction == 2) {
                          const int target = direction - 1;
                          const WordValue center =
                              target == 0 && sub
                                  ? ref.one + ref.normal + ref.normal / 2
                                  : ref.one;
                          if (Gecode::Word::WordView(s.x[target])
                                  .in(center - 8) &&
                              Gecode::Word::WordView(s.x[target])
                                  .in(center + 8))
                            return false;
                        }
                      }
                      if (solutions(s) != expected) {
                        olog << ind(2) << "wide network mismatch " << format << ' '
                                  << sub << ' ' << r << ' ' << direction
                                  << ' ' << holes << '\n';
                        return false;
                      }
                    }
              Network status_case(ref, false, Policy(), 0, 0, false, true);
              const auto exact =
                  expected_network(ref, false, Policy(), 0, 0, false, true);
              if (status_case.status() == SS_FAILED ||
                  !preserves(status_case, exact) ||
                  solutions(status_case) != exact)
                return false;
            }
            return true;
          }
        };

        class WideLifecycle : public Base {
          static bool delayed_add() {
            Pair original(WDT_FP32);
            for (int i = 0; i < 2; ++i)
              bounds(original, original.x[i], {0x3f800000, 0x3f800100});
            fp_add(original, original.x[0], original.x[1],
                   original.x[2], original.flags);
            if (original.status() == SS_FAILED)
              return false;
            for (unsigned int i = 1; i <= 70; ++i) {
              bounds(original, original.x[0],
                     {0x3f800000, 0x3f800100 - i});
              if (original.status() == SS_FAILED)
                return false;
            }
            std::unique_ptr<Pair> clone(static_cast<Pair*>(original.clone()));
            const WordValue unchanged = original.x[2].maximum();
            bounds(*clone, clone->x[1], {0x3f800000, 0x3f800001});
            if (clone->status() == SS_FAILED ||
                clone->x[2].maximum() != 0x4000005e ||
                original.x[2].maximum() != unchanged)
              return false;
            bounds(original, original.x[1], {0x3f800000, 0x3f800001});
            Pair fresh(WDT_FP32);
            bounds(fresh, fresh.x[0], {0x3f800000, 0x3f8000ba});
            bounds(fresh, fresh.x[1], {0x3f800000, 0x3f800001});
            fp_add(fresh, fresh.x[0], fresh.x[1], fresh.x[2],
                   fresh.flags);
            if (original.status() == SS_FAILED || fresh.status() == SS_FAILED)
              return false;
            return same_domains(original, *clone) &&
                   same_domains(original, fresh) &&
                   original.x[2].maximum() == 0x4000005e &&
                   !original.x[0].assigned() &&
                   !original.x[1].assigned() &&
                   closed_relation(original, false, Policy());
          }

        public:
          WideLifecycle() : Base("Word::Float::Arithmetic::Wide::Lifecycle") {}
          bool run() override {
            if (!delayed_add())
              return false;
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              for (bool sub : {false, true})
                for (int r = 0; r < 4; ++r)
                  for (int later = 1; later <= 3; ++later) {
                    const Policy p{static_cast<Policy::Rounding>(r)};
                    Network original(ref, sub, p, 3);
                    if (original.status() == SS_FAILED)
                      return false;
                    std::unique_ptr<Network> clone(
                        static_cast<Network*>(original.clone()));
                    clone->restrict_later(later, ref);
                    Network fresh(ref, sub, p, 3, later),
                        unchanged(ref, sub, p, 3);
                    const auto expected =
                        expected_network(ref, sub, p, 3, later);
                    const auto clone_status = clone->status(),
                               fresh_status = fresh.status();
                    if (unchanged.status() == SS_FAILED ||
                        !same_domains(original, unchanged))
                      return false;
                    if (clone_status == SS_FAILED ||
                        fresh_status == SS_FAILED) {
                      if (!expected.empty() || clone_status != fresh_status)
                        return false;
                      continue;
                    }
                    if (!same_domains(*clone, fresh))
                      return false;
                    if (!preserves(*clone, expected) ||
                        solutions(*clone, 1) != expected ||
                        solutions(fresh, 16) != expected ||
                        solutions(original) !=
                            expected_network(ref, sub, p, 3))
                      return false;
                  }
              Pair cancellation(f);
              cancellation.x[1] = cancellation.x[0];
              bounds(cancellation, cancellation.x[0],
                     {ref.one, ref.one + 25 * ref.normal});
              dom(cancellation, cancellation.x[2], ref.one);
              post_relation(cancellation, true, Policy(), cancellation.x[0],
                            cancellation.x[1], cancellation.x[2],
                            &cancellation.flags);
              if (cancellation.status() != SS_FAILED)
                return false;
              Pair doubled(f);
              doubled.x[1] = doubled.x[0];
              bounds(doubled, doubled.x[0],
                     {ref.one - ref.normal, ref.one + 25 * ref.normal});
              bounds(doubled, doubled.x[2],
                     {ref.one + ref.normal - 2, ref.one + ref.normal + 2});
              post_relation(doubled, false, Policy(), doubled.x[0],
                            doubled.x[1], doubled.x[2], &doubled.flags);
              if (doubled.status() == SS_FAILED ||
                  !closed_relation(doubled, false, Policy()))
                return false;
              // A broad result alias crosses the spacing needed to absorb 1.
              // Its class projection must finish and be closed on reposting.
              Pair absorbed(f);
              absorbed.x[2] = absorbed.x[0];
              bounds(absorbed, absorbed.x[0],
                     {ref.one, ref.one + 25 * ref.normal});
              dom(absorbed, absorbed.x[1], ref.one);
              fp_add(absorbed, absorbed.x[0], absorbed.x[1],
                     absorbed.x[2], absorbed.flags);
              if (absorbed.status() == SS_FAILED ||
                  absorbed.x[0].minimum() !=
                      ref.one + (ref.fraction + 1) * ref.normal ||
                  !closed_relation(absorbed, false, Policy()))
                return false;
              // Aliases include numerical cancellation, zero signs and NaN
              // quieting.
              for (bool sub : {false, true})
                for (bool result_alias : {false, true})
                  for (WordValue value : {WordValue(0), ref.sign, ref.one,
                                          ref.infinity, ref.infinity + 1}) {
                    Pair s(f);
                    s.x[1] = s.x[0];
                    if (result_alias)
                      s.x[2] = s.x[0];
                    post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2],
                                  &s.flags);
                    dom(s, s.x[0], value);
                    const auto expected =
                        ref.evaluate(sub, Policy(), value, value);
                    const bool allowed =
                        !result_alias || expected.bits == value;
                    if ((s.status() != SS_FAILED) != allowed)
                      return false;
                    if (allowed && (s.x[2].val() != expected.bits ||
                                    s.flags.val() != expected.flags))
                      return false;
                  }
              // Result aliases with independently varying second operand retain
              // their actual identity. Exhaust the four original bit-cube
              // assignments.
              Pair alias(f);
              alias.x[2] = alias.x[0];
              dom(alias, alias.x[0], ref.one, ref.one + 1);
              dom(alias, alias.x[1], WordValue(0), WordValue(1));
              fp_add(alias, alias.x[0], alias.x[1], alias.x[2],
                     alias.flags);
              branch(alias, WordVarArgs{alias.x[0], alias.x[1]},
                     WORD_VAL_LSB());
              DFS<Pair> search(&alias);
              unsigned int count = 0;
              while (std::unique_ptr<Pair> solution{search.next()}) {
                const auto expected =
                    ref.evaluate(false, Policy(), solution->x[0].val(),
                                 solution->x[1].val());
                if (solution->x[2].val() != expected.bits ||
                    solution->flags.val() != expected.flags)
                  return false;
                ++count;
              }
              if (count != 4)
                return false;
            }
            return true;
          }
        };

        class WideCancellation : public Base {
        public:
          WideCancellation()
              : Base("Word::Float::Arithmetic::Wide::Cancellation") {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              for (bool sub : {false, true})
                for (bool broad : {false, true}) {
                  Pair s(f);
                  bounds(s, s.x[0], {ref.one, ref.one + 13 * ref.normal});
                  const WordValue bsign = sub ? 0 : ref.sign;
                  bounds(s, s.x[1],
                         magnitudes(bsign, ref.one, ref.one + 13 * ref.normal));
                  const WordValue z = ref.one + ref.normal - 1;
                  bounds(s, s.x[2],
                         {z - (broad ? 1 : 0), z + (broad ? 1 : 0)});
                  post_relation(s, sub, Policy{Policy::RUP},
                                s.x[0], s.x[1], s.x[2], &s.flags);
                  if (s.status() == SS_FAILED)
                    return false;
                  if (!broad &&
                      s.x[0].maximum() >= ref.one + 13 * ref.normal)
                    return false;
                  if (!closed_relation(s, sub,
                                       Policy{Policy::RUP}))
                    return false;
                  // Exact witness: a=2*z and b=-z (or +z under subtraction).
                  dom(s, s.x[0], z + ref.normal);
                  dom(s, s.x[1], bsign + z);
                  dom(s, s.x[2], z);
                  if (s.status() == SS_FAILED || s.flags.val() != 0)
                    return false;
                }
            }
            return true;
          }
        };

        /// The three policy settings beside the default that once disabled
        /// the cancellation bound
        Policy cancellation_policy(int setting) {
          Policy p;
          if (setting == 0)
            p.overflow = Policy::SaturateFinite;
          else if (setting == 1)
            p.output_subnormals = Policy::SignedZero;
          else
            p.input_subnormals = Policy::SignedZero;
          return p;
        }

        // A negative operand, a positive operand and a result just above 3
        // with low significand bits set. Where both operand spacings exceed
        // the result's rounding cell no pair is supported, and interval
        // projection alone retreats one encoding per propagation there. One
        // status() must finish in a few propagations under every policy.
        class WideCancellationBudget : public Base {
          // Operands a in [-(8 + extra ulps), -1] and b in [1, 11 + extra
          // ulps]; z holds the first two codes above 3. The old creep moved
          // through the extra encodings of the binade [8,16).
          static bool check(WordDomainType f, bool sub, int setting,
                            WordValue extra) {
            const Reference ref(f);
            const Policy p = cancellation_policy(setting);
            const WordValue eight = ref.one + 3 * ref.normal;
            const WordValue three = ref.one + ref.normal + ref.normal / 2;
            const WordValue eleven = eight + 3 * ref.normal / 8;
            Pair s(f);
            bounds(s, s.x[0], {ref.sign | (eight + extra), ref.sign | ref.one});
            const Window b = {ref.one, eleven + extra};
            bounds(s, s.x[1],
                   sub ? magnitudes(ref.sign, b.minimum, b.maximum) : b);
            bounds(s, s.x[2], {three + 1, three + 2});
            post_relation(s, sub, p, s.x[0], s.x[1], s.x[2], &s.flags);
            StatusStatistics statistics;
            if (s.status(statistics) == SS_FAILED)
              return false;
            if (statistics.propagate > 8) {
              if (opt.log)
                olog << ind(2) << "cancellation budget " << f << ' ' << sub << ' '
                     << setting << ": " << statistics.propagate
                     << " propagations\n";
              return false;
            }
            // Exact witnesses: -1 + (4 + 2u) and -(8 - 2u) + 11 both give
            // 3 + 2u, where u is the spacing of [2,4).
            const WordValue witnesses[][2] = {
                {ref.sign | ref.one, ref.one + 2 * ref.normal + 1},
                {ref.sign | (eight - 1), eleven}};
            for (const auto& w : witnesses) {
              const WordValue second = sub ? w[1] | ref.sign : w[1];
              if (ref.evaluate(sub, p, w[0], second).bits != three + 2)
                return false;
              if (!Gecode::Word::WordView(s.x[0]).in(w[0]) ||
                  !Gecode::Word::WordView(s.x[1]).in(second) ||
                  !Gecode::Word::WordView(s.x[2]).in(three + 2))
                return false;
            }
            return true;
          }

        public:
          WideCancellationBudget()
              : Base("Word::Float::Arithmetic::Wide::CancellationBudget") {}
          bool run() override {
            for (int setting = 0; setting < 3; ++setting)
              for (bool sub : {false, true})
                if (!check(WDT_BF16, sub, setting, 240) ||
                    !check(WDT_FP32, sub, setting, 1000))
                  return false;
            return true;
          }
        };

        // Under each policy setting that once disabled the cancellation
        // bound, small BF16 windows keep every tuple the reference supports.
        class WideCancellationPolicies : public Base {
          struct Case {
            Window a, b, z;
          };
          static bool check(const Reference& ref, bool sub, const Policy& p,
                            const Case& c) {
            Pair s(ref.type);
            bounds(s, s.x[0], c.a);
            bounds(s, s.x[1], c.b);
            bounds(s, s.x[2], c.z);
            post_relation(s, sub, p, s.x[0], s.x[1], s.x[2], &s.flags);
            const bool failed = s.status() == SS_FAILED;
            for (WordValue a : float_order_range(16, c.a.minimum, c.a.maximum))
              for (WordValue b :
                   float_order_range(16, c.b.minimum, c.b.maximum)) {
                const FloatResult e = ref.evaluate(sub, p, a, b);
                if (!float_order_contains(16, c.z.minimum, c.z.maximum,
                                          e.bits))
                  continue;
                if (failed || !Gecode::Word::WordView(s.x[0]).in(a) ||
                    !Gecode::Word::WordView(s.x[1]).in(b) ||
                    !Gecode::Word::WordView(s.x[2]).in(e.bits) ||
                    !s.flags.in(e.flags)) {
                  if (opt.log)
                    olog << ind(2) << "cancellation policy removed " << std::hex << a
                         << ' ' << b << ' ' << e.bits << std::dec << " sub "
                         << sub << " rounding " << p.rounding << '\n';
                  return false;
                }
              }
            return true;
          }

        public:
          WideCancellationPolicies()
              : Base("Word::Float::Arithmetic::Wide::CancellationPolicies") {}
          bool run() override {
            const Reference ref(WDT_BF16);
            const WordValue sign = ref.sign, one = ref.one,
                            normal = ref.normal, maximum = ref.maximum;
            const WordValue three = one + normal + normal / 2;
            const Case cases[] = {
                // Cancellation across the binade [8,16) into a narrow z.
                {{sign | 0x4118, sign | 0x40f8}, {0x4128, 0x4140},
                 {three + 1, three + 2}},
                // Both zeros and subnormals against the normal boundary.
                {{sign | 0x10, 0x10}, {normal - 12, normal + 12},
                 {normal - 4, normal + 8}},
                // Subnormal differences that a flushed output cannot hold.
                {{sign | 6, 20}, {sign | 20, 6}, {1, 12}},
                // Near overflow, cancelling into multiples of the top spacing.
                {{maximum - 24, maximum}, {sign | maximum, sign | (maximum - 24)},
                 {0x7980, 0x7d40}},
                // Near overflow, with saturation at the maximum finite.
                {{maximum - 12, maximum}, {maximum - 12, maximum},
                 {maximum - 2, maximum}},
                // Cancellation around one into small results of both signs.
                {{one - 16, one + 16}, {sign | (one + 16), sign | (one - 16)},
                 {0x3a00, 0x3c40}},
                // A narrow negative result from mixed-sign operands.
                {{sign | (one + 3 * normal + 30), sign | (one + 2 * normal)},
                 {one + 2 * normal - 8, one + 3 * normal + 16},
                 {sign | (three + 3), sign | (three + 1)}}};
            for (int setting = 0; setting < 4; ++setting)
              for (int r = 0; r < 4; ++r) {
                Policy p = setting < 3 ? cancellation_policy(setting)
                                       : Policy{Policy::RNE, Policy::SignedZero,
                                                Policy::SignedZero,
                                                Policy::SaturateFinite};
                p.rounding = static_cast<Policy::Rounding>(r);
                for (bool sub : {false, true})
                  for (const Case& c : cases) {
                    const Case operands =
                        sub ? Case{c.a, {c.b.maximum ^ sign, c.b.minimum ^ sign},
                                   c.z}
                            : c;
                    if (!check(ref, sub, p, operands))
                      return false;
                  }
              }
            return true;
          }
        };

        // Rounds that do not converge publish what they reached. BF16 RDN
        // with SaturateFinite: a = [0xf4ee..0xf4cb], b = [0x74ca..0x7519], z
        // = [0x73b3..0x73b5]. Without known bits on a, b narrows to
        // [0x74f8..0x750d]; known bits 0x85/0x80 on a must narrow b no less,
        // while every supported tuple stays.
        class WideCancellationRounds : public Base {
          static bool check(const Reference& ref, const Policy& p, bool cube,
                            Window& narrowed) {
            Pair s(ref.type);
            bounds(s, s.x[0], {0xf4ee, 0xf4cb});
            if (cube)
              dom(s, s.x[0], WordValue(0x80), WordValue(0xfffa));
            bounds(s, s.x[1], {0x74ca, 0x7519});
            bounds(s, s.x[2], {0x73b3, 0x73b5});
            fp_add(s, s.x[0], s.x[1], s.x[2], s.flags, p);
            if (s.status() == SS_FAILED)
              return false;
            for (WordValue a : float_order_range(16, 0xf4ee, 0xf4cb))
              for (WordValue b : float_order_range(16, 0x74ca, 0x7519)) {
                if (cube && (a & 0x85) != 0x80)
                  continue;
                const FloatResult e = ref.evaluate(false, p, a, b);
                if (!float_order_contains(16, 0x73b3, 0x73b5, e.bits))
                  continue;
                if (!Gecode::Word::WordView(s.x[0]).in(a) ||
                    !Gecode::Word::WordView(s.x[1]).in(b) ||
                    !Gecode::Word::WordView(s.x[2]).in(e.bits) ||
                    !s.flags.in(e.flags))
                  return false;
              }
            narrowed = {s.x[1].minimum(), s.x[1].maximum()};
            return true;
          }

        public:
          WideCancellationRounds()
              : Base("Word::Float::Arithmetic::Wide::CancellationRounds") {}
          bool run() override {
            const Reference ref(WDT_BF16);
            Policy p;
            p.rounding = Policy::RDN;
            p.overflow = Policy::SaturateFinite;
            Window plain{0, 0}, known{0, 0};
            if (!check(ref, p, false, plain) || !check(ref, p, true, known))
              return false;
            if (plain.minimum != 0x74f8 || plain.maximum != 0x750d)
              return false;
            if (known.minimum < plain.minimum || known.maximum > plain.maximum) {
              if (opt.log)
                olog << ind(2) << "cancellation rounds: b " << std::hex << known.minimum
                     << ".." << known.maximum << std::dec << '\n';
              return false;
            }
            return true;
          }
        };

        // One operand of one value and known bits on the result: the
        // supports of the free operand form a lattice that the interval
        // projections crossed one step per propagation. One status() takes
        // at most 8 propagations and keeps every supported tuple. On small
        // BF16 windows the free operand ends at the hull of its supports,
        // found by enumeration; on FP32 the domains are pinned, with the
        // endpoints of the free operand as supported witnesses.
        class WideSumLattice : public Base {
          struct Case {
            WordDomainType format;
            bool subtract;
            /// Position of the operand of one value
            int fixed_at;
            Policy policy;
            WordValue fixed;
            Window free, z;
            WordValue free_mask, free_known, z_mask, z_known;
          };
          static WordValue all(const Case& c) {
            return (WordValue(1) << float_word_width(c.format)) - 1;
          }
          static void post(Pair& s, const Case& c) {
            const int free_at = 1 - c.fixed_at;
            dom(s, s.x[c.fixed_at], c.fixed);
            bounds(s, s.x[free_at], c.free);
            dom(s, s.x[free_at], c.free_known,
                c.free_known | (~c.free_mask & all(c)));
            bounds(s, s.x[2], c.z);
            dom(s, s.x[2], c.z_known, c.z_known | (~c.z_mask & all(c)));
            post_relation(s, c.subtract, c.policy, s.x[0], s.x[1], s.x[2],
                          &s.flags);
          }
          static FloatResult result(const Reference& ref, const Case& c,
                                 WordValue code) {
            return c.fixed_at == 0
                       ? ref.evaluate(c.subtract, c.policy, c.fixed, code)
                       : ref.evaluate(c.subtract, c.policy, code, c.fixed);
          }
          static bool supports(const Case& c, const FloatResult& e) {
            return float_order_contains(float_word_width(c.format),
                                        c.z.minimum, c.z.maximum, e.bits) &&
                   (e.bits & c.z_mask) == c.z_known;
          }
          static bool kept(Pair& s, const Case& c, WordValue code,
                           const FloatResult& e) {
            return Gecode::Word::WordView(s.x[1 - c.fixed_at]).in(code) &&
                   Gecode::Word::WordView(s.x[2]).in(e.bits) &&
                   s.flags.in(e.flags);
          }
          static bool budget(const Case& c, const StatusStatistics& st) {
            if (st.propagate <= 8)
              return true;
            if (opt.log)
              olog << ind(2) << "sum lattice " << float_word_width(c.format) << ' '
                   << std::hex << c.fixed << ' ' << c.free.minimum << ".."
                   << c.free.maximum << std::dec << ": " << st.propagate
                   << " propagations\n";
            return false;
          }
          // Every supported code of the free operand stays, and its domain
          // is the hull of them.
          static bool exhaustive(const Case& c) {
            const Reference ref(c.format);
            const unsigned int width = float_word_width(c.format);
            Pair s(c.format);
            post(s, c);
            StatusStatistics statistics;
            const bool failed = s.status(statistics) == SS_FAILED;
            if (!budget(c, statistics))
              return false;
            std::set<WordValue> supported;
            for (WordValue code :
                 float_order_range(width, c.free.minimum, c.free.maximum)) {
              if ((code & c.free_mask) != c.free_known)
                continue;
              const FloatResult e = result(ref, c, code);
              if (!supports(c, e))
                continue;
              supported.insert(code);
              if (failed || !kept(s, c, code, e))
                return false;
            }
            if (failed || supported.empty())
              return failed == supported.empty();
            const auto hull = float_order_extremes(width, supported);
            const WordVar x = s.x[1 - c.fixed_at];
            if (x.minimum() != hull.first || x.maximum() != hull.second) {
              if (opt.log)
                olog << ind(2) << "sum lattice hull " << std::hex << x.minimum() << ".."
                     << x.maximum() << " expected " << hull.first << ".."
                     << hull.second << std::dec << '\n';
              return false;
            }
            return true;
          }
          // The domains end at free and z, and the endpoints of free are
          // supported.
          static bool pinned(const Case& c, Window free, Window z) {
            const Reference ref(c.format);
            Pair s(c.format);
            post(s, c);
            StatusStatistics statistics;
            if (s.status(statistics) == SS_FAILED || !budget(c, statistics))
              return false;
            const WordVar x = s.x[1 - c.fixed_at];
            if (x.minimum() != free.minimum || x.maximum() != free.maximum ||
                s.x[2].minimum() != z.minimum || s.x[2].maximum() != z.maximum)
              return false;
            for (WordValue code : {free.minimum, free.maximum}) {
              const FloatResult e = result(ref, c, code);
              if (!supports(c, e) || !kept(s, c, code, e))
                return false;
            }
            return true;
          }
          // A BF16 case: the fixed operand of similar or smaller size, and
          // known low bits on the result that a support of the free operand
          // meets, mostly
          static bool random_case(std::mt19937_64& random, int i, Case& c) {
            const Reference ref(WDT_BF16);
            c.format = WDT_BF16;
            c.subtract = i % 2 != 0;
            c.fixed_at = (i / 2) % 2;
            c.policy = Policy();
            c.policy.rounding = static_cast<Policy::Rounding>((i / 4) % 4);
            if ((i / 16) % 3 == 1)
              c.policy.input_subnormals = Policy::SignedZero;
            else if ((i / 16) % 3 == 2)
              c.policy.output_subnormals = Policy::SignedZero;
            const WordValue least =
                c.policy.input_subnormals == Policy::SignedZero ? ref.normal
                                                                : 1;
            const WordValue span = 20 + random() % 600;
            WordValue low = least + random() % (ref.maximum - span - least);
            // Some windows start a binade, where sums cross into the one
            // below
            if (i % 3 == 1 && low >= ref.normal)
              low -= low % ref.normal;
            const WordValue sign = random() % 2 ? ref.sign : 0;
            c.free = magnitudes(sign, low, low + span);
            c.free_mask = i % 3 == 0 ? random() & 7 : 0;
            c.free_known = random() & c.free_mask;
            // The fixed operand from 9 binades below the free one to 3
            // above
            const int exponent = static_cast<int>(low / ref.normal) +
                                 static_cast<int>(random() % 13) - 9;
            const WordValue magnitude =
                std::min<WordValue>(ref.maximum,
                                    std::max(0, exponent) * ref.normal +
                                        random() % ref.normal);
            c.fixed = (random() % 2 ? ref.sign : 0) | magnitude;
            const WordValue witness = low + random() % (span + 1);
            const FloatResult e =
                result(ref, c, (sign | witness) & ~c.free_mask |
                                   c.free_known);
            const WordValue result_least =
                c.policy.output_subnormals == Policy::SignedZero ? ref.normal
                                                                 : 1;
            const WordValue m = e.bits & (ref.sign - 1);
            if (m < result_least || m >= ref.maximum)
              return false;
            const WordValue below = random() % 300, above = random() % 300;
            const WordValue z_low = m > result_least + below
                                        ? m - below
                                        : result_least;
            const WordValue z_high = std::min(ref.maximum - 1, m + above);
            c.z = magnitudes(e.bits & ref.sign, z_low, z_high);
            c.z_mask = (WordValue(2) << (random() % 5)) - 1;
            c.z_known = (random() % 8 == 0 ? random() : e.bits) & c.z_mask;
            return true;
          }

        public:
          WideSumLattice() : Base("Word::Float::Arithmetic::Wide::SumLattice") {}
          bool run() override {
            std::mt19937_64 random(11);
            int cases = 0;
            for (int i = 0; cases < 240; ++i) {
              Case c;
              if (!random_case(random, i, c))
                continue;
              ++cases;
              if (!exhaustive(c))
                return false;
            }
            // Every sum ties: x in [1, 1 + 40u] with u = 2^-7, y = -83*2^-9,
            // so x + y = (429 + 4t)*2^-9 lies halfway between codes of the
            // binade [1/2, 1) below that of x, and rounds to an even code:
            // x = 1 + tu supports z with known bit 0 clear for t <= 20.
            Policy nearest;
            const Case ties[] = {
                {WDT_BF16, false, 1, nearest, 0xbe26, {0x3f80, 0x3fa8},
                 {0x3f40, 0x3f7f}, 0, 0, 1, 0},
                {WDT_BF16, true, 1, nearest, 0x3e26, {0x3f80, 0x3fa8},
                 {0x3f40, 0x3f7f}, 0, 0, 1, 0}};
            for (const Case& c : ties)
              if (!exhaustive(c))
                return false;
            // FP32 add, RDN, known bits on a and on the result, b fixed
            Policy add;
            add.rounding = Policy::RDN;
            add.input_subnormals = Policy::SignedZero;
            add.overflow = Policy::SaturateFinite;
            add.zeros = Policy::Positive;
            const Case sum{WDT_FP32, false, 1, add, 0xc1e8b5ee,
                           {0xff7fffff, 0x7f7fffff}, {0xd00f6b9a, 0xa16f581a},
                           0x20043c2, 0x2000200, 0x10008050, 0x50};
            if (!pinned(sum, {0xcb7f3e3d, 0x3f76b23d},
                        {0xcb7f3e5b, 0xc1e1005d}))
              return false;
            // FP32 subtract, RDN, a fixed, known bits on the result
            Policy difference;
            difference.rounding = Policy::RDN;
            difference.output_subnormals = Policy::SignedZero;
            difference.overflow = Policy::SaturateFinite;
            difference.zeros = Policy::Positive;
            difference.nan = Policy::OperandFirst;
            const Case subtract{WDT_FP32, true, 0, difference, 0xfecb5ae2,
                                {0xff7fffff, 0x7f7fffff},
                                {0xff7fffff, 0xe8ed868d},
                                0, 0, 0xe00, 0xa00};
            return pinned(subtract, {0xfecb1add, 0x7f1a4e8e},
                          {0xff7ffbff, 0xfa000a00});
          }
        };

        // Sums with both operands free and known bits, and a subtraction
        // whose subtrahend's range holds zero: rounding each bound to the
        // next code with the known bits moves them one lattice step per
        // propagation, up to 18218 propagations (more than 20000 for the
        // last). One status() ends within 8 executions at
        // a fixpoint whose moved bounds are witnesses, whose results the
        // reference confirms. One case per rounding direction; the RDN and
        // RUP ones cancel, with operands of opposite signs in the same or
        // neighbouring binades.
        class WideSumReplays : public Base {
          static FloatRegression sum(const char* name, bool subtract,
                                     Policy p, FloatWord a, FloatWord b,
                                     FloatWord z, std::optional<WordValue> r,
                                     std::vector<std::array<WordValue, 4>> w) {
            return {name,
                    {subtract ? FloatOperation::Subtract : FloatOperation::Add,
                     WDT_FP32, p, false, {a, b, {0, 0, 0, 0}, z}},
                    {std::nullopt, std::nullopt, std::nullopt, r},
                    std::move(w)};
          }

        public:
          WideSumReplays()
              : Base("Word::Float::Arithmetic::Wide::SumReplays") {}
          bool run() override {
            const FloatRegression cases[] = {
                sum("RNE add, fixed result, low addend bits known", false,
                    float_dump_policy(0, 0, 0, 0, 0, 1, 0),
                    {0xc7ac4166, 0xbdb54308, 0, 0},
                    {0x3d7b2694, 0x42c729ba, 0x7, 0x1},
                    {0xc2fbf9ce, 0xc2fbf9ce, 0, 0},
                    0xc2fbf9ce,
                    {{0xc33dfce5, 0x427ffff9, 0, 0xc2fbf9ce},
                     {0xc2fc1933, 0x3d7b2801, 0, 0xc2fbf9ce},
                     {0xc2fc1933, 0x3d7b2699, 0, 0xc2fbf9ce}}),
                sum("RTZ subtract, fixed result, low subtrahend bits known", true,
                    float_dump_policy(1, 0, 0, 0, 0, 0, 0),
                    {0xd14116cc, 0xbcf97bac, 0, 0},
                    {0xee93f3bd, 0x7f7fffff, 0x7, 0x6},
                    {0xc017679c, 0xc017679c, 0, 0},
                    0xc017679c,
                    {{0xc17ffffd, 0xc15a2616, 0, 0xc017679c},
                     {0xbcf97f00, 0x4015749e, 0, 0xc017679c}}),
                sum("RDN subtract, cancelling, narrow result window", true,
                    float_dump_policy(2, 0, 0, 0, 0, 0, 0),
                    {0xb7f22c2a, 0x7f7fffff, 0x80000020, 0},
                    {0x864cdd, 0x7f7fffff, 0x1, 0},
                    {0x442deb15, 0x442deb36, 0x70, 0x10},
                    std::nullopt,
                    {{0x442deb16, 0x20000000, 0, 0x442deb15},
                     {0x45ffffdf, 0x45ea427c, 0, 0x442deb18},
                     {0x442deb18, 0x864cde, 0, 0x442deb17},
                     {0x44800000, 0x43a429d6, 0, 0x442deb15},
                     {0x44800000, 0x43a429c2, 0, 0x442deb1f}}),
                sum("RUP subtract, cancelling, narrow result window", true,
                    float_dump_policy(3, 0, 0, 0, 0, 0, 0),
                    {0x2d480c4b, 0x2eef5886, 0x7, 0},
                    {0xff7fffff, 0x7f7fffff, 0x7, 0x2},
                    {0x2dca3b1a, 0x2dca3b1c, 0, 0},
                    std::nullopt,
                    {{0x2d8a3b20, 0xacffffea, 0, 0x2dca3b1b},
                     {0x2eb28ec0, 0x2e7ffff2, 0, 0x2dca3b1c},
                     {0x2e000000, 0x2cd7139a, 0, 0x2dca3b1a},
                     {0x2e000000, 0x2cd71392, 0, 0x2dca3b1c}}),
                // The subtrahend's zero gives the minuend as the result, the
                // pass that settles the minuend against the result directly
                sum("RNE subtract with a zero subtrahend", true,
                    float_dump_policy(0, 0, 0, 0, 0, 0, 0),
                    {0xff7fffff, 0x7f7fffff, 0x1, 0},
                    {0x85fdd97e, 0x13b3b78d, 0, 0},
                    {0xff7fffff, 0x7f7fffff, 0x3, 0x1},
                    std::nullopt,
                    {{0x9ffffffc, 0x13a00000, 0, 0x9ffffffd},
                     {0x1ffffffe, 0x13a00000, 0, 0x1ffffffd},
                     {0xa000000, 0x85fdd97e, 0, 0xa007eed},
                     {0, 0x13b3b78d, 0, 0x93b3b78d}})};
            // Every case runs, so that a log names each one that fails
            const Reference ref(WDT_FP32);
            bool passed = true;
            for (const FloatRegression& c : cases) {
              passed &= check_float_regression(c, 8);
              const bool subtract =
                  c.posting.operation == FloatOperation::Subtract;
              for (const auto& w : c.witnesses)
                passed &= ref.evaluate(subtract, c.posting.policy, w[0], w[1]).bits == w[3];
            }
            return passed;
          }
        };

        // Sums whose operands are one variable, fp_add(x, x, r). x + x is
        // 2x, exact apart from overflow and the subnormal policies, so the
        // result's code is a monotone function of x's code in each piece
        // (project_double). Interval projections move a bound one code per
        // execution here, 7525 to 49808 executions. One status() of at most
        // 8 executions ends at a fixpoint whose moved
        // bounds are witnesses, whose results the reference confirms, or, for
        // the last posting, whose result's known bits no doubled x meets,
        // fails.
        class WideDoubledSums : public Base {
          static FloatRegression doubled(const char* name, Policy p,
                                         bool status, FloatWord x, FloatWord z,
                                         std::vector<std::array<WordValue, 4>> w,
                                         bool fails = false) {
            return {name,
                    {FloatOperation::Add, WDT_FP32, p, status,
                     {x, x, {0, 0, 0, 0}, z}, true},
                    {},
                    std::move(w),
                    fails};
          }

        public:
          WideDoubledSums()
              : Base("Word::Float::Arithmetic::Wide::DoubledSums") {}
          bool run() override {
            const FloatRegression cases[] = {
                doubled("RTZ add x + x",
                        float_dump_policy(1, 0, 0, 0, 0, 0, 0), false,
                        {0x6fb8bc, 0xfa842ce, 0, 0},
                        {0x800003e8, 0x2000035, 0x7, 0x7},
                        {{0x800007, 0x800007, 0, 0x1000007},
                         {0x180002f, 0x180002f, 0, 0x200002f}}),
                doubled("RTZ add x + x with flags",
                        float_dump_policy(1, 0, 1, 1, 0, 1, 0), true,
                        {0x804eb257, 0x6c40a82, 0, 0},
                        {0x83ce29be, 0x47716a7, 0x7, 0x3},
                        {{0x800003, 0x800003, 0, 0x1000003},
                         {0x3f716a3, 0x3f716a3, 0, 0x47716a3}}),
                doubled("RNE add x + x without support",
                        float_dump_policy(0, 0, 0, 0, 0, 0, 0), false,
                        {0xff800000, 0x7f800000, 0xf, 0xe},
                        {0x223ee1cd, 0x52e2263c, 0x80030c04, 0x30400}, {},
                        true)};
            // Every case runs, so that a log names each one that fails
            const Reference ref(WDT_FP32);
            bool passed = true;
            for (const FloatRegression& c : cases) {
              passed &= check_float_regression(c, 8);
              for (const auto& w : c.witnesses)
                passed &= ref.evaluate(false, c.posting.policy, w[0], w[1]).bits == w[3];
            }
            return passed;
          }
        };

        // sum_term_extreme, the bit search for both operands of a sum free,
        // against enumeration on terms of BF16 size and smaller.
        class WideSumTerms : public Base {
          using Integer = Gecode::Word::Arithmetic::Global::Integer;
          using SumTerm = Gecode::Word::Float::SumTerm;
          using SumTarget = Gecode::Word::Float::SumTarget;
          using Rounding = Gecode::Word::Float::MagnitudeRounding;
          using Direction = Gecode::Word::Float::Direction;
          struct Case {
            SumTerm x, y;
            bool has_y;
            long long fixed;
            unsigned int scale;
            long long low, high;
            unsigned int b;
            std::uint64_t mask, value;
            Rounding rounding;
          };
          static long long rounded(long long m, unsigned int b, Rounding r) {
            const long long unit = 1LL << b, q = m >> b, rest = m & (unit - 1);
            if (r == Rounding::Down || rest == 0)
              return q;
            if (r == Rounding::Up)
              return q + 1;
            const long long half = unit / 2;
            return rest > half || (rest == half && (q & 1)) ? q + 1 : q;
          }
          static bool meets(const Case& c, long long m) {
            if (m < c.low || m > c.high)
              return false;
            const long long q = rounded(m, c.b, c.rounding);
            return (static_cast<std::uint64_t>(q) & c.mask) == c.value;
          }
          static long long term(const SumTerm& t, std::uint64_t v) {
            const long long m = static_cast<long long>(v) << t.a;
            return t.negative ? -m : m;
          }
          static bool supported(const Case& c, std::uint64_t x) {
            if (!c.has_y)
              return meets(c, term(c.x, x) + c.fixed);
            for (std::uint64_t y = c.y.from; y <= c.y.to; ++y)
              if ((y & c.y.mask) == c.y.value &&
                  meets(c, term(c.x, x) + term(c.y, y) + c.fixed))
                return true;
            return false;
          }
          static bool check(const Case& c) {
            bool any = false;
            std::uint64_t first = 0, last = 0;
            for (std::uint64_t x = c.x.from; x <= c.x.to; ++x)
              if ((x & c.x.mask) == c.x.value && supported(c, x)) {
                if (!any)
                  first = x;
                last = x;
                any = true;
              }
            const SumTarget target{Integer(static_cast<std::uint64_t>(c.low)),
                                   Integer(static_cast<std::uint64_t>(c.high)),
                                   c.b, c.mask, c.value, c.rounding};
            for (Direction d : {Direction::Up, Direction::Down}) {
              std::uint64_t x = 0;
              const bool found = Gecode::Word::Float::sum_term_extreme(
                  c.x, c.has_y ? &c.y : nullptr, Integer::signed_value(c.fixed),
                  c.scale, target, d, x);
              const std::uint64_t expected = d == Direction::Up ? first : last;
              if (found == any && (!found || x == expected))
                continue;
              if (opt.log)
                olog << ind(2) << "sum terms: x " << c.x.from << ".." << c.x.to << " k"
                     << c.x.mask << '/' << c.x.value << " a" << c.x.a
                     << (c.x.negative ? "-" : "+") << " y " << c.y.from
                     << ".." << c.y.to << " k" << c.y.mask << '/' << c.y.value
                     << " a" << c.y.a << (c.y.negative ? "-" : "+")
                     << (c.has_y ? "" : " (none)") << " fixed " << c.fixed
                     << " z " << c.low << ".." << c.high << " b" << c.b
                     << " k" << c.mask << '/' << c.value << " rounding "
                     << static_cast<int>(c.rounding) << " direction "
                     << static_cast<int>(d) << ": " << (found ? "" : "none ")
                     << x << " expected " << (any ? "" : "none ") << expected
                     << '\n';
              return false;
            }
            return true;
          }
          // The cube of index i over bits bits: per bit unknown, 0 or 1
          static void cube(unsigned int i, unsigned int bits,
                           std::uint64_t& mask, std::uint64_t& value) {
            mask = value = 0;
            for (unsigned int bit = 0; bit < bits; ++bit, i /= 3)
              if (i % 3 != 0) {
                mask |= std::uint64_t(1) << bit;
                if (i % 3 == 2)
                  value |= std::uint64_t(1) << bit;
              }
          }
          // The sums of the terms of c, without the condition
          static void sum_range(const Case& c, long long& low,
                                long long& high) {
            low = std::min(term(c.x, c.x.from), term(c.x, c.x.to)) + c.fixed;
            high = std::max(term(c.x, c.x.from), term(c.x, c.x.to)) + c.fixed;
            if (c.has_y) {
              low += std::min(term(c.y, c.y.from), term(c.y, c.y.to));
              high += std::max(term(c.y, c.y.from), term(c.y, c.y.to));
            }
          }

        public:
          WideSumTerms() : Base("Word::Float::Arithmetic::Wide::SumTerms") {}
          bool run() override {
            const Rounding roundings[] = {Rounding::Down, Rounding::Up,
                                          Rounding::Nearest};
            // Width 3: every cube of x and y and of the low two bits of the
            // rounded sum, the whole ranges and every sign and rounding, at
            // five relations of the spacings.
            const unsigned int spacings[][3] = {
                {1, 1, 1}, {1, 1, 3}, {2, 1, 1}, {1, 3, 4}, {2, 2, 5}};
            for (const auto& s : spacings)
              for (int signs = 0; signs < 4; ++signs)
                for (Rounding r : roundings)
                  for (unsigned int xc = 0; xc < 27; ++xc)
                    for (unsigned int yc = 0; yc < 27; ++yc)
                      for (unsigned int zc = 0; zc < 9; ++zc) {
                        Case c{{0, 7, 0, 0, s[0], 3, (signs & 1) != 0},
                               {0, 7, 0, 0, s[1], 3, (signs & 2) != 0},
                               true, 0, 0, 1, 1000, s[2], 0, 0, r};
                        cube(xc, 3, c.x.mask, c.x.value);
                        cube(yc, 3, c.y.mask, c.y.value);
                        cube(zc, 2, c.mask, c.value);
                        if (!check(c))
                          return false;
                      }
            // Width 3: every range of x and y, no known bits on them, and
            // windows of the sum against them.
            for (const auto& s : spacings)
              for (int signs = 0; signs < 3; ++signs)
                for (Rounding r : roundings)
                  for (std::uint64_t xf = 0; xf < 8; ++xf)
                    for (std::uint64_t xt = xf; xt < 8; ++xt)
                      for (std::uint64_t yf = 0; yf < 8; ++yf)
                        for (std::uint64_t yt = yf; yt < 8; ++yt)
                          for (int w = 0; w < 3; ++w) {
                            Case c{{xf, xt, 0, 0, s[0], 3, signs == 1},
                                   {yf, yt, 0, 0, s[1], 3, signs == 2},
                                   true, 0, 0, 0, 0, s[2],
                                   std::uint64_t(w == 2), 0, r};
                            long long low, high;
                            sum_range(c, low, high);
                            const long long span = high - low;
                            c.low = std::max(1LL, low + span * w / 4 + w);
                            c.high = std::max(c.low, high - span * w / 3);
                            if (!check(c))
                              return false;
                          }
            // BF16 size (width 8, the hidden bit known as in a binade):
            // random ranges and known bits at many spacings, with and
            // without the second term, every sign and rounding.
            std::mt19937_64 random(5);
            const unsigned int as[] = {1, 2, 3, 6}, cs[] = {1, 2, 4, 11},
                               bs[] = {1, 2, 3, 5, 9, 13};
            for (unsigned int a : as)
              for (unsigned int cy : cs)
                for (unsigned int b : bs)
                  for (int signs = 0; signs < 5; ++signs)
                    for (Rounding r : roundings)
                      for (int i = 0; i < 6; ++i) {
                        const bool has_y = signs < 4;
                        Case c{{0, 0, 0x80, 0x80, a, 8, (signs & 1) != 0},
                               {0, 0, 0x80, 0x80, cy, 8, (signs & 2) != 0},
                               has_y, 0, 0, 0, 0, b, 0, 0, r};
                        for (SumTerm* t : {&c.x, &c.y}) {
                          std::uint64_t u = 128 + random() % 128,
                                        v = 128 + random() % 128;
                          if (i % 3 == 0) {
                            u = 128;
                            v = 255;
                          }
                          t->from = std::min(u, v);
                          t->to = std::max(u, v);
                          const std::uint64_t known =
                              i % 2 ? random() & random() & 0x7f
                                    : random() & 0x7f;
                          t->mask |= known;
                          t->value |= random() & known;
                        }
                        if (!has_y) {
                          c.scale = static_cast<unsigned int>(random() % 12);
                          c.fixed = static_cast<long long>(random() % 4096) -
                                    2048;
                          c.fixed *= 1LL << c.scale;
                        }
                        long long low, high;
                        sum_range(c, low, high);
                        if (high < 1)
                          continue;
                        low = std::max(1LL, low);
                        const long long span = high - low + 1;
                        const long long p =
                            low + static_cast<long long>(random() % span);
                        const long long q =
                            low + static_cast<long long>(random() % span);
                        c.low = std::min(p, q);
                        c.high = std::max(p, q);
                        if (i % 4 == 3) {
                          c.low = low;
                          c.high = high;
                        }
                        c.mask = random() & 15;
                        c.value = random() & c.mask;
                        if (!check(c))
                          return false;
                      }
            return true;
          }
        };

        class WideWalk : public Base {
        public:
          WideWalk() : Base("Word::Float::Arithmetic::Wide::Walk") {}
          bool run() override {
            // x+c=y and y+c=x with c=2^-23 have no solution, but over [1,2)
            // each round moves a bound by one ULP. A window of a few dozen
            // ULPs keeps the walk short while every move still applies.
            const Reference ref(WDT_FP32);
            Pair s(WDT_FP32);
            for (int i : {0, 2})
              bounds(s, s.x[i], {ref.one, ref.one + 40});
            dom(s, s.x[1], (127 - 23) * ref.normal);
            fp_add(s, s.x[0], s.x[1], s.x[2]);
            fp_add(s, s.x[2], s.x[1], s.x[0]);
            return s.status() == SS_FAILED;
          }
        };

        class WideOneUlp : public Base {
        public:
          WideOneUlp() : Base("Word::Float::Arithmetic::Wide::OneUlp") {}
          bool run() override {
            // x+0=z with x over the 2^23 encodings of [1,2) and z one ULP
            // shorter: the bound move on x is one encoding and applies.
            const Reference ref(WDT_FP32);
            Pair s(WDT_FP32);
            const WordValue top = ref.one + ref.normal - 1;
            bounds(s, s.x[0], {ref.one, top});
            dom(s, s.x[1], 0);
            bounds(s, s.x[2], {ref.one, top - 1});
            fp_add(s, s.x[0], s.x[1], s.x[2]);
            return s.status() != SS_FAILED && s.x[0].maximum() == top - 1;
          }
        };

        class WideBoundaryRelations : public Base {
          enum class ResultDomain {
            Unrestricted,
            Singleton,
            AroundTwo,
            OddAroundTwo
          };
          static bool check(const Reference& ref, bool sub, const Policy& p,
                            Window aw, Window bw, ResultDomain result_domain,
                            int alias = 0, bool holes = false) {
            using Row = std::array<WordValue, 4>;
            const unsigned int width = float_word_width(ref.type);
            std::set<Row> expected;
            for (WordValue a : float_order_range(width, aw.minimum, aw.maximum))
              for (WordValue b :
                   float_order_range(width, bw.minimum, bw.maximum)) {
                if (holes && (a & 2))
                  continue;
                const auto z = ref.evaluate(sub, p, a, b);
                if ((alias == 1 && z.bits != a) ||
                    (alias == 2 && z.bits != b))
                  continue;
                expected.insert(
                    {a, b, z.bits,
                     WordValue(z.flags)});
              }
            WordValue selected =
                expected.empty() ? 0 : expected.begin()->at(2);
            if (result_domain >= ResultDomain::AroundTwo)
              selected = (selected & ref.sign) | (ref.one + ref.normal);
            const WordValue radius = result_domain >= ResultDomain::AroundTwo ? 1 : 0;
            const Window zw = magnitudes(selected & ref.sign,
                                         (selected & ~ref.sign) - radius,
                                         (selected & ~ref.sign) + radius);
            if (result_domain != ResultDomain::Unrestricted) {
              for (auto i = expected.begin(); i != expected.end();) {
                if (!float_order_contains(width, zw.minimum, zw.maximum,
                                          (*i)[2]) ||
                    (result_domain == ResultDomain::OddAroundTwo && !((*i)[2] & 1)))
                  i = expected.erase(i);
                else
                  ++i;
              }
            }
            Pair s(ref.type);
            if (alias)
              s.x[2] = s.x[alias - 1];
            bounds(s, s.x[0], aw);
            if (holes)
              dom(s, s.x[0], WordValue(0), (2 * ref.sign - 1) ^ 2);
            bounds(s, s.x[1], bw);
            if (result_domain != ResultDomain::Unrestricted)
              bounds(s, s.x[2], zw);
            if (result_domain == ResultDomain::OddAroundTwo)
              dom(s, s.x[2], WordValue(1), 2 * ref.sign - 1);
            post_relation(s, sub, p, s.x[0], s.x[1], s.x[2],
                          &s.flags);
            if (s.status() == SS_FAILED)
              return expected.empty();
            for (const auto& tuple : expected) {
              for (int i = 0; i < 3; ++i)
                if (!Gecode::Word::WordView(s.x[i]).in(tuple[i]))
                  return false;
              if (!s.flags.in(static_cast<int>(tuple[3])))
                return false;
            }
            branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                   WORD_VAL_LSB());
            std::set<Row> found;
            DFS<Pair> search(&s);
            while (std::unique_ptr<Pair> solution{search.next()})
              found.insert({solution->x[0].val(),
                            solution->x[1].val(),
                            solution->x[2].val(),
                            WordValue(solution->flags.val())});
            if (found != expected) {
              olog << ind(2) << "wide boundary relation " << ref.type << ' '
                        << sub << ' ' << p.rounding << ' '
                        << static_cast<int>(result_domain) << ' ' << std::hex
                        << aw.minimum << ' ' << bw.minimum << std::dec << '\n';
              return false;
            }
            return true;
          }

        public:
          WideBoundaryRelations()
              : Base("Word::Float::Arithmetic::Wide::BoundaryRelations") {}
          bool run() override {
            for (WordDomainType format : {WDT_BF16, WDT_FP32}) {
              const Reference ref(format);
              const std::array<Window, 2> windows[] = {
                  {{{0, 3}, {ref.sign | 3, ref.sign}}},
                  {{{ref.normal - 2, ref.normal + 1},
                    {ref.sign | (ref.normal + 1), ref.sign | (ref.normal - 2)}}},
                  {{{ref.one, ref.one + 3},
                    {ref.one - (ref.fraction + 1) * ref.normal,
                     ref.one - (ref.fraction + 1) * ref.normal + 3}}},
                  {{{ref.maximum - 3, ref.maximum}, {ref.one, ref.one + 3}}},
                  {{{ref.maximum - 3, ref.maximum},
                    {ref.maximum - 3, ref.maximum}}},
                  {{{ref.sign | (ref.one + 3), ref.sign | ref.one},
                    {ref.one, ref.one + 3}}}};
              for (int r = 0; r < 4; ++r) {
                const auto rounding = static_cast<Policy::Rounding>(r);
                const Policy policies[] = {
                    Policy{rounding},
                    Policy{rounding, Policy::SignedZero},
                    Policy{rounding, Policy::Preserve,
                           Policy::SignedZero},
                    Policy{rounding, Policy::SignedZero,
                           Policy::SignedZero, Policy::SaturateFinite,
                           Policy::Positive, Policy::Canonical,
                           Policy::BeforeRounding, Policy::SignalingComparison}};
                for (bool sub : {false, true})
                  for (WordValue sign : {WordValue(0), ref.sign})
                    for (ResultDomain result_domain :
                         {ResultDomain::AroundTwo, ResultDomain::OddAroundTwo}) {
                      const WordValue bsign = sub ? sign : sign ^ ref.sign;
                      if (!check(ref, sub, Policy{rounding},
                                 magnitudes(sign, ref.one + 2 * ref.normal - 2,
                                            ref.one + 2 * ref.normal + 2),
                                 magnitudes(bsign, ref.one + ref.normal - 2,
                                            ref.one + ref.normal + 2),
                                 result_domain))
                        return false;
                    }
                const WordValue large =
                    ref.one + (ref.fraction + 1) * ref.normal;
                for (bool sub : {false, true})
                  for (WordValue sign : {WordValue(0), ref.sign})
                    for (bool holes : {false, true}) {
                      const Policy p{rounding};
                      if (!check(ref, sub, p,
                                 magnitudes(sign, large - 2, large + 3),
                                 {ref.one - 1, ref.one + 1},
                                 ResultDomain::Unrestricted, 1, holes) ||
                          !check(ref, sub, p,
                                 magnitudes(sign, ref.one - 2, ref.one + 3),
                                 {large - 2, large + 3},
                                 ResultDomain::Unrestricted, 2, holes))
                        return false;
                    }
                for (WordValue sign : {WordValue(0), ref.sign})
                  if (!check(ref, true, Policy{rounding},
                             magnitudes(sign, ref.one + ref.normal - 2,
                                        ref.one + ref.normal + 3),
                             magnitudes(sign, ref.one - 2, ref.one + 3),
                             ResultDomain::Unrestricted, 2))
                    return false;
                for (const auto& p : policies)
                  for (bool sub : {false, true})
                    for (const auto& window : windows)
                      for (bool inverse : {false, true})
                        if (!check(ref, sub, p, window[0], window[1],
                                   inverse ? ResultDomain::Singleton : ResultDomain::Unrestricted))
                          return false;
              }
            }
            return true;
          }
        };

        // Operands that admit NaNs and infinities, as a fresh word does, do
        // not delay propagation: their special values are projected by class
        // beside the finite values, and the status keeps only the flags of
        // the remaining classes.
        class WideSpecialOperands : public Base {
          // a op 2 = 1 with a fresh: a keeps exactly the finite supports,
          // from first to last in total order, as the same posting with a
          // finite a does.
          static bool solved(const Reference& ref, bool sub, WordValue first,
                             WordValue last) {
            Pair s(ref.type);
            const WordValue two = ref.one + ref.normal;
            dom(s, s.x[1], two);
            dom(s, s.x[2], ref.one);
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            StatusStatistics statistics;
            if (s.status(statistics) == SS_FAILED || statistics.propagate > 8)
              return false;
            const WordVar a = s.x[0];
            if (!a.bounded() || a.minimum() != first || a.maximum() != last) {
              if (opt.log)
                olog << ind(2) << "special operands " << ref.type << ' ' << sub
                     << std::hex << ": a in " << a.minimum() << ".."
                     << a.maximum() << std::dec << '\n';
              return false;
            }
            return s.flags.assigned() == (first == last) &&
                   closed_relation(s, sub, Policy());
          }
          // Fresh operands and a result in [1,2]: no class gives an invalid,
          // overflowing or underflowing result, so the status keeps 0 and X.
          static bool exact_status(const Reference& ref, bool sub) {
            Pair s(ref.type);
            bounds(s, s.x[2], {ref.one, ref.one + ref.normal});
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            if (s.status() == SS_FAILED)
              return false;
            for (int flag = 0; flag < 32; ++flag)
              if (s.flags.in(flag) != (flag == 0 || flag == 16))
                return false;
            return closed_relation(s, sub, Policy());
          }
          // An infinite a: a finite b, or an infinite b that adds the same
          // infinity (equal signs for addition, opposite ones for
          // subtraction), gives that infinity without flags; the other
          // infinity and the NaNs give NaNs with I or without flags. A
          // result in [1,2] admits none of them.
          static bool infinite(const Reference& ref, bool sub) {
            Pair s(ref.type);
            dom(s, s.x[0], ref.infinity);
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            if (s.status() == SS_FAILED || s.flags.size() != 2 ||
                !s.flags.in(0) || !s.flags.in(1) ||
                !closed_relation(s, sub, Policy()))
              return false;
            Pair none(ref.type);
            dom(none, none.x[0], ref.infinity);
            bounds(none, none.x[2], {ref.one, ref.one + ref.normal});
            post_relation(none, sub, Policy(), none.x[0], none.x[1],
                          none.x[2], &none.flags);
            return none.status() == SS_FAILED;
          }

          // The rows below pin the outcome of each pair of operand classes
          // from the fp_add contract, on operands that keep a whole class
          // unassigned.
          static bool fail(const Reference& ref, const char* row, bool sub) {
            if (opt.log)
              olog << ind(2) << "special operands " << ref.type << ' ' << row << ' '
                   << sub << '\n';
            return false;
          }
          static bool window(const WordVar& x, Window w) {
            return x.bounded() && x.minimum() == w.minimum &&
                   x.maximum() == w.maximum;
          }
          // Total order of the encodings: negative codes by decreasing
          // magnitude, then positive codes by increasing magnitude
          static WordValue order(const Reference& ref, WordValue code) {
            return code >= ref.sign ? 2 * ref.sign - 1 - code
                                    : ref.sign + code;
          }
          /// Every code, from the negative NaN of greatest payload to the
          /// positive one
          static Window all_codes(const Reference& ref) {
            return {ref.sign | (ref.sign - 1), ref.sign - 1};
          }
          static bool within(const Reference& ref, const WordVar& x,
                             Window w) {
            return x.bounded() &&
                   order(ref, x.minimum()) >= order(ref, w.minimum) &&
                   order(ref, x.maximum()) <= order(ref, w.maximum);
          }
          static bool flags_are(const IntVar& flags,
                                std::initializer_list<int> values) {
            for (int flag = 0; flag < 32; ++flag)
              if (flags.in(flag) != (std::find(values.begin(), values.end(),
                                               flag) != values.end()))
                return false;
            return true;
          }
          // a = +inf and a number or infinite b. The difference of equal
          // infinities (opposite ones for addition) is invalid: the
          // canonical NaN with I. Every other pair gives +inf exactly.
          static bool infinite_pairs(const Reference& ref, bool sub) {
            const WordValue canonical = ref.infinity + ref.quiet;
            const WordValue cancelling = sub ? ref.infinity
                                             : ref.sign | ref.infinity;
            for (int result = 0; result < 3; ++result) {
              Pair s(ref.type);
              dom(s, s.x[0], ref.infinity);
              bounds(s, s.x[1], {ref.sign | ref.infinity, ref.infinity});
              if (result == 1)
                dom(s, s.x[2], canonical);
              if (result == 2)
                dom(s, s.x[2], ref.infinity);
              post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2],
                            &s.flags);
              if (s.status() == SS_FAILED)
                return fail(ref, "infinite pairs failed", sub);
              bool ok;
              if (result == 0)
                // Exactly {+inf, canonical NaN}
                ok = window(s.x[2], {ref.infinity, canonical}) &&
                     s.x[2].unknown() == ref.quiet &&
                     flags_are(s.flags, {0, 1});
              else if (result == 1)
                ok = s.x[1].assigned() && s.x[1].val() == cancelling &&
                     flags_are(s.flags, {1});
              else
                // Every b but the cancelling infinity
                ok = window(s.x[1], sub ? Window{ref.sign | ref.infinity,
                                                 ref.maximum}
                                        : Window{ref.sign | ref.maximum,
                                                 ref.infinity}) &&
                     flags_are(s.flags, {0});
              if (!ok || !closed_relation(s, sub, Policy()))
                return fail(ref, "infinite pairs", sub);
            }
            return true;
          }
          // A positive number a and an infinite b give the infinity of b,
          // its sign negated for subtraction: -inf here, without flags.
          static bool infinite_b(const Reference& ref, bool sub) {
            Pair s(ref.type);
            bounds(s, s.x[0], {0, ref.maximum});
            dom(s, s.x[1], (sub ? 0 : ref.sign) | ref.infinity);
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            if (s.status() == SS_FAILED || !s.x[2].assigned() ||
                s.x[2].val() != (ref.sign | ref.infinity) ||
                !window(s.x[0], {0, ref.maximum}) || !flags_are(s.flags, {0}))
              return fail(ref, "infinite b", sub);
            return true;
          }
          // A NaN operand gives the canonical NaN under Canonical, and
          // otherwise the selected NaN, quiet and with its own sign: the
          // first signaling NaN under SignalFirst, the first NaN under
          // OperandFirst. A signaling operand sets I. Subtraction changes
          // no NaN sign.
          static bool nan_selection(const Reference& ref, bool sub) {
            const WordValue canonical = ref.infinity + ref.quiet;
            const Window negative_quiet =
                magnitudes(ref.sign, canonical, ref.sign - 1);
            const Window positive_quiet =
                magnitudes(0, canonical, ref.sign - 1);
            for (Policy::Nan nan : {Policy::SignalFirst, Policy::OperandFirst,
                                    Policy::Canonical})
              for (bool finite_a : {false, true}) {
                Policy p;
                p.nan = nan;
                Pair s(ref.type);
                // A negative quiet a and a positive signaling b, or a
                // positive number a and a negative quiet b
                if (finite_a) {
                  bounds(s, s.x[0], {0, ref.maximum});
                  bounds(s, s.x[1], negative_quiet);
                } else {
                  bounds(s, s.x[0], negative_quiet);
                  bounds(s, s.x[1], {ref.infinity + 1, canonical - 1});
                }
                post_relation(s, sub, p, s.x[0], s.x[1], s.x[2], &s.flags);
                if (s.status() == SS_FAILED)
                  return fail(ref, "nan selection failed", sub);
                const WordVar r = s.x[2];
                bool ok;
                if (nan == Policy::Canonical)
                  ok = r.assigned() && r.val() == canonical;
                else if (nan == Policy::OperandFirst || finite_a)
                  // a, or b after a number: every negative quiet NaN
                  ok = within(ref, r, negative_quiet) &&
                       r.in(negative_quiet.minimum) &&
                       r.in(negative_quiet.maximum);
                else
                  // b made quiet, payloads 1 to quiet - 1 and their sign
                  ok = within(ref, r, positive_quiet) &&
                       r.in(canonical + 1) && r.in(ref.sign - 1);
                if (!ok || !flags_are(s.flags, {finite_a ? 0 : 1}) ||
                    !closed_relation(s, sub, p))
                  return fail(ref, "nan selection", sub);
              }
            return true;
          }
          // closed_relation for x op x = result
          static bool closed_repeated(Pair& original, bool sub) {
            std::unique_ptr<Pair> clone(static_cast<Pair*>(original.clone()));
            post_relation(*clone, sub, Policy(), clone->x[0], clone->x[0],
                          clone->x[2], &clone->flags);
            return clone->status() != SS_FAILED &&
                   same_domains(original, *clone);
          }
          // x - x: a number gives an exact +0 under RNE, an infinity the
          // canonical NaN with I, a NaN that NaN made quiet, with I when it
          // is signaling.
          static bool repeated_difference(const Reference& ref) {
            const WordValue canonical = ref.infinity + ref.quiet;
            Pair fresh(ref.type);
            post_relation(fresh, true, Policy(), fresh.x[0], fresh.x[0],
                          fresh.x[2], &fresh.flags);
            // Every x has a difference, so x keeps every code
            if (fresh.status() == SS_FAILED ||
                !window(fresh.x[0], all_codes(ref)) || !fresh.x[2].in(0) ||
                !fresh.x[2].in(canonical) || !flags_are(fresh.flags, {0, 1}) ||
                !closed_repeated(fresh, true))
              return fail(ref, "x - x fresh", true);
            // A number x: +0 without flags
            Pair numbers(ref.type);
            bounds(numbers, numbers.x[0],
                   {ref.sign | ref.maximum, ref.maximum});
            post_relation(numbers, true, Policy(), numbers.x[0], numbers.x[0],
                          numbers.x[2], &numbers.flags);
            if (numbers.status() == SS_FAILED || !numbers.x[2].assigned() ||
                numbers.x[2].val() != 0 || !flags_are(numbers.flags, {0}) ||
                !closed_repeated(numbers, true))
              return fail(ref, "x - x numbers", true);
            // A result +0: x keeps exactly the numbers
            Pair zero(ref.type);
            dom(zero, zero.x[2], 0);
            post_relation(zero, true, Policy(), zero.x[0], zero.x[0],
                          zero.x[2], &zero.flags);
            if (zero.status() == SS_FAILED ||
                !window(zero.x[0], {ref.sign | ref.maximum, ref.maximum}) ||
                !flags_are(zero.flags, {0}) || !closed_repeated(zero, true))
              return fail(ref, "x - x zero", true);
            return true;
          }
          // x + x = +inf: +inf + +inf is +inf exactly, and a number x of
          // magnitude at least 2^127, the code infinity - normal, has 2x at
          // least 2^128, past the greatest finite value and half its
          // spacing, so it overflows to +inf with O|X under RNE; a smaller
          // x doubles exactly.
          static bool repeated_overflow(const Reference& ref) {
            Pair s(ref.type);
            dom(s, s.x[2], ref.infinity);
            post_relation(s, false, Policy(), s.x[0], s.x[0], s.x[2],
                          &s.flags);
            if (s.status() == SS_FAILED ||
                !window(s.x[0], {ref.infinity - ref.normal, ref.infinity}) ||
                !flags_are(s.flags, {0, 20}) || !closed_repeated(s, false))
              return fail(ref, "x + x overflow", false);
            return true;
          }
          // A status of I or U|X: without flushing an exact sum never
          // underflows, so the status is I, which only a signaling operand
          // or the difference of infinities gives, as a quiet NaN. Every a
          // and b is kept: a signaling b supports any a, and so on. The
          // result keeps the quiet NaNs of both signs.
          static bool invalid_status(const Reference& ref, bool sub) {
            Pair s(ref.type);
            dom(s, s.flags, IntSet({1, 24}));
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            const WordValue nan = ref.infinity | ref.quiet;
            if (s.status() == SS_FAILED || !flags_are(s.flags, {1}) ||
                (s.x[2].lo() & nan) != nan ||
                !window(s.x[2], all_codes(ref)) ||
                !window(s.x[0], all_codes(ref)) ||
                !window(s.x[1], all_codes(ref)) ||
                !closed_relation(s, sub, Policy()))
              return fail(ref, "invalid status", sub);
            return true;
          }
          // a op b = -inf: a sum of finite numbers past the greatest finite
          // magnitude rounds to -inf with O|X under RNE, and -inf with a
          // number or -inf (+inf as subtrahend) gives -inf exactly. So the
          // status is 0 or O|X, a is -inf or a number, and b likewise, or
          // +inf or a number for subtraction. No NaN gives an infinity.
          static bool negative_overflow(const Reference& ref, bool sub) {
            Pair s(ref.type);
            dom(s, s.x[2], ref.sign | ref.infinity);
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            const Window numbers_and_minus_infinity{ref.sign | ref.infinity,
                                                    ref.maximum};
            if (s.status() == SS_FAILED || !flags_are(s.flags, {0, 20}) ||
                !window(s.x[0], numbers_and_minus_infinity) ||
                !window(s.x[1], sub ? Window{ref.sign | ref.maximum,
                                             ref.infinity}
                                    : numbers_and_minus_infinity) ||
                !closed_relation(s, sub, Policy()))
              return fail(ref, "negative overflow", sub);
            return true;
          }
          // A positive signaling a and a negative quiet b, each with every
          // payload: under SignalFirst every pair gives a made quiet, with
          // its positive sign and I, so a and b keep their extreme payloads
          // and the result is a positive quiet NaN.
          static bool nan_payloads(const Reference& ref, bool sub) {
            const WordValue canonical = ref.infinity + ref.quiet;
            const Window signaling{ref.infinity + 1, canonical - 1};
            const Window negative_quiet =
                magnitudes(ref.sign, canonical, ref.sign - 1);
            Pair s(ref.type);
            bounds(s, s.x[0], signaling);
            bounds(s, s.x[1], negative_quiet);
            post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2], &s.flags);
            const WordVar r = s.x[2];
            if (s.status() == SS_FAILED || !window(s.x[0], signaling) ||
                !window(s.x[1], negative_quiet) ||
                !within(ref, r, magnitudes(0, canonical, ref.sign - 1)) ||
                !r.in(canonical + 1) || !r.in(ref.sign - 1) ||
                !flags_are(s.flags, {1}) || !closed_relation(s, sub, Policy()))
              return fail(ref, "nan payloads", sub);
            return true;
          }
          // x op x with a NaN x is x made quiet, with I when x is signaling.
          // A fresh x keeps every code. A result among the negative quiet
          // NaNs keeps exactly the negative NaNs of x, with status 0 or I.
          static bool repeated_nan(const Reference& ref, bool sub) {
            Pair fresh(ref.type);
            post_relation(fresh, sub, Policy(), fresh.x[0], fresh.x[0],
                          fresh.x[2], &fresh.flags);
            if (fresh.status() == SS_FAILED ||
                !window(fresh.x[0], all_codes(ref)) ||
                !closed_repeated(fresh, sub))
              return fail(ref, "x op x fresh", sub);
            const WordValue canonical = ref.infinity + ref.quiet;
            Pair s(ref.type);
            bounds(s, s.x[2], magnitudes(ref.sign, canonical, ref.sign - 1));
            post_relation(s, sub, Policy(), s.x[0], s.x[0], s.x[2], &s.flags);
            if (s.status() == SS_FAILED ||
                !window(s.x[0],
                        magnitudes(ref.sign, ref.infinity + 1, ref.sign - 1)) ||
                !flags_are(s.flags, {0, 1}) || !closed_repeated(s, sub))
              return fail(ref, "x op x nan", sub);
            return true;
          }

        public:
          WideSpecialOperands()
              : Base("Word::Float::Arithmetic::Wide::SpecialOperands", TestTags::check()) {}
          bool run() override {
            for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
              const Reference ref(f);
              // -1 + 2 = 1 and the tie -(1 - u/2) + 2 = 1 + u/2, u the
              // spacing at 1; 3 - 2 = 1 alone.
              const WordValue minus_one = ref.sign | ref.one;
              if (!solved(ref, false, minus_one, minus_one - 1) ||
                  !solved(ref, true, ref.one + ref.normal + ref.normal / 2,
                          ref.one + ref.normal + ref.normal / 2))
                return false;
              for (bool sub : {false, true})
                if (!exact_status(ref, sub) || !infinite(ref, sub) ||
                    !infinite_pairs(ref, sub) || !infinite_b(ref, sub) ||
                    !nan_selection(ref, sub) || !invalid_status(ref, sub) ||
                    !negative_overflow(ref, sub) || !nan_payloads(ref, sub) ||
                    !repeated_nan(ref, sub))
                  return false;
              if (!repeated_difference(ref) || !repeated_overflow(ref))
                return false;
            }
            return true;
          }
        };

        class WidePosting : public Base {
        public:
          WidePosting() : Base("Word::Float::Arithmetic::Wide::Posting") {}
          bool run() override {
            for (bool sub : {false, true})
              for (bool status : {false, true})
                for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
                  Pair s(f);
                  post_relation(s, sub, Policy(), s.x[0], s.x[1], s.x[2],
                                status ? &s.flags : nullptr);
                  if (s.status() == SS_FAILED)
                    return false;
                }
            Pair wide_flags(WDT_FP32);
            wide_flags.flags = IntVar(wide_flags, -10, 40);
            fp_add(wide_flags, wide_flags.x[0], wide_flags.x[1],
                   wide_flags.x[2], wide_flags.flags);
            if (wide_flags.flags.min() != 0 || wide_flags.flags.max() != 31)
              return false;
            Pair invalid_flags(WDT_BF16);
            invalid_flags.flags = IntVar(invalid_flags, 32, 32);
            fp_subtract(invalid_flags, invalid_flags.x[0],
                        invalid_flags.x[1], invalid_flags.x[2],
                        invalid_flags.flags);
            if (invalid_flags.status() != SS_FAILED)
              return false;
            return true;
          }
        };
        using PairTuple = std::array<WordValue, 4>;

        std::set<PairTuple> pair_solutions(Pair& s) {
          branch(s, WordVarArgs{s.x[0], s.x[1], s.x[2]},
                 WORD_VAR_NONE(), WORD_VAL_LSB());
          branch(s, s.flags, INT_VAL_MIN());
          DFS<Pair> search(&s);
          std::set<PairTuple> found;
          while (std::unique_ptr<Pair> next{search.next()})
            found.insert({next->x[0].val(), next->x[1].val(),
                          next->x[2].val(),
                          WordValue(next->flags.val())});
          return found;
        }

        // Sums and differences on float-typed words, whose bounds are in IEEE
        // total order, over negative-only and cross-zero windows.
        class WideFloatTyped : public Base {
          static bool check(const Reference& ref, bool sub, const Policy& p,
                            Window a, Window b, Window z) {
            const unsigned int width = float_word_width(ref.type);
            Pair s(ref.type);
            s.x[0] = float_window(s, ref.type, a.minimum, a.maximum);
            s.x[1] = float_window(s, ref.type, b.minimum, b.maximum);
            s.x[2] = float_window(s, ref.type, z.minimum, z.maximum);
            std::set<PairTuple> expected;
            for (WordValue av : float_order_range(width, a.minimum, a.maximum))
              for (WordValue bv :
                   float_order_range(width, b.minimum, b.maximum)) {
                const FloatResult e = ref.evaluate(sub, p, av, bv);
                if (s.x[2].in(e.bits))
                  expected.insert({av, bv, e.bits, WordValue(e.flags)});
              }
            post_relation(s, sub, p, s.x[0], s.x[1], s.x[2], &s.flags);
            if (s.status() == SS_FAILED ? expected.empty()
                                        : pair_solutions(s) == expected)
              return true;
            if (opt.log)
              olog << ind(2) << "float-typed " << (sub ? "subtract " : "add ")
                   << ref.type << ' ' << p.rounding << ' ' << std::hex
                   << a.minimum << ".." << a.maximum << ' ' << b.minimum
                   << ".." << b.maximum << ' ' << z.minimum << ".."
                   << z.maximum << std::dec << '\n';
            return false;
          }
          static bool one_half(const Reference& ref, Window w) {
            return ((w.minimum ^ w.maximum) & ref.sign) == 0;
          }
          static bool windows(const Reference& ref) {
            const WordValue sign = ref.sign, one = ref.one,
                            normal = ref.normal;
            const Window all_negative{sign | ref.infinity, sign};
            const Window all{sign | ref.infinity, ref.infinity};
            // Operand windows in total order, with the result window: all
            // values, or a negative window that drops the zeros and tails.
            const Window cases[][3] = {
                {{sign | (one + 4), sign | (one - 4)}, {one, one + 2}, all},
                {{sign | 3, 3}, {sign | 1, 1}, all},
                {{sign | (one + 2), sign | one}, {one - 2, one + 2}, all},
                {{sign | (normal + 1), sign | (normal - 2)},
                 {sign | 2, sign},
                 all_negative},
                {{sign | (one + 8), sign | (one - 8)},
                 {sign | (one + 1), sign | (one - 1)},
                 {sign | (one + normal + 1), sign | (one + normal)}},
                {{sign | (one + 2), sign | one},
                 {one - 3, one - 1},
                 {sign | 0x7f, sign | 1}}};
            for (bool sub : {false, true})
              for (int r = 0; r < 4; ++r)
                for (bool flush : {false, true}) {
                  const auto mode = static_cast<Policy::Rounding>(r);
                  const Policy p =
                      flush ? Policy{mode, Policy::SignedZero,
                                     Policy::SignedZero}
                            : Policy{mode};
                  for (const auto& c : cases) {
                    const Window b = sub ? Window{c[1].maximum ^ sign,
                                                  c[1].minimum ^ sign}
                                         : c[1];
                    if (sub && !one_half(ref, c[1]))
                      continue;
                    if (!check(ref, sub, p, c[0], b, c[2]))
                      return false;
                  }
                }
            return true;
          }
          // The cancellation bound of a negative sum: with a negative and b
          // positive, a + b = z admits only a >= z - C(z), so the most
          // negative endpoint of a moves, while the exact witness a = 2z,
          // b = -z stays.
          static bool cancellation(const Reference& ref) {
            const WordValue sign = ref.sign;
            const WordValue z = ref.one + ref.normal - 1;
            for (Policy::Rounding mode : {Policy::RUP, Policy::RDN})
              for (bool broad : {false, true}) {
                Pair s(ref.type);
                s.x[0] = float_window(s, ref.type,
                                      sign | (ref.one + 13 * ref.normal),
                                      sign | ref.one);
                s.x[1] = float_window(s, ref.type, ref.one,
                                      ref.one + 13 * ref.normal);
                s.x[2] = float_window(s, ref.type,
                                      sign | (z + (broad ? 1 : 0)),
                                      sign | (z - (broad ? 1 : 0)));
                fp_add(s, s.x[0], s.x[1], s.x[2], s.flags, Policy{mode});
                if (s.status() == SS_FAILED)
                  return false;
                if (!broad && s.x[0].minimum() ==
                                  (sign | (ref.one + 13 * ref.normal)))
                  return false;
                dom(s, s.x[0], sign | (z + ref.normal));
                dom(s, s.x[1], z);
                dom(s, s.x[2], sign | z);
                if (s.status() == SS_FAILED || s.flags.val() != 0)
                  return false;
              }
            return true;
          }
          // The cancellation bound of a negative result must take the
          // largest magnitude in each class: the odd result codes -0x7f and
          // -0x7d keep the witness -0x13f + 0xff = -0x7f, whose operand lies
          // beyond the bound from the smaller magnitude -0x7d.
          static bool cancellation_magnitude() {
            const Reference ref(WDT_BF16);
            Pair s(WDT_BF16);
            s.x[0] = float_window(s, WDT_BF16, 0x8140, 0x813e);
            s.x[1] = float_window(s, WDT_BF16, 0x00fe, 0x00ff);
            s.x[2] = WordVar(s, 16, WDT_BF16, 0x807f, 0x807d, known_bits(0xff81, 0x8001));
            std::set<PairTuple> expected;
            for (WordValue a : float_order_range(16, 0x8140, 0x813e))
              for (WordValue b : float_order_range(16, 0x00fe, 0x00ff)) {
                const FloatResult e = ref.evaluate(false, Policy(), a, b);
                if (s.x[2].in(e.bits))
                  expected.insert({a, b, e.bits, WordValue(e.flags)});
              }
            fp_add(s, s.x[0], s.x[1], s.x[2], s.flags);
            return expected.size() == 2 && s.status() != SS_FAILED &&
                   pair_solutions(s) == expected;
          }
          // A result over the float-typed operand window -3..3 ulp plus +0
          // has the total-order bounds -3 ulp and 3 ulp.
          static bool bound_deduction() {
            const Reference ref(WDT_FP32);
            Pair s(WDT_FP32);
            s.x[0] = float_window(s, WDT_FP32, 0x80000003, 0x00000003);
            s.x[1] = float_window(s, WDT_FP32, 0, 0);
            s.x[2] = WordVar(s, WDT_FP32);
            fp_add(s, s.x[0], s.x[1], s.x[2], s.flags);
            if (s.status() == SS_FAILED ||
                s.x[2].minimum() != 0x80000003 ||
                s.x[2].maximum() != 0x00000003)
              return false;
            std::set<PairTuple> expected;
            for (WordValue a : float_order_range(32, 0x80000003, 0x00000003)) {
              const FloatResult e = ref.evaluate(false, Policy(), a, 0);
              expected.insert({a, 0, e.bits, WordValue(e.flags)});
            }
            return pair_solutions(s) == expected;
          }

        public:
          WideFloatTyped()
              : Base("Word::Float::Arithmetic::Wide::FloatTyped") {}
          bool run() override {
            for (WordDomainType format : {WDT_BF16, WDT_FP32}) {
              const Reference ref(format);
              if (!windows(ref) || !cancellation(ref))
                return false;
            }
            return cancellation_magnitude() && bound_deduction();
          }
        };

        WideFloatTyped wide_float_typed;
        WideBoundaryRelations wide_boundary_relations;
        WideExact wide_exact;
        WideSymbolic wide_symbolic;
        WideLifecycle wide_lifecycle;
        WideCancellation wide_cancellation;
        WideCancellationBudget wide_cancellation_budget;
        WideCancellationPolicies wide_cancellation_policies;
        WideCancellationRounds wide_cancellation_rounds;
        WideSumLattice wide_sum_lattice;
        WideSumTerms wide_sum_terms;
        WideSumReplays wide_sum_replays;
        WideDoubledSums wide_doubled_sums;
        WideWalk wide_walk;
        WideOneUlp wide_one_ulp;
        WidePosting wide_posting;
        WideSpecialOperands wide_special_operands;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
