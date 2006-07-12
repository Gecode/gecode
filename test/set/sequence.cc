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
