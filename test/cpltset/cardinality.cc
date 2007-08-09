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

#include "test/cpltset.hh"

static const int d1r[4][2] = {
  {-4,-3},{-1,-1},{1,1},{3,5}
};
static IntSet d1(d1r,4);

static IntSet ds_33(-3,3);
static IntSet ds_4(4,4);
static IntSet ds_13(1,3);

class BddCardMinMax : public BddTest {
public:
  BddCardMinMax(const char* t) : BddTest(t,1,d1,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[0]);

    int c = 0;
    while (xr1()) {
      c += xr1.width();
      ++xr1;
    }
    return (0<= c && c <= 3);
  }

  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::hls_order(home, x);
    Gecode::cardinality(home, x[0], 0, 3);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddCardMinMax _bddcardminmax("Card::MinMax");

class BddCardEq : public BddTest {
public:
  BddCardEq(const char* t) : BddTest(t,1,d1,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[0]);
    
    int c = 0;
    while (xr1()) {
      c += xr1.width();
      ++xr1;
    }
    return (c == 3);
  }

  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::hls_order(home, x);
    Gecode::cardinality(home, x[0], 3, 3);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddCardEq _bddcardeq("Card::Eq");


class BddCardMinInf : public BddTest {
public:
  BddCardMinInf(const char* t) : BddTest(t,1,d1,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[0]);
    
    int c = 0;
    while (xr1()) {
      c += xr1.width();
      ++xr1;
    }
    return (c >= 1);
  }

  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    // Test lex-bit order
    Gecode::hls_order(home, x);
    Gecode::cardinality(home, x[0], 1, Gecode::Limits::Set::int_max);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddCardMinInf _bddcardmininf("Card::MinInf");

// STATISTICS: test-bdd
