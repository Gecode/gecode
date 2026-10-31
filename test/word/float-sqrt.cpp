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

      FloatWordVar variable(Space& home, FloatWordFormat format,
                            WordDomainType kind = WDT_UNSIGNED) {
        FloatWordVar x(home, format, kind);
        return float_word_width(format) <= 8
                   ? FloatWordVar(
                         home, format, x.bits(),
                         IntVar(home, 0, (1 << float_word_width(format)) - 1))
                   : x;
      }
      class RootSpace : public Space {
      public:
        FloatWordVar x, y;
        IntVar flags;
        RootSpace(FloatWordFormat format, WordDomainType xkind = WDT_UNSIGNED,
                  WordDomainType ykind = WDT_UNSIGNED)
            : x(variable(*this, format, xkind)),
              y(variable(*this, format, ykind)), flags(*this, 0, 31) {}
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
      long long tiny_units(WordValue code, FloatWordFormat f) {
        if (f == FWF_E2M1) {
          const int halves[] = {0, 1, 2, 3, 4, 6, 8, 12};
          return halves[code];
        }
        return code < 8 ? code : (8 + code % 8) << (code / 8 - 1);
      }
      Expected tiny_expected(FloatWordFormat f, WordValue code,
                             const Policy& p) {
        const WordValue sign = f == FWF_E2M1 ? 8 : 128;
        WordValue magnitude = code % sign;
        const WordValue normal = f == FWF_E2M1 ? 2 : 8;
        if (f == FWF_E4M3FN && magnitude == 127)
          return {p.nan == Policy::Canonical ? 127 : code, 0};
        if (p.input_subnormals == Policy::SignedZero && magnitude < normal)
          magnitude = 0;
        if (!magnitude)
          return {p.zeros == Policy::Positive ? 0 : code & sign, 0};
        if (code & sign)
          return {f == FWF_E2M1 ? 7U : 127U, 1};
        const long long scale = f == FWF_E2M1 ? 2 : 512;
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
        int flags = exact ? 0 : f == FWF_E2M1 && magnitude == 1 ? 24 : 16;
        if (p.output_subnormals == Policy::SignedZero && output < normal) {
          output = 0;
          flags |= 24;
        }
        return {output, flags};
      }
      bool check_concrete(FloatWordFormat f, WordValue source, const Policy& p,
                          Expected expected, bool wrong = false) {
        RootSpace s(f);
        dom(s, s.x.bits(), source);
        fp_sqrt(s, p, s.x, s.y, s.flags);
        if (s.status() == SS_FAILED || !s.y.bits().assigned() ||
            s.y.bits().val() != expected.result || !s.flags.assigned() ||
            s.flags.val() != expected.flags)
          return false;
        if (wrong) {
          for (bool wrong_result : {false, true}) {
            RootSpace rejected(f);
            dom(rejected, rejected.x.bits(), source);
            dom(rejected, rejected.y.bits(),
                expected.result ^ (wrong_result ? 1 : 0));
            rel(rejected, rejected.flags, IRT_EQ,
                expected.flags ^ (wrong_result ? 0 : 16));
            fp_sqrt(rejected, p, rejected.x, rejected.y, rejected.flags);
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
          for (FloatWordFormat f : {FWF_E2M1, FWF_E4M3FN})
            for (int rounding = 0; rounding < 4; ++rounding)
              for (int input = 0; input < 2; ++input)
                for (int output = 0; output < 2; ++output)
                  for (int zeros = 0; zeros < 2; ++zeros)
                    for (int nan = 0; nan < (f == FWF_E4M3FN ? 2 : 1); ++nan)
                      for (int tininess = 0; tininess < (f == FWF_E2M1 ? 2 : 1);
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
          const Policy ignored{Policy::RNE, Policy::Preserve,
                               Policy::Preserve, Policy::SaturateFinite,
                               Policy::Signed, Policy::OperandFirst,
                               Policy::BeforeRounding, Policy::Signaling};
          return check_concrete(FWF_E2M1, 1, ignored, {1, 24}, true) &&
                 check_concrete(FWF_E4M3FN, 0xff, ignored, {0xff, 0}, true);
        }
      } tiny;

      // Independent dyadic inequality oracle: compare individual binary digits
      // across the exponent range, without aligned shifts or root computation.
      struct Rational {
        WordValue numerator;
        int power;
      };
      Rational value(FloatWordFormat f, WordValue code) {
        const int fraction = f == FWF_FP32 ? 23 : 7;
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
      Rational squared_midpoint(FloatWordFormat f, WordValue a, WordValue b) {
        const Rational av = value(f, a), bv = value(f, b);
        const int power = std::min(av.power, bv.power);
        const WordValue sum =
            av.numerator * (WordValue(1) << (av.power - power)) +
            bv.numerator * (WordValue(1) << (bv.power - power));
        return {sum * sum, 2 * (power - 1)};
      }
      bool finite_tuple(FloatWordFormat f, WordValue source, WordValue result,
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
          for (FloatWordFormat f : {FWF_BF16, FWF_FP32}) {
            const WordValue one = f == FWF_FP32 ? 0x3f800000 : 0x3f80;
            const WordValue normal = f == FWF_FP32 ? 0x00800000 : 0x0080;
            const WordValue infinity = f == FWF_FP32 ? 0x7f800000 : 0x7f80;
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
                dom(s, s.x.bits(), input);
                fp_sqrt(s, p, s.x, s.y, s.flags);
                if (s.status() == SS_FAILED || !s.y.bits().assigned() ||
                    !s.flags.assigned() ||
                    !finite_tuple(f, input, s.y.bits().val(), mode,
                                  s.flags.val()) ||
                    !check_concrete(f, input, p,
                                    {s.y.bits().val(), s.flags.val()}, true))
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
              dom(s, s.x.bits(), fixed);
              fp_sqrt(s, Policy{mode}, s.x, s.x, s.flags);
              if (s.status() == SS_FAILED || s.flags.val() != 16)
                return false;
            }
          }
          return true;
        }
      } wide;
      using Tuple = std::array<WordValue, 3>;
      std::set<Tuple> solutions(RootSpace& root, bool status = true) {
        branch(root, WordVarArgs{root.x.bits(), root.y.bits()}, WORD_VAL_LSB());
        if (status)
          branch(root, root.flags, INT_VAL_MIN());
        DFS<RootSpace> search(&root);
        std::set<Tuple> found;
        while (std::unique_ptr<RootSpace> s{search.next()})
          found.insert({s->x.bits().val(), s->y.bits().val(),
                        status ? static_cast<WordValue>(s->flags.val()) : 0});
        return found;
      }
      FloatWordVar window(Space& home, FloatWordFormat f, WordDomainType kind,
                          WordValue first, WordValue last, bool sparse) {
        const WordValue free = sparse ? 0x15 : 0x1f;
        const WordValue base = first & ~WordValue(31);
        const WordValue lo = base, hi = base | free;
        return FloatWordVar(f, kind == WDT_CUBE
                                   ? WordVar(home, float_word_width(f), lo, hi)
                                   : WordVar(home, float_word_width(f), lo, hi,
                                             kind, first, last));
      }
      class Symbolic : public Base {
      public:
        Symbolic() : Base("Word::Float::Sqrt::Symbolic") {}
        bool run() override {
          const WordDomainType kinds[] = {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED};
          for (FloatWordFormat f : {FWF_BF16, FWF_FP32}) {
            const WordValue one = f == FWF_FP32 ? 0x3f800000 : 0x3f80;
            for (WordDomainType xkind : kinds)
              for (WordDomainType ykind : kinds)
                for (bool sparse : {false, true})
                  for (int rounding = 0; rounding < 4; ++rounding) {
                    const auto mode = static_cast<Policy::Rounding>(rounding);
                    RootSpace root(f);
                    root.x = window(root, f, xkind, one + 2, one + 29, sparse);
                    root.y = window(root, f, ykind, one + 3, one + 9, sparse);
                    std::set<Tuple> expected;
                    for (WordValue a = one; a < one + 32; ++a)
                      for (WordValue b = one; b < one + 32; ++b)
                        if (root.x.bits().in(a) && root.y.bits().in(b))
                          for (int flag : {0, 16})
                            if (finite_tuple(f, a, b, mode, flag))
                              expected.insert(
                                  {a, b, static_cast<WordValue>(flag)});
                    fp_sqrt(root, Policy{mode}, root.x,
                            root.y, root.flags);
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
            inverse.x = FloatWordVar(f, WordVar(inverse, float_word_width(f),
                                                WDT_UNSIGNED, one, one + 31));
            inverse.y =
                FloatWordVar(f, WordVar(inverse, float_word_width(f),
                                        WDT_UNSIGNED, one + 3, one + 5));
            fp_sqrt(inverse, Policy(), inverse.x, inverse.y, inverse.flags);
            if (inverse.status() == SS_FAILED ||
                inverse.x.bits().minimum() <= one ||
                inverse.x.bits().maximum() >= one + 31 ||
                inverse.x.bits().assigned())
              return false;
            // Real-valued fixed-point rewriting would wrongly remove these.
            for (Policy::Rounding mode : {Policy::RNE, Policy::RUP}) {
              RootSpace alias(f);
              alias.x = FloatWordVar(f, WordVar(alias, float_word_width(f),
                                                WDT_SIGNED, one - 2, one + 2));
              alias.y = alias.x;
              fp_sqrt(alias, Policy{mode}, alias.x, alias.y,
                      alias.flags);
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
          // through different Word orders and signaling/status correlation.
          for (FloatWordFormat f : {FWF_BF16, FWF_FP32}) {
            const unsigned int width = float_word_width(f);
            const WordValue infinity = f == FWF_FP32 ? 0x7f800000 : 0x7f80;
            const WordValue quiet = f == FWF_FP32 ? 0x400000 : 0x40;
            const WordValue sign = WordValue(1) << (width - 1);
            for (WordDomainType xkind : kinds)
              for (WordDomainType ykind : kinds) {
                RootSpace root(f);
                root.x = FloatWordVar(f, WordVar(root, width, infinity | 1,
                                                 sign | infinity | quiet | 3));
                if (xkind != WDT_CUBE)
                  root.x = FloatWordVar(
                      f, WordVar(root, width, infinity | 1,
                                 sign | infinity | quiet | 3, xkind,
                                 xkind == WDT_SIGNED ? sign | infinity | 1
                                                     : infinity | 1,
                                 xkind == WDT_SIGNED
                                     ? infinity | quiet | 3
                                     : sign | infinity | quiet | 3));
                root.y = FloatWordVar(f, WordVar(root, width, ykind));
                dom(root, root.y.bits(), sign | infinity | quiet | 3);
                fp_sqrt(root, Policy(), root.x, root.y, root.flags);
                const std::set<Tuple> expected{
                    {sign | infinity | 3, sign | infinity | quiet | 3, 1},
                    {sign | infinity | quiet | 3, sign | infinity | quiet | 3,
                     0}};
                if (solutions(root) != expected)
                  return false;
              }
          }
          for (FloatWordFormat f : {FWF_E2M1, FWF_E4M3FN})
            for (int alias = 0; alias < 3; ++alias) {
              RootSpace root(f);
              if (alias == 0)
                root.y = root.x;
              if (alias == 1)
                root.flags = root.x.encoding();
              if (alias == 2)
                root.flags = root.y.encoding();
              std::set<Tuple> expected;
              for (WordValue a = 0; a < (WordValue(1) << float_word_width(f));
                   ++a) {
                const Expected e = tiny_expected(f, a, Policy());
                if ((alias != 0 || a == e.result) &&
                    (alias != 1 || a == static_cast<WordValue>(e.flags)) &&
                    (alias != 2 || e.result == static_cast<WordValue>(e.flags)))
                  expected.insert(
                      {a, e.result, static_cast<WordValue>(e.flags)});
              }
              fp_sqrt(root, Policy(), root.x, root.y, root.flags);
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
          for (FloatWordFormat f : {FWF_BF16, FWF_FP32}) {
            const unsigned int width = float_word_width(f);
            const WordValue one = f == FWF_FP32 ? 0x3f800000 : 0x3f80;
            RootSpace root(f);
            root.x = FloatWordVar(
                f, WordVar(root, width, WDT_UNSIGNED, one + 1, one + 31));
            root.y = FloatWordVar(
                f, WordVar(root, width, WDT_SIGNED, one, one + 31));
            fp_sqrt(root, Policy(), root.x, root.y, root.flags);
            if (root.status() == SS_FAILED)
              return false;
            const WordValue old_minimum = root.x.bits().minimum();
            std::unique_ptr<RootSpace> clone(
                static_cast<RootSpace*>(root.clone()));
            // 0..15 -> 2..14 retains the same bit hull: bounds-only wakeup.
            const WordValue sign = WordValue(1) << (width - 1);
            if (Gecode::Word::WordView(clone->y.bits())
                    .narrow_rank_range(*clone, (one + 2) ^ sign,
                                       (one + 14) ^ sign) !=
                Gecode::Word::ME_WORD_BND)
              return false;
            if (clone->status() == SS_FAILED ||
                clone->x.bits().minimum() <= old_minimum ||
                root.x.bits().minimum() != old_minimum)
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
            status.x = FloatWordVar(
                f, WordVar(status, width, one,
                           one | (WordValue(1) << (width - 1)) | 1));
            fp_sqrt(status, Policy(), status.x, status.y, status.flags);
            if (status.status() == SS_FAILED || !status.flags.in(1))
              return false;
            const WordValue previous = status.x.bits().hi();
            rel(status, status.flags, IRT_NQ, 1);
            if (status.status() == SS_FAILED ||
                status.x.bits().hi() == previous ||
                (status.x.bits().hi() & (WordValue(1) << (width - 1))))
              return false;
            RootSpace smoke(f, WDT_CUBE, WDT_SIGNED);
            fp_sqrt(smoke, Policy(), smoke.x, smoke.y, smoke.flags);
            if (smoke.status() == SS_FAILED)
              return false;
          }
          RootSpace tiny(FWF_E2M1);
          const int inputs[] = {0, 1, 2, 3, 4, 6};
          dom(tiny, tiny.x.encoding(), IntSet(inputs, 6));
          fp_sqrt(tiny, Policy(), tiny.x, tiny.y, tiny.flags);
          if (tiny.status() == SS_FAILED)
            return false;
          std::unique_ptr<RootSpace> clone(
              static_cast<RootSpace*>(tiny.clone()));
          rel(*clone, clone->y.encoding(), IRT_NQ, 1);
          if (clone->status() == SS_FAILED || clone->x.encoding().in(1) ||
              !tiny.x.encoding().in(1))
            return false;
          return true;
        }
      } lifecycle;

      class Errors : public Base {
      public:
        Errors() : Base("Word::Float::Sqrt::Errors") {}
        bool run() override {
          // Binary16, mixed formats and missing tiny bindings reject at
          // posting.
          for (int invalid = 0; invalid < 3; ++invalid) {
            bool rejected = false;
            try {
              RootSpace s(invalid == 0 ? FWF_BINARY16 : FWF_FP32);
              if (invalid == 1)
                s.y = variable(s, FWF_BF16);
              if (invalid == 2) {
                FloatWordVar missing(s, FWF_E2M1);
                fp_sqrt(s, Policy(), missing, missing);
              } else {
                fp_sqrt(s, Policy(), s.x, s.y);
              }
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
