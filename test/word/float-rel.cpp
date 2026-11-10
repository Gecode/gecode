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
#include <vector>

namespace Test {
  namespace Word {
    namespace FloatRelations {
      using namespace Gecode;
      using Policy = FloatWordPolicy;

      WordVar variable(Space& home, WordDomainType format,
                       WordValueSetOption value_set = WVS_EXACT) {
        const unsigned int width = float_word_width(format);
        return WordVar(home, format, width <= 8 ? value_set : WVS_NONE);
      }
      WordVar constant(Space& home, WordDomainType format, WordValue code) {
        return float_code(home, format, code);
      }
      /// The values of a word of width at most 8
      std::set<WordValue> values(const WordVar& x) {
        std::set<WordValue> found;
        for (WordValue v = 0; v < (WordValue(1) << x.width()); ++v)
          if (x.in(v))
            found.insert(v);
        return found;
      }
      using MinMax = void (*)(Home, WordVar, WordVar, WordVar, IntVar,
                              const Policy&, IntPropLevel);
      const MinMax operations[] = {fp_minimum, fp_maximum, fp_minimum_number,
                                   fp_maximum_number};

      struct Value {
        WordValue code;
        long long value;
        bool nan = false, signaling = false;
      };
      // Integer units of 2^-9 (E4M3FN) or 2^-1 (E2M1), independent of the
      // propagator's encoding-order comparisons.
      Value decode_tiny(unsigned int code, WordDomainType format) {
        const unsigned int sign = format == WDT_E2M1 ? 8 : 128;
        unsigned int magnitude = code % (sign);
        long long value;
        if (format == WDT_E2M1) {
          const int halves[] = {0, 1, 2, 3, 4, 6, 8, 12};
          value = halves[magnitude];
        } else {
          unsigned int exponent = magnitude / 8, fraction = magnitude % 8;
          value = exponent == 0 ? fraction : (8 + fraction) << (exponent - 1);
        }
        return {code, code >= sign ? -value : value,
                format == WDT_E4M3FN && magnitude == 127, false};
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
      Value flush_input(Value v, WordDomainType format, const Policy& p) {
        unsigned int width = float_word_width(format);
        WordValue sign = WordValue(1) << (width - 1),
                  magnitude = v.code & (sign - 1);
        WordValue normal = format == WDT_E2M1     ? 2
                           : format == WDT_E4M3FN ? 8
                           : format == WDT_BF16   ? 128
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
      Expected expected_minmax(Value a, Value b, WordDomainType format,
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
          if (format == WDT_FP32)
            result |= 0x400000;
          if (format == WDT_BF16)
            result |= 0x40;
          if (p.nan == Policy::Canonical)
            result = format == WDT_FP32   ? 0x7fc00000
                     : format == WDT_BF16 ? 0x7fc0
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
        WordValue normal = format == WDT_E2M1     ? 2
                           : format == WDT_E4M3FN ? 8
                           : format == WDT_BF16   ? 128
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
        WordVar x, y;
        std::array<WordVar, 4> results;
        BoolVarArray truths;
        IntVarArray flags;
        PairSpace(WordDomainType format, WordValue a, WordValue b,
                  const Policy& p, IntPropLevel ipl)
            : x(constant(*this, format, a)), y(constant(*this, format, b)),
              truths(*this, 8, 0, 1), flags(*this, 12, 0, 31) {
          for (int r = 0; r < 8; ++r)
            fp_compare(*this, x, static_cast<FloatWordRelType>(r), y,
                       truths[r], flags[r], p, ipl);
          for (int i = 0; i < 4; ++i) {
            results[i] = variable(*this, format);
            operations[i](*this, x, y, results[i], flags[i + 8], p, ipl);
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
      bool check_pair(WordDomainType format, Value a, Value b,
                      const Policy& policy, IntPropLevel ipl) {
        PairSpace s(format, a.code, b.code, policy, ipl);
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
          if (!s.results[i].assigned() ||
              s.results[i].val() != e.code ||
              !s.flags[i + 8].assigned() || s.flags[i + 8].val() != e.flags)
            return false;
        }
        return true;
      }
      // Gradual, fully flushed with signaling comparison, and output flushed.
      std::array<Policy, 3> tiny_policies(void) {
        return {Policy(),
                Policy{Policy::RUP, Policy::SignedZero, Policy::SignedZero,
                       Policy::SaturateFinite, Policy::Positive,
                       Policy::Canonical, Policy::BeforeRounding,
                       Policy::Signaling},
                Policy{Policy::RTZ, Policy::Preserve, Policy::SignedZero,
                       Policy::Format, Policy::Signed, Policy::OperandFirst}};
      }

      class TinyPairs : public Base {
      public:
        TinyPairs() : Base("Word::Float::Relations::TinyPairs",TestTags::sweep()) {}
        bool run(void) override {
          for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
            for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN}) {
              const unsigned int count = 1U << float_word_width(f);
              for (unsigned int a = 0; a < count; ++a)
                for (unsigned int b = 0; b < count; ++b)
                  for (const Policy& p : tiny_policies())
                    if (!check_pair(f, decode_tiny(a, f), decode_tiny(b, f),
                                    p, ipl))
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
          for (WordDomainType f : {WDT_FP32, WDT_BF16})
            for (Value raw_a : fp32)
              for (Value raw_b : fp32) {
                Value a = raw_a, b = raw_b;
                if (f == WDT_BF16) {
                  a.code = (a.code >> 16) |
                           (a.value == 1 || a.value == -1 ? 1 : 0);
                  b.code = (b.code >> 16) |
                           (b.value == 1 || b.value == -1 ? 1 : 0);
                }
                for (const Policy& p : policies)
                  for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                    if (!check_pair(f, a, b, p, ipl))
                      return false;
              }
          return true;
        }
      } wide_pairs;

      class SymbolicSpace : public Space {
      public:
        WordVar x, y, z;
        BoolVar truth;
        IntVar flags;
        SymbolicSpace(WordDomainType f)
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
          for (WordDomainType f :
               {WDT_E2M1, WDT_E4M3FN, WDT_BF16, WDT_FP32}) {
            SymbolicSpace s(f);
            fp_compare(s, s.x, FWRT_EQ, s.x, s.truth, s.flags);
            rel(s, s.truth, IRT_EQ, 0);
            if (f == WDT_E2M1) {
              if (s.status() != SS_FAILED)
                return false;
            } else {
              if (s.status() == SS_FAILED)
                return false;
              if (float_word_width(f) <= 8) {
                if (values(s.x) != std::set<WordValue>{127, 255})
                  return false;
              } else {
                const WordValue exponent = f == WDT_FP32 ? 0x7f800000 : 0x7f80;
                if ((s.x.lo() & exponent) != exponent)
                  return false;
              }
            }
            SymbolicSpace order(f);
            const unsigned int sign = 1U << (float_word_width(f) - 1);
            dom(order, order.x, 0);
            fp_compare(order, order.x, FWRT_LE, order.y, order.truth,
                       order.flags);
            rel(order, order.truth, IRT_EQ, 1);
            if (order.status() == SS_FAILED ||
                (order.y.hi() & sign) != 0)
              return false;
            if (float_word_width(f) > 8) {
              SymbolicSpace root(f);
              const WordValue one = f == WDT_FP32 ? 0x3f800000 : 0x3f80;
              root.x = float_window(root, f, one, one + 3);
              root.y = float_window(root, f, one + 4, one + 7);
              fp_minimum(root, root.x, root.y, root.z, root.flags);
              dom(root, root.z, one + 2);
              if (root.status() == SS_FAILED || !root.x.assigned() ||
                  root.x.val() != one + 2 || root.y.assigned())
                return false;
              branch(root, WordVarArgs{root.y}, WORD_VAL_LSB());
              DFS<SymbolicSpace> search(&root);
              std::set<WordValue> actual;
              while (std::unique_ptr<SymbolicSpace> sol{search.next()})
                actual.insert(sol->y.val());
              if (actual !=
                  std::set<WordValue>{one + 4, one + 5, one + 6, one + 7})
                return false;
              continue;
            }
            SymbolicSpace root(f);
            const WordValArgs codes{0, 1, 2, 3, sign, sign + 1};
            dom(root, root.x, codes);
            dom(root, root.y, codes);
            fp_minimum(root, root.x, root.y, root.z, root.flags);
            dom(root, root.z, 1);
            if (root.status() == SS_FAILED || root.x.in(sign + 1))
              return false;
            std::unique_ptr<SymbolicSpace> clone(
                static_cast<SymbolicSpace*>(root.clone()));
            rel(*clone, clone->x, WRT_NQ, WordValue(2));
            branch(*clone, WordVarArgs{clone->x, clone->y},
                   WORD_VAL_LSB());
            DFS<SymbolicSpace> search(clone.get());
            std::set<std::pair<WordValue, WordValue>> actual, expected;
            while (std::unique_ptr<SymbolicSpace> sol{search.next()})
              actual.emplace(sol->x.val(), sol->y.val());
            for (WordValue a : codes)
              for (WordValue b : codes)
                if (a != 2 &&
                    expected_minmax(decode_tiny(static_cast<unsigned int>(a), f),
                                    decode_tiny(static_cast<unsigned int>(b), f),
                                    f, Policy(), 0)
                            .code == 1)
                  expected.emplace(a, b);
            if (actual != expected || !root.x.in(2))
              return false;
            // A result alias filters complete operand/result tuples, not
            // projections.
            SymbolicSpace alias(f);
            fp_minimum(alias, alias.x, alias.y, alias.x,
                       alias.flags);
            dom(alias, alias.x, 2);
            dom(alias, alias.y, 1);
            if (alias.status() != SS_FAILED)
              return false;
            // An operand that is also the result keeps only the values that
            // the selection returns unchanged.
            for (bool first : {true, false}) {
              SymbolicSpace kept(f);
              dom(kept, kept.x, WordValArgs{2, 4});
              dom(kept, kept.y, first ? 2 : 4);
              if (first)
                fp_minimum(kept, kept.x, kept.y, kept.x, kept.flags);
              else
                fp_maximum(kept, kept.y, kept.x, kept.x, kept.flags);
              if (kept.status() == SS_FAILED || !kept.x.assigned() ||
                  kept.x.val() != (first ? 2 : 4))
                return false;
            }
          }
          // False less includes NaN, and status restrictions retain its
          // correlation.
          for (int truth = 0; truth < 2; ++truth) {
            SymbolicSpace s(WDT_FP32);
            dom(s, s.x, 0x7fc12345);
            dom(s, s.y, 0x3f800000);
            fp_compare(s, s.x, FWRT_LE, s.y, s.truth, s.flags);
            rel(s, s.truth, IRT_EQ, truth);
            if ((s.status() == SS_FAILED) != (truth == 1))
              return false;
          }
          // LE is strict and LQ is not, as for the other relation types.
          for (FloatWordRelType r : {FWRT_LE, FWRT_LQ}) {
            SymbolicSpace s(WDT_FP32);
            dom(s, s.x, 0x3f800000);
            dom(s, s.y, 0x3f800000);
            fp_compare(s, s.x, r, s.y, s.truth, s.flags);
            if (s.status() == SS_FAILED || !s.truth.assigned() ||
                s.truth.val() != (r == FWRT_LQ))
              return false;
          }
          SymbolicSpace flags(WDT_BF16);
          dom(flags, flags.x, 0x7f81);
          dom(flags, flags.y, 0x3f80);
          fp_minimum_number(flags, flags.x, flags.y, flags.z,
                            flags.flags);
          rel(flags, flags.flags, IRT_EQ, 0);
          return flags.status() == SS_FAILED;
        }
      } inverse;
      class RootSpace : public Space {
      public:
        WordVar x, y, result;
        BoolVar truth;
        IntVar flags;
        RootSpace(WordDomainType f, WordValueSetOption value_set)
            : x(variable(*this, f, value_set)),
              y(variable(*this, f, value_set)),
              result(variable(*this, f, value_set)), truth(*this, 0, 1),
              flags(*this, 0, 31) {}
        RootSpace(RootSpace& s) : Space(s) {
          x.update(*this, s.x);
          y.update(*this, s.y);
          result.update(*this, s.result);
          truth.update(*this, s.truth);
          flags.update(*this, s.flags);
        }
        Space* copy(void) override { return new RootSpace(*this); }
      };

      /// Comparison relations 0 to 7, then the four selections 8 to 11
      void post_operation(RootSpace& s, const Policy& p, int operation,
                          IntPropLevel ipl) {
        if (operation < 8)
          fp_compare(s, s.x, static_cast<FloatWordRelType>(operation), s.y,
                     s.truth, s.flags, p, ipl);
        else
          operations[operation - 8](s, s.x, s.y, s.result, s.flags, p, ipl);
      }

      /// Supported output (truth or result code) and flags of a pair
      Expected expected_outcome(WordDomainType f, const Policy& p,
                                int operation, WordValue a, WordValue b) {
        const Value av = decode_tiny(static_cast<unsigned int>(a), f),
                    bv = decode_tiny(static_cast<unsigned int>(b), f);
        if (operation >= 8)
          return expected_minmax(av, bv, f, p, operation - 8);
        const bool truth = expected_predicate(flush_input(av, f, p),
                                              flush_input(bv, f, p), operation);
        const bool invalid =
            p.comparison == Policy::Quiet ? av.signaling || bv.signaling
                                          : av.nan || bv.nan;
        return {truth ? WordValue(1) : WordValue(0), invalid ? 1 : 0};
      }

      class TinyRoot : public Base {
        using Tuple = std::array<WordValue, 4>;

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
        /// a word of their hull and total-order bounds, and return the values
        /// of \a x.
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
            x = WordVar(home, width, x.domain_type(), bounds.first,
                        bounds.second, cube_bits(width, ones, any), WVS_NONE);
          }
          return values(x);
        }
        static std::set<Tuple> solutions(RootSpace& root, int operation) {
          branch(root, WordVarArgs{root.x, root.y},
                 WORD_VAL_LSB());
          if (operation < 8)
            branch(root, BoolVarArgs{root.truth}, BOOL_VAL_MIN());
          else
            branch(root, WordVarArgs{root.result}, WORD_VAL_LSB());
          branch(root, IntVarArgs{root.flags}, INT_VAL_MIN());
          DFS<RootSpace> search(&root);
          std::set<Tuple> found;
          while (std::unique_ptr<RootSpace> s{search.next()})
            found.insert({s->x.val(), s->y.val(),
                          operation < 8
                              ? static_cast<WordValue>(s->truth.val())
                              : s->result.val(),
                          static_cast<WordValue>(s->flags.val())});
          return found;
        }
        /// Test whether \a x keeps exactly, or the hull and bounds of, \a p
        static bool projected(const WordVar& x, const std::set<WordValue>& p,
                              WordValueSetOption value_set) {
          if (value_set == WVS_EXACT)
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
        // Propagation keeps every tuple that the oracle supports, search
        // finds exactly those tuples, and IPL_DOM keeps their projections.
        bool keeps_supported(WordDomainType f, WordValueSetOption value_set,
                             IntPropLevel ipl, const Policy& p,
                             int operation) {
          RootSpace root(f, value_set);
          const std::set<WordValue> xs = restrict(root, root.x, random_codes(f));
          const std::set<WordValue> ys = restrict(root, root.y, random_codes(f));
          std::set<WordValue> results;
          // A selection result may be its first (1) or second (2) operand.
          const int alias = operation < 8 ? 0 : static_cast<int>(_rand(3U));
          if (alias != 0)
            root.result = alias == 1 ? root.x : root.y;
          if (operation < 8) {
            const int truth = static_cast<int>(_rand(3U)) - 1;
            if (truth >= 0)
              rel(root, root.truth, IRT_EQ, truth);
            const int status = static_cast<int>(_rand(3U)) - 1;
            if (status >= 0)
              rel(root, root.flags, IRT_EQ, status);
          } else {
            if (alias == 0 && _rand(2U) == 0)
              results = restrict(root, root.result, random_codes(f));
            const int flags[] = {0, 1, 24, 25};
            if (_rand(2U) == 0)
              rel(root, root.flags, IRT_EQ, flags[_rand(4U)]);
          }
          std::set<Tuple> expected;
          for (WordValue a : xs)
            for (WordValue b : ys) {
              const Expected e = expected_outcome(f, p, operation, a, b);
              const bool admitted =
                  operation < 8 ? root.truth.in(static_cast<int>(e.code))
                                : results.empty() || results.count(e.code) != 0;
              const bool aliased = alias == 0 || e.code == (alias == 1 ? a : b);
              if (admitted && aliased && root.flags.in(e.flags))
                expected.insert(
                    {a, b, e.code, static_cast<WordValue>(e.flags)});
            }
          post_operation(root, p, operation, ipl);
          if (root.status() == SS_FAILED)
            return expected.empty();
          std::set<WordValue> px, py, pout;
          std::set<int> pflags;
          for (const Tuple& t : expected) {
            if (!root.x.in(t[0]) || !root.y.in(t[1]) ||
                (operation < 8 ? !root.truth.in(static_cast<int>(t[2]))
                               : !root.result.in(t[2])) ||
                !root.flags.in(static_cast<int>(t[3])))
              return false;
            px.insert(t[0]);
            py.insert(t[1]);
            pout.insert(t[2]);
            pflags.insert(static_cast<int>(t[3]));
          }
          if (ipl == IPL_DOM && !expected.empty()) {
            if (!projected(root.x, px, value_set) ||
                !projected(root.y, py, value_set) ||
                (operation >= 8 &&
                 !projected(root.result, pout, value_set)) ||
                (operation < 8 &&
                 root.truth.size() != static_cast<unsigned int>(pout.size())) ||
                root.flags.size() != static_cast<unsigned int>(pflags.size()))
              return false;
          }
          return solutions(root, operation) == expected;
        }

      public:
        TinyRoot() : Base("Word::Float::Relations::TinyRoot") {}
        bool run(void) override {
          for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN})
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                for (const Policy& p : tiny_policies())
                  for (int operation = 0; operation < 12; ++operation)
                    for (int trial = 0; trial < 3; ++trial)
                      if (!keeps_supported(f, value_set, ipl, p, operation))
                        return false;
          return true;
        }
      } tiny_root;

