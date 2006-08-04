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
#include "gecode/support/dynamic-array.hh"

static int vals[] = {3, 4, 5, 6, 7, 8};
static IntSet _p(vals, sizeof(vals)/sizeof(int));

typedef Support::DynamicArray<DFA::Transition> Transitions;

// Create all states for permutation of symbols
void
permute(int i_state, int i, int n, int p,
	Transitions& t, int& n_t, int& n_state) {
  if (i<n-1) {
    for (int j=n; j--; )
      if (!(p & (1 << j))) {
	t[n_t].i_state = i_state;
	t[n_t].symbol  = j;
	t[n_t].o_state = ++n_state;
	n_t++;
	permute(n_state,i+1,n,p | (1 << j),
		t,n_t,n_state);
      }
  } else {
    for (int j=n; j--; )
      if (!(p & (1 << j))) {
	t[n_t].i_state = i_state;
	t[n_t].symbol  = j;
	t[n_t].o_state = 1;
	n_t++;
      }
  }
}

// Construct a distinct %DFA
DFA
construct(int n) {
  Transitions t;
  int n_t     = 0;
  int n_state = 1;
  permute(0,0,n,0,t,n_t,n_state);
  t[n_t].i_state = -1;
  int f[] = {1,-1};
  DFA d(0,t,f);
  return d;
}

/*
 * Stress the regular constraint
 *
 * Creates a huge DFA specifying that all values are pairwise distinct
 * and use the DFA for propagation.
 */
struct StressRegular : public StressTest {
  int n;
  StressRegular()
    : StressTest("Regular",_p) {}
  virtual CheckResult check(const StressTestSpace *s) const {
    for (int i=0; i<s->x.size(); ++i)
      if (!s->x[i].assigned()) return CR_UNSPEC;
    for (int o = 0; o <= 3*n; ++o) {
      for (int i = 0; i < n; ++i) {
	for (int j = i+1; j < n; ++j) {
	  if (s->x[o+i].val() == s->x[o+j].val())
	    return CR_FOS;
	}
      }
    }
    return CR_OK;
  }
  virtual void initialize(Space* home, IntVarArray& x, int param) {
    n = param;
    DFA a = construct(param);
    IntVarArgs y(param);
    for (int i = 0; i <= 3*param; ++i) {
      for (int j = param; j--; )
	y[j] = x[i+j];
      regular(home, y, a);
    }
    for (int j=param; j--; )
      y[j] = x[j];
    branch(home, y, BVAR_NONE, BVAL_MED);
  }
  virtual int    vars(int param) { return 4*param; }
  virtual IntSet dom(int param)  { return IntSet(0, param); }
};

static StressRegular _reg;

// STATISTICS: test-stress
