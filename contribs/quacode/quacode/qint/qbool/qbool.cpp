/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/int/bool.cpp
 * and is under the same license as given below:
 *
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

#include <gecode/int/bool.hh>
#include <gecode/int/rel.hh>
#include <quacode/qint/qbool.hh>
#include <quacode/qcsp.hh>

namespace Gecode {

  void
  qrel(Home home, QBoolVar qx0, BoolOpType o, QBoolVar qx1, BoolVar x2) {
    using namespace Int;
    if (home.failed()) return;
    switch (o) {
    case BOT_AND:
      {
        NegBoolView n0(qx0.x); NegBoolView n1(qx1.x); NegBoolView n2(x2);
        GECODE_ES_FAIL((Bool::QOr<NegBoolView,NegBoolView,NegBoolView>
                        ::post(home,n0,qx0.q,qx0.r,n1,qx1.q,qx1.r,n2)));
      }
      break;
    case BOT_OR:
      GECODE_ES_FAIL((Bool::QOr<BoolView,BoolView,BoolView>
                      ::post(home,qx0.x,qx0.q,qx0.r,qx1.x,qx1.q,qx1.r,x2)));
      break;
    case BOT_IMP:
      {
        NegBoolView n0(qx0.x);
        GECODE_ES_FAIL((Bool::QOr<NegBoolView,BoolView,BoolView>
                        ::post(home,n0,qx0.q,qx0.r,qx1.x,qx1.q,qx1.r,x2)));
      }
      break;
    case BOT_EQV:
      GECODE_ES_FAIL((Bool::QEqv<BoolView,BoolView,BoolView>
                      ::post(home,qx0,qx1,x2)));
      break;
    case BOT_XOR:
      GECODE_ES_FAIL((Bool::QXorv<BoolView,BoolView,BoolView>
                      ::post(home,qx0,qx1,x2)));
      break;
    default:
      throw UnknownOperation("Int::rel");
    }
  }

  void
  qclause(Home home, BoolOpType o, QBoolVarArgs x, QBoolVarArgs y, int n) {
    using namespace Int;
    if ((n < 0) || (n > 1))
      throw NotZeroOne("Int::rel");
    if (home.failed()) return;
    switch (o) {
    case BOT_AND:
      if (n == 0) {
        x.unique(); y.unique();
        ViewArray<NegBoolView> xv(home,x.size());
        QuantArgs qx(x.size());
        IntArgs rx(x.size());
        for (int i=x.size(); i--; ) {
          NegBoolView n(x[i].x); xv[i]=n; qx[i]=x[i].q; rx[i]=x[i].r;
        }
        ViewArray<BoolView> yv(home,y.size());
        QuantArgs qy(y.size());
        IntArgs ry(y.size());
        for (int i=y.size(); i--; ) {
          yv[i]=y[i].x; qy[i]=y[i].q; ry[i]=y[i].r;
        }
        GECODE_ES_FAIL((Bool::QClauseTrue<NegBoolView,BoolView>
                        ::post(home,xv,qx,rx,yv,qy,ry)));
      } else {
        for (int i=x.size(); i--; ) {
          BoolView b(x[i].x); GECODE_ME_FAIL(b.one(home));
        }
        for (int i=y.size(); i--; ) {
          BoolView b(y[i].x); GECODE_ME_FAIL(b.zero(home));
        }
      }
      break;
    case BOT_OR:
      if (n == 0) {
        for (int i=x.size(); i--; ) {
          BoolView b(x[i].x); GECODE_ME_FAIL(b.zero(home));
        }
        for (int i=y.size(); i--; ) {
          BoolView b(y[i].x); GECODE_ME_FAIL(b.one(home));
        }
      } else {
        x.unique(); y.unique();
        ViewArray<BoolView> xv(home,x.size());
        QuantArgs qx(x.size());
        IntArgs rx(x.size());
        for (int i=x.size(); i--; ) {
          xv[i]=x[i].x; qx[i]=x[i].q; rx[i]=x[i].r;
        }
        ViewArray<NegBoolView> yv(home,y.size());
        QuantArgs qy(y.size());
        IntArgs ry(y.size());
        for (int i=y.size(); i--; ) {
          NegBoolView n(y[i].x); yv[i]=n; qy[i]=y[i].q; ry[i]=y[i].r;
        }
        GECODE_ES_FAIL((Bool::QClauseTrue<BoolView,NegBoolView>
                        ::post(home,xv,qx,rx,yv,qy,ry)));
      }
      break;
    default:
      throw IllegalOperation("Int::clause");
    }
  }

  void
  qclause(Home home, BoolOpType o, QBoolVarArgs x, QBoolVarArgs y, BoolVar z) {
    using namespace Int;
    if (home.failed()) return;
    switch (o) {
    case BOT_AND:
      {
        x.unique(); y.unique();
        ViewArray<NegBoolView> xv(home,x.size());
        QuantArgs qx(x.size());
        IntArgs rx(x.size());
        for (int i=x.size(); i--; ) {
          NegBoolView n(x[i].x); xv[i]=n; qx[i]=x[i].q; rx[i]=x[i].r;
        }
        ViewArray<BoolView> yv(home,y.size());
        QuantArgs qy(y.size());
        IntArgs ry(y.size());
        for (int i=y.size(); i--; ) {
          yv[i]=y[i].x; qy[i]=y[i].q; ry[i]=y[i].r;
        }
        NegBoolView nz(z);
        GECODE_ES_FAIL((Bool::QClause<NegBoolView,BoolView>
                        ::post(home,xv,qx,rx,yv,qy,ry,nz)));
      }
      break;
    case BOT_OR:
      {
        x.unique(); y.unique();
        ViewArray<BoolView> xv(home,x.size());
        QuantArgs qx(x.size());
        IntArgs rx(x.size());
        for (int i=x.size(); i--; ) {
          xv[i]=x[i].x; qx[i]=x[i].q; rx[i]=x[i].r;
        }
        ViewArray<NegBoolView> yv(home,y.size());
        QuantArgs qy(y.size());
        IntArgs ry(y.size());
        for (int i=y.size(); i--; ) {
          NegBoolView n(y[i].x); yv[i]=n; qy[i]=y[i].q; ry[i]=y[i].r;
        }
        GECODE_ES_FAIL((Bool::QClause<BoolView,NegBoolView>
                        ::post(home,xv,qx,rx,yv,qy,ry,z)));
      }
      break;
    default:
      throw IllegalOperation("Int::clause");
    }
  }

}

// STATISTICS: int-post
