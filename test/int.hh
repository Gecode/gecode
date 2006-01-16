/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_TEST_INT_HH__
#define __GECODE_TEST_INT_HH__

#include "../int.hh"
#include "test.hh"

using namespace Gecode;
using namespace Int;

class Assignment {
protected:
  int n;
  IntSetValues* dsv;
  IntSet d;
  bool done;
public:
  Assignment(int, const IntSet&);
  virtual void reset(void);
  virtual bool operator()(void) const {
    return !done;
  }
  virtual void operator++(void);
  virtual int  operator[](int i) const {
    assert((i>=0) && (i<n));
    return dsv[i].val();
  }
  int size(void) const {
    return n;
  }
  virtual ~Assignment(void) {
    delete [] dsv;
  }
};

std::ostream&
operator<<(std::ostream&, const Assignment&);

class IntTest : public Test {
protected:
  int    arity;
  IntSet dom;
  bool   reified;
  bool   domain;

  virtual Assignment* make_assignment();
  virtual bool do_search_test() { return true; }
public:
  IntTest(const char* t, 
	  int a, const IntSet& d, bool r=false, int cost = 1,
	  bool dom=false) 
    : Test("Int",t,cost), arity(a), dom(d), reified(r), domain(dom)  {
  }
  /// Check for solution
  virtual bool solution(const Assignment&) const = 0;
  /// Post propagator
  virtual void post(Space* home, IntVarArray& x) = 0;
  /// Post reified propagator
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {}
  /// Perform test
  virtual bool run(const Options& opt);
};

#endif

// STATISTICS: test-int