      class TinyBounds : public Base {
        // E4M3FN encodings of small values and the NaN.
        static constexpr WordValue zero = 0x00, half = 0x30, one = 0x38,
                                   one_half = 0x3c, two = 0x40, three = 0x44,
                                   four = 0x48, minus_one = 0xb8, nan = 0x7f;

        static bool strict_order(WordValueSetOption value_set) {
          RootSpace s(WDT_E4M3FN, value_set);
          dom(s, s.x, WordValArgs{one, one_half, two, three, four});
          dom(s, s.y, WordValArgs{zero, half, two});
          rel(s, s.truth, IRT_EQ, 1);
          fp_compare(s, s.x, FWRT_LE, s.y, s.truth, s.flags);
          if (s.status() == SS_FAILED || !s.y.assigned() ||
              s.y.val() != two || s.x.maximum() >= two ||
              !s.x.in(one) || !s.x.in(one_half))
            return false;
          return value_set == WVS_NONE ||
                 values(s.x) == std::set<WordValue>{one, one_half};
        }
        static bool decided_truth(WordValueSetOption value_set) {
          RootSpace s(WDT_E4M3FN, value_set);
          dom(s, s.x, WordValArgs{one, one_half, two});
          dom(s, s.y, WordValArgs{three, four});
          fp_compare(s, s.x, FWRT_LE, s.y, s.truth, s.flags);
          return s.status() != SS_FAILED && s.truth.assigned() &&
                 s.truth.val() == 1 && s.flags.assigned() &&
                 s.flags.val() == 0 && !s.x.assigned() &&
                 !s.y.assigned();
        }
        static bool quiet_signaling(WordValueSetOption value_set) {
          const Policy signaling = tiny_policies()[1];
          RootSpace s(WDT_E4M3FN, value_set);
          dom(s, s.x, WordValArgs{minus_one, one, two});
          dom(s, s.y, WordValArgs{zero, three});
          fp_compare(s, s.x, FWRT_EQ, s.y, s.truth, s.flags, signaling);
          return s.status() != SS_FAILED && s.flags.assigned() &&
                 s.flags.val() == 0 && !s.truth.assigned();
        }
        static bool equal_levels(void) {
          for (IntPropLevel ipl : {IPL_DEF, IPL_DOM}) {
            RootSpace s(WDT_E4M3FN, WVS_EXACT);
            dom(s, s.x, WordValArgs{one, two, three});
            dom(s, s.y, WordValArgs{one, three});
            rel(s, s.truth, IRT_EQ, 1);
            fp_compare(s, s.x, FWRT_EQ, s.y, s.truth, s.flags, ipl);
            if (s.status() == SS_FAILED || s.x.in(two) != (ipl == IPL_DEF) ||
                !s.x.in(one) || !s.x.in(three))
              return false;
          }
          return true;
        }
        static bool fixed_minimum(WordValueSetOption value_set) {
          RootSpace s(WDT_E4M3FN, value_set);
          dom(s, s.x, WordValArgs{minus_one, one, two, three});
          dom(s, s.y, WordValArgs{half, one, two});
          dom(s, s.result, one);
          fp_minimum(s, s.x, s.y, s.result, s.flags);
          if (s.status() == SS_FAILED || s.x.in(minus_one) ||
              s.y.in(half) || !s.x.in(one) ||
              !s.x.in(three) || !s.y.in(one) ||
              !s.y.in(two))
            return false;
          return value_set == WVS_NONE ||
                 (values(s.x) == std::set<WordValue>{one, two, three} &&
                  values(s.y) == std::set<WordValue>{one, two});
        }
        static bool nan_selections(WordValueSetOption value_set) {
          // A non-NaN maximum excludes the NaN operand.
          RootSpace maximum(WDT_E4M3FN, value_set);
          dom(maximum, maximum.x, WordValArgs{one, nan});
          dom(maximum, maximum.y, WordValArgs{two, three});
          dom(maximum, maximum.result, WordValArgs{two, three});
          fp_maximum(maximum, maximum.x, maximum.y, maximum.result,
                     maximum.flags);
          if (maximum.status() == SS_FAILED || !maximum.x.assigned() ||
              maximum.x.val() != one || maximum.y.assigned())
            return false;
          // A NaN operand passes the other operand through.
          RootSpace minimum(WDT_E4M3FN, value_set);
          dom(minimum, minimum.x, nan);
          dom(minimum, minimum.y, WordValArgs{one, two});
          fp_minimum_number(minimum, minimum.x, minimum.y,
                            minimum.result, minimum.flags);
          if (minimum.status() == SS_FAILED || !minimum.result.in(one) ||
              !minimum.result.in(two) || minimum.result.in(nan) ||
              minimum.result.in(zero))
            return false;
          // A NaN number selection needs two NaN operands.
          RootSpace number(WDT_E4M3FN, value_set);
          dom(number, number.x, WordValArgs{one, nan});
          dom(number, number.y, WordValArgs{two, nan});
          dom(number, number.result, nan);
          fp_maximum_number(number, number.x, number.y,
                            number.result, number.flags);
          return number.status() != SS_FAILED && number.x.assigned() &&
                 number.x.val() == nan && number.y.assigned() &&
                 number.y.val() == nan;
        }

