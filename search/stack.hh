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
#include "search/node.hh"
#include "support/dynamic-stack.hh"

namespace Gecode { namespace Search {

  /**
   * \brief Stack of nodes in search tree base-class
   *
   */
  template <class Node>
  class BaseStack : public Support::DynamicStack<Node> {
  public:
    void reset(void);
  };

  /**
   * \brief Stack of nodes for copying
   *
   * Maintains the invariant that it contains
   * the path to the next node to be explored.
   *
   */
  class CopyStack : public BaseStack<CopyNode> {
  public:
    CopyStack(void);
    void push(Space*, unsigned int);
  };


  /**
   * \brief Stack of nodes for recomputation
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
  class ReCoStack : public BaseStack<ReCoNode> {
  private:
    const unsigned int a_d;
  public:
    ReCoStack(unsigned int);

    BranchingDesc* push(Space*, Space*, unsigned int);
    bool next(FullStatistics&);
    Space* recompute(unsigned int&,FullStatistics&);
  };

}}

#include "search/stack.icc"

#endif

// STATISTICS: search-any
