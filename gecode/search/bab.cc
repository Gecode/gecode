/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include "gecode/search.hh"

namespace Gecode { namespace Search {

  /*
   * The invariant maintained by the engine is:
   *   For all nodes stored at a depth less than mark, there
   *   is no guarantee of betterness. For those above the mark,
   *   betterness is guaranteed.
   *
   * The engine maintains the path on the stack for the current
   * node to be explored.
   *
   */

  BabEngine::ExploreStatus
  BabEngine::explore(Space*& s1, Space*& s2) {
    start();
    /*
     * Upon entry, cur can be either NULL (after a solution
     * has been returned) or set to a space that has been
     * requested to be constrained.
     * When a solution is found, the solution is returned in s1
     * and ES_SOLUTION is returned.
     * When a space is requested to be constrained, the space
     * to be constrained is returned in s1 and s2 refers to the
     * currently best solution. In this case ES_CONSTRAIN is returned.
     *
     */
    while (true) {
      if (stop(stacksize())) {
	s1 = NULL;
	return ES_SOLUTION;
      }
      if (cur == NULL) {
	if (!rcs.next(*this)) {
	  s1 = NULL;
	  return ES_SOLUTION;
	}
	cur = rcs.recompute(d,*this);
	EngineCtrl::current(cur);
	if (rcs.entries() <= mark) {
	  // Next space needs to be constrained
	  mark = rcs.entries();
	  d  = 0; // Force copy!
	  s1 = cur;
	  s2 = best;
	  return ES_CONSTRAIN;
	}
      }
      switch (cur->status(propagate)) {
      case SS_FAILED:
	fail++;
	delete cur;
	cur = NULL;
	EngineCtrl::current(NULL);
	break;
      case SS_SOLVED:
	delete best;
	best = cur;
	mark = rcs.entries();
	s1 = best->clone(true,propagate);
	clone++;
	cur = NULL;
	EngineCtrl::current(NULL);
	return ES_SOLUTION;
      case SS_BRANCH:
	{
	  Space* c;
	  if ((d == 0) || (d >= c_d)) {
	    c = cur->clone(true,propagate);
	    clone++;
	    d = 1;
	  } else {
	    c = NULL;
	    d++;
	  }
	  const BranchingDesc* desc = rcs.push(cur,c);
	  EngineCtrl::push(c,desc);
	  cur->commit(desc,0);
	  commit++;
	  break;
	}
      default: 
	GECODE_NEVER;
      }
    }
    return ES_SOLUTION;
  }




  BAB::BAB(Space* s, unsigned int c_d, unsigned int a_d, Stop* st, size_t sz) 
    : e(c_d,a_d,st,sz) {
    unsigned long int p = 0;
    Space* c = (s->status(p) == SS_FAILED) ? NULL : s->clone(true,p);
    e.init(c);
    e.propagate += p;
    e.current(s);
    e.current(NULL);
    e.current(c);
    if (c == NULL)
      e.fail += 1;
  }

  bool
  BAB::stopped(void) const {
    return e.stopped();
  }

  Statistics
  BAB::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return s;
  }

}}

// STATISTICS: search-any
