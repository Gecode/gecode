/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Tias Guns <tias.guns@cs.kuleuven.be>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Tias Guns, 2009
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

#include <gecode/int/linear.hh>

namespace Gecode { namespace Int { namespace Linear {

  /// Inverse the relation
  inline IntRelType 
  inverse(const IntRelType r) {
    switch (r) {
      case IRT_EQ: return IRT_NQ; break;
      case IRT_NQ: return IRT_EQ; break;
      case IRT_GQ: return IRT_LE; break;
      case IRT_LQ: return IRT_GR; break;
      case IRT_LE: return IRT_GQ; break;
      case IRT_GR: return IRT_LQ; break;
      default: GECODE_NEVER;
    }
    throw UnknownRelation("Int::linear");
  }

  /// Eliminate assigned views
  inline void
  eliminate(Term<BoolView>* t, int &n, double& d) {
    for (int i=n; i--; )
      if (t[i].x.one()) {
        d -= t[i].a; t[i]=t[--n];
      } else if (t[i].x.zero()) {
        t[i]=t[--n];
      }

    Limits::check(d,"Int::linear");
  }

  /// Rewrite non-strict relations
  inline void
  rewrite(IntRelType &r, double &d) {
      switch (r) {
      case IRT_EQ: case IRT_NQ: case IRT_LQ: case IRT_GQ:
        break;
      case IRT_LE:
        d--; r = IRT_LQ; break;
      case IRT_GR:
        d++; r = IRT_GQ; break;
      default:
        throw UnknownRelation("Int::linear");
      }
  }

