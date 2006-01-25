/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include "int.hh"
#include "support/sort.hh"
#include "support/static-stack.hh"


namespace Gecode { namespace Int { namespace Regular {

  /**
   * \brief Sort transition array by input state
   */
  class TransByI_State {
  public:
    forceinline bool
    operator()(const DFA::Transition& x, const DFA::Transition& y) {
      return x.i_state < y.i_state;
    }
    forceinline static void
    sort(DFA::Transition t[], int n) {
      TransByI_State tbis;
      Support::quicksort<DFA::Transition,TransByI_State>(t,n,tbis);
    }
  };

  /**
   * \brief Sort transition array by symbol (value)
   */
  class TransBySymbol {
  public:
    forceinline bool
    operator()(const DFA::Transition& x, const DFA::Transition& y) {
      return x.symbol < y.symbol;
    }
    forceinline static void
    sort(DFA::Transition t[], int n) {
      TransBySymbol tbs;
      Support::quicksort<DFA::Transition,TransBySymbol>(t,n,tbs);
    }
  };

  /**
   * \brief Sort transition array by symbol and then input states
   */
  class TransBySymbolI_State {
  public:
    forceinline bool
    operator()(const DFA::Transition& x, const DFA::Transition& y) {
      return ((x.symbol < y.symbol) ||
	      (x.symbol == y.symbol) && (x.i_state < y.i_state));
    }
    forceinline static void
    sort(DFA::Transition t[], int n) {
      TransBySymbolI_State tbsi;
      Support::quicksort<DFA::Transition,TransBySymbolI_State>(t,n,tbsi);
    }
  };

  /**
   * \brief Sort transition array by output state
   */
  class TransByO_State {
  public:
    forceinline bool
    operator()(const DFA::Transition& x, const DFA::Transition& y) {
      return x.o_state < y.o_state;
    }
    forceinline static void
    sort(DFA::Transition t[], int n) {
      TransByO_State tbos;
      Support::quicksort<DFA::Transition,TransByO_State>(t,n,tbos);
    }
  };


  /**
   * \brief Stategroup is used to compute a partition of states
   */
  class StateGroup {
  public:
    int state;
    int group;
  };

  /**
   * \brief Sort groups stated by group and then state
   */
  class StateGroupByGroup {
  public:
    forceinline bool
    operator()(const StateGroup& x, const StateGroup& y) {
      return ((x.group < y.group) ||
	      (x.group == y.group) && (x.state < y.state));
    }
    static void
    sort(StateGroup sg[], int n) {
      StateGroupByGroup o;
      Support::quicksort<StateGroup,StateGroupByGroup>(sg,n,o);
    }
  };

  /**
   * \brief %GroupStates is used to index %StateGroup by group
   */
  class GroupStates {
  public:
    StateGroup* fst;
    StateGroup* lst;
  };

  /// State is not reachable
#define S_NONE       0
  /// State is reachable from start state
#define S_FROM_START 1
  /// Final state is reachable from state
#define S_TO_FINAL   2
  /// State is final
#define S_FINAL      4

}}}

namespace Gecode {

