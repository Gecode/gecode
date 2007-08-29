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

static IntSet ds_012(-1,2);

class SelectUnion : public SetTest {
public:
  /// Create and register test
  SelectUnion(const char* t)
    : SetTest(t,5,ds_012,false) {}
  /// Test whether \a x is solution
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
  /// Post constraint on \a x
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
  /// Create and register test
  SelectInter(const char* t)
    : SetTest(t,5,ds_012,false) {}
  /// Test whether \a x is solution
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
  /// Post constraint on \a x
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
  /// Create and register test
  SelectSet(const char* t)
    : SetTest(t,4,ds_012,false,true) {}
  /// Test whether \a x is solution
  virtual bool solution(const SetAssignment& x) const {
    if (x.intval() < 0 || x.intval() > 2)
      return false;
    CountableSetRanges z(x.lub, x[3]);
    CountableSetRanges y(x.lub, x[x.intval()]);
    return Iter::Ranges::equal(y, z);
  }
  /// Post constraint on \a x
  virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
    SetVarArgs xs(x.size()-1);
    for (int i=x.size()-1; i--;)
      xs[i]=x[i];
    Gecode::selectSet(home, xs, y[0], x[x.size()-1]);
  }
};
SelectSet _selectset("Select::Set");

// STATISTICS: test-set
