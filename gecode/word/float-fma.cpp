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

#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float-wide.hpp>
#include <gecode/word/float.hpp>
#include <gecode/word/float/fma.hpp>

namespace Gecode {
  namespace {
    /** \brief Return the format of a after checking the formats of a, b, c
     * and result and then the policy
     *
     * All words share one format, or a and b are BF16 and c and result
     * FP32.
     */
    WordDomainType validate_fma(const Policy& policy, WordVar a, WordVar b,
                                WordVar c, WordVar result) {
      const char* name = "Word::fp_fma";
      const WordDomainType format = Word::Float::float_format(a, name);
      const WordDomainType b_format = Word::Float::float_format(b, name);
      const WordDomainType c_format = Word::Float::float_format(c, name);
      const WordDomainType result_format =
          Word::Float::float_format(result, name);
      const bool same = b_format == format && c_format == format &&
                        result_format == format;
      const bool mixed = format == WDT_BF16 && b_format == WDT_BF16 &&
                         c_format == WDT_FP32 && result_format == WDT_FP32;
      if (!same && !mixed)
        throw Word::FormatMismatch(name);
      if (format != WDT_BF16 && format != WDT_FP32 && format != WDT_E2M1 &&
          format != WDT_E4M3FN)
        throw Word::OutOfLimits(name);
      Word::Float::check(policy, name);
      return format;
    }

    void post_fma(Home home, const Policy& policy, WordVar a, WordVar b,
                  WordVar c, WordVar result, IntVar* status,
                  IntPropLevel ipl) {
      const WordDomainType format = validate_fma(policy, a, b, c, result);
      const bool wide = format == WDT_BF16 || format == WDT_FP32;
      GECODE_POST;
      if (status != nullptr) {
        Int::IntView flags(*status);
        GECODE_ME_FAIL(flags.gq(home, 0));
        GECODE_ME_FAIL(flags.lq(home, 31));
      }
      if (wide && a.assigned() && b.assigned() && c.assigned()) {
        const WideResult outcome = evaluate_wide_fma(
            WideFormat(format), WideFormat(result.domain_type()), policy,
            a.val(), b.val(), c.val());
        if (status != nullptr)
          GECODE_ME_FAIL(Int::IntView(*status).eq(home, outcome.status));
        GECODE_ME_FAIL(Word::WordView(result).eq(home, outcome.code));
        return;
      }
      if (wide) {
        (void)new (home) WideFma(home, a, b, c, result, status, policy);
        return;
      }
      const TinyFma tiny{TinyFormat(format), policy};
      const Word::Float::Consistency level = Word::Float::consistency(ipl);
      if (status != nullptr)
        GECODE_ES_FAIL(
            (Word::Float::TinyTernary<TinyFma, Int::IntView>::post(
                home, a, b, c, result, *status, tiny, level)));
      else
        GECODE_ES_FAIL(
            (Word::Float::TinyTernary<TinyFma, Word::Float::OmittedStatus>::
                 post(home, a, b, c, result, Word::Float::OmittedStatus(), tiny,
                      level)));
    }
  } // namespace

  void fp_fma(Home home, WordVar a, WordVar b, WordVar c, WordVar result,
              IntPropLevel ipl) {
    fp_fma(home, a, b, c, result, FloatWordPolicy(), ipl);
  }
  void fp_fma(Home home, WordVar a, WordVar b, WordVar c, WordVar result,
              IntVar status, IntPropLevel ipl) {
    fp_fma(home, a, b, c, result, status, FloatWordPolicy(), ipl);
  }
  void fp_fma(Home home, WordVar a, WordVar b, WordVar c, WordVar result,
              const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_fma(home, policy, a, b, c, result, nullptr, ipl);
  }
  void fp_fma(Home home, WordVar a, WordVar b, WordVar c, WordVar result,
              IntVar status, const FloatWordPolicy& policy,
              IntPropLevel ipl) {
    post_fma(home, policy, a, b, c, result, &status, ipl);
  }
} // namespace Gecode

// STATISTICS: word-post
