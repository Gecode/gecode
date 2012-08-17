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
   * Positive bounds consistent power
   *
   */
  template<class VA, class VB>
  inline ExecStatus
  prop_pow_plus_bnd(Space& home, VA x0, int n, VB x1) {
    bool mod;
    do {
      mod = false;
      {
        ModEvent me = x0.lq(home,fnroot(x1.max(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.gq(home,cnroot(x1.min(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.lq(home,pow(x0.max(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.gq(home,pow(x0.min(),n));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
    } while (mod);
    return ES_OK;
  }

  template<class VA, class VB>
  forceinline
  PowPlusBnd<VA,VB>::PowPlusBnd(Home home, VA x0, int n0, VB x1)
    : MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>(home,x0,x1), n(n0) {}

  template<class VA, class VB>
  forceinline ExecStatus
  PowPlusBnd<VA,VB>::post(Home home, VA x0, int n, VB x1) {
    GECODE_ME_CHECK(x0.gq(home,0));
    GECODE_ME_CHECK(x1.gq(home,0));
    GECODE_ES_CHECK(prop_pow_plus_bnd(home,x0,n,x1));
    if (!x0.assigned()) {
      assert(!x1.assigned());
      (void) new (home) PowPlusBnd<VA,VB>(home,x0,n,x1);
    }
    return ES_OK;
  }

  template<class VA, class VB>
  forceinline
  PowPlusBnd<VA,VB>::PowPlusBnd(Space& home, bool share, PowPlusBnd<VA,VB>& p)
    : MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>(home,share,p),
      n(p.n) {}

  template<class VA, class VB>
  Actor*
  PowPlusBnd<VA,VB>::copy(Space& home, bool share) {
    return new (home) PowPlusBnd<VA,VB>(home,share,*this);
  }

  template<class VA, class VB>
  ExecStatus
  PowPlusBnd<VA,VB>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(prop_pow_plus_bnd(home,x0,n,x1));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }



  /*
   * Bounds consistent power
   *
   */

  template<bool even>
  inline ExecStatus
  prop_pow_bnd(Space& home, IntView x0, int n, IntView x1) {
    assert((x0.min() < 0) && (0 < x0.max()));
    if (even) {
      assert(x1.min() >= 0);
      int u = fnroot(x1.max(),n);
      GECODE_ME_CHECK(x0.lq(home,u));
      GECODE_ME_CHECK(x0.gq(home,-u));
      GECODE_ME_CHECK(x1.lq(home,std::max(pow(x0.max(),n),
                                          pow(-x0.min(),n))));
    } else {
      assert((x1.min() < 0) && (0 < x1.max()));
      GECODE_ME_CHECK(x0.lq(home,fnroot(x1.max(),n)));
      GECODE_ME_CHECK(x0.gq(home,-fnroot(-x1.min(),n)));
      GECODE_ME_CHECK(x1.lq(home,pow(x0.max(),n)));
      GECODE_ME_CHECK(x1.gq(home,pow(x0.min(),n)));
    }       
    return ES_OK;
  }

  template<bool even>
  forceinline
  PowBnd<even>::PowBnd(Home home, IntView x0, int n0, IntView x1)
    : BinaryPropagator<IntView,PC_INT_BND>(home,x0,x1), n(n0) {}

  template<bool even>
  inline ExecStatus
  PowBnd<even>::post(Home home, IntView x0, int n, IntView x1) {
    if (n >= sizeof(int) * CHAR_BIT) {
      // The integer limits allow only -1, 0, 1 for x0
      GECODE_ME_CHECK(x0.lq(home,1));
      GECODE_ME_CHECK(x0.gq(home,-1));
      // Just rewrite to values that can be handeled without overflow
      n = even ? 2 : 1;
    }

    if (even) {
      if (n == 0) {
        GECODE_ME_CHECK(x1.eq(home,1));
        return ES_OK;
      } else if (n == 2) {
        return SqrBnd<IntView>::post(home,x0,x1);
      }
    } else {
      if (n == 1)
        return Rel::EqBnd<IntView,IntView>::post(home,x0,x1);
    }

    if (same(x0,x1)) {
      assert(n != 0);
      GECODE_ME_CHECK(x0.lq(home,1));
      GECODE_ME_CHECK(x0.gq(home,even ? 0 : -1));
      return ES_OK;
    }

    // Limits values such that no overflow can occur
    assert(Limits::max == -Limits::min);
    {
      int l = fnroot(Limits::max,n);
      GECODE_ME_CHECK(x0.lq(home,l));
      GECODE_ME_CHECK(x0.gq(home,-l));
    }

    if ((x0.min() >= 0) || ((x1.min() >= 0) && !even))
      return PowPlusBnd<IntView,IntView>::post(home,x0,n,x1);

    if (even && (x0.max() <= 0))
      return PowPlusBnd<MinusView,IntView>
        ::post(home,MinusView(x0),n,x1);

    if (!even && ((x0.max() <= 0) || (x1.max() <= 0)))
      return PowPlusBnd<MinusView,MinusView>
        ::post(home,MinusView(x0),n,MinusView(x1));
    
    if (even)
      GECODE_ME_CHECK(x1.gq(home,0));
    
    assert((x0.min() < 0) && (x0.max() > 0));

    if (even) {
      GECODE_ME_CHECK(x1.lq(home,std::max(pow(x0.min(),n),
                                          pow(x0.max(),n))));
    } else {
      GECODE_ME_CHECK(x1.lq(home,pow(x0.max(),n)));
      GECODE_ME_CHECK(x1.gq(home,pow(x0.min(),n)));
    }

    (void) new (home) PowBnd(home,x0,n,x1);
    return ES_OK;
  }

  template<bool even>
  forceinline
  PowBnd<even>::PowBnd(Space& home, bool share, PowBnd& p)
    : BinaryPropagator<IntView,PC_INT_BND>(home,share,p),
      n(p.n) {}

  template<bool even>
  Actor*
  PowBnd<even>::copy(Space& home, bool share) {
    return new (home) PowBnd(home,share,*this);
  }

  template<bool even>
  ExecStatus
  PowBnd<even>::propagate(Space& home, const ModEventDelta&) {
    if ((x0.min() >= 0) || ((x1.min() >= 0) && !even))
      GECODE_REWRITE(*this,(PowPlusBnd<IntView,IntView>
                            ::post(home(*this),x0,n,x1)));
    
    if (even && (x0.max() <= 0))
      GECODE_REWRITE(*this,(PowPlusBnd<MinusView,IntView>
                            ::post(home(*this),MinusView(x0),n,x1)));

    if (!even && ((x0.max() <= 0) || (x1.max() <= 0)))
      GECODE_REWRITE(*this,(PowPlusBnd<MinusView,MinusView>
                            ::post(home(*this),MinusView(x0),n,
                                   MinusView(x1))));

    GECODE_ES_CHECK(prop_pow_bnd<even>(home,x0,n,x1));
    
    if (x0.assigned() && x1.assigned())
      return (pow(x0.val(),n) == x1.val()) ?
        home.ES_SUBSUMED(*this) : ES_FAILED;

    return ES_NOFIX;
  }


  /*
   * Value mappings for power and nroot
   *
   */

  /// Mapping integer to power
  class ValuesMapPow {
  protected:
    /// Exponent
    int n;
  public:
    /// Initialize with exponent \a n0
    forceinline ValuesMapPow(int n0) : n(n0) {}
    /// Perform mapping
    forceinline int val(int x) const {
      return pow(x,n);
    }
  };

  /// Mapping integer (must be an n-th power) to n-th root
  class ValuesMapNroot {
  protected:
    /// Exponent
    int n;
  public:
    /// Initialize with exponent \a n0
    forceinline ValuesMapNroot(int n0) : n(n0) {}
    /// Perform mapping
    forceinline int val(int x) const {
      return fnroot(x,n);
    }
  };

  /// Mapping integer (must be an n-th power) to n-th root (signed)
  class ValuesMapNrootSigned {
  protected:
    /// Exponent
    int n;
  public:
    /// Initialize with exponent \a n0
    forceinline ValuesMapNrootSigned(int n0) : n(n0) {}
    /// Perform mapping
    forceinline int val(int x) const {
      if (x < 0)
        return -fnroot(-x,n);
      else
        return fnroot(x,n);
    }
  };


  /*
   * Positive domain consistent power
   *
   */
  template<class VA, class VB>
  forceinline
  PowPlusDom<VA,VB>::PowPlusDom(Home home, VA x0, int n0, VB x1)
    : MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM>(home,x0,x1), n(n0) {}

  template<class VA, class VB>
  forceinline ExecStatus
  PowPlusDom<VA,VB>::post(Home home, VA x0, int n, VB x1) {
    GECODE_ME_CHECK(x0.gq(home,0));
    GECODE_ME_CHECK(x1.gq(home,0));
    GECODE_ES_CHECK(prop_pow_plus_bnd(home,x0,n,x1));
    if (!x0.assigned()) {
      assert(!x1.assigned());
      (void) new (home) PowPlusDom<VA,VB>(home,x0,n,x1);
    }
    return ES_OK;
  }

  template<class VA, class VB>
  forceinline
  PowPlusDom<VA,VB>::PowPlusDom(Space& home, bool share, PowPlusDom<VA,VB>& p)
    : MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM>(home,share,p),
      n(p.n) {}

  template<class VA, class VB>
  Actor*
  PowPlusDom<VA,VB>::copy(Space& home, bool share) {
    return new (home) PowPlusDom<VA,VB>(home,share,*this);
  }

  template<class VA, class VB>
  PropCost
  PowPlusDom<VA,VB>::cost(const Space&, const ModEventDelta& med) const {
    if (VA::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (VA::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<class VA, class VB>
  ExecStatus
  PowPlusDom<VA,VB>::propagate(Space& home, const ModEventDelta& med) {
    if (VA::me(med) != ME_INT_DOM) {
      GECODE_ES_CHECK(prop_pow_plus_bnd(home,x0,n,x1));
      return x0.assigned() ?
        home.ES_SUBSUMED(*this)
        : home.ES_NOFIX_PARTIAL(*this,VA::med(ME_INT_DOM));
    }

    {
      ViewValues<VA> v0(x0);
      ValuesMapPow vmp(n);
      Iter::Values::Map<ViewValues<VA>,ValuesMapPow> s0(v0,vmp);
      GECODE_ME_CHECK(x1.inter_v(home,s0,false));
    }

    {
      ViewValues<VB> v1(x1);
      ValuesMapNroot vmn(n);
      Iter::Values::Map<ViewValues<VB>,ValuesMapNroot> s1(v1,vmn);
      GECODE_ME_CHECK(x0.inter_v(home,s1,false));
    }

    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Domain consistent power
   *
   */

  template<bool even>
  forceinline
  PowDom<even>::PowDom(Home home, IntView x0, int n0, IntView x1)
    : BinaryPropagator<IntView,PC_INT_DOM>(home,x0,x1), n(n0) {}

  template<bool even>
  inline ExecStatus
  PowDom<even>::post(Home home, IntView x0, int n, IntView x1) {
    if (n >= sizeof(int) * CHAR_BIT) {
      // The integer limits allow only -1, 0, 1 for x0
      GECODE_ME_CHECK(x0.lq(home,1));
      GECODE_ME_CHECK(x0.gq(home,-1));
      // Just rewrite to values that can be handeled without overflow
      n = even ? 2 : 1;
    }

    if (even) {
      if (n == 0) {
        GECODE_ME_CHECK(x1.eq(home,1));
        return ES_OK;
      } else if (n == 2) {
        return SqrDom<IntView>::post(home,x0,x1);
      }
    } else {
      if (n == 1)
        return Rel::EqDom<IntView,IntView>::post(home,x0,x1);
    }

    if (same(x0,x1)) {
      assert(n != 0);
      GECODE_ME_CHECK(x0.lq(home,1));
      GECODE_ME_CHECK(x0.gq(home,even ? 0 : -1));
      return ES_OK;
    }

    // Limits values such that no overflow can occur
    assert(Limits::max == -Limits::min);
    {
      int l = fnroot(Limits::max,n);
      GECODE_ME_CHECK(x0.lq(home,l));
      GECODE_ME_CHECK(x0.gq(home,-l));
    }

    if ((x0.min() >= 0) || ((x1.min() >= 0) && !even))
      return PowPlusDom<IntView,IntView>::post(home,x0,n,x1);

    if (even && (x0.max() <= 0))
      return PowPlusDom<MinusView,IntView>
        ::post(home,MinusView(x0),n,x1);

    if (!even && ((x0.max() <= 0) || (x1.max() <= 0)))
      return PowPlusDom<MinusView,MinusView>
        ::post(home,MinusView(x0),n,MinusView(x1));
    
    if (even)
      GECODE_ME_CHECK(x1.gq(home,0));
    
    assert((x0.min() < 0) && (x0.max() > 0));

    if (even) {
      GECODE_ME_CHECK(x1.lq(home,std::max(pow(x0.min(),n),
                                          pow(x0.max(),n))));
    } else {
      GECODE_ME_CHECK(x1.lq(home,pow(x0.max(),n)));
      GECODE_ME_CHECK(x1.gq(home,pow(x0.min(),n)));
    }

    (void) new (home) PowDom(home,x0,n,x1);
    return ES_OK;
  }

  template<bool even>
  forceinline
  PowDom<even>::PowDom(Space& home, bool share, PowDom& p)
    : BinaryPropagator<IntView,PC_INT_DOM>(home,share,p), n(p.n) {}

  template<bool even>
  Actor*
  PowDom<even>::copy(Space& home, bool share) {
    return new (home) PowDom(home,share,*this);
  }

  template<bool even>
  PropCost
  PowDom<even>::cost(const Space&, const ModEventDelta& med) const {
    if (IntView::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (IntView::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<bool even>
  ExecStatus
  PowDom<even>::propagate(Space& home, const ModEventDelta& med) {
    if (IntView::me(med) != ME_INT_DOM) {
      if ((x0.min() >= 0) || ((x1.min() >= 0) && !even))
        GECODE_REWRITE(*this,(PowPlusDom<IntView,IntView>
                              ::post(home(*this),x0,n,x1)));
    
      if (even && (x0.max() <= 0))
        GECODE_REWRITE(*this,(PowPlusDom<MinusView,IntView>
                              ::post(home(*this),MinusView(x0),n,x1)));
      
      if (!even && ((x0.max() <= 0) || (x1.max() <= 0)))
        GECODE_REWRITE(*this,(PowPlusDom<MinusView,MinusView>
                              ::post(home(*this),MinusView(x0),n,
                                     MinusView(x1))));

      GECODE_ES_CHECK(prop_pow_bnd<even>(home,x0,n,x1));

      if (x0.assigned() && x1.assigned())
        return (pow(x0.val(),n) == x1.val()) ?
          home.ES_SUBSUMED(*this) : ES_FAILED;

      return home.ES_NOFIX_PARTIAL(*this,IntView::med(ME_INT_DOM));
    }

    assert((x0.min() < 0) && (0 < x0.max()));

    Region r(home);
    if (even) {
      ViewValues<IntView> i(x0), j(x0);
      using namespace Iter::Values;
      Positive<ViewValues<IntView> > pos(i);
      Negative<ViewValues<IntView> > neg(j);
      Minus m(r,neg);

      ValuesMapPow vmp(n);
      Map<Positive<ViewValues<IntView> >,ValuesMapPow,true> sp(pos,vmp);
      Map<Minus,ValuesMapPow,true> sm(m,vmp);
      Union<Map<Positive<ViewValues<IntView> >,ValuesMapPow,true>,
        Map<Minus,ValuesMapPow,true> > u(sp,sm);
      GECODE_ME_CHECK(x1.inter_v(home,u,false));
    } else {
      ViewValues<IntView> v0(x0);
      ValuesMapPow vmp(n);
      Iter::Values::Map<ViewValues<IntView>,ValuesMapPow> s0(v0,vmp);
      GECODE_ME_CHECK(x1.inter_v(home,s0,false));
    }

    if (even) {
      ViewValues<IntView> i(x1), j(x1);
      using namespace Iter::Values;
      ValuesMapNroot vmn(n);
      Map<ViewValues<IntView>,ValuesMapNroot,true> si(i,vmn), sj(j,vmn);
      Minus mi(r,si);
      Union<Minus,
        Map<ViewValues<IntView>,ValuesMapNroot,true> > u(mi,sj);
      GECODE_ME_CHECK(x0.inter_v(home,u,false));
    } else {
      ViewValues<IntView> v1(x1);
      ValuesMapNrootSigned vmn(n);
      Iter::Values::Map<ViewValues<IntView>,ValuesMapNrootSigned> s1(v1,vmn);
      GECODE_ME_CHECK(x0.inter_v(home,s1,false));
    }

    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: int-prop

