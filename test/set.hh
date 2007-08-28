/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#ifndef __GECODE_TEST_SET_HH__
#define __GECODE_TEST_SET_HH__

#include "gecode/set.hh"
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
  CpltAssignment ir;
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

class SetTest : public TestBase {
private:
  int     arity;
  IntSet  lub;
  bool    reified;
  int    withInt;

  void removeFromLub(int, SetVar&, int, const IntSet&);
  void addToGlb(int, SetVar&, int, const IntSet&);
  SetAssignment* make_assignment(void);
public:
  SetTest(const std::string& s,
          int a, const IntSet& d, bool r=false, int w=0)
    : TestBase("Set::"+s), arity(a), lub(d), reified(r), withInt(w)  {}
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
