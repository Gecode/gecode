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

#include "search/dfs-reco.hh"

namespace Gecode { namespace Search {

  DfsReCoEngine::~DfsReCoEngine(void) {
    delete cur;
    ds.reset();
  }

  void
  DfsReCoEngine::reset(Space* s) {
    delete cur;
    ds.reset();
    cur = s;
    d   = 0;
    FullStatistics::reset(s);
  }

  size_t
  DfsReCoEngine::stacksize(void) const {
    return ds.size();
  }

  Space*
  DfsReCoEngine::explore(void) {
    while (true) {
      while (cur) {
	unsigned int alt;
	switch (cur->status(alt,propagate)) {
	case SS_FAILED:
	  fail++;
	  delete cur;
	  cur = NULL;
	  FullStatistics::current(NULL);
	  break;
	case SS_SOLVED:
	  {
	    Space* s = cur;
	    cur = NULL;
	    FullStatistics::current(NULL);
	    return s;
	  }
	case SS_BRANCH:
	  {
	    Space* c;
	    if ((d == 0) || (d >= c_d)) {
	      clone++;
	      c = cur->clone();
	      d = 1;
	    } else {
	      c = NULL;
	      if (alt > 1)
		d++;
	    }
	    BranchingDesc* desc = ds.push(cur,c,alt);
	    FullStatistics::push(c,desc);
	    commit++;
	    cur->commit(0, desc);
	    break;
	  }
	}
      }
      if (!ds.next(*this))
	return NULL;
      cur = ds.recompute(d,*this);
      FullStatistics::current(cur);
    }
    return NULL;
  }

}}

// STATISTICS: search-any