  void
  DFA::init(int start, Transition t_spec[], int f_spec[], bool minimize) {
    using namespace Int;
    using namespace Regular;
    // Compute number of states and transitions
    int n_states = start;
    int n_trans  = 0;
    for (Transition* t = &t_spec[0]; t->i_state >= 0; t++) {
      n_states = std::max(n_states,t->i_state);
      n_states = std::max(n_states,t->o_state);
      n_trans++;
    }
    for (int* f = &f_spec[0]; *f >= 0; f++)
      n_states = std::max(n_states,*f);
    n_states++;

    // Temporary structure for transitions
    GECODE_AUTOARRAY(Transition, trans, n_trans);
    for (int i = n_trans; i--; )
      trans[i] = t_spec[i];
    // Temporary structures for finals
    GECODE_AUTOARRAY(int,  final,    n_states+1);
    GECODE_AUTOARRAY(bool, is_final, n_states+1);
    int n_finals = 0;
    for (int i = n_states+1; i--; )
      is_final[i] = false;
    for (int* f = &f_spec[0]; *f != -1; f++) {
      is_final[*f]      = true;
      final[n_finals++] = *f;
    }

    if (minimize) {
      // Sort transitions by symbol and i_state
      TransBySymbolI_State::sort(trans, n_trans);
      GECODE_AUTOARRAY(Transition*, idx, n_trans+1);
      //  idx[i]...idx[i+1]-1 gives where transitions for symbol i start
      int n_symbols = 0;
      {
	int j = 0;
	while (j < n_trans) {
	  idx[n_symbols++] = &trans[j];
	  int s = trans[j].symbol;
	  while ((j < n_trans) && (s == trans[j].symbol))
	    j++;
	}
	idx[n_symbols] = &trans[j];
	assert(j == n_trans);
      }
      // Map states to groups
      GECODE_AUTOARRAY(int,         s2g,  n_states+1);
      GECODE_AUTOARRAY(StateGroup,  part, n_states+1);
      GECODE_AUTOARRAY(GroupStates, g2s,  n_states+1);
      // Initialize: final states is group one, all other group zero
      for (int i = n_states+1; i--; ) {
	part[i].state = i;
	part[i].group = is_final[i] ? 1 : 0;
	s2g[i]        = part[i].group;
      }
      // Important: the state n_state is the dead state, conceptually
      // if there is no transition for a symbol and input state, it is
      // assumed that there is an implicit transition to n_state

      // Set up the indexing data structure, sort by group
      StateGroupByGroup::sort(part,n_states+1);
      int n_groups;
      if (part[0].group == part[n_states].group) {
	// No final states, just one group
	g2s[0].fst = &part[0]; g2s[0].lst = &part[n_states+1];
	n_groups = 1;
      } else  {
	int i = 0;
	assert(part[0].group == 0);
	do i++; while (part[i].group == 0);
	g2s[0].fst = &part[0]; g2s[0].lst = &part[i];
	g2s[1].fst = &part[i]; g2s[1].lst = &part[n_states+1];
	n_groups = 2;
      }

      // Do the refinement
      {
	int m_groups;
	do {
	  m_groups = n_groups;
	  // Iterate over symbols
	  for (int sidx = n_symbols; sidx--; ) {
	    // Iterate over groups
	    for (int g = n_groups; g--; ) {
	      // Ignore singleton groups
	      if (g2s[g].lst-g2s[g].fst > 1) {
		// Apply transitions to group states
		// This exploits that both transitions as well as
		// stategroups are sorted by (input) state
		Transition* t     = idx[sidx];
		Transition* t_lst = idx[sidx+1];
		for (StateGroup* sg = g2s[g].fst; sg<g2s[g].lst; sg++) {
		  while ((t < t_lst) && (t->i_state < sg->state))
		    t++;
		  // Compute group resulting from transition
		  if ((t < t_lst) && (t->i_state == sg->state))
		    sg->group = s2g[t->o_state];
		  else
		    sg->group = s2g[n_states]; // Go to dead state
		}
		// Sort group by groups from transitions
		StateGroupByGroup::sort(g2s[g].fst, 
					static_cast<int>(g2s[g].lst-g2s[g].fst));
		// Group must be split?
		if (g2s[g].fst->group != (g2s[g].lst-1)->group) {
		  // Skip first group
		  StateGroup* sg = g2s[g].fst+1;
		  while ((sg-1)->group == sg->group)
		    sg++;
		  // Start splitting
		  StateGroup* lst = g2s[g].lst;
		  g2s[g].lst = sg;
		  while (sg < lst) {
		    s2g[sg->state] = n_groups;
		    g2s[n_groups].fst  = sg++;
		    while ((sg < lst) && ((sg-1)->group == sg->group)) {
		      s2g[sg->state] = n_groups; sg++;
		    }
		    g2s[n_groups++].lst = sg;
		  }
		}
	      }
	    }
	  }
	} while (n_groups != m_groups);
	// New start state
	start = s2g[start];
	// Compute new final states
	n_finals = 0;
	for (int g = n_groups; g--; )
	  for (StateGroup* sg = g2s[g].fst; sg < g2s[g].lst; sg++)
	    if (is_final[sg->state]) {
	      final[n_finals++] = g;
	      break;
	    }
	// Compute representatives
	GECODE_AUTOARRAY(int, s2r, n_states+1);
	for (int i = n_states+1; i--; )
	  s2r[i] = -1;
	for (int g = n_groups; g--; )
	  s2r[g2s[g].fst->state] = g;
	// Clean transitions
	int j = 0;
	for (int i = 0; i<n_trans; i++)
	  if (s2r[trans[i].i_state] != -1) {
	    trans[j].i_state = s2g[trans[i].i_state];
	    trans[j].symbol  = trans[i].symbol;
	    trans[j].o_state = s2g[trans[i].o_state];
	    j++;
	  }
	n_trans  = j;
	n_states = n_groups;
      }
    }

    // Do a reachability analysis for all states starting from start state
    Support::StaticStack<int> visit(n_states);
    GECODE_AUTOARRAY(int, state, n_states);
    for (int i=n_states; i--; )
      state[i] = S_NONE;

    {
      // Sort all transitions according to i_state and create index structure
      //  idx[i]...idx[i+1]-1 gives where transitions for state i start
      TransByI_State::sort(trans, n_trans);
      GECODE_AUTOARRAY(Transition*, idx, n_states+1);
      {
	int j = 0;
	for (int i=0; i<n_states; i++) {
	  idx[i] = &trans[j];
	  while ((j < n_trans) && (i == trans[j].i_state))
	    j++;
	}
	idx[n_states] = &trans[j];
	assert(j == n_trans);
      }

      state[start] = S_FROM_START;
      visit.push(start);
      while (!visit.empty()) {
	int s = visit.pop();
	for (Transition* t = idx[s]; t < idx[s+1]; t++)
	  if (!(state[t->o_state] & S_FROM_START)) {
	    state[t->o_state] |= S_FROM_START;
	    visit.push(t->o_state);
	  }
      }
    }

    // Do a reachability analysis for all states to a final state
    {
      // Sort all transitions according to o_state and create index structure
      //  idx[i]...idx[i+1]-1 gives where transitions for state i start
      TransByO_State::sort(trans, n_trans);
      GECODE_AUTOARRAY(Transition*, idx, n_states+1);
      {
	int j = 0;
	for (int i=0; i<n_states; i++) {
	  idx[i] = &trans[j];
	  while ((j < n_trans) && (i == trans[j].o_state))
	    j++;
	}
	idx[n_states] = &trans[j];
	assert(j == n_trans);
      }

      for (int i = n_finals; i--; ) {
	state[final[i]] |= (S_TO_FINAL | S_FINAL);
	visit.push(final[i]);
      }
      while (!visit.empty()) {
	int s = visit.pop();
	for (Transition* t = idx[s]; t < idx[s+1]; t++)
	  if (!(state[t->i_state] & S_TO_FINAL)) {
	    state[t->i_state] |= S_TO_FINAL;
	    visit.push(t->i_state);
	  }
      }
    }

    // Now all reachable states are known (also the final ones)
    GECODE_AUTOARRAY(int, re, n_states);
    for (int i = n_states; i--; )
      re[i] = -1;

    // Renumber states
    int m_states = 0;
    // Start state gets zero
    re[start] = m_states++;

    // Renumber final states
    for (int i = n_states; i--; )
      if ((state[i] == (S_FINAL | S_FROM_START | S_TO_FINAL)) && (re[i] < 0))
	re[i] = m_states++;
    // If start state is final, final states start from zero, otherwise from one
    int final_fst = (state[start] & S_FINAL) ? 0 : 1;
    int final_lst = m_states;
    // final_fst...final_lst-1 are the final states

    // Renumber remaining states
    for (int i = n_states; i--; )
      if ((state[i] == (S_FROM_START | S_TO_FINAL)) && (re[i] < 0))
	re[i] = m_states++;

    // Count number of remaining transitions
    int m_trans = 0;
    for (int i = n_trans; i--; )
      if ((re[trans[i].i_state] >= 0) && (re[trans[i].o_state] >= 0))
	m_trans++;

    // All done... Construct the automaton
    DFAI* d = reinterpret_cast<DFAI*>
      (Memory::malloc(sizeof(DFAI) + sizeof(Transition)*(m_trans-1)));
    d->use_cnt   = 1;
    d->n_states  = m_states;
    d->n_trans   = m_trans;
    d->final_fst = final_fst;
    d->final_lst = final_lst;
    {
      int j = 0;
      Transition* r = &d->trans[0];
      for (int i = 0; i<n_trans; i++)
	if ((re[trans[i].i_state] >= 0) && (re[trans[i].o_state] >= 0)) {
	  r[j].symbol  = trans[i].symbol;
	  r[j].i_state = re[trans[i].i_state];
	  r[j].o_state = re[trans[i].o_state];
	  j++;
	}
      TransBySymbol::sort(r,m_trans);
    }
    dfai = d;
  }

}

