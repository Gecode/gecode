/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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
#include <climits>

#include <gecode/int/support-values.hh>

namespace Gecode { namespace Int { namespace Arithmetic {

  /*
   * Arithmetic help functions
   *
   */

  /// Multiply \a x and \a y as type \a Val
  template<class Val>
  Val m(int x, int y);

  /// Multiply \a x and \a y as type \a Val
  template<class Val>
  Val m(int x, double y);

  template<>
  forceinline double
  m(int x, int y) {
    return static_cast<double>(x)*static_cast<double>(y);
  }

  template<>
  forceinline double
  m(int x, double y) {
    return static_cast<double>(x)*y;
  }

  template<>
  forceinline int
  m(int x, int y) {
    return x*y;
  }

  /// Compute \f$\lceil x/y\rceil\f$ where \a x and \a y are non-negative
  template<class Val>
  int c_d_p(int x, Val y);
  /// Compute \f$\lfloor x/y\rfloor\f$ where \a x and \a y are non-negative
  template<class Val>
  int f_d_p(int x, Val y);

  template<>
  forceinline int
  c_d_p<int>(int x, int y) {
    assert((x >= 0) && (y >= 0));
    return (x+y-1)/y;
  }
  template<>
  forceinline int
  c_d_p<double>(int x, double y) {
    assert((x >= 0) && (y >= 0));
    return static_cast<int>(ceil(static_cast<double>(x) / y));
  }
  template<>
  forceinline int
  f_d_p<int>(int x, int y) {
    assert((x >= 0) && (y >= 0));
    return x/y;
  }
  template<>
  forceinline int
  f_d_p<double>(int x, double y) {
    assert((x >= 0) && (y >= 0));
    return static_cast<int>(floor(static_cast<double>(x) / y));
  }


  /// Compute \f$\lfloor x/y\rfloor\f$
  forceinline int
  f_d(int x, int y) {
    return static_cast<int>(floor(static_cast<double>(x) /
                                  static_cast<double>(y)));
  }

  /// Compute \f$\lceil x/y\rceil\f$
  forceinline int
  c_d(int x, int y) {
    return static_cast<int>(ceil(static_cast<double>(x) /
                                 static_cast<double>(y)));
  }

  /// Test whether \a x is postive
  template<class View>
  forceinline bool
  pos(const View& x) {
    return x.min() > 0;
  }
  /// Test whether \a x is negative
  template<class View>
  forceinline bool
  neg(const View& x) {
    return x.max() < 0;
  }
  /// Test whether \a x is neither positive nor negative
  template<class View>
  forceinline bool
  any(const View& x) {
    return (x.min() <= 0) && (x.max() >= 0);
  }


  /*
   * Propagator for x * y = x
   *
   */

  template<class View, PropCond pc>
  forceinline
  MultZeroOne<View,pc>::MultZeroOne(Home home, View x0, View x1)
    : BinaryPropagator<View,pc>(home,x0,x1) {}

  template<class View, PropCond pc>
  forceinline RelTest
  MultZeroOne<View,pc>::equal(View x, int n) {
    if (pc == PC_INT_DOM) {
      return rtest_eq_dom(x,n);
    } else {
      return rtest_eq_bnd(x,n);
    }
  }

