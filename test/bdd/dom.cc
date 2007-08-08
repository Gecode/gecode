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
