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

#ifndef __GECODE_SEARCH_DFS_RECO_HH__
#define __GECODE_SEARCH_DFS_RECO_HH__

#include "search.hh"
#include "search/reco-stack.hh"

namespace Gecode { namespace Search {

  /**
   * \brief Recomputing depth-first engine implementation
   *
   */
  class DfsReCoEngine : public PlainEngine {
  private:
    ReCoStack          ds;
    Space*             cur;
    const unsigned int c_d;
    unsigned int       d;
  public:
    DfsReCoEngine(Space*,unsigned int,unsigned int,size_t);
    virtual void reset(Space*);
    virtual size_t stacksize(void) const;
    virtual ~DfsReCoEngine(void);
    virtual Space* explore(void);
  };

}}

#include "search/dfs-reco.icc"

#endif

// STATISTICS: search-any
