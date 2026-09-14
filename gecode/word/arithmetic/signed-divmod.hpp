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

namespace Gecode { namespace Word { namespace Arithmetic {

  namespace SignedDivModSupport {
    forceinline bool is_negative(WordValue value, WordValue sign) {
      return (value & sign) != 0;
    }
    forceinline WordValue negate(WordValue value, WordValue mask) {
      return (~value+1) & mask;
    }
    forceinline WordValue compute_magnitude(WordValue value, WordValue sign,
                                            WordValue mask) {
      return is_negative(value,sign) ? negate(value,mask) : value;
    }
    template<SignedDivModOperation op>
    forceinline WordValue evaluate(WordValue a, WordValue b,
                                   WordValue sign, WordValue mask) {
      if (b == 0)
        return (op == SDO_DIV) ? (is_negative(a,sign) ? 1 : mask) : a;
      const bool is_a_negative=is_negative(a,sign);
      const bool is_b_negative=is_negative(b,sign);
      const WordValue am=compute_magnitude(a,sign,mask);
      const WordValue bm=compute_magnitude(b,sign,mask);
      if (op == SDO_DIV) {
        const WordValue q=am/bm;
        return (is_a_negative != is_b_negative) ? negate(q,mask) : q;
      }
      WordValue r=am%bm;
      if (is_a_negative)
        r=negate(r,mask);
      const bool needs_sign_adjustment=(op == SDO_MOD) && (r != 0) &&
        (is_a_negative != is_b_negative);
      if (needs_sign_adjustment)
        r=(r+b) & mask;
      return r;
    }
    forceinline bool has_changed(WordView x, WordValue lo, WordValue hi) {
      return (x.lo() != lo) || (x.hi() != hi);
    }
    forceinline ExecStatus
    propagate_equality(Home home, WordView x, WordView y) {
      for (;;) {
        const WordValue lo=x.lo()|y.lo(), hi=x.hi()&y.hi();
        GECODE_ME_CHECK(x.narrow(home,lo,hi));
        GECODE_ME_CHECK(y.narrow(home,lo,hi));
        const bool is_stable=(x.lo() == lo) && (x.hi() == hi) &&
          (y.lo() == lo) && (y.hi() == hi);
        if (is_stable)
          break;
      }
      return (x.assigned() && y.assigned()) ? ES_OK : ES_FIX;
    }
    template<class A, class R>
    forceinline ExecStatus
    propagate_common_residue(Home home, A a, WordValue divisor, R r) {
      // Both signed remainder conventions differ from a by a multiple of b.
      // Its power-of-two factor therefore equates these low bits, even for
      // negative operands and divisors. Unsigned negation also handles MIN.
      assert(divisor != 0U);
      const WordValue low=(divisor & (WordValue(0)-divisor))-1U;
      if (low == 0U)
        return ES_OK;
      const WordValue lo=(a.lo()|r.lo())&low;
      const WordValue hi=(a.hi()&r.hi())&low;
      GECODE_ME_CHECK(a.narrow(home,(a.lo()&~low)|lo,
                               (a.hi()&~low)|hi));
      GECODE_ME_CHECK(r.narrow(home,(r.lo()&~low)|lo,
                               (r.hi()&~low)|hi));
      return ES_OK;
    }

    /// Supported dividend cube after inverse quotient projection
    struct DividendProjectionResult {
      bool has_support;
      WordCube cube;
    };

    forceinline DividendProjectionResult
    project_dividend_interval(WordView a, WordValue minimum,
                               WordValue maximum) {
      const WordDomainResult result=compute_domain_closure(
        a.width(),WDT_UNSIGNED,{{a.lo(),a.hi()},{minimum,maximum}});
      return {result.is_consistent,result.domain.cube};
    }

    forceinline DividendProjectionResult
    project_dividend(WordView a, WordValue divisor, WordValue quotient) {
      const WordValue mask=a.mask(), sign=sign_bit(a.width());
      assert((divisor != 0U) && (divisor != mask));
      const WordValue divisor_magnitude=compute_magnitude(divisor,sign,mask);
      const WordValue quotient_magnitude=compute_magnitude(quotient,sign,mask);
      if (quotient_magnitude == 0U) {
        // Zero quotient admits both signs until the cube excludes an interval.
        const DividendProjectionResult nonnegative=project_dividend_interval(
          a,0,divisor_magnitude-1U);
        const DividendProjectionResult negative=(divisor_magnitude > 1U) ?
          project_dividend_interval(a,negate(divisor_magnitude-1U,mask),mask) :
          DividendProjectionResult{false,{0,0}};
        if (!nonnegative.has_support)
          return negative;
        if (!negative.has_support)
          return nonnegative;
        return {true,DivMod::merge_cubes(nonnegative.cube,negative.cube)};
      }
      const bool is_dividend_negative=is_negative(quotient,sign) !=
        is_negative(divisor,sign);
      const WordValue limit=is_dividend_negative ? sign : sign-1U;
      if (quotient_magnitude > limit/divisor_magnitude)
        return {false,{0,0}};
      const WordValue minimum=quotient_magnitude*divisor_magnitude;
      const WordValue maximum=minimum+
        std::min(divisor_magnitude-1U,limit-minimum);
      return project_dividend_interval(a,
        is_dividend_negative ? negate(maximum,mask) : minimum,
        is_dividend_negative ? negate(minimum,mask) : maximum);
    }

