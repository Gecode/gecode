/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date: 2005-11-07 11:52:03 +0100 (mån, 07 nov 2005) $ by $Author: zayenz $
 *     $Revision: 2511 $
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

#ifndef __GECODE_TEST_STRESS_HH__
#define __GECODE_TEST_STRESS_HH__

#include "kernel.hh"
#include "../int.hh"
#include "test.hh"

using namespace Gecode;
using namespace Int;

class StressTestSpace : public Space {
public:
  IntVarArray x;
private:
  const Options opt;
public:
  StressTestSpace(int n, IntSet dom, const Options& o);
  StressTestSpace(bool share, StressTestSpace& s);
  virtual Space* copy(bool share);
  bool is_failed(void);
};

enum CheckResult {
  CR_OK,
  CR_FOS,
  CR_SOF,
  CR_UNSPEC
};

class StressTest : public Test {
protected:
  IntSetValues param;
public:
  StressTest(const char* t, const IntSet& p) 
    : Test("Stress", t), param(p) {}
  // Check for solution
  virtual CheckResult check(const StressTestSpace *s) const = 0;
  // Run test-space
  virtual StressTestSpace *run(StressTestSpace *s) const;
  // Create propagators
  virtual void initialize(Space* home, IntVarArray& x, int param) = 0;
  virtual int    vars(int param) = 0;
  virtual IntSet dom(int param)  = 0;
  // Perform test
  virtual bool run(const Options& opt);
};

#endif

// STATISTICS: test-stress

