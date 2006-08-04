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

#include "test/set.hh"

static IntSet ds_012(-1,2);

class SelectUnion : public SetTest {
public:
  SelectUnion(const char* t)
    : SetTest(t,5,ds_012,false) {}
  virtual bool solution(const SetAssignment& x) const {
    int selected = 0;
    for (CountableSetValues sel2(x.lub, x[3]); sel2(); ++sel2, selected++);
    CountableSetValues x4v(x.lub, x[4]);
    if (selected==0)
      return !x4v();
    GECODE_AUTOARRAY(CountableSetRanges, sel, selected);
    CountableSetValues selector(x.lub, x[3]);
    for (int i=selected; i--;++selector) {
      if (selector.val()>=3 || selector.val()<0)
	return false;
      sel[i].init(x.lub, x[selector.val()]);
    }
    Iter::Ranges::NaryUnion<CountableSetRanges> u(sel, selected);

    CountableSetRanges z(x.lub, x[4]);
    return Iter::Ranges::equal(u, z);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(x.size()-2);
    for (int i=x.size()-2; i--;)
      xs[i]=x[i];
    Gecode::selectUnion(home, xs, x[x.size()-2], x[x.size()-1]);
  }
};
SelectUnion _selectunion("Select::Union");

class SelectInter : public SetTest {
public:
  SelectInter(const char* t)
    : SetTest(t,5,ds_012,false) {}
  virtual bool solution(const SetAssignment& x) const {
    int selected = 0;
    for (CountableSetValues sel2(x.lub, x[3]); sel2(); ++sel2, selected++);
    CountableSetRanges x4r(x.lub, x[4]);
    if (selected==0)
      return Iter::Ranges::size(x4r)==Limits::Set::card_max;
    GECODE_AUTOARRAY(CountableSetRanges, sel, selected);
    CountableSetValues selector(x.lub, x[3]);
    for (int i=selected; i--;++selector) {
      if (selector.val()>=3 || selector.val()<0)
	return false;
      sel[i].init(x.lub, x[selector.val()]);
    }
    Iter::Ranges::NaryInter<CountableSetRanges> u(sel, selected);

    CountableSetRanges z(x.lub, x[4]);
    return Iter::Ranges::equal(u, z);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(x.size()-2);
    for (int i=x.size()-2; i--;)
      xs[i]=x[i];
    Gecode::selectInter(home, xs, x[x.size()-2], x[x.size()-1]);
  }
};
SelectInter _selectinter("Select::Inter");

class SelectSet : public SetTest {
public:
  SelectSet(const char* t)
    : SetTest(t,4,ds_012,false,true) {}
  virtual bool solution(const SetAssignment& x) const {
    if (x.intval() < 0 || x.intval() > 2)
      return false;
    CountableSetRanges z(x.lub, x[3]);
    CountableSetRanges y(x.lub, x[x.intval()]);
    return Iter::Ranges::equal(y, z);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
    SetVarArgs xs(x.size()-1);
    for (int i=x.size()-1; i--;)
      xs[i]=x[i];
    Gecode::selectSet(home, xs, y[0], x[x.size()-1]);
  }
};
SelectSet _selectset("Select::Set");

// STATISTICS: test-set
