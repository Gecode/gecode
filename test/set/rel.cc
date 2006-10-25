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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "test/set.hh"

static IntSet ds_33(-3,3);

class RelBinNEq : public SetTest {
public:
  RelBinNEq(const char* t)
    : SetTest(t,2,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return !Iter::Ranges::equal(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_NQ, x[1]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_NQ, x[1], b);
  }
};
RelBinNEq _relneq("Rel::BinNEq");

class RelBinNEqS : public SetTest {
public:
  RelBinNEqS(const char* t)
    : SetTest(t,1,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    return false;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_NQ, x[0]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_NQ, x[0], b);
  }
};
RelBinNEqS _relneqs("Rel::Sharing::BinNEqS");

class RelBinEq : public SetTest {
public:
  RelBinEq(const char* t)
    : SetTest(t,2,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::equal(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_EQ, x[1]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_EQ, x[1], b);
  }
};
RelBinEq _releq("Rel::BinEq");

class RelBinEqS : public SetTest {
public:
  RelBinEqS(const char* t)
    : SetTest(t,1,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_EQ, x[0]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_EQ, x[0], b);
  }
};
RelBinEqS _releqs("Rel::Sharing::BinEqS");

class RelBinSub : public SetTest {
public:
  RelBinSub(const char* t)
    : SetTest(t,2,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_SUB, x[1]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_SUB, x[1], b);
  }
};
RelBinSub _relsub("Rel::BinSub");

class RelBinSubS : public SetTest {
public:
  RelBinSubS(const char* t)
    : SetTest(t,1,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_SUB, x[0]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_SUB, x[0], b);
  }
};
RelBinSubS _relsubs("Rel::Sharing::BinSubS");

class RelBinDisj : public SetTest {
public:
  RelBinDisj(const char* t)
    : SetTest(t,2,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::disjoint(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_DISJ, x[1]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_DISJ, x[1], b);
  }
};
RelBinDisj _reldisj("Rel::BinDisj");

class RelBinDisjS : public SetTest {
public:
  RelBinDisjS(const char* t)
    : SetTest(t,1,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    return !xr0();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_DISJ, x[0]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_DISJ, x[0], b);
  }
};
RelBinDisjS _reldisjs("Rel::Sharing::BinDisjS");

class RelBinCompl : public SetTest {
public:
  RelBinCompl(const char* t)
    : SetTest(t,2,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    Set::RangesCompl<CountableSetRanges> xr1c(xr1);
    return Iter::Ranges::equal(xr0, xr1c);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_CMPL, x[1]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_CMPL, x[1], b);
  }
};
RelBinCompl _relcompl("Rel::BinCompl");

class RelBinComplS : public SetTest {
public:
  RelBinComplS(const char* t)
    : SetTest(t,1,ds_33,true) {}
  virtual bool solution(const SetAssignment& x) const {
    return false;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SRT_CMPL, x[0]);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
    Gecode::rel(home, x[0], SRT_CMPL, x[0], b);
  }
};
RelBinComplS _relcompls("Rel::Sharing::BinComplS");

// STATISTICS: test-set
