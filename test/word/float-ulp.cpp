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
#include <gecode/float-word.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <gecode/word/arithmetic/global-integer.hpp>

#include <array>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

namespace Test {
  namespace Word {
    namespace Float {
      namespace ULP {
        using namespace Gecode;

        const WordDomainType formats[] = {WDT_FP32, WDT_BF16, WDT_BINARY16,
                                           WDT_E4M3FN, WDT_E2M1};
        struct Format {
          int width, fraction, bias;
          WordValue maximum;
        };
        Format format(WordDomainType f) {
          switch (f) {
          case WDT_FP32:
            return {32, 23, 127, 0x7f7fffff};
          case WDT_BF16:
            return {16, 7, 127, 0x7f7f};
          case WDT_BINARY16:
            return {16, 10, 15, 0x7bff};
          case WDT_E4M3FN:
            return {8, 3, 7, 0x7e};
          case WDT_E2M1:
            return {4, 1, 1, 7};
          default:
            GECODE_NEVER;
            return {};
          }
        }
        struct Dyadic {
          bool finite, negative;
          uint64_t significand;
          int exponent;
        };
        // Oracle: reduce the exact integer significand and power of two. It
        // neither embeds through FP32 nor uses the production decomposition or
        // host floats.
        Dyadic decode(WordDomainType f, WordValue code) {
          Format s = format(f);
          WordValue m = code % (WordValue(1) << (s.width - 1));
          unsigned int e = static_cast<unsigned int>(m >> s.fraction);
          uint64_t significand = m % (uint64_t(1) << s.fraction);
          if (e)
            significand += uint64_t(1) << s.fraction;
          int exponent = (e ? static_cast<int>(e) : 1) - s.bias - s.fraction;
          while (significand && (significand % 2 == 0)) {
            significand /= 2;
            ++exponent;
          }
          return {m <= s.maximum, bool(code >> (s.width - 1)), significand,
                  exponent};
        }
        int top_bit(uint64_t n) {
          int bit = -1;
          do {
            ++bit;
          } while (n >>= 1);
          return bit;
        }
        int spacing(WordDomainType reference, Dyadic x) {
          Format r = format(reference);
          int binade =
              x.significand ? top_bit(x.significand) + x.exponent : 1 - r.bias;
          return std::max(binade, 1 - r.bias) - r.fraction;
        }
        bool rank(WordDomainType reference, Dyadic x, int64_t& result) {
          if (!x.finite)
            return false;
          if (x.significand == 0) {
            result = 0;
            return true;
          }
          Format r = format(reference);
          int binade = top_bit(x.significand) + x.exponent;
          int quantum = std::max(binade, 1 - r.bias) - r.fraction;
          // The significand is odd: a negative shift cannot be exact.
          int shift = x.exponent - quantum;
          if (shift < 0 || shift > r.fraction ||
              binade > static_cast<int>(r.maximum >> r.fraction) - r.bias)
            return false;
          uint64_t scaled = x.significand << shift;
          uint64_t m = scaled;
          if (binade >= 1 - r.bias)
            m = (uint64_t(binade + r.bias) << r.fraction) + scaled -
                (uint64_t(1) << r.fraction);
          if (m > r.maximum)
            return false;
          result =
              x.negative ? -static_cast<int64_t>(m) : static_cast<int64_t>(m);
          return true;
        }
        bool steps(WordDomainType reference, WordDomainType fa, WordValue a,
                   WordDomainType fb, WordValue b, WordValue& result) {
          int64_t ar, br;
          if (!rank(reference, decode(fa, a), ar) ||
              !rank(reference, decode(fb, b), br))
            return false;
          result = static_cast<WordValue>(ar >= br ? ar - br : br - ar);
          return true;
        }
        WordVar operand(Space& home, WordDomainType f,
                        WordValueSetOption value_set = WVS_EXACT) {
          return WordVar(home, f,
                         float_word_width(f) <= 8 ? value_set : WVS_NONE);
        }
        /// The values of a word of width at most 8
        std::set<WordValue> values(const WordVar& x) {
          std::set<WordValue> found;
          for (WordValue v = 0; v < (WordValue(1) << x.width()); ++v)
            if (x.in(v))
              found.insert(v);
          return found;
        }

        enum class Observation { Spacing, Steps };

        class Model : public Space {
        public:
          WordVar a, b;
          IntVar q;
          WordVar d;
          Model(WordDomainType reference, WordDomainType fa,
                WordDomainType fb, Observation observation,
                WordDomainType distance = WDT_UNSIGNED,
                IntPropLevel ipl = IPL_DEF,
                WordValueSetOption value_set = WVS_EXACT)
              : a(operand(*this, fa, value_set)),
                b(operand(*this, fb, value_set)),
                q(*this, -200, 200),
                d(*this, float_word_width(reference), distance) {
            if (observation == Observation::Spacing)
              ulp_spacing(*this, reference, a, q);
            else
              ulp_steps(*this, reference, a, b, d, ipl);
          }
          Model(Model& s) : Space(s) {
            a.update(*this, s.a);
            b.update(*this, s.b);
            q.update(*this, s.q);
            d.update(*this, s.d);
          }
          Space* copy(void) override { return new Model(*this); }
          void inputs(Observation observation) {
            branch(*this, WordVarArgs{a}, WORD_VAL_LSB());
            if (observation == Observation::Steps)
              branch(*this, WordVarArgs{b}, WORD_VAL_LSB());
          }
        };

        class Tiny : public Base {
        public:
          Tiny(void) : Base("Word::Float::ULP::Tiny") {}
          bool run(void) override {
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
              if (!complete(ipl))
                return false;
            return true;
          }
          bool complete(IntPropLevel ipl) {
            for (WordDomainType reference : formats)
              for (WordDomainType fa : {WDT_E2M1, WDT_E4M3FN}) {
                Model local(reference, fa, fa, Observation::Spacing);
                local.inputs(Observation::Spacing);
                DFS<Model> search(&local);
                std::set<WordValue> seen;
                while (std::unique_ptr<Model> s{search.next()}) {
                  WordValue a = s->a.val();
                  Dyadic x = decode(fa, a);
                  if (!x.finite || !s->q.assigned() ||
                      s->q.val() != spacing(reference, x) ||
                      !seen.insert(a).second)
                    return false;
                }
                for (WordValue a = 0; a < (WordValue(1) << format(fa).width);
                     ++a)
                  if ((seen.count(a) != 0) != decode(fa, a).finite)
                    return false;
                for (WordDomainType fb : {WDT_E2M1, WDT_E4M3FN}) {
                  Model root(reference, fa, fb, Observation::Steps,
                             WDT_UNSIGNED, ipl);
                  root.inputs(Observation::Steps);
                  DFS<Model> pairs(&root);
                  std::set<std::pair<WordValue, WordValue>> projected;
                  while (std::unique_ptr<Model> s{pairs.next()}) {
                    WordValue a = s->a.val(), b = s->b.val(),
                              expected;
                    if (!steps(reference, fa, a, fb, b, expected) ||
                        !s->d.assigned() || s->d.val() != expected ||
                        !projected.emplace(a, b).second)
                      return false;
                  }
                  for (WordValue a = 0; a < (WordValue(1) << format(fa).width);
                       ++a)
                    for (WordValue b = 0;
                         b < (WordValue(1) << format(fb).width); ++b) {
                      WordValue expected;
                      if ((projected.count({a, b}) != 0) !=
                          steps(reference, fa, a, fb, b, expected))
                        return false;
                    }
                }
              }
            return true;
          }
        };

        std::set<WordValue> boundaries(WordDomainType f) {
          Format s = format(f);
          std::set<WordValue> result{0,
                                     1,
                                     (WordValue(1) << s.fraction) - 1,
                                     s.maximum,
                                     s.maximum + 1,
                                     (WordValue(1) << (s.width - 1)) - 1};
          for (int bit = 1; bit < s.fraction; ++bit) {
            WordValue power = WordValue(1) << bit;
            result.insert(power - 1);
            result.insert(power);
            result.insert(power + 1);
          }
          for (WordValue e = 1; e <= (s.maximum >> s.fraction); ++e) {
            WordValue power = e << s.fraction;
            result.insert(power - 1);
            result.insert(power);
            result.insert(power + 1);
          }
          const std::set<WordValue> positive = result;
          for (WordValue code : positive)
            result.insert(code | (WordValue(1) << (s.width - 1)));
          return result;
        }