  template<class View, PropCond pc>
  forceinline ExecStatus
  MultZeroOne<View,pc>::post(Home home, View x0, View x1) {
    switch (equal(x0,0)) {
    case RT_FALSE:
      GECODE_ME_CHECK(x1.eq(home,1));
      break;
    case RT_TRUE:
      break;
    case RT_MAYBE:
      switch (equal(x1,1)) {
      case RT_FALSE:
        GECODE_ME_CHECK(x0.eq(home,0));
        break;
      case RT_TRUE:
        break;
      case RT_MAYBE:
        (void) new (home) MultZeroOne<View,pc>(home,x0,x1);
        break;
      default: GECODE_NEVER;
      }
      break;
    default: GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class View, PropCond pc>
  forceinline
  MultZeroOne<View,pc>::MultZeroOne(Space& home, bool share,
                                    MultZeroOne<View,pc>& p)
    : BinaryPropagator<View,pc>(home,share,p) {}

  template<class View, PropCond pc>
  Actor*
  MultZeroOne<View,pc>::copy(Space& home, bool share) {
    return new (home) MultZeroOne<View,pc>(home,share,*this);
  }

  template<class View, PropCond pc>
  ExecStatus
  MultZeroOne<View,pc>::propagate(Space& home, const ModEventDelta&) {
    switch (equal(x0,0)) {
    case RT_FALSE:
      GECODE_ME_CHECK(x1.eq(home,1));
      break;
    case RT_TRUE:
      break;
    case RT_MAYBE:
      switch (equal(x1,1)) {
      case RT_FALSE:
        GECODE_ME_CHECK(x0.eq(home,0));
        break;
      case RT_TRUE:
        break;
      case RT_MAYBE:
        return ES_FIX;
      default: GECODE_NEVER;
      }
      break;
    default: GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
  }


  /*
   * Positive bounds consistent multiplication
   *
   */
  template<class Val, class VA, class VB, class VC>
  forceinline ExecStatus
  prop_mult_plus_bnd(Space& home, Propagator& p, VA x0, VB x1, VC x2) {
    assert(pos(x0) && pos(x1) && pos(x2));
    bool mod;
    do {
      mod = false;
      {
        ModEvent me = x2.lq(home,m<Val>(x0.max(),x1.max()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x2.gq(home,m<Val>(x0.min(),x1.min()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.lq(home,f_d_p<Val>(x2.max(),x1.min()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x0.gq(home,c_d_p<Val>(x2.min(),x1.max()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.lq(home,f_d_p<Val>(x2.max(),x0.min()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
      {
        ModEvent me = x1.gq(home,c_d_p<Val>(x2.min(),x0.max()));
        if (me_failed(me)) return ES_FAILED;
        mod |= me_modified(me);
      }
    } while (mod);
    return x0.assigned() && x1.assigned() ?
      home.ES_SUBSUMED(p) : ES_FIX;
  }

  template<class Val, class VA, class VB, class VC>
  forceinline
  MultPlusBnd<Val,VA,VB,VC>::MultPlusBnd(Home home, VA x0, VB x1, VC x2)
    : MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>
  (home,x0,x1,x2) {}

  template<class Val, class VA, class VB, class VC>
  forceinline
  MultPlusBnd<Val,VA,VB,VC>::MultPlusBnd(Space& home, bool share,
                                   MultPlusBnd<Val,VA,VB,VC>& p)
    : MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>
  (home,share,p) {}

  template<class Val, class VA, class VB, class VC>
  Actor*
  MultPlusBnd<Val,VA,VB,VC>::copy(Space& home, bool share) {
    return new (home) MultPlusBnd<Val,VA,VB,VC>(home,share,*this);
  }

  template<class Val, class VA, class VB, class VC>
  ExecStatus
  MultPlusBnd<Val,VA,VB,VC>::propagate(Space& home, const ModEventDelta&) {
    return prop_mult_plus_bnd<Val,VA,VB,VC>(home,*this,x0,x1,x2);
  }

  template<class Val, class VA, class VB, class VC>
  forceinline ExecStatus
  MultPlusBnd<Val,VA,VB,VC>::post(Home home, VA x0, VB x1, VC x2) {
    GECODE_ME_CHECK(x0.gr(home,0));
    GECODE_ME_CHECK(x1.gr(home,0));
    GECODE_ME_CHECK(x2.gq(home,(static_cast<double>(x0.min()) *
                                static_cast<double>(x1.min()))));
    double u = static_cast<double>(x0.max()) * static_cast<double>(x1.max());
    if (u > INT_MAX) {
      (void) new (home) MultPlusBnd<double,VA,VB,VC>(home,x0,x1,x2);
    } else {
      GECODE_ME_CHECK(x2.lq(home,u));
      (void) new (home) MultPlusBnd<int,VA,VB,VC>(home,x0,x1,x2);
    }
    return ES_OK;
  }


  /*
   * Bounds consistent multiplication
   *
   */
  template<class View>
  forceinline
  MultBnd<View>::MultBnd(Home home, View x0, View x1, View x2)
    : TernaryPropagator<View,PC_INT_BND>(home,x0,x1,x2) {}

  template<class View>
  forceinline
  MultBnd<View>::MultBnd(Space& home, bool share, MultBnd<View>& p)
    : TernaryPropagator<View,PC_INT_BND>(home,share,p) {}

  template<class View>
  Actor*
  MultBnd<View>::copy(Space& home, bool share) {
    return new (home) MultBnd<View>(home,share,*this);
  }

  template<class View>
  ExecStatus
  MultBnd<View>::propagate(Space& home, const ModEventDelta&) {
    if (pos(x0)) {
      if (pos(x1) || pos(x2)) goto rewrite_ppp;
      if (neg(x1) || neg(x2)) goto rewrite_pnn;
      goto prop_pxx;
    }
    if (neg(x0)) {
      if (neg(x1) || pos(x2)) goto rewrite_nnp;
      if (pos(x1) || neg(x2)) goto rewrite_npn;
      goto prop_nxx;
    }
    if (pos(x1)) {
      if (pos(x2)) goto rewrite_ppp;
      if (neg(x2)) goto rewrite_npn;
      goto prop_xpx;
    }
    if (neg(x1)) {
      if (pos(x2)) goto rewrite_nnp;
      if (neg(x2)) goto rewrite_pnn;
      goto prop_xnx;
    }

    assert(any(x0) && any(x1));
    GECODE_ME_CHECK(x2.lq(home,std::max(m<double>(x0.max(),x1.max()),
                                        m<double>(x0.min(),x1.min()))));
    GECODE_ME_CHECK(x2.gq(home,std::min(m<double>(x0.min(),x1.max()),
                                        m<double>(x0.max(),x1.min()))));

    if (x0.assigned()) {
      assert((x0.val() == 0) && (x2.val() == 0));
      return home.ES_SUBSUMED(*this);
    }

    if (x1.assigned()) {
      assert((x1.val() == 0) && (x2.val() == 0));
      return home.ES_SUBSUMED(*this);
    }

    return ES_NOFIX;

  prop_xpx:
    std::swap(x0,x1);
  prop_pxx:
    assert(pos(x0) && any(x1) && any(x2));

    GECODE_ME_CHECK(x2.lq(home,m<double>(x0.max(),x1.max())));
    GECODE_ME_CHECK(x2.gq(home,m<double>(x0.max(),x1.min())));

    if (pos(x2)) goto rewrite_ppp;
    if (neg(x2)) goto rewrite_pnn;

    GECODE_ME_CHECK(x1.lq(home,f_d(x2.max(),x0.min())));
    GECODE_ME_CHECK(x1.gq(home,c_d(x2.min(),x0.min())));

    if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(x2.eq(home,m<double>(x0.val(),x1.val())));
      return home.ES_SUBSUMED(*this);
    }

    return ES_NOFIX;

  prop_xnx:
    std::swap(x0,x1);
  prop_nxx:
    assert(neg(x0) && any(x1) && any(x2));

    GECODE_ME_CHECK(x2.lq(home,m<double>(x0.min(),x1.min())));
    GECODE_ME_CHECK(x2.gq(home,m<double>(x0.min(),x1.max())));

    if (pos(x2)) goto rewrite_nnp;
    if (neg(x2)) goto rewrite_npn;

    GECODE_ME_CHECK(x1.lq(home,f_d(x2.min(),x0.max())));
    GECODE_ME_CHECK(x1.gq(home,c_d(x2.max(),x0.max())));

    if (x0.assigned() && x1.assigned()) {
      GECODE_ME_CHECK(x2.eq(home,m<double>(x0.val(),x1.val())));
      return home.ES_SUBSUMED(*this);
    }

    return ES_NOFIX;

  rewrite_ppp:
    GECODE_REWRITE(*this,(MultPlusBnd<double,IntView,IntView,IntView>
                         ::post(home(*this),x0,x1,x2)));
  rewrite_nnp:
    GECODE_REWRITE(*this,(MultPlusBnd<double,MinusView,MinusView,IntView>
                         ::post(home(*this),MinusView(x0),MinusView(x1),x2)));
  rewrite_pnn:
    std::swap(x0,x1);
  rewrite_npn:
    GECODE_REWRITE(*this,(MultPlusBnd<double,MinusView,IntView,MinusView>
                         ::post(home(*this),MinusView(x0),x1,MinusView(x2))));
  }

  template<class View>
  ExecStatus
  MultBnd<View>::post(Home home, View x0, View x1, View x2) {
    if (same(x0,x1))
      return SqrBnd<View>::post(home,x0,x2);
    if (same(x0,x2))
      return MultZeroOne<View,PC_INT_BND>::post(home,x0,x1);
    if (same(x1,x2))
      return MultZeroOne<View,PC_INT_BND>::post(home,x1,x0);
    if (pos(x0)) {
      if (pos(x1) || pos(x2)) goto post_ppp;
      if (neg(x1) || neg(x2)) goto post_pnn;
    } else if (neg(x0)) {
      if (neg(x1) || pos(x2)) goto post_nnp;
      if (pos(x1) || neg(x2)) goto post_npn;
    } else if (pos(x1)) {
      if (pos(x2)) goto post_ppp;
      if (neg(x2)) goto post_npn;
    } else if (neg(x1)) {
      if (pos(x2)) goto post_nnp;
      if (neg(x2)) goto post_pnn;
    }
    {
      double a =
        static_cast<double>(x0.min()) * static_cast<double>(x1.min());
      double b =
        static_cast<double>(x0.min()) * static_cast<double>(x1.max());
      double c =
        static_cast<double>(x0.max()) * static_cast<double>(x1.min());
      double d =
        static_cast<double>(x0.max()) * static_cast<double>(x1.max());
      GECODE_ME_CHECK(x2.gq(home,std::min(std::min(a,b),std::min(c,d))));
      GECODE_ME_CHECK(x2.lq(home,std::max(std::max(a,b),std::max(c,d))));
      (void) new (home) MultBnd<View>(home,x0,x1,x2);
    }
    return ES_OK;

  post_ppp:
    return MultPlusBnd<double,IntView,IntView,IntView>::post(home,x0,x1,x2);
  post_nnp:
    return MultPlusBnd<double,MinusView,MinusView,IntView>::post(home,
      MinusView(x0),MinusView(x1),x2);
  post_pnn:
    std::swap(x0,x1);
  post_npn:
    return MultPlusBnd<double,MinusView,IntView,MinusView>::post(home,
      MinusView(x0),x1,MinusView(x2));
  }


  /*
   * Positive domain consistent multiplication
   *
   */
  template<class Val, class View>
  forceinline ExecStatus
  prop_mult_dom(Space& home, Propagator& p, View x0, View x1, View x2) {
    Region r(home);
    SupportValues<View,Region> s0(r,x0), s1(r,x1), s2(r,x2);
    while (s0()) {
      while (s1()) {
        if (s2.support(m<Val>(s0.val(),s1.val()))) {
          s0.support(); s1.support();
        }
        ++s1;
      }
      s1.reset(); ++s0;
    }
    GECODE_ME_CHECK(s0.tell(home));
    GECODE_ME_CHECK(s1.tell(home));
    GECODE_ME_CHECK(s2.tell(home));
    return x0.assigned() && x1.assigned() ? home.ES_SUBSUMED(p) : ES_FIX;
  }

  template<class Val, class VA, class VB, class VC>
  forceinline
  MultPlusDom<Val,VA,VB,VC>::MultPlusDom(Home home, VA x0, VB x1, VC x2)
    : MixTernaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM,VC,PC_INT_DOM>
  (home,x0,x1,x2) {}

  template<class Val, class VA, class VB, class VC>
  forceinline
  MultPlusDom<Val,VA,VB,VC>::MultPlusDom(Space& home, bool share,
                                         MultPlusDom<Val,VA,VB,VC>& p)
    : MixTernaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM,VC,PC_INT_DOM>
      (home,share,p) {}

  template<class Val, class VA, class VB, class VC>
  Actor*
  MultPlusDom<Val,VA,VB,VC>::copy(Space& home, bool share) {
    return new (home) MultPlusDom<Val,VA,VB,VC>(home,share,*this);
  }

  template<class Val, class VA, class VB, class VC>
  PropCost
  MultPlusDom<Val,VA,VB,VC>::cost(const Space&,
                                  const ModEventDelta& med) const {
    if (VA::me(med) == ME_INT_DOM)
      return PropCost::ternary(PropCost::HI);
    else
      return PropCost::ternary(PropCost::LO);
  }

  template<class Val, class VA, class VB, class VC>
  ExecStatus
  MultPlusDom<Val,VA,VB,VC>::propagate(Space& home, const ModEventDelta& med) {
    if (VA::me(med) != ME_INT_DOM) {
      GECODE_ES_CHECK((prop_mult_plus_bnd<Val,VA,VB,VC>(home,*this,x0,x1,x2)));
      return home.ES_FIX_PARTIAL(*this,VA::med(ME_INT_DOM));
    }
    IntView y0(x0.varimp()), y1(x1.varimp()), y2(x2.varimp());
    return prop_mult_dom<Val,IntView>(home,*this,y0,y1,y2);
  }

  template<class Val, class VA, class VB, class VC>
  forceinline ExecStatus
  MultPlusDom<Val,VA,VB,VC>::post(Home home, VA x0, VB x1, VC x2) {
    GECODE_ME_CHECK(x0.gr(home,0));
    GECODE_ME_CHECK(x1.gr(home,0));
    GECODE_ME_CHECK(x2.gq(home,(static_cast<double>(x0.min()) *
                                static_cast<double>(x1.min()))));
    double u = static_cast<double>(x0.max()) * static_cast<double>(x1.max());
    if (u > INT_MAX) {
      (void) new (home) MultPlusDom<double,VA,VB,VC>(home,x0,x1,x2);
    } else {
      GECODE_ME_CHECK(x2.lq(home,u));
      (void) new (home) MultPlusDom<int,VA,VB,VC>(home,x0,x1,x2);
    }
    return ES_OK;
  }


  /*
   * Bounds consistent multiplication
   *
   */
  template<class View>
  forceinline
  MultDom<View>::MultDom(Home home, View x0, View x1, View x2)
    : TernaryPropagator<View,PC_INT_DOM>(home,x0,x1,x2) {}

  template<class View>
  forceinline
  MultDom<View>::MultDom(Space& home, bool share, MultDom<View>& p)
    : TernaryPropagator<View,PC_INT_DOM>(home,share,p) {}

  template<class View>
  Actor*
  MultDom<View>::copy(Space& home, bool share) {
    return new (home) MultDom<View>(home,share,*this);
  }

  template<class View>
  PropCost
  MultDom<View>::cost(const Space&, const ModEventDelta& med) const {
    if (View::me(med) == ME_INT_DOM)
      return PropCost::ternary(PropCost::HI);
    else
      return PropCost::ternary(PropCost::LO);
  }

  template<class View>
  ExecStatus
  MultDom<View>::propagate(Space& home, const ModEventDelta& med) {
    if (View::me(med) != ME_INT_DOM) {
      if (pos(x0)) {
        if (pos(x1) || pos(x2)) goto rewrite_ppp;
        if (neg(x1) || neg(x2)) goto rewrite_pnn;
        goto prop_pxx;
      }
      if (neg(x0)) {
        if (neg(x1) || pos(x2)) goto rewrite_nnp;
        if (pos(x1) || neg(x2)) goto rewrite_npn;
        goto prop_nxx;
      }
      if (pos(x1)) {
        if (pos(x2)) goto rewrite_ppp;
        if (neg(x2)) goto rewrite_npn;
        goto prop_xpx;
      }
      if (neg(x1)) {
        if (pos(x2)) goto rewrite_nnp;
        if (neg(x2)) goto rewrite_pnn;
        goto prop_xnx;
      }

      assert(any(x0) && any(x1));
      GECODE_ME_CHECK(x2.lq(home,std::max(m<double>(x0.max(),x1.max()),
                                          m<double>(x0.min(),x1.min()))));
      GECODE_ME_CHECK(x2.gq(home,std::min(m<double>(x0.min(),x1.max()),
                                          m<double>(x0.max(),x1.min()))));

      if (x0.assigned()) {
        assert((x0.val() == 0) && (x2.val() == 0));
        return home.ES_SUBSUMED(*this);
      }

      if (x1.assigned()) {
        assert((x1.val() == 0) && (x2.val() == 0));
        return home.ES_SUBSUMED(*this);
      }

      return home.ES_NOFIX_PARTIAL(*this,View::med(ME_INT_DOM));

    prop_xpx:
      std::swap(x0,x1);
    prop_pxx:
      assert(pos(x0) && any(x1) && any(x2));

      GECODE_ME_CHECK(x2.lq(home,m<double>(x0.max(),x1.max())));
      GECODE_ME_CHECK(x2.gq(home,m<double>(x0.max(),x1.min())));

      if (pos(x2)) goto rewrite_ppp;
      if (neg(x2)) goto rewrite_pnn;

      GECODE_ME_CHECK(x1.lq(home,f_d(x2.max(),x0.min())));
      GECODE_ME_CHECK(x1.gq(home,c_d(x2.min(),x0.min())));

      if (x0.assigned() && x1.assigned()) {
        GECODE_ME_CHECK(x2.eq(home,m<double>(x0.val(),x1.val())));
        return home.ES_SUBSUMED(*this);
      }

      return home.ES_NOFIX_PARTIAL(*this,View::med(ME_INT_DOM));

    prop_xnx:
      std::swap(x0,x1);
    prop_nxx:
      assert(neg(x0) && any(x1) && any(x2));

      GECODE_ME_CHECK(x2.lq(home,m<double>(x0.min(),x1.min())));
      GECODE_ME_CHECK(x2.gq(home,m<double>(x0.min(),x1.max())));

      if (pos(x2)) goto rewrite_nnp;
      if (neg(x2)) goto rewrite_npn;

      GECODE_ME_CHECK(x1.lq(home,f_d(x2.min(),x0.max())));
      GECODE_ME_CHECK(x1.gq(home,c_d(x2.max(),x0.max())));

      if (x0.assigned() && x1.assigned()) {
        GECODE_ME_CHECK(x2.eq(home,m<double>(x0.val(),x1.val())));
        return home.ES_SUBSUMED(*this);
      }

      return home.ES_NOFIX_PARTIAL(*this,View::med(ME_INT_DOM));

    rewrite_ppp:
      GECODE_REWRITE(*this,(MultPlusDom<double,IntView,IntView,IntView>
                           ::post(home(*this),x0,x1,x2)));
    rewrite_nnp:
      GECODE_REWRITE(*this,(MultPlusDom<double,MinusView,MinusView,IntView>
                           ::post(home(*this),
                                  MinusView(x0),MinusView(x1),x2)));
    rewrite_pnn:
      std::swap(x0,x1);
    rewrite_npn:
      GECODE_REWRITE(*this,(MultPlusDom<double,MinusView,IntView,MinusView>
                           ::post(home(*this),
                                  MinusView(x0),x1,MinusView(x2))));
    }
    return prop_mult_dom<double,View>(home,*this,x0,x1,x2);
  }

  template<class View>
  ExecStatus
  MultDom<View>::post(Home home, View x0, View x1, View x2) {
    if (same(x0,x1))
      return SqrDom<View>::post(home,x0,x2);
    if (same(x0,x2))
      return MultZeroOne<View,PC_INT_DOM>::post(home,x0,x1);
    if (same(x1,x2))
      return MultZeroOne<View,PC_INT_DOM>::post(home,x1,x0);
    if (pos(x0)) {
      if (pos(x1) || pos(x2)) goto post_ppp;
      if (neg(x1) || neg(x2)) goto post_pnn;
    } else if (neg(x0)) {
      if (neg(x1) || pos(x2)) goto post_nnp;
      if (pos(x1) || neg(x2)) goto post_npn;
    } else if (pos(x1)) {
      if (pos(x2)) goto post_ppp;
      if (neg(x2)) goto post_npn;
    } else if (neg(x1)) {
      if (pos(x2)) goto post_nnp;
      if (neg(x2)) goto post_pnn;
    }
    {
      double a =
        static_cast<double>(x0.min()) * static_cast<double>(x1.min());
      double b =
        static_cast<double>(x0.min()) * static_cast<double>(x1.max());
      double c =
        static_cast<double>(x0.max()) * static_cast<double>(x1.min());
      double d =
        static_cast<double>(x0.max()) * static_cast<double>(x1.max());
      GECODE_ME_CHECK(x2.gq(home,std::min(std::min(a,b),std::min(c,d))));
      GECODE_ME_CHECK(x2.lq(home,std::max(std::max(a,b),std::max(c,d))));
      (void) new (home) MultDom<View>(home,x0,x1,x2);
    }
    return ES_OK;

  post_ppp:
    return MultPlusDom<double,IntView,IntView,IntView>::post(home,x0,x1,x2);
  post_nnp:
    return MultPlusDom<double,MinusView,MinusView,IntView>::post(home,
      MinusView(x0),MinusView(x1),x2);
  post_pnn:
    std::swap(x0,x1);
  post_npn:
    return MultPlusDom<double,MinusView,IntView,MinusView>::post(home,
      MinusView(x0),x1,MinusView(x2));
  }

}}}

// STATISTICS: int-prop

