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

#include "test/cpltset.hh"

static const int d1r[4][2] = {
  {-4,-3},{-1,-1},{1,1},{3,5}
};
static IntSet d1(d1r,4);

static IntSet ds_33(-3,3);
static IntSet ds_4(4,4);
static IntSet ds_13(1,3);

class BddAtmost : public BddTest {
public:
  /// Create and register test
  BddAtmost(const char* t) : BddTest(t,2,ds_13,false) {}
  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[0]);
    CountableSetRanges xr2(x.lub, x[1]);

    Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
      d(xr1, xr2);
    int c = 0;
    while (d()) {
      c += d.width();
      ++d;
    }
    return (c <= 1);
  }

  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::atmost(home, x[0], x[1], 1);
  }
/// Post reified constraint on \a x for \a b
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddAtmost _bddatmost("Atmost::1");

class BddAtmostLexLe : public BddTest {
public:
  /// Create and register test
  BddAtmostLexLe(const char* t) : BddTest(t,2,ds_13,false) {}
  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[0]);
    CountableSetRanges xr2(x.lub, x[1]);

    Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
      d(xr1, xr2);
    int c = 0;
    while (d()) {
      c += d.width();
      ++d;
    }

    CountableSetValues xv1(x.lub, x[0]);
    CountableSetValues xv2(x.lub, x[1]);
    int a = iter2int(xv1, 3);
    int b = iter2int(xv2, 3);
    
    return (c <= 1) && a < b;
  }

  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::atmostLex(home, x[0], x[1], 1, SRT_LE);
  }
};
BddAtmostLexLe _bddatmostlexle("Atmost::Lex::Le");


class BddAtmostIntSet : public BddTest {
public:
  /// Create and register test
  BddAtmostIntSet(const char* t) : BddTest(t,1,ds_33,false) {}
  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[0]);
    IntSetRanges ir(ds_33);
    Iter::Ranges::Inter<CountableSetRanges, IntSetRanges>  d(xr1, ir);
    int c = 0;
    while (d()) {
      c += d.width();
      ++d;
    }
    
    return (c <= 1);
  }

  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::atmost(home, x[0], ds_33, 1);
  }
};
BddAtmostIntSet _bddatmostintset("Atmost::IntSet");

// STATISTICS: test-bdd
