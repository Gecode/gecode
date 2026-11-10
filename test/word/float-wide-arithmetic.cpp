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

namespace Test {
  namespace Word {
    namespace Float {
      namespace {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        using Integer = Gecode::Word::Arithmetic::Global::Integer;

        struct Expected {
          WordValue bits;
          int flags;
        };

        // Independent neighbor oracle, in minimum-subnormal units (not
        // production's half-unit scale). Binary search includes two virtual
        // finite exponent fields beyond the format, so overflow is decided by
        // precision-rounded neighbors.
        struct Reference {
          WordDomainType format;
          unsigned int fraction;
          WordValue sign, normal, infinity, maximum, quiet, one;
          explicit Reference(WordDomainType f)
              : format(f), fraction(f == WDT_BF16 ? 7 : 23),
                sign(WordValue(1) << (fraction + 8)),
                normal(WordValue(1) << fraction), infinity(255 * normal),
                maximum(infinity - 1), quiet(normal / 2), one(127 * normal) {}
          Integer decode(WordValue magnitude) const {
            unsigned int exponent =
                static_cast<unsigned int>(magnitude / normal);
            Integer value(exponent ? normal + magnitude % normal : magnitude);
            if (exponent)
              value = value * Integer::power_two(exponent - 1);
            return value;
          }
          bool nan(WordValue bits) const { return bits % sign > infinity; }
          Expected evaluate(bool subtract, const Policy& p, WordValue a,
                            WordValue b) const {
            const WordValue am = a % sign, bm = b % sign;
            const bool an = a >= sign, bn = (b >= sign) != subtract;
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
            if (am == infinity && bm == infinity && an != bn)
              return {infinity + quiet,
                      1};
            if (am == infinity || bm == infinity)
              return {infinity + ((am == infinity ? an : bn) ? sign : 0), 0};
            const Integer av =
                p.input_subnormals == Policy::SignedZero && am < normal
                    ? Integer()
                    : decode(am);
            const Integer bv =
                p.input_subnormals == Policy::SignedZero && bm < normal
                    ? Integer()
                    : decode(bm);
            const Integer sum = (an ? -av : av) + (bn ? -bv : bv);
            bool negative = sum.zero()
                                ? (an == bn ? an : p.rounding == Policy::RDN)
                                : sum.neg();
            const Integer exact = sum.abs();
            WordValue low = 0, high = 257 * normal;
            while (low < high) {
              const WordValue middle = (low + high + 1) / 2;
              if (decode(middle) <= exact)
                low = middle;
              else
                high = middle - 1;
            }
            WordValue chosen = low;
            if (decode(low) != exact) {
              if (p.rounding == Policy::RNE) {
                const Integer below = exact - decode(low),
                              above = decode(low + 1) - exact;
                if (above < below || (above == below && (low & 1)))
                  ++chosen;
              } else if ((p.rounding == Policy::RUP && !negative) ||
                         (p.rounding == Policy::RDN && negative)) {
                ++chosen;
              }
            }
            int flags = decode(chosen) == exact ? 0 : 16;
            if (chosen > maximum) {
              flags = 20;
              const bool infinite = p.overflow == Policy::Format &&
                                    (p.rounding == Policy::RNE ||
                                     (p.rounding == Policy::RUP && !negative) ||
                                     (p.rounding == Policy::RDN && negative));
              chosen = infinite ? infinity : maximum;
            } else if (chosen && chosen < normal &&
                       p.output_subnormals == Policy::SignedZero) {
              chosen = 0;
              flags |= 24;
            }
            if (!chosen && p.zeros == Policy::Positive)
              negative = false;
            return {chosen + (negative ? sign : 0), flags};
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
          const Expected expected = ref.evaluate(subtract, p, a, b);
          Pair s(ref.format);
          dom(s, s.x[0], a);
          dom(s, s.x[1], b);
          post_relation(s, subtract, p, s.x[0], s.x[1], s.x[2],
                        &s.flags);
          if (s.status() == SS_FAILED)
            return false;
          if (!s.x[2].assigned() ||
              s.x[2].val() != expected.bits || !s.flags.assigned() ||
              s.flags.val() != expected.flags) {
            std::cerr << "wide exact mismatch " << ref.fraction << ' '
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
              const Expected value = ref.evaluate(e.sub, Policy(), e.a, e.b);
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
              v = WordVar(*this, ref.format);
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
              const Expected z = ref.evaluate(subtract, p, a, b);
              if ((exact_status || later == 3) && z.flags != 0)
                continue;
              if ((direction == 1 || direction == 2) &&
                  z.bits != ref.one + ref.normal)
                continue;
              if (direction != 3) {
                tuples.insert({a, b, z.bits, 0, 0, WordValue(z.flags), 0});
              } else {
                for (WordValue c = ref.one - 1; c <= ref.one + 1; ++c) {
                  const Expected out = ref.evaluate(subtract, p, z.bits, c);
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
                        std::cerr << "wide network mismatch " << format << ' '
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
            const unsigned int width = float_word_width(ref.format);
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
            Pair s(ref.format);
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
              std::cerr << "wide boundary relation " << ref.format << ' '
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
                           Policy::BeforeRounding, Policy::Signaling}};
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
            const unsigned int width = float_word_width(ref.format);
            Pair s(ref.format);
            s.x[0] = float_window(s, ref.format, a.minimum, a.maximum);
            s.x[1] = float_window(s, ref.format, b.minimum, b.maximum);
            s.x[2] = float_window(s, ref.format, z.minimum, z.maximum);
            std::set<PairTuple> expected;
            for (WordValue av : float_order_range(width, a.minimum, a.maximum))
              for (WordValue bv :
                   float_order_range(width, b.minimum, b.maximum)) {
                const Expected e = ref.evaluate(sub, p, av, bv);
                if (s.x[2].in(e.bits))
                  expected.insert({av, bv, e.bits, WordValue(e.flags)});
              }
            post_relation(s, sub, p, s.x[0], s.x[1], s.x[2], &s.flags);
            if (s.status() == SS_FAILED ? expected.empty()
                                        : pair_solutions(s) == expected)
              return true;
            if (opt.log)
              olog << "float-typed " << (sub ? "subtract " : "add ")
                   << ref.format << ' ' << p.rounding << ' ' << std::hex
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
                Pair s(ref.format);
                s.x[0] = float_window(s, ref.format,
                                      sign | (ref.one + 13 * ref.normal),
                                      sign | ref.one);
                s.x[1] = float_window(s, ref.format, ref.one,
                                      ref.one + 13 * ref.normal);
                s.x[2] = float_window(s, ref.format,
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
                const Expected e = ref.evaluate(false, Policy(), a, b);
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
              const Expected e = ref.evaluate(false, Policy(), a, 0);
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
        WideWalk wide_walk;
        WideOneUlp wide_one_ulp;
        WidePosting wide_posting;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
