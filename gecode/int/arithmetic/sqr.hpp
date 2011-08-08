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
   * Positive bounds consistent squaring
   *
   */
  template<class VA, class VB>
  forceinline ExecStatus
  prop_sqr_plus_bnd(Space& home, VA x0, VB x1) {
    bool mod;
    do {
      mod = false;
      {
        ModEvent me = x0.lq(home,floor(::sqrt(static_cast<double>(x1.max()))));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.gq(home,ceil(::sqrt(static_cast<double>(x1.min()))));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.lq(home,x0.max()*x0.max());
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.gq(home,x0.min()*x0.min());
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
    } while (mod);
    return ES_OK;
  }

  template<class VA, class VB>
  forceinline
  SqrPlusBnd<VA,VB>::SqrPlusBnd(Home home, VA x0, VB x1)
    : MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>(home,x0,x1) {}

  template<class VA, class VB>
  forceinline ExecStatus
  SqrPlusBnd<VA,VB>::post(Home home, VA x0, VB x1) {
    GECODE_ES_CHECK(prop_sqr_plus_bnd(home,x0,x1));
    (void) new (home) SqrPlusBnd<VA,VB>(home,x0,x1);
    return ES_OK;
  }

  template<class VA, class VB>
  forceinline
  SqrPlusBnd<VA,VB>::SqrPlusBnd(Space& home, bool share, SqrPlusBnd<VA,VB>& p)
    : MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>(home,share,p) {}

  template<class VA, class VB>
  Actor*
  SqrPlusBnd<VA,VB>::copy(Space& home, bool share) {
    return new (home) SqrPlusBnd<VA,VB>(home,share,*this);
  }

  template<class VA, class VB>
  ExecStatus
  SqrPlusBnd<VA,VB>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(prop_sqr_plus_bnd(home,x0,x1));
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }



  /*
   * Bounds consistent squaring
   *
   */

  template<class View>
  forceinline
  SqrBnd<View>::SqrBnd(Home home, View x0, View x1)
    : BinaryPropagator<View,PC_INT_BND>(home,x0,x1) {}

  template<class View>
  forceinline ExecStatus
  SqrBnd<View>::post(Home home, View x0, View x1) {
    GECODE_ME_CHECK(x1.gq(home,0));
    if (same(x0,x1)) {
      GECODE_ME_CHECK(x1.lq(home,1));
    } else {
      GECODE_ME_CHECK(x0.lq(home,floor(::sqrt(static_cast<double>
                                              (Limits::max)))));
      GECODE_ME_CHECK(x0.gq(home,-floor(::sqrt(static_cast<double>
                                               (-Limits::min)))));
      if (x0.min() >= 0)
        return SqrPlusBnd<IntView,IntView>::post(home,x0,x1);
      if (x0.max() <= 0)
        return SqrPlusBnd<MinusView,IntView>::post(home,MinusView(x0),x1);
      GECODE_ME_CHECK(x1.lq(home,
                            std::max(x0.min()*x0.min(),x0.max()*x0.max())));
      (void) new (home) SqrBnd<View>(home,x0,x1);
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  SqrBnd<View>::SqrBnd(Space& home, bool share, SqrBnd<View>& p)
    : BinaryPropagator<View,PC_INT_BND>(home,share,p) {}

  template<class View>
  Actor*
  SqrBnd<View>::copy(Space& home, bool share) {
    return new (home) SqrBnd<View>(home,share,*this);
  }

  template<class View>
  ExecStatus
  SqrBnd<View>::propagate(Space& home, const ModEventDelta&) {
    assert(x1.min() >= 0);
    if (x0.min() >= 0)
      GECODE_REWRITE(*this,(SqrPlusBnd<IntView,IntView>::post(home(*this),x0,x1)));
    if (x0.max() <= 0)
      GECODE_REWRITE(*this,(SqrPlusBnd<MinusView,IntView>::post(home(*this),
        MinusView(x0),x1)));

    GECODE_ME_CHECK(x1.lq(home,std::max(x0.min()*x0.min(),
                                        x0.max()*x0.max())));

    int s = static_cast<int>(floor(::sqrt(static_cast<double>(x1.max()))));

    GECODE_ME_CHECK(x0.gq(home,-s));
    GECODE_ME_CHECK(x0.lq(home,s));

    if (x0.assigned() && x1.assigned())
      return (x0.val()*x0.val() == x1.val()) ?
        home.ES_SUBSUMED(*this) : ES_FAILED;

    return ES_NOFIX;
  }

  /*
   * Value mappings for squaring and square root
   *
   */

  /// Mapping integer to square
  class ValuesMapSqr {
  public:
    /// Perform mapping
    forceinline int val(int n) const {
      return n*n;
    }
  };

  /// Mapping integer to square root
  class ValuesMapSqrt {
  public:
    /// Perform mapping
    forceinline int val(int n) const {
      return static_cast<int>(floor(::sqrt(static_cast<double>(n))));
    }
  };


  /*
   * Positive domain consistent squaring
   *
   */
  template<class VA, class VB>
  forceinline
  SqrPlusDom<VA,VB>::SqrPlusDom(Home home, VA x0, VB x1)
    : MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM>(home,x0,x1) {}

  template<class VA, class VB>
  forceinline ExecStatus
  SqrPlusDom<VA,VB>::post(Home home, VA x0, VB x1) {
    GECODE_ES_CHECK(prop_sqr_plus_bnd(home,x0,x1));
    (void) new (home) SqrPlusDom<VA,VB>(home,x0,x1);
    return ES_OK;
  }

  template<class VA, class VB>
  forceinline
  SqrPlusDom<VA,VB>::SqrPlusDom(Space& home, bool share, SqrPlusDom<VA,VB>& p)
    : MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM>(home,share,p) {}

  template<class VA, class VB>
  Actor*
  SqrPlusDom<VA,VB>::copy(Space& home, bool share) {
    return new (home) SqrPlusDom<VA,VB>(home,share,*this);
  }

  template<class VA, class VB>
  PropCost
  SqrPlusDom<VA,VB>::cost(const Space&, const ModEventDelta& med) const {
    if (VA::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (VA::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<class VA, class VB>
  ExecStatus
  SqrPlusDom<VA,VB>::propagate(Space& home, const ModEventDelta& med) {
    if (VA::me(med) != ME_INT_DOM) {
      GECODE_ES_CHECK(prop_sqr_plus_bnd(home,x0,x1));
      return x0.assigned() ?
        home.ES_SUBSUMED(*this)
        : home.ES_NOFIX_PARTIAL(*this,VA::med(ME_INT_DOM));
    }

    {
      ViewValues<VA> v0(x0);
      Iter::Values::Map<ViewValues<VA>,ValuesMapSqr> s0(v0);
      GECODE_ME_CHECK(x1.inter_v(home,s0,false));
    }

    {
      ViewValues<VB> v1(x1);
      Iter::Values::Map<ViewValues<VB>,ValuesMapSqrt> s1(v1);
      GECODE_ME_CHECK(x0.inter_v(home,s1,false));
    }

    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Domain consistent squaring
   *
   */

  template<class View>
  forceinline
  SqrDom<View>::SqrDom(Home home, View x0, View x1)
    : BinaryPropagator<View,PC_INT_DOM>(home,x0,x1) {}

  template<class View>
  forceinline ExecStatus
  SqrDom<View>::post(Home home, View x0, View x1) {
    GECODE_ME_CHECK(x1.gq(home,0));
    if (same(x0,x1)) {
      GECODE_ME_CHECK(x1.lq(home,1));
    } else {
      GECODE_ME_CHECK(x0.lq(home,floor(::sqrt(static_cast<double>
                                              (Limits::max)))));
      GECODE_ME_CHECK(x0.gq(home,-floor(::sqrt(static_cast<double>
                                               (-Limits::min)))));
      if (x0.min() >= 0)
        return SqrPlusDom<IntView,IntView>::post(home,x0,x1);
      if (x0.max() <= 0)
        return SqrPlusDom<MinusView,IntView>::post(home,MinusView(x0),x1);
      GECODE_ME_CHECK(x1.lq(home,
                            std::max(x0.min()*x0.min(),x0.max()*x0.max())));
      (void) new (home) SqrDom<View>(home,x0,x1);
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  SqrDom<View>::SqrDom(Space& home, bool share, SqrDom<View>& p)
    : BinaryPropagator<View,PC_INT_DOM>(home,share,p) {}

  template<class View>
  Actor*
  SqrDom<View>::copy(Space& home, bool share) {
    return new (home) SqrDom<View>(home,share,*this);
  }

  template<class View>
  PropCost
  SqrDom<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_INT_VAL)
      return PropCost::unary(PropCost::LO);
    else if (View::me(med) == ME_INT_DOM)
      return PropCost::binary(PropCost::HI);
    else
      return PropCost::binary(PropCost::LO);
  }

  template<class View>
  ExecStatus
  SqrDom<View>::propagate(Space& home, const ModEventDelta& med) {
    assert(x1.min() >= 0);
    if (View::me(med) != ME_INT_DOM) {
      if (x0.min() >= 0)
        GECODE_REWRITE(*this,(SqrPlusDom<IntView,IntView>::post(home(*this),x0,x1)));
      if (x0.max() <= 0)
        GECODE_REWRITE(*this,(SqrPlusDom<MinusView,IntView>::post(home(*this),
          MinusView(x0),x1)));

      GECODE_ME_CHECK(x1.lq(home,std::max(x0.min()*x0.min(),
                                          x0.max()*x0.max())));

      int s = static_cast<int>(floor(::sqrt(static_cast<double>(x1.max()))));

      GECODE_ME_CHECK(x0.gq(home,-s));
      GECODE_ME_CHECK(x0.lq(home,s));

      if (x0.assigned() && x1.assigned())
        return (x0.val()*x0.val() == x1.val()) ?
          home.ES_SUBSUMED(*this) : ES_FAILED;
      return home.ES_NOFIX_PARTIAL(*this,View::med(ME_INT_DOM));

    }

    Region r(home);
    {
      ViewValues<View> i(x0), j(x0);
      using namespace Iter::Values;
      Positive<ViewValues<View> > p(i);
      Negative<ViewValues<View> > n(j);
      Minus m(r,n);

      Map<Positive<ViewValues<View> >,ValuesMapSqr,true> sp(p);
      Map<Minus,ValuesMapSqr,true> sm(m);
      Union<Map<Positive<ViewValues<View> >,ValuesMapSqr,true>,
        Map<Minus,ValuesMapSqr,true> > u(sp,sm);
      GECODE_ME_CHECK(x1.inter_v(home,u,false));
    }

    {
      ViewValues<View> i(x1), j(x1);
      using namespace Iter::Values;
      Map<ViewValues<View>,ValuesMapSqrt,true> si(i), sj(j);
      Minus mi(r,si);
      Union<Minus,
        Map<ViewValues<View>,ValuesMapSqrt,true> > u(mi,sj);
      GECODE_ME_CHECK(x0.inter_v(home,u,false));
    }

    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: int-prop

