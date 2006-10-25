/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

static int vals[] = {10, 100, 1000, 10000};
static IntSet _p(vals, sizeof(vals)/sizeof(int));

/*
 * Stress the implementation of variable domains
 *
 * Chops first a large number of holes into some variable domains
 * and then shrinks the domain.
 */
struct Domain : public StressTest {
  Domain()
    : StressTest("Domain", _p) {
  }
  virtual CheckResult check(const StressTestSpace *s) const {
    if (!s) return CR_OK;
    for (int i = 0; i < s->x.size(); ++i) {
      if (!s->x[i].assigned()) return CR_SOF;
    }
    return CR_UNSPEC;
  }
  virtual void initialize(Space* home, IntVarArray& x, int param) {
    // Cut holes: expand
    for (int i = 5; i--; ) {
      for (int j = 0; j <= 5*param; j++)
        rel(home, x[i], IRT_NQ, 5*j);
      for (int j = 0; j <= 5*param; j++)
        rel(home, x[i], IRT_NQ, 5*j+2);
      for (int j = 0; j <= 5*param; j++)
        rel(home, x[i], IRT_NQ, 5*j+4);
    }
    // Contract
    for (int j = 0; j <= 5*param/2; j++)
      for (int i = 5; i--; ) {
        rel(home, x[i], IRT_GQ, 5*j);
        rel(home, x[i], IRT_LQ, 5*(j + (5*param/2)));
      }
  }
  virtual int    vars(int param) { return 5; }
  virtual IntSet dom(int param)  { return IntSet(0, 5*param); }
};

static Domain _domain;

// STATISTICS: test-stress
