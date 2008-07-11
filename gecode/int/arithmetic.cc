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
  abs(Space* home, IntVar x0, IntVar x1, IntConLevel icl, PropKind) {
    if (home->failed()) return;
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::AbsDom<IntView>::post(home,x0,x1));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::AbsBnd<IntView>::post(home,x0,x1));
    }
  }


  void
  max(Space* home, IntVar x0, IntVar x1, IntVar x2, 
      IntConLevel icl, PropKind) {
    if (home->failed()) return;
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::MaxDom<IntView>::post(home,x0,x1,x2));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::MaxBnd<IntView>::post(home,x0,x1,x2));
    }
  }

  void
  max(Space* home, const IntVarArgs& x, IntVar y,
      IntConLevel icl, PropKind) {
    if (x.size() == 0)
      throw TooFewArguments("Int::max");
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::NaryMaxDom<IntView>::post(home,xv,y));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::NaryMaxBnd<IntView>::post(home,xv,y));
    }
  }


  void
  min(Space* home, IntVar x0, IntVar x1, IntVar x2, 
      IntConLevel icl, PropKind) {
    if (home->failed()) return;
    MinusView m0(x0); MinusView m1(x1); MinusView m2(x2);
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::MaxDom<MinusView>::post(home,m0,m1,m2));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::MaxBnd<MinusView>::post(home,m0,m1,m2));
    }
  }

  void
  min(Space* home, const IntVarArgs& x, IntVar y, 
      IntConLevel icl, PropKind) {
    if (x.size() == 0)
      throw TooFewArguments("Int::min");
    if (home->failed()) return;
    ViewArray<MinusView> m(home,x.size());
    for (int i=x.size(); i--; )
      m[i].init(x[i]);
    MinusView my(y);
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::NaryMaxDom<MinusView>::post(home,m,my));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::NaryMaxBnd<MinusView>::post(home,m,my));
    }
  }


  void
  mult(Space* home, IntVar x0, IntVar x1, IntVar x2, 
       IntConLevel icl, PropKind) {
    if (home->failed()) return;
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::MultDom<IntView>::post(home,x0,x1,x2));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::MultBnd<IntView>::post(home,x0,x1,x2));
    }
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
  sqrt(Space* home, IntVar x0, IntVar x1, IntConLevel icl, PropKind) {
    if (home->failed()) return;
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Arithmetic::SqrtDom<IntView>::post(home,x0,x1));
    } else {
      GECODE_ES_FAIL(home,Arithmetic::SqrtBnd<IntView>::post(home,x0,x1));
    }
  }

  void
  divmod(Space* home, IntVar x0, IntVar x1, IntVar x2, IntVar x3,
         IntConLevel, PropKind) {
    if (home->failed()) return;

    IntVar prod(home, Int::Limits::min, Int::Limits::max);
    GECODE_ES_FAIL(home, 
                   Arithmetic::MultBnd<IntView>::post(home,x1,x2,prod));
    Linear::Term<IntView> t[3];
    t[0].a = 1; t[0].x = prod;
    t[1].a = 1; t[1].x = x3;
    int min, max;
    Linear::estimate(t,2,0,min,max);
    IntView x0v(x0);
    GECODE_ME_FAIL(home, x0v.gq(home,min));
    GECODE_ME_FAIL(home, x0v.lq(home,max));
    t[2].a=-1; t[2].x=x0;
    Linear::post(home,t,3,IRT_EQ,0);
    if (home->failed()) return;
    IntView x1v(x1);
    GECODE_ES_FAIL(home,
      Arithmetic::DivMod<IntView>::post(home,x1,x3));
  }

  void
  div(Space* home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel, PropKind) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,
      (Arithmetic::DivBnd<IntView>::post(home,x0,x1,x2)));
  }

  void
  mod(Space* home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel icl, PropKind pk) {
    if (home->failed()) return;
    IntVar _div(home, Int::Limits::min, Int::Limits::max);
    divmod(home, x0, x1, _div, x2, icl, pk);
  }

  namespace {
    GECODE_REGISTER1(Arithmetic::AbsBnd<IntView>);
    GECODE_REGISTER1(Arithmetic::AbsDom<IntView>);

    GECODE_REGISTER1(Arithmetic::MaxBnd<IntView>);
    GECODE_REGISTER1(Arithmetic::MaxDom<IntView>);
    GECODE_REGISTER1(Arithmetic::MaxBnd<MinusView>);
    GECODE_REGISTER1(Arithmetic::MaxDom<MinusView>);
    GECODE_REGISTER1(Arithmetic::NaryMaxBnd<IntView>);
    GECODE_REGISTER1(Arithmetic::NaryMaxDom<IntView>);
    GECODE_REGISTER1(Arithmetic::NaryMaxBnd<MinusView>);
    GECODE_REGISTER1(Arithmetic::NaryMaxDom<MinusView>);

    GECODE_REGISTER2(Arithmetic::MultZeroOne<IntView,PC_INT_BND>);
    GECODE_REGISTER2(Arithmetic::MultZeroOne<IntView,PC_INT_DOM>);
    GECODE_REGISTER4(Arithmetic::MultPlusBnd<double,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusBnd<double,MinusView,IntView,MinusView>);
    GECODE_REGISTER4(Arithmetic::MultPlusBnd<double,MinusView,MinusView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusBnd<int,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusBnd<int,MinusView,IntView,MinusView>);
    GECODE_REGISTER4(Arithmetic::MultPlusBnd<int,MinusView,MinusView,IntView>);
    GECODE_REGISTER1(Arithmetic::MultBnd<IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusDom<double,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusDom<double,MinusView,IntView,MinusView>);
    GECODE_REGISTER4(Arithmetic::MultPlusDom<double,MinusView,MinusView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusDom<int,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::MultPlusDom<int,MinusView,IntView,MinusView>);
    GECODE_REGISTER4(Arithmetic::MultPlusDom<int,MinusView,MinusView,IntView>);
    GECODE_REGISTER1(Arithmetic::MultDom<IntView>);

    GECODE_REGISTER1(Arithmetic::SqrBnd<IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusBnd<IntView,IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusBnd<MinusView,IntView>);
    GECODE_REGISTER1(Arithmetic::SqrDom<IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusDom<IntView,IntView>);
    GECODE_REGISTER2(Arithmetic::SqrPlusDom<MinusView,IntView>);

    GECODE_REGISTER1(Arithmetic::SqrtBnd<IntView>);
    GECODE_REGISTER1(Arithmetic::SqrtDom<IntView>);
    GECODE_REGISTER1(Arithmetic::DivMod<IntView>);
    
    GECODE_REGISTER4(Arithmetic::DivPlusBnd<int,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::DivPlusBnd<double,IntView,IntView,IntView>);
    GECODE_REGISTER4(Arithmetic::DivPlusBnd<double,MinusView,
                                            MinusView,IntView>);
    GECODE_REGISTER5(Arithmetic::DivPlusBnd<double,IntView,MinusView,
                                            MinusView,false>);
    GECODE_REGISTER5(Arithmetic::DivPlusBnd<double,MinusView,IntView,
                                            MinusView,false>);
    GECODE_REGISTER1(Arithmetic::DivBnd<IntView>);
  }
}

// STATISTICS: int-post

