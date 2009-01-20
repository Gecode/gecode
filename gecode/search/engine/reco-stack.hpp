/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

namespace Gecode { namespace Search {

  /*
   * Node for recomputation
   *
   */

  forceinline
  ReCoNode::ReCoNode(void) {}

  forceinline
  ReCoNode::ReCoNode(Space* s, Space* c)
    : _space(c), _alt(0), _desc(s->description()) {}

  forceinline Space*
  ReCoNode::space(void) const {
    return _space;
  }
  forceinline void
  ReCoNode::space(Space* s) {
    _space = s;
  }

  forceinline unsigned int
  ReCoNode::alt(void) const {
    return _alt;
  }
  forceinline bool
  ReCoNode::rightmost(void) const {
    return _alt+1 == _desc->alternatives();
  }
  forceinline void
  ReCoNode::next(void) {
    _alt++;
  }

  forceinline const BranchingDesc*
  ReCoNode::desc(void) const {
    return _desc;
  }

  forceinline void
  ReCoNode::dispose(void) {
    delete _space;
    delete _desc;
  }



  /*
   * Depth-first stack with recomputation
   *
   */

  forceinline
  ReCoStack::ReCoStack(unsigned int a_d0) : ds(heap), a_d(a_d0) {}

  forceinline const BranchingDesc*
  ReCoStack::push(EngineCtrl& stat, Space* s, Space* c) {
    ReCoNode sn(s,c);
    ds.push(sn);
    if (stat.depth < static_cast<unsigned int>(ds.entries()))
      stat.depth = ds.entries();
    return sn.desc();
  }

  template <class DescType>
  forceinline const BranchingDesc*
  ReCoStack::nextDesc(EngineCtrl& stat, int& alt, int& closedDescs) {
    closedDescs = 0;
    while (!ds.empty())
      if (ds.top().rightmost()) {
        stat.pop(ds.top().space(),ds.top().desc());
        if (dynamic_cast<const DescType*>(ds.top().desc()))
          closedDescs++;
        ds.pop().dispose();
      } else {
        ds.top().next();
        alt = ds.top().alt();
        return ds.top().desc();
      }
    return NULL;
  }

  template <class DescType, bool inclusive>
  forceinline void
  ReCoStack::closeBranch(EngineCtrl& stat) {
    while (!ds.empty()) {
      if (dynamic_cast<const DescType*>(ds.top().desc())) {
        if (inclusive && !ds.empty()) {
          stat.pop(ds.top().space(),ds.top().desc());
          ds.pop().dispose();      
        }
        break;       
      }
      stat.pop(ds.top().space(),ds.top().desc());
      ds.pop().dispose();
    }
  }

  forceinline bool
  ReCoStack::next(EngineCtrl& stat) {
    // Generate path for next node and return whether node exists.
    while (!ds.empty())
      if (ds.top().rightmost()) {
        stat.pop(ds.top().space(),ds.top().desc());
        ds.pop().dispose();
      } else {
        ds.top().next();
        return true;
      }
    return false;
  }

  forceinline void
  ReCoStack::commit(Space* s, int i) const {
    const ReCoNode& n = ds[i];
    s->commit(*n.desc(),n.alt());
  }

  forceinline int
  ReCoStack::lc(void) const {
    int l = ds.entries()-1;
    while (ds[l].space() == NULL)
      l--;
    return l;
  }

  forceinline int
  ReCoStack::entries(void) const {
    return ds.entries();
  }

  forceinline size_t
  ReCoStack::stacksize(void) const {
    return ds.size();
  }

  forceinline void
  ReCoStack::unwind(int l) {
    assert((ds[l].space() == NULL) || ds[l].space()->failed());
    int n = ds.entries();
    for (int i=l; i<n; i++)
      ds.pop().dispose();
    assert(ds.entries() == l);
  }

  inline void
  ReCoStack::reset(void) {
    while (!ds.empty())
      ds.pop().dispose();
  }

  template <bool constrain>
  forceinline Space*
  ReCoStack::recompute(unsigned int& d, EngineCtrl& stat,
                       const Space* best, int& mark) {
    assert(!ds.empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((ds.top().space() != NULL) && ds.top().rightmost()) {
      Space* s = ds.top().space();
      s->commit(*ds.top().desc(),ds.top().alt());
      assert(ds.entries()-1 == lc());
      if (constrain && (mark > ds.entries()-1)) {
        mark = ds.entries()-1;
        s->constrain(*best);
      }
      ds.top().space(NULL);
      stat.lao(s);
      d = 0;
      return s;
    }
    // General case for recomputation
    int l = lc();             // Position of last clone
    int n = ds.entries();     // Number of stack entries
    // New distance, if no adaptive recomputation
    d = static_cast<unsigned int>(n - l); 

    Space* s = ds[l].space(); // Last clone

    if (constrain && (l < mark)) {
      mark = l;
      s->constrain(*best);
      // The space on the stack could be failed now as an additional
      // constraint might have been added.
      if (s->status(stat) == SS_FAILED) {
        // s does not need deletion as it is on the stack (unwind does this)
        stat.fail++;
        unwind(l);
        return NULL;
      }
      // It is important to replace the space on the stack with the
      // copy: a copy might be much smaller due to flushed caches
      // of propagators
      Space* c = s->clone();
      ds[l].space(c);
      stat.constrained(s,c);
    } else {
      s = s->clone();
    }

    if (d < a_d) {
      // No adaptive recomputation
      for (int i=l; i<n; i++)
        commit(s,i);
    } else {
      int m = l + static_cast<int>(d >> 1); // Middle between copy and top
      int i = l;            // To iterate over all entries
      // Recompute up to middle
      for (; i<m; i++ )
        commit(s,i);
      // Skip over all rightmost branches
      for (; (i<n) && ds[i].rightmost(); i++)
        commit(s,i);
      // Is there any point to make a copy?
      if (i<n-1) {
        // Propagate to fixpoint
        SpaceStatus ss = s->status(stat);
        /*
         * Again, the space might already propagate to failure
         *
         * This can be for two reasons:
         *  - constrain is true, so we fail
         *  - the space has weakly monotonic propagators
         */
        if (ss == SS_FAILED) {
          // s must be deleted as it is not on the stack
          delete s;
          stat.fail++;
          unwind(i);
          return NULL;
        }
        ds[i].space(s->clone());
        stat.adapt(ds[i].space());
        d = static_cast<unsigned int>(n-i);
      }
      // Finally do the remaining commits
      for (; i<n; i++)
        commit(s,i);
    }
    return s;
  }

}}

// STATISTICS: search-any
