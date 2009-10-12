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

namespace Gecode {

  inline IntVar
  abs(Home home, IntVar x, IntConLevel icl) {
    if ((icl == ICL_DOM) && (x.min() >= 0))
      return x;
    IntVar y(home, Int::Limits::min, Int::Limits::max);
    abs(home, x, y, icl);
    return y;
  }

  inline IntVar
  min(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    min(home, x, y, z, icl);
    return z;
  }

  inline IntVar
  min(Home home, const IntVarArgs& x, IntConLevel icl) {
    IntVar y(home, Int::Limits::min, Int::Limits::max);
    Gecode::min(home, x, y, icl);
    return y;
  }

  inline IntVar
  max(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    max(home, x, y, z, icl);
    return z;
  }

  inline IntVar
  max(Home home, const IntVarArgs& x, IntConLevel icl) {
    IntVar y(home, Int::Limits::min, Int::Limits::max);
    Gecode::max(home, x, y, icl);
    return y;
  }

  inline IntVar
  mult(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    mult(home, x, y, z, icl);
    return z;
  }

  inline IntVar
  div(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    div(home, x, y, z, icl);
    return z;
  }

  inline IntVar
  mod(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    mod(home, x, y, z, icl);
    return z;
  }

  inline IntVar
  sqr(Home home, IntVar x, IntConLevel icl) {
    IntVar y(home, 0, Int::Limits::max);
    sqr(home, x, y, icl);
    return y;
  }

  inline IntVar
  sqrt(Home home, IntVar x, IntConLevel icl) {
    IntVar y(home, 0, Int::Limits::max);
    sqrt(home, x, y, icl);
    return y;
  }

  inline IntVar
  plus(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    IntVarArgs xy(2);
    xy[0]=x; xy[1]=y;
    linear(home, xy, IRT_EQ, z, icl);
    return z;
  }

  inline IntVar
  minus(Home home, IntVar x, IntVar y, IntConLevel icl) {
    IntVar z(home, Int::Limits::min, Int::Limits::max);
    IntVarArgs xy(2); IntArgs a(2, 1,-1);
    xy[0]=x; xy[1]=y;
    linear(home, a, xy, IRT_EQ, z, icl);
    return z;
  }

}

// STATISTICS: minimodel-any
