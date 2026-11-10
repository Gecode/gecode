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

#ifndef __GECODE_WORD_ARITHMETIC_NUMBER_VALUE_HPP__
#define __GECODE_WORD_ARITHMETIC_NUMBER_VALUE_HPP__

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline WordValue
  compute_magnitude(WordValue value, unsigned int width, bool is_signed) {
    const bool is_nonnegative=!is_signed || ((value & sign_bit(width)) == 0U);
    if (is_nonnegative)
      return value;
    return (~value+1U) & width_mask(width);
  }

  forceinline WordValue
  compute_gcd(WordValue x, WordValue y) {
    while (y != 0U) {
      const WordValue remainder=x%y;
      x=y;
      y=remainder;
    }
    return x;
  }

  template<bool is_signed>
  forceinline WordValue
  compute_word_gcd(WordValue x, WordValue y, unsigned int width) {
    return compute_gcd(compute_magnitude(x,width,is_signed),
                       compute_magnitude(y,width,is_signed));
  }

  template<bool is_signed>
  forceinline bool
  is_divisible(WordValue divisor, WordValue dividend, unsigned int width) {
    const WordValue divisor_magnitude=compute_magnitude(divisor,width,is_signed);
    const WordValue dividend_magnitude=compute_magnitude(dividend,width,is_signed);
    return (divisor_magnitude == 0U) ? (dividend_magnitude == 0U) :
      ((dividend_magnitude%divisor_magnitude) == 0U);
  }

  forceinline WordValue
  compute_trailing_zero_mask(WordValue value) {
    if (value == 0U)
      return 0U;
    WordValue mask=0U;
    while ((value & 1U) == 0U) {
      mask=(mask << 1) | 1U;
      value >>= 1;
    }
    return mask;
  }

  forceinline WordValue
  compute_interval_hull(WordValue maximum) {
    return maximum | low_through_highest(maximum);
  }

}}}

#endif

// STATISTICS: word-prop
