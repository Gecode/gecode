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

#include "search.hh"

namespace Gecode { namespace Search {

  forceinline
  DfsEngine::DfsEngine(unsigned int c_d0, unsigned int a_d0,
		       size_t sz)
    : FullStatistics(sz), ds(a_d0), cur(NULL), c_d(c_d0), d(0) {}

  
  forceinline void
  DfsEngine::init(Space* s) {
    cur = s;
  }

  forceinline void
  DfsEngine::reset(Space* s) {
    delete cur;
    ds.reset();
    cur = s;
    d   = 0;
    FullStatistics::reset(s);
  }

  forceinline size_t
  DfsEngine::stacksize(void) const {
    return ds.size();
  }

  forceinline Space*
  DfsEngine::explore(void) {
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

  forceinline
  DfsEngine::~DfsEngine(void) {
    delete cur;
    ds.reset();
  }

  /*
   * Control for DFS
   *
   */

  DFS::DFS(Space* s, unsigned int c_d, unsigned int a_d, size_t sz) 
    : e(c_d,a_d,sz) {
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
  
  Space*
  DFS::next(void) {
    return e.explore();
  }

  Statistics
  DFS::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return s;
  }

}}

// STATISTICS: search-any
