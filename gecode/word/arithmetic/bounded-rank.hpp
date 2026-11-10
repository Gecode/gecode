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
 */

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_RANK_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_RANK_HPP

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Checked rank arithmetic; value is meaningful only when is_valid is true.
  struct BoundRankResult {
    bool is_valid;
    WordValue value;
  };

  /// Optional interval deduction; absence skips narrowing rather than failing.
  struct BoundIntervalResult {
    bool has_interval;
    WordRankInterval interval;
  };

  /// Add signed values represented as ranks, failing on signed overflow.
  forceinline BoundRankResult
  add_signed_ranks(WordValue x, WordValue y, WordValue sign, WordValue mask) {
    if (y >= sign) {
      const WordValue delta=y-sign;
      if (x > mask-delta) return BoundRankResult{false,0};
      return BoundRankResult{true,x+delta};
    }
    const WordValue delta=sign-y;
    if (x < delta) return BoundRankResult{false,0};
    return BoundRankResult{true,x-delta};
  }

  /// Subtract signed ranks, failing on signed overflow.
  forceinline BoundRankResult
  subtract_signed_ranks(WordValue x, WordValue y,
                        WordValue sign, WordValue mask) {
    if (y >= sign) {
      const WordValue delta=y-sign;
      if (x < delta) return BoundRankResult{false,0};
      return BoundRankResult{true,x-delta};
    }
    const WordValue delta=sign-y;
    if (x > mask-delta) return BoundRankResult{false,0};
    return BoundRankResult{true,x+delta};
  }

  /// Multiply signed ranks, failing if the magnitude exceeds the signed range.
  forceinline BoundRankResult
  multiply_signed_ranks(WordValue x, WordValue y,
                        WordValue sign, WordValue mask) {
    const bool is_x_negative=x < sign, is_y_negative=y < sign;
    const WordValue x_magnitude=is_x_negative ? sign-x : x-sign;
    const WordValue y_magnitude=is_y_negative ? sign-y : y-sign;
    const bool is_negative=is_x_negative != is_y_negative;
    const WordValue limit=is_negative ? sign : sign-1;
    const bool has_overflow=(x_magnitude != 0) && (y_magnitude > limit/x_magnitude);
    if (has_overflow) return BoundRankResult{false,0};
    const WordValue magnitude=x_magnitude*y_magnitude;
    const WordValue value=is_negative ? sign-magnitude : sign+magnitude;
    return BoundRankResult{value <= mask,value};
  }

}}}

#endif
