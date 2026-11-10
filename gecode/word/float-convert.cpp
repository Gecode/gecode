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

#include <algorithm>
#include <gecode/float-word.hh>
#include <gecode/word/float.hpp>
#include <gecode/word/float/convert.hpp>

namespace Gecode {
  namespace Word {
    namespace {
      // Return a Boolean variable that is true exactly when FP32 source has
      // class cls.
      BoolVar class_indicator(Home home, WordVar source, FloatWordClass cls) {
        BoolVar truth(home, 0, 1);
        Float::classify(home, source, WDT_FP32, cls, truth);
        return truth;
      }

      // FP32 self-conversion consists only of classification, bit operations
      // and selection. Keep this wide identity relation out of the bounded scan
      // actor.
      void post_self_conversion(Home home, const Policy& policy, WordVar source,
                                WordVar result, IntVar status) {
        constexpr Format fp32 = Float::format_facts(WDT_FP32);
        WordVar value = source;
        BoolVar nan = class_indicator(home, source, FWC_NAN);
        WordVar quiet(home, 32), selected(home, 32);
        if (policy.nan == Policy::Canonical)
          dom(home, quiet, fp32.canonical_nan());
        else
          rel(home, value, WOT_OR, fp32.quiet(), quiet);
        ite(home, nan, quiet, value, selected);
        value = selected;
        BoolVar flushed(home, 0, 0);
        if (policy.input_subnormals == Policy::SignedZero ||
            policy.output_subnormals == Policy::SignedZero) {
          BoolVar subnormal = class_indicator(home, source, FWC_SUBNORMAL);
          WordVar zero(home, 32), next(home, 32);
          rel(home, source, WOT_AND, fp32.sign(), zero);
          ite(home, subnormal, zero, value, next);
          value = next;
          if (policy.input_subnormals == Policy::Preserve)
            flushed = subnormal;
        }
        if (policy.zeros == Policy::Positive) {
          ite(home, class_indicator(home, value, FWC_ZERO), WordValue(0), value,
              result);
        } else {
          rel(home, value, WRT_EQ, result);
        }
        if (status.varimp())
          linear(home, IntArgs{invalid, underflow | inexact},
                 BoolVarArgs{class_indicator(home, source, FWC_SIGNALING_NAN),
                             flushed},
                 IRT_EQ, status);
      }
    } // namespace
  } // namespace Word

  namespace {
    // Assign the result and status of the conversion of an assigned source,
    // or fail when they exclude them.
    void decide_conversion(Home home, const FloatWordPolicy& policy,
                           WordVar source, WordVar result, IntVar status) {
      const Word::Conversion conversion = Word::convert(
          source.val(), Word::Float::format_facts(source.domain_type()),
          Word::Float::format_facts(result.domain_type()), policy);
      if (status.varimp())
        GECODE_ME_FAIL(Int::IntView(status).eq(home, conversion.flags));
      GECODE_ME_FAIL(Word::WordView(result).eq(home, conversion.bits));
    }

    // Check both formats before posting, and post the conversion.
    void post_convert(Home home, const FloatWordPolicy& policy, WordVar source,
                      WordVar result, IntVar status) {
      const char* name = "Word::fp_convert";
      const WordDomainType from = Word::Float::float_format(source, name);
      const WordDomainType to = Word::Float::float_format(result, name);
      Word::Float::check(policy, name);
      GECODE_POST;
      if (status.varimp()) {
        Int::IntView flags(status);
        GECODE_ME_FAIL(flags.gq(home, 0));
        GECODE_ME_FAIL(flags.lq(home, 31));
      }
      if (source.assigned())
        decide_conversion(home, policy, source, result, status);
      else if (from == WDT_FP32 && to == WDT_FP32)
        Word::post_self_conversion(home, policy, source, result, status);
      else
        Word::MixedConversion::post(home, policy, source, result, status);
    }
  } // namespace

  void fp_convert(Home home, WordVar source, WordVar result,
                  const FloatWordPolicy& policy) {
    post_convert(home, policy, source, result, IntVar());
  }
  void fp_convert(Home home, WordVar source, WordVar result, IntVar status,
                  const FloatWordPolicy& policy) {
    post_convert(home, policy, source, result, status);
  }
} // namespace Gecode

// STATISTICS: word-post
