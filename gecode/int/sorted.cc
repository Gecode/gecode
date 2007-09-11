/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
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

#include "gecode/int/sorted.hh"

namespace Gecode {

  void 
  sorted(Space* home, const IntVarArgs& x, const IntVarArgs& y,
         IntConLevel, PropKind) {
    using namespace Int;
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::Sorted");
    if (x.same(y))
      throw ArgumentSame("Int::Sorted");
      
    if (home->failed()) return;

    // construct single tuple for propagation without permutation variables
    ViewArray<ViewTuple<IntView,1> > x0(home, x.size());
    for (int i = x.size(); i--; )
      x0[i][0] = x[i];
    ViewArray<IntView> y0(home, y);

    GECODE_ES_FAIL(home,
                   (Sorted::
                    Sorted<IntView, ViewTuple<IntView,1>, false>::
                    post(home, x0, y0)));
  }

  void 
  sorted(Space* home, const IntVarArgs& x, const IntVarArgs& y, 
         const IntVarArgs& z, IntConLevel, PropKind) {
    using namespace Int;
    if ((x.size() != y.size()) || (x.size() != z.size()))
      throw ArgumentSizeMismatch("Int::Sorted");
    if (x.same(y) || x.same(z) || y.same(z))
      throw ArgumentSame("Int::Sorted");
    
    if (home->failed()) return;

    ViewArray<ViewTuple<IntView, 2> > xz0(home, x.size());

    // assert that permutation variables encode a permutation
    ViewArray<IntView> y0(home, y);
    for (int i = x.size(); i--; ) {
      xz0[i][0]=x[i]; xz0[i][1] = z[i];
    }

    GECODE_ES_FAIL(home,
                   (Sorted::
                    Sorted<IntView, ViewTuple<IntView,2>, true>::
                    post(home, xz0, y0)));
  }
}

// STATISTICS: int-post