      public:
        TinyBounds() : Base("Word::Float::Relations::TinyBounds") {}
        bool run(void) override {
          for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
            if (!strict_order(value_set) || !decided_truth(value_set) ||
                !quiet_signaling(value_set) || !fixed_minimum(value_set) ||
                !nan_selections(value_set))
              return false;
          return equal_levels();
        }
      } tiny_bounds;

      /// Complete solutions of comparisons and selections on float-typed
      /// words whose windows are negative or cross zero
      class FloatTyped : public Base {
        using Tuple = std::array<WordValue, 4>;
        // Total-order window of encodings
        struct Range {
          WordValue minimum, maximum;
        };
        struct Case {
          Range x, y, result;
        };
        // Wide values as signed magnitudes, which order like the numbers.
        static Value decode(WordValue code, WordDomainType f) {
          if (float_word_width(f) <= 8)
            return decode_tiny(static_cast<unsigned int>(code), f);
          const WordValue sign = WordValue(1) << (float_word_width(f) - 1);
          const WordValue infinity = f == WDT_FP32 ? 0x7f800000 : 0x7f80;
          const WordValue quiet = f == WDT_FP32 ? 0x400000 : 0x40;
          const WordValue magnitude = code & (sign - 1);
          const bool nan = magnitude > infinity;
          const long long value = static_cast<long long>(magnitude);
          return {code, (code & sign) ? -value : value, nan,
                  nan && (magnitude & quiet) == 0};
        }
        static Expected outcome(WordDomainType f, const Policy& p,
                                int operation, WordValue a, WordValue b) {
          const Value av = decode(a, f), bv = decode(b, f);
          if (operation >= 8)
            return expected_minmax(av, bv, f, p, operation - 8);
          const bool truth = expected_predicate(flush_input(av, f, p),
                                                flush_input(bv, f, p),
                                                operation);
          const bool invalid =
              p.comparison == Policy::Quiet ? av.signaling || bv.signaling
                                            : av.nan || bv.nan;
          return {truth ? WordValue(1) : WordValue(0), invalid ? 1 : 0};
        }
        static std::set<Tuple> solutions(RootSpace& root, int operation) {
          branch(root, WordVarArgs{root.x, root.y},
                 WORD_VAL_LSB());
          if (operation < 8)
            branch(root, BoolVarArgs{root.truth}, BOOL_VAL_MIN());
          else
            branch(root, WordVarArgs{root.result}, WORD_VAL_LSB());
          branch(root, IntVarArgs{root.flags}, INT_VAL_MIN());
          DFS<RootSpace> search(&root);
          std::set<Tuple> found;
          while (std::unique_ptr<RootSpace> s{search.next()})
            found.insert({s->x.val(), s->y.val(),
                          operation < 8
                              ? static_cast<WordValue>(s->truth.val())
                              : s->result.val(),
                          static_cast<WordValue>(s->flags.val())});
          return found;
        }
        bool check(WordDomainType f, const Case& c, const Policy& p,
                   int operation, WordValueSetOption value_set,
                   IntPropLevel ipl) {
          const unsigned int width = float_word_width(f);
          RootSpace root(f, value_set);
          root.x = float_window(root, f, c.x.minimum, c.x.maximum, value_set);
          root.y = float_window(root, f, c.y.minimum, c.y.maximum, value_set);
          root.result = float_window(root, f, c.result.minimum,
                                     c.result.maximum, value_set);
          std::set<Tuple> expected;
          for (WordValue a : float_order_range(width, c.x.minimum, c.x.maximum))
            for (WordValue b :
                 float_order_range(width, c.y.minimum, c.y.maximum)) {
              const Expected e = outcome(f, p, operation, a, b);
              if (operation < 8 || root.result.in(e.code))
                expected.insert({a, b, e.code, WordValue(e.flags)});
            }
          post_operation(root, p, operation, ipl);
          if (solutions(root, operation) == expected)
            return true;
          if (opt.log)
            olog << "float-typed relation " << f << " operation " << operation
                 << " value set " << static_cast<int>(value_set) << " ipl "
                 << ipl << std::hex << " x " << c.x.minimum << ".."
                 << c.x.maximum << " y " << c.y.minimum << ".."
                 << c.y.maximum << std::dec << '\n';
          return false;
        }
        bool format_cases(WordDomainType f) {
          const unsigned int width = float_word_width(f);
          const WordValue s = WordValue(1) << (width - 1);
          const Range all{s | (s - 1), s - 1};
          // Per format: a cross-zero and a negative window with the full
          // result, then windows that reach the NaNs below negative
          // infinity and the zeros, with a cross-zero result window.
          std::vector<Case> cases;
          switch (f) {
          case WDT_E2M1:
            cases = {{{s | 3, 3}, {s | 5, s | 1}, all},
                     {{s | 6, s | 2}, {s | 2, 4}, {s | 4, 2}}};
            break;
          case WDT_E4M3FN:
            cases = {{{s | 6, 6}, {s | 0x3c, s | 0x38}, all},
                     {{s | 0x7f, s | 0x7d}, {s | 2, 2}, {s | 0x40, 0x10}}};
            break;
          case WDT_BF16:
            cases = {{{s | 3, 3}, {s | 0x3f82, s | 0x3f80}, all},
                     {{s | 0x7f81, s | 0x7f7f}, {s | 2, 2}, {s | 0x7f80, 0x10}}};
            break;
          default:
            cases = {{{s | 3, 3}, {s | 0x3f800002, s | 0x3f800000}, all},
                     {{s | 0x7f800001, s | 0x7f7fffff},
                      {s | 2, 2},
                      {s | 0x7f800000, 0x10}}};
            break;
          }
          for (const Case& c : cases)
            for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
              for (IntPropLevel ipl : {IPL_DEF, IPL_DOM})
                for (const Policy& p : tiny_policies())
                  for (int operation = 0; operation < 12; ++operation)
                    if ((width <= 8 || value_set == WVS_NONE) &&
                        !check(f, c, p, operation, value_set, ipl))
                      return false;
          return true;
        }

      public:
        FloatTyped() : Base("Word::Float::Relations::FloatTyped") {}
        bool run(void) override {
          for (WordDomainType f : {WDT_E2M1, WDT_E4M3FN, WDT_BF16, WDT_FP32})
            if (!format_cases(f))
              return false;
          return true;
        }
      } float_typed;

      class Errors : public Base {
      public:
        Errors() : Base("Word::Float::Relations::Errors") {}
        bool run(void) override {
          SymbolicSpace invalid(WDT_FP32);
          int caught = 0;
          try {
            fp_compare(invalid, invalid.x,
                       static_cast<FloatWordRelType>(99), invalid.y,
                       invalid.truth);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            fp_compare(invalid, invalid.x, FWRT_EQ,
                       variable(invalid, WDT_BF16), invalid.truth);
          } catch (const Gecode::Word::FormatMismatch&) {
            ++caught;
          }
          try {
            fp_compare(invalid, variable(invalid, WDT_BINARY16),
                       FWRT_EQ, variable(invalid, WDT_BINARY16),
                       invalid.truth);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          return caught == 3;
        }
      } errors;
    } // namespace FloatRelations
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
