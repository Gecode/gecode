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

#ifndef __GECODE_SEARCH_BAB_RECO_HH__
#define __GECODE_SEARCH_BAB_RECO_HH__

#include "search.hh"
#include "search/stack.hh"

namespace Gecode { namespace Search {

  /**
   * \brief Recomputing branch-and-bound engine implementation
   *
   */
  class BabReCoEngine : public BabEngine {
  private:
    NodeStack          ds;
    unsigned int       mark;
    Space*             cur;
    Space*             best;
    const unsigned int c_d;
    unsigned int       d;
    size_t    sz_space; ///< Size of one space
    size_t    sz_cur;   ///< Size of current space
    size_t    sz;       ///< Current total space
  public:
    BabReCoEngine(Space*,unsigned int,unsigned int,size_t);
    virtual size_t stacksize(void) const;
    virtual bool explore(Space*&,Space*&);
    virtual ~BabReCoEngine(void);
  };

}}

#include "search/bab-reco.icc"

#endif

// STATISTICS: search-any
