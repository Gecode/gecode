/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
 *     Mikael Lagerkvist, 2005
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

#include "test/stress.hh"

static int vals[] = {10, 100, 1000, 1000000, 1 << 24};
static IntSet _p(vals, sizeof(vals)/sizeof(int));
static bool gflag = false;

/*
 * Creates two propagates requiring a huge amount of propagation
 * steps to detect failure. Stresses how fast a system can control
 * propagation.
 */
struct Exec : public StressTest {
  bool flag;
  Exec()
    : StressTest("Exec", _p), flag(gflag) {
    gflag = !gflag;
  }
  virtual CheckResult check(const StressTestSpace *s) const {
    return (s==NULL) ? CR_OK : CR_SOF;
  }
  virtual void initialize(Space* home, IntVarArray& x, int param) {
    if (flag) {
      rel(home, x[0], IRT_LE, x[1]);
      rel(home, x[1], IRT_LE, x[0]);
    } else {
      rel(home, x[1], IRT_LE, x[0]);
      rel(home, x[0], IRT_LE, x[1]);
    }
  }
  virtual int    vars(int param) { return 2; }
  virtual IntSet dom(int param)  { return IntSet(0, param); }
};

static Exec _exec;

// STATISTICS: test-stress

