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

#include "gecode/int/arithmetic.hh"

namespace Gecode {

  using namespace Int;

  void
  abs(Space* home, IntVar x0, IntVar x1, IntConLevel cl) {
    if (home->failed()) return;
    switch (cl) {
    case ICL_DOM:
      GECODE_ES_FAIL(home,Arithmetic::AbsDom<IntView>::post(home,x0,x1));
      break;
    default:
      GECODE_ES_FAIL(home,Arithmetic::AbsBnd<IntView>::post(home,x0,x1));
      break;
    }
  }


  void
  max(Space* home, IntVar x0, IntVar x1, IntVar x2, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Arithmetic::Max<IntView>::post(home,x0,x1,x2));
  }

  void
  max(Space* home, const IntVarArgs& x, IntVar y, IntConLevel) {
    if (x.size() == 0)
      throw ArgumentEmpty("Int::max");
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(home,Arithmetic::NaryMax<IntView>::post(home,xv,y));
  }


  void
  min(Space* home, IntVar x0, IntVar x1, IntVar x2, IntConLevel) {
    if (home->failed()) return;
    MinusView m0(x0); MinusView m1(x1); MinusView m2(x2);
    GECODE_ES_FAIL(home,Arithmetic::Max<MinusView>::post(home,m0,m1,m2));
  }

  void
  min(Space* home, const IntVarArgs& x, IntVar y, IntConLevel) {
    if (x.size() == 0)
      throw ArgumentEmpty("Int::min");
    if (home->failed()) return;
    ViewArray<MinusView> m(home,x.size());
    for (int i=x.size(); i--; )
      m[i].init(x[i]);
    MinusView my(y);
    GECODE_ES_FAIL(home,Arithmetic::NaryMax<MinusView>::post(home,m,my));
  }


  void
  mult(Space* home, IntVar x0, IntVar x1, IntVar x2, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Arithmetic::Mult<IntView>::post(home,x0,x1,x2));
  }

}

// STATISTICS: int-post

