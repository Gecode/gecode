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

#ifndef __GECODE_WORD_ARITHMETIC_GCD_FILTER_HPP__
#define __GECODE_WORD_ARITHMETIC_GCD_FILTER_HPP__

#include <gecode/word/arithmetic/number-domain.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  template<bool is_signed>
  Int::RelTest
  test_gcd(WordView x, WordView y, WordView result) {
    const bool has_impossible_zero=result.assigned() && (result.val() == 0U) &&
      (!x.in(0U) || !y.in(0U));
    if (has_impossible_zero)
      return Int::RT_FALSE;
    const bool is_x_unit=x.assigned() &&
      (compute_magnitude(x.val(),x.width(),is_signed) == 1U);
    if (is_x_unit) {
      if (!result.in(1U)) return Int::RT_FALSE;
      if (result.assigned()) return Int::RT_TRUE;
    }
    const bool is_y_unit=y.assigned() &&
      (compute_magnitude(y.val(),y.width(),is_signed) == 1U);
    if (is_y_unit) {
      if (!result.in(1U)) return Int::RT_FALSE;
      if (result.assigned()) return Int::RT_TRUE;
    }
    const bool are_operands_assigned=x.assigned() && y.assigned();
    if (are_operands_assigned) {
      const WordValue gcd=compute_word_gcd<is_signed>(x.val(),y.val(),x.width());
      if (!result.in(gcd)) return Int::RT_FALSE;
      return result.assigned() ? Int::RT_TRUE : Int::RT_MAYBE;
    }
    return Int::RT_MAYBE;
  }

  forceinline bool
  restrict_gcd_cube_result(Home home, WordView x, WordView y, WordView result) {
    WordValue upper=std::max(x.hi(),y.hi());
    if (!x.in(0U)) upper=std::min(upper,x.hi());
    if (!y.in(0U)) upper=std::min(upper,y.hi());
    return !me_failed(result.narrow(
      home,result.lo(),result.hi()&compute_interval_hull(upper)));
  }

  template<bool is_signed>
  NumberFilterResult
  restrict_gcd_cube_operands(Home home, WordView x, WordView y, WordValue gcd) {
    if (gcd == 0U) {
      if (me_failed(x.eq(home,0U))) return NumberFilterResult::failed;
      if (me_failed(y.eq(home,0U))) return NumberFilterResult::failed;
      return NumberFilterResult::entailed;
    }
    const WordValue low=compute_trailing_zero_mask(gcd);
    if (low != 0U) {
      if (me_failed(x.narrow(home,x.lo(),x.hi()&~low)))
        return NumberFilterResult::failed;
      if (me_failed(y.narrow(home,y.lo(),y.hi()&~low)))
        return NumberFilterResult::failed;
    }
    const bool has_incompatible_operand=
      (x.assigned() &&
       ((compute_magnitude(x.val(),x.width(),is_signed)%gcd) != 0U)) ||
      (y.assigned() &&
       ((compute_magnitude(y.val(),y.width(),is_signed)%gcd) != 0U));
    return has_incompatible_operand ? NumberFilterResult::failed :
      NumberFilterResult::active;
  }

  /// Apply one pass of the cube GCD rules before checking its fixpoint.
  template<bool is_signed>
  NumberFilterResult
  filter_gcd_cube_pass(Home home, WordView x, WordView y, WordView result) {
    if (!is_signed) {
      if (!restrict_gcd_cube_result(home,x,y,result))
        return NumberFilterResult::failed;
    }
    const bool has_unit_operand=
      (x.assigned() && (compute_magnitude(x.val(),x.width(),is_signed) == 1U)) ||
      (y.assigned() && (compute_magnitude(y.val(),y.width(),is_signed) == 1U));
    if (has_unit_operand)
      return me_failed(result.eq(home,1U)) ? NumberFilterResult::failed :
        NumberFilterResult::entailed;
    if (result.assigned()) {
      const NumberFilterResult filtered=restrict_gcd_cube_operands<is_signed>(
        home,x,y,result.val());
      if (filtered != NumberFilterResult::active) return filtered;
    }
    const bool are_operands_assigned=x.assigned() && y.assigned();
    if (are_operands_assigned) {
      const WordValue gcd=compute_word_gcd<is_signed>(x.val(),y.val(),x.width());
      return me_failed(result.eq(home,gcd)) ? NumberFilterResult::failed :
        NumberFilterResult::entailed;
    }
    return NumberFilterResult::active;
  }

  /// Close GCD deductions on the views used by the cube fallback.
  template<bool is_signed>
  NumberFilterResult
  filter_gcd_cube(Home home, WordView x, WordView y, WordView result) {
    for (;;) {
      const WordValue old[6]={x.lo(),x.hi(),y.lo(),y.hi(),result.lo(),result.hi()};
      const NumberFilterResult filtered=filter_gcd_cube_pass<is_signed>(
        home,x,y,result);
      if (filtered != NumberFilterResult::active) return filtered;
      const WordValue now[6]={x.lo(),x.hi(),y.lo(),y.hi(),result.lo(),result.hi()};
      bool has_changed=false;
      for (unsigned int i=0; i<6; i++) has_changed |= old[i] != now[i];
      if (!has_changed) return filtered;
    }
  }

  /// Restrict a positive fixed operand's possible GCD divisors.
  forceinline bool
  restrict_gcd_divisor(BoundLocalDomain& result, WordValue magnitude) {
    if (magnitude == 0U) return true;
    if (!result.intersect_range(1U,magnitude)) return false;
    const WordValue quotient=magnitude/result.maximum;
    if (quotient != magnitude/result.minimum) return true;
    return ((magnitude%quotient) == 0U) &&
      assign_value(result,magnitude/quotient);
  }

  forceinline bool
  restrict_gcd_result_bounds(const BoundLocalDomain& x,
                             const BoundLocalDomain& y,
                             BoundLocalDomain& result) {
    WordValue upper=std::max(compute_maximum_magnitude(x),
                             compute_maximum_magnitude(y));
    if (!has_value(x,0U))
      upper=std::min(upper,compute_maximum_magnitude(x));
    if (!has_value(y,0U))
      upper=std::min(upper,compute_maximum_magnitude(y));
    const WordValue lower=(!has_value(x,0U) || !has_value(y,0U)) ?
      1U : 0U;
    return result.intersect_range(lower,upper);
  }

  /// Apply an assigned result to both operands, in role order.
  template<bool is_signed>
  NumberFilterResult
  restrict_gcd_operands(BoundLocalDomain& x, BoundLocalDomain& y,
                        WordValue gcd) {
    if (gcd == 0U)
      return (assign_value(x,0U) && assign_value(y,0U)) ?
        NumberFilterResult::entailed : NumberFilterResult::failed;
    const WordValue low=compute_trailing_zero_mask(gcd);
    const bool can_continue=clear_bits(x,low) && clear_bits(y,low) &&
      restrict_multiples<is_signed>(x,gcd) && restrict_multiples<is_signed>(y,gcd);
    if (!can_continue) return NumberFilterResult::failed;
    return NumberFilterResult::active;
  }

  /// Apply GCD deductions to local domains before synchronization.
  template<bool is_signed>
  NumberFilterResult
  filter_gcd_local(BoundLocalDomain& x, BoundLocalDomain& y,
                    BoundLocalDomain& result) {
    const bool is_unsigned_alias=!is_signed && (&x == &y);
    if (is_unsigned_alias) {
      if (!intersect_equal_domains(x,result)) return NumberFilterResult::failed;
      return ((&x == &result) || (is_assigned(x) && is_assigned(result))) ?
        NumberFilterResult::entailed : NumberFilterResult::active;
    }
    const bool has_unit_operand=
      (is_assigned(x) && (compute_magnitude(x.lo,x.width,is_signed) == 1U)) ||
      (is_assigned(y) && (compute_magnitude(y.lo,y.width,is_signed) == 1U));
    if (has_unit_operand)
      return assign_value(result,1U) ? NumberFilterResult::entailed :
        NumberFilterResult::failed;
    if (!restrict_gcd_result_bounds(x,y,result)) return NumberFilterResult::failed;

    // A tell to an aliased result can also assign an operand, including zero.
    if (is_assigned(x)) {
      if (!restrict_gcd_divisor(result,compute_magnitude(x.lo,x.width,is_signed)))
        return NumberFilterResult::failed;
    }
    if (is_assigned(y)) {
      if (!restrict_gcd_divisor(result,compute_magnitude(y.lo,y.width,is_signed)))
        return NumberFilterResult::failed;
    }
    if (is_assigned(result)) {
      const NumberFilterResult filtered=restrict_gcd_operands<is_signed>(
        x,y,result.lo);
      if (filtered != NumberFilterResult::active) return filtered;
    }
    const bool are_operands_assigned=is_assigned(x) && is_assigned(y);
    if (are_operands_assigned)
      return assign_value(result,compute_word_gcd<is_signed>(x.lo,y.lo,x.width)) ?
        NumberFilterResult::entailed : NumberFilterResult::failed;
    const bool is_signed_alias=is_signed && (&x == &y);
    if (is_signed_alias) {
      if (!result.intersect_range(compute_minimum_magnitude(x),
                                  compute_maximum_magnitude(x)))
        return NumberFilterResult::failed;
    }
    return NumberFilterResult::active;
  }

  /// Close mandatory GCD rules with each aliased role synchronized once.
  template<bool is_signed>
  NumberFilterResult
  close_gcd_mandatory(BoundLocalDomain (&domains)[3],
                      const unsigned int (&alias)[3]) {
    for (;;) {
      const BoundLocalDomain old[3]={domains[0],domains[1],domains[2]};
      for (unsigned int i=0; i<3; i++) domains[i].deferred=true;
      const NumberFilterResult filtered=filter_gcd_local<is_signed>(
        domains[alias[0]],domains[alias[1]],domains[alias[2]]);
      if (filtered == NumberFilterResult::failed) return filtered;
      for (unsigned int i=0; i<3; i++) {
        domains[i].deferred=false;
        if (alias[i] != i) continue;
        if (!domains[i].synchronize()) return NumberFilterResult::failed;
      }
      const bool is_fixpoint=(domains[0] == old[0]) && (domains[1] == old[1]) &&
        (domains[2] == old[2]);
      if (is_fixpoint) return filtered;
    }
  }

  /// Optional GCD work can run out of budget without proving failure.
  enum class GcdWorkStatus { complete, failed, exhausted };

  /// Quotient interval and traversal direction within one sign segment.
  struct GcdQuotientRange {
    WordValue minimum;
    WordValue maximum;
    bool is_increasing;
  };

  /// An absent rank segment has no range to probe; present ranges may be empty.
  struct GcdSegmentQuotients {
    bool has_segment;
    GcdQuotientRange range;
  };

  /// The quotient is valid only when the search completed.
  struct GcdQuotientResult {
    GcdWorkStatus status;
    WordValue quotient;
  };

  /// The ranked endpoint is valid only when the search completed.
  struct GcdEndpointResult {
    GcdWorkStatus status;
    WordValue rank;
  };

  /// The filter result is accepted only when the trial completed.
  struct GcdTrialResult {
    GcdWorkStatus status;
    NumberFilterResult filtered;
  };

  forceinline bool
  spend_gcd_work(unsigned int& remaining) {
    if (remaining == 0U) return false;
    remaining--;
    return true;
  }

  forceinline GcdQuotientRange
  compute_gcd_quotient_range(WordValue minimum, WordValue maximum,
                             WordValue gcd, bool is_increasing) {
    return {minimum/gcd + ((minimum%gcd) != 0U), maximum/gcd, is_increasing};
  }

  /// Probe arithmetic quotients, consuming one work unit per coprimality test.
  inline GcdQuotientResult
  probe_coprime_quotient(const GcdQuotientRange& range, WordValue fixed_quotient,
                        unsigned int& remaining) {
    if (range.minimum > range.maximum) return {GcdWorkStatus::failed,0U};
    WordValue quotient=range.is_increasing ? range.minimum : range.maximum;
    const WordValue last=range.is_increasing ? range.maximum : range.minimum;
    for (;;) {
      if (!spend_gcd_work(remaining)) return {GcdWorkStatus::exhausted,0U};
      if (compute_gcd(fixed_quotient,quotient) == 1U)
        return {GcdWorkStatus::complete,quotient};
      if (quotient == last) return {GcdWorkStatus::failed,0U};
      if (range.is_increasing) quotient++; else quotient--;
    }
  }

  forceinline GcdSegmentQuotients
  prepare_gcd_quotient_range(const BoundLocalDomain& domain, WordValue gcd,
                             bool is_lower, bool is_negative) {
    const WordValue sign=(domain.kind == WDT_SIGNED) ?
      sign_bit(domain.width) : 0U;
    const WordValue first=is_negative ? domain.minimum :
      std::max(domain.minimum,sign);
    const WordValue last=is_negative ? std::min(domain.maximum,sign-1U) :
      domain.maximum;
    if (first > last) return {false,{0U,0U,false}};
    const WordValue minimum=compute_rank_magnitude(domain,
      is_negative ? last : first);
    const WordValue maximum=compute_rank_magnitude(domain,
      is_negative ? first : last);
    return {true,compute_gcd_quotient_range(
      minimum,maximum,gcd,is_lower != is_negative)};
  }

  /// Probe ranked arithmetic multiples under the shared remaining budget.
  inline GcdEndpointResult
  probe_gcd_endpoint(const BoundLocalDomain& domain, WordValue magnitude,
                      WordValue gcd, bool is_lower, unsigned int& remaining) {
    const WordValue sign=(domain.kind == WDT_SIGNED) ?
      sign_bit(domain.width) : 0U;
    for (unsigned int i=0; i<(sign ? 2U : 1U); i++) {
      const bool is_negative=sign && (is_lower ? i == 0U : i == 1U);
      const GcdSegmentQuotients segment=prepare_gcd_quotient_range(
        domain,gcd,is_lower,is_negative);
      if (!segment.has_segment) continue;
      const GcdQuotientResult found=probe_coprime_quotient(
        segment.range,magnitude/gcd,remaining);
      if (found.status == GcdWorkStatus::exhausted) return {found.status,0U};
      if (found.status == GcdWorkStatus::failed) continue;
      // The quotient limits prove that this product is representable.
      const WordValue candidate_magnitude=gcd*found.quotient;
      const WordValue rank=is_negative ?
        compute_negative_rank(candidate_magnitude,domain.width) :
        Word::rank(domain.kind,domain.width,candidate_magnitude);
      return {GcdWorkStatus::complete,rank};
    }
    return {GcdWorkStatus::failed,0U};
  }

  inline GcdWorkStatus
  restrict_gcd_coprime(BoundLocalDomain& domain, WordValue magnitude,
                       WordValue gcd, unsigned int& remaining) {
    if ((magnitude%gcd) != 0U) return GcdWorkStatus::failed;
    if (magnitude == gcd) return GcdWorkStatus::complete;
    for (bool is_lower : {true,false}) {
      const GcdEndpointResult endpoint=probe_gcd_endpoint(
        domain,magnitude,gcd,is_lower,remaining);
      if (endpoint.status != GcdWorkStatus::complete) return endpoint.status;
      // A cube hole declines the tell. It must not start a synchronized walk.
      const WordValue value=Word::rank(domain.kind,domain.width,endpoint.rank);
      if (has_value(domain,value)) {
        if (is_lower) domain.minimum=endpoint.rank;
        else domain.maximum=endpoint.rank;
      }
    }
    return GcdWorkStatus::complete;
  }

  template<bool is_signed>
  GcdWorkStatus
  restrict_gcd_assigned(BoundLocalDomain& x, BoundLocalDomain& y,
                        BoundLocalDomain& result, unsigned int& remaining) {
    const bool is_positive_result=is_assigned(result) && (result.lo != 0U);
    if (!is_positive_result) return GcdWorkStatus::complete;
    if (is_assigned(x)) {
      const GcdWorkStatus status=restrict_gcd_coprime(
        y,compute_magnitude(x.lo,x.width,is_signed),result.lo,remaining);
      if (status != GcdWorkStatus::complete) return status;
    }
    if (is_assigned(y))
      return restrict_gcd_coprime(
        x,compute_magnitude(y.lo,y.width,is_signed),result.lo,remaining);
    return GcdWorkStatus::complete;
  }

  /// Close all optional consequences under one transaction-wide budget.
  template<bool is_signed>
  GcdTrialResult
  close_gcd_optional(BoundLocalDomain (&domains)[3],
                     const unsigned int (&alias)[3]) {
    const unsigned int limit=64U;
    unsigned int remaining=limit;
    // Bind roles into this trial array, never into the mandatory baseline.
    BoundLocalDomain* role[3]={&domains[alias[0]],&domains[alias[1]],
                               &domains[alias[2]]};
    for (;;) {
      if (!spend_gcd_work(remaining))
        return {GcdWorkStatus::exhausted,NumberFilterResult::active};
      const BoundLocalDomain old[3]={domains[0],domains[1],domains[2]};
      for (unsigned int i=0; i<3; i++) domains[i].deferred=true;
      const NumberFilterResult filtered=filter_gcd_local<is_signed>(
        *role[0],*role[1],*role[2]);
      if (filtered == NumberFilterResult::failed)
        return {GcdWorkStatus::failed,filtered};
      const GcdWorkStatus status=restrict_gcd_assigned<is_signed>(
        *role[0],*role[1],*role[2],remaining);
      if (status != GcdWorkStatus::complete) return {status,filtered};
      for (unsigned int i=0; i<3; i++) {
        domains[i].deferred=false;
        if (alias[i] != i) continue;
        if (!spend_gcd_work(remaining)) return {GcdWorkStatus::exhausted,filtered};
        if (!domains[i].synchronize()) return {GcdWorkStatus::failed,filtered};
      }
      const bool is_fixpoint=(domains[0] == old[0]) && (domains[1] == old[1]) &&
        (domains[2] == old[2]);
      if (is_fixpoint) return {GcdWorkStatus::complete,filtered};
    }
  }

  /// Publish a completed optional trial into the mandatory local domains.
  template<bool is_signed>
  NumberFilterResult
  try_gcd_optional(BoundLocalDomain (&domains)[3],
                    const unsigned int (&alias)[3], NumberFilterResult mandatory) {
    const bool can_probe=(mandatory != NumberFilterResult::entailed) &&
      is_assigned(domains[alias[2]]) &&
      (is_assigned(domains[alias[0]]) || is_assigned(domains[alias[1]]));
    if (!can_probe) return mandatory;
    BoundLocalDomain trial[3]={domains[0],domains[1],domains[2]};
    const GcdTrialResult result=close_gcd_optional<is_signed>(trial,alias);
    if (result.status == GcdWorkStatus::failed) return NumberFilterResult::failed;
    // Exhaustion discards both domain changes and the trial's entailment.
    if (result.status == GcdWorkStatus::exhausted) return mandatory;
    // Acceptance includes a synchronized no-change pass. Rescheduling cannot
    // continue a partially published endpoint walk and needs no saved budget.
    for (unsigned int i=0; i<3; i++) domains[i]=trial[i];
    return result.filtered;
  }

}}}

#endif

// STATISTICS: word-prop
