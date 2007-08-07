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

class BddAtmost : public BddTest {
public:
  BddAtmost(const char* t) : BddTest(t,2,ds_13,false) {}
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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::atmost(home, x[0], x[1], 1);
  }
//   virtual void post(Space* home, BddVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddAtmost _bddatmost("Atmost::1");

class BddAtmostLexLe : public BddTest {
public:
  BddAtmostLexLe(const char* t) : BddTest(t,2,ds_13,false) {}
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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::atmostLex(home, x[0], x[1], 1, SRT_LE, SCL_DOM);
  }
};
BddAtmostLexLe _bddatmostlexle("Atmost::Lex::Le");


class BddAtmostIntSet : public BddTest {
public:
  BddAtmostIntSet(const char* t) : BddTest(t,1,ds_33,false) {}
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

  virtual void post(Space* home, BddVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::atmost(home, x[0], ds_33, 1, SCL_DOM);
  }
};
BddAtmostIntSet _bddatmostintset("Atmost::IntSet");

// STATISTICS: test-bdd