    forceinline ExecStatus
    narrow_inverse_dividend(Home home, WordView a, WordValue divisor,
                             WordValue quotient) {
      const DividendProjectionResult result=project_dividend(a,divisor,quotient);
      if (!result.has_support)
        return ES_FAILED;
      GECODE_ME_CHECK(a.narrow(home,result.cube.lo,result.cube.hi));
      return ES_OK;
    }

    template<class A, class R>
    forceinline ExecStatus
    propagate_power_of_two_mod(Home home, A a, WordValue divisor, R r) {
      const WordValue low=divisor-1U;
      const WordValue lo=(a.lo()|r.lo())&low;
      const WordValue hi=(a.hi()&r.hi())&low;
      GECODE_ME_CHECK(r.narrow(home,lo,hi));
      GECODE_ME_CHECK(a.narrow(home,(a.lo()&~low)|lo,
                               (a.hi()&~low)|hi));
      return ES_OK;
    }

    /// Propagation status and whether it terminates the enclosing division
    struct DivisionStepResult {
      bool should_return;
      ExecStatus status;
    };

    /// Preserve equality completion separately from continued division rules.
    template<SignedDivModOperation op>
    forceinline DivisionStepResult
    propagate_zero_divisor(Home home, WordView a, WordView r) {
      if (op != SDO_DIV)
        return {true,(a == r) ? ES_OK : propagate_equality(home,a,r)};
      const WordValue mask=a.mask(), sign=sign_bit(a.width());
      if ((a.lo()&sign) != 0)
        return {true,me_failed(r.eq(home,1)) ? ES_FAILED : ES_OK};
      if ((a.hi()&sign) == 0)
        return {true,me_failed(r.eq(home,mask)) ? ES_FAILED : ES_OK};
      if (me_failed(r.narrow(home,1,mask)))
        return {true,ES_FAILED};
      if (mask == 1)
        return {true,ES_OK};
      if (!r.assigned())
        return {false,ES_OK};
      if (r.val() == 1) {
        if (me_failed(a.narrow(home,a.lo()|sign,a.hi())))
          return {true,ES_FAILED};
      } else if (r.val() == mask) {
        if (me_failed(a.narrow(home,a.lo(),a.hi()&~sign)))
          return {true,ES_FAILED};
      } else {
        return {true,ES_FAILED};
      }
      return {false,ES_OK};
    }

    template<SignedDivModOperation op>
    forceinline ExecStatus
    propagate_result_sign(Home home, WordView a, WordView b, WordView r,
                           WordValue sign) {
      if (op == SDO_DIV) {
        const bool is_a_negative=(a.lo()&sign) != 0;
        const bool is_a_nonnegative=(a.hi()&sign) == 0;
        const bool is_b_negative=(b.lo()&sign) != 0;
        const bool is_b_nonnegative=(b.hi()&sign) == 0;
        const bool has_opposite_signs=(is_a_negative && is_b_nonnegative) ||
          (is_a_nonnegative && is_b_negative);
        const bool has_negative_quotient=(b.lo() != 0) &&
          has_opposite_signs && (r.lo() != 0);
        if (has_negative_quotient)
          GECODE_ME_CHECK(r.narrow(home,r.lo()|sign,r.hi()));
        return ES_OK;
      }
      const WordView source=(op == SDO_REM) ? a : b;
      const bool has_sign_source=(op == SDO_REM) || (b.lo() != 0);
      const bool is_nonnegative_result=has_sign_source &&
        ((source.hi()&sign) == 0);
      if (is_nonnegative_result) {
        GECODE_ME_CHECK(r.narrow(home,r.lo(),r.hi()&~sign));
      } else {
        const bool is_negative_result=((source.lo()&sign) != 0) &&
          (r.lo() != 0);
        if (is_negative_result)
          GECODE_ME_CHECK(r.narrow(home,r.lo()|sign,r.hi()));
      }
      return ES_OK;
    }

  }

