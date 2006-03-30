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

#ifndef __GECODE_SEARCH_RECO_STACK_HH__
#define __GECODE_SEARCH_RECO_STACK_HH__

#include "search.hh"
#include "support/dynamic-stack.hh"

namespace Gecode { namespace Search {

  /**
   * \brief %Search tree node for recomputation
   *
   */
  class ReCoNode {
  protected:
    /// Space corresponding to this node (might be NULL)
    Space*         _space;
    /// Current alternative 
    unsigned int   _alt;
    /// Last alternative
    unsigned int   _last;
    /// Braching description
    BranchingDesc* _desc;
  public:
    /// Node for space \a s with clone \a c (possibly NULL) and alternatives \a alt
    ReCoNode(Space* s, Space* c, unsigned int alt);

    /// Return space for node
    Space* space(void) const; 
    /// Return number for alternatives
    unsigned int alt(void) const; 
    /// Return branching description
    BranchingDesc* desc(void) const; 

    /// Set space to \a s
    void space(Space* s);
    /// Set number of alternatives to \a a
    void alt(unsigned int a);
    /// Set branching description to \a d
    void desc(BranchingDesc* d);

    /// Test whether current alternative is rightmost
    bool rightmost(void) const;
    /// Movre to next alternative
    void next(void);
    /// Return the rightmost alternative and remove it
    unsigned int share(void);

    /// Delete space for node
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
  class ReCoStack : public Support::DynamicStack<ReCoNode> {
  private:
    /// Adaptive recomputation distance
    const unsigned int a_d;
  public:
    /// Initialize with adaptive recomputation distance \a a_d
    ReCoStack(unsigned int a_d);
    
    /// Push space \a c (a clone of \a a or NULL) with alternatives \a a
    BranchingDesc* push(Space* s, Space* c, unsigned int a);

    /// Generate path for next node and return whether a next node exists
    bool next(FullStatistics& s);

    /// Recompute space according to path with copying distance \a d
    Space* recompute(unsigned int& d, 
		     FullStatistics& s);

    /// Reset stack
    void reset(void);
  };

}}

#include "search/reco-stack.icc"

#endif

// STATISTICS: search-any
