/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date: 2005-10-31 14:43:20 +0100 (Mon, 31 Oct 2005) $ by $Author: zayenz $
 *     $Revision: 2440 $
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

#ifndef __GECODE_TEST_SET_HH__
#define __GECODE_TEST_SET_HH__

#include "../set.hh"
#include "test.hh"
#include "int.hh"

using namespace Gecode;
using namespace Set;

class CountableSetValues {
private:
  IntSetValues dv;
  int cur;
  int i;
public:
  CountableSetValues(void) {}
  CountableSetValues(const IntSet& d0, int cur0) : dv(d0), cur(cur0), i(1) {
    if (! (i & cur))
      operator++();
  }
  void init(const IntSet& d0, int cur0) {
    dv = d0;
    cur = cur0;
    i = 1;
    if (! (i & cur))
      operator++();
  }
  bool operator()(void) const {
    return i<=cur;
  }
  void operator++(void) {
    do {
      ++dv;
      i = i<<1;
    } while (! (i & cur) && i<cur);
  }
  int val(void) const { return dv.val(); }
};

class CountableSetRanges : public Iter::Values::ToRanges<CountableSetValues> {
private:
  CountableSetValues v;
public:
  CountableSetRanges(void) {}
  CountableSetRanges(const IntSet& d, int cur) : v(d, cur) {
    Iter::Values::ToRanges<CountableSetValues>::init(v);
  }
  void init(const IntSet& d, int cur) {
    v.init(d, cur);
    Iter::Values::ToRanges<CountableSetValues>::init(v);
  }
};

class CountableSet {
private:
  IntSet d;
  //  IntSet curd;
  unsigned int cur;
  unsigned int lubmax;
public:
  CountableSet(const IntSet&);
  CountableSet(void) {}
  void init(const IntSet&);
  void reset(void) { cur = 0; }
  bool operator()(void) const { return cur<lubmax; }
  void operator++(void);
  int val(void) const;
};

class SetAssignment {
private:
  int n;
  CountableSet* dsv;
  Assignment ir;
  bool done;
public:
  IntSet lub;
  int withInt;
  SetAssignment(int, const IntSet&, int withInt = 0);
  void reset(void);
  bool operator()(void) const {
    return !done;
  }
  void operator++(void);
  int operator[](int i) const {
    assert((i>=0) && (i<n));
    return dsv[i].val();
  }
  int intval(void) const { return ir[0]; }
  const Assignment& ints(void) const { return ir; }
  int size(void) const {
    return n;
  }
  ~SetAssignment(void) {
    delete [] dsv;
  }
};

std::ostream&
operator<<(std::ostream&, const SetAssignment&);

class SetTest : public Test {
private:
  int     arity;
  IntSet  lub;
  bool    reified;
  int    withInt;

  void removeFromLub(int, SetVar&, int, const IntSet&);
  void addToGlb(int, SetVar&, int, const IntSet&);
  SetAssignment* make_assignment(void);
public:
  SetTest(const char* t, 
	  int a, const IntSet& d, bool r=false, int w=0,
	  int cost = 1) 
    : Test("Set",t,cost), arity(a), lub(d), reified(r), withInt(w)  {
  }
  /// Check for solution
  virtual bool solution(const SetAssignment&) const = 0;
  /// Post propagator
  virtual void post(Space* home, SetVarArray& x, IntVarArray& y) = 0;
  /// Post reified propagator
  virtual void post(Space* home, SetVarArray& x, IntVarArray& y, BoolVar b) {}
  /// Perform test
  virtual bool run(const Options& opt);
};

#endif

// STATISTICS: test-set
