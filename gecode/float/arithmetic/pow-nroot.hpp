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

namespace Gecode { namespace Float { namespace Arithmetic {

  /// Propagator maintaining that a float view does not contain zero
  template<class View>
  class NonZero : public UnaryPropagator<View,PC_FLOAT_BND> {
  protected:
    using UnaryPropagator<View,PC_FLOAT_BND>::x0;
    /// Constructor for cloning a p
    NonZero(Space& home, NonZero<View>& p)
      : UnaryPropagator<View,PC_FLOAT_BND>(home,p) {}
    /// Constructor for posting
    NonZero(Home home, View x)
      : UnaryPropagator<View,PC_FLOAT_BND>(home,x) {}
  public:
    /// Post a non-zero propagator
    static ExecStatus post(Home home, View x) {
      if ((x.max() < 0.0) || (x.min() > 0.0))
        return ES_OK;
      if ((x.min() == 0.0) && (x.max() == 0.0))
        return ES_FAILED;
      (void) new (home) NonZero<View>(home,x);
      return ES_OK;
    }
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home) {
      return new (home) NonZero<View>(home,*this);
    }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      if ((x0.max() < 0.0) || (x0.min() > 0.0))
        return home.ES_SUBSUMED(*this);
      return ((x0.min() == 0.0) && (x0.max() == 0.0))
        ? ES_FAILED : ES_FIX;
    }
  };

  /// Return a downward-rounded positive integer power
  forceinline FloatNum
  positive_pow_down(FloatNum x, int n) {
    typedef gecode_boost::numeric::interval_lib::rounded_arith_std<FloatNum>
      DirectRoundingBase;
    typedef gecode_boost::numeric::interval_lib::save_state<DirectRoundingBase>
      DirectRounding;
    DirectRounding r;
    FloatNum y = 1.0;
    while (n > 0) {
      if ((n & 1) != 0)
        y = r.mul_down(y,x);
      n >>= 1;
      if (n > 0)
        x = r.mul_down(x,x);
    }
    return y;
  }

  /// Return nth root for a non-negative interval
  forceinline FloatVal
  positive_nroot(const FloatVal& x, int n) {
    if (((n % 2) == 0) && (x.min() == 0.0)) {
      if (x.max() == 0.0)
        return FloatVal(0.0,0.0);

      // std::pow is not guaranteed to be correctly rounded.  Use it only as
      // a seed, and certify the upper bound with outward interval arithmetic.
      FloatNum u = std::pow(x.max(), 1.0 / static_cast<FloatNum>(n));
      if (!(u > 0.0))
        u = std::nextafter(0.0,Limits::max);
      else if (u > Limits::max)
        u = Limits::max;

      if (positive_pow_down(u,n) < x.max()) {
        FloatNum l = u;
        do {
          u = (l <= Limits::max / 2.0) ? l * 2.0 : Limits::max;
          if (u > Limits::max)
            u = Limits::max;
          else if (!(u > l))
            u = std::nextafter(l,Limits::max);
          if (u == l)
            break;
          if (positive_pow_down(u,n) < x.max())
            l = u;
          else
            break;
        } while (true);

        // Find the least representable value between the failed lower bound
        // and the certified upper bound.  This keeps the result tight even if
        // the initial approximation was many ulps too small.
        while (std::nextafter(l,u) < u) {
          FloatNum m = l + (u-l) / 2.0;
          if (!(m > l) || !(m < u))
            m = std::nextafter(l,u);
          if (positive_pow_down(m,n) < x.max())
            l = m;
          else
            u = m;
        }
      }
      return FloatVal(0.0,u);
    }
    return nroot(x,n);
  }


  /*
   * Bounds consistent square operator
   *
   */

