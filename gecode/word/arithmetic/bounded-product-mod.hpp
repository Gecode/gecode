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

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline bool
  bound_product_mod_mul(WordValue x, WordValue y, WordValue& z) {
    if ((x != 0U) && (y > (~WordValue(0))/x))
      return false;
    z=x*y;
    return true;
  }

  forceinline WordValue
  bound_product_mod_sat_add(WordValue x, WordValue y) {
    return x > (~WordValue(0))-y ? ~WordValue(0) : x+y;
  }

  forceinline WordValue
  bound_product_mod_ceil_div(WordValue x, WordValue y) {
    assert(y != 0U);
    return x/y + ((x%y) != 0U);
  }

  forceinline WordValue
  bound_product_mod_gcd(WordValue x, WordValue y) {
    while (y != 0U) {
      const WordValue r=x%y; x=y; y=r;
    }
    return x;
  }

  forceinline WordValue
  bound_product_mod_inverse(WordValue a, WordValue m) {
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

  forceinline bool
  bound_product_mod_progression(BoundLocalDomain& x, WordValue residue,
                                WordValue step) {
    if (step == 1U)
      return true;
    WordValue lower;
    if (x.minimum <= residue) {
      lower=residue;
    } else {
      const WordValue rem=(x.minimum-residue)%step;
      const WordValue add=(rem == 0U) ? 0U : step-rem;
      if (x.minimum > (~WordValue(0))-add)
        return false;
      lower=x.minimum+add;
    }
    if ((x.maximum < residue) || (lower > x.maximum))
      return false;
    const WordValue upper=x.maximum-(x.maximum-residue)%step;
    return x.range(lower,upper);
  }

  forceinline bool
  bound_product_mod_inverse_range(BoundLocalDomain& factor,
                                  const BoundLocalDomain& cofactor,
                                  WordValue target_min,
                                  WordValue target_max) {
    if (cofactor.maximum == 0U)
      return target_min == 0U;
    WordValue minimum=bound_product_mod_ceil_div(
      target_min,cofactor.maximum);
    WordValue maximum=factor.maximum;
    if (cofactor.minimum != 0U)
      maximum=std::min(maximum,target_max/cofactor.minimum);
    return factor.range(minimum,maximum);
  }

  forceinline ExecStatus
  bound_product_mod_modulus(Home home, Int::IntView modulus,
                            WordValue minimum, WordValue maximum) {
    if (minimum > static_cast<WordValue>(Int::Limits::max))
      return ES_FAILED;
    GECODE_ME_CHECK(modulus.gq(home,static_cast<int>(minimum)));
    if (maximum < static_cast<WordValue>(Int::Limits::max))
      GECODE_ME_CHECK(modulus.lq(home,static_cast<int>(maximum)));
    return ES_OK;
  }

  forceinline ExecStatus
  bound_product_mod_cube(Home home, BoundLocalView x, BoundLocalView y,
                         Int::IntView modulus, BoundLocalView result,
                         bool& subsumed) {
    const WordValue result_hi=product_mod_result_hull(
      result.width(),static_cast<unsigned int>(modulus.max()-1));
    GECODE_ME_CHECK(result.narrow(home,result.lo(),result.hi()&result_hi));
    if (result.lo() >= static_cast<WordValue>(Int::Limits::max))
      return ES_FAILED;
    GECODE_ME_CHECK(modulus.gq(home,static_cast<int>(result.lo()+1U)));
    if (modulus.assigned() && (modulus.val() == 1)) {
      GECODE_ME_CHECK(result.narrow(home,0U,0U));
      subsumed=true; return ES_OK;
    }
    if ((x.assigned() && (x.val() == 0U)) ||
        (y.assigned() && (y.val() == 0U))) {
      GECODE_ME_CHECK(result.narrow(home,0U,0U));
      subsumed=true; return ES_OK;
    }
    if (result.lo() != 0U) {
      if ((x.lo() == 0U) && (x.hi() != 0U) &&
          ((x.hi() & (x.hi()-1U)) == 0U))
        GECODE_ME_CHECK(x.narrow(home,x.hi(),x.hi()));
      if ((y.lo() == 0U) && (y.hi() != 0U) &&
          ((y.hi() & (y.hi()-1U)) == 0U))
        GECODE_ME_CHECK(y.narrow(home,y.hi(),y.hi()));
    }
    if (x.assigned() && y.assigned()) {
      if (modulus.assigned()) {
        const WordValue value=product_mod_value(
          x.val(),y.val(),static_cast<WordValue>(modulus.val()));
        GECODE_ME_CHECK(result.narrow(home,value,value));
        subsumed=true; return ES_OK;
      }
      WordValue product;
      if (bound_product_mod_mul(x.val(),y.val(),product) &&
          (product < static_cast<WordValue>(modulus.min()))) {
        GECODE_ME_CHECK(result.narrow(home,product,product));
        subsumed=true;
      }
    }
    return ES_OK;
  }

  forceinline ExecStatus
  bound_product_mod_ranges(Home home, BoundLocalDomain& x,
                           BoundLocalDomain& y, Int::IntView modulus,
                           BoundLocalDomain& result, bool& subsumed) {
    WordValue pmin, pmax;
    if (!bound_product_mod_mul(x.minimum,y.minimum,pmin) ||
        !bound_product_mod_mul(x.maximum,y.maximum,pmax))
      return ES_FAILED;
    const WordValue mmax=static_cast<WordValue>(modulus.max());
    WordValue rmax=std::min(pmax,mmax-1U);
    if (!result.range(0U,rmax)) return ES_FAILED;
    GECODE_ES_CHECK(bound_product_mod_modulus(
      home,modulus,result.minimum+1U,Int::Limits::max));

    WordValue mmin=static_cast<WordValue>(modulus.min());
    if (pmax < mmin) {
      if (!result.range(pmin,pmax)) return ES_FAILED;
      if (x.minimum == x.maximum && y.minimum == y.maximum)
        subsumed=true;
    }

    const WordValue current_mmax=static_cast<WordValue>(modulus.max());
    const WordValue kmin=pmin/current_mmax;
    const WordValue kmax=pmax/mmin;
    if (kmin == kmax) {
      const WordValue k=kmin;
      WordValue low_shift, high_shift;
      if (!bound_product_mod_mul(k,current_mmax,low_shift) ||
          !bound_product_mod_mul(k,mmin,high_shift))
        return ES_FAILED;
      if (!result.range(pmin-low_shift,pmax-high_shift))
        return ES_FAILED;
      WordValue target_min, target_max;
      if (!bound_product_mod_mul(k,mmin,target_min)) return ES_FAILED;
      target_min=bound_product_mod_sat_add(target_min,result.minimum);
      if (!bound_product_mod_mul(k,current_mmax,target_max)) return ES_FAILED;
      target_max=bound_product_mod_sat_add(target_max,result.maximum);
      if (!bound_product_mod_inverse_range(x,y,target_min,target_max) ||
          !bound_product_mod_inverse_range(y,x,target_min,target_max))
        return ES_FAILED;
    }

    if (modulus.assigned() && result.minimum == result.maximum) {
      const WordValue m=static_cast<WordValue>(modulus.val());
      int free=0; BoundLocalDomain* factor=nullptr; WordValue cofactor=1U%m;
      if (x.minimum == x.maximum)
        cofactor=(cofactor*(x.minimum%m))%m;
      else { free++; factor=&x; }
      if (y.minimum == y.maximum)
        cofactor=(cofactor*(y.minimum%m))%m;
      else { free++; factor=&y; }
      if (free == 1) {
        const WordValue g=bound_product_mod_gcd(cofactor,m);
        if ((result.minimum%g) != 0U) return ES_FAILED;
        const WordValue step=m/g;
        WordValue residue=0U;
        if (step > 1U) {
          const WordValue a=(cofactor/g)%step;
          const WordValue b=result.minimum/g;
          residue=(bound_product_mod_inverse(a,step)*b)%step;
        }
        if (!bound_product_mod_progression(*factor,residue,step))
          return ES_FAILED;
      }
    }

    if ((x.minimum == x.maximum) && (y.minimum == y.maximum) &&
        (result.minimum == result.maximum)) {
      WordValue p;
      if (!bound_product_mod_mul(x.minimum,y.minimum,p))
        return ES_FAILED;
      const WordValue r=result.minimum;
      if (p == r) {
        if (p >= static_cast<WordValue>(Int::Limits::max))
          return ES_FAILED;
        GECODE_ES_CHECK(bound_product_mod_modulus(
          home,modulus,p+1U,Int::Limits::max));
        subsumed=true;
      } else {
        GECODE_ES_CHECK(bound_product_mod_modulus(home,modulus,r+1U,p-r));
      }
    }
    if (modulus.assigned() && (x.minimum == x.maximum) &&
        (y.minimum == y.maximum)) {
      const WordValue value=product_mod_value(
        x.minimum,y.minimum,static_cast<WordValue>(modulus.val()));
      if (!result.range(value,value)) return ES_FAILED;
      subsumed=true;
    }
    return ES_OK;
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
  BoundProductMod::cost(const Space&, const ModEventDelta& med) const {
    return (UnsignedWordView::me(med) == ME_WORD_BND) ?
      PropCost::ternary(PropCost::LO) :
      PropCost::linear(PropCost::HI,x.width());
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

  inline ExecStatus
  BoundProductMod::narrow(Home home, UnsignedWordView x,
                          UnsignedWordView y, Int::IntView modulus,
                          UnsignedWordView result, bool cube,
                          bool& bits, bool& subsumed) {
    GECODE_ME_CHECK(modulus.gq(home,1));
    BoundLocalDomain d[3]; BoundLocalView v[3];
    bound_alias_domains(x,y,result,d,v);
    BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
    if (x.varimp() == y.varimp()) role[1]=role[0];
    if (x.varimp() == result.varimp()) role[2]=role[0];
    else if (y.varimp() == result.varimp()) role[2]=role[1];
    const WordValue initial_lo[3]={d[0].lo,d[1].lo,d[2].lo};
    const WordValue initial_hi[3]={d[0].hi,d[1].hi,d[2].hi};
    subsumed=false;
    for (;;) {
      const BoundLocalDomain old[3]={d[0],d[1],d[2]};
      const int old_mmin=modulus.min(), old_mmax=modulus.max();
      for (unsigned int i=0; i<3; i++) d[i].deferred=true;
      if (cube)
        GECODE_ES_CHECK(bound_product_mod_cube(
          home,v[0],v[1],modulus,v[2],subsumed));
      GECODE_ES_CHECK(bound_product_mod_ranges(
        home,*role[0],*role[1],modulus,*role[2],subsumed));
      bool cube_again=false;
      for (unsigned int i=0; i<3; i++) {
        d[i].deferred=false;
        bool distinct=true;
        for (unsigned int j=0; j<i; j++)
          if (role[i] == role[j]) { distinct=false; break; }
        if (distinct) {
          const WordValue before_lo=role[i]->lo, before_hi=role[i]->hi;
          if (!role[i]->synchronize()) return ES_FAILED;
          cube_again |= (role[i]->lo != before_lo) ||
            (role[i]->hi != before_hi);
        }
      }
      const bool changed=!(d[0] == old[0]) || !(d[1] == old[1]) ||
        !(d[2] == old[2]) || (modulus.min() != old_mmin) ||
        (modulus.max() != old_mmax);
      cube = cube && cube_again;
      if (!changed) break;
    }
    bits=false;
    for (unsigned int i=0; i<3; i++)
      bits |= (d[i].lo != initial_lo[i]) || (d[i].hi != initial_hi[i]);
    return bound_publish_distinct(home,x,y,result,d);
  }

  forceinline bool
  BoundProductMod::numeric_regime(UnsignedWordView x, UnsignedWordView y) {
    return (x.rank_maximum() == 0U) ||
      (y.rank_maximum() <= x.mask()/x.rank_maximum());
  }

  inline ExecStatus
  BoundProductMod::post(Home home, UnsignedWordView x, UnsignedWordView y,
                        Int::IntView modulus, UnsignedWordView result) {
    bool bits, subsumed;
    GECODE_ES_CHECK(narrow(home,x,y,modulus,result,true,bits,subsumed));
    if (!subsumed)
      (void) new (home) BoundProductMod(home,x,y,modulus,result);
    return ES_OK;
  }

  inline ExecStatus
  BoundProductMod::propagate(Space& home, const ModEventDelta& med) {
    bool bits, subsumed;
    const bool bounds=UnsignedWordView::me(med) == ME_WORD_BND;
    GECODE_ES_CHECK(narrow(home,x,y,modulus,result,!bounds,bits,subsumed));
    if (subsumed) return home.ES_SUBSUMED(*this);
    if (bounds && bits)
      return home.ES_NOFIX_PARTIAL(*this,
                                   UnsignedWordView::med(ME_WORD_BITS));
    return ES_FIX;
  }

  inline ExecStatus
  post_product_mod(Home home, WordView x, WordView y,
                   Int::IntView modulus, WordView result) {
    if ((x.domain_type() == WDT_UNSIGNED) &&
        (y.domain_type() == WDT_UNSIGNED) &&
        (result.domain_type() == WDT_UNSIGNED)) {
      UnsignedWordView bx(x.varimp()), by(y.varimp()), br(result.varimp());
      if (BoundProductMod::numeric_regime(bx,by))
        return BoundProductMod::post(home,bx,by,modulus,br);
    }
    return ProductMod::post(home,x,y,modulus,result);
  }

}}}

#endif

// STATISTICS: word-prop
