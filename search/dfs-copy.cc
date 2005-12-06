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



#include "search/dfs-copy.hh"

namespace Gecode { namespace Search {

  DfsCopyEngine::~DfsCopyEngine(void) {
    delete cur;
    ds.reset();
  }

  void
  DfsCopyEngine::reset(Space* s) {
    delete cur;
    ds.reset();
    cur = s;
    FullStatistics::reset(s);
  }

  size_t
  DfsCopyEngine::stacksize(void) const {
    return ds.size();
  }

  Space*
  DfsCopyEngine::explore(void) {
    while (true) {
      if (cur == NULL) {
	if (ds.empty())
	  return NULL;
	unsigned int alt = ds.top().alt();
	if (ds.top().rightmost()) {
	  cur = ds.pop().space();
	  FullStatistics::pop(cur);
	} else {
	  clone++;
	  cur = ds.top().space()->clone(true,propagate);
	  ds.top().next();
	}
	commit++;
	cur->commit(alt,NULL,propagate);
	FullStatistics::current(cur);
      }
      unsigned int alt;
      switch (cur->status(alt,propagate)) {
      case SS_FAILED: {
	fail++;
	delete cur;
	cur = NULL;
	FullStatistics::current(NULL);
	break;
      }
      case SS_SOLVED: {
	Space *s = cur;
	cur = NULL;
	FullStatistics::current(NULL);
	return s;
      }
      case SS_BRANCH: {
	if (alt > 1) {
	  ds.push(cur,alt);
	  FullStatistics::push(ds.top().space());
	  clone++;
	}
	commit++;
	cur->commit(0,NULL,propagate);
	break;
      }
      }
    }
  }

}}

// STATISTICS: search-any
