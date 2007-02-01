/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
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

#ifndef __GECODE_TEST_BRANCH_HH__
#define __GECODE_TEST_BRANCH_HH__

#include "gecode/int.hh"

#include "test.hh"

using namespace Gecode;
using namespace Int;


/**
 * \brief Base class for tests for branching completeness.
 *
 */
class BranchCompleteTest : public Test {
protected:
  /// Number of variables
  int arity;
  /// Domain of variables
  IntSet dom;

  IntConLevel randicl() {
    switch(Test::randgen(4)) {
    case 0: return ICL_DEF;
    case 1: return ICL_VAL;
    case 2: return ICL_BND;
    case 3: return ICL_DOM;
    }
    return ICL_DEF;
  }
public:
  /// Constructor
  BranchCompleteTest(const char* t, int a, const IntSet& d)
    : Test("Branch::Complete",t), arity(a), dom(d) {
  }
  /// Perform test
  virtual bool run(const Options& opt);
  /// Post propagators
  virtual void post(Space* home, IntVarArray& x) = 0;
};


#endif

// STATISTICS: test-branch
