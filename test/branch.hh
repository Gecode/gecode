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

#ifndef __GECODE_TEST_BRANCH_HH__
#define __GECODE_TEST_BRANCH_HH__

#include "gecode/int.hh"

#include "test/test.hh"

namespace Test {

using namespace Gecode;
using namespace Int;


/**
 * \brief Base class for tests for branching completeness.
 *
 */
class BranchCompleteTest : public Base {
protected:
  /// Number of variables
  int arity;
  /// Domain of variables
  IntSet dom;

  IntConLevel randicl(void) {
    switch (Base::rand(4)) {
    case 0: return ICL_DEF;
    case 1: return ICL_VAL;
    case 2: return ICL_BND;
    case 3: return ICL_DOM;
    }
    return ICL_DEF;
  }
public:
  /// Constructor
  BranchCompleteTest(const std::string& s, int a, const IntSet& d)
    : Base("Branch::Complete::"+s), arity(a), dom(d) {
  }
  /// Perform test
  virtual bool run(const Options& opt);
  /// Post propagators
  virtual void post(Space* home, IntVarArray& x) = 0;
};

}

#endif

// STATISTICS: test-branch