        class Boundaries : public Base {
        public:
          Boundaries(void) : Base("Word::Float::ULP::Boundaries") {}
          bool run(void) override {
            for (WordDomainType reference : formats)
              for (WordDomainType f : {WDT_FP32, WDT_BF16, WDT_BINARY16}) {
                Model local(reference, f, f, Observation::Spacing),
                    distance(reference, f, f, Observation::Steps);
                if (local.status() == SS_FAILED ||
                    distance.status() == SS_FAILED)
                  return false;
                for (WordValue code : boundaries(f)) {
                  std::unique_ptr<Model> s(static_cast<Model*>(local.clone()));
                  dom(*s, s->a, code);
                  Dyadic x = decode(f, code);
                  bool failed = s->status() == SS_FAILED;
                  if (failed != !x.finite ||
                      (!failed && (!s->q.assigned() ||
                                   s->q.val() != spacing(reference, x))))
                    return false;
                  std::unique_ptr<Model> d(
                      static_cast<Model*>(distance.clone()));
                  dom(*d, d->a, code);
                  dom(*d, d->b, 0);
                  WordValue expected;
                  bool valid = steps(reference, f, code, f, 0, expected);
                  failed = d->status() == SS_FAILED;
                  if (failed != !valid ||
                      (!failed && (!d->d.assigned() || d->d.val() != expected)))
                    return false;
                }
              }
            // Exercise both directions of every cross-format pair, including
            // precision, subnormal normalization and range boundaries.
            for (WordDomainType reference : formats)
              for (WordDomainType fa : formats)
                for (WordDomainType fb : formats) {
                  Model root(reference, fa, fb, Observation::Steps);
                  if (root.status() == SS_FAILED)
                    return false;
                  const Format a = format(fa), b = format(fb);
                  const WordValue ac[] = {0, 1, WordValue(a.bias) << a.fraction,
                                          a.maximum};
                  const WordValue bc[] = {WordValue(1) << (b.width - 1), 1,
                                          (WordValue(b.bias) << b.fraction) + 1,
                                          b.maximum};
                  for (int i = 0; i < 4; ++i) {
                    std::unique_ptr<Model> s(static_cast<Model*>(root.clone()));
                    dom(*s, s->a, ac[i]);
                    dom(*s, s->b, bc[i]);
                    WordValue expected;
                    bool valid =
                        steps(reference, fa, ac[i], fb, bc[i], expected);
                    bool failed = s->status() == SS_FAILED;
                    if (failed != !valid ||
                        (!failed &&
                         (!s->d.assigned() || s->d.val() != expected)))
                      return false;
                  }
                }
            // Exercise the private finite embedding for every tiny encoding,
            // with a wide endpoint so the tiny-pair table cannot supply it.
            for (WordDomainType reference : formats)
              for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN}) {
                Model root(reference, f, WDT_FP32, Observation::Steps);
                dom(root, root.b, 0);
                if (root.status() == SS_FAILED)
                  return false;
                for (WordValue code = 0;
                     code < (WordValue(1) << format(f).width); ++code) {
                  std::unique_ptr<Model> s(static_cast<Model*>(root.clone()));
                  dom(*s, s->a, code);
                  WordValue expected;
                  bool valid = steps(reference, f, code, WDT_FP32, 0, expected);
                  bool failed = s->status() == SS_FAILED;
                  if (failed != !valid ||
                      (!failed && (!s->d.assigned() || s->d.val() != expected)))
                    return false;
                }
              }
            Model precision(WDT_BF16, WDT_FP32, WDT_FP32, Observation::Steps);
            dom(precision, precision.a, 0x3f800000);
            dom(precision, precision.b, 0x3f808000); // 1 + 2^-8
            if (precision.status() != SS_FAILED)
              return false;
            for (WordDomainType f : formats)
              for (WordDomainType distance :
                   {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
                Model s(f, f, f, Observation::Steps, distance);
                WordValue maximum = format(f).maximum;
                dom(s, s.a, maximum);
                dom(s, s.b,
                    maximum | (WordValue(1) << (format(f).width - 1)));
                if (s.status() == SS_FAILED || !s.d.assigned() ||
                    s.d.val() != 2 * maximum)
                  return false;
              }
            // These scales must not clamp to the target's largest finite
            // binade.
            for (const auto& c :
                 {std::make_pair(WDT_E4M3FN, WordValue(0x44000000)),
                  std::make_pair(WDT_E2M1, WordValue(0x41000000))}) {
              Model s(c.first, WDT_FP32, WDT_FP32, Observation::Spacing);
              dom(s, s.a, c.second);
              if (s.status() == SS_FAILED || !s.q.assigned() ||
                  s.q.val() != (c.first == WDT_E4M3FN ? 6 : 2))
                return false;
            }
            return true;
          }
        };

        class Lifecycle : public Base {
          static bool inverse_and_holes(void) {
            for (WordDomainType distance :
                 {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
              for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN})
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                  Model s(f, f, f, Observation::Steps, distance, ipl);
                  dom(s, s.a, WordValArgs{2, 3, 4});
                  dom(s, s.b, WordValArgs{3, 4, 5});
                  dom(s, s.d, 1);
                  ulp_spacing(s, f, s.a, s.q);
                  if (s.status() == SS_FAILED)
                    return false;
                  std::unique_ptr<Model> clone(static_cast<Model*>(s.clone()));
                  // At IPL_DOM, interior value 3 disappears after both
                  // observations have propagated.
                  rel(*clone, clone->a, WRT_NQ, WordValue(3));
                  rel(*clone, clone->q, IRT_EQ, f == WDT_E2M1 ? -1 : -9);
                  if (clone->status() == SS_FAILED || !s.a.in(3))
                    return false;
                  if (ipl == IPL_DOM &&
                      (clone->b.in(4) ||
                       (f == WDT_E2M1 && (!clone->a.assigned() ||
                                          clone->a.val() != 2 ||
                                          !clone->b.assigned() ||
                                          clone->b.val() != 3))))
                    return false;
                  s.inputs(Observation::Steps);
                  DFS<Model> search(&s);
                  std::set<std::pair<WordValue, WordValue>> seen;
                  while (std::unique_ptr<Model> solution{search.next()}) {
                    if (!solution->d.assigned() || solution->d.val() != 1 ||
                        !solution->q.assigned())
                      return false;
                    seen.emplace(solution->a.val(),
                                 solution->b.val());
                  }
                  if (seen != std::set<std::pair<WordValue, WordValue>>{
                                  {2, 3}, {3, 4}, {4, 3}, {4, 5}})
                    return false;
                }
              Model large(WDT_FP32, WDT_E4M3FN, WDT_E2M1, Observation::Steps, distance);
              dom(large, large.a, 0xfe);
              dom(large, large.b, 7);
              WordValue span;
              if (!steps(WDT_FP32, WDT_E4M3FN, 0xfe, WDT_E2M1, 7, span) ||
                  span <= static_cast<WordValue>(Int::Limits::max) ||
                  large.status() == SS_FAILED || !large.d.assigned() ||
                  large.d.val() != span)
                return false;
              Model wide(WDT_BF16, WDT_BF16, WDT_BF16, Observation::Steps, distance);
              dom(wide, wide.a, 0x3f80);
              dom(wide, wide.d, 1);
              // A bounded inverse search should recover exactly the two
              // neighbors.
              rel(wide, wide.b, WRT_UGQ, 0x3f7e);
              rel(wide, wide.b, WRT_ULE, 0x3f82);
              wide.inputs(Observation::Steps);
              DFS<Model> inverse(&wide);
              std::set<WordValue> neighbors;
              while (std::unique_ptr<Model> solution{inverse.next()})
                neighbors.insert(solution->b.val());
              if (neighbors != std::set<WordValue>{0x3f7f, 0x3f81})
                return false;
            }
            Model local(WDT_BF16, WDT_FP32, WDT_FP32, Observation::Spacing);
            rel(local, local.q, IRT_EQ, -7);
            if (local.status() == SS_FAILED)
              return false;
            dom(local, local.a,
                0x40000000); // 2 has exponent -6, not -7.
            return local.status() == SS_FAILED;
          }
          static bool aliases_and_errors(void) {
            for (WordDomainType f : formats) {
              Model same(f, f, f, Observation::Spacing);
              ulp_steps(same, f, same.a, same.a, same.d);
              if (same.status() == SS_FAILED || !same.d.assigned() ||
                  same.d.val() != 0)
                return false;
              Model alias(f, f, f, Observation::Spacing);
              dom(alias, alias.b, 0);
              ulp_steps(alias, f, alias.a, alias.b, alias.a);
              dom(alias, alias.a, 1);
              if (alias.status() == SS_FAILED)
                return false;
              Model incorrect(f, f, f, Observation::Steps);
              dom(incorrect, incorrect.a, 0);
              dom(incorrect, incorrect.b, 0);
              dom(incorrect, incorrect.d, 1);
              if (incorrect.status() != SS_FAILED)
                return false;
            }
            // Equal raw bits with different 16-bit types are different values.
            Model shared(WDT_FP32, WDT_BF16, WDT_BINARY16, Observation::Spacing);
            ulp_steps(shared, WDT_FP32, shared.a,
                      reinterpret(shared, shared.a, WDT_BINARY16), shared.d);
            dom(shared, shared.a, 0x3c00);
            WordValue expected;
            if (!steps(WDT_FP32, WDT_BF16, 0x3c00, WDT_BINARY16, 0x3c00,
                       expected) ||
                shared.status() == SS_FAILED || !shared.d.assigned() ||
                shared.d.val() != expected)
              return false;
            Model same_unrepresentable(WDT_BF16, WDT_FP32, WDT_FP32, Observation::Spacing);
            ulp_steps(same_unrepresentable, WDT_BF16, same_unrepresentable.a,
                      same_unrepresentable.a, same_unrepresentable.d);
            dom(same_unrepresentable, same_unrepresentable.a, 0x3f808000);
            if (same_unrepresentable.status() != SS_FAILED)
              return false;
            Model missing(WDT_BF16, WDT_BF16, WDT_BF16, Observation::Spacing);
            int caught = 0;
            try {
              ulp_spacing(missing, static_cast<WordDomainType>(99), missing.a,
                          missing.q);
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              ulp_steps(missing, WDT_FP32, missing.a, missing.b, missing.d);
            } catch (const Gecode::Word::WidthMismatch&) {
              ++caught;
            }
            if (caught != 2)
              return false;
            // Tiny words without a value set.
            Model setless(WDT_BF16, WDT_E2M1, WDT_E2M1, Observation::Steps,
                          WDT_UNSIGNED, IPL_DEF, WVS_NONE);
            ulp_spacing(setless, WDT_BF16, setless.a, setless.q);
            dom(setless, setless.a, 3);
            dom(setless, setless.b, 13);
            return steps(WDT_BF16, WDT_E2M1, 3, WDT_E2M1, 13, expected) &&
                   setless.status() != SS_FAILED && setless.q.assigned() &&
                   setless.q.val() == spacing(WDT_BF16, decode(WDT_E2M1, 3)) &&
                   setless.d.assigned() && setless.d.val() == expected;
          }

