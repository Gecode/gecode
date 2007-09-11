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

  using namespace Int;
  void 
  sorted(Space* home, const IntVarArgs& x, const IntVarArgs& y,
         IntConLevel, PropKind) {
    if (home->failed())
      return;

    int n  = x.size();
    int n2 = 2*n;

    // construct single tuple for propagation without permutation variables
    ViewArray<ViewTuple<IntView,1> > x0(home, n);
    for (int i = n; i--; ) {
      x0[i][0] = x[i];
    }
    ViewArray<IntView> y0(home, y);

    ViewArray<IntView> xy(home, n2);
    for (int i = 0; i < n; i++) {
      xy[i] = x0[i][0];
    }
    for (int i = n; i < n2; i++) {
      xy[i] = y0[i - n];
    }
    if (xy.shared()) {
      throw ArgumentSame("Int::Sorted");
    }
    if (n != y.size()) {
      throw ArgumentSizeMismatch("Int::Sorted");
    }


    GECODE_ES_FAIL(home,
                   (Sorted::
                    Sorted<IntView, ViewTuple<IntView,1>, false>::
                    post(home, x0, y0)));
  }

  void 
  sorted(Space* home, const IntVarArgs& x, const IntVarArgs& y, 
         const IntVarArgs& z,
         IntConLevel, PropKind) {
    int n = x.size();
    int n2 = 2*n;
    int n3 = 3*n;
    
    if ((n != y.size()) || (n != z.size())) {
      throw ArgumentSizeMismatch("Int::sorted");
    }
    if (home->failed()) {
      return;
    }

    ViewArray<ViewTuple<IntView, 2> > xz0(home, n);

    // assert that permutation variables encode a permutation
    ViewArray<IntView> pz0(home, n);
    ViewArray<IntView> y0(home, y);
    ViewArray<IntView> xyz(home, n3);



    for (int i = n; i--; ) {
      xz0[i][0] = x[i];
      xz0[i][1] = z[i];
      pz0[i]    = z[i];
      // Constrain z_i to a valid index
      GECODE_ME_FAIL(home,xz0[i][1].gq(home,0));
      GECODE_ME_FAIL(home,xz0[i][1].lq(home,n - 1));
    }

    // assert permutation
    distinct(home, z, ICL_BND);

    for (int i = 0; i < n; i++) {
      xyz[i] = xz0[i][0];
    }
    for (int i = n; i < n2; i++) {
      xyz[i] = y0[i - n];
    }
    for (int i = n2; i < n3; i++) {
      xyz[i] = xz0[i - n2][1];
    }

    if (xyz.shared()) {
      throw ArgumentSame("Int::sorted");
    }

    GECODE_ES_FAIL(home,
                   (Sorted::
                    Sorted<IntView, ViewTuple<IntView,2>, true>::
                    post(home, xz0, y0)));
  }
}

// STATISTICS: int-post
