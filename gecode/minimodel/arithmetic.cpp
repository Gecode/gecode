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

#include <gecode/minimodel.hh>

namespace Gecode {

  IntVar
  abs(Home home, IntVar x, IntConLevel icl) {
    if (x.min() >= 0)
      return x;
    IntVar y(home, Int::Limits::min, Int::Limits::max);
    abs(home, x, y, icl);
    return y;
  }

  IntVar
  min(Home home, IntVar x, IntVar y, IntConLevel icl) {
    if (x.max() <= y.min())
      return x;
    if (y.max() <= x.min())
      return y;
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    min(home, x, y, z, icl);
    return z;
  }

  IntVar
  max(Home home, IntVar x, IntVar y, IntConLevel icl) {
    if (x.max() <= y.min())
      return y;
    if (y.max() <= x.min())
      return x;
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    max(home, x, y, z, icl);
    return z;
  }

  IntVar
  mult(Home home, IntVar x, IntVar y, IntConLevel icl) {
    if (x.assigned() && (x.val() == 0))
      return x;
    if (x.assigned() && (x.val() == 1))
      return y;
    if (y.assigned() && (y.val() == 0))
      return y;
    if (y.assigned() && (y.val() == 1))
      return x;
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    mult(home, x, y, z, icl);
    return z;
  }

  IntVar
  sqr(Home home, IntVar x, IntConLevel icl) {
    if (x.assigned() && ((x.val() == 0) || (x.val() == 1)))
      return x;
    IntVar y(home, 0, Int::Limits::max);
    sqr(home, x, y, icl);
    return y;
  }

  IntVar
  sqrt(Home home, IntVar x, IntConLevel icl) {
    if (x.assigned() && ((x.val() == 0) || (x.val() == 1)))
      return x;
    IntVar y(home, 0, Int::Limits::max);
    sqrt(home, x, y, icl);
    return y;
  }

  IntVar
  plus(Home home, IntVar x, IntVar y, IntConLevel icl) {
    if (x.assigned() && (x.val() == 0))
      return y;
    if (y.assigned() && (y.val() == 0))
      return x;
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    IntVarArgs xy(2);
    xy[0]=x; xy[1]=y;
    linear(home, xy, IRT_EQ, z, icl);
    return z;
  }

  IntVar
  minus(Home home, IntVar x, IntVar y, IntConLevel icl) {
    if (y.assigned() && (y.val() == 0))
      return x;
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    IntVarArgs xy(2); IntArgs a(2, 1,-1);
    xy[0]=x; xy[1]=y;
    linear(home, a, xy, IRT_EQ, z, icl);
    return z;
  }

}

// STATISTICS: minimodel-any