        public:
          Lifecycle(void) : Base("Word::Float::ULP::Lifecycle") {}
          bool run(void) override {
            return inverse_and_holes() && aliases_and_errors();
          }
        };

        class Objective : public IntLexMaximizeSpace {
        public:
          WordVarArray outputs;
          IntVarArray digits;
          Objective(bool fp32)
              : outputs(*this, 2, fp32 ? WDT_FP32 : WDT_BF16),
                digits(*this, fp32 ? 2 : 1, 0, (1 << 30) - 1) {
            unsigned int width = fp32 ? 32 : 16;
            BoolVar opposite(*this, 0, 1);
            WordValue one = fp32 ? 0x7f7fffff : 0x3f80;
            WordValue other = fp32 ? 0x7f7fffff : 0x4080;
            WordVar positive(*this, width, {one}),
                negative(*this, width, {one | (WordValue(1) << (width - 1))}),
                second(*this, width, {other}),
                distance(*this, width, WDT_UNSIGNED);
            ite(*this, opposite, negative, positive, outputs[0]);
            ite(*this, opposite, positive, second, outputs[1]);
            WordDomainType f = fp32 ? WDT_FP32 : WDT_BF16;
            ulp_steps(*this, f, outputs[0],
                      outputs[1], distance);
            if (fp32) {
              WordVar high(*this, 2, WDT_UNSIGNED),
                  low(*this, 30, WDT_UNSIGNED);
              extract(*this, distance, 30, 2, high);
              extract(*this, distance, 0, 30, low);
              channel(*this, high, digits[0], WI_UNSIGNED);
              channel(*this, low, digits[1], WI_UNSIGNED);
            } else {
              channel(*this, distance, digits[0], WI_UNSIGNED);
            }
            branch(*this, opposite, BOOL_VAL_MIN());
            branch(*this, outputs, WORD_VAL_LSB());
            branch(*this, digits, INT_VAR_NONE(), INT_VAL_MIN());
          }
          Objective(Objective& s) : IntLexMaximizeSpace(s) {
            outputs.update(*this, s.outputs);
            digits.update(*this, s.digits);
          }
          Space* copy(void) override { return new Objective(*this); }
          IntVarArgs cost(void) const override { return digits; }
        };
        class Optimize : public Base {
        public:
          Optimize(void) : Base("Word::Float::ULP::Optimize") {}
          bool run(void) override {
            for (bool fp32 : {false, true}) {
              Objective root(fp32);
              BAB<Objective> search(&root);
              std::unique_ptr<Objective> best;
              while (Objective* solution = search.next())
                best.reset(solution);
              if (!best || search.stopped())
                return false;
              WordValue cost = best->digits[0].val();
              if (fp32)
                cost = (cost << 30) + best->digits[1].val();
              if (cost != (fp32 ? WordValue(4278190078ULL) : WordValue(32512)))
                return false;
            }
            return true;
          }
        };
        using Integer = Gecode::Word::Arithmetic::Global::Integer;

        int error_digits(WordDomainType f) {
          return f == WDT_FP32 || f == WDT_BF16 ? 18 : 14;
        }
        int error_scale(WordDomainType f) { return 276 - format(f).fraction; }
        Integer lattice_value(Dyadic x) {
          Integer n =
              Integer(x.significand) * Integer::power_two(x.exponent + 149);
          return x.negative ? -n : n;
        }
        Integer error_numerator(WordDomainType f, WordDomainType fa,
                                WordValue a, WordDomainType fb, WordValue b,
                                WordDomainType fr, WordValue r) {
          const Integer difference =
              (lattice_value(decode(fa, a)) - lattice_value(decode(fb, b)))
                  .abs();
          return difference * Integer::power_two(127 - format(f).fraction -
                                                 spacing(f, decode(fr, r)));
        }
        IntArgs numerator_digits(WordDomainType f, const Integer& numerator) {
          IntArgs digits(error_digits(f));
          for (int i = 0; i < digits.size(); ++i) {
            int digit = 0;
            for (int bit = 0; bit < 30; ++bit)
              if (numerator.bit(30 * (digits.size() - i - 1) + bit))
                digit |= 1 << bit;
            digits[i] = digit;
          }
          return digits;
        }
        bool equal_digits(const IntVarArgs& actual, const IntArgs& expected) {
          for (int i = 0; i < actual.size(); ++i)
            if (!actual[i].assigned() || actual[i].val() != expected[i])
              return false;
          return true;
        }
        Integer assigned_numerator(const IntVarArgs& digits) {
          Integer n;
          for (int i = 0; i < digits.size(); ++i)
            n = n * Integer::power_two(30) + Integer(digits[i].val());
          return n;
        }

        enum class ErrorAlias {
          Separate,
          EqualEndpoints,
          ReferenceA,
          ReferenceB,
          Reinterpreted,
          RepeatedDigit
        };
        class ErrorModel : public IntLexMaximizeSpace {
        public:
          WordVar a, b, r;
          IntVarArray digits;
          ErrorModel(WordDomainType f, WordDomainType fa, WordDomainType fb,
                     WordDomainType fr,
                     ErrorAlias alias = ErrorAlias::Separate,
                     IntPropLevel ipl = IPL_DEF,
                     WordValueSetOption value_set = WVS_EXACT)
              : a(operand(*this, fa, value_set)),
                b(operand(*this, fb, value_set)),
                r(operand(*this, fr, value_set)),
                digits(*this, error_digits(f), -1, Int::Limits::max) {
            switch (alias) {
            case ErrorAlias::EqualEndpoints:
              b = a;
              break;
            case ErrorAlias::ReferenceA:
              r = a;
              break;
            case ErrorAlias::ReferenceB:
              r = b;
              break;
            case ErrorAlias::Reinterpreted:
              b = reinterpret(*this, a, fb);
              break;
            case ErrorAlias::RepeatedDigit:
              for (int i = 1; i < digits.size(); ++i)
                digits[i] = digits[0];
              break;
            case ErrorAlias::Separate:
              break;
            }
            ulp_error(*this, f, a, b, r, digits, ipl);
          }
          ErrorModel(ErrorModel& s) : IntLexMaximizeSpace(s) {
            a.update(*this, s.a);
            b.update(*this, s.b);
            r.update(*this, s.r);
            digits.update(*this, s.digits);
          }
          Space* copy(void) override { return new ErrorModel(*this); }
          IntVarArgs cost(void) const override { return digits; }
          void inputs(void) {
            for (WordVar x : {a, b, r})
              branch(*this, WordVarArgs{x}, WORD_VAL_LSB());
          }
        };

