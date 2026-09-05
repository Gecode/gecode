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

  forceinline WordValue
  number_magnitude(WordValue value, unsigned int width, bool sign) {
    if (!sign || ((value & sign_bit(width)) == 0U))
      return value;
    return (~value+1U) & width_mask(width);
  }

  forceinline WordValue
  number_gcd_value(WordValue x, WordValue y) {
    while (y != 0U) {
      const WordValue r=x%y; x=y; y=r;
    }
    return x;
  }

  template<bool sign>
  forceinline WordValue
  number_gcd(WordValue x, WordValue y, unsigned int width) {
    return number_gcd_value(number_magnitude(x,width,sign),
                            number_magnitude(y,width,sign));
  }

  template<bool sign>
  forceinline bool
  number_divides(WordValue divisor, WordValue dividend,
                 unsigned int width) {
    const WordValue d=number_magnitude(divisor,width,sign);
    const WordValue n=number_magnitude(dividend,width,sign);
    return (d == 0U) ? (n == 0U) : ((n%d) == 0U);
  }

  forceinline WordValue
  number_low_mask(WordValue value) {
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
  number_interval_hull(WordValue maximum) {
    return maximum | low_through_highest(maximum);
  }

  template<bool sign>
  Int::RelTest
  gcd_test(WordView x, WordView y, WordView result) {
    if (result.assigned() && (result.val() == 0U) &&
        (!x.in(0U) || !y.in(0U)))
      return Int::RT_FALSE;
    if (x.assigned() &&
        (number_magnitude(x.val(),x.width(),sign) == 1U)) {
      if (!result.in(1U)) return Int::RT_FALSE;
      if (result.assigned()) return Int::RT_TRUE;
    }
    if (y.assigned() &&
        (number_magnitude(y.val(),y.width(),sign) == 1U)) {
      if (!result.in(1U)) return Int::RT_FALSE;
      if (result.assigned()) return Int::RT_TRUE;
    }
    if (x.assigned() && y.assigned()) {
      const WordValue g=number_gcd<sign>(x.val(),y.val(),x.width());
      if (!result.in(g)) return Int::RT_FALSE;
      return result.assigned() ? Int::RT_TRUE : Int::RT_MAYBE;
    }
    return Int::RT_MAYBE;
  }

  template<bool sign>
  ExecStatus
  gcd_cube_narrow(Home home, WordView x, WordView y, WordView result,
                  bool& subsumed) {
    subsumed=false;
    for (;;) {
      const WordValue old[6]={x.lo(),x.hi(),y.lo(),y.hi(),
                              result.lo(),result.hi()};
      if (!sign) {
        WordValue upper=std::max(x.hi(),y.hi());
        if (!x.in(0U)) upper=std::min(upper,x.hi());
        if (!y.in(0U)) upper=std::min(upper,y.hi());
        GECODE_ME_CHECK(result.narrow(
          home,result.lo(),result.hi()&number_interval_hull(upper)));
      }
      if ((x.assigned() &&
           (number_magnitude(x.val(),x.width(),sign) == 1U)) ||
          (y.assigned() &&
           (number_magnitude(y.val(),y.width(),sign) == 1U))) {
        GECODE_ME_CHECK(result.eq(home,1U));
        subsumed=true; return ES_OK;
      }
      if (result.assigned()) {
        const WordValue g=result.val();
        if (g == 0U) {
          GECODE_ME_CHECK(x.eq(home,0U));
          GECODE_ME_CHECK(y.eq(home,0U));
          subsumed=true; return ES_OK;
        }
        const WordValue low=number_low_mask(g);
        if (low != 0U) {
          GECODE_ME_CHECK(x.narrow(home,x.lo(),x.hi()&~low));
          GECODE_ME_CHECK(y.narrow(home,y.lo(),y.hi()&~low));
        }
        if ((x.assigned() &&
             ((number_magnitude(x.val(),x.width(),sign)%g) != 0U)) ||
            (y.assigned() &&
             ((number_magnitude(y.val(),y.width(),sign)%g) != 0U)))
          return ES_FAILED;
      }
      if (x.assigned() && y.assigned()) {
        GECODE_ME_CHECK(result.eq(
          home,number_gcd<sign>(x.val(),y.val(),x.width())));
        subsumed=true; return ES_OK;
      }
      const WordValue now[6]={x.lo(),x.hi(),y.lo(),y.hi(),
                              result.lo(),result.hi()};
      bool changed=false;
      for (unsigned int i=0; i<6; i++) changed |= old[i] != now[i];
      if (!changed) return ES_OK;
    }
  }

  forceinline bool
  number_local_contains(const BoundLocalDomain& d, WordValue value) {
    return cube_contains(d.lo,d.hi,value,width_mask(d.width)) &&
      (Word::rank(d.kind,d.width,value) >= d.minimum) &&
      (Word::rank(d.kind,d.width,value) <= d.maximum);
  }

  forceinline bool
  number_local_assigned(const BoundLocalDomain& d) {
    return (d.lo == d.hi) && (d.minimum == d.maximum);
  }

  forceinline bool
  number_local_value(BoundLocalDomain& d, WordValue value) {
    const WordValue r=Word::rank(d.kind,d.width,value);
    d.lo |= value; d.hi &= value;
    d.minimum=std::max(d.minimum,r); d.maximum=std::min(d.maximum,r);
    return ((d.lo & ~d.hi) == 0U) && (d.minimum <= d.maximum);
  }

  forceinline bool
  number_local_zero_bits(BoundLocalDomain& d, WordValue bits) {
    d.hi &= ~bits;
    return (d.lo & ~d.hi) == 0U;
  }

  forceinline WordValue
  number_local_magnitude(const BoundLocalDomain& d, WordValue rank) {
    return number_magnitude(Word::rank(d.kind,d.width,rank),d.width,
                            d.kind == WDT_SIGNED);
  }

  forceinline WordValue
  number_local_max_abs(const BoundLocalDomain& d) {
    return std::max(number_local_magnitude(d,d.minimum),
                    number_local_magnitude(d,d.maximum));
  }

  forceinline WordValue
  number_local_min_abs(const BoundLocalDomain& d) {
    if (d.kind != WDT_SIGNED)
      return number_local_magnitude(d,d.minimum);
    if (number_local_contains(d,0U)) return 0U;
    const WordValue m=width_mask(d.width);
    const WordValue sign=sign_bit(d.width);
    WordValue ordered_lo, ordered_hi;
    ordered_cube(d.kind,d.width,d.lo,d.hi,ordered_lo,ordered_hi);
    WordValue minimum=m;
    if (d.minimum < sign) {
      WordValue candidate;
      const WordValue bound=std::min(d.maximum,sign-1U);
      if (cube_predecessor(ordered_lo,ordered_hi,bound,m,candidate) &&
          (candidate >= d.minimum))
        minimum=number_local_magnitude(d,candidate);
    }
    if (d.maximum >= sign) {
      WordValue candidate;
      const WordValue bound=std::max(d.minimum,sign);
      if (cube_successor(ordered_lo,ordered_hi,bound,m,candidate) &&
          (candidate <= d.maximum))
        minimum=std::min(minimum,number_local_magnitude(d,candidate));
    }
    return minimum;
  }

  forceinline bool
  number_unsigned_multiples(BoundLocalDomain& d, WordValue g) {
    return bound_progression(d,0U,g);
  }

  forceinline WordValue
  number_negative_rank(WordValue magnitude, unsigned int width) {
    const WordValue encoded=(~magnitude+1U)&width_mask(width);
    return Word::rank(WDT_SIGNED,width,encoded);
  }

  forceinline bool
  number_signed_multiples(BoundLocalDomain& d, WordValue g) {
    return bound_progression(d,sign_bit(d.width)%g,g);
  }

  template<bool sign>
  forceinline bool
  number_local_multiples(BoundLocalDomain& d, WordValue g) {
    return sign ? number_signed_multiples(d,g) :
      number_unsigned_multiples(d,g);
  }

  forceinline bool
  number_local_equal(BoundLocalDomain& x, BoundLocalDomain& y) {
    const WordValue lo=x.lo|y.lo, hi=x.hi&y.hi;
    const WordValue minimum=std::max(x.minimum,y.minimum);
    const WordValue maximum=std::min(x.maximum,y.maximum);
    x.lo=y.lo=lo; x.hi=y.hi=hi;
    x.minimum=y.minimum=minimum; x.maximum=y.maximum=maximum;
    return ((lo & ~hi) == 0U) && (minimum <= maximum);
  }

  template<bool sign>
  bool
  gcd_local(BoundLocalDomain& x, BoundLocalDomain& y,
            BoundLocalDomain& result, bool& subsumed) {
    subsumed=false;
    if (!sign && (&x == &y)) {
      if (!number_local_equal(x,result)) return false;
      subsumed=(&x == &result) ||
        (number_local_assigned(x) && number_local_assigned(result));
      return true;
    }
    if (number_local_assigned(x) &&
        (number_magnitude(x.lo,x.width,sign) == 1U)) {
      subsumed=true; return number_local_value(result,1U);
    }
    if (number_local_assigned(y) &&
        (number_magnitude(y.lo,y.width,sign) == 1U)) {
      subsumed=true; return number_local_value(result,1U);
    }
    WordValue upper=std::max(number_local_max_abs(x),number_local_max_abs(y));
    if (!number_local_contains(x,0U))
      upper=std::min(upper,number_local_max_abs(x));
    if (!number_local_contains(y,0U))
      upper=std::min(upper,number_local_max_abs(y));
    const WordValue lower=(!number_local_contains(x,0U) ||
                           !number_local_contains(y,0U)) ? 1U : 0U;
    if (!result.range(lower,upper)) return false;

    if (number_local_assigned(result)) {
      const WordValue g=result.lo;
      if (g == 0U) {
        subsumed=true;
        return number_local_value(x,0U) && number_local_value(y,0U);
      }
      const WordValue low=number_low_mask(g);
      if (!number_local_zero_bits(x,low) ||
          !number_local_zero_bits(y,low) ||
          !number_local_multiples<sign>(x,g) ||
          !number_local_multiples<sign>(y,g))
        return false;
    }
    if (number_local_assigned(x) && number_local_assigned(y)) {
      subsumed=true;
      return number_local_value(result,
        number_gcd<sign>(x.lo,y.lo,x.width));
    }
    if (sign && (&x == &y)) {
      if (!result.range(number_local_min_abs(x),number_local_max_abs(x)))
        return false;
    }
    return true;
  }

  template<bool sign>
  Int::RelTest
  divides_test(WordView divisor, WordView dividend) {
    if (divisor == dividend) return Int::RT_TRUE;
    if (dividend.assigned() && (dividend.val() == 0U)) return Int::RT_TRUE;
    if (divisor.assigned()) {
      const WordValue d=number_magnitude(
        divisor.val(),divisor.width(),sign);
      if (d == 1U) return Int::RT_TRUE;
      if ((d == 0U) && !dividend.in(0U)) return Int::RT_FALSE;
      if (dividend.assigned())
        return number_divides<sign>(divisor.val(),dividend.val(),
                                    divisor.width()) ?
          Int::RT_TRUE : Int::RT_FALSE;
    }
    return Int::RT_MAYBE;
  }

  template<bool sign>
  ExecStatus
  divides_cube_narrow(Home home, WordView divisor, WordView dividend,
                      bool& subsumed) {
    subsumed=false;
    if (divisor == dividend) { subsumed=true; return ES_OK; }
    if (dividend.assigned() && (dividend.val() == 0U)) {
      subsumed=true; return ES_OK;
    }
    if (divisor.assigned()) {
      const WordValue d=number_magnitude(
        divisor.val(),divisor.width(),sign);
      if (d == 0U) {
        GECODE_ME_CHECK(dividend.eq(home,0U));
        subsumed=true; return ES_OK;
      }
      if (d == 1U) { subsumed=true; return ES_OK; }
      const WordValue low=number_low_mask(d);
      GECODE_ME_CHECK(dividend.narrow(
        home,dividend.lo(),dividend.hi()&~low));
    }
    if (divisor.assigned() && dividend.assigned()) {
      if (!number_divides<sign>(divisor.val(),dividend.val(),
                                divisor.width()))
        return ES_FAILED;
      subsumed=true;
    }
    return ES_OK;
  }

  template<bool sign>
  bool
  divides_local(BoundLocalDomain& divisor, BoundLocalDomain& dividend,
                bool& subsumed) {
    subsumed=false;
    if (&divisor == &dividend) { subsumed=true; return true; }
    if (number_local_assigned(dividend) && (dividend.lo == 0U)) {
      subsumed=true; return true;
    }
    if (number_local_assigned(divisor)) {
      const WordValue d=number_magnitude(divisor.lo,divisor.width,sign);
      if (d == 0U) {
        subsumed=true; return number_local_value(dividend,0U);
      }
      if (d == 1U) { subsumed=true; return true; }
      if (!number_local_zero_bits(dividend,number_low_mask(d)) ||
          !number_local_multiples<sign>(dividend,d))
        return false;
    }
    if (!number_local_contains(dividend,0U)) {
      if (!sign) {
        if (!divisor.range(1U,divisor.maximum)) return false;
      } else {
        const WordValue zero=sign_bit(divisor.width);
        if (divisor.minimum == zero) {
          if (!divisor.range(zero+1U,divisor.maximum)) return false;
        } else if (divisor.maximum == zero) {
          if (!divisor.range(divisor.minimum,zero-1U)) return false;
        }
      }
    }
    if (number_local_assigned(dividend) && (dividend.lo != 0U)) {
      const WordValue a=number_magnitude(dividend.lo,dividend.width,sign);
      if (!sign) {
        if (!divisor.range(1U,a)) return false;
      } else if (a < sign_bit(divisor.width)) {
        if (!divisor.range(number_negative_rank(a,divisor.width),
                           Word::rank(WDT_SIGNED,divisor.width,a)))
          return false;
      }
    }
    if (number_local_assigned(divisor) && number_local_assigned(dividend)) {
      if (!number_divides<sign>(divisor.lo,dividend.lo,divisor.width))
        return false;
      subsumed=true;
    }
    return true;
  }

  template<bool sign>
  forceinline
  Gcd<sign>::Gcd(Home home, WordView x, WordView y, WordView result)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,x,y,result) {}

  template<bool sign>
  forceinline
  Gcd<sign>::Gcd(Space& home, Gcd& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  template<bool sign>
  ExecStatus
  Gcd<sign>::post(Home home, WordView x, WordView y, WordView result) {
    bool subsumed;
    GECODE_ES_CHECK(gcd_cube_narrow<sign>(home,x,y,result,subsumed));
    if (!subsumed) (void) new (home) Gcd(home,x,y,result);
    return ES_OK;
  }

  template<bool sign>
  Actor*
  Gcd<sign>::copy(Space& home) { return new (home) Gcd(home,*this); }

  template<bool sign>
  PropCost
  Gcd<sign>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<bool sign>
  ExecStatus
  Gcd<sign>::propagate(Space& home, const ModEventDelta&) {
    bool subsumed;
    GECODE_ES_CHECK(gcd_cube_narrow<sign>(home,x0,x1,x2,subsumed));
    return subsumed ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View, bool sign>
  forceinline
  BoundGcd<View,sign>::BoundGcd(Home home, View x0, View y0,
                                UnsignedWordView result0)
    : Propagator(home), x(x0), y(y0), result(result0) {
    x.subscribe(home,*this,PC_WORD_DOM);
    y.subscribe(home,*this,PC_WORD_DOM);
    result.subscribe(home,*this,PC_WORD_DOM);
  }

  template<class View, bool sign>
  forceinline
  BoundGcd<View,sign>::BoundGcd(Space& home, BoundGcd& p)
    : Propagator(home,p) {
    x.update(home,p.x); y.update(home,p.y); result.update(home,p.result);
  }

  template<class View, bool sign>
  ExecStatus
  BoundGcd<View,sign>::narrow(Home home, View x, View y,
                              UnsignedWordView result, bool& bits,
                              bool& subsumed) {
    BoundLocalDomain d[3]={bound_snapshot(x),bound_snapshot(y),
                           bound_snapshot(result)};
    BoundLocalDomain* role[3]={&d[0],&d[1],&d[2]};
    if (x.varimp() == y.varimp()) role[1]=role[0];
    if (x.varimp() == result.varimp()) role[2]=role[0];
    else if (y.varimp() == result.varimp()) role[2]=role[1];
    const WordValue initial_lo[3]={d[0].lo,d[1].lo,d[2].lo};
    const WordValue initial_hi[3]={d[0].hi,d[1].hi,d[2].hi};
    for (;;) {
      const BoundLocalDomain old[3]={d[0],d[1],d[2]};
      for (unsigned int i=0; i<3; i++) d[i].deferred=true;
      if (!gcd_local<sign>(*role[0],*role[1],*role[2],subsumed))
        return ES_FAILED;
      for (unsigned int i=0; i<3; i++) {
        d[i].deferred=false;
        bool distinct=true;
        for (unsigned int j=0; j<i; j++)
          distinct &= role[i] != role[j];
        if (distinct && !role[i]->synchronize()) return ES_FAILED;
      }
      if ((d[0] == old[0]) && (d[1] == old[1]) && (d[2] == old[2]))
        break;
    }
    bits=false;
    for (unsigned int i=0; i<3; i++)
      bits |= (d[i].lo != initial_lo[i]) || (d[i].hi != initial_hi[i]);
    GECODE_ES_CHECK(bound_publish(home,x,d[0]));
    if (y.varimp() != x.varimp()) GECODE_ES_CHECK(bound_publish(home,y,d[1]));
    if ((result.varimp() != x.varimp()) &&
        (result.varimp() != y.varimp()))
      GECODE_ES_CHECK(bound_publish(home,result,d[2]));
    return ES_OK;
  }

  template<class View, bool sign>
  ExecStatus
  BoundGcd<View,sign>::post(Home home, View x, View y,
                            UnsignedWordView result) {
    bool bits, subsumed;
    GECODE_ES_CHECK(narrow(home,x,y,result,bits,subsumed));
    if (!subsumed) (void) new (home) BoundGcd(home,x,y,result);
    return ES_OK;
  }

  template<class View, bool sign>
  Actor*
  BoundGcd<View,sign>::copy(Space& home) {
    return new (home) BoundGcd(home,*this);
  }

  template<class View, bool sign>
  PropCost
  BoundGcd<View,sign>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x.width());
  }

  template<class View, bool sign>
  void
  BoundGcd<View,sign>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_DOM);
    y.reschedule(home,*this,PC_WORD_DOM);
    result.reschedule(home,*this,PC_WORD_DOM);
  }

  template<class View, bool sign>
  size_t
  BoundGcd<View,sign>::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_DOM);
    y.cancel(home,*this,PC_WORD_DOM);
    result.cancel(home,*this,PC_WORD_DOM);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View, bool sign>
  ExecStatus
  BoundGcd<View,sign>::propagate(Space& home, const ModEventDelta&) {
    bool bits, subsumed;
    GECODE_ES_CHECK(narrow(home,x,y,result,bits,subsumed));
    return subsumed ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<bool sign>
  ExecStatus
  post_gcd(Home home, WordView x, WordView y, WordView result) {
    if (!sign && (x.domain_type() == WDT_UNSIGNED) &&
        (y.domain_type() == WDT_UNSIGNED) &&
        (result.domain_type() == WDT_UNSIGNED))
      return BoundGcd<UnsignedWordView,false>::post(
        home,UnsignedWordView(x.varimp()),UnsignedWordView(y.varimp()),
        UnsignedWordView(result.varimp()));
    if (sign && (x.domain_type() == WDT_SIGNED) &&
        (y.domain_type() == WDT_SIGNED) &&
        (result.domain_type() == WDT_UNSIGNED))
      return BoundGcd<SignedWordView,true>::post(
        home,SignedWordView(x.varimp()),SignedWordView(y.varimp()),
        UnsignedWordView(result.varimp()));
    return Gcd<sign>::post(home,x,y,result);
  }

  template<ReifyMode rm, bool sign>
  forceinline
  ReGcd<rm,sign>::ReGcd(Home home, WordView x0, WordView y0,
                        WordView result0, Int::BoolView b0)
    : Propagator(home), x(x0), y(y0), result(result0), b(b0) {
    x.subscribe(home,*this,PC_WORD_BITS);
    y.subscribe(home,*this,PC_WORD_BITS);
    result.subscribe(home,*this,PC_WORD_BITS);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool sign>
  ExecStatus
  ReGcd<rm,sign>::post(Home home, WordView x, WordView y, WordView result,
                       Int::BoolView b) {
    if (b.one()) return (rm == RM_PMI) ? ES_OK :
      post_gcd<sign>(home,x,y,result);
    if (b.zero() && (rm == RM_IMP)) return ES_OK;
    const Int::RelTest rt=gcd_test<sign>(x,y,result);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    }
    (void) new (home) ReGcd(home,x,y,result,b);
    return ES_OK;
  }

  template<ReifyMode rm, bool sign>
  forceinline
  ReGcd<rm,sign>::ReGcd(Space& home, ReGcd& p) : Propagator(home,p) {
    x.update(home,p.x); y.update(home,p.y); result.update(home,p.result);
    b.update(home,p.b);
  }

  template<ReifyMode rm, bool sign>
  Actor*
  ReGcd<rm,sign>::copy(Space& home) { return new (home) ReGcd(home,*this); }

  template<ReifyMode rm, bool sign>
  PropCost
  ReGcd<rm,sign>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::ternary(PropCost::LO);
  }

  template<ReifyMode rm, bool sign>
  void
  ReGcd<rm,sign>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_WORD_BITS);
    y.reschedule(home,*this,PC_WORD_BITS);
    result.reschedule(home,*this,PC_WORD_BITS);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool sign>
  size_t
  ReGcd<rm,sign>::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_BITS); y.cancel(home,*this,PC_WORD_BITS);
    result.cancel(home,*this,PC_WORD_BITS);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home); return sizeof(*this);
  }

  template<ReifyMode rm, bool sign>
  ExecStatus
  ReGcd<rm,sign>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,post_gcd<sign>(home(*this),x,y,result));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      const Int::RelTest rt=gcd_test<sign>(x,y,result);
      if (rt == Int::RT_TRUE) return ES_FAILED;
      return (rt == Int::RT_FALSE) ? home.ES_SUBSUMED(*this) : ES_FIX;
    }
    const Int::RelTest rt=gcd_test<sign>(x,y,result);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return home.ES_SUBSUMED(*this);
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

  template<bool sign>
  forceinline
  CubeDivides<sign>::CubeDivides(Home home, WordView divisor,
                                 WordView dividend)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,divisor,dividend) {}

  template<bool sign>
  forceinline
  CubeDivides<sign>::CubeDivides(Space& home, CubeDivides& p)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  template<bool sign>
  ExecStatus
  CubeDivides<sign>::post(Home home, WordView divisor, WordView dividend) {
    bool subsumed;
    GECODE_ES_CHECK(divides_cube_narrow<sign>(
      home,divisor,dividend,subsumed));
    if (!subsumed) (void) new (home) CubeDivides(home,divisor,dividend);
    return ES_OK;
  }

  template<bool sign>
  Actor*
  CubeDivides<sign>::copy(Space& home) {
    return new (home) CubeDivides(home,*this);
  }

  template<bool sign>
  PropCost
  CubeDivides<sign>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<bool sign>
  ExecStatus
  CubeDivides<sign>::propagate(Space& home, const ModEventDelta&) {
    bool subsumed;
    GECODE_ES_CHECK(divides_cube_narrow<sign>(home,x0,x1,subsumed));
    return subsumed ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View, bool sign>
  forceinline
  Divides<View,sign>::Divides(Home home, View divisor, View dividend)
    : BinaryPropagator<View,PC_WORD_DOM>(home,divisor,dividend) {}

  template<class View, bool sign>
  forceinline
  Divides<View,sign>::Divides(Space& home, Divides& p)
    : BinaryPropagator<View,PC_WORD_DOM>(home,p) {}

  template<class View, bool sign>
  ExecStatus
  Divides<View,sign>::narrow(Home home, View divisor, View dividend,
                             bool& bits, bool& subsumed) {
    BoundLocalDomain d[2]={bound_snapshot(divisor),bound_snapshot(dividend)};
    BoundLocalDomain* role[2]={&d[0],&d[1]};
    if (divisor.varimp() == dividend.varimp()) role[1]=role[0];
    const WordValue initial_lo[2]={d[0].lo,d[1].lo};
    const WordValue initial_hi[2]={d[0].hi,d[1].hi};
    for (;;) {
      const BoundLocalDomain old[2]={d[0],d[1]};
      d[0].deferred=d[1].deferred=true;
      if (!divides_local<sign>(*role[0],*role[1],subsumed))
        return ES_FAILED;
      d[0].deferred=d[1].deferred=false;
      if (!d[0].synchronize()) return ES_FAILED;
      if ((role[1] != role[0]) && !d[1].synchronize()) return ES_FAILED;
      if ((d[0] == old[0]) && (d[1] == old[1])) break;
    }
    bits=(d[0].lo != initial_lo[0]) || (d[0].hi != initial_hi[0]) ||
      (d[1].lo != initial_lo[1]) || (d[1].hi != initial_hi[1]);
    GECODE_ES_CHECK(bound_publish(home,divisor,d[0]));
    if (role[1] != role[0]) GECODE_ES_CHECK(bound_publish(home,dividend,d[1]));
    return ES_OK;
  }

  template<class View, bool sign>
  ExecStatus
  Divides<View,sign>::post(Home home, View divisor, View dividend) {
    bool bits, subsumed;
    GECODE_ES_CHECK(narrow(home,divisor,dividend,bits,subsumed));
    if (!subsumed) (void) new (home) Divides(home,divisor,dividend);
    return ES_OK;
  }

  template<class View, bool sign>
  Actor*
  Divides<View,sign>::copy(Space& home) {
    return new (home) Divides(home,*this);
  }

  template<class View, bool sign>
  PropCost
  Divides<View,sign>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x0.width());
  }

  template<class View, bool sign>
  ExecStatus
  Divides<View,sign>::propagate(Space& home, const ModEventDelta&) {
    bool bits, subsumed;
    GECODE_ES_CHECK(narrow(home,x0,x1,bits,subsumed));
    return subsumed ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<bool sign>
  ExecStatus
  post_divides(Home home, WordView divisor, WordView dividend) {
    const WordDomainType kind=sign ? WDT_SIGNED : WDT_UNSIGNED;
    if ((divisor.domain_type() == kind) &&
        (dividend.domain_type() == kind)) {
      if (sign)
        return Divides<SignedWordView,true>::post(
          home,SignedWordView(divisor.varimp()),
          SignedWordView(dividend.varimp()));
      return Divides<UnsignedWordView,false>::post(
        home,UnsignedWordView(divisor.varimp()),
        UnsignedWordView(dividend.varimp()));
    }
    return CubeDivides<sign>::post(home,divisor,dividend);
  }

  template<ReifyMode rm, bool sign>
  forceinline
  ReDivides<rm,sign>::ReDivides(Home home, WordView divisor0,
                                WordView dividend0, Int::BoolView b0)
    : Propagator(home), divisor(divisor0), dividend(dividend0), b(b0) {
    divisor.subscribe(home,*this,PC_WORD_BITS);
    dividend.subscribe(home,*this,PC_WORD_BITS);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool sign>
  ExecStatus
  ReDivides<rm,sign>::post(Home home, WordView divisor, WordView dividend,
                           Int::BoolView b) {
    if (b.one()) return (rm == RM_PMI) ? ES_OK :
      post_divides<sign>(home,divisor,dividend);
    if (b.zero() && (rm == RM_IMP)) return ES_OK;
    const Int::RelTest rt=divides_test<sign>(divisor,dividend);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return ES_OK;
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return ES_OK;
    }
    (void) new (home) ReDivides(home,divisor,dividend,b);
    return ES_OK;
  }

  template<ReifyMode rm, bool sign>
  forceinline
  ReDivides<rm,sign>::ReDivides(Space& home, ReDivides& p)
    : Propagator(home,p) {
    divisor.update(home,p.divisor); dividend.update(home,p.dividend);
    b.update(home,p.b);
  }

  template<ReifyMode rm, bool sign>
  Actor*
  ReDivides<rm,sign>::copy(Space& home) {
    return new (home) ReDivides(home,*this);
  }

  template<ReifyMode rm, bool sign>
  PropCost
  ReDivides<rm,sign>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<ReifyMode rm, bool sign>
  void
  ReDivides<rm,sign>::reschedule(Space& home) {
    divisor.reschedule(home,*this,PC_WORD_BITS);
    dividend.reschedule(home,*this,PC_WORD_BITS);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm, bool sign>
  size_t
  ReDivides<rm,sign>::dispose(Space& home) {
    divisor.cancel(home,*this,PC_WORD_BITS);
    dividend.cancel(home,*this,PC_WORD_BITS);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home); return sizeof(*this);
  }

  template<ReifyMode rm, bool sign>
  ExecStatus
  ReDivides<rm,sign>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI) return home.ES_SUBSUMED(*this);
      GECODE_REWRITE(*this,post_divides<sign>(home(*this),divisor,dividend));
    }
    if (b.zero()) {
      if (rm == RM_IMP) return home.ES_SUBSUMED(*this);
      const Int::RelTest rt=divides_test<sign>(divisor,dividend);
      if (rt == Int::RT_TRUE) return ES_FAILED;
      return (rt == Int::RT_FALSE) ? home.ES_SUBSUMED(*this) : ES_FIX;
    }
    const Int::RelTest rt=divides_test<sign>(divisor,dividend);
    if (rt == Int::RT_TRUE) {
      if (rm != RM_IMP) GECODE_ME_CHECK(b.one(home));
      return home.ES_SUBSUMED(*this);
    }
    if (rt == Int::RT_FALSE) {
      if (rm != RM_PMI) GECODE_ME_CHECK(b.zero(home));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
