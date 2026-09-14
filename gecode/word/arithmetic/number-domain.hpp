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

#ifndef __GECODE_WORD_ARITHMETIC_NUMBER_DOMAIN_HPP__
#define __GECODE_WORD_ARITHMETIC_NUMBER_DOMAIN_HPP__

#include <gecode/word/arithmetic/number-value.hpp>
#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-progression.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Failure, a remaining relation, or entailment after number filtering.
  enum class NumberFilterResult { failed, active, entailed };

  forceinline bool
  has_value(const BoundLocalDomain& domain, WordValue value) {
    return cube_contains(domain.lo,domain.hi,value,width_mask(domain.width)) &&
      (Word::rank(domain.kind,domain.width,value) >= domain.minimum) &&
      (Word::rank(domain.kind,domain.width,value) <= domain.maximum);
  }

  forceinline bool
  is_assigned(const BoundLocalDomain& domain) {
    return (domain.lo == domain.hi) && (domain.minimum == domain.maximum);
  }

  forceinline bool
  assign_value(BoundLocalDomain& domain, WordValue value) {
    const WordValue rank=Word::rank(domain.kind,domain.width,value);
    domain.lo |= value;
    domain.hi &= value;
    domain.minimum=std::max(domain.minimum,rank);
    domain.maximum=std::min(domain.maximum,rank);
    return ((domain.lo & ~domain.hi) == 0U) && (domain.minimum <= domain.maximum);
  }

  forceinline bool
  clear_bits(BoundLocalDomain& domain, WordValue bits) {
    domain.hi &= ~bits;
    return (domain.lo & ~domain.hi) == 0U;
  }

  forceinline WordValue
  compute_rank_magnitude(const BoundLocalDomain& domain, WordValue rank) {
    return compute_magnitude(Word::rank(domain.kind,domain.width,rank),domain.width,
                            domain.kind == WDT_SIGNED);
  }

  forceinline WordValue
  compute_maximum_magnitude(const BoundLocalDomain& domain) {
    return std::max(compute_rank_magnitude(domain,domain.minimum),
                    compute_rank_magnitude(domain,domain.maximum));
  }

  forceinline WordValue
  compute_minimum_magnitude(const BoundLocalDomain& domain) {
    if (domain.kind != WDT_SIGNED)
      return compute_rank_magnitude(domain,domain.minimum);
    if (has_value(domain,0U)) return 0U;
    const WordValue mask=width_mask(domain.width);
    const WordValue sign=sign_bit(domain.width);
    const WordCube ordered =
      order_cube(domain.kind,domain.width,{domain.lo,domain.hi});
    WordValue minimum=mask;
    if (domain.minimum < sign) {
      WordValue candidate;
      const WordValue bound=std::min(domain.maximum,sign-1U);
      const bool has_negative_candidate=
        cube_predecessor(ordered.lo,ordered.hi,bound,mask,candidate) &&
        (candidate >= domain.minimum);
      if (has_negative_candidate)
        minimum=compute_rank_magnitude(domain,candidate);
    }
    if (domain.maximum >= sign) {
      WordValue candidate;
      const WordValue bound=std::max(domain.minimum,sign);
      const bool has_nonnegative_candidate=
        cube_successor(ordered.lo,ordered.hi,bound,mask,candidate) &&
        (candidate <= domain.maximum);
      if (has_nonnegative_candidate)
        minimum=std::min(minimum,compute_rank_magnitude(domain,candidate));
    }
    return minimum;
  }

  forceinline bool
  restrict_unsigned_multiples(BoundLocalDomain& domain, WordValue divisor) {
    return bound_progression(domain,0U,divisor);
  }

  forceinline WordValue
  compute_negative_rank(WordValue magnitude, unsigned int width) {
    const WordValue encoded=(~magnitude+1U)&width_mask(width);
    return Word::rank(WDT_SIGNED,width,encoded);
  }

  forceinline bool
  restrict_signed_multiples(BoundLocalDomain& domain, WordValue divisor) {
    return bound_progression(
      domain,sign_bit(domain.width)%divisor,divisor);
  }

  template<bool is_signed>
  forceinline bool
  restrict_multiples(BoundLocalDomain& domain, WordValue divisor) {
    return is_signed ? restrict_signed_multiples(domain,divisor) :
      restrict_unsigned_multiples(domain,divisor);
  }

  forceinline bool
  intersect_equal_domains(BoundLocalDomain& x, BoundLocalDomain& y) {
    const WordValue lo=x.lo|y.lo, hi=x.hi&y.hi;
    const WordValue minimum=std::max(x.minimum,y.minimum);
    const WordValue maximum=std::min(x.maximum,y.maximum);
    x.lo=y.lo=lo;
    x.hi=y.hi=hi;
    x.minimum=y.minimum=minimum;
    x.maximum=y.maximum=maximum;
    return ((lo & ~hi) == 0U) && (minimum <= maximum);
  }

}}}

#endif

// STATISTICS: word-prop