        bool check_ground_error(ErrorModel& root, WordDomainType f,
                                WordValue a, WordValue b, WordValue r,
                                bool wrong = false) {
          std::unique_ptr<ErrorModel> s(static_cast<ErrorModel*>(root.clone()));
          dom(*s, s->a, a);
          dom(*s, s->b, b);
          dom(*s, s->r, r);
          const bool finite = decode(s->a.domain_type(), a).finite &&
                              decode(s->b.domain_type(), b).finite &&
                              decode(s->r.domain_type(), r).finite;
          IntArgs expected;
          if (finite) {
            expected = numerator_digits(f, error_numerator(f, s->a.domain_type(), a,
                                                           s->b.domain_type(), b,
                                                           s->r.domain_type(), r));
            if (wrong)
              rel(*s, s->digits[s->digits.size() - 1], IRT_EQ,
                  expected[expected.size() - 1] ^ 1);
          }
          const bool failed = s->status() == SS_FAILED;
          const bool okay = failed == (!finite || wrong) &&
                            (failed || equal_digits(s->digits, expected));
          if (!okay)
            std::cerr << "ULP error mismatch format=" << f
                      << " values=" << std::hex << a << "," << b << "," << r
                      << std::dec << " failed=" << failed
                      << " expected=" << expected << " actual=" << s->digits
                      << std::endl;
          return okay;
        }

        class ErrorTiny : public Base {
          // Project inputs only: the relation must assign every cost digit.
          static bool enumerate(ErrorModel& root, WordDomainType f,
                                const std::set<WordValue>& codes) {
            root.inputs();
            DFS<ErrorModel> search(&root);
            std::set<std::array<WordValue, 3>> seen;
            while (std::unique_ptr<ErrorModel> s{search.next()}) {
              const WordValue a = s->a.val(), b = s->b.val(),
                              r = s->r.val();
              if (!equal_digits(
                      s->digits,
                      numerator_digits(f, error_numerator(f, s->a.domain_type(), a,
                                                          s->b.domain_type(), b,
                                                          s->r.domain_type(), r))) ||
                  !seen.insert({a, b, r}).second)
                return false;
            }
            if (search.stopped())
              return false;
            for (WordValue a : codes)
              for (WordValue b : codes)
                for (WordValue r : codes)
                  if (seen.count({a, b, r}) != 1)
                    return false;
            return seen.size() == codes.size() * codes.size() * codes.size();
          }

        public:
          ErrorTiny(void) : Base("Word::Float::ULP::Error::Tiny") {}
          bool run(void) override {
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
              if (!complete(ipl))
                return false;
            return true;
          }
          static bool complete(IntPropLevel ipl) {
            std::set<WordValue> fp4;
            for (WordValue i = 0; i < 16; ++i)
              fp4.insert(i);
            // Signed zeros, subnormals, the first exponent boundary, 1 and -1.
            const std::set<WordValue> fp8{0, 0x80, 1, 7, 8, 9, 0x38, 0xb8};
            const WordValArgs subset(
                std::vector<WordValue>(fp8.begin(), fp8.end()));
            for (WordDomainType f : formats) {
              ErrorModel small(f, WDT_E2M1, WDT_E2M1, WDT_E2M1,
                               ErrorAlias::Separate, ipl);
              if (!enumerate(small, f, fp4))
                return false;
              ErrorModel subset_root(f, WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN,
                                     ErrorAlias::Separate, ipl);
              for (WordVar x :
                   {subset_root.a, subset_root.b, subset_root.r})
                dom(subset_root, x, subset);
              if (!enumerate(subset_root, f, fp8))
                return false;
              // Every FP8 code in each role, including rejection of both NaNs.
              for (int role = 0; role < 3; ++role) {
                ErrorModel root(f, WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN,
                                ErrorAlias::Separate, ipl);
                if (root.status() == SS_FAILED)
                  return false;
                for (WordValue c = 0; c < 256; ++c) {
                  WordValue values[] = {0x38, 0xb8, 0x38};
                  values[role] = c;
                  if (!check_ground_error(root, f, values[0], values[1],
                                          values[2]))
                    return false;
                }
              }
            }
            return true;
          }
        };

        class ErrorBoundaries : public Base {
        public:
          ErrorBoundaries(void) : Base("Word::Float::ULP::Error::Boundaries") {}
          bool run(void) override {
            for (WordDomainType f : formats) {
              ErrorModel full(f, WDT_FP32, WDT_FP32, WDT_FP32);
              if (full.status() == SS_FAILED)
                return false;
              // Attainable maxima and minimum positive numerator, all digits.
              if (!check_ground_error(full, f, 0xff7fffff, 0x7f7fffff, 0) ||
                  !check_ground_error(full, f, 0, 1, 0x7f7fffff) ||
                  !check_ground_error(full, f, 0, 2, 0x7f7fffff) ||
                  !check_ground_error(full, f, 0, 0x80000000, 0) ||
                  !check_ground_error(full, f, 0x80000000, 0, 0x80000000) ||
                  !check_ground_error(full, f, 0x7f7fffff, 0x7f7fffff, 0) ||
                  !check_ground_error(full, f, 0, 1, 0x7f7fffff, true))
                return false;
              const Integer maximum =
                  Integer((1U << 24) - 1) *
                  Integer::power_two(254 + 126 + format(f).bias);
              if (error_numerator(f, WDT_FP32, 0xff7fffff, WDT_FP32, 0x7f7fffff,
                                  WDT_FP32, 0) != maximum ||
                  error_numerator(f, WDT_FP32, 0, WDT_FP32, 1, WDT_FP32,
                                  0x7f7fffff) != Integer(1))
                return false;
              const int first_bits = f == WDT_BINARY16 ? 29
                                     : f == WDT_E2M1   ? 15
                                                       : 21;
              if (full.digits[0].max() > (1 << first_bits) - 1)
                return false;
              IntVarArray digits = ulp_error_digits(full, f);
              if (digits.size() != error_digits(f) ||
                  digits[0].min() != 0 ||
                  digits[0].max() != (1 << first_bits) - 1 ||
                  digits[1].min() != 0 || digits[1].max() != (1 << 30) - 1 ||
                  ulp_error_scale(f) != error_scale(f))
                return false;
              // Borrow/carry chains and shift stages around 30-bit boundaries.
              for (unsigned int exponent :
                   {1,   2,   30,  31,  32,  33,  60,  61,  64,
                    65,  90,  91,  120, 121, 128, 129, 150, 151,
                    180, 181, 210, 211, 240, 241, 253, 254}) {
                const WordValue code = WordValue(exponent) << 23;
                for (WordValue r :
                     {WordValue(0), code, WordValue(0x7f7fffff)}) {
                  if (!check_ground_error(full, f, code - 1, code + 1, r) ||
                      !check_ground_error(full, f, code | 0x80000000, code + 1,
                                          r) ||
                      !check_ground_error(full, f, code + 1, 1, r))
                    return false;
                }
              }
              for (WordValue r : {WordValue(0x44000000), WordValue(0x41000000),
                                  WordValue(1), WordValue(0)})
                if (!check_ground_error(full, f, 0x3f800000, 0x3f808000, r))
                  return false;
              // Independently vary all five formats in each of the three roles.
              for (WordDomainType value_format : formats)
                for (int role = 0; role < 3; ++role) {
                  WordDomainType fs[] = {WDT_BF16, WDT_BINARY16, WDT_FP32};
                  fs[role] = value_format;
                  ErrorModel mixed(f, fs[0], fs[1], fs[2]);
                  if (mixed.status() == SS_FAILED)
                    return false;
                  const Format v = format(value_format);
                  const WordValue sign = WordValue(1) << (v.width - 1);
                  const WordValue one = WordValue(v.bias) << v.fraction;
                  for (WordValue c :
                       {WordValue(0), sign, WordValue(1), sign | 1,
                        (WordValue(1) << v.fraction) - 1,
                        WordValue(1) << v.fraction, one - 1, one, one + 1,
                        v.maximum, sign | v.maximum, v.maximum + 1, sign - 1}) {
                    WordValue values[] = {0x3f80, 0x3c00, 0x3f800000};
                    values[role] = c;
                    if (!check_ground_error(mixed, f, values[0], values[1],
                                            values[2]))
                      return false;
                  }
                }
              ErrorModel negative(f, WDT_FP32, WDT_BINARY16, WDT_BF16);
              if (negative.status() == SS_FAILED ||
                  !check_ground_error(negative, f, 0xbf800001, 0x8001, 0xbf80))
                return false;
            }
            ErrorModel fraction(WDT_BF16, WDT_FP32, WDT_FP32, WDT_FP32);
            if (fraction.status() == SS_FAILED ||
                !check_ground_error(fraction, WDT_BF16, 0x3f800000, 0x3f808000,
                                    0x3f800000) ||
                error_numerator(WDT_BF16, WDT_FP32, 0x3f800000, WDT_FP32,
                                0x3f808000, WDT_FP32, 0x3f800000) !=
                    Integer::power_two(error_scale(WDT_BF16) - 1))
              return false;
            return true;
          }
        };

