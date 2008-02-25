/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "gecode/int/linear.hh"

namespace Gecode { namespace Int { namespace Linear {

  void
  post_pos_unit(Space* home, 
                Term<BoolView>* t_p, int n_p,
                IntRelType r, IntView y, int c,
                PropKind) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(EqBoolView<BoolView,IntView>
                             ::post(home,x,y,c)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(NqBoolView<BoolView,IntView>
                             ::post(home,x,y,c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(GqBoolView<BoolView,IntView>
                             ::post(home,x,y,c)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        MinusView z(y);
        GECODE_ES_FAIL(home,(GqBoolView<NegBoolView,MinusView>
                             ::post(home,x,z,n_p-c)));
      }
      break;
    default: GECODE_NEVER;
    }
  }
  
  void
  post_pos_unit(Space* home, 
                Term<BoolView>* t_p, int n_p,
                IntRelType r, ZeroIntView, int c, 
                PropKind pk) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(EqBoolInt<BoolView>::post(home,x,c,pk)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(NqBoolInt<BoolView>::post(home,x,c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(GqBoolInt<BoolView>::post(home,x,c,pk)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> x(home,n_p);
        for (int i=n_p; i--; )
          x[i]=t_p[i].x;
        GECODE_ES_FAIL(home,(GqBoolInt<NegBoolView>::post(home,x,n_p-c,pk)));
      }
      break;
    default: GECODE_NEVER;
    }
  }
  
  void
  post_neg_unit(Space* home, 
                Term<BoolView>* t_n, int n_n,
                IntRelType r, IntView y, int c,
                PropKind) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        MinusView z(y);
        GECODE_ES_FAIL(home,(EqBoolView<BoolView,MinusView>
                             ::post(home,x,z,-c)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        MinusView z(y);
        GECODE_ES_FAIL(home,(NqBoolView<BoolView,MinusView>
                             ::post(home,x,z,-c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<NegBoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL(home,(GqBoolView<NegBoolView,IntView>
                             ::post(home,x,y,n_n+c)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        MinusView z(y);
        GECODE_ES_FAIL(home,(GqBoolView<BoolView,MinusView>
                             ::post(home,x,z,-c)));
      }
      break;
    default: GECODE_NEVER;
    }
  }
  
  void
  post_neg_unit(Space* home, 
                Term<BoolView>* t_n, int n_n,
                IntRelType r, ZeroIntView, int c, 
                PropKind pk) {
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL(home,(EqBoolInt<BoolView>::post(home,x,-c,pk)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL(home,(NqBoolInt<BoolView>::post(home,x,-c)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<NegBoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL(home,(GqBoolInt<NegBoolView>::post(home,x,n_n+c,pk)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<BoolView> x(home,n_n);
        for (int i=n_n; i--; )
          x[i]=t_n[i].x;
        GECODE_ES_FAIL(home,(GqBoolInt<BoolView>::post(home,x,-c,pk)));
      }
      break;
    default: GECODE_NEVER;
    }
  }


  void
  post_mixed(Space* home,
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
      GECODE_ES_FAIL(home,
                     (EqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,
                     (NqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_GQ:
      {
        MinusView m(y);
        GECODE_ES_FAIL(home,
                       (LqBoolScale<ScaleBoolArray,ScaleBoolArray,MinusView>
                        ::post(home,b_n,b_p,m,-c)));
      }
      break;
    default:
      GECODE_NEVER;
    }
  }
  

  void
  post_mixed(Space* home, 
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
      GECODE_ES_FAIL(home,
                     (EqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,
                     (NqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_p,b_n,y,c)));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,b_n,b_p,y,-c)));
      break;
    default:
      GECODE_NEVER;
    }
  }
  
  template <class View>
  forceinline void
  post_all(Space* home, 
           Term<BoolView>* t, int n,
           IntRelType r, View x, int c, PropKind pk) {

    Limits::check(c,"Int::linear");

    {
      double d = c;

      // Eliminate non-strict relations
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

      // Eliminate assigned views
      for (int i=n; i--; )
        if (t[i].x.one()) {
          d -= t[i].a; t[i]=t[--n];
        } else if (t[i].x.zero()) {
          t[i]=t[--n];
        }

      Limits::check(d,"Int::linear");
      
      c = static_cast<int>(d);
    }
    
    Term<BoolView> *t_p, *t_n;
    int n_p, n_n;
    bool unit = normalize<BoolView>(t,n,t_p,n_p,t_n,n_n);

    if (n == 0) {
      switch (r) {
      case IRT_EQ: GECODE_ME_FAIL(home,x.eq(home,-c)); break;
      case IRT_NQ: GECODE_ME_FAIL(home,x.nq(home,-c)); break;
      case IRT_GQ: GECODE_ME_FAIL(home,x.lq(home,-c)); break;
      case IRT_LQ: GECODE_ME_FAIL(home,x.gq(home,-c)); break;
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
      post_pos_unit(home,t_p,n_p,r,x,c,pk);
    } else if (unit && (n_p == 0)) {
      // All coefficients are -1
      post_neg_unit(home,t_n,n_n,r,x,c,pk);
    } else {
      // Mixed coefficients
      post_mixed(home,t_p,n_p,t_n,n_n,r,x,c);
    }
  }
  

  void
  post(Space* home, 
       Term<BoolView>* t, int n, IntRelType r, IntView x, int c, 
       IntConLevel, PropKind pk) {
    post_all(home,t,n,r,x,c,pk);
  }
  
  void
  post(Space* home, 
       Term<BoolView>* t, int n, IntRelType r, int c, 
       IntConLevel, PropKind pk) {
    ZeroIntView x;
    post_all(home,t,n,r,x,c,pk);
  }

  void
  post(Space* home, 
       Term<BoolView>* t, int n, IntRelType r, IntView x, int c, BoolView b,
       IntConLevel icl, PropKind pk) {
    int l, u;
    estimate(t,n,0,l,u);
    IntVar z(home,l,u); IntView zv(z);
    post_all(home,t,n,IRT_EQ,zv,0,pk);
    rel(home,z,r,x,b,icl,pk);    
  }
  
  void
  post(Space* home, 
       Term<BoolView>* t, int n, IntRelType r, int c, BoolView b,
       IntConLevel icl, PropKind pk) {
    int l, u;
    estimate(t,n,0,l,u);
    IntVar z(home,l,u); IntView zv(z);
    post_all(home,t,n,IRT_EQ,zv,0,pk);
    rel(home,z,r,c,b,icl,pk);
  }

}}}

// STATISTICS: int-post

