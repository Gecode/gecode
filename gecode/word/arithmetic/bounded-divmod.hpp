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
 */

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_DIVMOD_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_DIVMOD_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-rank.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  enum BoundUnsignedDivModOperation {
    BUD_DIV,
    BUD_MOD
  };

  forceinline WordValue
  bound_sat_add(WordValue x, WordValue y, WordValue mask) {
    return (x > mask-y) ? mask : x+y;
  }

  forceinline WordValue
  bound_sat_mult(WordValue x, WordValue y, WordValue mask) {
    return ((x != 0) && (y > mask/x)) ? mask : x*y;
  }

  forceinline bool
  bound_unsigned_div_ranges(BoundLocalDomain& a, BoundLocalDomain& b,
                            BoundLocalDomain& q) {
    const WordValue mask=width_mask(a.width);
    if (b.maximum == 0)
      return q.intersect_range(mask,mask);
    assert(b.minimum != 0);
    if (!q.intersect_range(a.minimum/b.maximum,a.maximum/b.minimum))
      return false;
    const WordValue amin=bound_sat_mult(q.minimum,b.minimum,mask);
    WordValue amax=bound_sat_mult(q.maximum,b.maximum,mask);
    amax=bound_sat_add(amax,b.maximum-1,mask);
    if (!a.intersect_range(amin,amax)) return false;
    WordValue bmin=b.minimum, bmax=b.maximum;
    if (q.maximum != mask)
      bmin=std::max(bmin,a.minimum/(q.maximum+1)+1);
    if (q.minimum != 0)
      bmax=std::min(bmax,a.maximum/q.minimum);
    return b.intersect_range(bmin,bmax);
  }

  forceinline bool
  bound_unsigned_mod_ranges(BoundLocalDomain& a, BoundLocalDomain& b,
                            BoundLocalDomain& r) {
    if (b.maximum == 0) {
      const WordValue minimum=std::max(a.minimum,r.minimum);
      const WordValue maximum=std::min(a.maximum,r.maximum);
      return a.intersect_range(minimum,maximum) && r.intersect_range(minimum,maximum);
    }
    assert(b.minimum != 0);
    if (!r.intersect_range(0,std::min(a.maximum,b.maximum-1))) return false;
    if (!a.intersect_range(r.minimum,a.maximum)) return false;
    return b.intersect_range(std::max(b.minimum,r.minimum+1),b.maximum);
  }

  /// Narrow self-division before selecting the current divisor regime.
  template<SignedDivModOperation op>
  forceinline ExecStatus
  narrow_signed_identity(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView output(result);
    const WordValue mask=width_mask(a.width);
    const WordValue sign=sign_bit(a.width);
    if (dividend == divisor) {
      const bool is_zero_dividend=(a.minimum == sign) && (a.maximum == sign);
      const WordValue expected=(op == SDO_DIV) ?
        (is_zero_dividend ? mask : 1) : 0;
      if (me_failed(output.narrow(home,expected,expected)) ||
          !result.intersect_range(expected^sign,expected^sign))
        return ES_FAILED;
    }
    return ES_OK;
  }

  forceinline BoundIntervalResult
  project_signed_quotient_interval(WordRankInterval a, WordRankInterval b,
                                   WordValue sign, WordValue mask) {
    const WordValue minus_one_rank=mask^sign;
    const bool wraps_at_minimum=(a.minimum == 0) &&
      (b.minimum <= minus_one_rank) && (b.maximum >= minus_one_rank);
    // MIN/-1 remains legal, but its wrap invalidates endpoint projection.
    if (wraps_at_minimum) return {false,{0,0}};
    const WordValue dividend_endpoints[2]={a.minimum^sign,a.maximum^sign};
    const WordValue divisor_endpoints[2]={b.minimum^sign,b.maximum^sign};
    WordValue minimum=mask, maximum=0;
    for (unsigned int i=0; i<2; i++)
      for (unsigned int j=0; j<2; j++) {
        const WordValue rank=SignedDivModSupport::evaluate<SDO_DIV>(
          dividend_endpoints[i],divisor_endpoints[j],sign,mask)^sign;
        minimum=std::min(minimum,rank);
        maximum=std::max(maximum,rank);
      }
    return {true,{minimum,maximum}};
  }

  forceinline BoundIntervalResult
  project_signed_dividend_interval(WordRankInterval quotient, WordValue divisor,
                                   WordValue sign, WordValue mask) {
    const WordValue divisor_rank=divisor^sign;
    const BoundRankResult minimum_quotient_product=multiply_signed_ranks(
      quotient.minimum,divisor_rank,sign,mask);
    const BoundRankResult maximum_quotient_product=
      minimum_quotient_product.is_valid ? multiply_signed_ranks(
        quotient.maximum,divisor_rank,sign,mask) : BoundRankResult{false,0};
    const bool has_products=minimum_quotient_product.is_valid &&
      maximum_quotient_product.is_valid;
    if (!has_products) return {false,{0,0}};
    WordValue minimum=std::min(minimum_quotient_product.value,
                               maximum_quotient_product.value);
    WordValue maximum=std::max(minimum_quotient_product.value,
                               maximum_quotient_product.value);
    const WordValue magnitude=SignedDivModSupport::compute_magnitude(
      divisor,sign,mask);
    const WordValue delta=sign+(magnitude-1);
    if (minimum <= sign) {
      const BoundRankResult expanded=
        subtract_signed_ranks(minimum,delta,sign,mask);
      minimum=expanded.is_valid ? expanded.value : 0;
    }
    if (maximum >= sign) {
      const BoundRankResult expanded=add_signed_ranks(maximum,delta,sign,mask);
      maximum=expanded.is_valid ? expanded.value : mask;
    }
    return {true,{minimum,maximum}};
  }

  forceinline WordRankInterval
  project_signed_remainder_interval(WordRankInterval divisor,
                                    WordRankInterval source,
                                    WordValue sign, WordValue mask) {
    const WordValue divisor_endpoints[2]={divisor.minimum^sign,
                                         divisor.maximum^sign};
    WordValue magnitude=0;
    for (unsigned int j=0; j<2; j++)
      magnitude=std::max(magnitude,SignedDivModSupport::compute_magnitude(
        divisor_endpoints[j],sign,mask));
    const WordValue limit=magnitude-1;
    if (source.maximum < sign) return {sign-limit,sign};
    if (source.minimum >= sign) return {sign,sign+limit};
    return {sign-limit,sign+limit};
  }

  forceinline BoundRankResult
  project_required_divisor_magnitude(WordRankInterval result, WordValue sign) {
    WordValue required=0;
    if (result.maximum < sign) required=sign-result.maximum;
    else if (result.minimum > sign) required=result.minimum-sign;
    if (required == 0) return {true,0};
    if (required >= sign) return {false,0};
    return {true,required+1};
  }

  /// Apply the quotient projection before reading inverse dividend bounds.
  forceinline ExecStatus
  narrow_signed_quotient_ranges(BoundLocalDomain& a, BoundLocalDomain& b,
                                BoundLocalDomain& result) {
    const WordValue mask=width_mask(a.width), sign=sign_bit(a.width);
    const BoundIntervalResult forward=project_signed_quotient_interval(
      snapshot_bound_interval(a),snapshot_bound_interval(b),sign,mask);
    if (forward.has_interval)
      if (!result.intersect_range(forward.interval.minimum,
                                  forward.interval.maximum)) return ES_FAILED;
    const BoundLocalView divisor(b);
    if (divisor.assigned()) {
      const BoundIntervalResult inverse=project_signed_dividend_interval(
        snapshot_bound_interval(result),divisor.val(),sign,mask);
      if (inverse.has_interval)
        if (!a.intersect_range(inverse.interval.minimum,inverse.interval.maximum))
          return ES_FAILED;
    }
    return ES_OK;
  }

  /// Narrow the remainder/modulo result before requiring a divisor magnitude.
  template<SignedDivModOperation op>
  forceinline ExecStatus
  narrow_signed_remainder_ranges(BoundLocalDomain& a, BoundLocalDomain& b,
                                 BoundLocalDomain& result) {
    const WordValue mask=width_mask(a.width), sign=sign_bit(a.width);
    const BoundLocalDomain& source=(op == SDO_REM) ? a : b;
    const WordRankInterval interval=project_signed_remainder_interval(
      snapshot_bound_interval(b),snapshot_bound_interval(source),sign,mask);
    if (!result.intersect_range(interval.minimum,interval.maximum))
      return ES_FAILED;
    const BoundRankResult required=project_required_divisor_magnitude(
      snapshot_bound_interval(result),sign);
    if (!required.is_valid) return ES_FAILED;
    if (required.value == 0) return ES_OK;
    if (b.maximum < sign) {
      if (!b.intersect_range(b.minimum,std::min(b.maximum,sign-required.value)))
        return ES_FAILED;
    } else if (b.minimum > sign) {
      if (!b.intersect_range(std::max(b.minimum,sign+required.value),b.maximum))
        return ES_FAILED;
    }
    return ES_OK;
  }

  /// Apply total signed division semantics for an exactly zero divisor.
  template<SignedDivModOperation op>
  forceinline ExecStatus
  narrow_signed_zero_divisor(Home home, BoundLocalDomain& a,
                            BoundLocalDomain& result) {
    BoundLocalView dividend(a), output(result);
    const WordValue mask=width_mask(a.width), sign=sign_bit(a.width);
    if (op == SDO_DIV) {
      const WordValue negative_rank=WordValue(1)^sign;
      const WordValue positive_rank=mask^sign;
      WordValue minimum=std::min(negative_rank,positive_rank);
      WordValue maximum=std::max(negative_rank,positive_rank);
      if (a.maximum < sign) minimum=maximum=negative_rank;
      else if (a.minimum >= sign) minimum=maximum=positive_rank;
      if (!result.intersect_range(minimum,maximum)) return ES_FAILED;
      if (minimum == maximum) {
        if (me_failed(output.narrow(home,minimum^sign,minimum^sign)))
          return ES_FAILED;
      } else {
        if (me_failed(output.narrow(home,WordValue(1),mask))) return ES_FAILED;
      }
      return ES_OK;
    }
    const WordValue lo=dividend.lo()|output.lo();
    const WordValue hi=dividend.hi()&output.hi();
    if (me_failed(dividend.narrow(home,lo,hi)) ||
        me_failed(output.narrow(home,lo,hi))) return ES_FAILED;
    const WordValue minimum=std::max(a.minimum,result.minimum);
    const WordValue maximum=std::min(a.maximum,result.maximum);
    if (!a.intersect_range(minimum,maximum) ||
        !result.intersect_range(minimum,maximum)) return ES_FAILED;
    return ES_OK;
  }

  /// Select the current divisor regime after preceding local deductions.
  template<SignedDivModOperation op>
  forceinline ExecStatus
  narrow_signed_divisor(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    const WordValue sign=sign_bit(a.width);
    const bool is_zero_divisor=(b.minimum == sign) && (b.maximum == sign);
    if (is_zero_divisor) return narrow_signed_zero_divisor<op>(home,a,result);
    if (op == SDO_DIV) return narrow_signed_quotient_ranges(a,b,result);
    return narrow_signed_remainder_ranges<op>(a,b,result);
  }

  /// Apply shared-residue filtering before the positive power-of-two rule.
  template<SignedDivModOperation op>
  forceinline ExecStatus
  narrow_signed_residue(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView output(result);
    const WordValue sign=sign_bit(a.width);

    const bool can_apply_common_residue=(op != SDO_DIV) &&
      divisor.assigned() && (divisor.val() != 0U);
    if (can_apply_common_residue)
      GECODE_ES_CHECK(SignedDivModSupport::propagate_common_residue(
        home,dividend,divisor.val(),output));
    const bool can_apply_positive_power_mod=(op == SDO_MOD) &&
      divisor.assigned() && (divisor.val() != 0U) &&
      ((divisor.val()&sign) == 0U) &&
      ((divisor.val()&(divisor.val()-1U)) == 0U);
    if (can_apply_positive_power_mod) {
      ExecStatus es=SignedDivModSupport::propagate_power_of_two_mod(
        home,dividend,divisor.val(),output);
      if (es == ES_FAILED) return ES_FAILED;
    }
    return ES_OK;
  }

  /// Evaluate the selected signed operation when both inputs are assigned.
  template<SignedDivModOperation op>
  forceinline ExecStatus
  narrow_signed_assigned(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView output(result);
    const WordValue mask=width_mask(a.width);
    const WordValue sign=sign_bit(a.width);
    const bool has_assigned_inputs=dividend.assigned() && divisor.assigned();
    if (has_assigned_inputs) {
      const WordValue expected=SignedDivModSupport::evaluate<op>(
        dividend.val(),divisor.val(),sign,mask);
      if (me_failed(output.narrow(home,expected,expected)) ||
          !result.intersect_range(expected^sign,expected^sign))
        return ES_FAILED;
    }
    return ES_OK;
  }

  /// Narrow both self-division results before other interval deductions.
  forceinline ExecStatus
  narrow_unsigned_pair_identity(Home home, BoundLocalDomain& a,
                                BoundLocalDomain& b, BoundLocalDomain& q,
                                BoundLocalDomain& r) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView quotient(q);
    BoundLocalView remainder(r);
    const WordValue mask=width_mask(a.width);
    if (dividend == divisor) {
      const WordValue qv=(a.maximum == 0) ? mask : 1;
      if (me_failed(quotient.narrow(home,qv,qv)) ||
          me_failed(remainder.narrow(home,0,0)) ||
          !q.intersect_range(qv,qv) || !r.intersect_range(0,0))
        return ES_FAILED;
    }
    return ES_OK;
  }

  /// Propagate a fixed power-of-two residue in both directions.
  forceinline ExecStatus
  narrow_unsigned_power_mod(Home home, BoundLocalDomain& a,
                            BoundLocalDomain& b, BoundLocalDomain& result) {
    BoundLocalView dividend(a), divisor(b), output(result);
    if (!divisor.assigned()) return ES_OK;
    const WordValue value=divisor.val();
    const bool is_power_of_two=(value & (value-1)) == 0;
    if (!is_power_of_two) return ES_OK;
    const WordValue low=value-1;
    if (me_failed(output.narrow(home,dividend.lo()&low,dividend.hi()&low)) ||
        me_failed(dividend.narrow(home,dividend.lo()|(output.lo()&low),
          (dividend.hi()&~low)|(output.hi()&low)))) return ES_FAILED;
    return ES_OK;
  }

  /// Apply total zero-divisor semantics to quotient and remainder.
  forceinline ExecStatus
  narrow_unsigned_pair_zero_divisor(Home home, BoundLocalDomain& a,
                                    BoundLocalDomain& q, BoundLocalDomain& r) {
    BoundLocalView dividend(a), quotient(q), remainder(r);
    const WordValue mask=width_mask(a.width);
    if (me_failed(quotient.narrow(home,mask,mask)) ||
        !q.intersect_range(mask,mask)) return ES_FAILED;
    const WordValue lo=dividend.lo()|remainder.lo();
    const WordValue hi=dividend.hi()&remainder.hi();
    if (me_failed(dividend.narrow(home,lo,hi)) ||
        me_failed(remainder.narrow(home,lo,hi))) return ES_FAILED;
    const WordValue minimum=std::max(a.minimum,r.minimum);
    const WordValue maximum=std::min(a.maximum,r.maximum);
    if (!a.intersect_range(minimum,maximum) ||
        !r.intersect_range(minimum,maximum)) return ES_FAILED;
    return ES_OK;
  }

  /// Apply quotient, remainder and inverse bounds in their tell order.
  forceinline ExecStatus
  narrow_unsigned_pair_divisor(Home home, BoundLocalDomain& a,
                                BoundLocalDomain& b, BoundLocalDomain& q,
                                BoundLocalDomain& r) {
    const WordValue mask=width_mask(a.width);
    if (b.maximum == 0) return narrow_unsigned_pair_zero_divisor(home,a,q,r);
    if (!bound_unsigned_div_ranges(a,b,q) ||
        !bound_unsigned_mod_ranges(a,b,r))
      return ES_FAILED;
    GECODE_ES_CHECK(narrow_unsigned_power_mod(home,a,b,r));
    WordValue amin=bound_sat_mult(q.minimum,b.minimum,mask);
    amin=bound_sat_add(amin,r.minimum,mask);
    WordValue amax=bound_sat_mult(q.maximum,b.maximum,mask);
    amax=bound_sat_add(amax,r.maximum,mask);
    if (!a.intersect_range(amin,amax)) return ES_FAILED;
    return ES_OK;
  }

  /// Compute both exact results before telling either aliased output.
  forceinline ExecStatus
  narrow_unsigned_pair_assigned(Home home, BoundLocalDomain& a,
                                BoundLocalDomain& b, BoundLocalDomain& q,
                                BoundLocalDomain& r) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView quotient(q);
    BoundLocalView remainder(r);
    const WordValue mask=width_mask(a.width);
    const bool has_assigned_inputs=dividend.assigned() && divisor.assigned();
    if (has_assigned_inputs) {
      const WordValue qv=(divisor.val()==0) ? mask : dividend.val()/divisor.val();
      const WordValue rv=(divisor.val()==0) ? dividend.val() :
        dividend.val()%divisor.val();
      if (me_failed(quotient.narrow(home,qv,qv)) ||
          me_failed(remainder.narrow(home,rv,rv)) ||
          !q.intersect_range(qv,qv) || !r.intersect_range(rv,rv))
        return ES_FAILED;
    }
    return ES_OK;
  }

  /// Narrow the selected result of unsigned self-division.
  template<BoundUnsignedDivModOperation op>
  forceinline ExecStatus
  narrow_unsigned_identity(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView output(result);
    const WordValue mask=width_mask(a.width);
    if (dividend == divisor) {
      const WordValue expected=(op == BUD_MOD) ? 0 :
        ((a.maximum == 0) ? mask : 1);
      if (me_failed(output.narrow(home,expected,expected)) ||
          !result.intersect_range(expected,expected))
        return ES_FAILED;
    }
    return ES_OK;
  }

  /// Apply total zero-divisor semantics to the selected unsigned result.
  template<BoundUnsignedDivModOperation op>
  forceinline ExecStatus
  narrow_unsigned_zero_divisor(Home home, BoundLocalDomain& a,
                                    BoundLocalDomain& result) {
    BoundLocalView dividend(a), output(result);
    const WordValue mask=width_mask(a.width);
    if (op == BUD_DIV) {
      if (me_failed(output.narrow(home,mask,mask))) return ES_FAILED;
      if (!result.intersect_range(mask,mask)) return ES_FAILED;
    } else {
      const WordValue lo=dividend.lo()|output.lo();
      const WordValue hi=dividend.hi()&output.hi();
      if (me_failed(dividend.narrow(home,lo,hi)) ||
          me_failed(output.narrow(home,lo,hi))) return ES_FAILED;
    }
    return ES_OK;
  }

  /// Narrow the selected numeric relation before power-of-two residue filtering.
  template<BoundUnsignedDivModOperation op>
  forceinline ExecStatus
  narrow_unsigned_divisor(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    if (b.maximum == 0) return narrow_unsigned_zero_divisor<op>(home,a,result);
    assert(b.minimum != 0);
    const bool is_consistent=(op == BUD_DIV) ?
      bound_unsigned_div_ranges(a,b,result) :
      bound_unsigned_mod_ranges(a,b,result);
    if (!is_consistent) return ES_FAILED;
    if (op == BUD_MOD)
      GECODE_ES_CHECK(narrow_unsigned_power_mod(home,a,b,result));

    return ES_OK;
  }

  /// Evaluate the selected unsigned operation for assigned operands.
  template<BoundUnsignedDivModOperation op>
  forceinline ExecStatus
  narrow_unsigned_assigned(Home home, BoundLocalDomain& a, BoundLocalDomain& b,
                        BoundLocalDomain& result) {
    BoundLocalView dividend(a);
    BoundLocalView divisor(b);
    BoundLocalView output(result);
    const WordValue mask=width_mask(a.width);
    const bool has_assigned_inputs=dividend.assigned() && divisor.assigned();
    if (has_assigned_inputs) {
      const WordValue expected=(divisor.val() == 0) ?
        ((op == BUD_DIV) ? mask : dividend.val()) :
        ((op == BUD_DIV) ? dividend.val()/divisor.val() :
                           dividend.val()%divisor.val());
      if (me_failed(output.narrow(home,expected,expected)))
        return ES_FAILED;
      if (!result.intersect_range(expected,expected)) return ES_FAILED;
    }
    return ES_OK;
  }

  /// Selected unsigned division/modulo with a zero or zero-excluding divisor.
  template<BoundUnsignedDivModOperation op>
  class BoundUnsignedDivMod
    : public TernaryPropagator<UnsignedWordView,PC_WORD_DOM> {
  public:
    static bool numeric_regime(UnsignedWordView b) {
      return (b.rank_maximum() == 0) || (b.rank_minimum() != 0);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundUnsignedDivMod(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1,x2));
      const bool is_assigned=x0.assigned() && x1.assigned() && x2.assigned();
      return is_assigned ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView a,
                           UnsignedWordView b, UnsignedWordView result) {
      GECODE_ES_CHECK(narrow(home,a,b,result));
      if (a.varimp() == b.varimp())
        return ES_OK;
      const bool is_assigned=a.assigned() && b.assigned() && result.assigned();
      if (!is_assigned)
        (void) new (home) BoundUnsignedDivMod(home,a,b,result);
      return ES_OK;
    }
  protected:
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x0;
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x1;
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x2;
    BoundUnsignedDivMod(Home home, UnsignedWordView a,
                       UnsignedWordView b, UnsignedWordView r)
      : TernaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,a,b,r) {}
    BoundUnsignedDivMod(Space& home, BoundUnsignedDivMod& p)
      : TernaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,p) {}
    /// Close local deductions and publish only changed representatives.
    static ExecStatus narrow(Home home, UnsignedWordView a,
                             UnsignedWordView b, UnsignedWordView result) {
      const UnsignedWordView input[3]={a,b,result};
      BoundLocalPass<UnsignedWordView,3> pass(input);
      const BoundDomainSnapshot<3> initial=pass.snapshot();
      for (;;) {
        const BoundDomainSnapshot<3> previous=pass.snapshot();
        pass.defer_synchronization();
        GECODE_ES_CHECK(narrow_unsigned_identity<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        GECODE_ES_CHECK(narrow_unsigned_divisor<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        GECODE_ES_CHECK(narrow_unsigned_assigned<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        if (!pass.synchronize_distinct()) return ES_FAILED;
        if (pass.is_unchanged(previous)) break;
      }
      return pass.publish_changed(home,initial);
    }
  };

  /// Combined quotient/remainder with a zero or zero-excluding unsigned divisor.
  class BoundUnsignedDivModBoth : public Propagator {
  public:
    static bool numeric_regime(UnsignedWordView b) {
      return (b.rank_maximum() == 0) || (b.rank_minimum() != 0);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundUnsignedDivModBoth(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,a.width());
    }
    virtual void reschedule(Space& home) {
      a.reschedule(home,*this,PC_WORD_DOM); b.reschedule(home,*this,PC_WORD_DOM);
      q.reschedule(home,*this,PC_WORD_DOM); r.reschedule(home,*this,PC_WORD_DOM);
    }
    virtual size_t dispose(Space& home) {
      a.cancel(home,*this,PC_WORD_DOM); b.cancel(home,*this,PC_WORD_DOM);
      q.cancel(home,*this,PC_WORD_DOM); r.cancel(home,*this,PC_WORD_DOM);
      (void) Propagator::dispose(home); return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,a,b,q,r));
      const bool is_assigned=a.assigned() && b.assigned() &&
        q.assigned() && r.assigned();
      return is_assigned ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView a, UnsignedWordView b,
                           UnsignedWordView q, UnsignedWordView r) {
      GECODE_ES_CHECK(narrow(home,a,b,q,r));
      if (a.varimp() == b.varimp())
        return ES_OK;
      const bool is_assigned=a.assigned() && b.assigned() &&
        q.assigned() && r.assigned();
      if (!is_assigned)
        (void) new (home) BoundUnsignedDivModBoth(home,a,b,q,r);
      return ES_OK;
    }
  protected:
    UnsignedWordView a, b, q, r;
    BoundUnsignedDivModBoth(Home home, UnsignedWordView a0,
                            UnsignedWordView b0, UnsignedWordView q0,
                            UnsignedWordView r0)
      : Propagator(home), a(a0), b(b0), q(q0), r(r0) {
      a.subscribe(home,*this,PC_WORD_DOM); b.subscribe(home,*this,PC_WORD_DOM);
      q.subscribe(home,*this,PC_WORD_DOM); r.subscribe(home,*this,PC_WORD_DOM);
    }
    BoundUnsignedDivModBoth(Space& home, BoundUnsignedDivModBoth& p)
      : Propagator(home,p) {
      a.update(home,p.a); b.update(home,p.b); q.update(home,p.q); r.update(home,p.r);
    }
    /// Close local deductions and publish only changed representatives.
    static ExecStatus narrow(Home home, UnsignedWordView a,
                             UnsignedWordView b, UnsignedWordView q,
                             UnsignedWordView r) {
      const UnsignedWordView input[4]={a,b,q,r};
      BoundLocalPass<UnsignedWordView,4> pass(input);
      const BoundDomainSnapshot<4> initial=pass.snapshot();
      for (;;) {
        const BoundDomainSnapshot<4> previous=pass.snapshot();
        pass.defer_synchronization();
        GECODE_ES_CHECK(narrow_unsigned_pair_identity(
          home,pass.domain(0),pass.domain(1),pass.domain(2),pass.domain(3)));
        GECODE_ES_CHECK(narrow_unsigned_pair_divisor(
          home,pass.domain(0),pass.domain(1),pass.domain(2),pass.domain(3)));
        GECODE_ES_CHECK(narrow_unsigned_pair_assigned(
          home,pass.domain(0),pass.domain(1),pass.domain(2),pass.domain(3)));
        if (!pass.synchronize_distinct()) return ES_FAILED;
        if (pass.is_unchanged(previous)) break;
      }
      return pass.publish_changed(home,initial);
    }
  };

  /// Signed division/remainder/modulo with a zero or strictly signed divisor.
  template<SignedDivModOperation op>
  class BoundSignedDivMod
    : public TernaryPropagator<SignedWordView,PC_WORD_DOM> {
  public:
    static bool numeric_regime(SignedWordView b) {
      const WordValue sign=sign_bit(b.width());
      return ((b.rank_minimum() == sign) &&
              (b.rank_maximum() == sign)) ||
        (b.rank_maximum() < sign) || (b.rank_minimum() > sign);
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundSignedDivMod(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      GECODE_ES_CHECK(narrow(home,x0,x1,x2));
      const bool is_assigned=x0.assigned() && x1.assigned() && x2.assigned();
      return is_assigned ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, SignedWordView a, SignedWordView b,
                           SignedWordView result) {
      GECODE_ES_CHECK(narrow(home,a,b,result));
      if (a.varimp() == b.varimp())
        return ES_OK;
      const bool is_assigned=a.assigned() && b.assigned() && result.assigned();
      if (!is_assigned)
        (void) new (home) BoundSignedDivMod(home,a,b,result);
      return ES_OK;
    }
  protected:
    using TernaryPropagator<SignedWordView,PC_WORD_DOM>::x0;
    using TernaryPropagator<SignedWordView,PC_WORD_DOM>::x1;
    using TernaryPropagator<SignedWordView,PC_WORD_DOM>::x2;
    BoundSignedDivMod(Home home, SignedWordView a, SignedWordView b,
                      SignedWordView r)
      : TernaryPropagator<SignedWordView,PC_WORD_DOM>(home,a,b,r) {}
    BoundSignedDivMod(Space& home, BoundSignedDivMod& p)
      : TernaryPropagator<SignedWordView,PC_WORD_DOM>(home,p) {}
    /// Close local deductions and publish only changed representatives.
    static ExecStatus narrow(Home home, SignedWordView a, SignedWordView b,
                             SignedWordView result) {
      const SignedWordView input[3]={a,b,result};
      BoundLocalPass<SignedWordView,3> pass(input);
      const BoundDomainSnapshot<3> initial=pass.snapshot();
      for (;;) {
        const BoundDomainSnapshot<3> previous=pass.snapshot();
        pass.defer_synchronization();
        GECODE_ES_CHECK(narrow_signed_identity<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        GECODE_ES_CHECK(narrow_signed_divisor<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        GECODE_ES_CHECK(narrow_signed_residue<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        GECODE_ES_CHECK(narrow_signed_assigned<op>(
          home,pass.domain(0),pass.domain(1),pass.domain(2)));
        if (!pass.synchronize_distinct()) return ES_FAILED;
        if (pass.is_unchanged(previous)) break;
      }
      return pass.publish_changed(home,initial);
    }
  };

}}}

#endif

// STATISTICS: word-prop
