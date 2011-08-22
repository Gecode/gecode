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

#include <gecode/int/member.hh>

namespace Gecode {

  void
  member(Home home, const IntVarArgs& x, IntVar y,
         IntConLevel) {
    using namespace Int;
    if (home.failed()) return;

    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(Member::Prop<IntView>::post(home,xv,y));
  }

  void
  member(Home home, const BoolVarArgs& x, BoolVar y,
         IntConLevel) {
    using namespace Int;
    if (home.failed()) return;

    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(Member::Prop<BoolView>::post(home,xv,y));
  }

  void
  member(Home home, const IntVarArgs& x, IntVar y, BoolVar b,
         IntConLevel) {
    using namespace Int;
    if (home.failed()) return;

    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(Member::ReProp<IntView>::post(home,xv,y,b));
  }

  void
  member(Home home, const BoolVarArgs& x, BoolVar y, BoolVar b,
         IntConLevel) {
    using namespace Int;
    if (home.failed()) return;

    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(Member::ReProp<BoolView>::post(home,xv,y,b));
  }

}

// STATISTICS: int-post

