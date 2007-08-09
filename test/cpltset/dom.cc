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

class BddDomEqRange : public BddTest {
public:
  BddDomEqRange(const char* t) : BddTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr(x.lub, x[0]);
    IntSetRanges dr(ds_33);
    return Iter::Ranges::equal(xr, dr);
  }
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    Gecode::dom(home, x[0], SRT_EQ, ds_33);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, ds_33, b);
//   }
};
BddDomEqRange _bdddomeqrange("Dom::EqRange");

class BddDomEqDom : public BddTest {
public:
  BddDomEqDom(const char* t) : BddTest(t,1,d1,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr(x.lub, x[0]);
    IntSetRanges dr(d1);
    return Iter::Ranges::equal(xr, dr);
  }
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    Gecode::dom(home, x[0], SRT_EQ, d1);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_EQ, d1, b);
//   }
};
BddDomEqDom _bdddomeq("Dom::EqDom");



class BddDomSupRange : public BddTest {
public:
  BddDomSupRange(const char* t) : BddTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr(x.lub, x[0]);
    IntSetRanges dr(ds_33);
    return Iter::Ranges::subset(dr, xr);
  }
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    Gecode::dom(home, x[0], SRT_SUP, ds_33);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_SUP, ds_33, b);
//   }
};
BddDomSupRange _bdddomsuprange("Dom::SupRange");

class BddDomSupDom : public BddTest {
public:
  BddDomSupDom(const char* t) : BddTest(t,1,d1,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr(x.lub, x[0]);
    IntSetRanges dr(d1);
    return Iter::Ranges::subset(dr, xr);
  }
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    Gecode::dom(home, x[0], SRT_SUP, d1);
  }
//   virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&, BoolVar b) {
//     Gecode::dom(home, x[0], SRT_SUP, d1, b);
//   }
};
BddDomSupDom _bdddomsup("Dom::SupDom");

// STATISTICS: test-bdd
