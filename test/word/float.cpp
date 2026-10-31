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
#include <functional>
#include <memory>
#include <type_traits>

namespace Test {
  namespace Word {
    namespace Float {
      using namespace Gecode;

      class ObservationSpace : public Space {
      public:
        FloatWordVar x, negative, absolute, alias;
        BoolVarArray classes;
        ObservationSpace(FloatWordFormat format)
            : x(*this, format), negative(*this, format),
              absolute(*this, format), classes(*this, 9, 0, 1) {
          if (float_word_width(format) <= 8) {
            x = FloatWordVar(*this, format, x.bits(), IntVar(*this, 0, 255));
            negative = FloatWordVar(*this, format, negative.bits(),
                                    IntVar(*this, 0, 255));
            absolute = FloatWordVar(*this, format, absolute.bits(),
                                    IntVar(*this, 0, 255));
          }
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
      bool check(ObservationSpace& s, WordValue code, Category category) {
        const unsigned int width = s.x.bits().width();
        const WordValue sign = WordValue(1) << (width - 1);
        dom(s, s.x.bits(), code);
        if (s.status() == SS_FAILED || !s.negative.bits().assigned() ||
            !s.absolute.bits().assigned() ||
            s.negative.bits().val() != (code ^ sign) ||
            s.absolute.bits().val() != (code & (sign - 1)))
          return false;
        const bool expected[] = {
            category == Category::Zero,
            category == Category::Subnormal,
            category == Category::Normal,
            category <= Category::Normal,
            category == Category::Infinity,
            category == Category::QuietNan || category == Category::SignalingNan,
            category == Category::QuietNan,
            category == Category::SignalingNan,
            (code & sign) != 0};
        for (int c = 0; c < 9; ++c)
          if (!s.classes[c].assigned() || s.classes[c].val() != expected[c])
            return false;
        return true;
      }

      class Observations : public Base {
      public:
        Observations(void) : Base("Word::Float::Observations") {}
        virtual bool run(void) {
          // All 16 and 256 encodings, both signs, including the two E4M3FN
          // NaNs.
          for (unsigned int code = 0; code < 256; ++code) {
            ObservationSpace fp8(FWF_E4M3FN);
            unsigned int m = code % 128;
            Category cls = m == 0     ? Category::Zero
                           : m < 8    ? Category::Subnormal
                           : m == 127 ? Category::QuietNan
                                      : Category::Normal;
            if (!check(fp8, code, cls))
              return false;
            if (code < 16) {
              ObservationSpace fp4(FWF_E2M1);
              m = code % 8;
              if (!check(fp4, code,
                         m == 0   ? Category::Zero
                         : m == 1 ? Category::Subnormal
                                  : Category::Normal))
                return false;
            }
          }
          struct Boundary {
            FloatWordFormat format;
            WordValue code;
            Category category;
          };
          const Boundary boundaries[] = {{FWF_FP32, 0x00000000, Category::Zero},
                                         {FWF_FP32, 0x00000001, Category::Subnormal},
                                         {FWF_FP32, 0x007fffff, Category::Subnormal},
                                         {FWF_FP32, 0x00800000, Category::Normal},
                                         {FWF_FP32, 0x3f800000, Category::Normal},
                                         {FWF_FP32, 0x7f7fffff, Category::Normal},
                                         {FWF_FP32, 0x7f800000, Category::Infinity},
                                         {FWF_FP32, 0x7f800001, Category::SignalingNan},
                                         {FWF_FP32, 0x7fbfffff, Category::SignalingNan},
                                         {FWF_FP32, 0x7fc00000, Category::QuietNan},
                                         {FWF_FP32, 0x7fc12345, Category::QuietNan},
                                         {FWF_FP32, 0x7fffffff, Category::QuietNan},
                                         {FWF_BF16, 0x0000, Category::Zero},
                                         {FWF_BF16, 0x0001, Category::Subnormal},
                                         {FWF_BF16, 0x007f, Category::Subnormal},
                                         {FWF_BF16, 0x0080, Category::Normal},
                                         {FWF_BF16, 0x3f80, Category::Normal},
                                         {FWF_BF16, 0x7f7f, Category::Normal},
                                         {FWF_BF16, 0x7f80, Category::Infinity},
                                         {FWF_BF16, 0x7f81, Category::SignalingNan},
                                         {FWF_BF16, 0x7fbf, Category::SignalingNan},
                                         {FWF_BF16, 0x7fc0, Category::QuietNan},
                                         {FWF_BF16, 0x7fff, Category::QuietNan},
                                         {FWF_BINARY16, 0x0000, Category::Zero},
                                         {FWF_BINARY16, 0x0001, Category::Subnormal},
                                         {FWF_BINARY16, 0x03ff, Category::Subnormal},
                                         {FWF_BINARY16, 0x0400, Category::Normal},
                                         {FWF_BINARY16, 0x3c00, Category::Normal},
                                         {FWF_BINARY16, 0x7bff, Category::Normal},
                                         {FWF_BINARY16, 0x7c00, Category::Infinity},
                                         {FWF_BINARY16, 0x7c01, Category::SignalingNan},
                                         {FWF_BINARY16, 0x7dff, Category::SignalingNan},
                                         {FWF_BINARY16, 0x7e00, Category::QuietNan},
                                         {FWF_BINARY16, 0x7fff, Category::QuietNan}};
          for (const Boundary& b : boundaries)
            for (unsigned int negative = 0; negative < 2; ++negative) {
              ObservationSpace s(b.format);
              const WordValue sign = WordValue(negative)
                                     << (float_word_width(b.format) - 1);
              if (!check(s, b.code | sign, b.category))
                return false;
              FloatWordVar constant =
                  FloatWordVar::from_bits(s, b.format, b.code | sign);
              if (constant.bits().val() != (b.code | sign) ||
                  constant.has_encoding())
                return false;
            }
          return true;
        }
      };

      class Lifecycle : public Base {
        static bool holes_and_clone(void) {
          for (FloatWordFormat format : {FWF_E2M1, FWF_E4M3FN}) {
            ObservationSpace source(format);
            if (source.status() == SS_FAILED)
              return false;
            const int sign = 1 << (float_word_width(format) - 1);
            std::unique_ptr<ObservationSpace> clone(
                static_cast<ObservationSpace*>(source.clone()));
            if (clone->alias.bits().varimp() != clone->x.bits().varimp() ||
                clone->alias.encoding().varimp() !=
                    clone->x.encoding().varimp() ||
                clone->x.bits().varimp() == source.x.bits().varimp() ||
                clone->x.encoding().varimp() == source.x.encoding().varimp())
              return false;
            rel(*clone, clone->alias.encoding(), IRT_NQ, 3);
            rel(*clone, clone->alias.encoding(), IRT_NQ, 3 + sign);
            if (clone->status() == SS_FAILED ||
                clone->negative.encoding().in(3) ||
                clone->negative.encoding().in(3 + sign) ||
                clone->absolute.encoding().in(3) ||
                !source.x.encoding().in(3) || !source.absolute.encoding().in(3))
              return false;
            // Classification reuses the same domain, including reverse
            // propagation.
            rel(*clone, clone->classes[static_cast<int>(FWC_SIGN)], IRT_EQ, 0);
            if (clone->status() == SS_FAILED ||
                clone->x.encoding().max() >= sign)
              return false;
            dom(source, source.x.bits(), 3);
            if (!check(source, 3, format == FWF_E2M1 ? Category::Normal : Category::Subnormal))
              return false;
          }
          ObservationSpace wide(FWF_BF16);
          if (wide.status() == SS_FAILED)
            return false;
          std::unique_ptr<ObservationSpace> copy(
              static_cast<ObservationSpace*>(wide.clone()));
          FloatWordVar half = copy->x.reinterpret_bits(FWF_BINARY16);
          if (half.bits().varimp() != copy->alias.bits().varimp() ||
              half.has_encoding() || half.format() != FWF_BINARY16)
            return false;
          if (!check(*copy, 0xff81, Category::SignalingNan) || wide.x.bits().assigned())
            return false;
          return true;
        }
        static bool aliases_and_branching(void) {
          for (FloatWordFormat format :
               {FWF_E2M1, FWF_E4M3FN, FWF_BF16, FWF_FP32, FWF_BINARY16}) {
            ObservationSpace s(format);
            fp_negate(s, s.x, s.alias);
            if (s.status() != SS_FAILED)
              return false;
            ObservationSpace absolute(format);
            fp_abs(absolute, absolute.x, absolute.alias);
            if (absolute.status() == SS_FAILED ||
                (absolute.x.bits().hi() >> (float_word_width(format) - 1)) != 0)
              return false;
          }
          ObservationSpace root(FWF_E2M1);
          branch(root, WordVarArgs{root.x.bits()}, WORD_VAL_LSB());
          DFS<ObservationSpace> search(&root);
          int seen = 0;
          while (std::unique_ptr<ObservationSpace> solution{search.next()}) {
            if (!solution->x.encoding().assigned() ||
                !solution->negative.encoding().assigned() ||
                !solution->absolute.encoding().assigned())
              return false;
            seen |= 1 << solution->x.encoding().val();
          }
          return seen == 65535;
        }
        static bool errors_and_policies(void) {
          static_assert(std::is_copy_assignable<FloatWordPolicy>::value,
                        "Policies are plain values");
          ObservationSpace s(FWF_BF16);
          int caught = 0;
          try {
            FloatWordVar bad(FWF_FP32, s.x.bits());
          } catch (const Gecode::Word::WidthMismatch&) {
            ++caught;
          }
          try {
            s.x.reinterpret_bits(FWF_E2M1);
          } catch (const Gecode::Word::WidthMismatch&) {
            ++caught;
          }
          try {
            fp_negate(s, s.x, s.x.reinterpret_bits(FWF_BINARY16));
          } catch (const Gecode::Word::FormatMismatch&) {
            ++caught;
          }
          try {
            FloatWordVar::from_bits(s, FWF_BF16, 0x10000);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            FloatWordVar bad(s, static_cast<FloatWordFormat>(99));
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            FloatWordVar bad(s, FWF_BF16, s.x.bits(), IntVar(s, 0, 255));
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            FloatWordVar tiny(s, FWF_E2M1);
            fp_abs(s, tiny, tiny);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          try {
            fp_classify(s, s.x, static_cast<FloatWordClass>(99), s.classes[0]);
          } catch (const Gecode::Word::OutOfLimits&) {
            ++caught;
          }
          return caught == 8;
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
          ObservationSpace s(FWF_BF16);
          FloatWordVar a = s.x, b = s.negative, r = s.absolute;
          for (const Policy& p : invalid)
            if (!rejects([&] { fp_convert(s, p, a, r); }))
              return false;
          const Policy& p = invalid[6];
          BoolVar truth = s.classes[0];
          return rejects([&] { fp_compare(s, p, a, FWRT_EQ, b, truth); }) &&
                 rejects([&] { fp_minimum(s, p, a, b, r); }) &&
                 s.status() != SS_FAILED;
        }
        // A posting keeps its own policy copy.
        static bool policy_copied_at_posting(void) {
          ObservationSpace s(FWF_FP32);
          FloatWordVar half(s, FWF_BF16);
          FloatWordPolicy policy;
          policy.rounding = FloatWordPolicy::RTZ;
          fp_convert(s, policy, s.x, half);
          policy.rounding = FloatWordPolicy::RNE;
          // 0x3f80c000 rounds to 0x3f81 under RNE and to 0x3f80 under RTZ.
          dom(s, s.x.bits(), 0x3f80c000);
          return s.status() != SS_FAILED && half.bits().assigned() &&
                 half.bits().val() == 0x3f80;
        }

      public:
        Lifecycle(void) : Base("Word::Float::Lifecycle") {}
        virtual bool run(void) {
          return holes_and_clone() && aliases_and_branching() &&
                 errors_and_policies() && invalid_policy_fields() &&
                 policy_copied_at_posting();
        }
      };
      Observations observations;
      Lifecycle lifecycle;
    } // namespace Float
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
