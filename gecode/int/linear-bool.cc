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

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel icl) {
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

    int n_p = 0;
    int n_n = 0;
    preprocess<BoolView>(t,n,r,c,n_p,n_n);

    ScaleBoolArray pb(home,n_p);
    {
      ScaleBool* f=pb.fst();
      for (int i=n_p; i--; ) {
        f[i].x=t[i].x; f[i].a=t[i].a;
      }
    }
    ScaleBoolArray nb(home,n_n);
    {
      ScaleBool* f=nb.fst();
      for (int i=n_n; i--; ) {
        f[i].x=t[i+n_p].x; f[i].a=-t[i+n_p].a;
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
         IntConLevel icl) {
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

    int n_p = 0;
    int n_n = 0;
    int c=0;
    preprocess<BoolView>(t,n,r,c,n_p,n_n);

    ScaleBoolArray pb(home,n_p);
    {
      ScaleBool* f=pb.fst();
      for (int i=n_p; i--; ) {
        f[i].x=t[i].x; f[i].a=t[i].a;
      }
    }
    ScaleBoolArray nb(home,n_n);
    {
      ScaleBool* f=nb.fst();
      for (int i=n_n; i--; ) {
        f[i].x=t[i+n_p].x; f[i].a=-t[i+n_p].a;
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
