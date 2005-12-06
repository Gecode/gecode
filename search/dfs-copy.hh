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

#ifndef __GECODE_SEARCH_DFS_COPY_HH__
#define __GECODE_SEARCH_DFS_COPY_HH__

#include "search.hh"
#include "search/stack.hh"

namespace Gecode { namespace Search {

  /**
   * \brief Copying depth-first engine implementation
   *
   */
  class DfsCopyEngine : public PlainEngine {
  private:
    CopyStack ds;       ///< Stack of nodes
    Space*    cur;      ///< Current space (node)
  public:
    DfsCopyEngine(Space*,size_t);
    virtual void reset(Space*);
    virtual ~DfsCopyEngine(void);
    virtual size_t stacksize(void) const;
    virtual Space* explore(void);
  };

}}

#include "search/dfs-copy.icc"

#endif

// STATISTICS: search-any
