/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <Chris.Mears@monash.edu>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christopher Mears, 2011
 *     Christian Schulte, 2011
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/int/precede.hh>

namespace Gecode {

  void
  precede(Home home, const IntVarArgs& x, int s, int t, IntPropLevel) {
    using namespace Int;
    Limits::check(s,"Int::precede");
    Limits::check(t,"Int::precede");
    GECODE_POST;

    ViewArray<IntView> y(home, x);
    GECODE_ES_FAIL(Precede::Single<IntView>::post(home, y, s, t));
  }

  void
  precede(Home home, const IntVarArgs& x, const IntArgs& c, IntPropLevel) {
    using namespace Int;
    if (c.size() < 2)
      return;
    for (int i=c.size(); i--; )
      Limits::check(c[i],"Int::precede");
    GECODE_POST;

    for (int i=c.size()-1; i--; ) {
      ViewArray<IntView> y(home, x);
      GECODE_ES_FAIL(Precede::Single<IntView>::post(home, y, c[i], c[i+1]));
    }
  }

  void
  precede(Home home, OpenIntVarList list,
          int before, int after, IntPropLevel) {
    Int::Limits::check(before,"Int::precede");
    Int::Limits::check(after,"Int::precede");
    GECODE_POST;
    Int::Precede::Open<Int::IntView>::post
      (home,OpenViewList<Int::IntView>(list),before,after);
  }

  void
  precede(Home home, OpenIntVarList list,
          const IntArgs& values, IntPropLevel ipl) {
    if (values.size() < 2)
      return;
    for (int i=values.size(); i--; )
      Int::Limits::check(values[i],"Int::precede");
    GECODE_POST;
    for (int i=values.size()-1; i--; )
      Int::Precede::Open<Int::IntView>::post
        (home,OpenViewList<Int::IntView>(list),values[i],values[i+1]);
    (void) ipl;
  }

}

// STATISTICS: int-post

