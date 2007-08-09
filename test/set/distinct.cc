/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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
