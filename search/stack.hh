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
   * \brief %Search tree node for copying
   *
   */
  class CopyNode {
  protected:
    Space*       _space;
    unsigned int _alt;
    unsigned int _last;
  public:
    CopyNode(Space*, unsigned int);
    CopyNode(Space*, Space*, unsigned int);

    Space* space(void) const; void space(Space*);
    unsigned int alt(void) const; void alt(unsigned int);

    bool rightmost(void) const;
    void next(void);

    void dispose(void);
	
	  unsigned int share(void);
  };


  /**
   * \brief %Search tree node for recomputation
   *
   */
  class Node : public CopyNode {
  protected:
    BranchingDesc* _desc;
  public:
    Node(Space*, Space*, unsigned int);

    BranchingDesc* desc(void) const; void desc(BranchingDesc*);

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
  class NodeStack : public Support::DynamicStack<Node> {
  private:
    /// Adaptive recomputation distance
    const unsigned int a_d;
  public:
    /// Initialize with adaptive recomputation distance \a a_d
    NodeStack(unsigned int a_d);
    
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

#include "search/stack.icc"

#endif

// STATISTICS: search-any
