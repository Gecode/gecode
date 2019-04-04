/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include <gecode/int/arithmetic.hh>

namespace Gecode {

  void
  abs(Home home, IntVar x0, IntVar x1, IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::AbsDom<IntView>::post(home,x0,x1));
    } else {
      GECODE_ES_FAIL(Arithmetic::AbsBnd<IntView>::post(home,x0,x1));
    }
  }


  void
  max(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::MaxDom<IntView>::post(home,x0,x1,x2));
    } else {
      GECODE_ES_FAIL(Arithmetic::MaxBnd<IntView>::post(home,x0,x1,x2));
    }
  }

  void
  max(Home home, const IntVarArgs& x, IntVar y,
      IntPropLevel ipl) {
    using namespace Int;
    if (x.size() == 0)
      throw TooFewArguments("Int::max");
    GECODE_POST;
    ViewArray<IntView> xv(home,x);
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::NaryMaxDom<IntView>::post(home,xv,y));
    } else {
      GECODE_ES_FAIL(Arithmetic::NaryMaxBnd<IntView>::post(home,xv,y));
    }
  }

  void
  min(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    MinusView m0(x0); MinusView m1(x1); MinusView m2(x2);
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::MaxDom<MinusView>::post(home,m0,m1,m2));
    } else {
      GECODE_ES_FAIL(Arithmetic::MaxBnd<MinusView>::post(home,m0,m1,m2));
    }
  }

  void
  min(Home home, const IntVarArgs& x, IntVar y,
      IntPropLevel ipl) {
    using namespace Int;
    if (x.size() == 0)
      throw TooFewArguments("Int::min");
    GECODE_POST;
    ViewArray<MinusView> m(home,x.size());
    for (int i=0; i<x.size(); i++)
      m[i] = MinusView(x[i]);
    MinusView my(y);
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::NaryMaxDom<MinusView>::post(home,m,my));
    } else {
      GECODE_ES_FAIL(Arithmetic::NaryMaxBnd<MinusView>::post(home,m,my));
    }
  }


  void
  argmax(Home home, const IntVarArgs& x, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    if (x.size() == 0)
      throw TooFewArguments("Int::argmax");
    if (same(x,y))
      throw ArgumentSame("Int::argmax");
    GECODE_POST;
    // Constrain y properly
    IntView yv(y);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<IntView> ix(home,x.size());
    for (int i=0; i<x.size(); i++) {
      ix[i].idx=i; ix[i].view=x[i];
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<IntView,IntView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<IntView,IntView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmax(Home home, const IntVarArgs& x, int o, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    Limits::nonnegative(o,"Int::argmax");
    if (x.size() == 0)
      throw TooFewArguments("Int::argmax");
    if (same(x,y))
      throw ArgumentSame("Int::argmax");
    GECODE_POST;
    // Constrain y properly
    OffsetView yv(y,-o);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<IntView> ix(home,x.size());
    for (int i=0; i<x.size(); i++) {
      ix[i].idx=i; ix[i].view=x[i];
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<IntView,OffsetView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<IntView,OffsetView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmin(Home home, const IntVarArgs& x, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    if (x.size() == 0)
      throw TooFewArguments("Int::argmin");
    if (same(x,y))
      throw ArgumentSame("Int::argmin");
    GECODE_POST;
    // Constrain y properly
    IntView yv(y);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<MinusView> ix(home,x.size());
    for (int i=0; i<x.size(); i++) {
      ix[i].idx=i; ix[i].view=MinusView(x[i]);
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<MinusView,IntView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<MinusView,IntView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmin(Home home, const IntVarArgs& x, int o, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    Limits::nonnegative(o,"Int::argmin");
    if (x.size() == 0)
      throw TooFewArguments("Int::argmin");
    if (same(x,y))
      throw ArgumentSame("Int::argmin");
    GECODE_POST;
    // Constrain y properly
    OffsetView yv(y,-o);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<MinusView> ix(home,x.size());
    for (int i=0; i<x.size(); i++) {
      ix[i].idx=i; ix[i].view=MinusView(x[i]);
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<MinusView,OffsetView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<MinusView,OffsetView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmax(Home home, const BoolVarArgs& x, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    if (x.size() == 0)
      throw TooFewArguments("Int::argmax");
    GECODE_POST;
    // Constrain y properly
    IntView yv(y);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<BoolView> ix(home,x.size());
    for (int i=x.size(); i--; ) {
      ix[i].idx=i; ix[i].view=x[i];
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<BoolView,IntView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<BoolView,IntView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmax(Home home, const BoolVarArgs& x, int o, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    Limits::nonnegative(o,"Int::argmax");
    if (x.size() == 0)
      throw TooFewArguments("Int::argmax");
    GECODE_POST;
    // Constrain y properly
    OffsetView yv(y,-o);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<BoolView> ix(home,x.size());
    for (int i=x.size(); i--; ) {
      ix[i].idx=i; ix[i].view=x[i];
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<BoolView,OffsetView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<BoolView,OffsetView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmin(Home home, const BoolVarArgs& x, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    if (x.size() == 0)
      throw TooFewArguments("Int::argmin");
    GECODE_POST;
    // Constrain y properly
    IntView yv(y);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<NegBoolView> ix(home,x.size());
    for (int i=x.size(); i--; ) {
      ix[i].idx=i; ix[i].view=NegBoolView(x[i]);
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<NegBoolView,IntView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<NegBoolView,IntView,false>
                        ::post(home,ix,yv)));
  }

  void
  argmin(Home home, const BoolVarArgs& x, int o, IntVar y, bool tiebreak,
         IntPropLevel) {
    using namespace Int;
    Limits::nonnegative(o,"Int::argmin");
    if (x.size() == 0)
      throw TooFewArguments("Int::argmin");
    GECODE_POST;
    // Constrain y properly
    OffsetView yv(y,-o);
    GECODE_ME_FAIL(yv.gq(home,0));
    GECODE_ME_FAIL(yv.le(home,x.size()));
    // Construct index view array
    IdxViewArray<NegBoolView> ix(home,x.size());
    for (int i=x.size(); i--; ) {
      ix[i].idx=i; ix[i].view=NegBoolView(x[i]);
    }
    if (tiebreak)
        GECODE_ES_FAIL((Arithmetic::ArgMax<NegBoolView,OffsetView,true>
                        ::post(home,ix,yv)));
    else
        GECODE_ES_FAIL((Arithmetic::ArgMax<NegBoolView,OffsetView,false>
                        ::post(home,ix,yv)));
  }

  void
  mult(Home home, IntVar x0, IntVar x1, IntVar x2,
       IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::MultDom::post(home,x0,x1,x2));
    } else {
      GECODE_ES_FAIL(Arithmetic::MultBnd::post(home,x0,x1,x2));
    }
  }


  void
  divmod(Home home, IntVar x0, IntVar x1, IntVar x2, IntVar x3,
         IntPropLevel) {
    using namespace Int;
    GECODE_POST;

    IntVar prod(home, Int::Limits::min, Int::Limits::max);
    GECODE_ES_FAIL(Arithmetic::MultBnd::post(home,x1,x2,prod));
    Linear::Term<IntView> t[3];
    t[0].a = 1; t[0].x = prod;
    t[1].a = 1; t[1].x = x3;
    int min, max;
    Linear::estimate(t,2,0,min,max);
    IntView x0v(x0);
    GECODE_ME_FAIL(x0v.gq(home,min));
    GECODE_ME_FAIL(x0v.lq(home,max));
    t[2].a=-1; t[2].x=x0;
    Linear::post(home,t,3,IRT_EQ,0,IPL_BND);
    if (home.failed()) return;
    IntView x1v(x1);
    GECODE_ES_FAIL(
      Arithmetic::DivMod<IntView>::post(home,x0,x1,x3));
  }

  void
  div(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel) {
    using namespace Int;
    GECODE_POST;
    GECODE_ES_FAIL(
      (Arithmetic::DivBnd<IntView>::post(home,x0,x1,x2)));
  }

  void
  mod(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    IntVar _div(home, Int::Limits::min, Int::Limits::max);
    divmod(home, x0, x1, _div, x2, ipl);
  }

  void
  sqr(Home home, IntVar x0, IntVar x1, IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    Arithmetic::SqrOps ops;
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::PowDom<Arithmetic::SqrOps>
                     ::post(home,x0,x1,ops));
    } else {
      GECODE_ES_FAIL(Arithmetic::PowBnd<Arithmetic::SqrOps>
                     ::post(home,x0,x1,ops));
    }
  }

  void
  sqrt(Home home, IntVar x0, IntVar x1, IntPropLevel ipl) {
    using namespace Int;
    GECODE_POST;
    Arithmetic::SqrOps ops;
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::NrootDom<Arithmetic::SqrOps>
                     ::post(home,x0,x1,ops));
    } else {
      GECODE_ES_FAIL(Arithmetic::NrootBnd<Arithmetic::SqrOps>
                     ::post(home,x0,x1,ops));
    }
  }

  void
  pow(Home home, IntVar x0, int n, IntVar x1, IntPropLevel ipl) {
    using namespace Int;
    Limits::nonnegative(n,"Int::pow");
    GECODE_POST;
    if (n == 2) {
      sqr(home, x0, x1, ipl);
      return;
    }
    Arithmetic::PowOps ops(n);
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::PowDom<Arithmetic::PowOps>
                     ::post(home,x0,x1,ops));
    } else {
      GECODE_ES_FAIL(Arithmetic::PowBnd<Arithmetic::PowOps>
                     ::post(home,x0,x1,ops));
    }
  }

  void
  nroot(Home home, IntVar x0, int n, IntVar x1, IntPropLevel ipl) {
    using namespace Int;
    Limits::positive(n,"Int::nroot");
    GECODE_POST;
    if (n == 2) {
      sqrt(home, x0, x1, ipl);
      return;
    }
    Arithmetic::PowOps ops(n);
    if (vbd(ipl) == IPL_DOM) {
      GECODE_ES_FAIL(Arithmetic::NrootDom<Arithmetic::PowOps>
                     ::post(home,x0,x1,ops));
    } else {
      GECODE_ES_FAIL(Arithmetic::NrootBnd<Arithmetic::PowOps>
                     ::post(home,x0,x1,ops));
    }
  }

}

// STATISTICS: int-post
