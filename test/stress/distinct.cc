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

static int vals[] = {10, 100, 1000};
static IntSet _p(vals, sizeof(vals)/sizeof(int));
static int smallvals[] = {10, 100};
static IntSet _smallp(smallvals, sizeof(smallvals)/sizeof(int));

/*
 * Stress distinct propagator
 *
 * Performs propagation on a pathological example. The example is taken
 * from: Jean-François Puget, A fast algorithm for the bound consistency
 * of alldiff constraints, AAAI-98, pages 359-366, July 1998,
 * Madison, WI, USA.
 */
class StressDistinct : public StressTest {
private:
  IntConLevel icl;
public:
  StressDistinct(const char* t, IntConLevel icl0, IntSet& p)
    : StressTest(t,p), icl(icl0) {}
  virtual CheckResult check(const StressTestSpace *s) const {
    for (int i=0; i<s->x.size(); i++) {
      if (!s->x[i].assigned()) return CR_UNSPEC;
      for (int j=i+1; j<s->x.size(); j++){
	if (s->x[i].val() == s->x[j].val())
	  return CR_FOS;
      }
    }
    return CR_OK;
  }
  virtual void initialize(Space* home, IntVarArray& x, int param) {
    distinct(home, x, icl);
    branch(home, x, BVAR_NONE, BVAL_MIN);
  }
  virtual int    vars(int param) { return 2*param+1; }
  virtual IntSet dom(int param)  { return IntSet(-param, param); }
};

namespace {
  StressDistinct _dom("Distinct::Dom",ICL_DOM, _smallp);
  StressDistinct _bnd("Distinct::Bnd",ICL_BND, _p);
  StressDistinct _val("Distinct::Val",ICL_VAL, _p);
}

// STATISTICS: test-stress

