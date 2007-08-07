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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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

class BddRelLe : public BddTest {
public:
  BddRelLe(const char* t) : BddTest(t,2,ds_13,false) {}

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
  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::rel(home, x[0], SRT_LE, x[1],SCL_DOM);
  }
//   virtual void post(Space* home, BddVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_LE, d1, b,SCL_DOM);
//   }
};
BddRelLe _bddrelle("Rel::Le");

class BddRelLq : public BddTest {
public:
  BddRelLq(const char* t) : BddTest(t,2,ds_13,false) {}

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
  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::rel(home, x[0], SRT_LQ, x[1],SCL_DOM);
  }
//   virtual void post(Space* home, BddVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_LQ, d1, b,SCL_DOM);
//   }
};
BddRelLq _bddrellq("Rel::Lq");


class BddRelLeDiff : public BddTest {
public:
  BddRelLeDiff(const char* t) : BddTest(t,2,ds_33,false) {}
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
  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::dom(home, x[0], SRT_DISJ, ds_13);
    Gecode::dom(home, x[1], SRT_SUB, ds_13);
    Gecode::rel(home, x[0], SRT_LE, x[1],SCL_DOM);
  }
//   virtual void post(Space* home, BddVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b,SCL_DOM);
//   }
};
BddRelLe _bddrellediff("Rel::Le::Diff");


class BddRelDisj : public BddTest {
public:
  BddRelDisj(const char* t) : BddTest(t,2,ds_13,false) {}

  virtual bool solution(const SetAssignment& x) const {
    // std::cerr << "solution  reldisj\n";
    CountableSetRanges xr1(x.lub, x[0]);
    CountableSetRanges xr2(x.lub, x[1]);
    Gecode::Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> inter(xr1, xr2);
    // if (!xr1() && !xr2()) return true;
    return !inter();
  }
  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::rel(home, x[0], SRT_DISJ, x[1], SCL_DOM);
  }
};
BddRelDisj _bddreldisj("Rel::Disj");

// STATISTICS: test-bdd