        class ErrorInverse : public Base {
          static bool bounded_triples(IntRelType bound, IntPropLevel ipl) {
            const WordDomainType f = WDT_E2M1;
            ErrorModel root(f, f, f, f, ErrorAlias::Separate, ipl);
            const WordValArgs codes{0, 1, 2, 3, 4, 8, 9, 10};
            const WordValArgs references{0, 2, 4, 6};
            dom(root, root.a, codes);
            dom(root, root.b, codes);
            dom(root, root.r, references);
            // Exactly 3/2, with one common scale even when reference varies.
            const Integer threshold =
                Integer(3) * Integer::power_two(error_scale(f) - 1);
            rel(root, root.digits, bound, numerator_digits(f, threshold));
            root.inputs();
            DFS<ErrorModel> search(&root);
            std::set<std::array<WordValue, 3>> seen, expected;
            for (WordValue a : codes)
              for (WordValue b : codes)
                for (WordValue r : references) {
                  Integer q = error_numerator(f, f, a, f, b, f, r);
                  if (bound == IRT_EQ ? q == threshold : q <= threshold)
                    expected.insert({a, b, r});
                }
            while (std::unique_ptr<ErrorModel> s{search.next()}) {
              const WordValue a = s->a.val(), b = s->b.val(),
                              r = s->r.val();
              if (!equal_digits(s->digits,
                                numerator_digits(
                                    f, error_numerator(f, f, a, f, b, f, r))) ||
                  !seen.insert({a, b, r}).second)
                return false;
            }
            return !search.stopped() && seen == expected;
          }
          static bool pruning(void) {
            ErrorModel endpoint(WDT_FP32, WDT_FP32, WDT_FP32, WDT_FP32);
            dom(endpoint, endpoint.a, 0);
            dom(endpoint, endpoint.r, 0x7f7fffff);
            rel(endpoint, endpoint.b, WRT_UGQ, 1);
            rel(endpoint, endpoint.b, WRT_ULQ, 7);
            // Fix one low bit so unsigned field extraction retains the
            // ordering.
            channel(endpoint, endpoint.b, 0, BoolVar(endpoint, 1, 1));
            if (endpoint.status() == SS_FAILED ||
                endpoint.b.maximum() != 7)
              return false;
            rel(endpoint, endpoint.digits, IRT_LQ,
                numerator_digits(WDT_FP32, Integer(3)));
            if (endpoint.status() == SS_FAILED ||
                endpoint.b.maximum() > 3 || endpoint.b.assigned())
              return false;
            ErrorModel reference(WDT_BF16, WDT_FP32, WDT_FP32, WDT_BF16);
            dom(reference, reference.a, 0);
            dom(reference, reference.b, 1);
            dom(reference, reference.r, 0x4000, 0x40ff);
            if (reference.status() == SS_FAILED)
              return false;
            rel(reference, reference.digits, IRT_EQ,
                numerator_digits(WDT_BF16, Integer::power_two(126)));
            return reference.status() != SS_FAILED &&
                   !reference.r.assigned() &&
                   !reference.r.in(0x3f80) &&
                   !reference.r.in(0x4080) &&
                   reference.r.in(0x4000);
          }
          static bool holes(void) {
            ErrorModel root(WDT_E2M1, WDT_E2M1, WDT_E2M1, WDT_E2M1);
            dom(root, root.a, WordValArgs{2, 3, 4});
            dom(root, root.b, 0);
            dom(root, root.r, 0);
            if (root.status() == SS_FAILED)
              return false;
            std::unique_ptr<ErrorModel> clone(
                static_cast<ErrorModel*>(root.clone()));
            rel(*clone, clone->a, WRT_NQ, WordValue(3));
            if (clone->status() == SS_FAILED || !root.a.in(3))
              return false;
            clone->inputs();
            DFS<ErrorModel> search(clone.get());
            std::set<WordValue> seen;
            while (std::unique_ptr<ErrorModel> s{search.next()}) {
              const WordValue a = s->a.val();
              if (!equal_digits(s->digits,
                                numerator_digits(
                                    WDT_E2M1, error_numerator(
                                                  WDT_E2M1, WDT_E2M1, a,
                                                  WDT_E2M1, 0, WDT_E2M1, 0))))
                return false;
              seen.insert(a);
            }
            return seen == std::set<WordValue>{2, 4} && !search.stopped();
          }

        public:
          ErrorInverse(void) : Base("Word::Float::ULP::Error::Inverse") {}
          bool run(void) override {
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
              if (!bounded_triples(IRT_EQ, ipl) || !bounded_triples(IRT_LQ, ipl))
                return false;
            return pruning() && holes();
          }
        };

        class ErrorAliases : public Base {
        public:
          ErrorAliases(void) : Base("Word::Float::ULP::Error::Aliases") {}
          bool run(void) override {
            for (ErrorAlias alias :
                 {ErrorAlias::EqualEndpoints, ErrorAlias::ReferenceA, ErrorAlias::ReferenceB,
                  ErrorAlias::Reinterpreted}) {
              ErrorModel root(WDT_FP32, WDT_BF16,
                              alias == ErrorAlias::Reinterpreted ? WDT_BINARY16 : WDT_BF16,
                              WDT_BF16, alias);
              const bool shared_reference =
                  alias == ErrorAlias::ReferenceA || alias == ErrorAlias::ReferenceB;
              const WordValue a = shared_reference ? 0x3f80 : 0x3c00;
              const WordValue b = shared_reference ? 0x4080 : 0x3c00;
              const WordValue reference = alias == ErrorAlias::ReferenceB ? b : a;
              if (root.status() == SS_FAILED ||
                  !check_ground_error(root, WDT_FP32, a, b, reference) ||
                  !check_ground_error(root, WDT_FP32, 0x7f80, 0x7f80, 0x7f80))
                return false;
            }
            ErrorModel repeated(WDT_E2M1, WDT_E2M1, WDT_E2M1, WDT_E2M1,
                                ErrorAlias::RepeatedDigit);
            if (repeated.status() == SS_FAILED ||
                !check_ground_error(repeated, WDT_E2M1, 2, 2, 0))
              return false;
            std::unique_ptr<ErrorModel> incompatible(
                static_cast<ErrorModel*>(repeated.clone()));
            dom(*incompatible, incompatible->a, 2);
            dom(*incompatible, incompatible->b, 0);
            dom(*incompatible, incompatible->r, 0);
            if (incompatible->status() != SS_FAILED)
              return false;
            ErrorModel errors(WDT_E2M1, WDT_E2M1, WDT_E2M1, WDT_FP32);
            int caught = 0;
            for (int length : {0, 13, 15, 18}) {
              try {
                ulp_error(errors, WDT_E2M1, errors.a, errors.b,
                          errors.r, IntVarArgs(errors, length, 0, 1));
              } catch (const Gecode::Word::OutOfLimits&) {
                ++caught;
              }
            }
            try {
              ulp_error(errors, static_cast<WordDomainType>(99),
                        errors.a, errors.b, errors.r,
                        errors.digits);
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              (void)ulp_error_digits(errors,
                                     static_cast<WordDomainType>(99));
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              (void)ulp_error_scale(static_cast<WordDomainType>(99));
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            if (caught != 7)
              return false;
            // Aliases of the exact tiny propagator.
            for (ErrorAlias alias :
                 {ErrorAlias::EqualEndpoints, ErrorAlias::ReferenceA,
                  ErrorAlias::ReferenceB, ErrorAlias::RepeatedDigit}) {
              ErrorModel root(WDT_E2M1, WDT_E2M1, WDT_E2M1, WDT_E2M1,
                              alias, IPL_DOM);
              const bool equal = alias == ErrorAlias::EqualEndpoints ||
                                 alias == ErrorAlias::RepeatedDigit;
              const WordValue a = equal ? 2 : 3, b = equal ? 2 : 5;
              const WordValue reference = alias == ErrorAlias::ReferenceB ? b
                                          : alias == ErrorAlias::ReferenceA
                                              ? a
                                              : 0;
              if (root.status() == SS_FAILED ||
                  !check_ground_error(root, WDT_E2M1, a, b, reference))
                return false;
              if (alias == ErrorAlias::RepeatedDigit) {
                std::unique_ptr<ErrorModel> different(
                    static_cast<ErrorModel*>(root.clone()));
                dom(*different, different->a, 2);
                dom(*different, different->b, 0);
                if (different->status() != SS_FAILED)
                  return false;
                // Equal digits are all zero, so only equal values remain.
                if (!root.digits[0].assigned() || root.digits[0].val() != 0)
                  return false;
              }
            }
            for (int value : {-1, 1 << 15}) {
              ErrorModel invalid(WDT_E2M1, WDT_E2M1, WDT_E2M1, WDT_E2M1);
              rel(invalid, invalid.digits[0], IRT_EQ, value);
              if (invalid.status() != SS_FAILED)
                return false;
            }
            return true;
          }
        };

