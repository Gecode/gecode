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
 *     $Date: 2009-01-20 13:23:36 +0100 (Tue, 20 Jan 2009) $ by $Author: schulte $
 *     $Revision: 8073 $
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
    /// %Stop-object to be used
    Stop* st;
    /// Whether engine has been stopped
    bool _stopped;
    /// Memory required for a single space
    size_t mem_space;
    /// Memory for the current space (including memory for caching)
    size_t mem_cur;
    /// Current total memory
    size_t mem_total;
  public:
    /// Initialize with stop-object \a st and space size \a sz
    EngineCtrl(Stop* st, size_t sz);
    /// Reset stop information
    void start(void);
    /// Check whether engine must be stopped (with additional stackspace \a sz)
    bool stop(size_t sz);
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
    Space*               _space;
    /// Current alternative
    unsigned int         _alt;
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
    /// Adaptive recomputation distance
    const unsigned int a_d;
  public:
    /// Initialize with adaptive recomputation distance \a a_d
    ReCoStack(unsigned int a_d);
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
    /// Recompute space according to path with copying distance \a d
    Space* recompute(unsigned int& d, EngineCtrl& s);
    /// Recompute space according to path with copying distance \a d
    Space* recompute(unsigned int& d, EngineCtrl& s, 
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
    /// Recomputation stack of nodes
    ReCoStack          rcs;
    /// Current space being explored
    Space*             cur;
    /// Copying recomputation distance
    const unsigned int c_d;
    /// Distance until next clone
    unsigned int       d;
  public:
    /**
     * \brief Initialize engine
     * \param c_d minimal recomputation distance
     * \param a_d adaptive recomputation distance
     * \param st %Stop-object
     * \param sz size of one space
     */
    DfsEngine(unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
    /// Initialize engine to start at space \a s
    void init(Space* s);
    /// Reset engine to restart at space \a s
    void reset(Space* s);
    /// Reset engine to restart at failed space
    void reset(void);
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
    ProbeEngine(Stop* st, size_t s);
    /// Initialize with space \a s and discrepancy \a d
    void init(Space* s, unsigned int d);
    /// Reset with space \a s and discrepancy \a d
    void reset(Space* s, unsigned int d);
    /// Return stack size used by engine
    size_t stacksize(void) const;
    /// Destructor
    ~ProbeEngine(void);
    /// %Search for next solution
    Space* explore(void);
    /// Test whether probing is done
    bool done(void) const;
  };
  
  /**
   * \brief Implementation of depth-first branch-and-bound search engines
   */
  class BabEngine : public EngineCtrl {
  private:
    /// Recomputation stack of nodes
    ReCoStack          rcs;
    /// Current space being explored
    Space*             cur;
    /// Number of entries not yet constrained to be better
    int                mark;
    /// Best solution found so far
    Space*             best;
    /// Copying recomputation distance
    const unsigned int c_d;
    /// Distance until next clone
    unsigned int       d;
  public:
    /**
     * \brief Initialize engine
     * \param c_d minimal recomputation distance
     * \param a_d adaptive recomputation distance
     * \param st %Stop-object
     * \param sz size of one space
     */
    BabEngine(unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
    /// Initialize engine to start at space \a s
    void init(Space* s);
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
