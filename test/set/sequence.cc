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

static IntSet ds_33(-1,2);

class Sequence : public SetTest {
public:
  Sequence(const char* t)
    : SetTest(t,4,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    int max = Limits::Set::int_min - 1;
    for (int i=0; i<4; i++) {
      CountableSetRanges xir(x.lub, x[i]);
      IntSet xi(xir);
      if (xi.size() > 0) {
        int oldMax = max;
        max = xi.max();
        if (xi.min() <= oldMax)
          return false;
      }
    }
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::sequence(home, x);
  }
};
Sequence _sequence("Sequence::Sequence");

class SeqU : public SetTest {
public:
  SeqU(const char* t)
    : SetTest(t,4,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    int max = Limits::Set::int_min - 1;
    for (int i=0; i<3; i++) {
      CountableSetRanges xir(x.lub, x[i]);
      IntSet xi(xir);
      if (xi.size() > 0) {
        int oldMax = max;
        max = xi.max();
        if (xi.min() <= oldMax)
          return false;
      }
    }
    GECODE_AUTOARRAY(CountableSetRanges, isrs, 3);
    isrs[0].init(x.lub, x[0]);
    isrs[1].init(x.lub, x[1]);
    isrs[2].init(x.lub, x[2]);
    Iter::Ranges::NaryUnion<CountableSetRanges> u(isrs, 3);
    CountableSetRanges x3r(x.lub, x[3]);
    return Iter::Ranges::equal(u, x3r);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(x.size()-1);
    for (int i=x.size()-1; i--;)
      xs[i]=x[i];
    Gecode::sequentialUnion(home, xs, x[x.size()-1]);
  }
};
SeqU _sequ("Sequence::SeqU");

// STATISTICS: test-set