        class ErrorOptimize : public Base {
          static bool candidates(bool varying_reference) {
            ErrorModel root(WDT_BF16, WDT_BF16, WDT_BF16, WDT_BF16);
            // Shared selector keeps a finite set of complete tuples. The second
            // set includes fractions and a changing reference binade.
            const std::array<WordValue, 3> fixed[] = {{0xbf80, 0x3f80, 0x3f80},
                                                      {0x3f80, 0x4080, 0x3f80}};
            const std::array<WordValue, 3> varying[] = {
                {0x3f80, 0x3f80, 0x3f80},
                {0x3f7f, 0x3f80, 0x3f80},
                {0x3f7f, 0x3f80, 0x4000},
                {0x3f80, 0x4080, 0x4080},
                {0xbf80, 0x3f80, 0x3f80}};
            const int count = varying_reference ? 5 : 2;
            const auto* rows = varying_reference ? varying : fixed;
            IntVar selected(root, 0, count - 1);
            BoolVarArgs choices(root, count, 0, 1);
            channel(root, choices, selected);
            Integer expected;
            for (int i = 0; i < count; ++i) {
              rel(root, root.a, WRT_EQ, rows[i][0],
                  Reify(choices[i], RM_IMP));
              rel(root, root.b, WRT_EQ, rows[i][1],
                  Reify(choices[i], RM_IMP));
              rel(root, root.r, WRT_EQ, rows[i][2],
                  Reify(choices[i], RM_IMP));
              Integer q =
                  error_numerator(WDT_BF16, WDT_BF16, rows[i][0], WDT_BF16,
                                  rows[i][1], WDT_BF16, rows[i][2]);
              if (q > expected)
                expected = q;
            }
            branch(root, selected, INT_VAL_MIN());
            root.inputs();
            branch(root, root.digits, INT_VAR_NONE(), INT_VAL_MIN());
            BAB<ErrorModel> search(&root);
            std::unique_ptr<ErrorModel> best;
            Integer previous;
            int incumbents = 0;
            while (std::unique_ptr<ErrorModel> s{search.next()}) {
              for (const IntVar& digit : s->digits)
                if (!digit.assigned())
                  return false;
              Integer current = assigned_numerator(s->digits);
              if (incumbents && current <= previous)
                return false;
              previous = current;
              ++incumbents;
              best = std::move(s);
            }
            if (!best || search.stopped() || previous != expected ||
                incumbents < 2)
              return false;
            if (!varying_reference &&
                (best->a.val() != 0x3f80 ||
                 best->b.val() != 0x4080 ||
                 previous != Integer(384) * Integer::power_two(269)))
              return false;
            return true;
          }
          static bool thresholds(void) {
            using Gecode::Word::Arithmetic::Global::ceil_div;
            using Gecode::Word::Arithmetic::Global::floor_div;
            // A non-dyadic 1/3 threshold distinguishes floor from ceiling.
            for (WordDomainType f : formats) {
              const Integer unit = Integer::power_two(error_scale(f));
              const Integer floor = floor_div(unit, Integer(3));
              const Integer ceil = ceil_div(unit, Integer(3));
              for (const Integer& q : {floor, ceil}) {
                const Integer scaled_error = q * Integer(3);
                if ((q <= floor) != (scaled_error <= unit) ||
                    (q <= ceil - Integer(1)) != (scaled_error < unit) ||
                    (q >= ceil) != (scaled_error >= unit) ||
                    (q >= floor + Integer(1)) != (scaled_error > unit))
                  return false;
                // Order differs only in the last digit at this scale.
                IntArgs a = numerator_digits(f, q),
                        b = numerator_digits(f, q + Integer(1));
                if (!std::lexicographical_compare(a.begin(), a.end(), b.begin(),
                                                  b.end()))
                  return false;
              }
            }
            return true;
          }

        public:
          ErrorOptimize(void) : Base("Word::Float::ULP::Error::Optimize") {}
          bool run(void) override {
            return candidates(false) && candidates(true) && thresholds();
          }
        };
        /// Tiny observations on words with or without value sets
        class TinyModel : public Space {
        public:
          WordVar a, b, r;
          IntVar q;
          WordVar d;
          IntVarArray digits;
          TinyModel(WordDomainType reference, WordDomainType fa,
                    WordDomainType fb, WordDomainType fr,
                    WordValueSetOption value_set,
                    WordDomainType distance = WDT_UNSIGNED)
              : a(operand(*this, fa, value_set)),
                b(operand(*this, fb, value_set)),
                r(operand(*this, fr, value_set)),
                q(*this, -200, 200),
                d(*this, float_word_width(reference), distance,
                  float_word_width(reference) <= 8 ? value_set : WVS_NONE),
                digits(*this, error_digits(reference), 0, (1 << 30) - 1) {}
          TinyModel(TinyModel& s) : Space(s) {
            a.update(*this, s.a);
            b.update(*this, s.b);
            r.update(*this, s.r);
            q.update(*this, s.q);
            d.update(*this, s.d);
            digits.update(*this, s.digits);
          }
          Space* copy(void) override { return new TinyModel(*this); }
        };

        /// Test whether \a x keeps exactly, or the hull and bounds of, \a p
        bool projected(const WordVar& x, const std::set<WordValue>& p) {
          if (x.has_value_set())
            return values(x) == p;
          WordValue ones = ~WordValue(0), any = 0;
          for (WordValue v : p) {
            ones &= v;
            any |= v;
          }
          const auto bounds = float_order_extremes(x.width(), p);
          return x.lo() == ones && x.hi() == any &&
                 x.minimum() == bounds.first && x.maximum() == bounds.second;
        }

        class TinyRoot : public Base {
          using Tuple = std::array<WordValue, 3>;