  template<class A, class B>
  forceinline
  Pow<A,B>::Pow(Home home, A x0, B x1, int n)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1), m_n(n) {}

  template<class A, class B>
  ExecStatus
  Pow<A,B>::post(Home home, A x0, B x1, int n) {
    if (n == 0) {
      GECODE_ME_CHECK(x1.eq(home,1.0));
      return NonZero<A>::post(home,x0);
    }

    GECODE_ME_CHECK(x1.eq(home,pow(x0.domain(),n)));
    if ((x1.min() == 0.0) && (x1.max() == 0.0)) {
      GECODE_ME_CHECK(x0.eq(home,0.0));
      return ES_OK;
    }

    if ((n % 2) == 0) {
      if (x1.min() < 0.0) return ES_FAILED;
      FloatVal d((x1.min() <= 0.0) ? 0.0 : x1.min(), x1.max());
      FloatVal r = positive_nroot(d,n);
      if (x0.min() >= 0)
        GECODE_ME_CHECK(x0.eq(home,r));
      else if (x0.max() <= 0)
        GECODE_ME_CHECK(x0.eq(home,-r));
      else
        GECODE_ME_CHECK(x0.eq(home,
                              hull(
                                  r,
                                  -r
                              )
                        ));
    } else {
      GECODE_ME_CHECK(x0.eq(home,nroot(x1.domain(),n)));
    }

    // Inverse propagation can tighten x0.  Recompute the forward image before
    // an assigned x0 causes the constraint to be discarded.
    GECODE_ME_CHECK(x1.eq(home,pow(x0.domain(),n)));

    if (!x0.assigned()) (void) new (home) Pow<A,B>(home,x0,x1,n);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  Pow<A,B>::Pow(Space& home, Pow<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,p), m_n(p.m_n) {}

  template<class A, class B>
  Actor*
  Pow<A,B>::copy(Space& home) {
    return new (home) Pow<A,B>(home,*this);
  }

  template<class A, class B>
  ExecStatus
  Pow<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,pow(x0.domain(),m_n)));

    if ((x1.min() == 0.0) && (x1.max() == 0.0)) {
      GECODE_ME_CHECK(x0.eq(home,0.0));
      return home.ES_SUBSUMED(*this);
    }

    if ((m_n % 2) == 0) {
      if (x1.min() < 0.0) return ES_FAILED;
      FloatVal d((x1.min() <= 0.0) ? 0.0 : x1.min(), x1.max());
      FloatVal r = positive_nroot(d,m_n);
      if (x0.min() >= 0)
        GECODE_ME_CHECK(x0.eq(home,r));
      else if (x0.max() <= 0)
        GECODE_ME_CHECK(x0.eq(home,-r));
      else
        GECODE_ME_CHECK(x0.eq(home,
                              hull(
                                  r,
                                  -r
                              )
                        ));
    } else {
      GECODE_ME_CHECK(x0.eq(home,nroot(x1.domain(),m_n)));
    }
    GECODE_ME_CHECK(x1.eq(home,pow(x0.domain(),m_n)));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_NOFIX;
  }

  /*
   * Bounds consistent square root operator
   *
   */

  template<class A, class B>
  forceinline
  NthRoot<A,B>::NthRoot(Home home, A x0, B x1, int n)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1), m_n(n) {}

  template<class A, class B>
  ExecStatus
  NthRoot<A,B>::post(Home home, A x0, B x1, int n) {
    if (n == 0) return ES_FAILED;
    GECODE_ME_CHECK(x0.gq(home,0.0));
    if (x0.min() < 0.0) return ES_FAILED;
    FloatVal d((x0.min() <= 0.0) ? 0.0 : x0.min(), x0.max());
    GECODE_ME_CHECK(x1.eq(home,positive_nroot(d,n)));
    GECODE_ME_CHECK(x0.eq(home,pow(x1.domain(),n)));
    (void) new (home) NthRoot<A,B>(home,x0,x1,n);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  NthRoot<A,B>::NthRoot(Space& home, NthRoot<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,p), m_n(p.m_n) {}

  template<class A, class B>
  Actor*
  NthRoot<A,B>::copy(Space& home) {
    return new (home) NthRoot<A,B>(home,*this);
  }

  template<class A, class B>
  ExecStatus
  NthRoot<A,B>::propagate(Space& home, const ModEventDelta&) {
    if (x0.min() < 0.0) return ES_FAILED;
    FloatVal d((x0.min() <= 0.0) ? 0.0 : x0.min(), x0.max());
    GECODE_ME_CHECK(x1.eq(home,positive_nroot(d,m_n)));
    GECODE_ME_CHECK(x0.eq(home,pow(x1.domain(),m_n)));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_NOFIX;
  }


}}}

// STATISTICS: float-prop
