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

#include "gecode/search.hh"

namespace Gecode { namespace Search {

  /*
   * Nodes for the probing engine (just remember next alternative
   * to try)
   *
   */

  forceinline
  ProbeEngine::ProbeNode::ProbeNode(Space* s, unsigned int a)
    : _space(s), _alt(a) {}

  forceinline Space*
  ProbeEngine::ProbeNode::space(void) const {
    return _space;
  }

  forceinline void
  ProbeEngine::ProbeNode::space(Space* s) {
    _space = s;
  }

  forceinline unsigned int
  ProbeEngine::ProbeNode::alt(void) const {
    return _alt;
  }

  forceinline void
  ProbeEngine::ProbeNode::alt(unsigned int a) {
    _alt = a;
  }


  /*
   * The probing engine: computes all solutions with
   * exact number of discrepancies (solutions with
   * fewer discrepancies are discarded)
   *
   */

  forceinline
  ProbeEngine::ProbeEngine(Stop* st, size_t sz) 
    : EngineCtrl(st,sz) {}

  forceinline void
  ProbeEngine::init(Space* s, unsigned int d0) {
    cur = s;
    d   = d0;
  }

  forceinline void
  ProbeEngine::reset(Space* s, unsigned int d0) {
    delete cur;
    assert(ds.empty());
    cur = s;
    d   = d0;
    EngineCtrl::reset(s);
  }

  forceinline size_t
  ProbeEngine::stacksize(void) const {
    return ds.size();
  }

  forceinline
  ProbeEngine::~ProbeEngine(void) {
    delete cur;
    while (!ds.empty())
      delete ds.pop().space();
  }

  forceinline Space*
  ProbeEngine::explore(void) {
    while (true) {
      if (stop(stacksize()))
	return NULL;
      if (cur == NULL) {
      backtrack:
	if (ds.empty())
	  return NULL;
	unsigned int a = ds.top().alt();
	if (a == 0) {
	  cur = ds.pop().space();
	  EngineCtrl::pop(cur);
	} else {
	  ds.top().alt(a-1);
	  cur = ds.top().space()->clone();
	  clone++;
	}
	cur->commit(a,NULL,propagate);
	EngineCtrl::current(cur);
	d++;
      }
    check_discrepancy:
      if (d == 0) {
	Space* s = cur;
	unsigned int alt;
	while (s->status(alt) == SS_BRANCH) {
	  if (stop(stacksize()))
	    return NULL;
	  s->commit(0,NULL,propagate);
	}
	cur = NULL;
	EngineCtrl::current(NULL);
	if (s->failed()) {
	  delete s;
	  goto backtrack;
	}
	return s;
      }
      unsigned int alt;
      switch (cur->status(alt,propagate)) {
      case SS_FAILED:
	fail++;
      case SS_SOLVED:	
	delete cur;
	cur = NULL;
	EngineCtrl::current(NULL);
	goto backtrack;
      case SS_BRANCH:
	{
	  if (alt > 1) {
	    unsigned int d_a = (d >= alt-1) ? alt-1 : d;
	    Space* cc = cur->clone();
	    EngineCtrl::push(cc);
	    ProbeNode sn(cc,d_a-1);
	    clone++;
	    ds.push(sn);
	    cur->commit(d_a,NULL,propagate);
	    d -= d_a;
	  } else {
	    cur->commit(0,NULL,propagate);
	  }
	  commit++;
	  goto check_discrepancy;
	}
      }
    }
  }


  /*
   * The LDS engine proper (_LDS means: all the logic but just
   * for space, type casts are done in LDS)
   *
   */

  LDS::LDS(Space* s, unsigned int d, Stop* st, size_t sz)
    : d_cur(0), d_max(d), no_solution(true), e(st,sz) {
    unsigned int alt;
    if (s->status(alt) == SS_FAILED) {
      root = NULL;
      e.init(NULL,0);
      e.fail += 1;
      e.current(s);
    } else {
      root = s;
      Space* c = (d_max == 0) ? s : s->clone();
      e.init(c,0);
      e.current(s);
      e.current(NULL);
      e.current(c);
    }
  }

  LDS::~LDS(void) {
    delete root;
  }

  Space*
  LDS::next(void) {
    while (true) {
      Space* s = e.explore();
      if (s != NULL) {
	no_solution = false;
	return s;
      }
      if (no_solution || (++d_cur > d_max))
	break;
      no_solution = true;
      if (d_cur == d_max) {
	e.reset(root,d_cur);
	root = NULL;
      } else {
	e.clone++;
	e.reset(root->clone(),d_cur);
      }
    }
    return NULL;
  }

  Statistics
  LDS::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return e;
  }

}}

// STATISTICS: search-any
