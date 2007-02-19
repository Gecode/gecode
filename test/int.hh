/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

#include "gecode/int.hh"
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

class RandomAssignment : public Assignment {
protected:
  int *vals;
  int count;
  int left;
  virtual int randval(void);
public:
  RandomAssignment(int, const IntSet&, int);
  virtual void reset(void);
  virtual bool operator()(void) const {
    return left > 0;
  }
  virtual int  operator[](int i) const {
    assert((i>=0) && (i<n));
    return vals[i];
  }
  virtual void operator++(void);
  virtual ~RandomAssignment(void) {
    delete [] vals;
  }
};

std::ostream&
operator<<(std::ostream&, const Assignment&);

/**
 * \brief Base class for tests with integer constraints
 *
 */
class IntTest : public Test {
protected:
  /// Number of variables
  int arity;
  /// Domain of variables
  IntSet dom;
  /// Does the constraint also exist as reified constraint
  bool reified;
  /// Consistency level of the propagator
  IntConLevel icl;
  /// Whether to test for domain-consistency
  bool testdom;

  virtual Assignment* make_assignment(void);
  virtual bool do_search_test(void) { return true; }
public:
  /// Constructor
  IntTest(const char* t, int a, const IntSet& d,
          bool r=false, IntConLevel i=ICL_DEF,
          bool td=true)
    : Test("Int",t), arity(a), dom(d), reified(r), icl(i), testdom(td)  {
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

