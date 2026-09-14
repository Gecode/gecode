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

#ifndef __GECODE_WORD_ARITHMETIC_DIVIDES_FILTER_HPP__
#define __GECODE_WORD_ARITHMETIC_DIVIDES_FILTER_HPP__

#include <gecode/word/arithmetic/number-domain.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  template<bool is_signed>
  Int::RelTest
  test_divides(WordView divisor, WordView dividend) {
    if (divisor == dividend) return Int::RT_TRUE;
    const bool is_zero_dividend=dividend.assigned() && (dividend.val() == 0U);
    if (is_zero_dividend) return Int::RT_TRUE;
    if (divisor.assigned()) {
      const WordValue magnitude=compute_magnitude(
        divisor.val(),divisor.width(),is_signed);
      if (magnitude == 1U) return Int::RT_TRUE;
      const bool is_impossible_zero=(magnitude == 0U) && !dividend.in(0U);
      if (is_impossible_zero) return Int::RT_FALSE;
      if (dividend.assigned())
        return is_divisible<is_signed>(divisor.val(),dividend.val(),
                                    divisor.width()) ?
          Int::RT_TRUE : Int::RT_FALSE;
    }
    return Int::RT_MAYBE;
  }

  /// Apply divisibility deductions to the cube fallback views.
  template<bool is_signed>
  NumberFilterResult
  filter_divides_cube(Home home, WordView divisor, WordView dividend) {
    if (divisor == dividend) return NumberFilterResult::entailed;
    const bool is_zero_dividend=dividend.assigned() && (dividend.val() == 0U);
    if (is_zero_dividend) return NumberFilterResult::entailed;
    if (divisor.assigned()) {
      const WordValue magnitude=compute_magnitude(
        divisor.val(),divisor.width(),is_signed);
      if (magnitude == 0U) {
        if (me_failed(dividend.eq(home,0U)))
          return NumberFilterResult::failed;
        return NumberFilterResult::entailed;
      }
      if (magnitude == 1U) return NumberFilterResult::entailed;
      const WordValue low=compute_trailing_zero_mask(magnitude);
      if (me_failed(dividend.narrow(
        home,dividend.lo(),dividend.hi()&~low)))
        return NumberFilterResult::failed;
    }
    const bool are_operands_assigned=divisor.assigned() && dividend.assigned();
    if (are_operands_assigned) {
      if (!is_divisible<is_signed>(divisor.val(),dividend.val(),
                                divisor.width()))
        return NumberFilterResult::failed;
      return NumberFilterResult::entailed;
    }
    return NumberFilterResult::active;
  }

  template<bool is_signed>
  NumberFilterResult
  restrict_dividend(BoundLocalDomain& dividend, WordValue magnitude) {
    if (magnitude == 0U)
      return assign_value(dividend,0U) ? NumberFilterResult::entailed :
        NumberFilterResult::failed;
    if (magnitude == 1U) return NumberFilterResult::entailed;
    const bool can_continue=
      clear_bits(dividend,compute_trailing_zero_mask(magnitude)) &&
      restrict_multiples<is_signed>(dividend,magnitude);
    if (!can_continue) return NumberFilterResult::failed;
    return NumberFilterResult::active;
  }

  template<bool is_signed>
  bool
  restrict_divisor_for_nonzero_dividend(BoundLocalDomain& divisor,
                                        const BoundLocalDomain& dividend) {
    if (!is_signed) {
      if (!divisor.intersect_range(1U,divisor.maximum)) return false;
    } else {
      const WordValue zero=sign_bit(divisor.width);
      if (divisor.minimum == zero) {
        if (!divisor.intersect_range(zero+1U,divisor.maximum)) return false;
      } else if (divisor.maximum == zero) {
        if (!divisor.intersect_range(divisor.minimum,zero-1U)) return false;
      }
    }
    const WordValue magnitude=compute_maximum_magnitude(dividend);
    if (!is_signed)
      return divisor.intersect_range(1U,magnitude);
    if (magnitude < sign_bit(divisor.width))
      return divisor.intersect_range(
        compute_negative_rank(magnitude,divisor.width),
        Word::rank(WDT_SIGNED,divisor.width,magnitude));
    return true;
  }

  /// Apply divisibility deductions to local domains before synchronization.
  template<bool is_signed>
  NumberFilterResult
  filter_divides_local(BoundLocalDomain& divisor, BoundLocalDomain& dividend) {
    if (&divisor == &dividend) return NumberFilterResult::entailed;
    const bool is_zero_dividend=is_assigned(dividend) && (dividend.lo == 0U);
    if (is_zero_dividend) return NumberFilterResult::entailed;
    if (is_assigned(divisor)) {
      const WordValue magnitude=compute_magnitude(
        divisor.lo,divisor.width,is_signed);
      const NumberFilterResult filtered=restrict_dividend<is_signed>(
        dividend,magnitude);
      if (filtered != NumberFilterResult::active) return filtered;
    }
    if (!has_value(dividend,0U)) {
      if (!restrict_divisor_for_nonzero_dividend<is_signed>(divisor,dividend))
        return NumberFilterResult::failed;
    }
    const bool are_operands_assigned=is_assigned(divisor) && is_assigned(dividend);
    if (are_operands_assigned)
      return is_divisible<is_signed>(divisor.lo,dividend.lo,divisor.width) ?
        NumberFilterResult::entailed : NumberFilterResult::failed;
    return NumberFilterResult::active;
  }

  /// Close divisibility rules with each distinct role synchronized once.
  template<bool is_signed>
  NumberFilterResult
  close_divides(BoundLocalDomain (&domains)[2], bool is_aliased) {
    BoundLocalDomain& dividend=domains[is_aliased ? 0 : 1];
    for (;;) {
      const BoundLocalDomain old[2]={domains[0],domains[1]};
      domains[0].deferred=domains[1].deferred=true;
      const NumberFilterResult filtered=filter_divides_local<is_signed>(
        domains[0],dividend);
      if (filtered == NumberFilterResult::failed) return filtered;
      domains[0].deferred=domains[1].deferred=false;
      if (!domains[0].synchronize()) return NumberFilterResult::failed;
      if (!is_aliased) {
        if (!domains[1].synchronize()) return NumberFilterResult::failed;
      }
      const bool is_fixpoint=(domains[0] == old[0]) && (domains[1] == old[1]);
      if (is_fixpoint) return filtered;
    }
  }

}}}

#endif

// STATISTICS: word-prop