std::ostream&
operator<<(std::ostream& os, const Gecode::DFA& dfa) {
  os << "Start state: 0" << std::endl
     << "States:      0..." << dfa.n_states()-1 << std::endl
     << "Transitions:";
  for (int s = 0; s < static_cast<int>(dfa.n_states()); s++) {
    Gecode::DFA::Transitions t(dfa);
    int n = 0;
    while (t()) {
      if (t.transition()->i_state == s) {
	if ((n % 4) == 0)
	  os << std::endl << "\t";
	os << "[" << t.transition()->i_state << "]"
	   << "- " << t.transition()->symbol << " >"
	   << "[" << t.transition()->o_state << "]  ";
	++n;
      }
      ++t;
    }
  }
  os << std::endl << "Final states: "
     << std::endl
     << "\t[" << dfa.final_fst() << "] ... ["
     << dfa.final_lst()-1 << "]"
     << std::endl;
  return os;
}

namespace Gecode {

  DFA::DFAI*
  DFA::DFAI::copy(void) {
    DFAI* d = reinterpret_cast<DFAI*>
      (Memory::malloc(sizeof(DFAI) + sizeof(Transition)*(n_trans-1)));
    d->use_cnt   = 1;
    d->n_states  = n_states;
    d->n_trans   = n_trans;
    d->final_fst = final_fst;
    d->final_lst = final_lst;
    memcpy(&d->trans[0], &trans[0], sizeof(Transition)*n_trans);
    return d;
  }

}

// STATISTICS: int-prop

