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

#include "gecode/minimodel.hh"

#include <algorithm>

namespace Gecode {

#define GECODE_MM_RETURN_FAILED                 \
if (home->failed()) {                           \
  IntVar _x(home,0,0); return _x;               \
}

  IntVar
  abs(Space* home, IntVar x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    if ((icl == ICL_DOM) && (x.min() >= 0))
      return x;
    int min, max;
    if (x.min() >= 0) {
      min = x.min(); max = x.max();
    } else if (x.max() <= 0) {
      min = -x.max(); max = -x.min();
    } else {
      min = 0; max = std::max(-x.min(),x.max());
    }
    IntVar y(home, min, max);
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
    int min = Int::Limits::max;
    int max = Int::Limits::max;
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
    int min = Int::Limits::min;
    int max = Int::Limits::min;
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
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    mult(home, x, y, z, icl);
    return z;
  }

  IntVar
  sqr(Space* home, IntVar x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar y(home, 0, Int::Limits::max);
    mult(home, x, x, y, icl);
    return y;
  }

  IntVar
  sqrt(Space* home, IntVar x, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar y(home, 0, std::max(0,x.max()));
    mult(home, y, y, x, icl);
    return y;
  }

  IntVar
  plus(Space* home, IntVar x, IntVar y, IntConLevel icl) {
    GECODE_MM_RETURN_FAILED;
    IntVar z(home,x.min()+y.min(),x.max()+y.max());
    Int::Linear::Term<Int::IntView> ts[3];
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
    Int::Linear::Term<Int::IntView> ts[3];
    ts[0].a =  1; ts[0].x = x;
    ts[1].a = -1; ts[1].x = y;
    ts[2].a = -1; ts[2].x = z;
    Int::Linear::post(home, ts, 3, IRT_EQ, 0, icl);
    return z;
  }

#undef GECODE_MM_RETURN_FAILED

}

// STATISTICS: minimodel-any