  template<SignedDivModOperation op>
  forceinline
  SignedDivMod<op>::SignedDivMod(Home home, WordView a, WordView b,
                                 WordView r)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,a,b,r) {}

  template<SignedDivModOperation op>
  forceinline
  SignedDivMod<op>::SignedDivMod(Space& home, SignedDivMod<op>& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  template<SignedDivModOperation op>
  forceinline ExecStatus
  SignedDivMod<op>::narrow(Home home, WordView a, WordView b, WordView r) {
    const WordValue mask=a.mask();
    const WordValue sign=WordValue(1) << (a.width()-1);
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue rlo=r.lo(), rhi=r.hi();

      if (b.assigned()) {
        if (b.val() == 0) {
          const SignedDivModSupport::DivisionStepResult result=
            SignedDivModSupport::propagate_zero_divisor<op>(home,a,r);
          if (result.should_return)
            return result.status;
        }
        if (b.val() == 1) {
          if (op == SDO_DIV) {
            if (a == r)
              return ES_OK;
            return SignedDivModSupport::propagate_equality(home,a,r);
          }
          GECODE_ME_CHECK(r.eq(home,0));
          return ES_OK;
        }
        const bool has_unit_remainder=(b.val() == mask) && (op != SDO_DIV);
        if (has_unit_remainder) {
          GECODE_ME_CHECK(r.eq(home,0));
          return ES_OK;
        }
        const bool can_share_residue=(op != SDO_DIV) && (b.val() != 0U);
        if (can_share_residue)
          GECODE_ES_CHECK(SignedDivModSupport::propagate_common_residue(
            home,a,b.val(),r));
        const bool can_infer_dividend=(op == SDO_DIV) && (b.val() != 0U) &&
          (b.val() != mask) && r.assigned();
        if (can_infer_dividend)
          GECODE_ES_CHECK(SignedDivModSupport::narrow_inverse_dividend(
            home,a,b.val(),r.val()));
        const bool is_positive_power_of_two=(op == SDO_MOD) && (b.val() != 0U) &&
          ((b.val()&sign) == 0U) && ((b.val()&(b.val()-1U)) == 0U);
        if (is_positive_power_of_two)
          GECODE_ES_CHECK(SignedDivModSupport::propagate_power_of_two_mod(
            home,a,b.val(),r));
      }

      const bool has_assigned_operands=a.assigned() && b.assigned();
      if (has_assigned_operands) {
        GECODE_ME_CHECK(r.eq(home,SignedDivModSupport::evaluate<op>(
          a.val(),b.val(),sign,mask)));
        return ES_OK;
      }

      const bool has_zero_result=a.assigned() && (a.val() == 0) &&
        ((op != SDO_DIV) || (b.lo() != 0));
      if (has_zero_result) {
        GECODE_ME_CHECK(r.eq(home,0));
        return ES_OK;
      }

      GECODE_ES_CHECK(SignedDivModSupport::propagate_result_sign<op>(
        home,a,b,r,sign));

      const bool is_stable=!SignedDivModSupport::has_changed(a,alo,ahi) &&
        !SignedDivModSupport::has_changed(b,blo,bhi) &&
        !SignedDivModSupport::has_changed(r,rlo,rhi);
      if (is_stable)
        break;
    }
    return ES_FIX;
  }

  template<SignedDivModOperation op>
  forceinline ExecStatus
  SignedDivMod<op>::post(Home home, WordView a, WordView b, WordView r) {
    const bool has_self_remainder=(op != SDO_DIV) && (a == b);
    if (has_self_remainder) {
      GECODE_ME_CHECK(r.eq(home,0));
      return ES_OK;
    }
    const bool is_negation=b.assigned() && (b.val() == a.mask()) &&
      (op == SDO_DIV);
    if (is_negation)
      return Neg::post(home,a,r);
    ExecStatus es=narrow(home,a,b,r);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) SignedDivMod<op>(home,a,b,r);
    return ES_OK;
  }

  template<SignedDivModOperation op>
  forceinline Actor*
  SignedDivMod<op>::copy(Space& home) {
    return new (home) SignedDivMod<op>(home,*this);
  }

  template<SignedDivModOperation op>
  forceinline PropCost
  SignedDivMod<op>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  template<SignedDivModOperation op>
  forceinline ExecStatus
  SignedDivMod<op>::propagate(Space& home, const ModEventDelta&) {
    const bool is_negation=x1.assigned() && (x1.val() == x0.mask()) &&
      (op == SDO_DIV);
    if (is_negation) {
      GECODE_REWRITE(*this,(Neg::post(home(*this),x0,x2)));
    }
    ExecStatus es=narrow(home,x0,x1,x2);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