          /// Values that differ from a random base in up to four random bits
          std::set<WordValue> random_codes(WordDomainType f) {
            const unsigned int width = float_word_width(f);
            const WordValue base = _rand(1U << width);
            WordValue free_bits = 0;
            for (unsigned int i = 0, n = 1 + _rand(4U); i < n; ++i)
              free_bits |= WordValue(1) << _rand(width);
            std::set<WordValue> codes;
            while (codes.empty())
              for (WordValue v = 0; v <= free_bits; ++v)
                if ((v & ~free_bits) == 0 && _rand(2U) == 0)
                  codes.insert(base ^ v);
            return codes;
          }
          /// Restrict \a x to \a codes, or without a value set replace it by
          /// a word of their hull and total-order bounds, and return the
          /// values of \a x.
          static std::set<WordValue> restrict(Space& home, WordVar& x,
                                              const std::set<WordValue>& codes) {
            if (x.has_value_set()) {
              dom(home, x,
                  WordValArgs(std::vector<WordValue>(codes.begin(), codes.end())));
            } else {
              // A list would post a membership constraint beyond the domain.
              WordValue ones = ~WordValue(0), any = 0;
              for (WordValue v : codes) {
                ones &= v;
                any |= v;
              }
              const unsigned int width = x.width();
              const auto bounds = float_order_extremes(width, codes);
              x = WordVar(home, width, x.domain_type(),
                          bounds.first, bounds.second, cube_bits(width, ones, any), WVS_NONE);
            }
            return values(x);
          }
          static WordDomainType tiny_format(unsigned int i) {
            return i == 0 ? WDT_E2M1 : WDT_E4M3FN;
          }
          static std::set<Tuple> solutions(TinyModel& root, int roles) {
            WordVarArgs inputs{root.a};
            if (roles > 1)
              inputs << root.b;
            if (roles > 2)
              inputs << root.r;
            branch(root, inputs, WORD_VAL_LSB());
            DFS<TinyModel> search(&root);
            std::set<Tuple> found;
            while (std::unique_ptr<TinyModel> s{search.next()})
              found.insert({s->a.val(),
                            roles > 1 ? s->b.val() : 0,
                            roles > 2 ? s->r.val() : 0});
            return found;
          }
          /// Spacing keeps exactly the supported references and exponents.
          bool spacing_case(WordValueSetOption value_set) {
            const WordDomainType reference = formats[_rand(5U)];
            const WordDomainType fr = tiny_format(_rand(2U));
            TinyModel root(reference, fr, fr, fr, value_set);
            const std::set<WordValue> rs = restrict(root, root.a, random_codes(fr));
            const int least = -12 + static_cast<int>(_rand(14U));
            rel(root, root.q, IRT_GQ, least);
            rel(root, root.q, IRT_LQ, least + static_cast<int>(_rand(4U)));
            std::set<WordValue> supported;
            std::set<int> exponents;
            for (WordValue c : rs)
              if (decode(fr, c).finite &&
                  root.q.in(spacing(reference, decode(fr, c)))) {
                supported.insert(c);
                exponents.insert(spacing(reference, decode(fr, c)));
              }
            ulp_spacing(root, reference, root.a, root.q);
            if (root.status() == SS_FAILED)
              return supported.empty();
            if (supported.empty() || !projected(root.a, supported) ||
                root.q.size() != exponents.size())
              return false;
            for (int e : exponents)
              if (!root.q.in(e))
                return false;
            std::set<Tuple> expected;
            for (WordValue c : supported)
              expected.insert({c, 0, 0});
            return solutions(root, 1) == expected;
          }
          /// Steps keeps every supported pair, and IPL_DOM their projection.
          bool steps_case(WordValueSetOption value_set, IntPropLevel ipl) {
            const WordDomainType reference = formats[_rand(5U)];
            const WordDomainType fa = tiny_format(_rand(2U)),
                                  fb = tiny_format(_rand(2U));
            TinyModel root(reference, fa, fb, fa, value_set);
            const std::set<WordValue> as = restrict(root, root.a, random_codes(fa));
            const std::set<WordValue> bs = restrict(root, root.b, random_codes(fb));
            const unsigned int width = float_word_width(reference);
            const WordValue limit =
                _rand(2U) == 0 ? 0 : 1 + _rand(width <= 4 ? 14U : 64U);
            if (limit != 0)
              rel(root, root.d, WRT_ULQ, limit);
            std::set<Tuple> expected;
            std::set<WordValue> pa, pb, pd;
            for (WordValue a : as)
              for (WordValue b : bs) {
                WordValue distance;
                if (steps(reference, fa, a, fb, b, distance) &&
                    (limit == 0 || distance <= limit)) {
                  expected.insert({a, b, 0});
                  pa.insert(a);
                  pb.insert(b);
                  pd.insert(distance);
                }
              }
            ulp_steps(root, reference, root.a, root.b, root.d, ipl);
            if (root.status() == SS_FAILED)
              return expected.empty();
            for (const Tuple& t : expected)
              if (!root.a.in(t[0]) || !root.b.in(t[1]))
                return false;
            for (WordValue distance : pd)
              if (!root.d.in(distance))
                return false;
            if (ipl == IPL_DOM && !expected.empty() &&
                (!projected(root.a, pa) ||
                 !projected(root.b, pb) ||
                 (root.d.has_value_set() && values(root.d) != pd)))
              return false;
            return solutions(root, 2) == expected;
          }
          /// Error keeps every supported triple, and IPL_DOM the projection
          /// of the triples whose digits remain.
          bool error_case(WordValueSetOption value_set, IntPropLevel ipl) {
            const WordDomainType reference = formats[_rand(5U)];
            const WordDomainType fa = tiny_format(_rand(2U)),
                                  fb = tiny_format(_rand(2U)),
                                  fr = tiny_format(_rand(2U));
            TinyModel root(reference, fa, fb, fr, value_set);
            const std::set<WordValue> as = restrict(root, root.a, random_codes(fa));
            const std::set<WordValue> bs = restrict(root, root.b, random_codes(fb));
            const std::set<WordValue> rs = restrict(root, root.r, random_codes(fr));
            // Keep each digit at zero or at the digit of a random triple.
            const IntArgs witness = numerator_digits(
                reference,
                error_numerator(reference, fa, *as.begin(), fb, *bs.rbegin(),
                                fr, *rs.begin()));
            for (int i = 0; i < witness.size(); ++i)
              if (_rand(2U) == 0)
                dom(root, root.digits[i],
                    IntSet(IntArgs{0, witness[i]}));
            std::set<Tuple> expected;
            std::set<WordValue> pa, pb, pr;
            for (WordValue a : as)
              for (WordValue b : bs)
                for (WordValue r : rs) {
                  if (!decode(fa, a).finite || !decode(fb, b).finite ||
                      !decode(fr, r).finite)
                    continue;
                  const IntArgs digits = numerator_digits(
                      reference, error_numerator(reference, fa, a, fb, b, fr, r));
                  bool admitted = true;
                  for (int i = 0; i < digits.size(); ++i)
                    admitted = admitted && root.digits[i].in(digits[i]);
                  if (admitted) {
                    expected.insert({a, b, r});
                    pa.insert(a);
                    pb.insert(b);
                    pr.insert(r);
                  }
                }
            ulp_error(root, reference, root.a, root.b, root.r, root.digits,
                      ipl);
            if (root.status() == SS_FAILED)
              return expected.empty();
            for (const Tuple& t : expected)
              if (!root.a.in(t[0]) || !root.b.in(t[1]) ||
                  !root.r.in(t[2]))
                return false;
            if (ipl == IPL_DOM && !expected.empty() &&
                (!projected(root.a, pa) ||
                 !projected(root.b, pb) ||
                 !projected(root.r, pr)))
              return false;
            return solutions(root, 3) == expected;
          }

        public:
          TinyRoot(void) : Base("Word::Float::ULP::TinyRoot") {}
          bool run(void) override {
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              for (int trial = 0; trial < 20; ++trial) {
                if (!spacing_case(value_set))
                  return false;
                for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                  if (!steps_case(value_set, ipl) ||
                      !error_case(value_set, ipl))
                    return false;
              }
            return true;
          }
        };

        class TinyBounds : public Base {
          // Steps narrows each endpoint to the distance interval around the
          // other endpoint's ranks, and the distance to their span.
          static bool steps_bounds(WordValueSetOption value_set) {
            TinyModel near(WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN,
                           value_set);
            dom(near, near.a, 0x38, 0x39);
            dom(near, near.b,
                WordValArgs{0x30, 0x36, 0x3a, 0x3b, 0x40});
            rel(near, near.d, WRT_ULQ, WordValue(2));
            ulp_steps(near, WDT_E4M3FN, near.a, near.b, near.d);
            if (near.status() == SS_FAILED || near.b.in(0x30) ||
                near.b.in(0x40) || !near.b.in(0x36) ||
                !near.b.in(0x3b) || near.a.assigned())
              return false;
            for (WordDomainType distance : {WDT_UNSIGNED, WDT_CUBE}) {
              TinyModel far(WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN,
                            value_set, distance);
              dom(far, far.a, 0x38, 0x39);
              dom(far, far.b, 0x40, 0x41);
              ulp_steps(far, WDT_E4M3FN, far.a, far.b, far.d);
              if (far.status() == SS_FAILED || !far.d.in(7) || !far.d.in(9))
                return false;
              // A cube without a value set keeps the hull of 7 to 9.
              if (distance == WDT_CUBE && !far.d.has_value_set()
                      ? far.d.hi() != 0x0f
                      : far.d.in(6) || far.d.in(10))
                return false;
              if (distance == WDT_UNSIGNED &&
                  (far.d.minimum() != 7 || far.d.maximum() != 9))
                return false;
            }
            return true;
          }
          // A wide signed distance keeps the tightest signed bounds of its
          // supports, which here lie on both sides of the sign bit.
          static bool signed_distance_bounds(void) {
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
              TinyModel s(WDT_BF16, WDT_E2M1, WDT_E2M1, WDT_E2M1, WVS_EXACT,
                          WDT_SIGNED);
              dom(s, s.a, 0x7);
              dom(s, s.b, WordValArgs{0x0, 0xf});
              ulp_steps(s, WDT_BF16, s.a, s.b, s.d, ipl);
              if (s.status() == SS_FAILED || !s.d.in(0x40c0) ||
                  !s.d.in(0x8180) || s.d.minimum() != 0x8180 ||
                  s.d.maximum() != 0x40c0)
                return false;
            }
            return true;
          }
          // A fixed exponent keeps the references of its binades, from
          // 0x8f in total order up to 0x0f.
          static bool spacing_bounds(void) {
            TinyModel s(WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN, WDT_E4M3FN,
                        WVS_NONE);
            rel(s, s.q, IRT_EQ, -9);
            ulp_spacing(s, WDT_E4M3FN, s.a, s.q);
            return s.status() != SS_FAILED && s.a.lo() == 0 &&
                   s.a.hi() == 0x8f && s.a.minimum() == 0x8f &&
                   s.a.maximum() == 0x0f;
          }
          // A digit threshold bounds an operand without a value set to -1
          // (0xa) and 1 (2) at IPL_DOM; the default decomposition only keeps
          // the bound values.
          static bool error_bounds(void) {
            for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
              TinyModel s(WDT_E2M1, WDT_E2M1, WDT_E2M1, WDT_E2M1, WVS_NONE);
              dom(s, s.b, 0);
              dom(s, s.r, 0);
              ulp_error(s, WDT_E2M1, s.a, s.b, s.r, s.digits, ipl);
              rel(s, s.digits, IRT_LQ,
                  numerator_digits(WDT_E2M1, error_numerator(
                                                 WDT_E2M1, WDT_E2M1, 2,
                                                 WDT_E2M1, 0, WDT_E2M1, 0)));
              if (s.status() == SS_FAILED || s.a.assigned() ||
                  !s.a.in(2) || !s.a.in(10) ||
                  (ipl == IPL_DOM && (s.a.minimum() != 0xa ||
                                      s.a.maximum() != 2)))
                return false;
            }
            return true;
          }

