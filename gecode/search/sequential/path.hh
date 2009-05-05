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

#ifndef __GECODE_SEARCH_SEQUENTIAL_PATH_HH__
#define __GECODE_SEARCH_SEQUENTIAL_PATH_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Sequential {

  /**
   * \brief Depth-first path (stack of nodes) supporting recomputation
   *
   * Maintains the invariant that it contains
   * the path of the node being currently explored. This
   * is required to support recomputation, of course.
   *
   * The path supports adaptive recomputation controlled
   * by the value of a_d: only if the recomputation
   * distance is at least this large, an additional
   * clone is created.
   *
   */
  class Path {
  public:
    /// %Search tree node for recomputation
    class Node {
    protected:
      /// Space corresponding to this node (might be NULL)
      Space* _space;
      /// Current alternative
      unsigned int _alt;
      /// Braching description
      const BranchingDesc* _desc;
    public:
      /// Default constructor
      Node(void);
      /// Node for space \a s with clone \a c (possibly NULL)
      Node(Space* s, Space* c);
      
      /// Return space for node
      Space* space(void) const;
      /// Set space to \a s
      void space(Space* s);
      
      /// Return branching description
      const BranchingDesc* desc(void) const;
      
      /// Return number for alternatives
      unsigned int alt(void) const;
      /// Test whether current alternative is rightmost
      bool rightmost(void) const;
      /// Move to next alternative
      void next(void);
      
      /// Free memory for node
      void dispose(void);
    };
    /// Stack to store node information
    Support::DynamicStack<Node,Heap> ds;
  public:
    /// Initialize
    Path(void);
    /// Push space \a c (a clone of \a s or NULL)
    const BranchingDesc* push(Worker& stat, Space* s, Space* c);
    /// Generate path for next node and return whether a next node exists
    bool next(Worker& s);
    /// Return position on stack of last copy
    int lc(void) const;
    /// Unwind the stack up to position \a l (after failure)
    void unwind(int l);
    /// Commit space \a s as described by stack entry at position \a i
    void commit(Space* s, int i) const;
    /// Recompute space according to path 
    Space* recompute(unsigned int& d, unsigned int a_d, Worker& s);
    /// Recompute space according to path
    Space* recompute(unsigned int& d, unsigned int a_d, Worker& s,
                     const Space* best, int& mark);
    /// Return number of entries on stack
    int entries(void) const;
    /// Return size used
    size_t size(void) const;
    /// Reset stack
    void reset(void);
  };


  /*
   * Node for recomputation
   *
   */
  forceinline
  Path::Node::Node(void) {}

  forceinline
  Path::Node::Node(Space* s, Space* c)
    : _space(c), _alt(0), _desc(s->description()) {}

  forceinline Space*
  Path::Node::space(void) const {
    return _space;
  }
  forceinline void
  Path::Node::space(Space* s) {
    _space = s;
  }

  forceinline unsigned int
  Path::Node::alt(void) const {
    return _alt;
  }
  forceinline bool
  Path::Node::rightmost(void) const {
    return _alt+1 == _desc->alternatives();
  }
  forceinline void
  Path::Node::next(void) {
    _alt++;
  }

  forceinline const BranchingDesc*
  Path::Node::desc(void) const {
    return _desc;
  }

  forceinline void
  Path::Node::dispose(void) {
    delete _space;
    delete _desc;
  }



  /*
   * Depth-first stack with recomputation
   *
   */

  forceinline
  Path::Path(void) : ds(heap) {}

  forceinline const BranchingDesc*
  Path::push(Worker& stat, Space* s, Space* c) {
    Node sn(s,c);
    ds.push(sn);
    stat.stack_depth(static_cast<unsigned long int>(ds.entries()));
    return sn.desc();
  }

  forceinline bool
  Path::next(Worker& stat) {
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
  Path::commit(Space* s, int i) const {
    const Node& n = ds[i];
    s->commit(*n.desc(),n.alt());
  }

  forceinline int
  Path::lc(void) const {
    int l = ds.entries()-1;
    while (ds[l].space() == NULL)
      l--;
    return l;
  }

  forceinline int
  Path::entries(void) const {
    return ds.entries();
  }

  forceinline size_t
  Path::size(void) const {
    return ds.size();
  }

  forceinline void
  Path::unwind(int l) {
    assert((ds[l].space() == NULL) || ds[l].space()->failed());
    int n = ds.entries();
    for (int i=l; i<n; i++)
      ds.pop().dispose();
    assert(ds.entries() == l);
  }

  inline void
  Path::reset(void) {
    while (!ds.empty())
      ds.pop().dispose();
  }

  forceinline Space*
  Path::recompute(unsigned int& d, unsigned int a_d, Worker& stat) {
    assert(!ds.empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((ds.top().space() != NULL) && ds.top().rightmost()) {
      Space* s = ds.top().space();
      s->commit(*ds.top().desc(),ds.top().alt());
      assert(ds.entries()-1 == lc());
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

  forceinline Space*
  Path::recompute(unsigned int& d, unsigned int a_d, Worker& stat,
                  const Space* best, int& mark) {
    assert(!ds.empty());
    // Recompute space according to path
    // Also say distance to copy (d == 0) requires immediate copying

    // Check for LAO
    if ((ds.top().space() != NULL) && ds.top().rightmost()) {
      Space* s = ds.top().space();
      s->commit(*ds.top().desc(),ds.top().alt());
      assert(ds.entries()-1 == lc());
      if (mark > ds.entries()-1) {
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

    if (l < mark) {
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

}}}

#endif

// STATISTICS: search-any
