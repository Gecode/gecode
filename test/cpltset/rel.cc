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

class BddRelLe : public BddTest {
public:
  /// Create and register test
  BddRelLe(const char* t) : BddTest(t,2,ds_13,false) {}

  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    CountableSetValues xr1(x.lub, x[0]);
    CountableSetValues xr2(x.lub, x[1]);
    // in lex-bit order empty is the smallest
    // std::cout << x[0] << " and " << x[1] << "\n";
    int a = (iter2int(xr1, 3));
    int b = (iter2int(xr2, 3));
    // std::cout << a << " < " << b << "\n";
    return a < b;
//     return false;
  }
  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::rel(home, x[0], SRT_LE, x[1]);
  }
/// Post reified constraint on \a x for \a b
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_LE, d1, b,SCL_DOM);
//   }
};
BddRelLe _bddrelle("Rel::Le");

class BddRelLq : public BddTest {
public:
  /// Create and register test
  BddRelLq(const char* t) : BddTest(t,2,ds_13,false) {}

  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    CountableSetValues xr1(x.lub, x[0]);
    CountableSetValues xr2(x.lub, x[1]);
    // in lex-bit order empty is the smallest
    // std::cout << x[0] << " and " << x[1] << "\n";
    int a = (iter2int(xr1, 3));
    int b = (iter2int(xr2, 3));
    // std::cout << a << " < " << b << "\n";
    return a <= b;
//     return false;
  }
  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::rel(home, x[0], SRT_LQ, x[1]);
  }
/// Post reified constraint on \a x for \a b
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_LQ, d1, b,SCL_DOM);
//   }
};
BddRelLq _bddrellq("Rel::Lq");


class BddRelLeDiff : public BddTest {
public:
  /// Create and register test
  BddRelLeDiff(const char* t) : BddTest(t,2,ds_33,false) {}
  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    CountableSetValues xr1(x.lub, x[0]);
    CountableSetValues xr2(x.lub, x[1]);
    // in lex-bit order empty is the smallest
    // std::cout << x[0] << " and " << x[1] << "\n";
    int a = (iter2int(xr1,3));
    int b = (iter2int(xr2,3));
    // std::cout << a << " < " << b << "\n";
    return a < b;
//     return false;
  }
  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::dom(home, x[0], SRT_DISJ, ds_13);
    Gecode::dom(home, x[1], SRT_SUB, ds_13);
    Gecode::rel(home, x[0], SRT_LE, x[1]);
  }
/// Post reified constraint on \a x for \a b
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b,SCL_DOM);
//   }
};
BddRelLe _bddrellediff("Rel::Le::Diff");


class BddRelDisj : public BddTest {
public:
  /// Create and register test
  BddRelDisj(const char* t) : BddTest(t,2,ds_13,false) {}

  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    // std::cerr << "solution  reldisj\n";
    CountableSetRanges xr1(x.lub, x[0]);
    CountableSetRanges xr2(x.lub, x[1]);
    Gecode::Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> inter(xr1, xr2);
    // if (!xr1() && !xr2()) return true;
    return !inter();
  }
  /// Post constraint on \a x
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::rel(home, x[0], SRT_DISJ, x[1]);
  }
};
BddRelDisj _bddreldisj("Rel::Disj");

// STATISTICS: test-bdd
