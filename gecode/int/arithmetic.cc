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

#include "gecode/int/arithmetic.hh"

namespace Gecode {

  using namespace Int;

  void
  abs(Space* home, IntVar x0, IntVar x1, IntConLevel cl, PropKind) {
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
  max(Space* home, IntVar x0, IntVar x1, IntVar x2, IntConLevel, PropKind) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Arithmetic::Max<IntView>::post(home,x0,x1,x2));
  }

  void
  max(Space* home, const IntVarArgs& x, IntVar y, IntConLevel, PropKind) {
    if (x.size() == 0)
      throw TooFewArguments("Int::max");
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(home,Arithmetic::NaryMax<IntView>::post(home,xv,y));
  }


  void
  min(Space* home, IntVar x0, IntVar x1, IntVar x2, IntConLevel, PropKind) {
    if (home->failed()) return;
    MinusView m0(x0); MinusView m1(x1); MinusView m2(x2);
    GECODE_ES_FAIL(home,Arithmetic::Max<MinusView>::post(home,m0,m1,m2));
  }

  void
  min(Space* home, const IntVarArgs& x, IntVar y, IntConLevel, PropKind) {
    if (x.size() == 0)
      throw TooFewArguments("Int::min");
    if (home->failed()) return;
    ViewArray<MinusView> m(home,x.size());
    for (int i=x.size(); i--; )
      m[i].init(x[i]);
    MinusView my(y);
    GECODE_ES_FAIL(home,Arithmetic::NaryMax<MinusView>::post(home,m,my));
  }


  void
  mult(Space* home, IntVar x0, IntVar x1, IntVar x2, IntConLevel, PropKind) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Arithmetic::Mult<IntView>::post(home,x0,x1,x2));
  }


  void
  sqr(Space* home, IntVar x0, IntVar x1, IntConLevel icl, PropKind) {
    if (home->failed()) return;
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::SqrDom<IntView>::post(home,x0,x1));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::SqrBnd<IntView>::post(home,x0,x1));
    }
  }

  void
  sqrt(Space* home, IntVar x0, IntVar x1, IntConLevel, PropKind) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Arithmetic::Sqrt<IntView>::post(home,x0,x1));
  }

  namespace {
    GECODE_REGISTER1(Arithmetic::AbsBnd<IntView>);
    GECODE_REGISTER1(Arithmetic::AbsDom<IntView>);
    GECODE_REGISTER1(Arithmetic::Max<IntView>);
    GECODE_REGISTER1(Arithmetic::Max<MinusView>);
    GECODE_REGISTER1(Arithmetic::Mult<IntView>);
    GECODE_REGISTER1(Arithmetic::MultZeroOne<IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlus<double,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlus<double,MinusView,IntView,MinusView>);
    GECODE_REGISTER4(Arithmetic::MultPlus<double,MinusView,MinusView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlus<int,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlus<int,MinusView,IntView,MinusView>);
    GECODE_REGISTER4(Arithmetic::MultPlus<int,MinusView,MinusView,IntView>);
    GECODE_REGISTER1(Arithmetic::NaryMax<IntView>);
    GECODE_REGISTER1(Arithmetic::NaryMax<MinusView>);
    GECODE_REGISTER1(Arithmetic::SqrBnd<IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusBnd<IntView,IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusBnd<MinusView,IntView>);
    GECODE_REGISTER1(Arithmetic::SqrDom<IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusDom<IntView,IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusDom<MinusView,IntView>);
    GECODE_REGISTER1(Arithmetic::Sqrt<IntView>);
  }
}

// STATISTICS: int-post

