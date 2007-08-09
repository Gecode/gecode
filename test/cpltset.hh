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

#ifndef __GECODE_TEST_BDD_HH__
#define __GECODE_TEST_BDD_HH__

#include "gecode/cpltset.hh"
#include "test.hh"
#include "int.hh"
#ifdef GECODE_HAVE_SET_VARS
#include "set.hh"
#endif
#include "log.hh"

using namespace Gecode;
using namespace CpltSet;

#ifdef GECODE_HAVE_SET_VARS

#else

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

#endif

class BddTest : public Test {
private:
  BuddyMgr   mgr;
  int     arity;
  IntSet  lub;
  bool    reified;
  int    withInt;
  int ivs;
  int ics;

  void removeFromLub(int, CpltSetVar&, int, const IntSet&);
  void addToGlb(int, CpltSetVar&, int, const IntSet&);
  SetAssignment* make_assignment(void);
public:
  BddTest(const char* t, int a, const IntSet& d, bool r=false, int w=0, 
	  int mn=10000, int mc=1000) 
    : Test("Bdd",t), arity(a), lub(d), reified(r), withInt(w), 
      ivs(mn), ics(mc)  {}

  /// Check for solution
  virtual bool solution(const SetAssignment&) const = 0;
  /// Post propagator
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray& y) = 0;
  /// Post reified propagator
  virtual void post(Space* home, CpltSetVarArray& x, IntVarArray& y, BoolVar b) {}
  /// Perform test
  virtual bool run(const Options& opt);

  template <class I>
  int iter2int(I& r, int u) const{
    if (!r()) {
      return 0;
    }
    int v = 0;
    // compute the bit representation of the assignment 
    // and convert it to the corresponding integer
    while(r()) {
      v  |= (1 << r.val()); // due to reversed lex ordering
      ++r;
    }
    return v;
  }
  
  /// Provide manager access
  BuddyMgr& manager(void) { return mgr; }
  void manager( BuddyMgr& m) { mgr = m; }
  int varsize(void) { return ivs; }
  int cachesize(void) { return ics; }

};

#endif

// STATISTICS: test-bdd
