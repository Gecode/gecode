/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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

  /**
   * \brief %Search engine control including memory information
   */
  class EngineCtrl : public Statistics {
  protected:
    /// Whether engine has been stopped
    bool _stopped;
    /// Memory required for a single space
    size_t mem_space;
    /// Memory for the current space (including memory for caching)
    size_t mem_cur;
    /// Current total memory
    size_t mem_total;
  public:
    /// Initialize with space size \a sz
    EngineCtrl(size_t sz);
    /// Reset stop information
    void start(void);
    /// Check whether engine must be stopped (with additional stackspace \a sz)
    bool stop(Stop* st, size_t sz);
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// New space \a s and branching description \a d get pushed on stack
    void push(const Space* s, const BranchingDesc* d);
    /// Space \a s1 is replaced by space \a s2 due to constraining
    void constrained(const Space* s1, const Space* s2);
    /// New space \a s is added for adaptive recomputation
    void adapt(const Space* s);
    /// Space \a s and branching description \a d get popped from stack
    void pop(const Space* s, const BranchingDesc* d);
    /// Space \a s gets used for LAO (removed from stack)
    void lao(const Space* s);
    /// Space \a s becomes current space (\a s = NULL: current space deleted)
    void current(const Space* s);
    /// Reset statistics for space \a s
    void reset(const Space* s);
    /// Reset statistics for failed space
    void reset(void);
  };

  /**
   * \brief %Search tree node for recomputation
   *
   */
  class ReCoNode {
  protected:
    /// Space corresponding to this node (might be NULL)
    Space* _space;
    /// Current alternative
    unsigned int _alt;
    /// Braching description
    const BranchingDesc* _desc;
  public:
    /// Default constructor
    ReCoNode(void);
    /// Node for space \a s with clone \a c (possibly NULL)
    ReCoNode(Space* s, Space* c);

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


  /**
   * \brief Stack of nodes supporting recomputation
   *
   * Maintains the invariant that it contains
   * the path of the node being currently explored. This
   * is required to support recomputation, of course.
   *
   * The stack supports adaptive recomputation controlled
   * by the value of a_d: only if the recomputation
   * distance is at least this large, an additional
   * clone is created.
   *
   */
  class ReCoStack {
  private:
    /// Stack to store node information
    Support::DynamicStack<ReCoNode,Heap> ds;
  public:
    /// Initialize
    ReCoStack(void);
    /// Push space \a c (a clone of \a s or NULL)
    const BranchingDesc* push(EngineCtrl& stat, Space* s, Space* c);
    /// Generate path for next node and return BranchingDesc for next node if its type is \a DescType, or NULL otherwise
    template <class DescType>
    const BranchingDesc* nextDesc(EngineCtrl& s, int& alt,
                                  int& closedDescs);
    /// Generate path for next node with BranchingDesc type DescType
    template <class DescType, bool inclusive>
    void closeBranch(EngineCtrl& s);
    /// Generate path for next node and return whether a next node exists
    bool next(EngineCtrl& s);
    /// Return position on stack of last copy
    int lc(void) const;
    /// Unwind the stack up to position \a l (after failure)
    void unwind(int l);
    /// Commit space \a s as described by stack entry at position \a i
    void commit(Space* s, int i) const;
    /// Recompute space according to path 
    Space* recompute(unsigned int& d, unsigned int a_d, EngineCtrl& s);
    /// Recompute space according to path
    Space* recompute(unsigned int& d, unsigned int a_d, EngineCtrl& s,
                     const Space* best, int& mark);
    /// Return number of entries on stack
    int entries(void) const;
    /// Return stack size used
    size_t stacksize(void) const;
    /// Reset stack
    void reset(void);
  };

  /**
   * \brief Depth-first search engine implementation
   *
   */
  class DfsEngine : public EngineCtrl {
  private:
    /// Search options
    Options opt;
    /// Recomputation stack of nodes
    ReCoStack rcs;
    /// Current space being explored
    Space* cur;
    /// Distance until next clone
    unsigned int d;
  public:
    /// Initialize for space \a s (of size \a sz) with options \a o)
    DfsEngine(Space* s, size_t sz, const Options& o);
    /// Reset engine to restart at space \a s
    void reset(Space* s);
    /// %Search for next solution
    Space* explore(void);
    /// Return stack size used by engine
    size_t stacksize(void) const;
    /// Destructor
    ~DfsEngine(void);
  };

  /**
   * \brief Probing engine for %LDS
   *
   */
  class ProbeEngine : public EngineCtrl {
  protected:
    /// %Node in the search tree for %LDS
    class ProbeNode {
    private:
      /// %Space of current node
      Space*               _space;
      /// Branching description
      const BranchingDesc* _desc;
      /// Next alternative to try
      unsigned int         _alt;
    public:
      /// Default constructor
      ProbeNode(void);
      /// Initialize with node \a s, description \a d, and alternative \a a
      ProbeNode(Space* s, const BranchingDesc* d, unsigned int a);
      /// Return space
      Space* space(void) const;
      /// Return branching description
      const BranchingDesc* desc(void) const;
      /// Return next alternative
      unsigned int alt(void) const;
      /// %Set next alternative
      void next(void);
    };
    /// %Stack storing current path in search tree
    Support::DynamicStack<ProbeNode,Heap> ds;
    /// Current space
    Space* cur;
    /// Current discrepancy
    unsigned int d;
    /// Whether entire search space has been exhausted
    bool exhausted;
  public:
    /// Initialize for spaces of size \a s
    ProbeEngine(size_t s);
    /// Initialize with space \a s and discrepancy \a d
    void init(Space* s, unsigned int d);
    /// Reset with space \a s and discrepancy \a d
    void reset(Space* s, unsigned int d);
    /// Return stack size used by engine
    size_t stacksize(void) const;
    /// Destructor
    ~ProbeEngine(void);
    /// %Search for next solution
    Space* explore(Stop* st);
    /// Test whether probing is done
    bool done(void) const;
  };

  /**
   * \brief Implementation of depth-first branch-and-bound search engines
   */
  class BabEngine : public EngineCtrl {
  private:
    /// Search options
    Options opt;
    /// Recomputation stack of nodes
    ReCoStack rcs;
    /// Current space being explored
    Space* cur;
    /// Distance until next clone
    unsigned int d;
    /// Number of entries not yet constrained to be better
    int mark;
    /// Best solution found so far
    Space* best;
  public:
    /// Initialize with space \a s (of size \a sz) and search options \a o
    BabEngine(Space* s, size_t sz, const Options& o);
    /// %Search for next better solution
    Space* explore(void);
    /// Return stack size used by engine
    size_t stacksize(void) const;
    /// Destructor
    ~BabEngine(void);
  };

}}

#include <gecode/search/engine/ctrl.hpp>
#include <gecode/search/engine/reco-stack.hpp>
#include <gecode/search/engine/dfs.hpp>
#include <gecode/search/engine/lds.hpp>
#include <gecode/search/engine/bab.hpp>

// STATISTICS: search-any
