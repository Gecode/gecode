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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_PRODUCT_MOD_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_PRODUCT_MOD_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Filter status, local cube progress and entailment after successful tells.
  struct BoundProductFilterResult {
    ExecStatus status;
    bool has_new_bits;
    bool is_entailed;
  };

  struct CheckedProductResult {
    bool is_valid;
    WordValue value;
  };

  struct ProductWindowResult {
    bool is_valid;
    WordValue minimum, maximum;
  };

  /// Calculate a host-width product only when both factors fit together.
  forceinline CheckedProductResult
  compute_checked_product(WordValue x, WordValue y) {
    const bool has_overflow=(x != 0U) && (y > (~WordValue(0))/x);
    if (has_overflow) return {false,0};
    return {true,x*y};
  }

  forceinline WordValue
  add_saturated_product(WordValue x, WordValue y) {
    return x > (~WordValue(0))-y ? ~WordValue(0) : x+y;
  }

  forceinline WordValue
  divide_product_ceiling(WordValue x, WordValue y) {
    assert(y != 0U);
    return x/y + ((x%y) != 0U);
  }

  forceinline bool
  narrow_product_factor(BoundLocalDomain& factor,
                                  const BoundLocalDomain& cofactor,
                                  WordValue target_min,
                                  WordValue target_max) {
    if (cofactor.maximum == 0U)
      return target_min == 0U;
    WordValue minimum=divide_product_ceiling(
      target_min,cofactor.maximum);
    WordValue maximum=factor.maximum;
    if (cofactor.minimum != 0U)
      maximum=std::min(maximum,target_max/cofactor.minimum);
    return factor.intersect_range(minimum,maximum);
  }

  forceinline ExecStatus
  narrow_product_modulus(Home home, Int::IntView modulus,
                            WordValue minimum, WordValue maximum) {
    if (minimum > static_cast<WordValue>(Int::Limits::max))
      return ES_FAILED;
    GECODE_ME_CHECK(modulus.gq(home,static_cast<int>(minimum)));
    if (maximum < static_cast<WordValue>(Int::Limits::max))
      GECODE_ME_CHECK(modulus.lq(home,static_cast<int>(maximum)));
    return ES_OK;
  }

  /// Apply cube deductions while retaining the live modulus tell/read order.
  forceinline ProductFilterResult
  narrow_bound_product_cube(Home home, BoundLocalView x, BoundLocalView y,
                            Int::IntView modulus, BoundLocalView result) {
    const WordValue result_hi=compute_product_mod_hull(
      result.width(),static_cast<unsigned int>(modulus.max()-1));
    if (me_failed(result.narrow(home,result.lo(),result.hi()&result_hi)))
      return {ES_FAILED,false};
    if (result.lo() >= static_cast<WordValue>(Int::Limits::max))
      return {ES_FAILED,false};
    if (me_failed(modulus.gq(home,static_cast<int>(result.lo()+1U))))
      return {ES_FAILED,false};
    const bool is_unit_modulus=modulus.assigned() && (modulus.val() == 1);
    if (is_unit_modulus) {
      if (me_failed(result.narrow(home,0U,0U))) return {ES_FAILED,false};
      return {ES_OK,true};
    }
    const bool has_zero_factor=(x.assigned() && (x.val() == 0U)) ||
      (y.assigned() && (y.val() == 0U));
    if (has_zero_factor) {
      if (me_failed(result.narrow(home,0U,0U))) return {ES_FAILED,false};
      return {ES_OK,true};
    }
    if (result.lo() != 0U) {
      const bool has_single_optional_x_bit=(x.lo() == 0U) && (x.hi() != 0U) &&
        ((x.hi() & (x.hi()-1U)) == 0U);
      if (has_single_optional_x_bit)
        if (me_failed(x.narrow(home,x.hi(),x.hi()))) return {ES_FAILED,false};
      const bool has_single_optional_y_bit=(y.lo() == 0U) && (y.hi() != 0U) &&
        ((y.hi() & (y.hi()-1U)) == 0U);
      if (has_single_optional_y_bit)
        if (me_failed(y.narrow(home,y.hi(),y.hi()))) return {ES_FAILED,false};
    }
    const bool is_assigned=x.assigned() && y.assigned();
    if (is_assigned) {
      if (modulus.assigned()) {
        const WordValue value=evaluate_product_mod(
          x.val(),y.val(),static_cast<WordValue>(modulus.val()));
        if (me_failed(result.narrow(home,value,value))) return {ES_FAILED,false};
        return {ES_OK,true};
      }
      const CheckedProductResult product=compute_checked_product(x.val(),y.val());
      const bool has_known_product=product.is_valid &&
        (product.value < static_cast<WordValue>(modulus.min()));
      if (has_known_product) {
        if (me_failed(result.narrow(home,product.value,product.value)))
          return {ES_FAILED,false};
        return {ES_OK,true};
      }
    }
    return {ES_OK,false};
  }

  /// Use the already narrowed remainder interval for one fixed floor quotient.
  forceinline ProductWindowResult
  compute_product_window(WordValue quotient, WordValue mmin, WordValue mmax,
                         const BoundLocalDomain& result) {
    const CheckedProductResult lower=compute_checked_product(quotient,mmin);
    if (!lower.is_valid) return {false,0,0};
    const WordValue minimum=add_saturated_product(lower.value,result.minimum);
    const CheckedProductResult upper=compute_checked_product(quotient,mmax);
    if (!upper.is_valid) return {false,0,0};
    const WordValue maximum=add_saturated_product(upper.value,result.maximum);
    return {true,minimum,maximum};
  }

  /// Narrow product/remainder intervals, then restrict factors sequentially.
  forceinline ProductFilterResult
  narrow_bound_product_ranges(Home home, BoundLocalDomain& x,
                              BoundLocalDomain& y, Int::IntView modulus,
                              BoundLocalDomain& result) {
    const CheckedProductResult lower=compute_checked_product(x.minimum,y.minimum);
    if (!lower.is_valid) return {ES_FAILED,false};
    const CheckedProductResult upper=compute_checked_product(x.maximum,y.maximum);
    if (!upper.is_valid) return {ES_FAILED,false};
    const WordValue pmin=lower.value, pmax=upper.value;
    const WordValue mmax=static_cast<WordValue>(modulus.max());
    const WordValue rmax=std::min(pmax,mmax-1U);
    if (!result.intersect_range(0U,rmax)) return {ES_FAILED,false};
    const ExecStatus status=narrow_product_modulus(
      home,modulus,result.minimum+1U,Int::Limits::max);
    if (status < ES_OK) return {status,false};

    bool is_entailed=false;
    const WordValue mmin=static_cast<WordValue>(modulus.min());
    if (pmax < mmin) {
      if (!result.intersect_range(pmin,pmax)) return {ES_FAILED,false};
      is_entailed=x.minimum == x.maximum && y.minimum == y.maximum;
    }

    const WordValue current_mmax=static_cast<WordValue>(modulus.max());
    const WordValue kmin=pmin/current_mmax;
    const WordValue kmax=pmax/mmin;
    if (kmin == kmax) {
      const WordValue k=kmin;
      const CheckedProductResult low_shift=compute_checked_product(k,current_mmax);
      if (!low_shift.is_valid) return {ES_FAILED,false};
      const CheckedProductResult high_shift=compute_checked_product(k,mmin);
      if (!high_shift.is_valid) return {ES_FAILED,false};
      if (!result.intersect_range(pmin-low_shift.value,pmax-high_shift.value))
        return {ES_FAILED,false};
      const ProductWindowResult window=compute_product_window(
        k,mmin,current_mmax,result);
      if (!window.is_valid) return {ES_FAILED,false};
      if (!narrow_product_factor(x,y,window.minimum,window.maximum))
        return {ES_FAILED,false};
      if (!narrow_product_factor(y,x,window.minimum,window.maximum))
        return {ES_FAILED,false};
    }

    const bool is_assigned=modulus.assigned() && (x.minimum == x.maximum) &&
      (y.minimum == y.maximum);
    if (is_assigned) {
      const WordValue value=evaluate_product_mod(
        x.minimum,y.minimum,static_cast<WordValue>(modulus.val()));
      if (!result.intersect_range(value,value)) return {ES_FAILED,false};
      is_entailed=true;
    }
    return {ES_OK,is_entailed};
  }

  forceinline
  BoundProductMod::BoundProductMod(Home home, UnsignedWordView x0,
                                   UnsignedWordView y0,
                                   Int::IntView modulus0,
                                   UnsignedWordView result0)
    : Propagator(home), x(x0), y(y0), modulus(modulus0), result(result0) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_WORD_DOM);
    y.subscribe(home,*this,PC_WORD_DOM);
    modulus.subscribe(home,*this,Int::PC_INT_BND);
    result.subscribe(home,*this,PC_WORD_DOM);
  }

  forceinline
  BoundProductMod::BoundProductMod(Space& home, BoundProductMod& p)
    : Propagator(home,p) {
    x.update(home,p.x); y.update(home,p.y);
    modulus.update(home,p.modulus); result.update(home,p.result);
  }

  forceinline Actor*
  BoundProductMod::copy(Space& home) {
    return new (home) BoundProductMod(home,*this);
  }

  forceinline PropCost
  BoundProductMod::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.width());
  }

  forceinline void
  BoundProductMod::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_DOM);
    y.reschedule(home,*this,PC_WORD_DOM);
    modulus.reschedule(home,*this,Int::PC_INT_BND);
    result.reschedule(home,*this,PC_WORD_DOM);
  }

  forceinline size_t
  BoundProductMod::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_DOM); y.cancel(home,*this,PC_WORD_DOM);
    modulus.cancel(home,*this,Int::PC_INT_BND);
    result.cancel(home,*this,PC_WORD_DOM);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /// Apply one local pass, retaining the order of live modulus tells and reads.
  forceinline ProductFilterResult
  narrow_bound_product_pass(Home home,
                            BoundLocalPass<UnsignedWordView,3>& pass,
                            Int::IntView modulus, WordValue mask) {
    bool is_entailed=false;
    if (modulus.assigned()) {
      const ProductFilterResult filtered=narrow_product_mod_coefficients(
        pass.domain(0),pass.domain(1),static_cast<WordValue>(modulus.val()),
        pass.domain(2));
      if (filtered.status < ES_OK) return {filtered.status,false};
      is_entailed |= filtered.is_entailed;
    }
    const bool is_assigned=(pass.domain(0).minimum == pass.domain(0).maximum) &&
      (pass.domain(1).minimum == pass.domain(1).maximum) &&
      (pass.domain(2).minimum == pass.domain(2).maximum);
    if (is_assigned) {
      const CheckedProductResult product=compute_checked_product(
        pass.domain(0).minimum,pass.domain(1).minimum);
      if (product.is_valid) {
        const ExecStatus status=narrow_product_mod_divisors(
          home,modulus,product.value,pass.domain(2).minimum);
        if (status < ES_OK) return {status,false};
        if (product.value == pass.domain(2).minimum) is_entailed=true;
      }
    }
    const bool has_nonwrapping_product=(pass.domain(0).maximum == 0U) ||
      (pass.domain(1).maximum <= mask/pass.domain(0).maximum);
    if (has_nonwrapping_product) {
      const ProductFilterResult filtered=narrow_bound_product_ranges(
        home,pass.domain(0),pass.domain(1),modulus,pass.domain(2));
      if (filtered.status < ES_OK) return {filtered.status,false};
      is_entailed |= filtered.is_entailed;
    }
    return {ES_OK,is_entailed};
  }

  /// Close deferred deductions before publishing any bounded Word variable.
  inline BoundProductFilterResult
  close_bound_product_mod(Home home, BoundLocalPass<UnsignedWordView,3>& pass,
                          Int::IntView modulus, WordValue mask,
                          bool needs_cube) {
    const BoundCubeSnapshot<3> initial=pass.snapshot_bits();
    const bool can_run_cube=needs_cube;
    bool is_entailed=false;
    for (;;) {
      const BoundDomainSnapshot<3> previous=pass.snapshot();
      const int old_mmin=modulus.min(), old_mmax=modulus.max();
      const unsigned int old_msize=modulus.size();
      pass.defer_synchronization();
      if (needs_cube) {
        const ProductFilterResult cube=narrow_bound_product_cube(
          home,pass.view(0),pass.view(1),modulus,pass.view(2));
        if (cube.status < ES_OK) return {cube.status,false,false};
        is_entailed |= cube.is_entailed;
      }
      const BoundCubeSnapshot<3> closed_cube=pass.snapshot_bits();
      const int cube_mmin=modulus.min(), cube_mmax=modulus.max();
      const unsigned int cube_msize=modulus.size();
      const ProductFilterResult filtered=narrow_bound_product_pass(
        home,pass,modulus,mask);
      if (filtered.status < ES_OK) return {filtered.status,false,false};
      is_entailed |= filtered.is_entailed;
      if (!pass.synchronize_changed(previous)) return {ES_FAILED,false,false};
      // Numeric closure only invalidates the cube stage through masks or
      // modulus changes. A bounds-only invocation leaves that stage queued.
      needs_cube=can_run_cube && (pass.has_new_bits(closed_cube) ||
        (modulus.min() != cube_mmin) || (modulus.max() != cube_mmax) ||
        (modulus.size() != cube_msize));
      const bool has_changed=!pass.is_unchanged(previous) ||
        (modulus.min() != old_mmin) || (modulus.max() != old_mmax) ||
        (modulus.size() != old_msize);
      if (!has_changed) break;
    }
    return {ES_OK,pass.has_new_bits(initial),is_entailed};
  }

  /// Own stable alias records through local closure and final publication.
  inline BoundProductFilterResult
  BoundProductMod::narrow(Home home, UnsignedWordView x,
                          UnsignedWordView y, Int::IntView modulus,
                          UnsignedWordView result, bool needs_cube) {
    if (me_failed(modulus.gq(home,1))) return {ES_FAILED,false,false};
    const UnsignedWordView input[3]={x,y,result};
    BoundLocalPass<UnsignedWordView,3> pass(input);
    const BoundProductFilterResult filtered=close_bound_product_mod(
      home,pass,modulus,x.mask(),needs_cube);
    if (filtered.status < ES_OK) return filtered;
    const ExecStatus status=pass.publish(home);
    if (status < ES_OK) return {status,false,false};
    return {status,filtered.has_new_bits,filtered.is_entailed};
  }

  forceinline bool
  BoundProductMod::can_use_numeric_stage(UnsignedWordView x, UnsignedWordView y) {
    return (x.rank_maximum() == 0U) ||
      (y.rank_maximum() <= x.mask()/x.rank_maximum());
  }

  inline ExecStatus
  BoundProductMod::post(Home home, UnsignedWordView x, UnsignedWordView y,
                        Int::IntView modulus, UnsignedWordView result) {
    const BoundProductFilterResult filtered=narrow(home,x,y,modulus,result,true);
    GECODE_ES_CHECK(filtered.status);
    if (!filtered.is_entailed)
      (void) new (home) BoundProductMod(home,x,y,modulus,result);
    return ES_OK;
  }

  inline ExecStatus
  BoundProductMod::propagate(Space& home, const ModEventDelta& med) {
    const bool is_bounds=UnsignedWordView::me(med) == ME_WORD_BND;
    const BoundProductFilterResult filtered=narrow(
      home,x,y,modulus,result,!is_bounds);
    GECODE_ES_CHECK(filtered.status);
    if (filtered.is_entailed) return home.ES_SUBSUMED(*this);
    const bool needs_bit_filter=is_bounds && filtered.has_new_bits;
    if (needs_bit_filter)
      return home.ES_NOFIX_PARTIAL(*this,
                                   UnsignedWordView::med(ME_WORD_BITS));
    return ES_FIX;
  }

  /// Apply algebraic filtering until bounds permit the numeric product stage.
  class RewritingProductMod : public Propagator {
  public:
    virtual Actor* copy(Space& home) { return new (home) RewritingProductMod(home,*this); }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,x.width());
    }
    virtual void reschedule(Space& home) {
      x.reschedule(home,*this,PC_WORD_DOM); y.reschedule(home,*this,PC_WORD_DOM);
      result.reschedule(home,*this,PC_WORD_DOM);
      modulus.reschedule(home,*this,Int::PC_INT_BND);
    }
    virtual size_t dispose(Space& home) {
      x.cancel(home,*this,PC_WORD_DOM); y.cancel(home,*this,PC_WORD_DOM);
      result.cancel(home,*this,PC_WORD_DOM);
      modulus.cancel(home,*this,Int::PC_INT_BND);
      home.ignore(*this,AP_WEAKLY);
      (void) Propagator::dispose(home); return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      const BoundProductFilterResult filtered=BoundProductMod::narrow(
        home,x,y,modulus,result,true);
      GECODE_ES_CHECK(filtered.status);
      if (filtered.is_entailed) return home.ES_SUBSUMED(*this);
      if (BoundProductMod::can_use_numeric_stage(x,y))
        GECODE_REWRITE(*this,(BoundProductMod::post(home(*this),x,y,modulus,result)));
      return ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView x, UnsignedWordView y,
                           Int::IntView modulus, UnsignedWordView result) {
      if (BoundProductMod::can_use_numeric_stage(x,y))
        return BoundProductMod::post(home,x,y,modulus,result);
      const BoundProductFilterResult filtered=BoundProductMod::narrow(
        home,x,y,modulus,result,true);
      GECODE_ES_CHECK(filtered.status);
      if (filtered.is_entailed) return ES_OK;
      if (BoundProductMod::can_use_numeric_stage(x,y))
        return BoundProductMod::post(home,x,y,modulus,result);
      (void) new (home) RewritingProductMod(home,x,y,modulus,result);
      return ES_OK;
    }
  protected:
    UnsignedWordView x, y, result;
    Int::IntView modulus;
    RewritingProductMod(Home home, UnsignedWordView a, UnsignedWordView b,
                        Int::IntView m, UnsignedWordView r)
      : Propagator(home), x(a), y(b), result(r), modulus(m) {
      home.notice(*this,AP_WEAKLY);
      x.subscribe(home,*this,PC_WORD_DOM); y.subscribe(home,*this,PC_WORD_DOM);
      result.subscribe(home,*this,PC_WORD_DOM);
      modulus.subscribe(home,*this,Int::PC_INT_BND);
    }
    RewritingProductMod(Space& home, RewritingProductMod& p) : Propagator(home,p) {
      x.update(home,p.x); y.update(home,p.y); result.update(home,p.result);
      modulus.update(home,p.modulus);
    }
  };

  inline ExecStatus
  post_product_mod(Home home, WordView x, WordView y,
                   Int::IntView modulus, WordView result) {
    const bool has_unsigned_operands=(x.domain_type() == WDT_UNSIGNED) &&
      (y.domain_type() == WDT_UNSIGNED) &&
      (result.domain_type() == WDT_UNSIGNED);
    if (has_unsigned_operands) {
      UnsignedWordView bx(x.varimp()), by(y.varimp()), br(result.varimp());
      return RewritingProductMod::post(home,bx,by,modulus,br);
    }
    return ProductMod::post(home,x,y,modulus,result);
  }

}}}

#endif

// STATISTICS: word-prop
