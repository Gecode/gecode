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
#include <memory>
#include <set>

namespace Test {
  namespace Word {
    namespace FloatSqrt {
      using namespace Gecode;
      using Policy = FloatWordPolicy;

      // A tiny word carries a value set unless value_set is WVS_NONE.
      WordVar variable(Space& home, WordDomainType format,
                       WordValueSetOption value_set = WVS_EXACT) {
        const unsigned int width = float_word_width(format);
        return WordVar(home, format, width <= 8 ? value_set : WVS_NONE);
      }
      class RootSpace : public Space {
      public:
        WordVar x, y;
        IntVar flags;
        explicit RootSpace(WordDomainType format,
                           WordValueSetOption value_set = WVS_EXACT)
            : x(variable(*this, format, value_set)),
              y(variable(*this, format, value_set)), flags(*this, 0, 31) {}
        RootSpace(RootSpace& s) : Space(s) {
          x.update(*this, s.x);
          y.update(*this, s.y);
          flags.update(*this, s.flags);
        }
        Space* copy() override { return new RootSpace(*this); }
      };
      struct Expected {
        WordValue result;
        int flags;
      };
      long long tiny_units(WordValue code, WordDomainType f) {
        if (f == WDT_E2M1) {
          const int halves[] = {0, 1, 2, 3, 4, 6, 8, 12};
          return halves[code];
        }
        return code < 8 ? code : (8 + code % 8) << (code / 8 - 1);
      }
      Expected tiny_expected(WordDomainType f, WordValue code,
                             const Policy& p) {
        const WordValue sign = f == WDT_E2M1 ? 8 : 128;
        WordValue magnitude = code % sign;
        const WordValue normal = f == WDT_E2M1 ? 2 : 8;
        if (f == WDT_E4M3FN && magnitude == 127)
          return {p.nan == Policy::Canonical ? 127 : code, 0};
        if (p.input_subnormals == Policy::SignedZero && magnitude < normal)
          magnitude = 0;
        if (!magnitude)
          return {p.zeros == Policy::Positive ? 0 : code & sign, 0};
        if (code & sign)
          return {f == WDT_E2M1 ? 7U : 127U, 1};
        const long long scale = f == WDT_E2M1 ? 2 : 512;
        const long long square = tiny_units(magnitude, f) * scale;
        WordValue low = 0;
        while (tiny_units(low + 1, f) * tiny_units(low + 1, f) <= square)
          ++low;
        const long long lv = tiny_units(low, f);
        const bool exact = lv * lv == square;
        WordValue output = low;
        if (!exact && p.rounding == Policy::RUP)
          ++output;
        if (!exact && p.rounding == Policy::RNE) {
          const long long sum = lv + tiny_units(low + 1, f);
          if (4 * square > sum * sum || (4 * square == sum * sum && (low & 1)))
            ++output;
        }
        int flags = exact ? 0 : f == WDT_E2M1 && magnitude == 1 ? 24 : 16;
        if (p.output_subnormals == Policy::SignedZero && output < normal) {
          output = 0;
          flags |= 24;
        }
        return {output, flags};
      }
      bool check_concrete(WordDomainType f, WordValue source, const Policy& p,
                          Expected expected, bool wrong = false,
                          WordValueSetOption value_set = WVS_EXACT) {
        RootSpace s(f, value_set);
        dom(s, s.x, source);
        fp_sqrt(s, s.x, s.y, s.flags, p);
        if (s.status() == SS_FAILED || !s.y.assigned() ||
            s.y.val() != expected.result || !s.flags.assigned() ||
            s.flags.val() != expected.flags)
          return false;
        if (wrong) {
          for (bool wrong_result : {false, true}) {
            RootSpace rejected(f);
            dom(rejected, rejected.x, source);
            dom(rejected, rejected.y,
                expected.result ^ (wrong_result ? 1 : 0));
            rel(rejected, rejected.flags, IRT_EQ,
                expected.flags ^ (wrong_result ? 0 : 16));
            fp_sqrt(rejected, rejected.x, rejected.y, rejected.flags, p);
            if (rejected.status() != SS_FAILED)
              return false;
          }
        }
        return true;
      }
      class Tiny : public Base {
      public:
        Tiny() : Base("Word::Float::Sqrt::Tiny") {}
        bool run() override {
          for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN})
            for (int rounding = 0; rounding < 4; ++rounding)
              for (int input = 0; input < 2; ++input)
                for (int output = 0; output < 2; ++output)
                  for (int zeros = 0; zeros < 2; ++zeros)
                    for (int nan = 0; nan < (f == WDT_E4M3FN ? 2 : 1); ++nan)
                      for (int tininess = 0; tininess < (f == WDT_E2M1 ? 2 : 1);
                           ++tininess) {
                        const Policy p{
                            static_cast<Policy::Rounding>(rounding),
                            static_cast<Policy::Subnormals>(input),
                            static_cast<Policy::Subnormals>(output),
                            Policy::Format, static_cast<Policy::Zeros>(zeros),
                            nan ? Policy::Canonical : Policy::SignalFirst,
                            static_cast<Policy::Tininess>(tininess)};
                        for (WordValue source = 0;
                             source < (WordValue(1) << float_word_width(f));
                             ++source)
                          if (!check_concrete(f, source, p,
                                              tiny_expected(f, source, p)))
                            return false;
                      }
          for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN})
            for (WordValue source = 0;
                 source < (WordValue(1) << float_word_width(f)); ++source)
              if (!check_concrete(f, source, Policy(),
                                  tiny_expected(f, source, Policy()), false,
                                  WVS_NONE))
                return false;
          const Policy ignored{Policy::RNE, Policy::Preserve,
                               Policy::Preserve, Policy::SaturateFinite,
                               Policy::Signed, Policy::OperandFirst,
                               Policy::BeforeRounding, Policy::Signaling};
          return check_concrete(WDT_E2M1, 1, ignored, {1, 24}, true) &&
                 check_concrete(WDT_E4M3FN, 0xff, ignored, {0xff, 0}, true);
        }
      } tiny;

      // Independent dyadic inequality oracle: compare individual binary digits
      // across the exponent range, without aligned shifts or root computation.
      struct Rational {
        WordValue numerator;
        int power;
      };
      Rational value(WordDomainType f, WordValue code) {
        const int fraction = f == WDT_FP32 ? 23 : 7;
        const WordValue unit = WordValue(1) << fraction;
        const int exponent = static_cast<int>(code / unit);
        return {code % unit + (exponent ? unit : 0),
                (exponent ? exponent : 1) - 127 - fraction};
      }
      int rational_order(Rational a, Rational b) {
        for (int power = 512; power >= -512; --power) {
          const int ashift = power - a.power, bshift = power - b.power;
          const bool abit =
              ashift >= 0 && ashift < 64 && ((a.numerator >> ashift) & 1);
          const bool bbit =
              bshift >= 0 && bshift < 64 && ((b.numerator >> bshift) & 1);
          if (abit != bbit)
            return abit ? 1 : -1;
        }
        return 0;
      }
      Rational squared(Rational r) {
        return {r.numerator * r.numerator, 2 * r.power};
      }
      Rational squared_midpoint(WordDomainType f, WordValue a, WordValue b) {
        const Rational av = value(f, a), bv = value(f, b);
        const int power = std::min(av.power, bv.power);
        const WordValue sum =
            av.numerator * (WordValue(1) << (av.power - power)) +
            bv.numerator * (WordValue(1) << (bv.power - power));
        return {sum * sum, 2 * (power - 1)};
      }
      bool finite_tuple(WordDomainType f, WordValue source, WordValue result,
                        Policy::Rounding rounding, int flags) {
        const Rational input = value(f, source);
        const int exact = rational_order(squared(value(f, result)), input);
        if (flags != (exact == 0 ? 0 : 16))
          return false;
        if (exact == 0)
          return true;
        if (rounding == Policy::RUP)
          return exact > 0 &&
                 rational_order(squared(value(f, result - 1)), input) < 0;
        if (rounding == Policy::RNE) {
          const int lower =
              rational_order(input, squared_midpoint(f, result - 1, result));
          const int upper =
              rational_order(input, squared_midpoint(f, result, result + 1));
          return (lower > 0 || (lower == 0 && !(result & 1))) &&
                 (upper < 0 || (upper == 0 && !(result & 1)));
        }
        return exact < 0 &&
               rational_order(squared(value(f, result + 1)), input) > 0;
      }
      class Wide : public Base {
      public:
        Wide() : Base("Word::Float::Sqrt::Wide") {}
        bool run() override {
          for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
            const WordValue one = f == WDT_FP32 ? 0x3f800000 : 0x3f80;
            const WordValue normal = f == WDT_FP32 ? 0x00800000 : 0x0080;
            const WordValue infinity = f == WDT_FP32 ? 0x7f800000 : 0x7f80;
            const WordValue sign = WordValue(1) << (float_word_width(f) - 1);
            const WordValue quiet = normal / 2;
            const WordValue inputs[] = {1,
                                        2,
                                        3,
                                        normal - 1,
                                        normal,
                                        normal + 1,
                                        one - 2,
                                        one - 1,
                                        one,
                                        one + 1,
                                        one + 2,
                                        one + normal - 1,
                                        one + normal,
                                        one + normal + 1,
                                        one + 2 * normal,
                                        infinity - 2,
                                        infinity - 1};
            for (int rounding = 0; rounding < 4; ++rounding) {
              const auto mode = static_cast<Policy::Rounding>(rounding);
              const Policy p{mode};
              for (WordValue input : inputs) {
                RootSpace s(f);
                dom(s, s.x, input);
                fp_sqrt(s, s.x, s.y, s.flags, p);
                if (s.status() == SS_FAILED || !s.y.assigned() ||
                    !s.flags.assigned() ||
                    !finite_tuple(f, input, s.y.val(), mode,
                                  s.flags.val()) ||
                    !check_concrete(f, input, p,
                                    {s.y.val(), s.flags.val()}, true))
                  return false;
              }
              for (Expected input : {Expected{0, 0},
                                     {sign, 0},
                                     {infinity, 0},
                                     {infinity | quiet | 3, 0},
                                     {sign | infinity | quiet | 3, 0}})
                if (!check_concrete(f, input.result, p, input, true))
                  return false;
              if (!check_concrete(f, sign | 1, p, {infinity | quiet, 1},
                                  true) ||
                  !check_concrete(f, sign | infinity, p, {infinity | quiet, 1},
                                  true) ||
                  !check_concrete(f, sign | infinity | 3, p,
                                  {sign | infinity | quiet | 3, 1}, true))
                return false;
              const Policy flush{mode, Policy::SignedZero,
                                 Policy::SignedZero, Policy::SaturateFinite,
                                 Policy::Positive, Policy::Canonical,
                                 Policy::BeforeRounding, Policy::Signaling};
              if (!check_concrete(f, sign | 1, flush, {0, 0}, true) ||
                  !check_concrete(f, sign | infinity | 3, flush,
                                  {infinity | quiet, 1}, true))
                return false;
            }
            for (Policy::Rounding mode : {Policy::RNE, Policy::RUP}) {
              const WordValue fixed = mode == Policy::RNE ? one - 1 : one + 1;
              RootSpace s(f);
              dom(s, s.x, fixed);
              fp_sqrt(s, s.x, s.x, s.flags, Policy{mode});
              if (s.status() == SS_FAILED || s.flags.val() != 16)
                return false;
            }
          }
          return true;
        }
      } wide;
      using Tuple = std::array<WordValue, 3>;
      std::set<Tuple> solutions(RootSpace& root, bool status = true) {
        branch(root, WordVarArgs{root.x, root.y}, WORD_VAL_LSB());
        if (status)
          branch(root, root.flags, INT_VAL_MIN());
        DFS<RootSpace> search(&root);
        std::set<Tuple> found;
        while (std::unique_ptr<RootSpace> s{search.next()})
          found.insert({s->x.val(), s->y.val(),
                        status ? static_cast<WordValue>(s->flags.val()) : 0});
        return found;
      }
      // The cube that frees the five low bits of first, or with sparse three
      // of them, intersected with the total-order window from first to last
      // when bounded holds.
      WordVar window(Space& home, WordDomainType f, bool bounded,
                     WordValue first, WordValue last, bool sparse) {
        const WordValue free = sparse ? 0x15 : 0x1f;
        const WordValue base = first & ~WordValue(31);
        const WordValue lo = base, hi = base | free;
        const unsigned int width = float_word_width(f);
        const WordValue sign = WordValue(1) << (width - 1);
        return WordVar(home, width, f,
                       bounded ? first : sign | (sign - 1),
                       bounded ? last : sign - 1, cube_bits(width, lo, hi));
      }
      class Symbolic : public Base {
      public:
        Symbolic() : Base("Word::Float::Sqrt::Symbolic") {}
        bool run() override {
          for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
            const WordValue one = f == WDT_FP32 ? 0x3f800000 : 0x3f80;
            for (bool xbounded : {false, true})
              for (bool ybounded : {false, true})
                for (bool sparse : {false, true})
                  for (int rounding = 0; rounding < 4; ++rounding) {
                    const auto mode = static_cast<Policy::Rounding>(rounding);
                    RootSpace root(f);
                    root.x = window(root, f, xbounded, one + 2, one + 29, sparse);
                    root.y = window(root, f, ybounded, one + 3, one + 9, sparse);
                    std::set<Tuple> expected;
                    for (WordValue a = one; a < one + 32; ++a)
                      for (WordValue b = one; b < one + 32; ++b)
                        if (root.x.in(a) && root.y.in(b))
                          for (int flag : {0, 16})
                            if (finite_tuple(f, a, b, mode, flag))
                              expected.insert(
                                  {a, b, static_cast<WordValue>(flag)});
                    fp_sqrt(root, root.x,
                            root.y, root.flags, Policy{mode});
                    if (root.status() == SS_FAILED) {
                      if (!expected.empty())
                        return false;
                    } else {
                      if (solutions(root) != expected)
                        return false;
                    }
                  }
            // An inverse interval must tighten both ends while retaining many
            // source choices. Adjacent encodings straddle the squared
            // thresholds.
            RootSpace inverse(f);
            inverse.x = float_window(inverse, f, one, one + 31);
            inverse.y = float_window(inverse, f, one + 3, one + 5);
            fp_sqrt(inverse, inverse.x, inverse.y, inverse.flags);
            if (inverse.status() == SS_FAILED ||
                inverse.x.minimum() <= one ||
                inverse.x.maximum() >= one + 31 ||
                inverse.x.assigned())
              return false;
            // Real-valued fixed-point rewriting would wrongly remove these.
            for (Policy::Rounding mode : {Policy::RNE, Policy::RUP}) {
              RootSpace alias(f);
              alias.x = float_window(alias, f, one - 2, one + 2);
              alias.y = alias.x;
              fp_sqrt(alias, alias.x, alias.y,
                      alias.flags, Policy{mode});
              std::set<Tuple> expected;
              for (WordValue a = one - 2; a <= one + 2; ++a)
                for (int flag : {0, 16})
                  if (finite_tuple(f, a, a, mode, flag))
                    expected.insert({a, a, static_cast<WordValue>(flag)});
              if (solutions(alias) != expected)
                return false;
            }
          }
          // Quieting preserves sign and payload, including inverse propagation
          // and signaling/status correlation.
          for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
            const unsigned int width = float_word_width(f);
            const WordValue infinity = f == WDT_FP32 ? 0x7f800000 : 0x7f80;
            const WordValue quiet = f == WDT_FP32 ? 0x400000 : 0x40;
            const WordValue sign = WordValue(1) << (width - 1);
            RootSpace root(f);
            // The total order puts the negative NaN of greatest payload first.
            root.x = WordVar(root, width, f,
                             sign | infinity | quiet | 3, infinity | quiet | 3,
                             cube_bits(width, infinity | 1,
                                       sign | infinity | quiet | 3));
            dom(root, root.y, sign | infinity | quiet | 3);
            fp_sqrt(root, root.x, root.y, root.flags);
            const std::set<Tuple> expected{
                {sign | infinity | 3, sign | infinity | quiet | 3, 1},
                {sign | infinity | quiet | 3, sign | infinity | quiet | 3, 0}};
            if (solutions(root) != expected)
              return false;
          }
          // The source and result are the same word.
          for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN}) {
            RootSpace root(f);
            root.y = root.x;
            std::set<Tuple> expected;
            for (WordValue a = 0; a < (WordValue(1) << float_word_width(f));
                 ++a) {
              const Expected e = tiny_expected(f, a, Policy());
              if (a == e.result)
                expected.insert({a, e.result, static_cast<WordValue>(e.flags)});
            }
            fp_sqrt(root, root.x, root.y, root.flags);
            if (solutions(root) != expected)
              return false;
          }
          return true;
        }
      } symbolic;

      class Lifecycle : public Base {
      public:
        Lifecycle() : Base("Word::Float::Sqrt::Lifecycle") {}
        bool run() override {
          for (WordDomainType f : {WDT_BF16, WDT_FP32}) {
            const unsigned int width = float_word_width(f);
            const WordValue one = f == WDT_FP32 ? 0x3f800000 : 0x3f80;
            RootSpace root(f);
            root.x = float_window(root, f, one + 1, one + 31);
            root.y = float_window(root, f, one, one + 31);
            fp_sqrt(root, root.x, root.y, root.flags);
            if (root.status() == SS_FAILED)
              return false;
            const WordValue old_minimum = root.x.minimum();
            std::unique_ptr<RootSpace> clone(
                static_cast<RootSpace*>(root.clone()));
            // 0..15 -> 2..14 retains the same bit hull: bounds-only wakeup.
            // Positive encodings rank with the sign bit set.
            const WordValue sign = WordValue(1) << (width - 1);
            if (Gecode::Word::WordView(clone->y)
                    .narrow_rank_range(*clone, (one + 2) | sign,
                                       (one + 14) | sign) !=
                Gecode::Word::ME_WORD_BND)
              return false;
            if (clone->status() == SS_FAILED ||
                clone->x.minimum() <= old_minimum ||
                root.x.minimum() != old_minimum)
              return false;
            std::set<Tuple> expected;
            for (WordValue a = one + 1; a <= one + 31; ++a)
              for (WordValue b = one + 2; b <= one + 14; ++b)
                for (int flag : {0, 16})
                  if (finite_tuple(f, a, b, Policy::RNE, flag))
                    expected.insert({a, b, static_cast<WordValue>(flag)});
            if (solutions(*clone) != expected)
              return false;
            // Status 1 is an interior value of {0,1,16}; removing it excludes
            // the negative-invalid case without changing the status bounds.
            RootSpace status(f);
            dom(status, status.x, one, one | sign | 1);
            fp_sqrt(status, status.x, status.y, status.flags);
            if (status.status() == SS_FAILED || !status.flags.in(1))
              return false;
            const WordValue previous = status.x.hi();
            rel(status, status.flags, IRT_NQ, 1);
            if (status.status() == SS_FAILED ||
                status.x.hi() == previous ||
                (status.x.hi() & sign))
              return false;
            RootSpace smoke(f);
            fp_sqrt(smoke, smoke.x, smoke.y, smoke.flags);
            if (smoke.status() == SS_FAILED)
              return false;
          }
          RootSpace tiny(WDT_E2M1);
          dom(tiny, tiny.x, WordValArgs{0, 1, 2, 3, 4, 6});
          fp_sqrt(tiny, tiny.x, tiny.y, tiny.flags);
          if (tiny.status() == SS_FAILED)
            return false;
          std::unique_ptr<RootSpace> clone(
              static_cast<RootSpace*>(tiny.clone()));
          // Only source 0.5 has root 0.5, so x loses that interior hole.
          rel(*clone, clone->y, WRT_NQ, WordValue(1));
          if (clone->status() == SS_FAILED || clone->x.in(1) ||
              !tiny.x.in(1))
            return false;
          return true;
        }
      } lifecycle;

      // Propagation from random partial domains keeps every tuple that the
      // oracle supports, and search finds exactly those tuples.
      class TinyRoot : public Base {
        // A nonempty random subset of the values below count.
        std::set<WordValue> random_values(WordValue count) {
          std::set<WordValue> values;
          const unsigned int sparsity = 1U + _rand(6U);
          while (values.empty())
            for (WordValue v = 0; v < count; ++v)
              if (_rand(sparsity) == 0)
                values.insert(v);
          return values;
        }
        bool keeps_supported(WordDomainType f, WordValueSetOption value_set,
                             const Policy& p) {
          const WordValue count = WordValue(1) << float_word_width(f);
          RootSpace root(f, value_set);
          const std::set<WordValue> sources = random_values(count),
                                    results = random_values(count),
                                    statuses = random_values(32);
          dom(root, root.x, WordValArgs(sources.begin(), sources.end()));
          dom(root, root.y, WordValArgs(results.begin(), results.end()));
          IntArgs flags;
          for (WordValue s : statuses)
            flags << static_cast<int>(s);
          dom(root, root.flags, IntSet(flags));
          std::set<Tuple> expected;
          for (WordValue a : sources) {
            const Expected e = tiny_expected(f, a, p);
            if (results.count(e.result) &&
                statuses.count(static_cast<WordValue>(e.flags)))
              expected.insert({a, e.result, static_cast<WordValue>(e.flags)});
          }
          fp_sqrt(root, root.x, root.y, root.flags, p);
          if (root.status() == SS_FAILED)
            return expected.empty();
          for (const Tuple& t : expected)
            if (!root.x.in(t[0]) || !root.y.in(t[1]) ||
                !root.flags.in(static_cast<int>(t[2])))
              return false;
          return solutions(root) == expected;
        }

      public:
        TinyRoot() : Base("Word::Float::Sqrt::TinyRoot") {}
        bool run() override {
          const Policy flush{Policy::RUP, Policy::SignedZero,
                             Policy::SignedZero, Policy::Format,
                             Policy::Positive, Policy::Canonical};
          for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
            for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN})
              for (int trial = 0; trial < 60; ++trial)
                if (!keeps_supported(f, value_set,
                                     trial % 2 == 0 ? Policy() : flush))
                  return false;
          return true;
        }
      } tiny_root;

      // Square roots on float-typed words, whose bounds are in IEEE total
      // order, over negative-only and cross-zero windows. Wide windows within
      // one sign half also run on unsigned-typed words.
      class FloatTyped : public Base {
        // The result and status of an assigned wide source: a negative
        // nonzero non-NaN source gives the default NaN, and a positive finite
        // one a tuple that the dyadic oracle accepts.
        static bool wide_expected(WordDomainType f, WordValue source,
                                  const Policy& p, Expected& expected) {
          RootSpace s(f);
          dom(s, s.x, source);
          fp_sqrt(s, s.x, s.y, s.flags, p);
          if (s.status() == SS_FAILED || !s.y.assigned())
            return false;
          expected = {s.y.val(), s.flags.val()};
          const WordValue sign = WordValue(1) << (float_word_width(f) - 1);
          const WordValue infinity = f == WDT_FP32 ? 0x7f800000 : 0x7f80;
          const WordValue quiet = f == WDT_FP32 ? 0x400000 : 0x40;
          const WordValue normal = f == WDT_FP32 ? 0x00800000 : 0x0080;
          const WordValue magnitude = source & (sign - 1);
          const bool flushed =
              p.input_subnormals == Policy::SignedZero && magnitude < normal;
          if (magnitude == 0 || magnitude > infinity || flushed)
            return true;
          if ((source & sign) != 0)
            return expected.result == (infinity | quiet) && expected.flags == 1;
          return magnitude == infinity ||
                 finite_tuple(f, source, expected.result, p.rounding,
                              expected.flags);
        }
        static bool check_window(WordDomainType f, WordValue minimum,
                                 WordValue maximum, const Policy& p) {
          const unsigned int width = float_word_width(f);
          RootSpace root(f);
          root.x = float_window(root, f, minimum, maximum);
          std::set<Tuple> expected;
          for (WordValue a : float_order_range(width, minimum, maximum)) {
            Expected e;
            if (width <= 8)
              e = tiny_expected(f, a, p);
            else if (!wide_expected(f, a, p, e))
              return false;
            expected.insert({a, e.result, static_cast<WordValue>(e.flags)});
          }
          fp_sqrt(root, root.x, root.y, root.flags, p);
          if (root.status() != SS_FAILED && solutions(root) == expected)
            return true;
          if (opt.log)
            olog << "float-typed sqrt " << f << " window " << minimum << ".."
                 << maximum << " mode " << p.rounding << '\n';
          return false;
        }

      public:
        FloatTyped() : Base("Word::Float::Sqrt::FloatTyped") {}
        bool run() override {
          for (WordDomainType f : {WDT_BF16, WDT_FP32, WDT_E4M3FN, WDT_E2M1}) {
            const unsigned int width = float_word_width(f);
            const WordValue sign = WordValue(1) << (width - 1);
            const WordValue one = f == WDT_FP32     ? 0x3f800000
                                  : f == WDT_BF16   ? 0x3f80
                                  : f == WDT_E4M3FN ? 0x38
                                                    : 2;
            // Total-order windows: negative subnormals, negative normals
            // around -1, and windows across zero, the last of them from -1 to
            // 1 for a tiny format only.
            const WordValue windows[][2] = {{sign | 3, sign | 1},
                                            {sign | (one + 2), sign | (one - 1)},
                                            {sign | 2, 2},
                                            {sign | (one + 1), one + 1}};
            for (const auto& window : windows) {
              if (width > 8 && window[1] == one + 1)
                continue;
              for (int rounding = 0; rounding < 4; ++rounding)
                for (bool flush : {false, true}) {
                  const Policy p{static_cast<Policy::Rounding>(rounding),
                                 flush ? Policy::SignedZero : Policy::Preserve};
                  if (!check_window(f, window[0], window[1], p))
                    return false;
                }
            }
          }
          return true;
        }
      } float_typed;

      class Errors : public Base {
      public:
        Errors() : Base("Word::Float::Sqrt::Errors") {}
        bool run() override {
          // Binary16 and mixed formats reject at posting.
          for (int invalid = 0; invalid < 2; ++invalid) {
            bool rejected = false;
            try {
              RootSpace s(invalid == 0 ? WDT_BINARY16 : WDT_FP32);
              if (invalid == 1)
                s.y = variable(s, WDT_BF16);
              fp_sqrt(s, s.x, s.y);
            } catch (const Gecode::Exception&) {
              rejected = true;
            }
            if (!rejected)
              return false;
          }
          return true;
        }
      } errors;
    } // namespace FloatSqrt
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
