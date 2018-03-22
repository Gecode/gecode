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

#ifndef __GECODE_SEARCH_PAR_PATH_HH__
#define __GECODE_SEARCH_PAR_PATH_HH__

#include <algorithm>

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>
#include <gecode/search/nogoods.hh>

namespace Gecode { namespace Search { namespace Par {

  /**
   * \brief Depth-first path (stack of edges) supporting recomputation
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
  template<class Tracer>
  class Path : public NoGoods {
    friend class Search::NoGoodsProp;
  public:
    /// Identity type
    typedef typename Tracer::ID ID;
    /// %Search tree edge for recomputation
    class Edge {
    protected:
      /// Space corresponding to this edge (might be NULL)
      Space* _space;
      /// Current alternative
      unsigned int _alt;
      /// Number of alternatives left
      unsigned int _alt_max;
      /// Choice
      const Choice* _choice;
      /// Node identifier
      ID _nid;
    public:
      /// Default constructor
      Edge(void);
      /// Edge for space \a s with clone \a c (possibly NULL)
      Edge(Space* s, Space* c, unsigned int nid);

      /// Return space for edge
      Space* space(void) const;
      /// Set space to \a s
      void space(Space* s);

      /// Return choice
      const Choice* choice(void) const;

      /// Return number for alternatives
      unsigned int alt(void) const;
      /// Return true number for alternatives (excluding lao optimization)
      unsigned int truealt(void) const;
      /// Test whether current alternative is rightmost
      bool rightmost(void) const;
      /// Test whether current alternative was LAO
      bool lao(void) const;
      /// Test whether there is an alternative that can be stolen
      bool work(void) const;
      /// Move to next alternative
      void next(void);
      /// Steal rightmost alternative and return its number
      unsigned int steal(void);

      /// Return node identifier
      unsigned int nid(void) const;

      /// Free memory for edge
      void dispose(void);
    };
  protected:
    /// Stack to store edge information
    Support::DynamicStack<Edge,Heap> ds;
    /// Depth limit for no-good generation
    unsigned int _ngdl;
    /// Number of edges that have work for stealing
    unsigned int n_work;
  public:
    /// Initialize with no-good depth limit \a l
    Path(unsigned int l);
    /// Return no-good depth limit
    unsigned int ngdl(void) const;
    /// Set no-good depth limit to \a l
    void ngdl(unsigned int l);
    /// Push space \a c (a clone of \a s or NULL)
    const Choice* push(Worker& stat, Space* s, Space* c, unsigned int nid);
    /// Generate path for next node
    void next(void);
    /// Provide access to topmost edge
    Edge& top(void) const;
    /// Test whether path is empty
    bool empty(void) const;
    /// Return position on stack of last copy
    int lc(void) const;
    /// Unwind the stack up to position \a l (after failure)
    void unwind(int l, Tracer& t);
    /// Commit space \a s as described by stack entry at position \a i
    void commit(Space* s, int i) const;
    /// Recompute space according to path
    Space* recompute(unsigned int& d, unsigned int a_d, Worker& s,
                     Tracer& t);
    /// Recompute space according to path
    Space* recompute(unsigned int& d, unsigned int a_d, Worker& s,
                     const Space& best, int& mark,
                     Tracer& t);
    /// Return number of entries on stack
    int entries(void) const;
    /// Reset stack and set no-good depth limit to \a l
    void reset(unsigned int l);
    /// Make a quick check whether stealing might be feasible
    bool steal(void) const;
    /// Steal work at depth \a d
    Space* steal(Worker& stat, unsigned long int& d,
                 Tracer& myt, Tracer& ot);
    /// Post no-goods
    void virtual post(Space& home) const;
  };

}}}

#include <gecode/search/par/path.hpp>

#endif

// STATISTICS: search-par
