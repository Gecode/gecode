/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <algorithm>

namespace Gecode { namespace Int { namespace Distinct {

  forceinline
  EqIte::EqIte(Home home, IntView x0, IntView x1, int d0, int d1)
    : BinaryPropagator<IntView,PC_INT_DOM>(home,x0,x1), c0(d0), c1(d1) {}

  forceinline
  EqIte::EqIte(Space& home, EqIte& p)
    : BinaryPropagator<IntView,PC_INT_DOM>(home,p),
      c0(p.c0), c1(p.c1) {}


  inline ExecStatus
  EqIte::post(Home home, IntView x0, IntView x1, int c0, int c1) {
    assert(!(x0.assigned() && (x0.val() == c0)) && x0.in(c0));
    GECODE_ME_CHECK(x1.lq(home,std::max(x0.max(),c1)));
    GECODE_ME_CHECK(x1.gq(home,std::min(x0.min(),c1)));
    (void) new (home) EqIte(home,x0,x1,c0,c1);
    return ES_OK;
  }

}}}

// STATISTICS: int-prop
