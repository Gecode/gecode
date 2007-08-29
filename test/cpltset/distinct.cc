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

#include "test/bdd.hh"

static const int d1r[4][2] = {
  {-4,-3},{-1,-1},{1,1},{3,5}
};
static IntSet d1(d1r,4);

static IntSet ds_33(-3,3);
static IntSet ds_4(4,4);
static IntSet ds_13(1,3);

class BddDistinct : public BddTest {
public:
  /// Create and register test
  BddDistinct(const char* t) : BddTest(t,3,ds_13) {}
  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      if (Gecode::Iter::Ranges::equal(xr0, xr1))
        return false;
    }
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr2(x.lub, x[2]);
      if (Gecode::Iter::Ranges::equal(xr0, xr2))
        return false;
    }
    {
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      if (Gecode::Iter::Ranges::equal(xr1, xr2))
        return false;
    }
    return true;
  }
  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    Gecode::hls_order(home, x);
    Gecode::distinct(home, x);
  }
};
BddDistinct _bdddistinct("Distinct");


// STATISTICS: test-bdd
