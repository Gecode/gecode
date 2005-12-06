/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *
 *  Last modified:
 *     $Date: 2005-11-01 16:01:21 +0100 (Tue, 01 Nov 2005) $ by $Author: zayenz $
 *     $Revision: 2465 $
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

#include "test/set.hh"

static const int d1r[4][2] = {
  {-4,-3},{-1,-1},{1,1},{3,5}
};
static IntSet d1(d1r,4);

static IntSet ds_33(-3,3);

class DomEqRange : public SetTest {
public:
  DomEqRange(const char* t) 
    : SetTest(t,1,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr(x.lub, x[0]);
    IntSetRanges dr(ds_33);
    return Iter::Ranges::equal(xr, dr);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::dom(home, x[0], SRT_EQ, ds_33);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::dom(home, x[0], SRT_EQ, ds_33, b);
  }
};
DomEqRange _domeqrange("Dom::EqRange");

class DomEqDom : public SetTest {
public:
  DomEqDom(const char* t) 
    : SetTest(t,1,d1,true,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr(x.lub, x[0]);
    IntSetRanges dr(d1);
    return Iter::Ranges::equal(xr, dr);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::dom(home, x[0], SRT_EQ, d1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::dom(home, x[0], SRT_EQ, d1, b);
  }
};
DomEqDom _domeq("Dom::EqDom");

// STATISTICS: test-set
