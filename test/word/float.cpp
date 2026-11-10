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
#include <gecode/float-word.hh>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Test {
  namespace Word {
    namespace Float {
      using namespace Gecode;

      class ObservationSpace : public Space {
      public:
        WordVar x, negative, absolute, alias;
        BoolVarArray classes;
        ObservationSpace(WordDomainType format,
                         WordValueSetOption value_set = WVS_EXACT)
            : x(float_variable(*this, format, value_set)),
              negative(float_variable(*this, format, value_set)),
              absolute(float_variable(*this, format, value_set)),
              classes(*this, 9, 0, 1) {
          alias = x;
          fp_negate(*this, x, negative);
          fp_abs(*this, negative, absolute);
          for (int c = 0; c < 9; ++c)
            fp_classify(*this, alias, static_cast<FloatWordClass>(c),
                        classes[c]);
        }
        ObservationSpace(ObservationSpace& s) : Space(s) {
          x.update(*this, s.x);
          negative.update(*this, s.negative);
          absolute.update(*this, s.absolute);
          alias.update(*this, s.alias);
          classes.update(*this, s.classes);
        }
        virtual Space* copy(void) { return new ObservationSpace(*this); }
      };

      // Independent class expectations, supplied by the encoded boundary cases.
      enum class Category {
        Zero,
        Subnormal,
        Normal,
        Infinity,
        QuietNan,
        SignalingNan
      };
      // Whether an encoding of category, with the given sign, has class c.
      bool has_class(Category category, bool negative, int c) {
        const bool expected[] = {
            category == Category::Zero,
            category == Category::Subnormal,
            category == Category::Normal,
            category <= Category::Normal,
            category == Category::Infinity,
            category == Category::QuietNan || category == Category::SignalingNan,
            category == Category::QuietNan,
            category == Category::SignalingNan,
            negative};
        return expected[c];
      }
      // The category of code from the shared format layout. Observations
      // also checks it against the literal boundary encodings.
      Category category_of(WordDomainType format, WordValue code) {
        const FloatFormat f = float_format(format);
        return f.is_zero(code)        ? Category::Zero
               : f.is_subnormal(code) ? Category::Subnormal
               : f.is_infinity(code)  ? Category::Infinity
               : f.is_signaling(code) ? Category::SignalingNan
               : f.is_nan(code)       ? Category::QuietNan
                                      : Category::Normal;
      }
      bool check(ObservationSpace& s, WordValue code, Category category) {
        const unsigned int width = s.x.width();
        const WordValue sign = WordValue(1) << (width - 1);
        dom(s, s.x, code);
        if (s.status() == SS_FAILED || !s.negative.assigned() ||
            !s.absolute.assigned() ||
            s.negative.val() != (code ^ sign) ||
            s.absolute.val() != (code & (sign - 1)))
          return false;
        for (int c = 0; c < 9; ++c)
          if (!s.classes[c].assigned() ||
              s.classes[c].val() != has_class(category, (code & sign) != 0, c))
            return false;
        return true;
      }

      class Observations : public Base {
      public:
        Observations(void) : Base("Word::Float::Observations", TestTags::check()) {}
        virtual bool run(void) {
          // All 16 and 256 encodings, both signs, including the two E4M3FN
          // NaNs, with and without value sets.
          for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (WordValue code = 0;
                   code < (WordValue(1) << float_word_width(format)); ++code) {
                ObservationSpace s(format, value_set);
                if (!check(s, code, category_of(format, code)))
                  return false;
              }
          struct Boundary {
            WordDomainType format;
            WordValue code;
            Category category;
          };
          const Boundary boundaries[] = {{WDT_FP32, 0x00000000, Category::Zero},
                                         {WDT_FP32, 0x00000001, Category::Subnormal},
                                         {WDT_FP32, 0x007fffff, Category::Subnormal},
                                         {WDT_FP32, 0x00800000, Category::Normal},
                                         {WDT_FP32, 0x3f800000, Category::Normal},
                                         {WDT_FP32, 0x7f7fffff, Category::Normal},
                                         {WDT_FP32, 0x7f800000, Category::Infinity},
                                         {WDT_FP32, 0x7f800001, Category::SignalingNan},
                                         {WDT_FP32, 0x7fbfffff, Category::SignalingNan},
                                         {WDT_FP32, 0x7fc00000, Category::QuietNan},
                                         {WDT_FP32, 0x7fc12345, Category::QuietNan},
                                         {WDT_FP32, 0x7fffffff, Category::QuietNan},
                                         {WDT_BF16, 0x0000, Category::Zero},
                                         {WDT_BF16, 0x0001, Category::Subnormal},
                                         {WDT_BF16, 0x007f, Category::Subnormal},
                                         {WDT_BF16, 0x0080, Category::Normal},
                                         {WDT_BF16, 0x3f80, Category::Normal},
                                         {WDT_BF16, 0x7f7f, Category::Normal},
                                         {WDT_BF16, 0x7f80, Category::Infinity},
                                         {WDT_BF16, 0x7f81, Category::SignalingNan},
                                         {WDT_BF16, 0x7fbf, Category::SignalingNan},
                                         {WDT_BF16, 0x7fc0, Category::QuietNan},
                                         {WDT_BF16, 0x7fff, Category::QuietNan},
                                         {WDT_BINARY16, 0x0000, Category::Zero},
                                         {WDT_BINARY16, 0x0001, Category::Subnormal},
                                         {WDT_BINARY16, 0x03ff, Category::Subnormal},
                                         {WDT_BINARY16, 0x0400, Category::Normal},
                                         {WDT_BINARY16, 0x3c00, Category::Normal},
                                         {WDT_BINARY16, 0x7bff, Category::Normal},
                                         {WDT_BINARY16, 0x7c00, Category::Infinity},
                                         {WDT_BINARY16, 0x7c01, Category::SignalingNan},
                                         {WDT_BINARY16, 0x7dff, Category::SignalingNan},
                                         {WDT_BINARY16, 0x7e00, Category::QuietNan},
                                         {WDT_BINARY16, 0x7fff, Category::QuietNan}};
          for (const Boundary& b : boundaries)
            for (unsigned int negative = 0; negative < 2; ++negative) {
              ObservationSpace s(b.format);
              const WordValue sign = WordValue(negative)
                                     << (float_word_width(b.format) - 1);
              if (category_of(b.format, b.code | sign) != b.category ||
                  !check(s, b.code | sign, b.category))
                return false;
            }
          return true;
        }
      };

      class UnarySpace : public Space {
      public:
        WordVar x, y;
        BoolVar truth;
        UnarySpace(WordDomainType format, WordValueSetOption value_set)
            : x(float_variable(*this, format, value_set)),
              y(float_variable(*this, format, value_set)), truth(*this, 0, 1) {}
        UnarySpace(UnarySpace& s) : Space(s) {
          x.update(*this, s.x);
          y.update(*this, s.y);
          truth.update(*this, s.truth);
        }
        virtual Space* copy(void) { return new UnarySpace(*this); }
      };

      class Lifecycle : public Base {
        enum class Operation { negate, abs, classify };
        using Tuple = std::array<WordValue, 2>;

        // A nonempty random subset of the encodings below count.
        std::set<WordValue> random_codes(WordValue count) {
          std::set<WordValue> codes;
          const unsigned int sparsity = 1U + _rand(8U);
          while (codes.empty())
            for (WordValue code = 0; code < count; ++code)
              if (_rand(sparsity) == 0)
                codes.insert(code);
          return codes;
        }
        // The result encoding, or the truth of class cls, for source a.
        static WordValue output(WordDomainType format, Operation operation,
                                int cls, WordValue a) {
          const WordValue sign = WordValue(1) << (float_word_width(format) - 1);
          switch (operation) {
          case Operation::negate:
            return a ^ sign;
          case Operation::abs:
            return a & (sign - 1);
          case Operation::classify:
            return has_class(category_of(format, a), (a & sign) != 0, cls);
          }
          GECODE_NEVER;
          return 0;
        }
        static std::set<Tuple> solutions(UnarySpace& root, Operation operation) {
          branch(root, WordVarArgs{root.x}, WORD_VAL_LSB());
          if (operation == Operation::classify)
            branch(root, BoolVarArgs{root.truth}, BOOL_VAL_MIN());
          else
            branch(root, WordVarArgs{root.y}, WORD_VAL_LSB());
          DFS<UnarySpace> search(&root);
          std::set<Tuple> found;
          while (std::unique_ptr<UnarySpace> s{search.next()})
            found.insert({s->x.val(),
                          operation == Operation::classify
                              ? static_cast<WordValue>(s->truth.val())
                              : s->y.val()});
          return found;
        }
        // Propagation from random partial domains keeps every tuple that the
        // oracle supports, and search finds exactly those tuples.
        bool keeps_supported(WordDomainType format,
                             WordValueSetOption value_set,
                             Operation operation) {
          const WordValue count = WordValue(1) << float_word_width(format);
          UnarySpace root(format, value_set);
          const std::set<WordValue> sources = random_codes(count);
          const std::set<WordValue> results = random_codes(count);
          const int cls = static_cast<int>(_rand(9U));
          const int truth = static_cast<int>(_rand(3U)) - 1;
          dom(root, root.x, WordValArgs(sources.begin(), sources.end()));
          std::set<Tuple> expected;
          for (WordValue a : sources) {
            const WordValue b = output(format, operation, cls, a);
            const bool admitted = operation == Operation::classify
                                      ? truth < 0 || b == static_cast<WordValue>(truth)
                                      : results.count(b) != 0;
            if (admitted)
              expected.insert({a, b});
          }
          if (operation == Operation::classify) {
            if (truth >= 0)
              rel(root, root.truth, IRT_EQ, truth);
            fp_classify(root, root.x, static_cast<FloatWordClass>(cls),
                        root.truth);
          } else {
            dom(root, root.y,
                WordValArgs(results.begin(), results.end()));
            if (operation == Operation::negate)
              fp_negate(root, root.x, root.y);
            else
              fp_abs(root, root.x, root.y);
          }
          if (root.status() == SS_FAILED)
            return expected.empty();
          for (const Tuple& t : expected)
            if (!root.x.in(t[0]) ||
                (operation == Operation::classify
                     ? !root.truth.in(static_cast<int>(t[1]))
                     : !root.y.in(t[1])))
              return false;
          return solutions(root, operation) == expected;
        }
        bool root_keeps_supported(void) {
          for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE})
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
              for (int trial = 0; trial < 40; ++trial)
                for (Operation operation :
                     {Operation::negate, Operation::abs, Operation::classify})
                  if (!keeps_supported(format, value_set, operation))
                    return false;
          return true;
        }
        static bool holes_and_clone(void) {
          for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN}) {
            ObservationSpace source(format);
            if (source.status() == SS_FAILED)
              return false;
            const WordValue sign = WordValue(1) << (float_word_width(format) - 1);
            std::unique_ptr<ObservationSpace> clone(
                static_cast<ObservationSpace*>(source.clone()));
            if (clone->alias.varimp() != clone->x.varimp() ||
                clone->x.varimp() == source.x.varimp())
              return false;
            rel(*clone, clone->alias, WRT_NQ, WordValue(3));
            rel(*clone, clone->alias, WRT_NQ, 3 | sign);
            if (clone->status() == SS_FAILED ||
                clone->negative.in(3) ||
                clone->negative.in(3 | sign) ||
                clone->absolute.in(3) ||
                !source.x.in(3) || !source.absolute.in(3))
              return false;
            // Classification reuses the same domain, including reverse
            // propagation.
            rel(*clone, clone->classes[static_cast<int>(FWC_SIGN)], IRT_EQ, 0);
            if (clone->status() == SS_FAILED || clone->x.hi() >= sign)
              return false;
            dom(source, source.x, 3);
            if (!check(source, 3, format == WDT_E2M1 ? Category::Normal : Category::Subnormal))
              return false;
          }
          ObservationSpace wide(WDT_BF16);
          if (wide.status() == SS_FAILED)
            return false;
          std::unique_ptr<ObservationSpace> copy(
              static_cast<ObservationSpace*>(wide.clone()));
          // The same bits read as binary16 follow the BF16 word.
          WordVar half = reinterpret(*copy, copy->alias, WDT_BINARY16);
          if (half.domain_type() != WDT_BINARY16)
            return false;
          if (!check(*copy, 0xff81, Category::SignalingNan) || wide.x.assigned() ||
              !half.assigned() || half.val() != 0xff81)
            return false;
          return true;
        }
        static bool aliases_and_branching(void) {
          for (WordDomainType format :
               {WDT_E2M1, WDT_E4M3FN, WDT_BF16, WDT_FP32, WDT_BINARY16}) {
            ObservationSpace s(format);
            fp_negate(s, s.x, s.alias);
            if (s.status() != SS_FAILED)
              return false;
            ObservationSpace absolute(format);
            fp_abs(absolute, absolute.x, absolute.alias);
            if (absolute.status() == SS_FAILED ||
                (absolute.x.hi() >> (float_word_width(format) - 1)) != 0)
              return false;
          }
          ObservationSpace root(WDT_E2M1);
          branch(root, WordVarArgs{root.x}, WORD_VAL_LSB());
          DFS<ObservationSpace> search(&root);
          int seen = 0;
          while (std::unique_ptr<ObservationSpace> solution{search.next()}) {
            if (!solution->negative.assigned() ||
                !solution->absolute.assigned())
              return false;
            seen |= 1 << solution->x.val();
          }
          return seen == 65535;
        }
        static bool errors_and_policies(void) {
          static_assert(std::is_copy_assignable<FloatWordPolicy>::value,
                        "Policies are plain values");
          ObservationSpace s(WDT_BF16);
          int caught = 0;
          try {
            fp_negate(s, s.x, WordVar(s, WDT_BINARY16));
          } catch (const Gecode::Word::FormatMismatch&) {
            ++caught;
          }
          try {
            float_code(s, WDT_BF16, 0x10000);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            (void)float_word_width(static_cast<WordDomainType>(99));
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            fp_classify(s, s.x, static_cast<FloatWordClass>(99), s.classes[0]);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          return caught == 4;
        }
        static bool rejects(const std::function<void()>& post) {
          try {
            post();
          } catch (const Gecode::Word::OutOfLimits&) {
            return true;
          }
          return false;
        }
        // Every numerical posting checks every field, even an ignored one.
        static bool invalid_policy_fields(void) {
          using Policy = FloatWordPolicy;
          Policy invalid[8];
          invalid[0].rounding = static_cast<Policy::Rounding>(99);
          invalid[1].input_subnormals = static_cast<Policy::Subnormals>(99);
          invalid[2].output_subnormals = static_cast<Policy::Subnormals>(-1);
          invalid[3].overflow = static_cast<Policy::Overflow>(99);
          invalid[4].zeros = static_cast<Policy::Zeros>(99);
          invalid[5].nan = static_cast<Policy::Nan>(99);
          invalid[6].tininess = static_cast<Policy::Tininess>(99);
          invalid[7].comparison = static_cast<Policy::Comparison>(99);
          ObservationSpace s(WDT_BF16);
          WordVar a = s.x, b = s.negative, r = s.absolute;
          for (const Policy& p : invalid)
            if (!rejects([&] { fp_convert(s, a, r, p); }))
              return false;
          const Policy& p = invalid[6];
          BoolVar truth = s.classes[0];
          return rejects([&] { fp_compare(s, a, FWRT_EQ, b, truth, p); }) &&
                 rejects([&] { fp_minimum(s, a, b, r, p); }) &&
                 rejects([&] { fp_add(s, a, b, r, p); }) &&
                 rejects([&] { fp_fma(s, a, b, a, r, p); }) &&
                 rejects([&] { fp_sqrt(s, a, r, p); }) &&
                 s.status() != SS_FAILED;
        }
        // A posting keeps its own policy copy.
        static bool policy_copied_at_posting(void) {
          ObservationSpace s(WDT_FP32);
          WordVar half = float_variable(s, WDT_BF16);
          FloatWordPolicy policy;
          policy.rounding = FloatWordPolicy::RTZ;
          fp_convert(s, s.x, half, policy);
          policy.rounding = FloatWordPolicy::RNE;
          // 0x3f80c000 rounds to 0x3f81 under RNE and to 0x3f80 under RTZ.
          dom(s, s.x, 0x3f80c000);
          return s.status() != SS_FAILED && half.assigned() &&
                 half.val() == 0x3f80;
        }

      public:
        Lifecycle(void) : Base("Word::Float::Lifecycle") {}
        virtual bool run(void) {
          return holes_and_clone() && aliases_and_branching() &&
                 errors_and_policies() && invalid_policy_fields() &&
                 policy_copied_at_posting() && root_keeps_supported();
        }
      };
      /// Complete solutions of the sign operations and classification on
      /// float-typed words whose windows are negative or cross zero
      class FloatTyped : public Base {
        using Tuple = std::array<WordValue, 2>;
        // Total-order window of encodings
        struct Range {
          WordValue minimum, maximum;
        };
        enum class Operation { negate, abs, classify };
        static bool check(WordDomainType format, Range x, Range y,
                          WordValueSetOption value_set, Operation operation,
                          int cls, int truth) {
          const unsigned int width = float_word_width(format);
          const WordValue sign = WordValue(1) << (width - 1);
          UnarySpace root(format, value_set);
          root.x = float_window(root, format, x.minimum, x.maximum, value_set);
          root.y = float_window(root, format, y.minimum, y.maximum, value_set);
          if (truth >= 0)
            rel(root, root.truth, IRT_EQ, truth);
          std::set<Tuple> expected, found;
          for (WordValue a : float_order_range(width, x.minimum, x.maximum)) {
            const WordValue b =
                operation == Operation::negate ? a ^ sign
                : operation == Operation::abs
                    ? a & (sign - 1)
                    : has_class(category_of(format, a), (a & sign) != 0, cls);
            if (operation == Operation::classify
                    ? truth < 0 || b == WordValue(truth)
                    : root.y.in(b))
              expected.insert({a, b});
          }
          switch (operation) {
          case Operation::negate:
            fp_negate(root, root.x, root.y);
            break;
          case Operation::abs:
            fp_abs(root, root.x, root.y);
            break;
          case Operation::classify:
            fp_classify(root, root.x, static_cast<FloatWordClass>(cls),
                        root.truth);
            break;
          }
          branch(root, WordVarArgs{root.x}, WORD_VAL_LSB());
          if (operation == Operation::classify)
            branch(root, BoolVarArgs{root.truth}, BOOL_VAL_MIN());
          else
            branch(root, WordVarArgs{root.y}, WORD_VAL_LSB());
          DFS<UnarySpace> search(&root);
          while (std::unique_ptr<UnarySpace> s{search.next()})
            found.insert({s->x.val(),
                          operation == Operation::classify
                              ? static_cast<WordValue>(s->truth.val())
                              : s->y.val()});
          if (found == expected)
            return true;
          if (opt.log)
            olog << ind(2) << "float-typed unary " << format << " operation "
                 << static_cast<int>(operation) << " class " << cls
                 << " truth " << truth << std::hex << " x " << x.minimum
                 << ".." << x.maximum << " y " << y.minimum << ".."
                 << y.maximum << std::dec << '\n';
          return false;
        }
        // Source windows: one crossing zero, one from the negative NaNs
        // down; result windows: all encodings, and one crossing zero.
        static bool format_cases(WordDomainType format, Range cross,
                                 Range negative, Range narrow) {
          const unsigned int width = float_word_width(format);
          const WordValue sign = WordValue(1) << (width - 1);
          const Range all{sign | (sign - 1), sign - 1};
          for (WordValueSetOption value_set : {WVS_EXACT, WVS_NONE}) {
            if (width > 8 && value_set == WVS_EXACT)
              continue;
            for (Range x : {cross, negative}) {
              for (Range y : {all, narrow})
                for (Operation operation : {Operation::negate, Operation::abs})
                  if (!check(format, x, y, value_set, operation, 0, -1))
                    return false;
              for (int cls = 0; cls < 9; ++cls)
                for (int truth : {-1, 0, 1})
                  if (!check(format, x, all, value_set, Operation::classify,
                             cls, truth))
                    return false;
            }
          }
          return true;
        }

      public:
        FloatTyped(void) : Base("Word::Float::FloatTyped") {}
        virtual bool run(void) {
          return format_cases(WDT_E2M1, {0xb, 3}, {0xf, 0xa}, {0x9, 2}) &&
                 format_cases(WDT_E4M3FN, {0x83, 3}, {0xff, 0xf8},
                              {0x81, 0x7c}) &&
                 format_cases(WDT_BF16, {0x8003, 3}, {0xff81, 0xff7e},
                              {0x8001, 0x7f80}) &&
                 format_cases(WDT_FP32, {0x80000003, 3},
                              {0xff800001, 0xff7ffffe},
                              {0x80000001, 0x7f800000});
        }
      };
      /// A space for posting operations without search
      class PostingSpace : public Space {
      public:
        PostingSpace(void) {}
        PostingSpace(PostingSpace& s) : Space(s) {}
        virtual Space* copy(void) { return new PostingSpace(*this); }
      };

      /** \brief Return the double with the binary64 bit pattern \a bits
       *
       * Infinities and NaNs are built this way, and negated by their sign
       * bit, because the Autoconf build compiles with -ffinite-math-only,
       * under which std::numeric_limits<double>::infinity(), std::nan and
       * negation of a non-finite value are not reliable.
       */
      double double_from_bits(std::uint64_t bits) {
        double value;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
      }
      /// Sign bit of a binary64 double
      constexpr std::uint64_t double_sign = std::uint64_t(1) << 63;
      /// Bit pattern of a positive binary64 infinity
      constexpr std::uint64_t double_infinity = 0x7ff0000000000000;
      /// Bit patterns of positive binary64 NaNs: quiet, quiet with a
      /// payload, signaling, and signaling with a full payload
      constexpr std::uint64_t double_nans[] = {0x7ff8000000000000, 0x7ff8000000000001, 0x7ff0000000000001,
                                               0x7ff7ffffffffffff};

      /// What a code of a float format encodes
      enum class CodeKind { Number, Infinity, NaN };
      /// Return what \a code of \a format encodes
      CodeKind code_kind(WordDomainType format, WordValue code) {
        const FloatFormat f = float_format(format);
        return f.is_infinity(code) ? CodeKind::Infinity : f.is_nan(code) ? CodeKind::NaN : CodeKind::Number;
      }
      /// Return the value of \a code in \a format, or a NaN, from its fields
      double decode_value(WordDomainType format, WordValue code) {
        const FloatFormat f = float_format(format);
        const std::uint64_t sign = f.negative(code) ? double_sign : 0;
        switch (code_kind(format, code)) {
        case CodeKind::Infinity:
          return double_from_bits(sign | double_infinity);
        case CodeKind::NaN:
          return double_from_bits(sign | double_nans[0]);
        case CodeKind::Number:
          break;
        }
        const WordValue m = f.magnitude(code), field = m >> f.fraction;
        const WordValue significand = field == 0 ? m : f.normal | (m & (f.normal - 1));
        const double magnitude = std::ldexp(static_cast<double>(significand),
                                            (field == 0 ? 1 : static_cast<int>(field)) - f.bias -
                                                static_cast<int>(f.fraction));
        return f.negative(code) ? -magnitude : magnitude;
      }

      /// float_constant() encodes exactly the values of a format and
      /// rejects every other double
      class Constants : public Base {
        /// float_constant() of the value of \a code gives an assigned word
        /// of \a format with exactly \a code and the default value set
        bool encodes(Space& home, WordDomainType format, WordValue code) {
          const WordVar w = float_constant(home, format, decode_value(format, code));
          if (w.assigned() && w.val() == code && w.domain_type() == format &&
              w.has_value_set() == (float_word_width(format) <= 8))
            return true;
          if (opt.log)
            olog << ind(2) << "float_constant " << format << " of code " << std::hex << code << std::dec << '\n';
          return false;
        }
        bool encodes_all(WordDomainType format) {
          PostingSpace home;
          for (WordValue code = 0; code < (WordValue(1) << float_word_width(format)); ++code)
            if (code_kind(format, code) != CodeKind::NaN && !encodes(home, format, code))
              return false;
          return true;
        }
        bool encodes_fp32(void) {
          PostingSpace home;
          const WordValue boundaries[] = {0x00000000, 0x00000001, 0x00000003, 0x007fffff, 0x00800000,
                                          0x3f800000, 0x3fc00001, 0x7f7fffff, 0x7f800000};
          for (WordValue code : boundaries)
            if (!encodes(home, WDT_FP32, code) || !encodes(home, WDT_FP32, code | 0x80000000))
              return false;
          for (int trial = 0; trial < 2000; ++trial) {
            const WordValue code = (WordValue(_rand(1U << 16)) << 16) | _rand(1U << 16);
            if (code_kind(WDT_FP32, code) != CodeKind::NaN && !encodes(home, WDT_FP32, code))
              return false;
          }
          return true;
        }
        bool rejects(Space& home, WordDomainType format, double value) {
          try {
            (void)float_constant(home, format, value);
          } catch (const Gecode::Word::OutOfLimits&) {
            return true;
          }
          if (opt.log)
            olog << ind(2) << "float_constant " << format << " accepted " << value << '\n';
          return false;
        }
        /// Inexact and out-of-range values, NaNs, and infinities of a
        /// format without them throw
        bool rejects_unrepresentable(WordDomainType format) {
          PostingSpace home;
          const WordValue one = float_format(format).one;
          const WordValue maximum = float_format(format).maximum;
          const double largest = decode_value(format, maximum);
          const double beyond = largest + (largest - decode_value(format, maximum - 1));
          const double rejected[] = {decode_value(format, 1) / 2,
                                     (decode_value(format, one) + decode_value(format, one + 1)) / 2,
                                     (decode_value(format, maximum - 1) + largest) / 2,
                                     beyond,
                                     2 * largest,
                                     std::numeric_limits<double>::max(),
                                     std::numeric_limits<double>::denorm_min()};
          for (double value : rejected)
            if (!rejects(home, format, value) || !rejects(home, format, -value))
              return false;
          for (std::uint64_t nan : double_nans)
            if (!rejects(home, format, double_from_bits(nan)) ||
                !rejects(home, format, double_from_bits(double_sign | nan)))
              return false;
          const bool has_infinity = float_format(format).infinity != 0;
          return has_infinity || (rejects(home, format, double_from_bits(double_infinity)) &&
                                  rejects(home, format, double_from_bits(double_sign | double_infinity)));
        }
        /// Expected codes of the special values of a float format
        struct SpecialCodes {
          WordDomainType format;
          /// Code of the sign bit alone, that is, of -0.0
          WordValue sign;
          /// Code of positive infinity, or 0 for a format without infinities
          WordValue infinity;
        };
        /// Infinities, NaNs and zeros as float_constant() and float_word()
        /// endpoints give the codes of \a s or throw
        bool special_values(const SpecialCodes& s) {
          PostingSpace home;
          const double positive_infinity = double_from_bits(double_infinity);
          const double negative_infinity = double_from_bits(double_sign | double_infinity);
          bool ok = true;
          if (s.infinity != 0) {
            const WordVar plus = float_constant(home, s.format, positive_infinity);
            const WordVar minus = float_constant(home, s.format, negative_infinity);
            const WordVar all = float_word(home, s.format, negative_infinity, positive_infinity);
            ok = plus.assigned() && plus.val() == s.infinity && minus.assigned() &&
                 minus.val() == (s.sign | s.infinity) && all.minimum() == (s.sign | s.infinity) &&
                 all.maximum() == s.infinity;
          } else {
            ok = rejects(home, s.format, positive_infinity) && rejects(home, s.format, negative_infinity) &&
                 rejects_word(home, s.format, 0.0, positive_infinity) &&
                 rejects_word(home, s.format, negative_infinity, 0.0);
          }
          for (std::uint64_t bits : double_nans)
            for (std::uint64_t sign : {std::uint64_t(0), double_sign}) {
              const double nan = double_from_bits(sign | bits);
              ok = ok && rejects(home, s.format, nan) && rejects_word(home, s.format, nan, 0.0) &&
                   rejects_word(home, s.format, 0.0, nan);
            }
          const WordVar negative_zero = float_constant(home, s.format, -0.0);
          const WordVar positive_zero = float_constant(home, s.format, 0.0);
          const WordVar zeros = float_word(home, s.format, -0.0, 0.0);
          ok = ok && negative_zero.assigned() && negative_zero.val() == s.sign && positive_zero.assigned() &&
               positive_zero.val() == 0 && zeros.minimum() == s.sign && zeros.maximum() == 0;
          if (!ok && opt.log)
            olog << ind(2) << "special values of " << s.format << '\n';
          return ok;
        }
        bool rejects_word(Space& home, WordDomainType format, double minimum, double maximum) {
          try {
            (void)float_word(home, format, minimum, maximum);
          } catch (const Gecode::Word::OutOfLimits&) {
            return true;
          }
          if (opt.log)
            olog << ind(2) << "float_word " << format << " accepted " << minimum << ".." << maximum << '\n';
          return false;
        }
        bool rejects_non_float_types(void) {
          PostingSpace home;
          for (WordDomainType type : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED, static_cast<WordDomainType>(99)})
            if (!rejects(home, type, 1.0) || !rejects(home, type, 0.0))
              return false;
          return true;
        }
        /// Solutions of x + c = y and the truth of x <= c in \a root
        static std::set<std::array<WordValue, 3>> sum_and_comparison(UnarySpace& root, WordVar c) {
          fp_add(root, root.x, c, root.y);
          fp_compare(root, root.x, FWRT_LQ, c, root.truth);
          branch(root, WordVarArgs{root.x, root.y}, WORD_VAL_LSB());
          branch(root, BoolVarArgs{root.truth}, BOOL_VAL_MIN());
          DFS<UnarySpace> search(&root);
          std::set<std::array<WordValue, 3>> found;
          while (std::unique_ptr<UnarySpace> s{search.next()})
            found.insert({s->x.val(), s->y.val(), static_cast<WordValue>(s->truth.val())});
          return found;
        }
        /// A constant of -1.5 from float_constant() gives the same
        /// solutions as its encoding
        bool posts_as_operand(void) {
          for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN}) {
            UnarySpace by_code(format, WVS_EXACT), by_value(format, WVS_EXACT);
            const WordValue code = format == WDT_E2M1 ? 0xb : 0xbc;
            const auto expected = sum_and_comparison(by_code, float_code(by_code, format, code));
            if (expected.size() != (WordValue(1) << float_word_width(format)) ||
                sum_and_comparison(by_value, float_constant(by_value, format, -1.5)) != expected) {
              if (opt.log)
                olog << ind(2) << "float_constant operand " << format << '\n';
              return false;
            }
          }
          return true;
        }

      public:
        Constants(void) : Base("Word::Float::Constants") {}
        virtual bool run(void) {
          for (WordDomainType format : {WDT_BF16, WDT_BINARY16, WDT_E4M3FN, WDT_E2M1})
            if (!encodes_all(format))
              return false;
          for (WordDomainType format : {WDT_FP32, WDT_BF16, WDT_BINARY16, WDT_E4M3FN, WDT_E2M1})
            if (!rejects_unrepresentable(format))
              return false;
          const SpecialCodes specials[] = {{WDT_FP32, 0x80000000, 0x7f800000},
                                           {WDT_BF16, 0x8000, 0x7f80},
                                           {WDT_BINARY16, 0x8000, 0x7c00},
                                           {WDT_E4M3FN, 0x80, 0},
                                           {WDT_E2M1, 0x8, 0}};
          for (const SpecialCodes& s : specials)
            if (!special_values(s))
              return false;
          return encodes_fp32() && rejects_non_float_types() && posts_as_operand();
        }
      };

      /// What a posting does
      enum class Outcome { posted, out_of_limits, format_mismatch, other };

      const char* outcome_name(Outcome outcome) {
        switch (outcome) {
        case Outcome::posted:
          return "posted";
        case Outcome::out_of_limits:
          return "OutOfLimits";
        case Outcome::format_mismatch:
          return "FormatMismatch";
        case Outcome::other:
          return "another exception or a posted actor";
        }
        return "";
      }

      /// Float operations with word operands, checked by Formats
      enum class Operation {
        convert, classify, negate, abs, sqrt, compare, minimum, maximum,
        minimum_number, maximum_number, add, subtract, multiply, divide, fma
      };
      const Operation operations[] = {
          Operation::convert,        Operation::classify,
          Operation::negate,         Operation::abs,
          Operation::sqrt,           Operation::compare,
          Operation::minimum,        Operation::maximum,
          Operation::minimum_number, Operation::maximum_number,
          Operation::add,            Operation::subtract,
          Operation::multiply,       Operation::divide,
          Operation::fma};

      /// Return the number of word operands of \a operation, results included
      int arity(Operation operation) {
        switch (operation) {
        case Operation::classify:
          return 1;
        case Operation::convert:
        case Operation::negate:
        case Operation::abs:
        case Operation::sqrt:
        case Operation::compare:
          return 2;
        case Operation::fma:
          return 4;
        default:
          return 3;
        }
      }
      /// Test whether all word operands of \a operation share one format
      bool same_format(Operation operation) {
        switch (operation) {
        case Operation::convert:
        case Operation::classify:
          return false;
        default:
          return true;
        }
      }
      /// Test whether \a operation checks its operands' support before the
      /// format of its result
      bool support_before_result(Operation operation) {
        switch (operation) {
        case Operation::minimum:
        case Operation::maximum:
        case Operation::minimum_number:
        case Operation::maximum_number:
          return true;
        default:
          return false;
        }
      }
      /// Test whether \a operation supports binary16
      bool supports_binary16(Operation operation) {
        switch (operation) {
        case Operation::convert:
        case Operation::classify:
        case Operation::negate:
        case Operation::abs:
          return true;
        default:
          return false;
        }
      }

      /// Test whether \a operation takes a FloatWordPolicy
      bool takes_policy(Operation operation) {
        switch (operation) {
        case Operation::classify:
        case Operation::negate:
        case Operation::abs:
          return false;
        default:
          return true;
        }
      }

      /// Test whether \a type is a float format
      bool is_float_format(WordDomainType type) {
        return type != WDT_CUBE && type != WDT_UNSIGNED && type != WDT_SIGNED;
      }

      /// Post \a operation on the words \a w
      void post(Space& home, Operation operation, const WordVarArgs& w) {
        BoolVar truth(home, 0, 1);
        switch (operation) {
        case Operation::convert:
          fp_convert(home, w[0], w[1]);
          break;
        case Operation::classify:
          fp_classify(home, w[0], FWC_NAN, truth);
          break;
        case Operation::negate:
          fp_negate(home, w[0], w[1]);
          break;
        case Operation::abs:
          fp_abs(home, w[0], w[1]);
          break;
        case Operation::sqrt:
          fp_sqrt(home, w[0], w[1]);
          break;
        case Operation::compare:
          fp_compare(home, w[0], FWRT_LE, w[1], truth);
          break;
        case Operation::minimum:
          fp_minimum(home, w[0], w[1], w[2]);
          break;
        case Operation::maximum:
          fp_maximum(home, w[0], w[1], w[2]);
          break;
        case Operation::minimum_number:
          fp_minimum_number(home, w[0], w[1], w[2]);
          break;
        case Operation::maximum_number:
          fp_maximum_number(home, w[0], w[1], w[2]);
          break;
        case Operation::add:
          fp_add(home, w[0], w[1], w[2]);
          break;
        case Operation::subtract:
          fp_subtract(home, w[0], w[1], w[2]);
          break;
        case Operation::multiply:
          fp_multiply(home, w[0], w[1], w[2]);
          break;
        case Operation::divide:
          fp_divide(home, w[0], w[1], w[2]);
          break;
        case Operation::fma:
          fp_fma(home, w[0], w[1], w[2], w[3]);
          break;
        }
      }

      /// Return what \a post does to \a home; a throw must post no actor
      Outcome outcome_of(Space& home, const std::function<void()>& post) {
        const unsigned int propagators = PropagatorGroup::all.size(home);
        Outcome outcome = Outcome::posted;
        try {
          post();
        } catch (const Gecode::Word::OutOfLimits&) {
          outcome = Outcome::out_of_limits;
        } catch (const Gecode::Word::FormatMismatch&) {
          outcome = Outcome::format_mismatch;
        } catch (const Gecode::Exception&) {
          outcome = Outcome::other;
        }
        if (outcome != Outcome::posted &&
            PropagatorGroup::all.size(home) != propagators)
          return Outcome::other;
        return outcome;
      }

      /// Format checks of every float operation before posting
      class Formats : public Base {
        static constexpr WordDomainType formats[] = {
            WDT_E2M1, WDT_E4M3FN, WDT_BF16, WDT_BINARY16, WDT_FP32};

        // Post operation on words of the given domain types and compare
        // the outcome with expected.
        bool expect(Operation operation, std::vector<WordDomainType> types,
                    unsigned int width, Outcome expected) {
          PostingSpace home;
          WordVarArgs w;
          for (WordDomainType type : types)
            w << (is_float_format(type) ? WordVar(home, type)
                                        : WordVar(home, width, type));
          const Outcome actual = outcome_of(
              home, [&] { post(home, operation, w); });
          if (actual == expected)
            return true;
          if (opt.log) {
            olog << ind(2) << "operation " << static_cast<int>(operation) << " on";
            for (WordDomainType type : types)
              olog << ind(2) << ' ' << type;
            olog << ind(2) << ": "
                 << outcome_name(actual) << ", expected "
                 << outcome_name(expected) << '\n';
          }
          return false;
        }
        // A format of the same width as format, or of another width.
        static WordDomainType partner(WordDomainType format) {
          switch (format) {
          case WDT_BF16:
            return WDT_BINARY16;
          case WDT_BINARY16:
            return WDT_FP32;
          case WDT_E4M3FN:
            return WDT_E2M1;
          default:
            return WDT_E4M3FN;
          }
        }
        // Uniform operands of format, then one operand replaced at a time
        // by a non-float word of the same width or by another format.
        bool operand_checks(Operation operation, WordDomainType format) {
          const int n = arity(operation);
          const unsigned int width = float_word_width(format);
          const std::vector<WordDomainType> uniform(n, format);
          const bool supported =
              format != WDT_BINARY16 || supports_binary16(operation);
          if (!expect(operation, uniform, width,
                      supported ? Outcome::posted : Outcome::out_of_limits))
            return false;
          for (int i = 0; i < n; ++i) {
            for (WordDomainType type : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
              std::vector<WordDomainType> types = uniform;
              types[i] = type;
              if (!expect(operation, types, width, Outcome::out_of_limits))
                return false;
            }
            std::vector<WordDomainType> types = uniform;
            types[i] = partner(format);
            const bool mismatch = n > 1 && same_format(operation);
            const bool partner_supported =
                types[i] != WDT_BINARY16 || supports_binary16(operation);
            const bool unsupported_first = !supported && i == n - 1 &&
                                           support_before_result(operation);
            const Outcome expected =
                mismatch && !unsupported_first ? Outcome::format_mismatch
                : supported && partner_supported ? Outcome::posted
                                                 : Outcome::out_of_limits;
            if (!expect(operation, types, width, expected))
              return false;
          }
          // A non-float operand takes precedence over mismatched formats.
          for (int i = 1; i < n; ++i) {
            std::vector<WordDomainType> types = uniform;
            types[0] = partner(format);
            types[i] = WDT_UNSIGNED;
            if (!expect(operation, types, width, Outcome::out_of_limits))
              return false;
          }
          return true;
        }
        // Format-taking functions reject a type that is not a float format.
        bool reference_checks(void) {
          for (WordDomainType type : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
            PostingSpace home;
            if (outcome_of(home, [&] { (void)float_word_width(type); }) !=
                Outcome::out_of_limits)
              return false;
          }
          return true;
        }

      public:
        Formats(void) : Base("Word::Float::Formats") {}
        virtual bool run(void) {
          for (Operation operation : operations)
            for (WordDomainType format : formats)
              if (!operand_checks(operation, format))
                return false;
          // The mixed signature: BF16 * BF16 + FP32 with an FP32 result.
          return expect(Operation::fma, {WDT_BF16, WDT_BF16, WDT_FP32, WDT_FP32},
                        16, Outcome::posted) &&
                 expect(Operation::fma, {WDT_BF16, WDT_BF16, WDT_FP32, WDT_BF16},
                        16, Outcome::format_mismatch) &&
                 expect(Operation::convert, {WDT_BINARY16, WDT_E2M1}, 16,
                        Outcome::posted) &&
                 reference_checks();
        }
      };

      /// Whether a tiny posting receives a status
      enum class StatusArgument { omitted, posted };
      /// Whether a tiny posting receives a policy
      enum class PolicyArgument { omitted, explicit_default };

      /// Words of the tiny formats and the results of every tiny operation
      class TinyDefaultSpace : public Space {
      public:
        /// Operands a, b and c, then results
        WordVarArray words;
        /// Status
        IntVarArray numbers;
        /// Classification or comparison truth
        BoolVarArray truths;
        /// Post operation on a few codes of format; WVS_DEFAULT creates
        /// every word without an explicit option. An operation that takes a
        /// policy receives the status and a default-constructed policy as
        /// status_argument and policy_argument select.
        TinyDefaultSpace(WordDomainType format, WordValueSetOption value_set,
                         Operation operation,
                         StatusArgument status_argument = StatusArgument::posted,
                         PolicyArgument policy_argument = PolicyArgument::omitted) {
          const WordValArgs codes = format == WDT_E2M1
                                        ? WordValArgs{0x0, 0x3, 0xa}
                                        : WordValArgs{0x00, 0x39, 0xb0};
          WordVarArgs w;
          for (int i = 0; i < 3; ++i) {
            w << word(format, value_set);
            dom(*this, w[i], codes);
          }
          IntVarArgs n;
          BoolVarArgs t;
          post_tiny(operation, format, value_set, status_argument,
                    policy_argument, w, n, t);
          words = WordVarArray(*this, w);
          numbers = IntVarArray(*this, n);
          truths = BoolVarArray(*this, t);
        }
        TinyDefaultSpace(TinyDefaultSpace& s) : Space(s) {
          words.update(*this, s.words);
          numbers.update(*this, s.numbers);
          truths.update(*this, s.truths);
        }
        virtual Space* copy(void) { return new TinyDefaultSpace(*this); }
        /// Return the values of all variables of an assigned space
        std::vector<long long> values(void) const {
          std::vector<long long> result;
          for (int i = 0; i < words.size(); ++i)
            result.push_back(static_cast<long long>(words[i].val()));
          for (int i = 0; i < numbers.size(); ++i)
            result.push_back(numbers[i].val());
          for (int i = 0; i < truths.size(); ++i)
            result.push_back(truths[i].val());
          return result;
        }

      private:
        WordVar word(WordDomainType format, WordValueSetOption value_set) {
          return value_set == WVS_DEFAULT ? WordVar(*this, format)
                                          : WordVar(*this, format, value_set);
        }
        // Call post with the trailing status and policy arguments that
        // status_argument and policy_argument select.
        template<class Post>
        static void post_trailing(Post post, IntVar status,
                                  StatusArgument status_argument,
                                  PolicyArgument policy_argument) {
          const FloatWordPolicy policy;
          const bool with_policy =
              policy_argument == PolicyArgument::explicit_default;
          if (status_argument == StatusArgument::omitted) {
            if (with_policy)
              post(policy);
            else
              post();
          } else if (with_policy) {
            post(status, policy);
          } else {
            post(status);
          }
        }
        // Post operation on the operands in w and append its results.
        void post_tiny(Operation operation, WordDomainType format,
                       WordValueSetOption value_set,
                       StatusArgument status_argument,
                       PolicyArgument policy_argument, WordVarArgs& w,
                       IntVarArgs& n, BoolVarArgs& t) {
          const WordVar a = w[0], b = w[1], c = w[2];
          const WordVar r = word(format, value_set);
          const IntVar status(*this, 0, 31);
          const BoolVar truth(*this, 0, 1);
          switch (operation) {
          case Operation::convert:
            post_trailing([&](auto... trailing) { fp_convert(*this, a, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::classify:
            fp_classify(*this, a, FWC_NORMAL, truth);
            break;
          case Operation::negate:
            fp_negate(*this, a, r);
            break;
          case Operation::abs:
            fp_abs(*this, a, r);
            break;
          case Operation::sqrt:
            post_trailing([&](auto... trailing) { fp_sqrt(*this, a, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::compare:
            post_trailing([&](auto... trailing) { fp_compare(*this, a, FWRT_LE, b, truth, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::minimum:
            post_trailing([&](auto... trailing) { fp_minimum(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::maximum:
            post_trailing([&](auto... trailing) { fp_maximum(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::minimum_number:
            post_trailing([&](auto... trailing) { fp_minimum_number(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::maximum_number:
            post_trailing([&](auto... trailing) { fp_maximum_number(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::add:
            post_trailing([&](auto... trailing) { fp_add(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::subtract:
            post_trailing([&](auto... trailing) { fp_subtract(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::multiply:
            post_trailing([&](auto... trailing) { fp_multiply(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::divide:
            post_trailing([&](auto... trailing) { fp_divide(*this, a, b, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          case Operation::fma:
            post_trailing([&](auto... trailing) { fp_fma(*this, a, b, c, r, trailing...); },
                          status, status_argument, policy_argument);
            break;
          }
          if (operation == Operation::classify ||
              operation == Operation::compare)
            t << truth;
          if (operation != Operation::classify &&
              operation != Operation::compare)
            w << r;
          if (takes_policy(operation) &&
              status_argument == StatusArgument::posted)
            n << status;
        }
      };

      /// Return the values of all solutions of \a root
      std::set<std::vector<long long>> solutions(TinyDefaultSpace& root) {
        branch(root, root.words, WORD_VAR_NONE(), WORD_VAL_LSB());
        if (root.numbers.size() > 0)
          branch(root, root.numbers, INT_VAR_NONE(), INT_VAL_MIN());
        if (root.truths.size() > 0)
          branch(root, root.truths, BOOL_VAR_NONE(), BOOL_VAL_MIN());
        DFS<TinyDefaultSpace> search(&root);
        std::set<std::vector<long long>> found;
        while (std::unique_ptr<TinyDefaultSpace> s{search.next()})
          found.insert(s->values());
        return found;
      }

      /// Tiny words carry a value set by default, and every tiny operation
      /// has the same solutions on words that opt out
      class TinyDefaults : public Base {
        static bool has_value_sets(const TinyDefaultSpace& s, bool expected) {
          for (int i = 0; i < s.words.size(); ++i)
            if (s.words[i].domain_type() != WDT_UNSIGNED &&
                s.words[i].has_value_set() != expected)
              return false;
          return true;
        }

      public:
        TinyDefaults(void) : Base("Word::Float::TinyDefaults") {}
        virtual bool run(void) {
          for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
            for (Operation operation : operations) {
              TinyDefaultSpace with_sets(format, WVS_DEFAULT, operation);
              TinyDefaultSpace without_sets(format, WVS_NONE, operation);
              if (!has_value_sets(with_sets, true) ||
                  !has_value_sets(without_sets, false))
                return false;
              const auto expected = solutions(with_sets);
              if (expected.empty() || solutions(without_sets) != expected) {
                if (opt.log)
                  olog << ind(2) << "tiny defaults " << format << " operation "
                       << static_cast<int>(operation) << '\n';
                return false;
              }
            }
          return true;
        }
      };
      /// Compile-time checks that each policy-taking posting accepts its
      /// policy after the operands and the optional status, and before the
      /// propagation level, and nowhere else
      namespace PolicyPosition {
        using Policy = const FloatWordPolicy&;
        using Rounding = FloatWordPolicy::Rounding;
        struct Add {
          template<class... Args>
          auto operator()(Args&&... args) const
              -> decltype(Gecode::fp_add(std::forward<Args>(args)...));
        };
        struct Compare {
          template<class... Args>
          auto operator()(Args&&... args) const
              -> decltype(Gecode::fp_compare(std::forward<Args>(args)...));
        };
        struct Convert {
          template<class... Args>
          auto operator()(Args&&... args) const
              -> decltype(Gecode::fp_convert(std::forward<Args>(args)...));
        };
        /// Whether \a Posting accepts a space followed by \a Args
        template<class Posting, class... Args>
        constexpr bool accepts = std::is_invocable_v<Posting, Space&, Args...>;

        static_assert(accepts<Add, WordVar, WordVar, WordVar>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, IntPropLevel>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, IntVar>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, IntVar, IntPropLevel>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, Policy>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, Policy, IntPropLevel>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, IntVar, Policy>);
        static_assert(accepts<Add, WordVar, WordVar, WordVar, IntVar, Policy, IntPropLevel>);
        static_assert(!accepts<Add, Policy, WordVar, WordVar, WordVar>);
        static_assert(!accepts<Add, Policy, WordVar, WordVar, WordVar, IntPropLevel>);
        static_assert(!accepts<Add, Policy, WordVar, WordVar, WordVar, IntVar>);
        static_assert(!accepts<Add, Policy, WordVar, WordVar, WordVar, IntVar, IntPropLevel>);
        static_assert(!accepts<Add, WordVar, WordVar, WordVar, IntPropLevel, Policy>);
        static_assert(!accepts<Add, WordVar, WordVar, WordVar, Policy, IntVar>);
        static_assert(!accepts<Add, WordVar, WordVar, WordVar, Rounding>);

        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, IntPropLevel>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, IntVar>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, IntVar, IntPropLevel>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, Policy>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, Policy, IntPropLevel>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, IntVar, Policy>);
        static_assert(accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, IntVar, Policy, IntPropLevel>);
        static_assert(!accepts<Compare, Policy, WordVar, FloatWordRelType, WordVar, BoolVar>);
        static_assert(!accepts<Compare, Policy, WordVar, FloatWordRelType, WordVar, BoolVar, IntPropLevel>);
        static_assert(!accepts<Compare, Policy, WordVar, FloatWordRelType, WordVar, BoolVar, IntVar>);
        static_assert(!accepts<Compare, Policy, WordVar, FloatWordRelType, WordVar, BoolVar, IntVar, IntPropLevel>);
        static_assert(!accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, IntPropLevel, Policy>);
        static_assert(!accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, Policy, IntVar>);
        static_assert(!accepts<Compare, WordVar, FloatWordRelType, WordVar, BoolVar, Rounding>);

        static_assert(accepts<Convert, WordVar, WordVar>);
        static_assert(accepts<Convert, WordVar, WordVar, IntVar>);
        static_assert(accepts<Convert, WordVar, WordVar, Policy>);
        static_assert(accepts<Convert, WordVar, WordVar, IntVar, Policy>);
        static_assert(!accepts<Convert, Policy, WordVar, WordVar>);
        static_assert(!accepts<Convert, Policy, WordVar, WordVar, IntVar>);
        static_assert(!accepts<Convert, WordVar, WordVar, Policy, IntVar>);
        static_assert(!accepts<Convert, WordVar, WordVar, Rounding>);
      } // namespace PolicyPosition

      /// A posting without a policy has the solutions and flags of the same
      /// posting with a default-constructed policy, and an explicit policy
      /// after the status takes effect
      class DefaultPolicy : public Base {
        /// Result or truth, then status, of an assigned posting
        using Observed = std::array<long long, 2>;
        /// Return the outcome of the tie E2M1 3 + 0.5 under \a policy, or
        /// without a policy when it is absent
        static Observed tie_sum(const std::optional<FloatWordPolicy>& policy) {
          PostingSpace home;
          const WordVar three = float_constant(home, WDT_E2M1, 3.0);
          const WordVar half = float_constant(home, WDT_E2M1, 0.5);
          const WordVar r(home, WDT_E2M1);
          const IntVar status(home, 0, 31);
          if (policy)
            fp_add(home, three, half, r, status, *policy, IPL_DOM);
          else
            fp_add(home, three, half, r, status, IPL_DOM);
          if (home.status() == SS_FAILED || !r.assigned() || !status.assigned())
            return {-1, -1};
          return {static_cast<long long>(r.val()), status.val()};
        }
        /// Return the outcome of the subnormal E2M1 0.5 == +0 under
        /// \a policy, or without a policy when it is absent
        static Observed half_equals_zero(const std::optional<FloatWordPolicy>& policy) {
          PostingSpace home;
          const WordVar half = float_constant(home, WDT_E2M1, 0.5);
          const WordVar zero = float_constant(home, WDT_E2M1, 0.0);
          const BoolVar truth(home, 0, 1);
          const IntVar status(home, 0, 31);
          if (policy)
            fp_compare(home, half, FWRT_EQ, zero, truth, status, *policy, IPL_DOM);
          else
            fp_compare(home, half, FWRT_EQ, zero, truth, status, IPL_DOM);
          if (home.status() == SS_FAILED || !truth.assigned() || !status.assigned())
            return {-1, -1};
          return {truth.val(), status.val()};
        }
        /// Test that every policy-taking tiny posting has the same solutions
        /// with and without a default-constructed policy
        static bool omitted_matches_default(void) {
          for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN})
            for (Operation operation : operations) {
              if (!takes_policy(operation))
                continue;
              for (StatusArgument status : {StatusArgument::omitted, StatusArgument::posted}) {
                TinyDefaultSpace omitted(format, WVS_DEFAULT, operation, status, PolicyArgument::omitted);
                TinyDefaultSpace explicit_default(format, WVS_DEFAULT, operation, status,
                                                  PolicyArgument::explicit_default);
                const auto expected = solutions(explicit_default);
                if (expected.empty() || solutions(omitted) != expected) {
                  if (opt.log)
                    olog << ind(2) << "default policy " << format << " operation " << static_cast<int>(operation)
                         << " status " << (status == StatusArgument::posted) << '\n';
                  return false;
                }
              }
            }
          return true;
        }

      public:
        DefaultPolicy(void) : Base("Word::Float::DefaultPolicy") {}
        virtual bool run(void) {
          FloatWordPolicy rtz;
          rtz.rounding = FloatWordPolicy::RTZ;
          FloatWordPolicy flush_inputs;
          flush_inputs.input_subnormals = FloatWordPolicy::SignedZero;
          // RNE rounds the tie 3.5 to 4 (0x6), RTZ to 3 (0x5), both inexact.
          return omitted_matches_default() && tie_sum(std::nullopt) == Observed{0x6, 16} &&
                 tie_sum(rtz) == Observed{0x5, 16} && half_equals_zero(std::nullopt) == Observed{0, 0} &&
                 half_equals_zero(flush_inputs) == Observed{1, 0};
        }
      };

      /// Numerical float operations that take a status and a policy
      const Operation numerical_operations[] = {
          Operation::convert,        Operation::sqrt,
          Operation::compare,        Operation::minimum,
          Operation::maximum,        Operation::minimum_number,
          Operation::maximum_number, Operation::add,
          Operation::subtract,       Operation::multiply,
          Operation::divide,         Operation::fma};

      /// Return the public name that postings of \a operation report
      const char* posting_name(Operation operation) {
        switch (operation) {
        case Operation::convert:
          return "Word::fp_convert";
        case Operation::classify:
          return "Word::fp_classify";
        case Operation::negate:
          return "Word::fp_negate";
        case Operation::abs:
          return "Word::fp_abs";
        case Operation::sqrt:
          return "Word::fp_sqrt";
        case Operation::compare:
          return "Word::fp_compare";
        case Operation::minimum:
          return "Word::fp_minimum";
        case Operation::maximum:
          return "Word::fp_maximum";
        case Operation::minimum_number:
          return "Word::fp_minimum_number";
        case Operation::maximum_number:
          return "Word::fp_maximum_number";
        case Operation::add:
          return "Word::fp_add";
        case Operation::subtract:
          return "Word::fp_subtract";
        case Operation::multiply:
          return "Word::fp_multiply";
        case Operation::divide:
          return "Word::fp_divide";
        case Operation::fma:
          return "Word::fp_fma";
        default:
          return "";
        }
      }

      /// Call \a post with the status, unless \a status is null, and \a policy
      template<class Post>
      void post_status_policy(Post post, IntVar* status,
                              const FloatWordPolicy& policy) {
        if (status != nullptr)
          post(*status, policy);
        else
          post(policy);
      }

      /** \brief Post \a operation on \a w, with the status unless \a status is
       * null, under \a policy
       *
       * A comparison relates its operands to \a truth by FWRT_LE.
       */
      void post_numerical(Space& home, Operation operation,
                          const WordVarArgs& w, BoolVar truth, IntVar* status,
                          const FloatWordPolicy& policy) {
        switch (operation) {
        case Operation::convert:
          post_status_policy([&](auto&&... s) { fp_convert(home, w[0], w[1], s...); },
                             status, policy);
          break;
        case Operation::sqrt:
          post_status_policy([&](auto&&... s) { fp_sqrt(home, w[0], w[1], s...); },
                             status, policy);
          break;
        case Operation::compare:
          post_status_policy(
              [&](auto&&... s) { fp_compare(home, w[0], FWRT_LE, w[1], truth, s...); },
              status, policy);
          break;
        case Operation::minimum:
          post_status_policy([&](auto&&... s) { fp_minimum(home, w[0], w[1], w[2], s...); },
                             status, policy);
          break;
        case Operation::maximum:
          post_status_policy([&](auto&&... s) { fp_maximum(home, w[0], w[1], w[2], s...); },
                             status, policy);
          break;
        case Operation::minimum_number:
          post_status_policy(
              [&](auto&&... s) { fp_minimum_number(home, w[0], w[1], w[2], s...); },
              status, policy);
          break;
        case Operation::maximum_number:
          post_status_policy(
              [&](auto&&... s) { fp_maximum_number(home, w[0], w[1], w[2], s...); },
              status, policy);
          break;
        case Operation::add:
          post_status_policy([&](auto&&... s) { fp_add(home, w[0], w[1], w[2], s...); },
                             status, policy);
          break;
        case Operation::subtract:
          post_status_policy([&](auto&&... s) { fp_subtract(home, w[0], w[1], w[2], s...); },
                             status, policy);
          break;
        case Operation::multiply:
          post_status_policy([&](auto&&... s) { fp_multiply(home, w[0], w[1], w[2], s...); },
                             status, policy);
          break;
        case Operation::divide:
          post_status_policy([&](auto&&... s) { fp_divide(home, w[0], w[1], w[2], s...); },
                             status, policy);
          break;
        case Operation::fma:
          post_status_policy([&](auto&&... s) { fp_fma(home, w[0], w[1], w[2], w[3], s...); },
                             status, policy);
          break;
        default:
          GECODE_NEVER;
        }
      }

      /** \brief Space memory that \a post allocates in \a home, in bytes
       *
       * Space memory is handed out downwards from the current heap chunk,
       * so the distance between allocations made before and after \a post
       * counts what \a post allocated.
       */
      long allocated_bytes(Space& home, const std::function<void()>& post) {
        char* const first = static_cast<char*>(home.ralloc(8));
        char* const second = static_cast<char*>(home.ralloc(8));
        post();
        char* const third = static_cast<char*>(home.ralloc(8));
        return static_cast<long>((second - third) - (first - second));
      }

      /// Return \a count words of \a format whose domains are full
      WordVarArgs float_words(Space& home, WordDomainType format, int count) {
        WordVarArgs w;
        for (int i = 0; i < count; ++i)
          w << WordVar(home, format);
        return w;
      }

      /// Posting hygiene of the float operations
      class Hygiene : public Base {
        // A word of each type, non-float types with the given width.
        static WordVarArgs typed_words(Space& home,
                                       const std::vector<WordDomainType>& types,
                                       unsigned int width) {
          WordVarArgs w;
          for (WordDomainType type : types)
            w << (is_float_format(type) ? WordVar(home, type) : WordVar(home, width, type));
          return w;
        }
        // An invalid posting throws expected at the operation's name and
        // leaves the space without a new propagator or a new variable.
        bool rejects(Operation operation, const std::vector<WordDomainType>& types,
                     unsigned int width, StatusArgument status_argument, bool invalid_policy,
                     Outcome expected) {
          PostingSpace home;
          const WordVarArgs w = typed_words(home, types, width);
          BoolVar truth(home, 0, 1);
          IntVar status(home, 0, 31);
          FloatWordPolicy policy;
          if (invalid_policy)
            policy.tininess = static_cast<FloatWordPolicy::Tininess>(99);
          const unsigned int propagators = PropagatorGroup::all.size(home);
          Outcome actual = Outcome::posted;
          std::string message;
          // Space memory is handed out downwards from the current heap
          // chunk, so equal steps between three allocations around the
          // posting show that it allocated nothing.
          char* const first = static_cast<char*>(home.ralloc(8));
          char* const second = static_cast<char*>(home.ralloc(8));
          try {
            switch (operation) {
            case Operation::classify:
              fp_classify(home, w[0], FWC_NAN, truth);
              break;
            case Operation::negate:
              fp_negate(home, w[0], w[1]);
              break;
            case Operation::abs:
              fp_abs(home, w[0], w[1]);
              break;
            default:
              post_numerical(home, operation, w, truth,
                             status_argument == StatusArgument::posted ? &status : nullptr,
                             policy);
            }
          } catch (const Gecode::Word::OutOfLimits& e) {
            actual = Outcome::out_of_limits;
            message = e.what();
          } catch (const Gecode::Word::FormatMismatch& e) {
            actual = Outcome::format_mismatch;
            message = e.what();
          } catch (const Gecode::Exception& e) {
            actual = Outcome::other;
            message = e.what();
          }
          char* const third = static_cast<char*>(home.ralloc(8));
          const long bytes = static_cast<long>((second - third) - (first - second));
          const std::string prefix = std::string(posting_name(operation)) + ": ";
          if (actual == expected && PropagatorGroup::all.size(home) == propagators &&
              bytes == 0 && message.compare(0, prefix.size(), prefix) == 0)
            return true;
          if (opt.log) {
            olog << ind(2) << posting_name(operation) << " on";
            for (WordDomainType type : types)
              olog << ind(2) << ' ' << type;
            olog << ind(2) << (status_argument == StatusArgument::posted ? " with" : " without")
                 << " status, " << (invalid_policy ? "invalid" : "valid") << " policy: "
                 << outcome_name(actual) << " (" << message << "), expected "
                 << outcome_name(expected) << "; " << bytes << " bytes, "
                 << PropagatorGroup::all.size(home) - propagators << " propagators\n";
          }
          return false;
        }
        // Invalid postings of operation with words of format.
        bool invalid_postings(Operation operation, WordDomainType format,
                              StatusArgument status_argument) {
          const int n = arity(operation);
          const unsigned int width = float_word_width(format);
          const std::vector<WordDomainType> uniform(n, format);
          const bool policy = takes_policy(operation);
          for (bool invalid_policy : {false, true}) {
            if (invalid_policy && !policy)
              continue;
            for (int i = 0; i < n; ++i) {
              std::vector<WordDomainType> types = uniform;
              types[i] = WDT_UNSIGNED;
              if (!rejects(operation, types, width, status_argument, invalid_policy,
                           Outcome::out_of_limits))
                return false;
              if (n > 1 && same_format(operation)) {
                types[i] = format == WDT_E4M3FN ? WDT_E2M1 : WDT_E4M3FN;
                if (!rejects(operation, types, width, status_argument, invalid_policy,
                             Outcome::format_mismatch))
                  return false;
              }
            }
            if (!supports_binary16(operation) &&
                !rejects(operation, std::vector<WordDomainType>(n, WDT_BINARY16), 16,
                         status_argument, invalid_policy, Outcome::out_of_limits))
              return false;
          }
          return !policy || rejects(operation, uniform, width, status_argument, true,
                                    Outcome::out_of_limits);
        }

        /// Propagators and space memory of a posting
        struct Footprint {
          unsigned int propagators;
          long bytes;
        };
        // Footprint of operation on unassigned words of format.
        static Footprint footprint(Operation operation, WordDomainType format,
                                   StatusArgument status_argument) {
          PostingSpace home;
          const WordVarArgs w = float_words(home, format, arity(operation));
          BoolVar truth(home, 0, 1);
          IntVar status(home, 0, 31);
          const long bytes = allocated_bytes(home, [&] {
            post_numerical(home, operation, w, truth,
                           status_argument == StatusArgument::posted ? &status : nullptr,
                           FloatWordPolicy());
          });
          return {PropagatorGroup::all.size(home), bytes};
        }
        // Omitting the status allocates no status variable and no actor
        // for it: no more propagators than posting a status, fewer for a
        // wide decomposition that relates the flags to the status, and a
        // single actor on tiny words and for a square root. A single actor
        // takes no more memory without the status; larger postings can
        // cross heap chunks, so their memory is not compared.
        bool omitted_status(Operation operation, WordDomainType format) {
          const Footprint with = footprint(operation, format, StatusArgument::posted);
          const Footprint without = footprint(operation, format, StatusArgument::omitted);
          const bool tiny = float_word_width(format) <= 8;
          const bool decomposed =
              !tiny && operation != Operation::convert && operation != Operation::sqrt &&
              operation != Operation::add && operation != Operation::subtract &&
              operation != Operation::multiply && operation != Operation::divide &&
              operation != Operation::fma;
          const bool compares_memory = with.propagators == 1;
          const bool ok = without.propagators <= with.propagators &&
                          (!decomposed || without.propagators < with.propagators) &&
                          ((!tiny && operation != Operation::sqrt) || without.propagators == 1) &&
                          (!compares_memory || without.bytes <= with.bytes);
          if (ok)
            return true;
          if (opt.log)
            olog << ind(2) << posting_name(operation) << " on " << format << ": "
                 << without.propagators << " propagators and " << without.bytes
                 << " bytes without status, " << with.propagators << " and " << with.bytes
                 << " with status\n";
          return false;
        }

        // A wide class extracts the magnitude and posts only the relations
        // it needs on it: one for finiteness, zero, infinity and the NaN
        // classes, two and their conjunction for the others. It used to
        // post 7 to 11 propagators.
        static bool wide_classify_size(void) {
          for (int cls = 0; cls < 9; ++cls) {
            const FloatWordClass c = static_cast<FloatWordClass>(cls);
            PostingSpace home;
            WordVar x(home, WDT_FP32);
            BoolVar truth(home, 0, 1);
            fp_classify(home, x, c, truth);
            const unsigned int limit =
                c == FWC_SIGN ? 1U
                : c == FWC_SUBNORMAL || c == FWC_NORMAL || c == FWC_SIGNALING_NAN
                    ? 4U
                    : 2U;
            if (PropagatorGroup::all.size(home) > limit)
              return false;
          }
          return true;
        }

      public:
        Hygiene(void) : Base("Word::Float::Hygiene") {}
        virtual bool run(void) {
          for (Operation operation : operations) {
            for (WordDomainType format : {WDT_E2M1, WDT_FP32})
              for (StatusArgument status_argument :
                   {StatusArgument::omitted, StatusArgument::posted})
                if ((status_argument == StatusArgument::omitted || takes_policy(operation)) &&
                    !invalid_postings(operation, format, status_argument))
                  return false;
          }
          for (Operation operation : numerical_operations)
            for (WordDomainType format : {WDT_E2M1, WDT_E4M3FN, WDT_BF16, WDT_FP32})
              if (!omitted_status(operation, format))
                return false;
          return wide_classify_size();
        }
      };

      /// Wide postings with assigned inputs decide at posting
      class AssignedWide : public Base {
        // Encodings of 1.5, 3, the least subnormal, infinity, a signaling
        // NaN, -1 and the greatest finite value.
        static std::vector<WordValue> codes(WordDomainType format) {
          const std::vector<WordValue> fp32 = {0x3fc00000, 0x40400000, 0x00000001, 0x7f800000,
                                               0x7fa00000, 0xbf800000, 0x7f7fffff};
          std::vector<WordValue> result;
          for (WordValue code : fp32)
            result.push_back(format == WDT_FP32 ? code : code >> 16 | (code & 0xffff ? 1 : 0));
          return result;
        }
        /// Values of result and status after propagation
        struct Observed {
          bool failed;
          WordValue result;
          int status;
        };
        // Post operation on words of formats with the inputs assigned to
        // inputs before posting (decide) or after posting (reference).
        static Observed observe(Operation operation, const std::vector<WordDomainType>& formats,
                                const std::vector<WordValue>& inputs, StatusArgument status_argument,
                                const FloatWordPolicy& policy, bool decide, bool& posted_actor) {
          PostingSpace home;
          WordVarArgs w;
          for (WordDomainType format : formats)
            w << WordVar(home, format);
          BoolVar truth(home, 0, 1);
          IntVar status(home, 0, 31);
          IntVar* s = status_argument == StatusArgument::posted ? &status : nullptr;
          const int n = static_cast<int>(inputs.size());
          if (decide)
            for (int i = 0; i < n; ++i)
              dom(home, w[i], inputs[static_cast<size_t>(i)]);
          post_numerical(home, operation, w, truth, s, policy);
          posted_actor = PropagatorGroup::all.size(home) != 0;
          if (!decide)
            for (int i = 0; i < n; ++i)
              dom(home, w[i], inputs[static_cast<size_t>(i)]);
          else if (!home.failed() &&
                   (!w[n].assigned() || (s != nullptr && !status.assigned())))
            return {true, 0, -1};
          if (home.status() == SS_FAILED)
            return {true, 0, 0};
          return {false, w[n].val(), s != nullptr ? status.val() : 0};
        }
        bool decides(Operation operation, const std::vector<WordDomainType>& formats,
                     const std::vector<WordValue>& inputs, StatusArgument status_argument,
                     const FloatWordPolicy& policy) {
          bool posted_actor = false, reference_actor = false;
          const Observed decided =
              observe(operation, formats, inputs, status_argument, policy, true, posted_actor);
          const Observed reference = observe(operation, formats, inputs, StatusArgument::posted,
                                             policy, false, reference_actor);
          const bool status_matches =
              status_argument == StatusArgument::omitted || decided.status == reference.status;
          if (!posted_actor && !decided.failed && !reference.failed &&
              decided.result == reference.result && status_matches)
            return true;
          if (opt.log) {
            olog << ind(2) << posting_name(operation) << " on " << formats[0];
            for (WordValue input : inputs)
              olog << ind(2) << " 0x" << std::hex << input << std::dec;
            olog << ind(2) << (posted_actor ? ": posted an actor" : "") << ", decided "
                 << (decided.failed ? "failed " : "") << std::hex << decided.result << std::dec
                 << '/' << decided.status << ", reference " << std::hex << reference.result
                 << std::dec << '/' << reference.status << '\n';
          }
          return false;
        }
        // A result that excludes the decided value fails at posting.
        static bool fails_excluded(void) {
          PostingSpace home;
          WordVar a(home, WDT_FP32), b(home, WDT_FP32), r(home, WDT_FP32);
          dom(home, a, WordValue(0x3fc00000));
          dom(home, b, WordValue(0x40400000));
          // 1.5 + 3 is 4.5 (0x40900000), not 4.
          dom(home, r, WordValue(0x40800000));
          fp_add(home, a, b, r);
          return home.failed() && PropagatorGroup::all.size(home) == 0;
        }
        // A status that excludes the decided flags fails at posting.
        static bool convert_fails_excluded(void) {
          PostingSpace home;
          WordVar source(home, WDT_FP32), result(home, WDT_BF16);
          IntVar status(home, 16, 16);
          dom(home, source, WordValue(0x3fc00000));
          // 1.5 is exact in BF16, so the status is 0, not inexact.
          fp_convert(home, source, result, status);
          return home.failed() && PropagatorGroup::all.size(home) == 0;
        }

      public:
        AssignedWide(void) : Base("Word::Float::AssignedWide") {}
        virtual bool run(void) {
          FloatWordPolicy flushing;
          flushing.rounding = FloatWordPolicy::RTZ;
          flushing.input_subnormals = FloatWordPolicy::SignedZero;
          flushing.output_subnormals = FloatWordPolicy::SignedZero;
          flushing.nan = FloatWordPolicy::Canonical;
          const Operation binary[] = {Operation::add, Operation::subtract, Operation::multiply,
                                      Operation::divide};
          for (WordDomainType format : {WDT_FP32, WDT_BF16}) {
            const std::vector<WordValue> c = codes(format);
            const std::vector<WordDomainType> two(2, format), three(3, format), four(4, format);
            for (const FloatWordPolicy& policy : {FloatWordPolicy(), flushing})
              for (StatusArgument s : {StatusArgument::omitted, StatusArgument::posted})
                for (WordValue x : c) {
                  if (!decides(Operation::sqrt, two, {x}, s, policy))
                    return false;
                  for (WordValue y : c) {
                    for (Operation operation : binary)
                      if (!decides(operation, three, {x, y}, s, policy))
                        return false;
                    for (WordValue z : {c[0], c[2], c[3], c[4]})
                      if (!decides(Operation::fma, four, {x, y, z}, s, policy))
                        return false;
                  }
                }
          }
          // BF16 * BF16 + FP32 with an FP32 result.
          const std::vector<WordValue> bf16 = codes(WDT_BF16), fp32 = codes(WDT_FP32);
          for (StatusArgument s : {StatusArgument::omitted, StatusArgument::posted})
            for (WordValue x : bf16)
              for (WordValue z : fp32)
                if (!decides(Operation::fma, {WDT_BF16, WDT_BF16, WDT_FP32, WDT_FP32},
                             {x, bf16[1], z}, s, FloatWordPolicy()))
                  return false;
          // Conversions between wide formats, rounding, overflowing and
          // underflowing among the codes.
          const std::vector<WordValue> binary16 = {0x3e00, 0x4200, 0x0001, 0x7c00, 0x7d00, 0xbc00, 0x7bff};
          const std::vector<std::vector<WordDomainType>> conversions = {
              {WDT_FP32, WDT_FP32}, {WDT_FP32, WDT_BF16}, {WDT_FP32, WDT_BINARY16}, {WDT_BF16, WDT_FP32},
              {WDT_BINARY16, WDT_BF16}};
          for (const std::vector<WordDomainType>& formats : conversions)
            for (const FloatWordPolicy& policy : {FloatWordPolicy(), flushing})
              for (StatusArgument s : {StatusArgument::omitted, StatusArgument::posted})
                for (WordValue x : formats[0] == WDT_BINARY16 ? binary16 : codes(formats[0]))
                  if (!decides(Operation::convert, formats, {x}, s, policy))
                    return false;
          return fails_excluded() && convert_fails_excluded();
        }
      };

      /// Wide square roots without status prune as with a free status
      class OmittedSqrt : public Base {
        /// Total-order bounds and cube of a word
        struct Domain {
          WordValue minimum, maximum, lo, hi;
        };
        /// Domains of the source and result, and whether they are one word
        struct Case {
          Domain source, result;
          bool aliased;
        };
        // The domain of a word of format.
        static WordVar word(Space& home, WordDomainType format, const Domain& d) {
          const unsigned int width = float_word_width(format);
          return WordVar(home, width, format, d.minimum, d.maximum, cube_bits(width, d.lo, d.hi));
        }
        /// Domain of a word after propagation
        struct Observed {
          bool failed;
          WordValue minimum, maximum, lo, hi;
          bool operator==(const Observed& other) const {
            return failed == other.failed &&
                   (failed || (minimum == other.minimum && maximum == other.maximum &&
                               lo == other.lo && hi == other.hi));
          }
        };
        static Observed observed(bool failed, const WordVar& x) {
          if (failed)
            return {true, 0, 0, 0, 0};
          return {false, x.minimum(), x.maximum(), x.lo(), x.hi()};
        }
        // Source and result after propagating the square root of the case,
        // with a free status when posted.
        static std::pair<Observed, Observed> propagate(WordDomainType format, const Case& c,
                                                       StatusArgument status_argument,
                                                       const FloatWordPolicy& policy) {
          PostingSpace home;
          WordVar x = word(home, format, c.source);
          WordVar y = c.aliased ? x : word(home, format, c.result);
          IntVar status(home, 0, 31);
          if (status_argument == StatusArgument::posted)
            fp_sqrt(home, x, y, status, policy);
          else
            fp_sqrt(home, x, y, policy);
          const bool failed = home.status() == SS_FAILED;
          return {observed(failed, x), observed(failed, y)};
        }
        // The cases on FP32, scaled to BF16 by dropping the low half.
        static std::vector<Case> cases(WordDomainType format) {
          const Domain full = {0xffffffff, 0x7fffffff, 0, 0xffffffff};
          const std::vector<Case> fp32 = {
              // 1 to 4, and the root bounded by 1.5 and 2.
              {{0x3f800000, 0x40800000, 0, 0xffffffff}, full, false},
              {full, {0x3fc00000, 0x40000000, 0, 0xffffffff}, false},
              // -4 to 1, and infinity through the positive NaNs.
              {{0xc0800000, 0x3f800000, 0, 0xffffffff}, full, false},
              {{0x7f800000, 0x7fffffff, 0, 0xffffffff}, full, false},
              // A sparse cube near 2 inside bounds, and a cube on the root.
              {{0x40000000, 0x40100000, 0x40000000, 0x40150015}, full, false},
              {full, {0x3f800000, 0x3fc00000, 0x3f800000, 0x3f8f00ff}, false},
              // Aliased: the fixed points 0, 1 and infinity.
              {{0x00000000, 0x3f800000, 0, 0xffffffff}, full, true},
              {full, full, true},
              {{0x3f000000, 0x7f800000, 0x3f000000, 0x7fffffff}, full, true}};
          if (format == WDT_FP32)
            return fp32;
          std::vector<Case> result;
          for (Case c : fp32) {
            for (Domain* d : {&c.source, &c.result}) {
              d->minimum >>= 16;
              d->maximum >>= 16;
              d->lo >>= 16;
              d->hi >>= 16;
            }
            result.push_back(c);
          }
          return result;
        }

      public:
        OmittedSqrt(void) : Base("Word::Float::OmittedSqrt") {}
        virtual bool run(void) {
          FloatWordPolicy flushing;
          flushing.rounding = FloatWordPolicy::RUP;
          flushing.input_subnormals = FloatWordPolicy::SignedZero;
          flushing.nan = FloatWordPolicy::Canonical;
          for (WordDomainType format : {WDT_FP32, WDT_BF16})
            for (const FloatWordPolicy& policy : {FloatWordPolicy(), flushing})
              for (const Case& c : cases(format)) {
                const auto with = propagate(format, c, StatusArgument::posted, policy);
                const auto without = propagate(format, c, StatusArgument::omitted, policy);
                if (with.first == without.first && with.second == without.second)
                  continue;
                if (opt.log)
                  olog << ind(2) << "fp_sqrt on " << format << " from 0x" << std::hex << c.source.minimum
                       << " to 0x" << c.source.maximum << std::dec
                       << (c.aliased ? " aliased" : "") << " prunes differently without status\n";
                return false;
              }
          return true;
        }
      };

      Observations observations;
      Lifecycle lifecycle;
      FloatTyped float_typed;
      Constants constants;
      Formats formats;
      TinyDefaults tiny_defaults;
      DefaultPolicy default_policy;
      Hygiene hygiene;
      AssignedWide assigned_wide;
      OmittedSqrt omitted_sqrt;
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
