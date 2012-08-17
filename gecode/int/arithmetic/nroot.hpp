/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <gecode/int/rel.hh>

#include <climits>
#include <algorithm>

namespace Gecode { namespace Int { namespace Arithmetic {

  /*
   * Bounds consistent nth root
   *
   */

  template<class View>
  forceinline ExecStatus
  prop_nroot_bnd(Space& home, View x0, int n, View x1) {
    bool mod;
    do {
      mod = false;
      {
        ModEvent me = x1.lq(home,fnroot(x0.max(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.gq(home,fnroot(x0.min(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.le(home,tpow(x1.max()+1,n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.gq(home,pow(x1.min(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
    } while (mod);
    return ES_OK;
  }

  template<class View>
  forceinline
  NrootBnd<View>::NrootBnd(Home home, View x0, int n0, View x1)
    : BinaryPropagator<View,PC_INT_BND>(home,x0,x1), n(n0) {
  }

  template<class View>
  forceinline ExecStatus
  NrootBnd<View>::post(Home home, View x0, int n, View x1) {
    GECODE_ME_CHECK(x0.gq(home,0));
    GECODE_ME_CHECK(x1.gq(home,0));

    if (n >= sizeof(int) * CHAR_BIT) {
      // The integer limits allow only 0 and 1 for x1
      GECODE_ME_CHECK(x1.lq(home,1));
    }

    if (n == 0) {
      GECODE_ME_CHECK(x1.eq(home,1));
      return ES_OK;
    } else if (n == 1) {
      return Rel::EqBnd<IntView,IntView>::post(home,x0,x1);      
    } else if (n == 2) {
      return SqrtBnd<IntView>::post(home,x0,x1);
    }

    if (same(x0,x1)) {
      assert(n > 1);
      GECODE_ME_CHECK(x1.lq(home,1));
      return ES_OK;
    } 

    // Limits values such that no overflow can occur
    GECODE_ME_CHECK(x1.lq(home,fnroot(Limits::max,n)));

    GECODE_ES_CHECK(prop_nroot_bnd(home,x0,n,x1));
    (void) new (home) NrootBnd<View>(home,x0,n,x1);

    return ES_OK;
  }

  template<class View>
  forceinline
  NrootBnd<View>::NrootBnd(Space& home, bool share, NrootBnd<View>& p)
    : BinaryPropagator<View,PC_INT_BND>(home,share,p), n(p.n) {}

  template<class View>
  Actor*
  NrootBnd<View>::copy(Space& home, bool share) {
    return new (home) NrootBnd<View>(home,share,*this);
  }

  template<class View>
  ExecStatus
  NrootBnd<View>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(prop_nroot_bnd(home,x0,n,x1));
    return x1.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Domain consistent nth root
   *
   */
  /// Mapping ranges to powers
  class RangesMapPow {
  protected:
    /// Exponent
    int n;
  public:
    /// Initialize with exponent \a n0
    forceinline RangesMapPow(int n0) : n(n0) {}
    /// Perform mapping of minimum
    forceinline int min(int x) const {
      return pow(x,n);
    }
    /// Perform mapping of maximum
    forceinline int max(int x) const {
      return tpow(x+1,n)-1;
    }
  };

  /// Mapping integer to n-th root
  class RangesMapNroot {
  protected:
    /// Index of root
    int n;
  public:
    /// Initialize with root index \a n0
    forceinline RangesMapNroot(int n0) : n(n0) {}
    /// Perform mapping of minimum
    forceinline int min(int x) const {
      return fnroot(x,n);
    }
    /// Perform mapping of maximum
    forceinline int max(int x) const {
      return fnroot(x,n);
    }
 };

  template<class View>
  forceinline
  NrootDom<View>::NrootDom(Home home, View x0, int n0, View x1)
    : BinaryPropagator<View,PC_INT_DOM>(home,x0,x1), n(n0) {}

  template<class View>
  forceinline ExecStatus
  NrootDom<View>::post(Home home, View x0, int n, View x1) {
    GECODE_ME_CHECK(x0.gq(home,0));
    GECODE_ME_CHECK(x1.gq(home,0));

    if (n >= sizeof(int) * CHAR_BIT) {
      // The integer limits allow only 0 and 1 for x1
      GECODE_ME_CHECK(x1.lq(home,1));
    }

    if (n == 0) {
      GECODE_ME_CHECK(x1.eq(home,1));
      return ES_OK;
    } else if (n == 1) {
      return Rel::EqDom<IntView,IntView>::post(home,x0,x1);      
    } else if (n == 2) {
      return SqrtDom<IntView>::post(home,x0,x1);
    }

    if (same(x0,x1)) {
      assert(n > 1);
      GECODE_ME_CHECK(x1.lq(home,1));
      return ES_OK;
    } 

    // Limits values such that no overflow can occur
    GECODE_ME_CHECK(x1.lq(home,fnroot(Limits::max,n)));

    GECODE_ES_CHECK(prop_nroot_bnd(home,x0,n,x1));
    (void) new (home) NrootDom<View>(home,x0,n,x1);

    return ES_OK;
  }

  template<class View>
  forceinline
  NrootDom<View>::NrootDom(Space& home, bool share, NrootDom<View>& p)
    : BinaryPropagator<View,PC_INT_DOM>(home,share,p), n(p.n) {}

  template<class View>
  Actor*
  NrootDom<View>::copy(Space& home, bool share) {
    return new (home) NrootDom<View>(home,share,*this);
  }

  template<class View>
  PropCost
  NrootDom<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (View::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<class View>
  ExecStatus
  NrootDom<View>::propagate(Space& home, const ModEventDelta& med) {
    if (View::me(med) != ME_INT_DOM) {
      GECODE_ES_CHECK(prop_nroot_bnd(home,x0,n,x1));
      return x1.assigned() ? home.ES_SUBSUMED(*this)
        : home.ES_NOFIX_PARTIAL(*this,View::med(ME_INT_DOM));
    }

    {
      ViewRanges<View> r(x0);
      RangesMapNroot rmn(n);
      Iter::Ranges::Map<ViewRanges<View>,RangesMapNroot,false> m(r,rmn);
      GECODE_ME_CHECK(x1.inter_r(home,m,false));
    }

    {
      ViewRanges<View> r(x1);
      RangesMapPow rmp(n);
      Iter::Ranges::Map<ViewRanges<View>,RangesMapPow,true> m(r,rmp);
      GECODE_ME_CHECK(x0.inter_r(home,m,false));
    }

    return x1.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


}}}

// STATISTICS: int-prop

