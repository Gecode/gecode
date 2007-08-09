/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

#include "test/set.hh"

static IntSet ds_33(-4,4);

class Convex : public SetTest {
public:
  Convex(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    if (!xr0())
      return true;
    ++xr0;
    if (!xr0())
      return true;
    return false;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::convex(home, x[0]);
  }
};
Convex _convex("Convex::Convex");

class ConvexHull : public SetTest {
public:
  ConvexHull(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);

    if (!xr0())
      return !xr1();

    int x0min = xr0.min();
    int x0max = xr0.max();
    ++xr0;
    if (!xr0()) {
      if (!xr1()) return false;
      if (x0min != xr1.min()) return false;
      int x1max = Limits::Set::int_min;
      while (xr1()) { x1max = xr1.max(); ++xr1;}
      if (x0max != x1max) return false;
      return true;
    }
    return false;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::convexHull(home, x[1], x[0]);
  }
};
ConvexHull _convexhull("Convex::ConvexHull");

class ConvexHullS : public SetTest {
public:
  ConvexHullS(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    if (!xr0())
      return true;
    ++xr0;
    if (!xr0())
      return true;
    return false;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::convexHull(home, x[0], x[0]);
  }
};
ConvexHullS _convexhulls("Convex::Sharing::ConvexHullS");

// STATISTICS: test-set
