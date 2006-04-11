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
    GECODE_AUTOARRAY(Linear::Term, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,icl);
  }

  void
  linear(Space* home,
	 const IntVarArgs& x, IntRelType r, int c, BoolVar b, IntConLevel icl) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term, t, x.size());
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
    GECODE_AUTOARRAY(Linear::Term, t, x.size());
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
    GECODE_AUTOARRAY(Linear::Term, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,b);
  }

  void
  linear(Space* home,
	 const IntVarArgs& x, IntRelType r, IntVar y, IntConLevel icl) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term, t, x.size()+1);
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
    GECODE_AUTOARRAY(Linear::Term, t, x.size()+1);
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
    GECODE_AUTOARRAY(Linear::Term, t, x.size()+1);
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
    GECODE_AUTOARRAY(Linear::Term, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,b);
  }


  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, int c,
	 IntConLevel) {
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    ConstIntView cv(c);
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,Linear::EqBool<ConstIntView>::post(home,xv,0,cv));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,Linear::NqBool<ConstIntView>::post(home,xv,0,cv));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,Linear::LqBool<ConstIntView>::post(home,xv,0,cv));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,Linear::LqBool<ConstIntView>::post(home,xv,-1,cv));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,Linear::GqBool<ConstIntView>::post(home,xv,0,cv));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,Linear::GqBool<ConstIntView>::post(home,xv,1,cv));
      break;
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
      GECODE_ES_FAIL(home,Linear::EqBool<IntView>::post(home,xv,0,y));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,Linear::NqBool<IntView>::post(home,xv,0,y));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,Linear::LqBool<IntView>::post(home,xv,0,y));
      break;
    case IRT_LE:
      GECODE_ES_FAIL(home,Linear::LqBool<IntView>::post(home,xv,-1,y));
      break;
    case IRT_GQ:
      GECODE_ES_FAIL(home,Linear::GqBool<IntView>::post(home,xv,0,y));
      break;
    case IRT_GR:
      GECODE_ES_FAIL(home,Linear::GqBool<IntView>::post(home,xv,1,y));
      break;
    default:
      throw UnknownRelation("Int::linear");
    }
  }

}

// STATISTICS: int-post

