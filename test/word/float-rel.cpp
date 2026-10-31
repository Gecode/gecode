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
    namespace FloatRelations {
      using namespace Gecode;
      using Policy = FloatWordPolicy;

      FloatWordVar variable(Space& home, FloatWordFormat format) {
        FloatWordVar x(home, format);
        if (float_word_width(format) <= 8)
          return FloatWordVar(
              home, format, x.bits(),
              IntVar(home, 0, (1 << float_word_width(format)) - 1));
        return x;
      }
      FloatWordVar constant(Space& home, FloatWordFormat format,
                            WordValue code) {
        FloatWordVar x = FloatWordVar::from_bits(home, format, code);
        if (float_word_width(format) <= 8)
          return FloatWordVar(
              home, format, x.bits(),
              IntVar(home, static_cast<int>(code), static_cast<int>(code)));
        return x;
      }
      using MinMax = void (*)(Home, const Policy&, FloatWordVar, FloatWordVar,
                              FloatWordVar, IntVar);
      const MinMax operations[] = {fp_minimum, fp_maximum, fp_minimum_number,
                                   fp_maximum_number};

      struct Value {
        WordValue code;
        long long value;
        bool nan = false, signaling = false;
      };
      // Integer units of 2^-9 (E4M3FN) or 2^-1 (E2M1), independent of the
      // propagator's encoding-order comparisons.
      Value decode_tiny(unsigned int code, FloatWordFormat format) {
        const unsigned int sign = format == FWF_E2M1 ? 8 : 128;
        unsigned int magnitude = code % (sign);
        long long value;
        if (format == FWF_E2M1) {
          const int halves[] = {0, 1, 2, 3, 4, 6, 8, 12};
          value = halves[magnitude];
        } else {
          unsigned int exponent = magnitude / 8, fraction = magnitude % 8;
          value = exponent == 0 ? fraction : (8 + fraction) << (exponent - 1);
        }
        return {code, code >= sign ? -value : value,
                format == FWF_E4M3FN && magnitude == 127, false};
      }
      bool expected_predicate(const Value& a, const Value& b, int relation) {
        const bool ordered = !a.nan && !b.nan;
        const bool table[] = {ordered && a.value == b.value,
                              !ordered || a.value != b.value,
                              ordered && a.value < b.value,
                              ordered && a.value <= b.value,
                              ordered && a.value > b.value,
                              ordered && a.value >= b.value,
                              !ordered,
                              ordered};
        return table[relation];
      }
      Value flush_input(Value v, FloatWordFormat format, const Policy& p) {
        unsigned int width = float_word_width(format);
        WordValue sign = WordValue(1) << (width - 1),
                  magnitude = v.code & (sign - 1);
        WordValue normal = format == FWF_E2M1     ? 2
                           : format == FWF_E4M3FN ? 8
                           : format == FWF_BF16   ? 128
                                                  : 0x800000;
        if (p.input_subnormals == Policy::SignedZero && magnitude < normal) {
          v.code &= sign;
          v.value = 0;
        }
        return v;
      }
      struct Expected {
        WordValue code;
        int flags;
      };
      Expected expected_minmax(Value a, Value b, FloatWordFormat format,
                               const Policy& p, int operation) {
        a = flush_input(a, format, p);
        b = flush_input(b, format, p);
        bool maximum = (operation % 2) != 0, number = operation >= 2;
        WordValue result;
        int flags = (a.signaling || b.signaling) ? 1 : 0;
        if ((a.nan || b.nan) && (!number || (a.nan && b.nan))) {
          Value selected = a.nan ? a : b;
          if (p.nan == Policy::SignalFirst && flags)
            selected = a.signaling ? a : b;
          result = selected.code;
          if (format == FWF_FP32)
            result |= 0x400000;
          if (format == FWF_BF16)
            result |= 0x40;
          if (p.nan == Policy::Canonical)
            result = format == FWF_FP32   ? 0x7fc00000
                     : format == FWF_BF16 ? 0x7fc0
                                          : 0x7f;
        } else if (a.nan || b.nan)
          result = a.nan ? b.code : a.code;
        else if (a.value == 0 && b.value == 0)
          result = maximum ? (a.code & b.code) : (a.code | b.code);
        else
          result = (maximum ? a.value >= b.value : a.value <= b.value)
                       ? a.code
                       : b.code;
        WordValue sign = WordValue(1) << (float_word_width(format) - 1),
                  magnitude = result & (sign - 1);
        WordValue normal = format == FWF_E2M1     ? 2
                           : format == FWF_E4M3FN ? 8
                           : format == FWF_BF16   ? 128
                                                  : 0x800000;
        if (p.output_subnormals == Policy::SignedZero && magnitude > 0 &&
            magnitude < normal) {
          result &= sign;
          flags |= 24;
        }
        if (p.zeros == Policy::Positive && (result & (sign - 1)) == 0)
          result = 0;
        return {result, flags};
      }

      class PairSpace : public Space {
      public:
        FloatWordVar x, y;
        std::array<FloatWordVar, 4> results;
        BoolVarArray truths;
        IntVarArray flags;
        PairSpace(FloatWordFormat format, WordValue a, WordValue b,
                  const Policy& p)
            : x(constant(*this, format, a)), y(constant(*this, format, b)),
              truths(*this, 8, 0, 1), flags(*this, 12, 0, 31) {
          for (int r = 0; r < 8; ++r)
            fp_compare(*this, p, x, static_cast<FloatWordRelType>(r), y,
                       truths[r], flags[r]);
          for (int i = 0; i < 4; ++i) {
            results[i] = variable(*this, format);
            operations[i](*this, p, x, y, results[i], flags[i + 8]);
          }
        }
        PairSpace(PairSpace& s) : Space(s) {
          x.update(*this, s.x);
          y.update(*this, s.y);
          truths.update(*this, s.truths);
          flags.update(*this, s.flags);
          for (int i = 0; i < 4; ++i)
            results[i].update(*this, s.results[i]);
        }
        Space* copy(void) override { return new PairSpace(*this); }
      };
      bool check_pair(FloatWordFormat format, Value a, Value b,
                      const Policy& policy) {
        PairSpace s(format, a.code, b.code, policy);
        if (s.status() == SS_FAILED)
          return false;
        Value av = flush_input(a, format, policy),
              bv = flush_input(b, format, policy);
        const int invalid = policy.comparison == Policy::Quiet
                                ? a.signaling || b.signaling
                                : a.nan || b.nan;
        for (int r = 0; r < 8; ++r)
          if (!s.truths[r].assigned() ||
              s.truths[r].val() != expected_predicate(av, bv, r) ||
              !s.flags[r].assigned() || s.flags[r].val() != invalid)
            return false;
        for (int i = 0; i < 4; ++i) {
          Expected e = expected_minmax(a, b, format, policy, i);
          if (!s.results[i].bits().assigned() ||
              s.results[i].bits().val() != e.code ||
              !s.flags[i + 8].assigned() || s.flags[i + 8].val() != e.flags)
            return false;
        }
        return true;
      }
      class TinyPairs : public Base {
      public:
        TinyPairs() : Base("Word::Float::Relations::TinyPairs",TestTags::sweep()) {}
        bool run(void) override {
          const Policy gradual;
          const Policy output_flushed{Policy::RTZ,
                                      Policy::Preserve, Policy::SignedZero,
                                      Policy::Format, Policy::Signed,
                                      Policy::OperandFirst};
          const Policy flushed{
              Policy::RUP, Policy::SignedZero,
              Policy::SignedZero, Policy::SaturateFinite, Policy::Positive,
              Policy::Canonical, Policy::BeforeRounding, Policy::Signaling};
          for (FloatWordFormat f : {FWF_E2M1, FWF_E4M3FN}) {
            const unsigned int count = 1U << float_word_width(f);
            for (unsigned int a = 0; a < count; ++a)
              for (unsigned int b = 0; b < count; ++b)
                for (const Policy* p : {&gradual, &flushed, &output_flushed})
                  if (!check_pair(f, decode_tiny(a, f), decode_tiny(b, f),
                                  *p))
                    return false;
          }
          return true;
        }
      } tiny_pairs;

      class WidePairs : public Base {
      public:
        WidePairs() : Base("Word::Float::Relations::WidePairs") {}
        bool run(void) override {
          // Values are ordered labels, not host floating computations. Zeros
          // share 0.
          const Value fp32[] = {{0xff800000, -6},
                                {0xff7fffff, -5},
                                {0xbf800000, -4},
                                {0x80800000, -3},
                                {0x807fffff, -2},
                                {0x80000001, -1},
                                {0x80000000, 0},
                                {0, 0},
                                {1, 1},
                                {0x007fffff, 2},
                                {0x00800000, 3},
                                {0x3f800000, 4},
                                {0x7f7fffff, 5},
                                {0x7f800000, 6},
                                {0x7f812345, 0, true, true},
                                {0xff812345, 0, true, true},
                                {0x7fc12345, 0, true},
                                {0xffc54321, 0, true}};
          const Policy policies[] = {
              Policy(),
              Policy{Policy::RTZ, Policy::Preserve,
                     Policy::SignedZero, Policy::Format, Policy::Positive,
                     Policy::OperandFirst, Policy::BeforeRounding,
                     Policy::Signaling},
              Policy{Policy::RDN, Policy::SignedZero,
                     Policy::Preserve, Policy::SaturateFinite, Policy::Signed,
                     Policy::Canonical}};
          for (FloatWordFormat f : {FWF_FP32, FWF_BF16})
            for (Value raw_a : fp32)
              for (Value raw_b : fp32) {
                Value a = raw_a, b = raw_b;
                if (f == FWF_BF16) {
                  a.code = (a.code >> 16) |
                           (a.value == 1 || a.value == -1 ? 1 : 0);
                  b.code = (b.code >> 16) |
                           (b.value == 1 || b.value == -1 ? 1 : 0);
                }
                for (const Policy& p : policies)
                  if (!check_pair(f, a, b, p))
                    return false;
              }
          return true;
        }
      } wide_pairs;

      class SymbolicSpace : public Space {
      public:
        FloatWordVar x, y, z;
        BoolVar truth;
        IntVar flags;
        SymbolicSpace(FloatWordFormat f)
            : x(variable(*this, f)), y(variable(*this, f)),
              z(variable(*this, f)), truth(*this, 0, 1), flags(*this, 0, 31) {
        }
        SymbolicSpace(SymbolicSpace& s) : Space(s) {
          x.update(*this, s.x);
          y.update(*this, s.y);
          z.update(*this, s.z);
          truth.update(*this, s.truth);
          flags.update(*this, s.flags);
        }
        Space* copy(void) override { return new SymbolicSpace(*this); }
      };
      class Inverse : public Base {
      public:
        Inverse() : Base("Word::Float::Relations::Inverse") {}
        bool run(void) override {
          for (FloatWordFormat f :
               {FWF_E2M1, FWF_E4M3FN, FWF_BF16, FWF_FP32}) {
            SymbolicSpace s(f);
            fp_compare(s, Policy(), s.x, FWRT_EQ, s.x, s.truth, s.flags);
            rel(s, s.truth, IRT_EQ, 0);
            if (f == FWF_E2M1) {
              if (s.status() != SS_FAILED)
                return false;
            } else {
              if (s.status() == SS_FAILED)
                return false;
              if (float_word_width(f) <= 8) {
                if (s.x.encoding().size() != 2 || !s.x.encoding().in(127) ||
                    !s.x.encoding().in(255))
                  return false;
              } else {
                const WordValue exponent =
                    f == FWF_FP32 ? 0x7f800000 : 0x7f80;
                if ((s.x.bits().lo() & exponent) != exponent)
                  return false;
              }
            }
            SymbolicSpace order(f);
            const unsigned int sign = 1U << (float_word_width(f) - 1);
            dom(order, order.x.bits(), 0);
            fp_compare(order, Policy(), order.x, FWRT_LE, order.y, order.truth,
                       order.flags);
            rel(order, order.truth, IRT_EQ, 1);
            if (order.status() == SS_FAILED ||
                (order.y.bits().hi() & sign) != 0)
              return false;
            if (float_word_width(f) > 8) {
              SymbolicSpace root(f);
              const unsigned int width = float_word_width(f);
              const WordValue one = f == FWF_FP32 ? 0x3f800000 : 0x3f80;
              root.x = FloatWordVar(
                  f, WordVar(root, width, WDT_UNSIGNED, one, one + 3));
              root.y = FloatWordVar(
                  f, WordVar(root, width, WDT_UNSIGNED, one + 4, one + 7));
              fp_minimum(root, Policy(), root.x, root.y, root.z, root.flags);
              dom(root, root.z.bits(), one + 2);
              if (root.status() == SS_FAILED || !root.x.bits().assigned() ||
                  root.x.bits().val() != one + 2 || root.y.bits().assigned())
                return false;
              branch(root, WordVarArgs{root.y.bits()}, WORD_VAL_LSB());
              DFS<SymbolicSpace> search(&root);
              std::set<WordValue> actual;
              while (std::unique_ptr<SymbolicSpace> sol{search.next()})
                actual.insert(sol->y.bits().val());
              if (actual !=
                  std::set<WordValue>{one + 4, one + 5, one + 6, one + 7})
                return false;
              continue;
            }
            SymbolicSpace root(f);
            const int codes[] = {0,
                                 1,
                                 2,
                                 3,
                                 static_cast<int>(sign),
                                 static_cast<int>(sign + 1)};
            dom(root, root.x.encoding(), IntSet(codes, 6));
            dom(root, root.y.encoding(), IntSet(codes, 6));
            fp_minimum(root, Policy(), root.x, root.y, root.z, root.flags);
            dom(root, root.z.bits(), 1);
            if (root.status() == SS_FAILED || root.x.encoding().in(sign + 1))
              return false;
            std::unique_ptr<SymbolicSpace> clone(
                static_cast<SymbolicSpace*>(root.clone()));
            rel(*clone, clone->x.encoding(), IRT_NQ, 2);
            branch(*clone, WordVarArgs{clone->x.bits(), clone->y.bits()},
                   WORD_VAL_LSB());
            DFS<SymbolicSpace> search(clone.get());
            std::set<std::pair<int, int>> actual, expected;
            while (std::unique_ptr<SymbolicSpace> sol{search.next()})
              actual.emplace(sol->x.encoding().val(),
                             sol->y.encoding().val());
            for (int a : codes)
              for (int b : codes)
                if (a != 2 &&
                    expected_minmax(decode_tiny(a, f), decode_tiny(b, f), f,
                                    Policy(), 0)
                            .code == 1)
                  expected.emplace(a, b);
            if (actual != expected || !root.x.encoding().in(2))
              return false;
            // A result alias filters complete operand/result tuples, not
            // projections.
            SymbolicSpace alias(f);
            fp_minimum(alias, Policy(), alias.x, alias.y, alias.x,
                       alias.flags);
            dom(alias, alias.x.bits(), 2);
            dom(alias, alias.y.bits(), 1);
            if (alias.status() != SS_FAILED)
              return false;
          }
          // False less includes NaN, and status restrictions retain its
          // correlation.
          for (int truth = 0; truth < 2; ++truth) {
            SymbolicSpace s(FWF_FP32);
            dom(s, s.x.bits(), 0x7fc12345);
            dom(s, s.y.bits(), 0x3f800000);
            fp_compare(s, Policy(), s.x, FWRT_LE, s.y, s.truth, s.flags);
            rel(s, s.truth, IRT_EQ, truth);
            if ((s.status() == SS_FAILED) != (truth == 1))
              return false;
          }
          // LE is strict and LQ is not, as for the other relation types.
          for (FloatWordRelType r : {FWRT_LE, FWRT_LQ}) {
            SymbolicSpace s(FWF_FP32);
            dom(s, s.x.bits(), 0x3f800000);
            dom(s, s.y.bits(), 0x3f800000);
            fp_compare(s, Policy(), s.x, r, s.y, s.truth, s.flags);
            if (s.status() == SS_FAILED || !s.truth.assigned() ||
                s.truth.val() != (r == FWRT_LQ))
              return false;
          }
          SymbolicSpace flags(FWF_BF16);
          dom(flags, flags.x.bits(), 0x7f81);
          dom(flags, flags.y.bits(), 0x3f80);
          fp_minimum_number(flags, Policy(), flags.x, flags.y, flags.z,
                            flags.flags);
          rel(flags, flags.flags, IRT_EQ, 0);
          return flags.status() == SS_FAILED;
        }
      } inverse;
      class Errors : public Base {
      public:
        Errors() : Base("Word::Float::Relations::Errors") {}
        bool run(void) override {
          SymbolicSpace invalid(FWF_FP32);
          int caught = 0;
          try {
            fp_compare(invalid, Policy(), invalid.x,
                       static_cast<FloatWordRelType>(99), invalid.y,
                       invalid.truth);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            fp_compare(invalid, Policy(), invalid.x, FWRT_EQ,
                       variable(invalid, FWF_BF16), invalid.truth);
          } catch (const Gecode::Word::FormatMismatch&) {
            ++caught;
          }
          try {
            fp_compare(invalid, Policy(), variable(invalid, FWF_BINARY16),
                       FWRT_EQ, variable(invalid, FWF_BINARY16),
                       invalid.truth);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            fp_minimum(invalid, Policy(), FloatWordVar(invalid, FWF_E2M1),
                       FloatWordVar(invalid, FWF_E2M1),
                       FloatWordVar(invalid, FWF_E2M1));
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          return caught == 4;
        }
      } errors;
    } // namespace FloatRelations
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
