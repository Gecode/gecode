/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include "gecode/search.hh"

namespace Gecode { namespace Search {

  /*
   * Control for DFS
   *
   */

  DFS::DFS(Space* s, unsigned int c_d, unsigned int a_d, Stop* st, size_t sz) 
    : e(c_d,a_d,st,sz) {
    unsigned int alt;
    unsigned long int p = 0;
    Space* c = (s->status(alt,p) == SS_FAILED) ? NULL : s->clone(true,p);
    e.init(c);
    e.propagate += p;
    e.current(s);
    e.current(NULL);
    e.current(c);
    if (c == NULL)
      e.fail += 1;
  }
  
  Space*
  DFS::next(void) {
    return e.explore();
  }

  bool
  DFS::stopped(void) const {
    return e.stopped();
  }

  Statistics
  DFS::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return s;
  }

}}

// STATISTICS: search-any
