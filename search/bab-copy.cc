/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Bugfixes provided by:
 *     Grégoire Dooms <dooms@info.ucl.ac.be>
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

#include "search/bab-copy.hh"

namespace Gecode { namespace Search {

  /*
   * The invarinat maintained by the engine is:
   *   For all nodes stored at a depth less than mark, there
   *   is no guarantee of betterness. For those above the mark,
   *   betterness is guaranteed.
   *
   * The engine maintains the path on the stack for the next
   * node to be explored.
   *
   */

  BabCopyEngine::~BabCopyEngine(void) {
    ds.reset();
    delete b;
    delete cur;
  }

  size_t
  BabCopyEngine::stacksize(void) const {
    return ds.size();
  }

  bool
  BabCopyEngine::explore(Space*& s1, Space*& s2) {
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
      if (cur == NULL) {
      backtrack:
	if (ds.empty()) {
	  s1 = NULL;
	  return true;
	}
	// Get the next alternative and space from the stack.
	if (ds.top().rightmost()) {
	  unsigned int alt = ds.top().alt();
	  cur = ds.pop().space();
	  FullStatistics::pop(cur);
	  cur->commit(alt,NULL,propagate);
	  FullStatistics::current(cur);
	  commit++;
	  // Next space needs to be constrained?
	  if (ds.entries()+1 <= mark) {
	    mark--;
	    s1 = cur;
	    s2 = b;
	    return false;
	  }
	} else {
	  cur = ds.top().space()->clone(true,propagate);
	  cur->commit(ds.top().alt(),NULL,propagate);
	  FullStatistics::current(cur);
	  ds.top().next();
	  clone++; commit++;
	  // Next space needs to be constrained?
	  if (ds.entries() <= mark) {
	    s1 = cur;
	    s2 = b;
	    return false;
	  }
	}
      }
      while (true) {
	unsigned int alt;
	switch (cur->status(alt,propagate)) {
	case SS_FAILED:
	  fail++;
	  delete cur;
	  cur = NULL;
	  FullStatistics::current(NULL);
	  goto backtrack;
	case SS_SOLVED:
	  delete b;
	  b = cur;
	  mark = ds.entries();
	  s1 = b->clone(true,propagate);
	  cur = NULL;
	  FullStatistics::current(NULL);
	  return true;
	case SS_BRANCH:
	  if (alt > 1) {
	    clone++;
	    ds.push(cur,alt);
	    FullStatistics::push(ds.top().space());
	  }
	  commit++;
	  cur->commit(0,NULL,propagate);
	  break;
	}
      }
    }
    return true;
  }

}}
	
// STATISTICS: search-any
