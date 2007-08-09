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

#include "gecode/int/linear.hh"

namespace Gecode {

  using namespace Int;

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel, PropKind pk) {
    if (home->failed()) return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c,ICL_DEF,pk);
  }

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel, PropKind pk) {
    if (home->failed()) return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y,0,ICL_DEF,pk);
  }

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel, PropKind pk) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) 
      return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c,ICL_DEF,pk);
  }

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel, PropKind pk) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) 
      return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y,0,ICL_DEF,pk);
  }

}

// STATISTICS: int-post
