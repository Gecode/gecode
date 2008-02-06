/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Bugfixes provided by:
 *     Stefano Gualandi
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
  ProbeEngine::ProbeNode
  ::ProbeNode(Space* s, const BranchingDesc* d, unsigned int a)
    : _space(s), _desc(d), _alt(a) {}

  forceinline Space*
  ProbeEngine::ProbeNode::space(void) const {
    return _space;
  }

  forceinline const BranchingDesc*
  ProbeEngine::ProbeNode::desc(void) const {
    return _desc;
  }

  forceinline unsigned int
  ProbeEngine::ProbeNode::alt(void) const {
    return _alt;
  }

  forceinline void
  ProbeEngine::ProbeNode::next(void) {
    _alt--;
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
    cur       = s;
    d         = d0;
    exhausted = true;
  }

  forceinline void
  ProbeEngine::reset(Space* s, unsigned int d0) {
    delete cur;
    assert(ds.empty());
    cur       = s;
    d         = d0;
    exhausted = true;
    EngineCtrl::reset(s);
  }

  forceinline size_t
  ProbeEngine::stacksize(void) const {
    return ds.size();
  }

  forceinline bool
  ProbeEngine::done(void) const {
    return exhausted;
  }

  forceinline
  ProbeEngine::~ProbeEngine(void) {
    delete cur;
    while (!ds.empty())
      delete ds.pop().space();
  }

  forceinline Space*
  ProbeEngine::explore(void) {
    start();
    while (true) {
      if (cur == NULL) {
      backtrack:
        if (ds.empty())
          return NULL;
        if (stop(stacksize()))
          return NULL;
        unsigned int a            = ds.top().alt();
        const BranchingDesc* desc = ds.top().desc();
        if (a == 0) {
          cur = ds.pop().space();
          EngineCtrl::pop(cur,desc);
          cur->commit(desc,0);
          delete desc;
        } else {
          ds.top().next();
          cur = ds.top().space()->clone(true,propagate);
          clone++;
          cur->commit(desc,a);
        }
        EngineCtrl::current(cur);
        d++;
      }
    check_discrepancy:
      if (d == 0) {
        Space* s = cur;
        while (s->status(propagate) == SS_BRANCH) {
          const BranchingDesc* desc = s->description();
          if (desc->alternatives() > 1)
            exhausted = false;
          s->commit(desc,0);
          delete desc;
        }
        cur = NULL;
        EngineCtrl::current(NULL);
        if (s->failed()) {
          delete s;
          goto backtrack;
        }
        return s;
      }
      switch (cur->status(propagate)) {
      case SS_FAILED:
        fail++;
      case SS_SOLVED:        
        delete cur;
        cur = NULL;
        EngineCtrl::current(NULL);
        goto backtrack;
      case SS_BRANCH:
        {
          const BranchingDesc* desc = cur->description();
          unsigned int alt          = desc->alternatives();
          if (alt > 1) {
            if (d < alt-1)
              exhausted = false;
            unsigned int d_a = (d >= alt-1) ? alt-1 : d;
            Space* cc = cur->clone(true,propagate);
            EngineCtrl::push(cc,desc);
            ProbeNode sn(cc,desc,d_a-1);
            clone++;
            ds.push(sn);
            cur->commit(desc,d_a);
            d -= d_a;
          } else {
            cur->commit(desc,0);
            delete desc;
          }
          commit++;
          goto check_discrepancy;
        }
      default: GECODE_NEVER;
      }
    }
  }


  /*
   * The LDS engine proper
   *
   */

  LDS::LDS(Space* s, unsigned int d, Stop* st, size_t sz)
    : root(NULL), d_cur(0), d_max(d), e(st,sz) {
    if (s->status(e.propagate) == SS_FAILED) {
      e.init(NULL,0);
      e.fail += 1;
      e.current(s);
    } else {
      e.clone++;;
      Space* c = s->clone();
      if (d_max > 0) {
        root = c->clone();
        e.clone++;
      }
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
      if (s != NULL)
        return s;
      if (((s == NULL) && e.stopped()) || (++d_cur > d_max) || e.done())
        break;
      if (d_cur == d_max) {
        if (root != NULL)
          e.reset(root,d_cur);
        root = NULL;
      } else if (root != NULL) {
        e.clone++;
        e.reset(root->clone(true,e.propagate),d_cur);
      }
    }
    return NULL;
  }

  bool
  LDS::stopped(void) const {
    return e.stopped();
  }

  Statistics
  LDS::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return e;
  }

}}

// STATISTICS: search-any
