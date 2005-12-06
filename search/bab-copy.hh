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

#ifndef __GECODE_SEARCH_BAB_COPY_HH__
#define __GECODE_SEARCH_BAB_COPY_HH__

#include "search.hh"
#include "search/stack.hh"

namespace Gecode { namespace Search {

  /**
   * \brief Copying branch-and-bound engine implementation
   *
   */
  class BabCopyEngine : public BabEngine {
  private:
    CopyStack ds;
    unsigned int mark;
    Space* cur;
    Space* b;
    size_t    sz_space; ///< Size of one space
    size_t    sz_cur;   ///< Size of current space
    size_t    sz;       ///< Current total space

  public:
    BabCopyEngine(Space*,size_t);
    virtual size_t stacksize(void) const;
    virtual ~BabCopyEngine(void);
    virtual bool explore(Space*&,Space*&);
  };

}}

#include "search/bab-copy.icc"

#endif

// STATISTICS: search-any
