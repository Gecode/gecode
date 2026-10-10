/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2012
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
 *
 */

#include <mpfr.h>

namespace Gecode { namespace Float { namespace Transcendental {

  // Use explicit MPFR rounding here: compiler assumptions about the hardware
  // rounding mode must not collapse the bounds of a fixed-base power.
  forceinline FloatVal
  fixed_base_pow(FloatNum base, const FloatVal& exponent) {
    mpfr_t b, x, y;
    mpfr_init2(b, std::numeric_limits<FloatNum>::digits);
    mpfr_init2(x, std::numeric_limits<FloatNum>::digits);
    mpfr_init2(y, std::numeric_limits<FloatNum>::digits);
    mpfr_set_d(b, base, GMP_RNDN);
    mpfr_set_d(x, base > 1.0 ? exponent.min() : exponent.max(), GMP_RNDN);
    mpfr_pow(y, b, x, GMP_RNDD);
    FloatNum lower = mpfr_get_d(y, GMP_RNDD);
    mpfr_set_d(x, base > 1.0 ? exponent.max() : exponent.min(), GMP_RNDN);
    mpfr_pow(y, b, x, GMP_RNDU);
    FloatNum upper = mpfr_get_d(y, GMP_RNDU);
    mpfr_clear(b);
    mpfr_clear(x);
    mpfr_clear(y);
    return FloatVal(lower, upper);
  }

  forceinline FloatVal
  fixed_base_log(FloatNum base, const FloatVal& value) {
    FloatVal numerator = log(value);
    FloatVal denominator = log(FloatVal(base));
    const FloatNum n[] = {numerator.min(), numerator.max()};
    const FloatNum d[] = {denominator.min(), denominator.max()};
    FloatNum lower = std::numeric_limits<FloatNum>::infinity();
    FloatNum upper = -lower;
    mpfr_t x, y, q;
    mpfr_init2(x, std::numeric_limits<FloatNum>::digits);
    mpfr_init2(y, std::numeric_limits<FloatNum>::digits);
    mpfr_init2(q, std::numeric_limits<FloatNum>::digits);
    // The base is positive and different from one, so log(base) has a fixed
    // nonzero sign. All endpoint quotients enclose either increasing or
    // decreasing logarithms without hardware-rounded interval division.
    for (int i=0; i<2; i++) {
      mpfr_set_d(x, n[i], GMP_RNDN);
      for (int j=0; j<2; j++) {
        mpfr_set_d(y, d[j], GMP_RNDN);
        mpfr_div(q, x, y, GMP_RNDD);
        lower = std::min(lower, mpfr_get_d(q, GMP_RNDD));
        mpfr_div(q, x, y, GMP_RNDU);
        upper = std::max(upper, mpfr_get_d(q, GMP_RNDU));
      }
    }
    mpfr_clear(x);
    mpfr_clear(y);
    mpfr_clear(q);
    return FloatVal(lower, upper);
  }

  /*
   * Bounds consistent exponential operator
   *
   */

  template<class A, class B>
  forceinline
  Exp<A,B>::Exp(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Exp<A,B>::post(Home home, A x0, B x1) {
    if (x0 == x1) {
      return ES_FAILED;
    } else {
      GECODE_ME_CHECK(x1.gq(home,0.0));
    }
    GECODE_ME_CHECK(x1.eq(home,exp(x0.domain())));
    if (x1.max() == 0.0)
      return ES_FAILED;
    GECODE_ME_CHECK(x0.eq(home,log(x1.domain())));
    (void) new (home) Exp<A,B>(home,x0,x1);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  Exp<A,B>::Exp(Space& home, Exp<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,p) {}

  template<class A, class B>
  Actor*
  Exp<A,B>::copy(Space& home) {
    return new (home) Exp<A,B>(home,*this);
  }

  template<class A, class B>
  ExecStatus
  Exp<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,exp(x0.domain())));
    if (x1.max() == 0.0)
      return ES_FAILED;
    GECODE_ME_CHECK(x0.eq(home,log(x1.domain())));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Bounds consistent logarithm operator with base
   *
   */

  template<class A, class B>
  forceinline
  Pow<A,B>::Pow(Home home, FloatNum base0, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1),
      base(base0) {}

  template<class A, class B>
  ExecStatus
  Pow<A,B>::post(Home home, FloatNum base, A x0, B x1) {
    if (base <= 0) return ES_FAILED;
    if (base == 1.0) {
      GECODE_ME_CHECK(x1.eq(home,1.0));
      return ES_OK;
    }
    GECODE_ME_CHECK(x1.gq(home,0.0));
    if (x1.max() == 0.0)
      return ES_FAILED;
    GECODE_ME_CHECK(x0.eq(home,fixed_base_log(base,x1.domain())));
    GECODE_ME_CHECK(x1.eq(home,fixed_base_pow(base,x0.domain())));
    (void) new (home) Pow<A,B>(home,base,x0,x1);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  Pow<A,B>::Pow(Space& home, Pow<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,p),
      base(p.base) {}

  template<class A, class B>
  Actor*
  Pow<A,B>::copy(Space& home) {
    return new (home) Pow<A,B>(home,*this);
  }

  template<class A, class B>
  ExecStatus
  Pow<A,B>::propagate(Space& home, const ModEventDelta&) {
    if (x1.max() == 0.0)
      return ES_FAILED;
    GECODE_ME_CHECK(x0.eq(home,fixed_base_log(base,x1.domain())));
    GECODE_ME_CHECK(x1.eq(home,fixed_base_pow(base,x0.domain())));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_NOFIX;
  }

}}}

// STATISTICS: float-prop

