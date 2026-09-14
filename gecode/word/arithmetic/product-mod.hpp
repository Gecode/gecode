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

#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-progression.hpp>

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Status and entailment contributed by one successful filtering stage.
  struct ProductFilterResult {
    ExecStatus status;
    bool is_entailed;
  };

  struct DivisorBoundsResult {
    bool is_valid;
    int minimum, maximum;
  };

  struct KnownResidueResult {
    bool has_value;
    WordValue value;
  };

  /// Add residues below a positive modulus without overflowing their sum.
  forceinline WordValue
  add_product_residues(WordValue x, WordValue y, WordValue modulus) {
    return x >= modulus-y ? x-(modulus-y) : x+y;
  }

  /// Reduce by a positive modulus before doubling, avoiding the full product.
  forceinline WordValue
  evaluate_product_mod(WordValue x, WordValue y, WordValue modulus) {
    x %= modulus;
    y %= modulus;
    WordValue product=0;
    while (y != 0) {
      if ((y & 1U) != 0)
        product=add_product_residues(product,x,modulus);
      y >>= 1;
      if (y != 0)
        x=add_product_residues(x,x,modulus);
    }
    return product;
  }

  forceinline WordValue
  compute_product_mod_hull(unsigned int width, unsigned int maximum) {
    WordValue hull=0;
    while (maximum != 0) {
      hull=(hull << 1) | 1U;
      maximum >>= 1;
    }
    return hull & width_mask(width);
  }

  forceinline bool
  has_single_optional_bit(WordView x) {
    return (x.lo() == 0) && (x.hi() != 0) &&
      ((x.hi() & (x.hi()-1)) == 0);
  }

  /// Project an unsigned cube without interpreting signed ranked endpoints.
  forceinline BoundLocalDomain
  snapshot_product_mod_cube(WordView x) {
    return BoundLocalDomain{x.width(),WDT_UNSIGNED,x.lo(),x.hi(),
                            x.lo(),x.hi(),false};
  }

  /// Invert coprime 0 < a < m <= Int::Limits::max using signed extended Euclid.
  forceinline WordValue
  invert_product_mod_coefficient(WordValue a, WordValue m) {
    long long int old_r=static_cast<long long int>(a);
    long long int r=static_cast<long long int>(m);
    long long int old_s=1, s=0;
    while (r != 0) {
      const long long int q=old_r/r;
      const long long int nr=old_r-q*r; old_r=r; r=nr;
      const long long int ns=old_s-q*s; old_s=s; s=ns;
    }
    old_s %= static_cast<long long int>(m);
    if (old_s < 0) old_s += static_cast<long long int>(m);
    return static_cast<WordValue>(old_s);
  }

  /// Filter a fixed coefficient independently of the full product width.
  forceinline bool
  narrow_product_mod_coefficient(BoundLocalDomain& factor, WordValue c,
                                 WordValue m, BoundLocalDomain& result) {
    const WordValue g=compute_bound_gcd(c,m);
    const WordValue low_mask=(g&(~g+1U))-1U;
    result.hi &= ~low_mask;
    const bool has_failed=(result.lo&~result.hi) != 0U ||
      !result.intersect_range(0U,m-1U) || !bound_progression(result,0U,g);
    if (has_failed) return false;
    if (c == 0U)
      return result.intersect_range(0U,0U);
    if (result.minimum != result.maximum)
      return true;
    if ((result.minimum%g) != 0U)
      return false;
    const WordValue step=m/g;
    if (step == 1U)
      return true;
    const WordValue residue=evaluate_product_mod(
      invert_product_mod_coefficient(c/g,step),result.minimum/g,step);
    return bound_progression(factor,residue,step);
  }

  /// Filter assigned coefficients and report this stage's entailment fact.
  forceinline ProductFilterResult
  narrow_product_mod_coefficients(BoundLocalDomain& x, BoundLocalDomain& y,
                                  WordValue m, BoundLocalDomain& result) {
    if (!result.intersect_range(0U,m-1U)) return {ES_FAILED,false};
    const bool is_assigned=(x.minimum == x.maximum) && (y.minimum == y.maximum);
    if (is_assigned) {
      const WordValue value=evaluate_product_mod(x.minimum,y.minimum,m);
      if (!result.intersect_range(value,value)) return {ES_FAILED,false};
      return {ES_OK,true};
    }
    if (m == 1U) {
      if (!result.intersect_range(0U,0U)) return {ES_FAILED,false};
      return {ES_OK,true};
    }
    const bool has_no_fixed_factor=
      (x.minimum != x.maximum) && (y.minimum != y.maximum);
    if (has_no_fixed_factor) return {ES_OK,false};
    const bool is_x_fixed=x.minimum == x.maximum;
    const WordValue c=(is_x_fixed ? x.minimum : y.minimum)%m;
    if (!narrow_product_mod_coefficient(is_x_fixed ? y : x,c,m,result))
      return {ES_FAILED,false};
    return {ES_OK,c == 0U};
  }

  /** \brief Owned local cube deductions and their valid publication prefix
   *
   * Representatives are indices, so returning this value cannot invalidate
   * aliases. A failed synchronization retains earlier representatives for
   * publication before its failure is forwarded, as in the original pass.
   */
  struct ProductCubeDeductionResult {
    ProductFilterResult filtered;
    BoundLocalDomain domains[3];
    unsigned int representatives[3];
    unsigned int publication_limit;
  };

  /// Deduce and synchronize one compact coefficient pass without Word tells.
  forceinline ProductCubeDeductionResult
  compute_product_cube_deductions(WordView x, WordView y, WordValue modulus,
                                  WordView result) {
    ProductCubeDeductionResult deductions={
      {ES_OK,false},
      {snapshot_product_mod_cube(x),snapshot_product_mod_cube(y),
       snapshot_product_mod_cube(result)},
      {0,1,2},0};
    if (x == y) deductions.representatives[1]=0;
    if (x == result) deductions.representatives[2]=0;
    else if (y == result)
      deductions.representatives[2]=deductions.representatives[1];
    for (unsigned int i=0; i<3; i++) deductions.domains[i].deferred=true;
    deductions.filtered=narrow_product_mod_coefficients(
      deductions.domains[deductions.representatives[0]],
      deductions.domains[deductions.representatives[1]],modulus,
      deductions.domains[deductions.representatives[2]]);
    if (deductions.filtered.status < ES_OK) return deductions;
    for (unsigned int i=0; i<3; i++) {
      if (deductions.representatives[i] != i) continue;
      if (!deductions.domains[i].synchronize()) {
        deductions.filtered={ES_FAILED,false};
        return deductions;
      }
      deductions.publication_limit=i+1;
    }
    return deductions;
  }

  /// Tell the valid prefix, then forward any later local synchronization failure.
  forceinline ProductFilterResult
  publish_product_cube_deductions(Home home, WordView (&views)[3],
                                  const ProductCubeDeductionResult& deductions) {
    for (unsigned int i=0; i<deductions.publication_limit; i++) {
      if (deductions.representatives[i] != i) continue;
      const BoundLocalDomain& domain=deductions.domains[i];
      const bool has_changed=(views[i].lo() != domain.lo) ||
        (views[i].hi() != domain.hi);
      if (has_changed)
        if (me_failed(views[i].narrow(home,domain.lo,domain.hi)))
          return {ES_FAILED,false};
    }
    return deductions.filtered;
  }

  /// Apply one compact pass; the actor retains its outer fixpoint loop.
  forceinline ProductFilterResult
  narrow_product_mod_cubes(Home home, WordView x, WordView y,
                          Int::IntView modulus, WordView result) {
    const ProductCubeDeductionResult deductions=compute_product_cube_deductions(
      x,y,static_cast<WordValue>(modulus.val()),result);
    WordView views[3]={x,y,result};
    return publish_product_cube_deductions(home,views,deductions);
  }

  forceinline
  ProductMod::ProductMod(Home home, WordView x0, WordView y0,
                         Int::IntView modulus0, WordView result0)
    : Propagator(home), x(x0), y(y0), modulus(modulus0), result(result0) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_WORD_BITS);
    y.subscribe(home,*this,PC_WORD_BITS);
    modulus.subscribe(home,*this,Int::PC_INT_BND);
    result.subscribe(home,*this,PC_WORD_BITS);
  }

  forceinline
  ProductMod::ProductMod(Space& home, ProductMod& p)
    : Propagator(home,p) {
    x.update(home,p.x);
    y.update(home,p.y);
    modulus.update(home,p.modulus);
    result.update(home,p.result);
  }

  forceinline Actor*
  ProductMod::copy(Space& home) {
    return new (home) ProductMod(home,*this);
  }

  forceinline PropCost
  ProductMod::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.width());
  }

  forceinline void
  ProductMod::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_BITS);
    y.reschedule(home,*this,PC_WORD_BITS);
    modulus.reschedule(home,*this,Int::PC_INT_BND);
    result.reschedule(home,*this,PC_WORD_BITS);
  }

  forceinline size_t
  ProductMod::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_BITS);
    y.cancel(home,*this,PC_WORD_BITS);
    modulus.cancel(home,*this,Int::PC_INT_BND);
    result.cancel(home,*this,PC_WORD_BITS);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /// Compute necessary modulus bounds for an exact unsigned product/result.
  forceinline DivisorBoundsResult
  compute_product_mod_divisors(WordValue product, WordValue result,
                               int minimum, int maximum) {
    const bool is_inconsistent=(product < result) ||
      (result >= static_cast<WordValue>(Int::Limits::max));
    if (is_inconsistent) return {false,0,0};
    minimum=std::max(minimum,static_cast<int>(result)+1);
    if (product == result)
      return {minimum <= maximum,minimum,maximum};
    const WordValue difference=product-result;
    if (difference < static_cast<WordValue>(maximum))
      maximum=static_cast<int>(difference);
    if (minimum > maximum) return {false,0,0};
    const WordValue quotient=difference/static_cast<WordValue>(maximum);
    if (quotient == difference/static_cast<WordValue>(minimum)) {
      // A fixed floor quotient leaves at most one divisor in the interval.
      if (difference%quotient != 0U) return {false,0,0};
      const WordValue candidate=difference/quotient;
      const bool is_outside=(candidate < static_cast<WordValue>(minimum)) ||
        (candidate > static_cast<WordValue>(maximum));
      if (is_outside) return {false,0,0};
      minimum=maximum=static_cast<int>(candidate);
    }
    return {true,minimum,maximum};
  }

  /// Publish exact-product divisor bounds through ordinary integer tells.
  forceinline ExecStatus
  narrow_product_mod_divisors(Home home, Int::IntView modulus,
                              WordValue product, WordValue result) {
    const int minimum=modulus.min(), maximum=modulus.max();
    const DivisorBoundsResult bounds=compute_product_mod_divisors(
      product,result,minimum,maximum);
    if (!bounds.is_valid) return ES_FAILED;
    GECODE_ME_CHECK(modulus.gq(home,bounds.minimum));
    GECODE_ME_CHECK(modulus.lq(home,bounds.maximum));
    return ES_OK;
  }

  /// Perform one compact pass; the actor owns repetition to a stable cube.
  forceinline ExecStatus
  prune_product_mod_pass(Home home, WordView x, WordView y,
                    Int::IntView modulus, WordView result) {
    GECODE_ME_CHECK(modulus.gq(home,1));

    // The cube hull of the integer interval [0,modulus.max()-1].
    const WordValue result_hi=compute_product_mod_hull(
      result.width(),static_cast<unsigned int>(modulus.max()-1));
    GECODE_ME_CHECK(result.narrow(home,result.lo(),result.hi()&result_hi));

    if (result.lo() >= static_cast<WordValue>(Int::Limits::max))
      return ES_FAILED;
    GECODE_ME_CHECK(modulus.gq(home,static_cast<int>(result.lo()+1)));

    if (modulus.assigned()) {
      const ProductFilterResult filtered=narrow_product_mod_cubes(
        home,x,y,modulus,result);
      GECODE_ES_CHECK(filtered.status);
      if (filtered.is_entailed) return ES_OK;
    }
    const bool has_zero_factor=(x.assigned() && (x.val() == 0)) ||
      (y.assigned() && (y.val() == 0));
    if (has_zero_factor) {
      GECODE_ME_CHECK(result.eq(home,0));
      return ES_OK;
    }

    if (result.lo() != 0) {
      if (has_single_optional_bit(x))
        GECODE_ME_CHECK(x.eq(home,x.hi()));
      if (has_single_optional_bit(y))
        GECODE_ME_CHECK(y.eq(home,y.hi()));
    }

    const bool is_assigned=x.assigned() && y.assigned();
    if (is_assigned) {
      if (modulus.assigned()) {
        GECODE_ME_CHECK(result.eq(home,evaluate_product_mod(
          x.val(),y.val(),static_cast<WordValue>(modulus.val()))));
        return ES_OK;
      }
      const WordValue xv=x.val(), yv=y.val();
      const bool has_exact_product=result.assigned() &&
        ((yv == 0U) || (xv <= ~WordValue(0)/yv));
      if (has_exact_product)
        GECODE_ES_CHECK(narrow_product_mod_divisors(
          home,modulus,xv*yv,result.val()));
      const WordValue limit=static_cast<WordValue>(modulus.min()-1);
      const bool has_known_product=(yv == 0) || (xv <= limit/yv);
      if (has_known_product) {
        GECODE_ME_CHECK(result.eq(home,xv*yv));
        return ES_OK;
      }
    }
    return ES_FIX;
  }

  /// Repeat compact deductions and tells until every observed domain is stable.
  forceinline ExecStatus
  ProductMod::prune(Home home, WordView x, WordView y,
                    Int::IntView modulus, WordView result) {
    for (;;) {
      const WordValue lo[3]={x.lo(),y.lo(),result.lo()};
      const WordValue hi[3]={x.hi(),y.hi(),result.hi()};
      const int mmin=modulus.min(), mmax=modulus.max();
      const ExecStatus es=prune_product_mod_pass(home,x,y,modulus,result);
      if (es != ES_FIX) return es;
      const bool is_unchanged=(lo[0] == x.lo()) && (hi[0] == x.hi()) &&
        (lo[1] == y.lo()) && (hi[1] == y.hi()) &&
        (lo[2] == result.lo()) && (hi[2] == result.hi()) &&
        (mmin == modulus.min()) && (mmax == modulus.max());
      if (is_unchanged) return ES_FIX;
    }
  }

  forceinline ExecStatus
  ProductMod::post(Home home, WordView x, WordView y,
                   Int::IntView modulus, WordView result) {
    ExecStatus es=prune(home,x,y,modulus,result);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) ProductMod(home,x,y,modulus,result);
    return ES_OK;
  }

  forceinline ExecStatus
  ProductMod::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=prune(home,x,y,modulus,result);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

  /// Query a forced residue without changing domains or classifying the result.
  forceinline KnownResidueResult
  find_known_residue(WordView x, WordView y, Int::IntView modulus) {
    const bool is_unit_modulus=modulus.assigned() && (modulus.val() == 1);
    if (is_unit_modulus) return {true,0};
    const bool has_zero_factor=(x.assigned() && (x.val() == 0)) ||
      (y.assigned() && (y.val() == 0));
    if (has_zero_factor) return {true,0};
    const bool is_assigned=x.assigned() && y.assigned();
    if (is_assigned) {
      if (modulus.assigned())
        return {true,evaluate_product_mod(
          x.val(),y.val(),static_cast<WordValue>(modulus.val()))};
      const WordValue limit=static_cast<WordValue>(modulus.min()-1);
      if (x.val() <= limit/y.val()) return {true,x.val()*y.val()};
    }
    return {false,0};
  }

  /// Reject impossible products before classifying a known residue, if any.
  forceinline Int::RelTest
  test_product_mod(WordView x, WordView y, Int::IntView modulus,
                   WordView result) {
    if (result.lo() >= static_cast<WordValue>(modulus.max()))
      return Int::RT_FALSE;

    if (modulus.assigned()) {
      BoundLocalDomain a=snapshot_product_mod_cube(x);
      BoundLocalDomain b=snapshot_product_mod_cube(y);
      BoundLocalDomain r=result.domain_type() == WDT_UNSIGNED ?
        snapshot_bound_domain(UnsignedWordView(result.varimp())) :
        snapshot_product_mod_cube(result);
      const ProductFilterResult filtered=narrow_product_mod_coefficients(
        a,b,static_cast<WordValue>(modulus.val()),r);
      if (filtered.status < ES_OK) return Int::RT_FALSE;
      if (filtered.is_entailed)
        return result.assigned() ? Int::RT_TRUE : Int::RT_MAYBE;
    }

    const bool has_exact_product=x.assigned() && y.assigned() &&
      result.assigned() && ((y.val() == 0U) || (x.val() <= ~WordValue(0)/y.val()));
    if (has_exact_product) {
      const int minimum=modulus.min(), maximum=modulus.max();
      const DivisorBoundsResult bounds=compute_product_mod_divisors(
        x.val()*y.val(),result.val(),minimum,maximum);
      if (!bounds.is_valid) return Int::RT_FALSE;
      const bool has_missing_singleton=(bounds.minimum == bounds.maximum) &&
        !modulus.in(bounds.minimum);
      if (has_missing_singleton) return Int::RT_FALSE;
    }

    const KnownResidueResult residue=find_known_residue(x,y,modulus);
    if (!residue.has_value) return Int::RT_MAYBE;
    if (!result.in(residue.value)) return Int::RT_FALSE;
    return result.assigned() ? Int::RT_TRUE : Int::RT_MAYBE;
  }

  template<ReifyMode rm>
  forceinline
  ReProductMod<rm>::ReProductMod(Home home, WordView x0, WordView y0,
                                 Int::IntView modulus0, WordView result0,
                                 Int::BoolView b0)
    : Propagator(home), x(x0), y(y0), modulus(modulus0), result(result0),
      b(b0) {
    home.notice(*this,AP_WEAKLY);
    x.subscribe(home,*this,PC_WORD_BITS);
    y.subscribe(home,*this,PC_WORD_BITS);
    modulus.subscribe(home,*this,Int::PC_INT_DOM);
    result.subscribe(home,*this,result.domain_type() == WDT_UNSIGNED ?
                     PC_WORD_DOM : PC_WORD_BITS);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  forceinline
  ReProductMod<rm>::ReProductMod(Space& home, ReProductMod& p)
    : Propagator(home,p) {
    x.update(home,p.x);
    y.update(home,p.y);
    modulus.update(home,p.modulus);
    result.update(home,p.result);
    b.update(home,p.b);
  }

  template<ReifyMode rm>
  forceinline Actor*
  ReProductMod<rm>::copy(Space& home) {
    return new (home) ReProductMod(home,*this);
  }

  template<ReifyMode rm>
  forceinline PropCost
  ReProductMod<rm>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.width());
  }

  template<ReifyMode rm>
  forceinline void
  ReProductMod<rm>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_BITS);
    y.reschedule(home,*this,PC_WORD_BITS);
    modulus.reschedule(home,*this,Int::PC_INT_DOM);
    result.reschedule(home,*this,result.domain_type() == WDT_UNSIGNED ?
                      PC_WORD_DOM : PC_WORD_BITS);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  forceinline size_t
  ReProductMod<rm>::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_BITS);
    y.cancel(home,*this,PC_WORD_BITS);
    modulus.cancel(home,*this,Int::PC_INT_DOM);
    result.cancel(home,*this,result.domain_type() == WDT_UNSIGNED ?
                  PC_WORD_DOM : PC_WORD_BITS);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    home.ignore(*this,AP_WEAKLY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<ReifyMode rm>
  ExecStatus
  ReProductMod<rm>::post(Home home, WordView x, WordView y,
                         Int::IntView modulus, WordView result,
                         Int::BoolView b) {
    GECODE_ME_CHECK(modulus.gq(home,1));
    if (b.one()) {
      if (rm == RM_PMI)
        return ES_OK;
      return post_product_mod(home,x,y,modulus,result);
    }
    const bool is_inactive=b.zero() && (rm == RM_IMP);
    if (is_inactive) return ES_OK;

    switch (test_product_mod(x,y,modulus,result)) {
    case Int::RT_TRUE:
      if (b.zero())
        return ES_FAILED;
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    case Int::RT_FALSE:
      if (b.one())
        return ES_FAILED;
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    case Int::RT_MAYBE:
      (void) new (home) ReProductMod(home,x,y,modulus,result,b);
      return ES_OK;
    default:
      GECODE_NEVER;
    }
    return ES_FAILED;
  }

  template<ReifyMode rm>
  ExecStatus
  ReProductMod<rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,(post_product_mod(
        home(*this),x,y,modulus,result)));
    }
    const bool is_inactive=b.zero() && (rm == RM_IMP);
    if (is_inactive) return home.ES_SUBSUMED(*this);

    switch (test_product_mod(x,y,modulus,result)) {
    case Int::RT_TRUE: {
      if (b.zero()) return ES_FAILED;
      const bool should_set_control=(rm != RM_IMP) && !b.one();
      if (should_set_control) GECODE_ME_CHECK(b.one_none(home));
      break;
    }
    case Int::RT_FALSE: {
      if (b.one()) return ES_FAILED;
      const bool should_set_control=(rm != RM_PMI) && !b.zero();
      if (should_set_control) GECODE_ME_CHECK(b.zero_none(home));
      break;
    }
    case Int::RT_MAYBE:
      return ES_FIX;
    default:
      GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
