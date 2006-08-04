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

#include "gecode/minimodel.hh"

#include <algorithm>

namespace Gecode {

#define GECODE_MM_RETURN_FAILED			\
if (home->failed()) {				\
  IntVar _x(home,0,0); return _x;		\
}

  IntVar
  abs(Space* home, IntVar x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    if (x.min() >= 0)
      return x;
    IntVar y(home,
	     std::min(-x.max(),x.min()),
	     std::max(-x.min(),x.max()));
    abs(home, x, y, icl);
    return y;
  }

  IntVar
  min(Space* home, IntVar x, IntVar y, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar z(home,
	     std::min(x.min(),y.min()),
	     std::min(x.max(),y.max()));
    min(home, x, y, z, icl);
    return z;
  }

  IntVar
  min(Space* home, const IntVarArgs& x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    int min = Limits::Int::int_max;
    int max = Limits::Int::int_max;
    for (int i=x.size(); i--; ) {
      min = std::min(min,x[i].min());
      max = std::min(max,x[i].max());
    }
    IntVar y(home, min, max);
    Gecode::min(home, x, y, icl);
    return y;
  }

  IntVar
  max(Space* home, IntVar x, IntVar y, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar z(home,
	     std::max(x.min(),y.min()),
	     std::max(x.max(),y.max()));
    max(home, x, y, z, icl);
    return z;
  }

  IntVar
  max(Space* home, const IntVarArgs& x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    int min = Limits::Int::int_min;
    int max = Limits::Int::int_min;
    for (int i=x.size(); i--; ) {
      min = std::max(min,x[i].min());
      max = std::max(max,x[i].max());
    }
    IntVar y(home, min, max);
    Gecode::max(home, x, y, icl);
    return y;
  }

  IntVar
  mult(Space* home, IntVar x, IntVar y, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar z(home, Limits::Int::int_min, Limits::Int::int_max);
    mult(home, x, y, z, icl);
    return z;
  }

  IntVar
  sqr(Space* home, IntVar x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar y(home, 0, Limits::Int::int_max);
    mult(home, x, x, y, icl);
    return y;
  }

  IntVar
  plus(Space* home, IntVar x, IntVar y, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar z(home,x.min()+y.min(),x.max()+y.max());
    Int::Linear::Term ts[3];
    ts[0].a =  1; ts[0].x = x;
    ts[1].a =  1; ts[1].x = y;
    ts[2].a = -1; ts[2].x = z;
    Int::Linear::post(home, ts, 3, IRT_EQ, 0, icl);
    return z;
  }

  IntVar
  minus(Space* home, IntVar x, IntVar y, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar z(home,x.min()-y.max(),x.max()-y.min());
    Int::Linear::Term ts[3];
    ts[0].a =  1; ts[0].x = x;
    ts[1].a = -1; ts[1].x = y;
    ts[2].a = -1; ts[2].x = z;
    Int::Linear::post(home, ts, 3, IRT_EQ, 0, icl);
    return z;
  }

#undef GECODE_MM_RETURN_FAILED

}

// STATISTICS: minimodel-any
