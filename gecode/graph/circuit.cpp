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

#include <gecode/graph/circuit.hh>

namespace Gecode {

  void
  circuit(Space& home, const IntVarArgs& x, IntConLevel icl) {
    if (x.same(home))
      throw Int::ArgumentSame("Graph::circuit");
    if (x.size() == 0)
      throw Int::TooFewArguments("Graph::circuit");
    if (home.failed()) return;
    ViewArray<Int::IntView> xv(home,x);
    if (icl == ICL_DOM) {
      GECODE_ES_FAIL(home,Graph::Circuit::Dom<Int::IntView>::post(home,xv));
    } else {
      GECODE_ES_FAIL(home,Graph::Circuit::Val<Int::IntView>::post(home,xv));
    }
  }

  void
  circuit(Space& home, const IntArgs& c, 
          const IntVarArgs& x, const IntVarArgs& y, IntVar z, 
          IntConLevel icl) {
    int n = x.size();
    if ((y.size() != n) || (c.size() != n*n))
      throw Int::ArgumentSizeMismatch("Graph::circuit");
    circuit(home, x, icl);
    if (home.failed()) return;
    IntArgs cx(n);
    for (int i=n; i--; ) {
      for (int j=0; j<n; j++)
        cx[j] = c[i*n+j];
      element(home, cx, x[i], y[i]);
    }
    linear(home, y, IRT_EQ, z);
  }

  void
  circuit(Space& home, const IntArgs& c, 
          const IntVarArgs& x, IntVar z, 
          IntConLevel icl) {
    if (home.failed()) return;
    IntVarArgs y(x.size());
    for (int i=x.size(); i--; )
      y[i].init(home, Int::Limits::min, Int::Limits::max);
    circuit(home, c, x, y, z);
  }

}

// STATISTICS: graph-post
