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
#include <cassert>
#include <cmath>
#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float.hpp>
#include <gecode/word/float/sqrt.hpp>

namespace Gecode {
  namespace {
    // Return the common format of source and result after checking the
    // formats and then the policy.
    WordDomainType validate(const Policy& p, WordVar source, WordVar result) {
      const char* name = "Word::fp_sqrt";
      const WordDomainType format = Word::Float::float_format(source, name);
      const WordDomainType result_format =
          Word::Float::float_format(result, name);
      if (result_format != format)
        throw Word::FormatMismatch(name);
      if (format == WDT_BINARY16)
        throw Word::OutOfLimits(name);
      Word::Float::check(p, name);
      return format;
    }

    void post_sqrt(Home home, const Policy& policy, WordVar source,
                   WordVar result, IntVar* status) {
      const Format format =
          Word::Float::format_facts(validate(policy, source, result));
      GECODE_POST;
      if (status != nullptr) {
        Int::IntView flags(*status);
        GECODE_ME_FAIL(flags.gq(home, 0));
        GECODE_ME_FAIL(flags.lq(home, 31));
      }
      if (source.assigned()) {
        const Evaluation e = evaluate(format, policy, source.val());
        if (status != nullptr)
          GECODE_ME_FAIL(Int::IntView(*status).eq(home, e.flags));
        GECODE_ME_FAIL(WordView(result).eq(home, e.result));
        return;
      }
      if (source.width() <= 8) {
        if (status != nullptr)
          GECODE_ES_FAIL(
              (Word::Float::TinyUnary<TinySqrt, Int::IntView>::post(
                  home, source, result, *status, TinySqrt{format, policy})));
        else
          GECODE_ES_FAIL(
              (Word::Float::TinyUnary<TinySqrtResult, Int::ConstIntView>::post(
                  home, source, result, Int::ConstIntView(0),
                  TinySqrtResult{TinySqrt{format, policy}})));
        return;
      }
      if (status != nullptr)
        (void)new (home) Sqrt<Int::IntView>(home, format, policy, source,
                                            result, *status);
      else
        (void)new (home) Sqrt<Word::Float::OmittedStatus>(
            home, format, policy, source, result,
            Word::Float::OmittedStatus());
    }
  } // namespace

  void fp_sqrt(Home home, WordVar source, WordVar result,
               const FloatWordPolicy& policy) {
    post_sqrt(home, policy, source, result, nullptr);
  }
  void fp_sqrt(Home home, WordVar source, WordVar result, IntVar status,
               const FloatWordPolicy& policy) {
    post_sqrt(home, policy, source, result, &status);
  }
} // namespace Gecode

// STATISTICS: word-post
