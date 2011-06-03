/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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

#include <gecode/int/no-overlap.hh>

namespace Gecode {

  void
  nooverlap(Home home, 
            const IntVarArgs& x, const IntArgs& w, 
            const IntVarArgs& y, const IntArgs& h,
            IntConLevel) {
    using namespace Int;
    using namespace NoOverlap;
    if (x.same(home,y))
      throw ArgumentSame("Int::nooverlap");
    int n = x.size();
    if ((n != w.size()) || (n != y.size()) || (n != h.size()))
      throw ArgumentSizeMismatch("Int::nooverlap");      
    for (int i=n; i--; ) {
      Limits::nonnegative(w[i],"Int::nooverlap");
      Limits::nonnegative(h[i],"Int::nooverlap");
    }
    if (home.failed()) return;

    Box<IntDim,2>* b 
      = static_cast<Space&>(home).alloc<Box<IntDim,2> >(n);
    for (int i=n; i--; ) {
      b[i][0] = IntDim(x[i],w[i]);
      b[i][1] = IntDim(y[i],h[i]);
    }

    GECODE_ES_FAIL((NoOverlap::Int<2>::post(home,b,n)));
  }

  void
  nooverlap(Home home, 
            const IntVarArgs& x, const IntVarArgs& w, 
            const IntVarArgs& y, const IntVarArgs& h,
            IntConLevel) {
    using namespace Int;
    using namespace NoOverlap;
    if (x.same(home,y) || x.same(home,w) || x.same(home,h))
      throw ArgumentSame("Int::nooverlap");
    int n = x.size();
    if ((n != w.size()) || (n != y.size()) || (n != h.size()))
      throw ArgumentSizeMismatch("Int::nooverlap");      
    if (home.failed()) return;

    for (int i=n; i--; ) {
      GECODE_ME_FAIL(IntView(w[i]).gq(home,0));
      GECODE_ME_FAIL(IntView(h[i]).gq(home,0));
    }

    Box<ViewDim,2>* b 
      = static_cast<Space&>(home).alloc<Box<ViewDim,2> >(n);
    for (int i=n; i--; ) {
      b[i][0] = ViewDim(x[i],w[i]);
      b[i][1] = ViewDim(y[i],h[i]);
    }

    GECODE_ES_FAIL((NoOverlap::View<2>::post(home,b,n)));
  }

}

// STATISTICS: int-post
