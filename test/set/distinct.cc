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

static IntSet ds_33(-2,2);
static IntSet ds_44(-4,4);

class AtmostOne : public SetTest {
public:
  AtmostOne(const char* t) 
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr0,xr1);
      if (Iter::Ranges::size(i)>1)
	return false;
    }
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr0,xr2);
      if (Iter::Ranges::size(i)>1)
	return false;
    }
    {
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr1,xr2);
      if (Iter::Ranges::size(i)>1)
	return false;
    }
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      if (Iter::Ranges::size(xr0)!=3 ||
	  Iter::Ranges::size(xr1)!=3 ||
	  Iter::Ranges::size(xr2)!=3)
	return false;
    }
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVar s1(home, IntSet::empty, -2, 2, 0,1);
    Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_EQ, s1);
    SetVar s2(home, IntSet::empty, -2, 2, 0,1);
    Gecode::rel(home, x[0], SOT_INTER, x[2], SRT_EQ, s2);
    SetVar s3(home, IntSet::empty, -2, 2, 0,1);
    Gecode::rel(home, x[1], SOT_INTER, x[2], SRT_EQ, s3);
    Gecode::atmostOne(home, x, 3);
  }
};
AtmostOne _atmostone("Distinct::AtmostOne");

class Distinct : public SetTest {
public:
  Distinct(const char* t) 
    : SetTest(t,4,ds_44,false) {}
  virtual bool solution(const SetAssignment& x) const {
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr0,xr1);
      if (!i())
	return false;
    }
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr0,xr2);
      if (!i())
	return false;
    }
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr3(x.lub, x[3]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr0,xr3);
      if (!i())
	return false;
    }
    {
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr1,xr2);
      if (!i())
	return false;
    }
    {
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr3(x.lub, x[3]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr1,xr3);
      if (!i())
	return false;
    }
    {
      CountableSetRanges xr2(x.lub, x[2]);
      CountableSetRanges xr3(x.lub, x[3]);
      Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> i(xr2,xr3);
      if (!i())
	return false;
    }
    {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      CountableSetRanges xr3(x.lub, x[3]);
      if (Iter::Ranges::size(xr0)!=3 ||
	  Iter::Ranges::size(xr1)!=3 ||
	  Iter::Ranges::size(xr2)!=3 ||
	  Iter::Ranges::size(xr3)!=3)
	return false;
    }
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_NQ, x[1]);
    Gecode::rel(home, x[0], SRT_NQ, x[2]);
    Gecode::rel(home, x[0], SRT_NQ, x[3]);
    Gecode::rel(home, x[1], SRT_NQ, x[2]);
    Gecode::rel(home, x[1], SRT_NQ, x[3]);
    Gecode::rel(home, x[2], SRT_NQ, x[3]);
    Gecode::atmostOne(home, x, 3);
  }
};
Distinct _distinct("Distinct::Distinct");

// STATISTICS: test-set
