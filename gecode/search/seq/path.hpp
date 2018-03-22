/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode { namespace Search { namespace Seq {

  /*
   * Edge for recomputation
   *
   */
  template<class Tracer>
  forceinline
  Path<Tracer>::Edge::Edge(void) {}

  template<class Tracer>
  forceinline
  Path<Tracer>::Edge::Edge(Space* s, Space* c, unsigned int nid)
    : _space(c), _alt(0), _choice(s->choice()), _nid(nid) {}

  template<class Tracer>
  forceinline Space*
  Path<Tracer>::Edge::space(void) const {
    return _space;
  }
  template<class Tracer>
  forceinline void
  Path<Tracer>::Edge::space(Space* s) {
    _space = s;
  }

  template<class Tracer>
  forceinline unsigned int
  Path<Tracer>::Edge::alt(void) const {
    return _alt;
  }
  template<class Tracer>
  forceinline unsigned int
  Path<Tracer>::Edge::truealt(void) const {
    return std::min(_alt,_choice->alternatives()-1);
  }
  template<class Tracer>
  forceinline bool
  Path<Tracer>::Edge::leftmost(void) const {
    return _alt == 0;
  }
  template<class Tracer>
  forceinline bool
  Path<Tracer>::Edge::rightmost(void) const {
    return _alt+1 >= _choice->alternatives();
  }
  template<class Tracer>
  forceinline bool
  Path<Tracer>::Edge::lao(void) const {
    return _alt >= _choice->alternatives();
  }
  template<class Tracer>
  forceinline void
  Path<Tracer>::Edge::next(void) {
    _alt++;
  }

  template<class Tracer>
  forceinline const Choice*
  Path<Tracer>::Edge::choice(void) const {
    return _choice;
  }

  template<class Tracer>
  forceinline unsigned int
  Path<Tracer>::Edge::nid(void) const {
    return _nid;
  }

  template<class Tracer>
  forceinline void
  Path<Tracer>::Edge::dispose(void) {
    delete _space;
    delete _choice;
  }



  /*
   * Depth-first stack with recomputation
   *
   */

  template<class Tracer>
  forceinline
  Path<Tracer>::Path(unsigned int l)
    : ds(heap), _ngdl(l) {}

  template<class Tracer>
  forceinline unsigned int
  Path<Tracer>::ngdl(void) const {
    return _ngdl;
  }

  template<class Tracer>
  forceinline void
  Path<Tracer>::ngdl(unsigned int l) {
    _ngdl = l;
  }

  template<class Tracer>
  forceinline const Choice*
  Path<Tracer>::push(Worker& stat, Space* s, Space* c, unsigned int nid) {
    if (!ds.empty() && ds.top().lao()) {
      // Topmost stack entry was LAO -> reuse
      ds.pop().dispose();
    }
    Edge sn(s,c,nid);
    ds.push(sn);
    stat.stack_depth(static_cast<unsigned long int>(ds.entries()));
    return sn.choice();
  }

  template<class Tracer>
  forceinline void
  Path<Tracer>::next(void) {
    while (!ds.empty())
      if (ds.top().rightmost()) {
        ds.pop().dispose();
      } else {
        ds.top().next();
        return;
      }
  }

  template<class Tracer>
  forceinline typename Path<Tracer>::Edge&
  Path<Tracer>::top(void) const {
    assert(!ds.empty());
    return ds.top();
  }

  template<class Tracer>
  forceinline bool
  Path<Tracer>::empty(void) const {
    return ds.empty();
  }

  template<class Tracer>
  forceinline void
  Path<Tracer>::commit(Space* s, int i) const {
    const Edge& n = ds[i];
    s->commit(*n.choice(),n.alt());
  }

  template<class Tracer>
  forceinline int
  Path<Tracer>::lc(void) const {
    int l = ds.entries()-1;
    while (ds[l].space() == NULL)
      l--;
    return l;
  }

  template<class Tracer>
  forceinline int
  Path<Tracer>::entries(void) const {
    return ds.entries();
  }

  template<class Tracer>
  forceinline void
  Path<Tracer>::unwind(int l, Tracer& t) {
    assert((ds[l].space() == NULL) || ds[l].space()->failed());
    int n = ds.entries();
    if (t) {
      for (int i=l; i<n; i++) {
        Path<Tracer>::Edge& top = ds.top();
        unsigned int fa = (i != l) ? top.alt() + 1 : top.alt();
        for (unsigned int a = fa; a < top.choice()->alternatives(); a++) {
          SearchTracer::EdgeInfo ei(t.wid(),top.nid(),a);
          t.skip(ei);
        }
        ds.pop().dispose();
      }
    } else {
      for (int i=l; i<n; i++)
        ds.pop().dispose();
    }
    assert(ds.entries() == l);
  }

  template<class Tracer>
  inline void
  Path<Tracer>::reset(void) {
    while (!ds.empty())
      ds.pop().dispose();
  }

  template<class Tracer>
  forceinline Space*
  Path<Tracer>::recompute(unsigned int& d, unsigned int a_d, Worker& stat,
                          Tracer& t) {
    assert(!ds.empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((ds.top().space() != NULL) && ds.top().rightmost()) {
      Space* s = ds.top().space();
      s->commit(*ds.top().choice(),ds.top().alt());
      assert(ds.entries()-1 == lc());
      ds.top().space(NULL);
      // Mark as reusable
      if (static_cast<unsigned int>(ds.entries()) > ngdl())
        ds.top().next();
      d = 0;
      return s;
    }
    // General case for recomputation
    int l = lc();             // Position of last clone
    int n = ds.entries();     // Number of stack entries
    // New distance, if no adaptive recomputation
    d = static_cast<unsigned int>(n - l);

    Space* s = ds[l].space()->clone(); // Last clone

    if (d < a_d) {
      // No adaptive recomputation
      for (int i=l; i<n; i++)
        commit(s,i);
    } else {
      int m = l + static_cast<int>(d >> 1); // Middle between copy and top
      int i = l; // To iterate over all entries
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
         * Again, the space might already propagate to failure (due to
         * weakly monotonic propagators).
         */
        if (ss == SS_FAILED) {
          // s must be deleted as it is not on the stack
          delete s;
          stat.fail++;
          unwind(i,t);
          return NULL;
        }
        ds[i].space(s->clone());
        d = static_cast<unsigned int>(n-i);
      }
      // Finally do the remaining commits
      for (; i<n; i++)
        commit(s,i);
    }
    return s;
  }

  template<class Tracer>
  forceinline Space*
  Path<Tracer>::recompute(unsigned int& d, unsigned int a_d, Worker& stat,
                          const Space& best, int& mark,
                          Tracer& t) {
    assert(!ds.empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((ds.top().space() != NULL) && ds.top().rightmost()) {
      Space* s = ds.top().space();
      s->commit(*ds.top().choice(),ds.top().alt());
      assert(ds.entries()-1 == lc());
      if (mark > ds.entries()-1) {
        mark = ds.entries()-1;
        s->constrain(best);
      }
      ds.top().space(NULL);
      // Mark as reusable
      if (static_cast<unsigned int>(ds.entries()) > ngdl())
        ds.top().next();
      d = 0;
      return s;
    }
    // General case for recomputation
    int l = lc();             // Position of last clone
    int n = ds.entries();     // Number of stack entries
    // New distance, if no adaptive recomputation
    d = static_cast<unsigned int>(n - l);

    Space* s = ds[l].space(); // Last clone

    if (l < mark) {
      mark = l;
      s->constrain(best);
      // The space on the stack could be failed now as an additional
      // constraint might have been added.
      if (s->status(stat) == SS_FAILED) {
        // s does not need deletion as it is on the stack (unwind does this)
        stat.fail++;
        unwind(l,t);
        return NULL;
      }
      // It is important to replace the space on the stack with the
      // copy: a copy might be much smaller due to flushed caches
      // of propagators
      Space* c = s->clone();
      ds[l].space(c);
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
          unwind(i,t);
          return NULL;
        }
        ds[i].space(s->clone());
        d = static_cast<unsigned int>(n-i);
      }
      // Finally do the remaining commits
      for (; i<n; i++)
        commit(s,i);
    }
    return s;
  }

  template<class Tracer>
  void
  Path<Tracer>::post(Space& home) const {
    GECODE_ES_FAIL(NoGoodsProp::post(home,*this));
  }

}}}

// STATISTICS: search-seq
