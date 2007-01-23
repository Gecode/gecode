/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int/linear.hh"

namespace Gecode {

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel) {
    using namespace Int;
    using namespace Int::Linear;
    if (home->failed()) return;
    int n=x.size();
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,EqBoolInt<BoolView>::post(home,xv,c));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,NqBoolInt<BoolView>::post(home,xv,c));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,GqBoolInt<BoolView>::post(home,xv,c));
      }
      break;
    case IRT_GR:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,GqBoolInt<BoolView>::post(home,xv,c+1));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> xv(home,n);
        for (int i=n; i--; ) {
          BoolView y(x[i]); xv[i] = y;
        }
        GECODE_ES_FAIL(home,GqBoolInt<NegBoolView>::post(home,xv,n-c));
      }
      break;
    case IRT_LE:
      {
        ViewArray<NegBoolView> xv(home,n);
        for (int i=n; i--; ) {
          BoolView y(x[i]); xv[i] = y;
        }
        GECODE_ES_FAIL(home,GqBoolInt<NegBoolView>::post(home,xv,n-c+1));
      }
      break;
    default:
      throw UnknownRelation("Int::linear");
    }
  }

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel) {
    using namespace Int;
    using namespace Int::Linear;
    if (home->failed()) return;
    int n = x.size();
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,(EqBoolView<BoolView,IntView>
                             ::post(home,xv,y,0)));
      }
      break;
    case IRT_NQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,(NqBoolView<BoolView,IntView>
                             ::post(home,xv,y,0)));
      }
      break;
    case IRT_LQ:
      {
        ViewArray<NegBoolView> xv(home,n);
        for (int i=n; i--; ) {
          BoolView y(x[i]); xv[i]=y;
        }
        MinusView my(y);
        GECODE_ES_FAIL(home,(GqBoolView<NegBoolView,MinusView>
                             ::post(home,xv,my,n)));
      }
      break;
    case IRT_LE:
      {
        ViewArray<NegBoolView> xv(home,n);
        for (int i=n; i--; ) {
          BoolView y(x[i]); xv[i]=y;
        }
        MinusView my(y);
        GECODE_ES_FAIL(home,(GqBoolView<NegBoolView,MinusView>
                             ::post(home,xv,my,n+1)));
      }
      break;
    case IRT_GQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,(GqBoolView<BoolView,IntView>
                             ::post(home,xv,y,0)));
      }
      break;
    case IRT_GR:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,(GqBoolView<BoolView,IntView>
                             ::post(home,xv,y,1)));
      }
      break;
    default:
      throw UnknownRelation("Int::linear");
    }
  }

  /*
  namespace Int { namespace Linear {

    template <class View>
    post_any(Space* home, 
             Term<BoolView>* t_p, int n_p,
             Term<BoolView>* t_n, int n_n,
             View x, int c) {
      ScaleBoolArray pb(home,n_p);
      {
        ScaleBool* f=pb.fst();
        for (int i=n_p; i--; ) {
          f[i].x=t_p[i].x; f[i].a=t_p[i].a;
        }
      }
      ScaleBoolArray nb(home,n_n);
      {
        ScaleBool* f=nb.fst();
        for (int i=n_n; i--; ) {
          f[i].x=t_n[i].x; f[i].a=-t_n[i].a;
        }
      }
      switch (r) {
      case IRT_EQ:
        GECODE_ES_FAIL(home,
                       (EqBoolScale<ScaleBoolArray,ScaleBoolArray,View>
                        ::post(home,pb,nb,z,c)));
        break;
      case IRT_LQ:
        GECODE_ES_FAIL(home,
                       (LqBoolScale<ScaleBoolArray,ScaleBoolArray,View>
                        ::post(home,pb,nb,z,c)));
        break;
      case IRT_NQ:
        GECODE_ES_FAIL(home,
                       (NqBoolScale<ScaleBoolArray,ScaleBoolArray,View>
                        ::post(home,pb,nb,z,c)));
        break;
      default:
        GECODE_NEVER;
      }
    }

    template <class View>
    void
    post(Space* home, Linear::Term<BoolView>* t, int n, IntRelType r, View x,
         double c) {
      // Eliminate assigned views
      for (int i=n; i--; )
        if (t[i].x.one()) {
          c -= t[i].a; t[i]=t[--n];
        } else if (t[i].x.zero()) {
          t[i]=t[--n];
        }
      if ((c < Limits::Int::int_min) || (c > Limits::Int::int_max))
        throw NumericalOverflow("Int::linear");
      
      // Separate into positive and negative coefficients
      int n_p = 0;
      int n_n = 0;
      bool unit = separate<BoolView>(t,n,n_p,n_n);
      
      Linear::Term<BoolView>* t_p = t;
      Linear::Term<BoolView>* t_n = t+n_p;
      
      // Check for overflow
      {
        double sl = x.max()+c;
        double su = x.min()+c;
        for (int i=n_p; i--; )
          su -= t_p[i].a;
        for (int i=n_n; i--; )
          sl += t_n[i].a;
        if ((su < Limits::Int::int_min) || (su > Limits::Int::int_max) ||
            (sl < Limits::Int::int_min) || (sl > Limits::Int::int_max))
          throw NumericalOverflow("Int::linear");
      }
      
      if (unit && (n_n == 0)) {
        /// All coefficients are 1
      } else if (unit && (n_p == 0)) {
        // All coefficients are -1
      } else {
        // Mixed coefficients
        //        normalize<BoolView>(t,n,r,c);
        for (int 
      }
    }

  }}

  */

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel) {
    using namespace Int;
    using namespace Int::Linear;

    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) 
      return;

    int n=x.size();
    GECODE_AUTOARRAY(Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    normalize<BoolView>(t,n,r,c);

    Term<BoolView> *t_p, *t_n;
    int n_p, n_n;
    separate<BoolView>(t,n,t_p,n_p,t_n,n_n);

    ScaleBoolArray pb(home,n_p);
    {
      ScaleBool* f=pb.fst();
      for (int i=n_p; i--; ) {
        f[i].x=t_p[i].x; f[i].a=t_p[i].a;
      }
    }
    ScaleBoolArray nb(home,n_n);
    {
      ScaleBool* f=nb.fst();
      for (int i=n_n; i--; ) {
        f[i].x=t_n[i].x; f[i].a=t_n[i].a;
      }
    }

    ZeroIntView z;

    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,
                     (EqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,pb,nb,z,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,pb,nb,z,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,
                     (NqBoolScale<ScaleBoolArray,ScaleBoolArray,ZeroIntView>
                      ::post(home,pb,nb,z,c)));
      break;
    default:
      GECODE_NEVER;
    }
  }

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel) {
    using namespace Int;
    using namespace Int::Linear;

    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) 
      return;

    int n=x.size();
    GECODE_AUTOARRAY(Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Term<BoolView> *t_p, *t_n;
    int n_p, n_n;
    separate<BoolView>(t,n,t_p,n_p,t_n,n_n);

    ScaleBoolArray pb(home,n_p);
    {
      ScaleBool* f=pb.fst();
      for (int i=n_p; i--; ) {
        f[i].x=t_p[i].x; f[i].a=t_p[i].a;
      }
    }
    ScaleBoolArray nb(home,n_n);
    {
      ScaleBool* f=nb.fst();
      for (int i=n_n; i--; ) {
        f[i].x=t_n[i].x; f[i].a=t_n[i].a;
      }
    }

    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,
                     (EqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,pb,nb,y,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,
                     (NqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,pb,nb,y,0)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,pb,nb,y,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,
                     (LqBoolScale<ScaleBoolArray,ScaleBoolArray,IntView>
                      ::post(home,pb,nb,y,-1)));
      break;
    case IRT_GQ:
      {
        MinusView my(y);
        GECODE_ES_FAIL(home,
                       (LqBoolScale<ScaleBoolArray,ScaleBoolArray,MinusView>
                        ::post(home,nb,pb,my,0)));
      }
      break;
    case IRT_GR:
      {
        MinusView my(y);
        GECODE_ES_FAIL(home,
                       (LqBoolScale<ScaleBoolArray,ScaleBoolArray,MinusView>
                        ::post(home,nb,pb,my,-1)));
      }
      break;
    default:
      throw UnknownRelation("Int::linear");
    }
  }

}

// STATISTICS: int-post
