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

namespace Test {
  namespace Word {
    namespace Float {
      namespace ULP {
        using namespace Gecode;

        const FloatWordFormat formats[] = {FWF_FP32, FWF_BF16, FWF_BINARY16,
                                           FWF_E4M3FN, FWF_E2M1};
        struct Format {
          int width, fraction, bias;
          WordValue maximum;
        };
        Format format(FloatWordFormat f) {
          switch (f) {
          case FWF_FP32:
            return {32, 23, 127, 0x7f7fffff};
          case FWF_BF16:
            return {16, 7, 127, 0x7f7f};
          case FWF_BINARY16:
            return {16, 10, 15, 0x7bff};
          case FWF_E4M3FN:
            return {8, 3, 7, 0x7e};
          case FWF_E2M1:
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
        Dyadic decode(FloatWordFormat f, WordValue code) {
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
        int spacing(FloatWordFormat reference, Dyadic x) {
          Format r = format(reference);
          int binade =
              x.significand ? top_bit(x.significand) + x.exponent : 1 - r.bias;
          return std::max(binade, 1 - r.bias) - r.fraction;
        }
        bool rank(FloatWordFormat reference, Dyadic x, int64_t& result) {
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
        bool steps(FloatWordFormat reference, FloatWordFormat fa, WordValue a,
                   FloatWordFormat fb, WordValue b, WordValue& result) {
          int64_t ar, br;
          if (!rank(reference, decode(fa, a), ar) ||
              !rank(reference, decode(fb, b), br))
            return false;
          result = static_cast<WordValue>(ar >= br ? ar - br : br - ar);
          return true;
        }
        FloatWordVar operand(Space& home, FloatWordFormat f,
                             WordDomainType storage) {
          FloatWordVar x(home, f, storage);
          if (float_word_width(f) <= 8)
            x = FloatWordVar(home, f, x.bits(), IntVar(home, 0, 255));
          return x;
        }

        enum class Observation { Spacing, Steps };

        class Model : public Space {
        public:
          FloatWordVar a, b;
          IntVar q;
          WordVar d;
          Model(FloatWordFormat reference, FloatWordFormat fa,
                FloatWordFormat fb, Observation observation,
                WordDomainType storage = WDT_UNSIGNED)
              : a(operand(*this, fa, storage)), b(operand(*this, fb, storage)),
                q(*this, -200, 200),
                d(*this, float_word_width(reference), storage) {
            if (observation == Observation::Spacing)
              ulp_spacing(*this, reference, a, q);
            else
              ulp_steps(*this, reference, a, b, d);
          }
          Model(Model& s) : Space(s) {
            a.update(*this, s.a);
            b.update(*this, s.b);
            q.update(*this, s.q);
            d.update(*this, s.d);
          }
          Space* copy(void) override { return new Model(*this); }
          void inputs(Observation observation) {
            if (a.has_encoding())
              branch(*this, a.encoding(), INT_VAL_MIN());
            else
              branch(*this, WordVarArgs{a.bits()}, WORD_VAL_LSB());
            if (observation == Observation::Steps) {
              if (b.has_encoding())
                branch(*this, b.encoding(), INT_VAL_MIN());
              else
                branch(*this, WordVarArgs{b.bits()}, WORD_VAL_LSB());
            }
          }
        };

        class Tiny : public Base {
        public:
          Tiny(void) : Base("Word::Float::ULP::Tiny") {}
          bool run(void) override {
            for (FloatWordFormat reference : formats)
              for (FloatWordFormat fa : {FWF_E2M1, FWF_E4M3FN}) {
                Model local(reference, fa, fa, Observation::Spacing);
                local.inputs(Observation::Spacing);
                DFS<Model> search(&local);
                std::set<WordValue> seen;
                while (std::unique_ptr<Model> s{search.next()}) {
                  WordValue a = s->a.bits().val();
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
                for (FloatWordFormat fb : {FWF_E2M1, FWF_E4M3FN}) {
                  Model root(reference, fa, fb, Observation::Steps);
                  root.inputs(Observation::Steps);
                  DFS<Model> pairs(&root);
                  std::set<std::pair<WordValue, WordValue>> projected;
                  while (std::unique_ptr<Model> s{pairs.next()}) {
                    WordValue a = s->a.bits().val(), b = s->b.bits().val(),
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

        std::set<WordValue> boundaries(FloatWordFormat f) {
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
            for (FloatWordFormat reference : formats)
              for (FloatWordFormat f : {FWF_FP32, FWF_BF16, FWF_BINARY16}) {
                Model local(reference, f, f, Observation::Spacing),
                    distance(reference, f, f, Observation::Steps);
                if (local.status() == SS_FAILED ||
                    distance.status() == SS_FAILED)
                  return false;
                for (WordValue code : boundaries(f)) {
                  std::unique_ptr<Model> s(static_cast<Model*>(local.clone()));
                  dom(*s, s->a.bits(), code);
                  Dyadic x = decode(f, code);
                  bool failed = s->status() == SS_FAILED;
                  if (failed != !x.finite ||
                      (!failed && (!s->q.assigned() ||
                                   s->q.val() != spacing(reference, x))))
                    return false;
                  std::unique_ptr<Model> d(
                      static_cast<Model*>(distance.clone()));
                  dom(*d, d->a.bits(), code);
                  dom(*d, d->b.bits(), 0);
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
            for (FloatWordFormat reference : formats)
              for (FloatWordFormat fa : formats)
                for (FloatWordFormat fb : formats) {
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
                    dom(*s, s->a.bits(), ac[i]);
                    dom(*s, s->b.bits(), bc[i]);
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
            for (FloatWordFormat reference : formats)
              for (FloatWordFormat f : {FWF_E2M1, FWF_E4M3FN}) {
                Model root(reference, f, FWF_FP32, Observation::Steps);
                dom(root, root.b.bits(), 0);
                if (root.status() == SS_FAILED)
                  return false;
                for (WordValue code = 0;
                     code < (WordValue(1) << format(f).width); ++code) {
                  std::unique_ptr<Model> s(static_cast<Model*>(root.clone()));
                  dom(*s, s->a.bits(), code);
                  WordValue expected;
                  bool valid = steps(reference, f, code, FWF_FP32, 0, expected);
                  bool failed = s->status() == SS_FAILED;
                  if (failed != !valid ||
                      (!failed && (!s->d.assigned() || s->d.val() != expected)))
                    return false;
                }
              }
            Model precision(FWF_BF16, FWF_FP32, FWF_FP32, Observation::Steps);
            dom(precision, precision.a.bits(), 0x3f800000);
            dom(precision, precision.b.bits(), 0x3f808000); // 1 + 2^-8
            if (precision.status() != SS_FAILED)
              return false;
            for (FloatWordFormat f : formats)
              for (WordDomainType storage :
                   {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
                Model s(f, f, f, Observation::Steps, storage);
                WordValue maximum = format(f).maximum;
                dom(s, s.a.bits(), maximum);
                dom(s, s.b.bits(),
                    maximum | (WordValue(1) << (format(f).width - 1)));
                if (s.status() == SS_FAILED || !s.d.assigned() ||
                    s.d.val() != 2 * maximum)
                  return false;
              }
            // These scales must not clamp to the target's largest finite
            // binade.
            for (const auto& c :
                 {std::make_pair(FWF_E4M3FN, WordValue(0x44000000)),
                  std::make_pair(FWF_E2M1, WordValue(0x41000000))}) {
              Model s(c.first, FWF_FP32, FWF_FP32, Observation::Spacing);
              dom(s, s.a.bits(), c.second);
              if (s.status() == SS_FAILED || !s.q.assigned() ||
                  s.q.val() != (c.first == FWF_E4M3FN ? 6 : 2))
                return false;
            }
            return true;
          }
        };

        class Lifecycle : public Base {
          static bool inverse_and_holes(void) {
            for (WordDomainType storage :
                 {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
              for (FloatWordFormat f : {FWF_E2M1, FWF_E4M3FN}) {
                Model s(f, f, f, Observation::Steps, storage);
                dom(s, s.a.encoding(), IntSet({2, 3, 4}));
                dom(s, s.b.encoding(), IntSet({3, 4, 5}));
                dom(s, s.d, 1);
                ulp_spacing(s, f, s.a, s.q);
                if (s.status() == SS_FAILED)
                  return false;
                std::unique_ptr<Model> clone(static_cast<Model*>(s.clone()));
                // Interior ID 3 disappears after both observations have
                // propagated.
                rel(*clone, clone->a.encoding(), IRT_NQ, 3);
                rel(*clone, clone->q, IRT_EQ, f == FWF_E2M1 ? -1 : -9);
                if (clone->status() == SS_FAILED || clone->b.encoding().in(4) ||
                    !s.a.encoding().in(3))
                  return false;
                if (f == FWF_E2M1 && (!clone->a.encoding().assigned() ||
                                      clone->a.encoding().val() != 2 ||
                                      !clone->b.encoding().assigned() ||
                                      clone->b.encoding().val() != 3))
                  return false;
                s.inputs(Observation::Steps);
                DFS<Model> search(&s);
                std::set<std::pair<int, int>> seen;
                while (std::unique_ptr<Model> solution{search.next()}) {
                  if (!solution->d.assigned() || solution->d.val() != 1 ||
                      !solution->q.assigned())
                    return false;
                  seen.emplace(solution->a.encoding().val(),
                               solution->b.encoding().val());
                }
                if (seen != std::set<std::pair<int, int>>{
                                {2, 3}, {3, 4}, {4, 3}, {4, 5}})
                  return false;
              }
              Model large(FWF_FP32, FWF_E4M3FN, FWF_E2M1, Observation::Steps, storage);
              dom(large, large.a.bits(), 0xfe);
              dom(large, large.b.bits(), 7);
              WordValue span;
              if (!steps(FWF_FP32, FWF_E4M3FN, 0xfe, FWF_E2M1, 7, span) ||
                  span <= static_cast<WordValue>(Int::Limits::max) ||
                  large.status() == SS_FAILED || !large.d.assigned() ||
                  large.d.val() != span)
                return false;
              Model wide(FWF_BF16, FWF_BF16, FWF_BF16, Observation::Steps, storage);
              dom(wide, wide.a.bits(), 0x3f80);
              dom(wide, wide.d, 1);
              // A bounded inverse search should recover exactly the two
              // neighbors.
              rel(wide, wide.b.bits(), WRT_UGQ, 0x3f7e);
              rel(wide, wide.b.bits(), WRT_ULE, 0x3f82);
              wide.inputs(Observation::Steps);
              DFS<Model> inverse(&wide);
              std::set<WordValue> neighbors;
              while (std::unique_ptr<Model> solution{inverse.next()})
                neighbors.insert(solution->b.bits().val());
              if (neighbors != std::set<WordValue>{0x3f7f, 0x3f81})
                return false;
            }
            Model local(FWF_BF16, FWF_FP32, FWF_FP32, Observation::Spacing);
            rel(local, local.q, IRT_EQ, -7);
            if (local.status() == SS_FAILED)
              return false;
            dom(local, local.a.bits(),
                0x40000000); // 2 has exponent -6, not -7.
            return local.status() == SS_FAILED;
          }
          static bool aliases_and_errors(void) {
            for (FloatWordFormat f : formats) {
              Model same(f, f, f, Observation::Spacing);
              ulp_steps(same, f, same.a, same.a, same.d);
              if (same.status() == SS_FAILED || !same.d.assigned() ||
                  same.d.val() != 0)
                return false;
              Model alias(f, f, f, Observation::Spacing);
              dom(alias, alias.b.bits(), 0);
              ulp_steps(alias, f, alias.a, alias.b, alias.a.bits());
              dom(alias, alias.a.bits(), 1);
              if (alias.status() == SS_FAILED)
                return false;
              Model incorrect(f, f, f, Observation::Steps);
              dom(incorrect, incorrect.a.bits(), 0);
              dom(incorrect, incorrect.b.bits(), 0);
              dom(incorrect, incorrect.d, 1);
              if (incorrect.status() != SS_FAILED)
                return false;
            }
            Model tiny(FWF_E2M1, FWF_E2M1, FWF_E2M1, Observation::Spacing);
            ulp_spacing(tiny, FWF_E2M1, tiny.a, tiny.a.encoding());
            if (tiny.status() != SS_FAILED)
              return false;
            // Equal raw bits with different 16-bit labels are different values.
            Model shared(FWF_FP32, FWF_BF16, FWF_BINARY16, Observation::Spacing);
            ulp_steps(shared, FWF_FP32, shared.a,
                      shared.a.reinterpret_bits(FWF_BINARY16), shared.d);
            dom(shared, shared.a.bits(), 0x3c00);
            WordValue expected;
            if (!steps(FWF_FP32, FWF_BF16, 0x3c00, FWF_BINARY16, 0x3c00,
                       expected) ||
                shared.status() == SS_FAILED || !shared.d.assigned() ||
                shared.d.val() != expected)
              return false;
            // Sharing an encoding ID across the two tiny formats does not
            // make their decoded values equal.
            Model shared_codes(FWF_FP32, FWF_E2M1, FWF_E4M3FN, Observation::Spacing);
            shared_codes.b =
                FloatWordVar(shared_codes, FWF_E4M3FN, shared_codes.b.bits(),
                             shared_codes.a.encoding());
            ulp_steps(shared_codes, FWF_FP32, shared_codes.a, shared_codes.b,
                      shared_codes.d);
            dom(shared_codes, shared_codes.a.bits(), 2);
            if (!steps(FWF_FP32, FWF_E2M1, 2, FWF_E4M3FN, 2, expected) ||
                shared_codes.status() == SS_FAILED ||
                !shared_codes.d.assigned() || shared_codes.d.val() != expected)
              return false;
            Model same_unrepresentable(FWF_BF16, FWF_FP32, FWF_FP32, Observation::Spacing);
            ulp_steps(same_unrepresentable, FWF_BF16, same_unrepresentable.a,
                      same_unrepresentable.a, same_unrepresentable.d);
            dom(same_unrepresentable, same_unrepresentable.a.bits(),
                0x3f808000);
            if (same_unrepresentable.status() != SS_FAILED)
              return false;
            Model missing(FWF_BF16, FWF_BF16, FWF_BF16, Observation::Spacing);
            int caught = 0;
            try {
              ulp_spacing(missing, static_cast<FloatWordFormat>(99), missing.a,
                          missing.q);
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              ulp_steps(missing, FWF_FP32, missing.a, missing.b, missing.d);
            } catch (const Gecode::Word::WidthMismatch&) {
              ++caught;
            }
            FloatWordVar unbound(missing, FWF_E2M1);
            try {
              ulp_spacing(missing, FWF_BF16, unbound, missing.q);
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              ulp_steps(missing, FWF_BF16, unbound, missing.a, missing.d);
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            return caught == 4;
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
              : outputs(*this, 2, fp32 ? 32 : 16, WDT_UNSIGNED),
                digits(*this, fp32 ? 2 : 1, 0, (1 << 30) - 1) {
            unsigned int width = fp32 ? 32 : 16;
            BoolVar opposite(*this, 0, 1);
            WordValue one = fp32 ? 0x7f7fffff : 0x3f80;
            WordValue other = fp32 ? 0x7f7fffff : 0x4080;
            WordVar positive(*this, width, one, one),
                negative(*this, width, one | (WordValue(1) << (width - 1)),
                         one | (WordValue(1) << (width - 1))),
                second(*this, width, other, other),
                distance(*this, width, WDT_UNSIGNED);
            ite(*this, opposite, negative, positive, outputs[0]);
            ite(*this, opposite, positive, second, outputs[1]);
            FloatWordFormat f = fp32 ? FWF_FP32 : FWF_BF16;
            ulp_steps(*this, f, FloatWordVar(f, outputs[0]),
                      FloatWordVar(f, outputs[1]), distance);
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

        int error_digits(FloatWordFormat f) {
          return f == FWF_FP32 || f == FWF_BF16 ? 18 : 14;
        }
        int error_scale(FloatWordFormat f) { return 276 - format(f).fraction; }
        Integer lattice_value(Dyadic x) {
          Integer n =
              Integer(x.significand) * Integer::power_two(x.exponent + 149);
          return x.negative ? -n : n;
        }
        Integer error_numerator(FloatWordFormat f, FloatWordFormat fa,
                                WordValue a, FloatWordFormat fb, WordValue b,
                                FloatWordFormat fr, WordValue r) {
          const Integer difference =
              (lattice_value(decode(fa, a)) - lattice_value(decode(fb, b)))
                  .abs();
          return difference * Integer::power_two(127 - format(f).fraction -
                                                 spacing(f, decode(fr, r)));
        }
        IntArgs numerator_digits(FloatWordFormat f, const Integer& numerator) {
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
          SharedEncoding,
          RepeatedDigit,
          EncodingDigit
        };
        class ErrorModel : public IntLexMaximizeSpace {
        public:
          FloatWordVar a, b, r;
          IntVarArray digits;
          ErrorModel(FloatWordFormat f, FloatWordFormat fa, FloatWordFormat fb,
                     FloatWordFormat fr, WordDomainType storage = WDT_UNSIGNED,
                     ErrorAlias alias = ErrorAlias::Separate)
              : a(operand(*this, fa, storage)), b(operand(*this, fb, storage)),
                r(operand(*this, fr, storage)),
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
              b = a.reinterpret_bits(fb);
              break;
            case ErrorAlias::SharedEncoding:
              b = FloatWordVar(*this, fb, b.bits(), a.encoding());
              break;
            case ErrorAlias::RepeatedDigit:
              for (int i = 1; i < digits.size(); ++i)
                digits[i] = digits[0];
              break;
            case ErrorAlias::EncodingDigit:
              digits[4] = a.encoding();
              break;
            case ErrorAlias::Separate:
              break;
            }
            ulp_error(*this, f, a, b, r, digits);
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
            for (FloatWordVar x : {a, b, r}) {
              if (x.has_encoding())
                branch(*this, x.encoding(), INT_VAL_MIN());
              else
                branch(*this, WordVarArgs{x.bits()}, WORD_VAL_LSB());
            }
          }
        };

        bool check_ground_error(ErrorModel& root, FloatWordFormat f,
                                WordValue a, WordValue b, WordValue r,
                                bool wrong = false) {
          std::unique_ptr<ErrorModel> s(static_cast<ErrorModel*>(root.clone()));
          dom(*s, s->a.bits(), a);
          dom(*s, s->b.bits(), b);
          dom(*s, s->r.bits(), r);
          const bool finite = decode(s->a.format(), a).finite &&
                              decode(s->b.format(), b).finite &&
                              decode(s->r.format(), r).finite;
          IntArgs expected;
          if (finite) {
            expected = numerator_digits(f, error_numerator(f, s->a.format(), a,
                                                           s->b.format(), b,
                                                           s->r.format(), r));
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
          static bool enumerate(ErrorModel& root, FloatWordFormat f,
                                const std::set<WordValue>& codes) {
            root.inputs();
            DFS<ErrorModel> search(&root);
            std::set<std::array<WordValue, 3>> seen;
            while (std::unique_ptr<ErrorModel> s{search.next()}) {
              const WordValue a = s->a.bits().val(), b = s->b.bits().val(),
                              r = s->r.bits().val();
              if (!equal_digits(
                      s->digits,
                      numerator_digits(f, error_numerator(f, s->a.format(), a,
                                                          s->b.format(), b,
                                                          s->r.format(), r))) ||
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
            std::set<WordValue> fp4;
            for (WordValue i = 0; i < 16; ++i)
              fp4.insert(i);
            // Signed zeros, subnormals, the first exponent boundary, 1 and -1.
            const std::set<WordValue> fp8{0, 0x80, 1, 7, 8, 9, 0x38, 0xb8};
            IntArgs subset;
            for (WordValue c : fp8)
              subset << static_cast<int>(c);
            for (FloatWordFormat f : formats) {
              ErrorModel small(f, FWF_E2M1, FWF_E2M1, FWF_E2M1);
              if (!enumerate(small, f, fp4))
                return false;
              ErrorModel subset_root(f, FWF_E4M3FN, FWF_E4M3FN, FWF_E4M3FN);
              for (FloatWordVar x :
                   {subset_root.a, subset_root.b, subset_root.r})
                dom(subset_root, x.encoding(), IntSet(subset));
              if (!enumerate(subset_root, f, fp8))
                return false;
              // Every FP8 code in each role, including rejection of both NaNs.
              for (int role = 0; role < 3; ++role) {
                ErrorModel root(f, FWF_E4M3FN, FWF_E4M3FN, FWF_E4M3FN);
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
            for (FloatWordFormat f : formats) {
              ErrorModel full(f, FWF_FP32, FWF_FP32, FWF_FP32);
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
              if (error_numerator(f, FWF_FP32, 0xff7fffff, FWF_FP32, 0x7f7fffff,
                                  FWF_FP32, 0) != maximum ||
                  error_numerator(f, FWF_FP32, 0, FWF_FP32, 1, FWF_FP32,
                                  0x7f7fffff) != Integer(1))
                return false;
              const int first_bits = f == FWF_BINARY16 ? 29
                                     : f == FWF_E2M1   ? 15
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
              for (FloatWordFormat value_format : formats)
                for (int role = 0; role < 3; ++role) {
                  FloatWordFormat fs[] = {FWF_BF16, FWF_BINARY16, FWF_FP32};
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
              for (WordDomainType storage :
                   {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
                ErrorModel signed_words(f, FWF_FP32, FWF_BINARY16, FWF_BF16,
                                        storage);
                if (signed_words.status() == SS_FAILED ||
                    !check_ground_error(signed_words, f, 0xbf800001, 0x8001,
                                        0xbf80))
                  return false;
              }
            }
            ErrorModel fraction(FWF_BF16, FWF_FP32, FWF_FP32, FWF_FP32);
            if (fraction.status() == SS_FAILED ||
                !check_ground_error(fraction, FWF_BF16, 0x3f800000, 0x3f808000,
                                    0x3f800000) ||
                error_numerator(FWF_BF16, FWF_FP32, 0x3f800000, FWF_FP32,
                                0x3f808000, FWF_FP32, 0x3f800000) !=
                    Integer::power_two(error_scale(FWF_BF16) - 1))
              return false;
            return true;
          }
        };

        class ErrorInverse : public Base {
          static bool bounded_triples(IntRelType bound) {
            const FloatWordFormat f = FWF_E2M1;
            ErrorModel root(f, f, f, f);
            const IntArgs codes{0, 1, 2, 3, 4, 8, 9, 10};
            const IntArgs references{0, 2, 4, 6};
            dom(root, root.a.encoding(), IntSet(codes));
            dom(root, root.b.encoding(), IntSet(codes));
            dom(root, root.r.encoding(), IntSet(references));
            // Exactly 3/2, with one common scale even when reference varies.
            const Integer threshold =
                Integer(3) * Integer::power_two(error_scale(f) - 1);
            rel(root, root.digits, bound, numerator_digits(f, threshold));
            root.inputs();
            DFS<ErrorModel> search(&root);
            std::set<std::array<WordValue, 3>> seen, expected;
            for (int a : codes)
              for (int b : codes)
                for (int r : references) {
                  Integer q = error_numerator(f, f, a, f, b, f, r);
                  if (bound == IRT_EQ ? q == threshold : q <= threshold)
                    expected.insert({WordValue(a), WordValue(b), WordValue(r)});
                }
            while (std::unique_ptr<ErrorModel> s{search.next()}) {
              const WordValue a = s->a.bits().val(), b = s->b.bits().val(),
                              r = s->r.bits().val();
              if (!equal_digits(s->digits,
                                numerator_digits(
                                    f, error_numerator(f, f, a, f, b, f, r))) ||
                  !seen.insert({a, b, r}).second)
                return false;
            }
            return !search.stopped() && seen == expected;
          }
          static bool pruning(void) {
            ErrorModel endpoint(FWF_FP32, FWF_FP32, FWF_FP32, FWF_FP32);
            dom(endpoint, endpoint.a.bits(), 0);
            dom(endpoint, endpoint.r.bits(), 0x7f7fffff);
            rel(endpoint, endpoint.b.bits(), WRT_UGQ, 1);
            rel(endpoint, endpoint.b.bits(), WRT_ULQ, 7);
            // Fix one low bit so unsigned field extraction retains the
            // ordering.
            channel(endpoint, endpoint.b.bits(), 0, BoolVar(endpoint, 1, 1));
            if (endpoint.status() == SS_FAILED ||
                endpoint.b.bits().maximum() != 7)
              return false;
            rel(endpoint, endpoint.digits, IRT_LQ,
                numerator_digits(FWF_FP32, Integer(3)));
            if (endpoint.status() == SS_FAILED ||
                endpoint.b.bits().maximum() > 3 || endpoint.b.bits().assigned())
              return false;
            ErrorModel reference(FWF_BF16, FWF_FP32, FWF_FP32, FWF_BF16);
            dom(reference, reference.a.bits(), 0);
            dom(reference, reference.b.bits(), 1);
            dom(reference, reference.r.bits(), 0x4000, 0x40ff);
            if (reference.status() == SS_FAILED)
              return false;
            rel(reference, reference.digits, IRT_EQ,
                numerator_digits(FWF_BF16, Integer::power_two(126)));
            return reference.status() != SS_FAILED &&
                   !reference.r.bits().assigned() &&
                   !reference.r.bits().in(0x3f80) &&
                   !reference.r.bits().in(0x4080) &&
                   reference.r.bits().in(0x4000);
          }
          static bool holes(void) {
            for (WordDomainType storage :
                 {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
              ErrorModel root(FWF_E2M1, FWF_E2M1, FWF_E2M1, FWF_E2M1, storage);
              dom(root, root.a.encoding(), IntSet({2, 3, 4}));
              dom(root, root.b.encoding(), 0);
              dom(root, root.r.encoding(), 0);
              if (root.status() == SS_FAILED)
                return false;
              std::unique_ptr<ErrorModel> clone(
                  static_cast<ErrorModel*>(root.clone()));
              rel(*clone, clone->a.encoding(), IRT_NQ, 3);
              if (clone->status() == SS_FAILED || !root.a.encoding().in(3))
                return false;
              clone->inputs();
              DFS<ErrorModel> search(clone.get());
              std::set<int> seen;
              while (std::unique_ptr<ErrorModel> s{search.next()}) {
                int a = s->a.encoding().val();
                if (!equal_digits(s->digits,
                                  numerator_digits(
                                      FWF_E2M1, error_numerator(
                                                    FWF_E2M1, FWF_E2M1, a,
                                                    FWF_E2M1, 0, FWF_E2M1, 0))))
                  return false;
                seen.insert(a);
              }
              if (seen != std::set<int>{2, 4} || search.stopped())
                return false;
            }
            return true;
          }

        public:
          ErrorInverse(void) : Base("Word::Float::ULP::Error::Inverse") {}
          bool run(void) override {
            return bounded_triples(IRT_EQ) && bounded_triples(IRT_LQ) &&
                   pruning() && holes();
          }
        };

        class ErrorAliases : public Base {
        public:
          ErrorAliases(void) : Base("Word::Float::ULP::Error::Aliases") {}
          bool run(void) override {
            for (ErrorAlias alias :
                 {ErrorAlias::EqualEndpoints, ErrorAlias::ReferenceA, ErrorAlias::ReferenceB,
                  ErrorAlias::Reinterpreted}) {
              ErrorModel root(FWF_FP32, FWF_BF16,
                              alias == ErrorAlias::Reinterpreted ? FWF_BINARY16 : FWF_BF16,
                              FWF_BF16, WDT_UNSIGNED, alias);
              const bool shared_reference =
                  alias == ErrorAlias::ReferenceA || alias == ErrorAlias::ReferenceB;
              const WordValue a = shared_reference ? 0x3f80 : 0x3c00;
              const WordValue b = shared_reference ? 0x4080 : 0x3c00;
              const WordValue reference = alias == ErrorAlias::ReferenceB ? b : a;
              if (root.status() == SS_FAILED ||
                  !check_ground_error(root, FWF_FP32, a, b, reference) ||
                  !check_ground_error(root, FWF_FP32, 0x7f80, 0x7f80, 0x7f80))
                return false;
            }
            ErrorModel shared(FWF_E2M1, FWF_E2M1, FWF_E4M3FN, FWF_E2M1,
                              WDT_UNSIGNED, ErrorAlias::SharedEncoding);
            if (shared.status() == SS_FAILED ||
                !check_ground_error(shared, FWF_E2M1, 2, 2, 0))
              return false;
            ErrorModel repeated(FWF_E2M1, FWF_E2M1, FWF_E2M1, FWF_E2M1,
                                WDT_UNSIGNED, ErrorAlias::RepeatedDigit);
            if (repeated.status() == SS_FAILED ||
                !check_ground_error(repeated, FWF_E2M1, 2, 2, 0))
              return false;
            std::unique_ptr<ErrorModel> incompatible(
                static_cast<ErrorModel*>(repeated.clone()));
            dom(*incompatible, incompatible->a.bits(), 2);
            dom(*incompatible, incompatible->b.bits(), 0);
            dom(*incompatible, incompatible->r.bits(), 0);
            if (incompatible->status() != SS_FAILED)
              return false;
            ErrorModel cost_alias(FWF_E2M1, FWF_E2M1, FWF_E2M1, FWF_FP32,
                                  WDT_UNSIGNED, ErrorAlias::EncodingDigit);
            if (cost_alias.status() == SS_FAILED ||
                !check_ground_error(cost_alias, FWF_E2M1, 2, 0, 0x42000000))
              return false;
            // Clone preserves the cost/encoding identity, independently of the
            // source.
            std::unique_ptr<ErrorModel> clone(
                static_cast<ErrorModel*>(cost_alias.clone()));
            if (clone->digits[4].varimp() != clone->a.encoding().varimp() ||
                clone->digits[4].varimp() == cost_alias.digits[4].varimp())
              return false;
            int caught = 0;
            for (int length : {0, 13, 15, 18}) {
              try {
                ulp_error(cost_alias, FWF_E2M1, cost_alias.a, cost_alias.b,
                          cost_alias.r, IntVarArgs(cost_alias, length, 0, 1));
              } catch (const Gecode::Word::OutOfLimits&) {
                ++caught;
              }
            }
            try {
              ulp_error(cost_alias, static_cast<FloatWordFormat>(99),
                        cost_alias.a, cost_alias.b, cost_alias.r,
                        cost_alias.digits);
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              (void)ulp_error_digits(cost_alias,
                                     static_cast<FloatWordFormat>(99));
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            try {
              (void)ulp_error_scale(static_cast<FloatWordFormat>(99));
            } catch (const Gecode::Word::OutOfLimits&) {
              ++caught;
            }
            FloatWordVar unbound(cost_alias, FWF_E2M1);
            for (int role = 0; role < 3; ++role) {
              try {
                ulp_error(
                    cost_alias, FWF_E2M1, role == 0 ? unbound : cost_alias.a,
                    role == 1 ? unbound : cost_alias.b,
                    role == 2 ? unbound : cost_alias.r, cost_alias.digits);
              } catch (const Gecode::Word::OutOfLimits&) {
                ++caught;
              }
            }
            if (caught != 10)
              return false;
            for (int value : {-1, 1 << 15}) {
              ErrorModel invalid(FWF_E2M1, FWF_E2M1, FWF_E2M1, FWF_E2M1);
              rel(invalid, invalid.digits[0], IRT_EQ, value);
              if (invalid.status() != SS_FAILED)
                return false;
            }
            return true;
          }
        };

        class ErrorOptimize : public Base {
          static bool candidates(bool varying_reference) {
            ErrorModel root(FWF_BF16, FWF_BF16, FWF_BF16, FWF_BF16);
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
              rel(root, root.a.bits(), WRT_EQ, rows[i][0],
                  Reify(choices[i], RM_IMP));
              rel(root, root.b.bits(), WRT_EQ, rows[i][1],
                  Reify(choices[i], RM_IMP));
              rel(root, root.r.bits(), WRT_EQ, rows[i][2],
                  Reify(choices[i], RM_IMP));
              Integer q =
                  error_numerator(FWF_BF16, FWF_BF16, rows[i][0], FWF_BF16,
                                  rows[i][1], FWF_BF16, rows[i][2]);
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
                (best->a.bits().val() != 0x3f80 ||
                 best->b.bits().val() != 0x4080 ||
                 previous != Integer(384) * Integer::power_two(269)))
              return false;
            return true;
          }
          static bool thresholds(void) {
            using Gecode::Word::Arithmetic::Global::ceil_div;
            using Gecode::Word::Arithmetic::Global::floor_div;
            // A non-dyadic 1/3 threshold distinguishes floor from ceiling.
            for (FloatWordFormat f : formats) {
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
        ErrorInverse error_inverse;
        ErrorAliases error_aliases;
        ErrorOptimize error_optimize;
        ErrorTiny error_tiny;
        ErrorBoundaries error_boundaries;
        Tiny tiny;
        Boundaries boundary_tests;
        Lifecycle lifecycle;
        Optimize optimize;
      } // namespace ULP
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
