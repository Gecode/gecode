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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int/linear.hh"

namespace Gecode {

  using namespace Int;

  /*
   * Exported post functions
   *
   */

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, int c, IntConLevel icl) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,icl);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, int c, BoolVar b, IntConLevel icl) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,b);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, int c,
         IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,icl);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, int c, BoolVar b,
         IntConLevel) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,b);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, IntVar y, IntConLevel icl) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,icl);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, IntVar y, BoolVar b,
         IntConLevel) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,b);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, IntVar y,
         IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,icl);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, IntVar y,
         BoolVar b, IntConLevel) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,b);
  }


  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    /*
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      IntVar ix(home,0,1); link(home,ix,x[i]);
      t[i].a=a[i]; t[i].x=ix;
    }
    Linear::post(home,t,x.size(),r,c,icl);
    */
    ZeroIntView z;
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Linear::EqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>::post(home,a,x,z,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>::post(home,a,x,z,c)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>::post(home,a,x,z,c-1)));
      break;
    case IRT_GQ:
      {
        IntArgs b(a.size());
        for (int i=a.size(); i--; )
          b[i]=-a[i];
        GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>::post(home,b,x,z,-c)));
        break;
      }
    case IRT_GR:
      {
        IntArgs b(a.size());
        for (int i=a.size(); i--; )
          b[i]=-a[i];
        GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>::post(home,b,x,z,-c-1)));
        break;
      }
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Linear::NqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>::post(home,a,x,z,c)));
      break;
    default:
      break;
    }
  }

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    /*
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      IntVar ix(home,0,1); link(home,ix,x[i]);
      t[i].a=a[i]; t[i].x=ix;
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,icl);
    */
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Linear::EqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>::post(home,a,x,y,0)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>::post(home,a,x,y,0)));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>::post(home,a,x,y,-1)));
      break;
    case IRT_GQ:
      {
        IntArgs b(a.size());
        for (int i=a.size(); i--; )
          b[i]=-a[i];
        MinusView my(y);
        GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,MinusView>::post(home,b,x,my,0)));
        break;
      }
    case IRT_GR:
      {
        IntArgs b(a.size());
        for (int i=a.size(); i--; )
          b[i]=-a[i];
        MinusView my(y);
        GECODE_ES_FAIL(home,(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,MinusView>::post(home,b,x,my,-1)));
        break;
      }
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Linear::NqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>::post(home,a,x,y,0)));
      break;
    default:
      break;
    }
  }

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,Linear::EqBoolInt<BoolView>::post(home,xv,c));
        break;
      }
    case IRT_NQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,Linear::NqBoolInt<BoolView>::post(home,xv,c));
        break;
      }
    case IRT_GQ:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,Linear::GqBoolInt<BoolView>::post(home,xv,c));
        break;
      }
    case IRT_GR:
      {
        ViewArray<BoolView> xv(home,x);
        GECODE_ES_FAIL(home,Linear::GqBoolInt<BoolView>::post(home,xv,c+1));
        break;
      }
    case IRT_LQ:
      {
        ViewArray<NegBoolView> xv(home,x.size());
        for (int i=x.size(); i--; ) {
          BoolView y(x[i]); xv[i] = y;
        }
        GECODE_ES_FAIL(home,Linear::GqBoolInt<NegBoolView>
                       ::post(home,xv,x.size()-c));
        break;
      }
    case IRT_LE:
      {
        ViewArray<NegBoolView> xv(home,x.size());
        for (int i=x.size(); i--; ) {
          BoolView y(x[i]); xv[i] = y;
        }
        GECODE_ES_FAIL(home,Linear::GqBoolInt<NegBoolView>
                       ::post(home,xv,x.size()-c+1));
        break;
      }
    default:
      throw UnknownRelation("Int::linear");
    }
  }

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel) {
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Linear::EqBoolView<BoolView,IntView>
                           ::post(home,xv,y,0)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Linear::NqBoolView<BoolView,IntView>
                           ::post(home,xv,y,0)));
      break;
    case IRT_LQ:
      {
        int n = x.size();
        ViewArray<NegBoolView> xv(home,n);
        for (int i=n; i--; ) {
          BoolView y(x[i]); xv[i]=y;
        }
        MinusView my(y);
        GECODE_ES_FAIL(home,(Linear::GqBoolView<NegBoolView,MinusView>
                             ::post(home,xv,my,n)));
        break;
      }
    case IRT_LE:
      {
        int n = x.size();
        ViewArray<NegBoolView> xv(home,n);
        for (int i=n; i--; ) {
          BoolView y(x[i]); xv[i]=y;
        }
        MinusView my(y);
        GECODE_ES_FAIL(home,(Linear::GqBoolView<NegBoolView,MinusView>
                             ::post(home,xv,my,n+1)));
        break;
      }
    case IRT_GQ:
      GECODE_ES_FAIL(home,(Linear::GqBoolView<BoolView,IntView>
                           ::post(home,xv,y,0)));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,(Linear::GqBoolView<BoolView,IntView>
                           ::post(home,xv,y,1)));
      break;
    default:
      throw UnknownRelation("Int::linear");
    }
  }

}

// STATISTICS: int-post

