/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Vincent Barichard, 2012
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

#include <algorithm>
#include <climits>

#include <gecode/float/linear.hh>

namespace Gecode { namespace Float { namespace Linear {

  void
  estimate(Term* t, int n, FloatVal c, FloatNum& l, FloatNum &u) {
    FloatVal est = c;
    for (int i=n; i--; ) 
      est += t[i].a * t[i].x.domain();
    FloatNum min = est.min();
    FloatNum max = est.max();
    if (min < Limits::min)
      min = Limits::min;
    if (min > Limits::max)
      min = Limits::max;
    l = min;
    if (max < Limits::min)
      max = Limits::min;
    if (max > Limits::max)
      max = Limits::max;
    u = max;
  }

  /// Sort linear terms by view
  class TermLess {
  public:
    forceinline bool
    operator ()(const Term& a, const Term& b) {
      return before(a.x,b.x);
    }
  };

  bool
  normalize(Term* t, int &n,
            Term* &t_p, int &n_p, Term* &t_n, int &n_n) {
    /*
     * Join coefficients for aliased variables:
     *
     */
    {
      // Group same variables
      TermLess tl;
      Support::quicksort<Term,TermLess>(t,n,tl);

      // Join adjacent variables
      int i = 0;
      int j = 0;
      while (i < n) {
        Limits::check(t[i].a,"Float::linear");
        FloatVal a = t[i].a;
        FloatView x = t[i].x;
        while ((++i < n) && same(t[i].x,x)) {
          a += t[i].a;
          Limits::check(a,"Float::linear");
        }
        if (a != 0.0) {
          t[j].a = a; t[j].x = x; j++;
        }
      }
      n = j;
    }

    /*
     * Partition into positive/negative coefficents
     *
     */
    if (n > 0) {
      int i = 0;
      int j = n-1;
      while (true) {
        while ((t[j].a < 0) && (--j >= 0)) ;
        while ((t[i].a > 0) && (++i <  n)) ;
        if (j <= i) break;
        std::swap(t[i],t[j]);
      }
      t_p = t;     n_p = i;
      t_n = t+n_p; n_n = n-n_p;
    } else {
      t_p = t; n_p = 0;
      t_n = t; n_n = 0;
    }

    /*
     * Make all coefficients positive
     *
     */
    for (int i=n_n; i--; )
      t_n[i].a = -t_n[i].a;

    /*
     * Test for unit coefficients only
     *
     */
    for (int i=n; i--; )
      if (t[i].a != 1)
        return false;
    return true;
  }

  /// Eliminate assigned views
  inline void
  eliminate(Term* t, int &n, FloatVal& d) {
    for (int i=n; i--; )
      if (t[i].x.assigned()) {
        d -= t[i].a * t[i].x.val();
        t[i]=t[--n];
      }
    if ((d < Limits::min) || (d > Limits::max))
      throw OutOfLimits("Float::linear");
  }

  /// Rewrite all inequations in terms of FRT_LQ
  inline void
  rewrite(FloatRelType &frt, FloatVal &d,
          Term* &t_p, int &n_p,
          Term* &t_n, int &n_n) {
    switch (frt) {
    case FRT_EQ: case FRT_LQ:
      break;
    case FRT_GQ:
      frt = FRT_LQ;
      std::swap(n_p,n_n); std::swap(t_p,t_n); d = -d;
      break;
    default:
      throw UnknownRelation("Float::linear");
    }
  }