  void
  post_pos_unit(Home home,
                Term<BoolView>* t_p, int n_p,
                IntRelType r, IntView y, int c) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((EqBoolView<BoolView,IntView>
                             ::post(home,x,y,c)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((NqBoolView<BoolView,IntView>
                             ::post(home,x,y,c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((GqBoolView<BoolView,IntView>
                             ::post(home,x,y,c)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=NegBoolView(t_p[i].x);
        MinusView z(y);
        GECODE_ES_FAIL((GqBoolView<NegBoolView,MinusView>
                             ::post(home,x,z,n_p-c)));
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post_pos_unit(Home home,
                Term<BoolView>* t_p, int n_p,
                IntRelType r, ZeroIntView, int c) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((EqBoolInt<BoolView>::post(home,x,c)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((NqBoolInt<BoolView>::post(home,x,c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((GqBoolInt<BoolView>::post(home,x,c)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=NegBoolView(t_p[i].x);
        GECODE_ES_FAIL((GqBoolInt<NegBoolView>::post(home,x,n_p-c)));
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post_pos_unit(Home home,
                Term<BoolView>* t_p, int n_p,
                IntRelType r, int c, BoolView b,
                IntConLevel) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((ReEqBoolInt<BoolView,BoolView>::post(home,x,c,b)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        NegBoolView nb(b);
        GECODE_ES_FAIL((ReEqBoolInt<BoolView,NegBoolView>::post(home,x,c,nb)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL((ReGqBoolInt<BoolView,BoolView>::post(home,x,c,b)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=NegBoolView(t_p[i].x);
        GECODE_ES_FAIL((ReGqBoolInt<NegBoolView,BoolView>::post(home,x,n_p-c,b)));
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post_neg_unit(Home home,
                Term<BoolView>* t_n, int n_n,
                IntRelType r, IntView y, int c) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        MinusView z(y);
        GECODE_ES_FAIL((EqBoolView<BoolView,MinusView>
                             ::post(home,x,z,-c)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        MinusView z(y);
        GECODE_ES_FAIL((NqBoolView<BoolView,MinusView>
                             ::post(home,x,z,-c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<NegBoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=NegBoolView(t_n[i].x);
        GECODE_ES_FAIL((GqBoolView<NegBoolView,IntView>
                             ::post(home,x,y,n_n+c)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        MinusView z(y);
        GECODE_ES_FAIL((GqBoolView<BoolView,MinusView>
                             ::post(home,x,z,-c)));
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post_neg_unit(Home home,
                Term<BoolView>* t_n, int n_n,
                IntRelType r, ZeroIntView, int c) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL((EqBoolInt<BoolView>::post(home,x,-c)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL((NqBoolInt<BoolView>::post(home,x,-c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<NegBoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=NegBoolView(t_n[i].x);
        GECODE_ES_FAIL((GqBoolInt<NegBoolView>::post(home,x,n_n+c)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL((GqBoolInt<BoolView>::post(home,x,-c)));
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post_neg_unit(Home home,
                Term<BoolView>* t_n, int n_n,
                IntRelType r, int c, BoolView b,
                IntConLevel) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL((ReEqBoolInt<BoolView,BoolView>::post(home,x,-c,b)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        NegBoolView nb(b);
        GECODE_ES_FAIL((ReEqBoolInt<BoolView,NegBoolView>::post(home,x,-c,nb)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<NegBoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=NegBoolView(t_n[i].x);
        GECODE_ES_FAIL((ReGqBoolInt<NegBoolView,BoolView>::post(home,x,n_n+c,b)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL((ReGqBoolInt<BoolView,BoolView>::post(home,x,-c,b)));
      }
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post_mixed(Home home,
             Term<BoolView>* t_p, int n_p,
             Term<BoolView>* t_n, int n_n,
             IntRelType r, IntView y, int c) {
    ScaleBoolArray b_p(home,n_p);
    {
      ScaleBool* f=b_p.fst();
      for (int i=n_p; i--; ) {
        f[i].x=t_p[i].x; f[i].a=t_p[i].a;
      }
    }
    ScaleBoolArray b_n(home,n_n);
    {
      ScaleBool* f=b_n.fst();
      for (int i=n_n; i--; ) {
        f[i].x=t_n[i].x; f[i].a=t_n[i].a;
      }
    }
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(
                     (EqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(
                     (NqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_GQ:
      {
        MinusView m(y);
        GECODE_ES_FAIL(
                       (LqBoolScale<ScaleBoolArray,ScaleBoolArray,MinusView>
                        ::post(home,b_n,b_p,m,-c)));
      }
      break;
    default:
      GECODE_NEVER;
    }
  }


  void
  post_mixed(Home home,
             Term<BoolView>* t_p, int n_p,
             Term<BoolView>* t_n, int n_n,
             IntRelType r, ZeroIntView y, int c) {
    ScaleBoolArray b_p(home,n_p);
    {
      ScaleBool* f=b_p.fst();
      for (int i=n_p; i--; ) {
        f[i].x=t_p[i].x; f[i].a=t_p[i].a;
      }
    }
    ScaleBoolArray b_n(home,n_n);
    {
      ScaleBool* f=b_n.fst();
      for (int i=n_n; i--; ) {
        f[i].x=t_n[i].x; f[i].a=t_n[i].a;
      }
    }
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(
                     (EqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(
                     (NqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_n,b_p,y,-c)));
      break;
    default:
      GECODE_NEVER;
    }
  }

  template<class View>
  forceinline void
  post_all(Home home,
           Term<BoolView>* t, int n,
           IntRelType r, View x, int c) {

    Limits::check(c,"Int::linear");

    double d = c;

    eliminate(t,n,d);

    rewrite(r,d);

    c = static_cast<int>(d);

    Term<BoolView> *t_p, *t_n;
    int n_p, n_n;
    bool unit = normalize<BoolView>(t,n,t_p,n_p,t_n,n_n);

    if (n == 0) {
      switch (r) {
      case IRT_EQ: GECODE_ME_FAIL(x.eq(home,-c)); break;
      case IRT_NQ: GECODE_ME_FAIL(x.nq(home,-c)); break;
      case IRT_GQ: GECODE_ME_FAIL(x.lq(home,-c)); break;
      case IRT_LQ: GECODE_ME_FAIL(x.gq(home,-c)); break;
      default: GECODE_NEVER;
      }
      return;
    }

    // Check for overflow
    {
      double sl = x.max()+c;
      double su = x.min()+c;
      for (int i=n_p; i--; )
        su -= t_p[i].a;
      for (int i=n_n; i--; )
        sl += t_n[i].a;
      Limits::check(sl,"Int::linear");
      Limits::check(su,"Int::linear");
    }

    if (unit && (n_n == 0)) {
      /// All coefficients are 1
      post_pos_unit(home,t_p,n_p,r,x,c);
    } else if (unit && (n_p == 0)) {
      // All coefficients are -1
      post_neg_unit(home,t_n,n_n,r,x,c);
    } else {
      // Mixed coefficients
      post_mixed(home,t_p,n_p,t_n,n_n,r,x,c);
    }
  }


  void
  post(Home home,
       Term<BoolView>* t, int n, IntRelType r, IntView x, int c,
       IntConLevel) {
    post_all(home,t,n,r,x,c);
  }

  void
  post(Home home,
       Term<BoolView>* t, int n, IntRelType r, int c,
       IntConLevel) {
    ZeroIntView x;
    post_all(home,t,n,r,x,c);
  }

  void
  post(Home home,
       Term<BoolView>* t, int n, IntRelType r, IntView x, BoolView b,
       IntConLevel icl) {
    int l, u;
    estimate(t,n,0,l,u);
    IntVar z(home,l,u); IntView zv(z);
    post_all(home,t,n,IRT_EQ,zv,0);
    rel(home,z,r,x,b,icl);
  }

  void
  post(Home home,
       Term<BoolView>* t, int n, IntRelType r, int c, BoolView b,
       IntConLevel icl) {

    if (b.one())
      return post(home,t,n,r,c,icl);
    if (b.zero())
      return post(home,t,n,inverse(r),c,icl);

    Limits::check(c,"Int::linear");

    double d = c;

    eliminate(t,n,d);

    rewrite(r,d);

    c = static_cast<int>(d);

    if (n == 0) {
      bool fail = false;
      switch (r) {
      case IRT_EQ: fail = (0 != c); break;
      case IRT_NQ: fail = (0 == c); break;
      case IRT_GQ: fail = (0 < c); break;
      case IRT_LQ: fail = (0 > c); break;
      default: GECODE_NEVER;
      }
      if ((fail ? b.zero(home) : b.one(home)) == ME_INT_FAILED)
        home.fail();
      return;
    }

    Term<BoolView> *t_p, *t_n;
    int n_p, n_n;
    bool unit = normalize<BoolView>(t,n,t_p,n_p,t_n,n_n);

    // Check for overflow
    {
      double sl = c;
      double su = c;
      for (int i=n_p; i--; )
        su -= t_p[i].a;
      for (int i=n_n; i--; )
        sl += t_n[i].a;
      Limits::check(sl,"Int::linear");
      Limits::check(su,"Int::linear");
    }

    if (unit && (n_n == 0)) {
      /// All coefficients are 1
      post_pos_unit(home,t_p,n_p,r,c,b,icl);
    } else if (unit && (n_p == 0)) {
      // All coefficients are -1
      post_neg_unit(home,t_n,n_n,r,c,b,icl);
    } else {
      // Mixed coefficients
      /*
       * Denormalize: Make all t_n coefficients negative again
       * (t_p and t_n are shared in t)
       */
      for (int i=n_n; i--; )
        t_n[i].a = -t_n[i].a;

      // Note: still slow implementation
      int l, u;
      estimate(t,n,0,l,u);
      IntVar z(home,l,u); IntView zv(z);
      post_all(home,t,n,IRT_EQ,zv,0);
      rel(home,z,r,c,b,icl);
    }
  }

}}}

// STATISTICS: int-post

