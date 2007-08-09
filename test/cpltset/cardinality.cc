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
