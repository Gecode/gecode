/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <cmath>

namespace Gecode { namespace Int { namespace Arithmetic {

  /*
   * Bounds consistent square root
   *
   */

  template<class View>
  forceinline ExecStatus
  prop_sqrt_bnd(Space& home, View x0, View x1) {
    bool mod;
    do {
      mod = false;
      {
        ModEvent me =
          x1.lq(home,floor(::sqrt(static_cast<double>(x0.max()))));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me =
          x1.gq(home,floor(::sqrt(static_cast<double>(x0.min()))));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
        }
      {
        double next = static_cast<double>(x1.max()+1);
        ModEvent me = x0.le(home,next*next);
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.gq(home,x1.min()*x1.min());
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
    } while (mod);
    return ES_OK;
  }

  template<class View>
  forceinline
  SqrtBnd<View>::SqrtBnd(Home home, View x0, View x1)
    : BinaryPropagator<View,PC_INT_BND>(home,x0,x1) {}

  template<class View>
  forceinline ExecStatus
  SqrtBnd<View>::post(Home home, View x0, View x1) {
    GECODE_ME_CHECK(x0.gq(home,0));
    if (same(x0,x1)) {
      GECODE_ME_CHECK(x1.lq(home,1));
    } else {
      GECODE_ME_CHECK(x1.gq(home,0));
      GECODE_ES_CHECK(prop_sqrt_bnd(home,x0,x1));
      (void) new (home) SqrtBnd<View>(home,x0,x1);
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  SqrtBnd<View>::SqrtBnd(Space& home, bool share, SqrtBnd<View>& p)
    : BinaryPropagator<View,PC_INT_BND>(home,share,p) {}

  template<class View>
  Actor*
  SqrtBnd<View>::copy(Space& home, bool share) {
    return new (home) SqrtBnd<View>(home,share,*this);
  }

  template<class View>
  ExecStatus
  SqrtBnd<View>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(prop_sqrt_bnd(home,x0,x1));
    return x1.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Domain consistent square root
   *
   */
  /// Mapping ranges to squares
  class RangesMapSqr {
  public:
    /// Perform mapping of minimum
    forceinline int min(int n) const {
      return n*n;
    }
    /// Perform mapping of maximum
    forceinline int max(int n) const {
      return (n+1)*(n+1)-1;
    }
  };

  /// Mapping integer to square root
  class RangesMapSqrt {
  public:
    /// Perform mapping of minimum
    forceinline int min(int n) const {
      return static_cast<int>(floor(::sqrt(static_cast<double>(n))));
    }
    /// Perform mapping of maximum
    forceinline int max(int n) const {
      return static_cast<int>(floor(::sqrt(static_cast<double>(n))));
    }
 };

  template<class View>
  forceinline
  SqrtDom<View>::SqrtDom(Home home, View x0, View x1)
    : BinaryPropagator<View,PC_INT_DOM>(home,x0,x1) {}

  template<class View>
  forceinline ExecStatus
  SqrtDom<View>::post(Home home, View x0, View x1) {
    GECODE_ME_CHECK(x0.gq(home,0));
    if (same(x0,x1)) {
      GECODE_ME_CHECK(x1.lq(home,1));
    } else {
      GECODE_ME_CHECK(x1.gq(home,0));
      GECODE_ES_CHECK(prop_sqrt_bnd(home,x0,x1));
      (void) new (home) SqrtDom<View>(home,x0,x1);
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  SqrtDom<View>::SqrtDom(Space& home, bool share, SqrtDom<View>& p)
    : BinaryPropagator<View,PC_INT_DOM>(home,share,p) {}

  template<class View>
  Actor*
  SqrtDom<View>::copy(Space& home, bool share) {
    return new (home) SqrtDom<View>(home,share,*this);
  }

  template<class View>
  PropCost
  SqrtDom<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (View::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<class View>
  ExecStatus
  SqrtDom<View>::propagate(Space& home, const ModEventDelta& med) {
    if (View::me(med) != ME_INT_DOM) {
      GECODE_ES_CHECK(prop_sqrt_bnd(home,x0,x1));
      return x1.assigned() ? home.ES_SUBSUMED(*this)
        : home.ES_NOFIX_PARTIAL(*this,View::med(ME_INT_DOM));
    }

    {
      ViewRanges<View> r(x0);
      Iter::Ranges::Map<ViewRanges<View>,RangesMapSqrt,false> m(r);
      GECODE_ME_CHECK(x1.inter_r(home,m,false));
    }

    {
      ViewRanges<View> r(x1);
      Iter::Ranges::Map<ViewRanges<View>,RangesMapSqr,true> m(r);
      GECODE_ME_CHECK(x0.inter_r(home,m,false));
    }

    return x1.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


}}}

// STATISTICS: int-prop

