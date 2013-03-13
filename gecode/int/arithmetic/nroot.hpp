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

  template<class Ops>
  forceinline ExecStatus
  prop_nroot_bnd(Space& home, IntView x0, IntView x1, const Ops& ops) {
    bool mod;
    do {
      mod = false;
      {
        ModEvent me = x1.lq(home,ops.fnroot(x0.max()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.gq(home,ops.fnroot(x0.min()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.le(home,ops.tpow(x1.max()+1));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.gq(home,ops.pow(x1.min()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
    } while (mod);
    return ES_OK;
  }

  template<class Ops>
  forceinline
  NrootBnd<Ops>::NrootBnd(Home home, IntView x0, IntView x1, const Ops& o)
    : BinaryPropagator<IntView,PC_INT_BND>(home,x0,x1), 
      ops(o) {
  }

  template<class Ops>
  forceinline ExecStatus
  NrootBnd<Ops>::post(Home home, IntView x0, IntView x1, Ops ops) {
    GECODE_ME_CHECK(x0.gq(home,0));
    GECODE_ME_CHECK(x1.gq(home,0));

    if (static_cast<unsigned int>(ops.exp()) >= sizeof(int) * CHAR_BIT) {
      // The integer limits allow only 0 and 1 for x1
      GECODE_ME_CHECK(x1.lq(home,1));
    }

    if (ops.exp() == 0) {
      GECODE_ME_CHECK(x1.eq(home,1));
      return ES_OK;
    } else if (ops.exp() == 1) {
      return Rel::EqBnd<IntView,IntView>::post(home,x0,x1);      
    }

    if (same(x0,x1)) {
      assert(ops.exp() > 1);
      GECODE_ME_CHECK(x1.lq(home,1));
      return ES_OK;
    } 

    // Limits values such that no overflow can occur
    GECODE_ME_CHECK(x1.lq(home,ops.fnroot(Limits::max)));

    GECODE_ES_CHECK(prop_nroot_bnd<Ops>(home,x0,x1,ops));
    (void) new (home) NrootBnd(home,x0,x1,ops);

    return ES_OK;
  }

  template<class Ops>
  forceinline
  NrootBnd<Ops>::NrootBnd(Space& home, bool share, NrootBnd<Ops>& p)
    : BinaryPropagator<IntView,PC_INT_BND>(home,share,p), 
      ops(p.ops) {}

  template<class Ops>
  Actor*
  NrootBnd<Ops>::copy(Space& home, bool share) {
    return new (home) NrootBnd<Ops>(home,share,*this);
  }

  template<class Ops>
  ExecStatus
  NrootBnd<Ops>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(prop_nroot_bnd(home,x0,x1,ops));
    return x1.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Domain consistent nth root
   *
   */
  /// Mapping ranges to powers
  template<class Ops>
  class RangesMapPow {
  protected:
    /// Power operations
    Ops ops;
  public:
    /// Initialize with operations \a o
    forceinline RangesMapPow(const Ops& o) : ops(o) {}
    /// Perform mapping of minimum
    forceinline int min(int x) const {
      return ops.pow(x);
    }
    /// Perform mapping of maximum
    forceinline int max(int x) const {
      return ops.tpow(x+1)-1;
    }
  };

  /// Mapping integer to n-th root
  template<class Ops>
  class RangesMapNroot {
  protected:
    /// Power operations
    Ops ops;
  public:
    /// Initialize with operations \a o
    forceinline RangesMapNroot(const Ops& o) : ops(o) {}
    /// Perform mapping of minimum
    forceinline int min(int x) const {
      return ops.fnroot(x);
    }
    /// Perform mapping of maximum
    forceinline int max(int x) const {
      return ops.fnroot(x);
    }
 };

  template<class Ops>
  forceinline
  NrootDom<Ops>::NrootDom(Home home, IntView x0, IntView x1, const Ops& o)
    : BinaryPropagator<IntView,PC_INT_DOM>(home,x0,x1),
      ops(o) {}

  template<class Ops>
  forceinline ExecStatus
  NrootDom<Ops>::post(Home home, IntView x0, IntView x1, Ops ops) {
    GECODE_ME_CHECK(x0.gq(home,0));
    GECODE_ME_CHECK(x1.gq(home,0));

    if (static_cast<unsigned int>(ops.exp()) >= sizeof(int) * CHAR_BIT) {
      // The integer limits allow only 0 and 1 for x1
      GECODE_ME_CHECK(x1.lq(home,1));
    }

    if (ops.exp() == 0) {
      GECODE_ME_CHECK(x1.eq(home,1));
      return ES_OK;
    } else if (ops.exp() == 1) {
      return Rel::EqDom<IntView,IntView>::post(home,x0,x1);      
    }

    if (same(x0,x1)) {
      assert(ops.exp() > 1);
      GECODE_ME_CHECK(x1.lq(home,1));
      return ES_OK;
    } 

    // Limits values such that no overflow can occur
    GECODE_ME_CHECK(x1.lq(home,ops.fnroot(Limits::max)));

    GECODE_ES_CHECK(prop_nroot_bnd(home,x0,x1,ops));
    (void) new (home) NrootDom<Ops>(home,x0,x1,ops);

    return ES_OK;
  }

  template<class Ops>
  forceinline
  NrootDom<Ops>::NrootDom(Space& home, bool share, NrootDom<Ops>& p)
    : BinaryPropagator<IntView,PC_INT_DOM>(home,share,p), 
      ops(p.ops) {}

  template<class Ops>
  Actor*
  NrootDom<Ops>::copy(Space& home, bool share) {
    return new (home) NrootDom<Ops>(home,share,*this);
  }

  template<class Ops>
  PropCost
  NrootDom<Ops>::cost(const Space&, const ModEventDelta& med) const {
    if (IntView::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (IntView::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<class Ops>
  ExecStatus
  NrootDom<Ops>::propagate(Space& home, const ModEventDelta& med) {
    if (IntView::me(med) != ME_INT_DOM) {
      GECODE_ES_CHECK(prop_nroot_bnd(home,x0,x1,ops));
      return x1.assigned() ? home.ES_SUBSUMED(*this)
        : home.ES_NOFIX_PARTIAL(*this,IntView::med(ME_INT_DOM));
    }

    {
      ViewRanges<IntView> r(x0);
      RangesMapNroot<Ops> rmn(ops);
      Iter::Ranges::Map<ViewRanges<IntView>,RangesMapNroot<Ops>,false> 
        m(r,rmn);
      GECODE_ME_CHECK(x1.inter_r(home,m,false));
    }

    {
      ViewRanges<IntView> r(x1);
      RangesMapPow<Ops> rmp(ops);
      Iter::Ranges::Map<ViewRanges<IntView>,RangesMapPow<Ops>,true> 
        m(r,rmp);
      GECODE_ME_CHECK(x0.inter_r(home,m,false));
    }

    return x1.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


}}}

// STATISTICS: int-prop