  /**
   * \brief Posting n-ary propagators
   *
   */
  template<class View>
  forceinline void
  post_nary(Home home,
            ViewArray<View>& x, ViewArray<View>& y, FloatRelType frt, 
            FloatVal c) {
    switch (frt) {
    case FRT_EQ:
      GECODE_ES_FAIL((Eq<View,View >::post(home,x,y,c)));
      break;
    case FRT_LQ:
      GECODE_ES_FAIL((Lq<View,View >::post(home,x,y,c)));
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post(Home home, Term* t, int n, FloatRelType frt, FloatVal c) {

    Limits::check(c,"Float::linear");

    FloatVal d = c;

    eliminate(t,n,d);

    Term *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = normalize(t,n,t_p,n_p,t_n,n_n);

    rewrite(frt,d,t_p,n_p,t_n,n_n);

    if (n == 0) {
      switch (frt) {
      case FRT_EQ: if (!d.in(0.0)) home.fail(); break;
      case FRT_LQ: if (d.max() < 0.0)  home.fail(); break;
      default: GECODE_NEVER;
      }
      return;
    }

    if (n == 1) {
      if (n_p == 1) {
        ScaleView y(t_p[0].a,t_p[0].x);
        switch (frt) {
        case FRT_EQ: GECODE_ME_FAIL(y.eq(home,d)); break;
        case FRT_LQ: GECODE_ME_FAIL(y.lq(home,d)); break;
        default: GECODE_NEVER;
        }
      } else {
        ScaleView y(t_n[0].a,t_n[0].x);
        switch (frt) {
        case FRT_EQ: GECODE_ME_FAIL(y.eq(home,-d)); break;
        case FRT_LQ: GECODE_ME_FAIL(y.gq(home,-d)); break;
        default: GECODE_NEVER;
        }
      }
      return;
    }

    if (is_unit) {
      // Unit coefficients
      ViewArray<FloatView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = t_p[i].x;
      ViewArray<FloatView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = t_n[i].x;
      post_nary<FloatView>(home,x,y,frt,d);
    } else {
      // Arbitrary coefficients 
      ViewArray<ScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = ScaleView(t_p[i].a,t_p[i].x);
      ViewArray<ScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = ScaleView(t_n[i].a,t_n[i].x);
      post_nary<ScaleView>(home,x,y,frt,d);
    }
  }

  /**
   * \brief Posting reified n-ary propagators
   *
   */
  template<class View, class ReifyView>
  forceinline void
  post_nary(Home home,
            ViewArray<View>& x, ViewArray<View>& y,
            FloatRelType frt, FloatVal c, Reify r) {
    switch (frt) {
    case FRT_EQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((ReEq<View,View,ReifyView,RM_EQV>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((ReEq<View,View,ReifyView,RM_IMP>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((ReEq<View,View,ReifyView,RM_PMI>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      default: GECODE_NEVER;
      }
      break;
    case FRT_LQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((ReLq<View,View,ReifyView,RM_EQV>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((ReLq<View,View,ReifyView,RM_IMP>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((ReLq<View,View,ReifyView,RM_PMI>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      default: GECODE_NEVER;
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post(Home home,
       Term* t, int n, FloatRelType frt, FloatVal c, Reify r, bool b) {
    Limits::check(c,"Float::linear");

    FloatVal d = c;

    eliminate(t,n,d);

    Term *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = normalize(t,n,t_p,n_p,t_n,n_n);

    rewrite(frt,d,t_p,n_p,t_n,n_n);

    if (n == 0) {
      // FIXME!
      bool fail = false;
      switch (frt) {
      case FRT_EQ: fail = (d != 0.0); break;
      case FRT_LQ: fail = (d < 0.0); break;
      default: GECODE_NEVER;
      }
      if (((fail && b)? 
           Int::BoolView(r.var()).zero(home) : 
           Int::BoolView(r.var()).one(home)) == Int::ME_INT_FAILED)
        home.fail();
      return;
    }

    if (is_unit) {
      ViewArray<FloatView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = t_p[i].x;
      ViewArray<FloatView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = t_n[i].x;
      if (b) 
        post_nary<FloatView,Int::BoolView>(home,x,y,frt,d,r);
      else   
        post_nary<FloatView,Int::NegBoolView>(home,x,y,frt,d,r);
    } else {
      // Arbitrary coefficients with double precision
      ViewArray<ScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = ScaleView(t_p[i].a,t_p[i].x);
      ViewArray<ScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = ScaleView(t_n[i].a,t_n[i].x);
      if (b) 
        post_nary<ScaleView,Int::BoolView>(home,x,y,frt,d,r);
      else   
        post_nary<ScaleView,Int::NegBoolView>(home,x,y,frt,d,r);
    }
  }

}}}

// STATISTICS: float-post

