/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Vincent Barichard, 2012
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

#include <gecode/float/arithmetic.hh>

namespace Gecode {

  void
  abs(Home home, FloatVar x0, FloatVar x1) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Abs<FloatView,FloatView>::post(home,x0,x1)));
  }


  void
  max(Home home, FloatVar x0, FloatVar x1, FloatVar x2) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Max<FloatView,FloatView,FloatView>::post(home,x0,x1,x2)));
  }

  void
  max(Home home, const FloatVarArgs& x, FloatVar y) {
    using namespace Float;
    if (x.size() == 0)
      throw TooFewArguments("Float::max");
    GECODE_POST;
    ViewArray<FloatView> xv(home,x);
    GECODE_ES_FAIL(Arithmetic::NaryMax<FloatView>::post(home,xv,y));
  }


  void
  min(Home home, FloatVar x0, FloatVar x1, FloatVar x2) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Min<FloatView,FloatView,FloatView>::post(home,x0,x1,x2)));
  }

  void
  min(Home home, const FloatVarArgs& x, FloatVar y) {
    using namespace Float;
    if (x.size() == 0)
      throw TooFewArguments("Float::min");
    GECODE_POST;
    ViewArray<MinusView> m(home,x.size());
    for (int i=x.size(); i--; )
      m[i] = MinusView(x[i]);
    MinusView my(y);
    GECODE_ES_FAIL(Arithmetic::NaryMax<MinusView>::post(home,m,my));
  }


  void
  mult(Home home, FloatVar x0, FloatVar x1, FloatVar x2) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Mult<FloatView>::post(home,x0,x1,x2)));
  }

  void
  sqr(Home home, FloatVar x0, FloatVar x1) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Sqr<FloatView>::post(home,x0,x1)));
  }

  void
  sqrt(Home home, FloatVar x0, FloatVar x1) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Sqrt<FloatView,FloatView>::post(home,x0,x1)));
  }

  void
  pow(Home home, FloatVar x0, int n, FloatVar x1) {
    using namespace Float;
    if (n < 0)
      throw OutOfLimits("pow");
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::Pow<FloatView,FloatView>::post(home,x0,x1,n)));
  }

  void
  nroot(Home home, FloatVar x0, int n, FloatVar x1) {
    using namespace Float;
    if (n < 0)
      throw OutOfLimits("nroot");
    GECODE_POST;
    GECODE_ES_FAIL((Arithmetic::NthRoot<FloatView,FloatView>::post(home,x0,x1,n)));
  }

  void
  div(Home home, FloatVar x0, FloatVar x1, FloatVar x2) {
    using namespace Float;
    GECODE_POST;
    GECODE_ES_FAIL(
      (Arithmetic::Div<FloatView,FloatView,FloatView>::post(home,x0,x1,x2)));
  }

}

// STATISTICS: float-post
