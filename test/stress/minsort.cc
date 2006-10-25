/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvis@gecode.org>
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

#undef min

static int vals[] = {10, 100, 200};
static IntSet _p(vals, sizeof(vals)/sizeof(int));

/*
 * Sorts param numbers using min and ordering-relations.
 */
struct MinSort : public StressTest {
  MinSort()
    : StressTest("MinSort", _p) {
  }
  virtual CheckResult check(const StressTestSpace *s) const {
    if (!s) return CR_FOS;
    for (int i = 0; i < s->x.size()-1; ++i) {
      if (!s->x[i].assigned() || !s->x[i+1].assigned()) return CR_FOS;
      if (s->x[i].val() < s->x[i+1].val()) return CR_FOS;
    }
    return CR_OK;
  }
  virtual void initialize(Space* home, IntVarArray& x, int param) {
    for (int i=1; i<param; i++) {
      IntVarArgs y(i);
      for (int j=0; j<i; j++)
        y[j] = x[j];
      IntVar m(home,0,2*param);
      min(home, y, m);
      rel(home, m, IRT_GR, x[i]);
    }
    branch(home, x, BVAR_NONE, BVAL_SPLIT_MAX);
  }
  virtual int    vars(int param) { return param; }
  virtual IntSet dom(int param)  { return IntSet(1, 2*param); }
};

static MinSort _minsort;

// STATISTICS: test-stress
