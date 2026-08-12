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
  product_mod_add(WordValue x, WordValue y, WordValue modulus) {
    return x >= modulus-y ? x-(modulus-y) : x+y;
  }

  forceinline WordValue
  product_mod_value(WordValue x, WordValue y, WordValue modulus) {
    x %= modulus;
    y %= modulus;
    WordValue product=0;
    while (y != 0) {
      if ((y & 1U) != 0)
        product=product_mod_add(product,x,modulus);
      y >>= 1;
      if (y != 0)
        x=product_mod_add(x,x,modulus);
    }
    return product;
  }

  forceinline WordValue
  product_mod_result_hull(unsigned int width, unsigned int maximum) {
    WordValue hull=0;
    while (maximum != 0) {
      hull=(hull << 1) | 1U;
      maximum >>= 1;
    }
    return hull & width_mask(width);
  }

  forceinline bool
  product_mod_single_nonzero(WordView x) {
    return (x.lo() == 0) && (x.hi() != 0) &&
      ((x.hi() & (x.hi()-1)) == 0);
  }

  forceinline
  ProductMod::ProductMod(Home home, WordView x0, WordView y0,
                         Int::IntView modulus0, WordView result0)
    : Propagator(home), x(x0), y(y0), modulus(modulus0), result(result0) {
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
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  forceinline ExecStatus
  ProductMod::prune(Home home, WordView x, WordView y,
                    Int::IntView modulus, WordView result) {
    GECODE_ME_CHECK(modulus.gq(home,1));

    // The cube hull of the integer interval [0,modulus.max()-1].
    const WordValue result_hi=product_mod_result_hull(
      result.width(),static_cast<unsigned int>(modulus.max()-1));
    GECODE_ME_CHECK(result.narrow(home,result.lo(),result.hi()&result_hi));

    if (result.lo() >= static_cast<WordValue>(Int::Limits::max))
      return ES_FAILED;
    GECODE_ME_CHECK(modulus.gq(home,static_cast<int>(result.lo()+1)));

    if (modulus.assigned() && (modulus.val() == 1)) {
      GECODE_ME_CHECK(result.eq(home,0));
      return ES_OK;
    }
    if ((x.assigned() && (x.val() == 0)) ||
        (y.assigned() && (y.val() == 0))) {
      GECODE_ME_CHECK(result.eq(home,0));
      return ES_OK;
    }

    if (result.lo() != 0) {
      if (product_mod_single_nonzero(x))
        GECODE_ME_CHECK(x.eq(home,x.hi()));
      if (product_mod_single_nonzero(y))
        GECODE_ME_CHECK(y.eq(home,y.hi()));
    }

    if (x.assigned() && y.assigned()) {
      if (modulus.assigned()) {
        GECODE_ME_CHECK(result.eq(home,product_mod_value(
          x.val(),y.val(),static_cast<WordValue>(modulus.val()))));
        return ES_OK;
      }
      const WordValue xv=x.val(), yv=y.val();
      const WordValue limit=static_cast<WordValue>(modulus.min()-1);
      if ((yv == 0) || (xv <= limit/yv)) {
        GECODE_ME_CHECK(result.eq(home,xv*yv));
        return ES_OK;
      }
    }
    return ES_FIX;
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

  forceinline Int::RelTest
  product_mod_test(WordView x, WordView y, Int::IntView modulus,
                   WordView result) {
    if (result.lo() >= static_cast<WordValue>(modulus.max()))
      return Int::RT_FALSE;

    WordValue value;
    bool known=false;
    if (modulus.assigned() && (modulus.val() == 1)) {
      value=0;
      known=true;
    } else if ((x.assigned() && (x.val() == 0)) ||
               (y.assigned() && (y.val() == 0))) {
      value=0;
      known=true;
    } else if (x.assigned() && y.assigned()) {
      if (modulus.assigned()) {
        value=product_mod_value(
          x.val(),y.val(),static_cast<WordValue>(modulus.val()));
        known=true;
      } else {
        const WordValue limit=static_cast<WordValue>(modulus.min()-1);
        if (x.val() <= limit/y.val()) {
          value=x.val()*y.val();
          known=true;
        }
      }
    }
    if (!known)
      return Int::RT_MAYBE;
    if (!result.in(value))
      return Int::RT_FALSE;
    return result.assigned() ? Int::RT_TRUE : Int::RT_MAYBE;
  }

  template<ReifyMode rm>
  forceinline
  ReProductMod<rm>::ReProductMod(Home home, WordView x0, WordView y0,
                                 Int::IntView modulus0, WordView result0,
                                 Int::BoolView b0)
    : Propagator(home), x(x0), y(y0), modulus(modulus0), result(result0),
      b(b0) {
    x.subscribe(home,*this,PC_WORD_BITS);
    y.subscribe(home,*this,PC_WORD_BITS);
    modulus.subscribe(home,*this,Int::PC_INT_BND);
    result.subscribe(home,*this,PC_WORD_BITS);
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
    modulus.reschedule(home,*this,Int::PC_INT_BND);
    result.reschedule(home,*this,PC_WORD_BITS);
    b.reschedule(home,*this,Int::PC_BOOL_VAL);
  }

  template<ReifyMode rm>
  forceinline size_t
  ReProductMod<rm>::dispose(Space& home) {
    x.cancel(home,*this,PC_WORD_BITS);
    y.cancel(home,*this,PC_WORD_BITS);
    modulus.cancel(home,*this,Int::PC_INT_BND);
    result.cancel(home,*this,PC_WORD_BITS);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
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
      return ProductMod::post(home,x,y,modulus,result);
    }
    if (b.zero() && (rm == RM_IMP))
      return ES_OK;

    switch (product_mod_test(x,y,modulus,result)) {
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
      GECODE_REWRITE(*this,(ProductMod::post(
        home(*this),x,y,modulus,result)));
    }
    if (b.zero() && (rm == RM_IMP))
      return home.ES_SUBSUMED(*this);

    switch (product_mod_test(x,y,modulus,result)) {
    case Int::RT_TRUE:
      if (b.zero())
        return ES_FAILED;
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one_none(home));
      break;
    case Int::RT_FALSE:
      if (b.one())
        return ES_FAILED;
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero_none(home));
      break;
    case Int::RT_MAYBE:
      return ES_FIX;
    default:
      GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