        public:
          TinyBounds(void) : Base("Word::Float::ULP::TinyBounds") {}
          bool run(void) override {
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              if (!steps_bounds(value_set))
                return false;
            return signed_distance_bounds() && spacing_bounds() && error_bounds();
          }
        };

        /// Complete solutions of the observations on float-typed operands
        /// whose windows are negative or cross zero; the distance stays an
        /// unsigned word.
        class FloatTyped : public Base {
          // Total-order window of encodings
          struct Range {
            WordValue minimum, maximum;
          };
          class Observed : public Space {
          public:
            WordVar a, b, r;
            IntVar q;
            WordVar d;
            IntVarArray digits;
            Observed(WordDomainType reference, WordDomainType fa, Range wa,
                     WordDomainType fb, Range wb)
                : a(float_window(*this, fa, wa.minimum, wa.maximum)),
                  b(float_window(*this, fb, wb.minimum, wb.maximum)),
                  r(float_window(*this, fa, wa.minimum, wa.maximum)),
                  q(*this, -200, 200),
                  d(*this, float_word_width(reference), WDT_UNSIGNED),
                  digits(ulp_error_digits(*this, reference)) {}
            Observed(Observed& s) : Space(s) {
              a.update(*this, s.a);
              b.update(*this, s.b);
              r.update(*this, s.r);
              q.update(*this, s.q);
              d.update(*this, s.d);
              digits.update(*this, s.digits);
            }
            Space* copy(void) override { return new Observed(*this); }
          };
          // Every format with a negative window and a cross-zero window;
          // the wide negative windows start at NaNs and negative infinity.
          static std::vector<Range> windows(WordDomainType f) {
            switch (f) {
            case WDT_E2M1:
              return {{0xf, 0xc}, {0xb, 3}};
            case WDT_E4M3FN:
              return {{0xff, 0xfc}, {0x83, 3}};
            case WDT_BINARY16:
              return {{0xfc01, 0xfbfe}, {0x8003, 3}};
            case WDT_BF16:
              return {{0xff81, 0xff7e}, {0x8003, 3}};
            default:
              return {{0xff800001, 0xff7ffffe}, {0x80000003, 3}};
            }
          }
          static bool spacings(WordDomainType reference, WordDomainType fa,
                               Range wa) {
            Observed s(reference, fa, wa, fa, wa);
            ulp_spacing(s, reference, s.a, s.q);
            std::set<std::pair<WordValue, int>> expected, found;
            for (WordValue a :
                 float_order_range(float_word_width(fa), wa.minimum,
                                   wa.maximum))
              if (decode(fa, a).finite)
                expected.insert({a, spacing(reference, decode(fa, a))});
            branch(s, WordVarArgs{s.a}, WORD_VAL_LSB());
            DFS<Observed> search(&s);
            while (std::unique_ptr<Observed> t{search.next()})
              found.insert({t->a.val(),
                            t->q.assigned() ? t->q.val() : 1000});
            return found == expected;
          }
          static bool distances(WordDomainType reference, WordDomainType fa,
                                Range wa, WordDomainType fb, Range wb,
                                IntPropLevel ipl) {
            Observed s(reference, fa, wa, fb, wb);
            ulp_steps(s, reference, s.a, s.b, s.d, ipl);
            std::set<std::array<WordValue, 3>> expected, found;
            for (WordValue a :
                 float_order_range(float_word_width(fa), wa.minimum,
                                   wa.maximum))
              for (WordValue b :
                   float_order_range(float_word_width(fb), wb.minimum,
                                     wb.maximum)) {
                WordValue distance;
                if (steps(reference, fa, a, fb, b, distance))
                  expected.insert({a, b, distance});
              }
            branch(s, WordVarArgs{s.a, s.b}, WORD_VAL_LSB());
            DFS<Observed> search(&s);
            while (std::unique_ptr<Observed> t{search.next()}) {
              if (!t->d.assigned())
                return false;
              found.insert({t->a.val(), t->b.val(), t->d.val()});
            }
            return found == expected;
          }
          // The reference value r has the format and window of a.
          static bool errors(WordDomainType f, WordDomainType fa, Range wa,
                             WordDomainType fb, Range wb, IntPropLevel ipl) {
            Observed s(f, fa, wa, fb, wb);
            ulp_error(s, f, s.a, s.b, s.r, s.digits, ipl);
            const std::vector<WordValue> as =
                float_order_range(float_word_width(fa), wa.minimum,
                                  wa.maximum);
            std::set<std::array<WordValue, 3>> expected, found;
            for (WordValue a : as)
              for (WordValue b :
                   float_order_range(float_word_width(fb), wb.minimum,
                                     wb.maximum))
                for (WordValue r : as)
                  if (decode(fa, a).finite && decode(fb, b).finite &&
                      decode(fa, r).finite)
                    expected.insert({a, b, r});
            branch(s, WordVarArgs{s.a, s.b, s.r},
                   WORD_VAL_LSB());
            DFS<Observed> search(&s);
            while (std::unique_ptr<Observed> t{search.next()}) {
              const WordValue a = t->a.val(), b = t->b.val(),
                              r = t->r.val();
              if (!equal_digits(t->digits,
                                numerator_digits(f, error_numerator(
                                                        f, fa, a, fb, b, fa,
                                                        r))))
                return false;
              found.insert({a, b, r});
            }
            return found == expected;
          }

        public:
          FloatTyped(void) : Base("Word::Float::ULP::FloatTyped") {}
          bool run(void) override {
            const WordDomainType operands[] = {WDT_E2M1, WDT_E4M3FN,
                                                WDT_BF16, WDT_FP32};
            for (WordDomainType reference : formats)
              for (WordDomainType fa : operands)
                for (Range wa : windows(fa)) {
                  if (!spacings(reference, fa, wa))
                    return false;
                  for (WordDomainType fb : operands)
                    for (Range wb : windows(fb))
                      for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                        if (!distances(reference, fa, wa, fb, wb, ipl))
                          return false;
                }
            // Errors on the windows near zero, and on tiny formats also on
            // the negative windows.
            for (WordDomainType f : {WDT_E4M3FN, WDT_BF16, WDT_FP32})
              for (WordDomainType fa : operands)
                for (WordDomainType fb : operands)
                  for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
                    const Range wa = windows(fa)[1], wb = windows(fb)[1];
                    if (!errors(f, fa, wa, fb, wb, ipl) ||
                        (float_word_width(fa) <= 8 &&
                         !errors(f, fa, windows(fa)[0], fb, wb, ipl)))
                      return false;
                  }
            return true;
          }
        };
        ErrorInverse error_inverse;
        ErrorAliases error_aliases;
        ErrorOptimize error_optimize;
        ErrorTiny error_tiny;
        ErrorBoundaries error_boundaries;
        Tiny tiny;
        Boundaries boundary_tests;
        Lifecycle lifecycle;
        Optimize optimize;
        TinyRoot tiny_root;
        TinyBounds tiny_bounds;
        FloatTyped float_typed;
      } // namespace ULP
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
