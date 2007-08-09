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

class BddDistinct : public BddTest {
public:
  BddDistinct(const char* t) : BddTest(t,3,ds_13) {}
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
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray&) {
    Gecode::hls_order(home, x);
    Gecode::distinct(home, x);
  }
};
BddDistinct _bdddistinct("Distinct");


// STATISTICS: test-bdd
