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

  namespace DivMod {
    forceinline WordValue find_min_nonzero(WordCube x) {
      return (x.lo != 0) ? x.lo : x.hi & (~x.hi+1);
    }
    forceinline WordCube compute_range_hull(unsigned int width,
                                            WordValue minimum,
                                            WordValue maximum) {
      assert(minimum <= maximum);
      WordValue varying=minimum^maximum;
      if (varying == 0)
        return {minimum,minimum};
      unsigned int bits=0;
      while (varying != 0) {
        varying >>= 1;
        bits++;
      }
      varying=width_mask(bits);
      const WordValue lo=minimum&~varying;
      return {lo,(lo|varying)&width_mask(width)};
    }
    forceinline ModEvent narrow_range(Space& home, WordView x,
                                      WordValue minimum, WordValue maximum) {
      const WordCube cube=compute_range_hull(x.width(),minimum,maximum);
      return x.narrow(home,cube.lo,cube.hi);
    }
    forceinline WordCube merge_cubes(WordCube x, WordCube y) {
      return {x.lo&y.lo,x.hi|y.hi};
    }
    forceinline bool has_changed(WordView x, WordValue lo, WordValue hi) {
      return (x.lo() != lo) || (x.hi() != hi);
    }

    forceinline WordCube compute_quotient_hull(unsigned int width,
                                               WordCube a, WordCube b) {
      const WordValue mask=width_mask(width);
      const WordValue divisor_min=find_min_nonzero(b);
      const WordCube cube=compute_range_hull(width,a.lo/b.hi,
                                             a.hi/divisor_min);
      return (b.lo == 0) ? merge_cubes(cube,{mask,mask}) : cube;
    }

    forceinline WordCube compute_remainder_hull(unsigned int width,
                                                WordCube a, WordCube b) {
      const WordValue maximum=std::min(b.hi-1,a.hi);
      const WordCube cube=compute_range_hull(width,0,maximum);
      return (b.lo == 0) ? merge_cubes(cube,a) : cube;
    }

    forceinline WordRankInterval
    compute_dividend_interval(WordCube q, WordValue divisor,
                               WordValue mask) {
      const WordValue maximum_quotient=mask/divisor;
      const WordValue quotient_max=std::min(q.hi,maximum_quotient);
      const WordValue minimum=q.lo*divisor;
      const WordValue room=mask-quotient_max*divisor;
      const WordValue maximum=quotient_max*divisor+
        std::min(room,divisor-1);
      return {minimum,maximum};
    }

    /// Apply the quotient limit before reading its inverse dividend interval.
    forceinline ExecStatus
    narrow_fixed_divisor_dividend(Home home, WordView a, WordValue divisor,
                                   WordView q) {
      const WordValue mask=a.mask(), maximum_quotient=mask/divisor;
      if (q.lo() > maximum_quotient)
        return ES_FAILED;
      GECODE_ME_CHECK(narrow_range(home,q,0,maximum_quotient));
      const WordRankInterval interval=compute_dividend_interval(
        {q.lo(),q.hi()},divisor,mask);
      GECODE_ME_CHECK(narrow_range(home,a,interval.minimum,interval.maximum));
      return ES_OK;
    }

    forceinline WordIntervalResult
    project_divisor(WordValue dividend, WordValue quotient, WordValue mask) {
      if (quotient == mask)
        return {true,{0,(dividend == mask) ? WordValue(1) : WordValue(0)}};
      if (quotient == 0)
        return (dividend == mask) ? WordIntervalResult{false,{0,0}} :
          WordIntervalResult{true,{dividend+1,mask}};
      const WordValue minimum=dividend/(quotient+1)+1;
      const WordValue maximum=dividend/quotient;
      return {minimum <= maximum,{minimum,maximum}};
    }

    forceinline ExecStatus
    narrow_inverse_divisor(Home home, WordView b, WordValue dividend,
                            WordValue quotient, WordValue mask) {
      const WordIntervalResult result=project_divisor(dividend,quotient,mask);
      if (!result.is_admitted)
        return ES_FAILED;
      if (result.interval.maximum == 0)
        GECODE_ME_CHECK(b.eq(home,0));
      else
        GECODE_ME_CHECK(narrow_range(home,b,result.interval.minimum,
                                     result.interval.maximum));
      return ES_OK;
    }

    /// Publish remainder bits before using them to narrow the dividend.
    forceinline ExecStatus
    narrow_fixed_divisor_remainder(Home home, WordView a, WordValue divisor,
                                    WordView r) {
      GECODE_ME_CHECK(narrow_range(home,r,0,divisor-1));
      const bool is_power_of_two=(divisor&(divisor-1)) == 0;
      if (is_power_of_two) {
        const WordValue low=divisor-1;
        GECODE_ME_CHECK(r.narrow(home,a.lo()&low,a.hi()&low));
        GECODE_ME_CHECK(a.narrow(home,a.lo()|(r.lo()&low),
                                 (a.hi()&~low)|(r.hi()&low)));
      }
      return ES_OK;
    }

    /// Assigned dividend admitted by a quotient and remainder pair
    struct DividendValueResult {
      bool has_value;
      WordValue value;
    };

    forceinline DividendValueResult
    compute_dividend_value(WordValue divisor, WordValue quotient,
                            WordValue remainder, WordValue mask) {
      const bool has_valid_results=(remainder < divisor) &&
        (quotient <= mask/divisor);
      if (!has_valid_results)
        return {false,0};
      const WordValue product=quotient*divisor;
      if (product > mask-remainder)
        return {false,0};
      return {true,product+remainder};
    }
  }

  forceinline Div::Div(Home home, WordView a, WordView b, WordView q)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,a,b,q) {}
  forceinline Div::Div(Space& home, Div& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline ExecStatus
  Div::narrow(Home home, WordView a, WordView b, WordView q) {
    const WordValue mask = a.mask();
    if (a == b) {
      if (a == q) {
        GECODE_ME_CHECK(q.eq(home,1));
        return ES_OK;
      }
      GECODE_ME_CHECK(q.narrow(home,1,mask));
      if (a.assigned()) {
        GECODE_ME_CHECK(q.eq(home,(a.val() == 0) ? mask : 1));
        return ES_OK;
      }
      if (q.assigned()) {
        const bool is_zero_divisor_result=(mask != 1) && (q.val() == mask);
        if (is_zero_divisor_result) {
          GECODE_ME_CHECK(a.eq(home,0));
          return ES_OK;
        }
        if (q.val() != 1)
          return ES_FAILED;
      }
      return (mask == 1) ? ES_OK : ES_FIX;
    }
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue qlo=q.lo(), qhi=q.hi();
      if (b.hi() == 0) {
        GECODE_ME_CHECK(q.eq(home,mask));
        return ES_OK;
      } else {
        const WordCube cube=DivMod::compute_quotient_hull(
          a.width(),{a.lo(),a.hi()},{b.lo(),b.hi()});
        GECODE_ME_CHECK(q.narrow(home,cube.lo,cube.hi));
      }
      const bool has_fixed_nonzero_divisor=b.assigned() && (b.val() != 0);
      if (has_fixed_nonzero_divisor) {
        const WordValue divisor=b.val();
        GECODE_ES_CHECK(DivMod::narrow_fixed_divisor_dividend(
          home,a,divisor,q));
      }
      const bool can_infer_divisor=a.assigned() && q.assigned();
      if (can_infer_divisor)
        GECODE_ES_CHECK(DivMod::narrow_inverse_divisor(
          home,b,a.val(),q.val(),mask));
      const bool is_stable=!DivMod::has_changed(a,alo,ahi) &&
        !DivMod::has_changed(b,blo,bhi) &&
        !DivMod::has_changed(q,qlo,qhi);
      if (is_stable)
        break;
    }
    const bool has_assigned_words=a.assigned() && b.assigned() && q.assigned();
    if (has_assigned_words) {
      const WordValue expected=(b.val()==0) ? mask : a.val()/b.val();
      return (q.val()==expected) ? ES_OK : ES_FAILED;
    }
    return ES_FIX;
  }
  forceinline ExecStatus Div::post(Home home, WordView a, WordView b,
                                   WordView q) {
    ExecStatus es=narrow(home,a,b,q);
    if (es == ES_FAILED) return ES_FAILED;
    if (es == ES_FIX) (void) new (home) Div(home,a,b,q);
    return ES_OK;
  }
  forceinline Actor* Div::copy(Space& home) {
    return new (home) Div(home,*this);
  }
  forceinline PropCost Div::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x0.width());
  }
  forceinline ExecStatus Div::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x0,x1,x2);
    if (es == ES_FAILED) return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

  forceinline Mod::Mod(Home home, WordView a, WordView b, WordView r)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,a,b,r) {}
  forceinline Mod::Mod(Space& home, Mod& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline ExecStatus
  Mod::narrow(Home home, WordView a, WordView b, WordView r) {
    const WordValue mask=a.mask();
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue rlo=r.lo(), rhi=r.hi();
      if (b.hi() == 0) {
        if (a == r)
          return ES_OK;
        const WordValue lo=a.lo()|r.lo(), hi=a.hi()&r.hi();
        GECODE_ME_CHECK(a.narrow(home,lo,hi));
        GECODE_ME_CHECK(r.narrow(home,lo,hi));
      } else {
        const WordCube remainder_cube=DivMod::compute_remainder_hull(
          a.width(),{a.lo(),a.hi()},{b.lo(),b.hi()});
        GECODE_ME_CHECK(r.narrow(home,remainder_cube.lo,remainder_cube.hi));
      }
      const bool has_fixed_nonzero_divisor=b.assigned() && (b.val() != 0);
      if (has_fixed_nonzero_divisor) {
        const WordValue divisor=b.val();
        GECODE_ES_CHECK(DivMod::narrow_fixed_divisor_remainder(
          home,a,divisor,r));
      }
      const bool can_infer_divisor_from_remainder=(b.lo() != 0) && r.assigned();
      if (can_infer_divisor_from_remainder) {
        if (r.val() == mask) return ES_FAILED;
        GECODE_ME_CHECK(DivMod::narrow_range(home,b,r.val()+1,mask));
      }
      const bool has_assigned_operands=a.assigned() && b.assigned();
      if (has_assigned_operands) {
        const WordValue expected=(b.val()==0) ? a.val() : a.val()%b.val();
        GECODE_ME_CHECK(r.eq(home,expected));
      }
      const bool is_stable=!DivMod::has_changed(a,alo,ahi) &&
        !DivMod::has_changed(b,blo,bhi) &&
        !DivMod::has_changed(r,rlo,rhi);
      if (is_stable)
        break;
    }
    const bool has_assigned_words=a.assigned() && b.assigned() && r.assigned();
    if (has_assigned_words) {
      const WordValue expected=(b.val()==0) ? a.val() : a.val()%b.val();
      return (r.val()==expected) ? ES_OK : ES_FAILED;
    }
    return ES_FIX;
  }
  forceinline ExecStatus Mod::post(Home home, WordView a, WordView b,
                                   WordView r) {
    if (a == b) {
      GECODE_ME_CHECK(r.eq(home,0));
      return ES_OK;
    }
    ExecStatus es=narrow(home,a,b,r);
    if (es == ES_FAILED) return ES_FAILED;
    if (es == ES_FIX) (void) new (home) Mod(home,a,b,r);
    return ES_OK;
  }
  forceinline Actor* Mod::copy(Space& home) {
    return new (home) Mod(home,*this);
  }
  forceinline PropCost Mod::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x0.width());
  }
  forceinline ExecStatus Mod::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x0,x1,x2);
    if (es == ES_FAILED) return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

  forceinline
  DivModBoth::DivModBoth(Home home, WordView dividend0, WordView divisor0,
                         WordView quotient0, WordView remainder0)
    : Propagator(home), dividend(dividend0), divisor(divisor0),
      quotient(quotient0), remainder(remainder0) {
    dividend.subscribe(home,*this,PC_WORD_BITS);
    divisor.subscribe(home,*this,PC_WORD_BITS);
    quotient.subscribe(home,*this,PC_WORD_BITS);
    remainder.subscribe(home,*this,PC_WORD_BITS);
  }
  forceinline DivModBoth::DivModBoth(Space& home, DivModBoth& p)
    : Propagator(home,p) {
    dividend.update(home,p.dividend);
    divisor.update(home,p.divisor);
    quotient.update(home,p.quotient);
    remainder.update(home,p.remainder);
  }

  forceinline ExecStatus
  DivModBoth::narrow(Home home, WordView a, WordView b, WordView q,
                     WordView r) {
    const WordValue mask=a.mask();
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue qlo=q.lo(), qhi=q.hi(), rlo=r.lo(), rhi=r.hi();

      if (b.hi() == 0) {
        GECODE_ME_CHECK(q.eq(home,mask));
        const bool has_distinct_remainder=!(a == r);
        if (has_distinct_remainder) {
          const WordValue lo=a.lo()|r.lo(), hi=a.hi()&r.hi();
          GECODE_ME_CHECK(a.narrow(home,lo,hi));
          GECODE_ME_CHECK(r.narrow(home,lo,hi));
        }
      } else {
        const WordCube cube=DivMod::compute_quotient_hull(
          a.width(),{a.lo(),a.hi()},{b.lo(),b.hi()});
        GECODE_ME_CHECK(q.narrow(home,cube.lo,cube.hi));

        const WordCube remainder_cube=DivMod::compute_remainder_hull(
          a.width(),{a.lo(),a.hi()},{b.lo(),b.hi()});
        GECODE_ME_CHECK(r.narrow(home,remainder_cube.lo,remainder_cube.hi));
      }

      const bool has_fixed_nonzero_divisor=b.assigned() && (b.val() != 0);
      if (has_fixed_nonzero_divisor) {
        const WordValue divisor=b.val();
        GECODE_ES_CHECK(DivMod::narrow_fixed_divisor_dividend(
          home,a,divisor,q));
        GECODE_ES_CHECK(DivMod::narrow_fixed_divisor_remainder(
          home,a,divisor,r));
        const bool has_assigned_results=q.assigned() && r.assigned();
        if (has_assigned_results) {
          const DivMod::DividendValueResult result=
            DivMod::compute_dividend_value(divisor,q.val(),r.val(),mask);
          if (!result.has_value)
            return ES_FAILED;
          GECODE_ME_CHECK(a.eq(home,result.value));
        }
      }

      const bool can_infer_divisor=a.assigned() && q.assigned();
      if (can_infer_divisor)
        GECODE_ES_CHECK(DivMod::narrow_inverse_divisor(
          home,b,a.val(),q.val(),mask));
      const bool can_infer_divisor_from_remainder=(b.lo() != 0) && r.assigned();
      if (can_infer_divisor_from_remainder) {
        if (r.val() == mask)
          return ES_FAILED;
        GECODE_ME_CHECK(DivMod::narrow_range(home,b,r.val()+1,mask));
      }
      const bool has_assigned_operands=a.assigned() && b.assigned();
      if (has_assigned_operands) {
        const WordValue expected_q=(b.val()==0) ? mask : a.val()/b.val();
        const WordValue expected_r=(b.val()==0) ? a.val() : a.val()%b.val();
        GECODE_ME_CHECK(q.eq(home,expected_q));
        GECODE_ME_CHECK(r.eq(home,expected_r));
      }

      const bool is_stable=!DivMod::has_changed(a,alo,ahi) &&
        !DivMod::has_changed(b,blo,bhi) &&
        !DivMod::has_changed(q,qlo,qhi) && !DivMod::has_changed(r,rlo,rhi);
      if (is_stable)
        break;
    }
    const bool has_assigned_words=a.assigned() && b.assigned() &&
      q.assigned() && r.assigned();
    if (has_assigned_words) {
      const WordValue expected_q=(b.val()==0) ? mask : a.val()/b.val();
      const WordValue expected_r=(b.val()==0) ? a.val() : a.val()%b.val();
      return ((q.val()==expected_q) && (r.val()==expected_r)) ?
        ES_OK : ES_FAILED;
    }
    const bool is_zero_divisor_identity=(b.hi() == 0) && (a == r) &&
      q.assigned() && (q.val() == mask);
    if (is_zero_divisor_identity)
      return ES_OK;
    return ES_FIX;
  }

  forceinline ExecStatus
  DivModBoth::post(Home home, WordView a, WordView b, WordView q, WordView r) {
    if (a == b) {
      GECODE_ME_CHECK(r.eq(home,0));
      return Div::post(home,a,b,q);
    }
    ExecStatus es=narrow(home,a,b,q,r);
    if (es == ES_FAILED) return ES_FAILED;
    if (es == ES_FIX) (void) new (home) DivModBoth(home,a,b,q,r);
    return ES_OK;
  }
  forceinline Actor* DivModBoth::copy(Space& home) {
    return new (home) DivModBoth(home,*this);
  }
  forceinline PropCost
  DivModBoth::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,dividend.width());
  }
  forceinline void DivModBoth::reschedule(Space& home) {
    dividend.reschedule(home,*this,PC_WORD_BITS);
    divisor.reschedule(home,*this,PC_WORD_BITS);
    quotient.reschedule(home,*this,PC_WORD_BITS);
    remainder.reschedule(home,*this,PC_WORD_BITS);
  }
  forceinline size_t DivModBoth::dispose(Space& home) {
    dividend.cancel(home,*this,PC_WORD_BITS);
    divisor.cancel(home,*this,PC_WORD_BITS);
    quotient.cancel(home,*this,PC_WORD_BITS);
    remainder.cancel(home,*this,PC_WORD_BITS);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
  forceinline ExecStatus
  DivModBoth::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,dividend,divisor,quotient,remainder);
    if (es == ES_FAILED) return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
