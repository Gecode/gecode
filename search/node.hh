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

#ifndef __GECODE_SEARCH_NODE_HH__
#define __GECODE_SEARCH_NODE_HH__

#include "search.hh"

/*
 * Node information
 *
 */

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
  class ReCoNode : public CopyNode {
  protected:
    BranchingDesc* _desc;
  public:
    ReCoNode(Space*, Space*, unsigned int);

    BranchingDesc* desc(void) const; void desc(BranchingDesc*);

    void dispose(void);
  };

}}

#include "search/node.icc"

#endif

// STATISTICS: search-any
