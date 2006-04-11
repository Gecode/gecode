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

  forceinline
  BabEngine::BabEngine(unsigned int c_d0, unsigned int a_d,
		       Stop* st, size_t sz)
    : EngineCtrl(st,sz), ds(a_d), cur(NULL), mark(0), best(NULL), 
      c_d(c_d0), d(0) {}
  

  forceinline void
  BabEngine::init(Space* s) {
    cur = s;
  }


  forceinline size_t
  BabEngine::stacksize(void) const {
    return ds.size();
  }

  bool
  BabEngine::explore(Space*& s1, Space*& s2) {
    /*
     * Upon entry, cur can be either NULL (after a solution
     * has been returned) or set to a space that has been
     * requested to be constrained.
     * When a solution is found, the solution is returned in s1
     * and true is returned.
     * When a space is requested to be constrained, the space
     * to be constrained is returned in s1 and s2 refers to the
     * currently best solution. In this case false is returned.
     *
     */
    while (true) {
      if (stop(stacksize())) {
	s1 = NULL;
	return true;
      }
      if (cur == NULL) {
	if (!ds.next(*this)) {
	  s1 = NULL;
	  return true;
	}
	cur = ds.recompute(d,*this);
	EngineCtrl::current(cur);
	if (ds.entries() <= mark) {
	  // Next space needs to be constrained
	  mark = ds.entries();
	  d  = 0; // Force copy!
	  s1 = cur;
	  s2 = best;
	  return false;
	}
      }
      unsigned int alt;
      switch (cur->status(alt,propagate)) {
      case SS_FAILED:
	fail++;
	delete cur;
	cur = NULL;
	EngineCtrl::current(NULL);
	break;
      case SS_SOLVED:
	delete best;
	best = cur;
	mark = ds.entries();
	s1 = best->clone();
	clone++;
	cur = NULL;
	EngineCtrl::current(NULL);
	return true;
      case SS_BRANCH:
	{
	  Space* c;
	  if ((d == 0) || (d >= c_d)) {
	    c = cur->clone();
	    clone++;
	    d = 1;
	  } else {
	    c = NULL;
	    if (alt > 1)
	      d++;
	  }
	  BranchingDesc* desc = ds.push(cur,c,alt);
	  EngineCtrl::push(c,desc);
	  cur->commit(0,desc,propagate);
	  commit++;
	  break;
	}
      }
    }
    return true;
  }

  BabEngine::~BabEngine(void) {
    ds.reset();
    delete best;
    delete cur;
  }



  BAB::BAB(Space* s, unsigned int c_d, unsigned int a_d, Stop* st, size_t sz) 
    : e(c_d,a_d,st,sz) {
    unsigned int alt;
    unsigned long int p = 0;
    Space* c = (s->status(alt,p) == SS_FAILED) ? NULL : s->clone();
    e.init(c);
    e.propagate += p;
    e.current(s);
    e.current(NULL);
    e.current(c);
    if (c == NULL)
      e.fail += 1;
  }

  Statistics
  BAB::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return s;
  }

}}

// STATISTICS: search-any
