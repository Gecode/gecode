/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Martin Mann <mmann@informatik.uni-freiburg.de>
 *     Sebastian Will <will@informatik.uni-freiburg.de>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *     Martin Mann, 2008
 *     Sebastian Will, 2008
 *
 *  Last modified:
 *     $Date: 2007-04-05 15:25:13 +0200 (Thu, 05 Apr 2007) $ by $Author: tack $
 *     $Revision: 4322 $
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
   * Control for DDS
   *
   */

  DDS::DDS(Space* s, unsigned int c_d, unsigned int a_d, Stop* st,
           size_t sz)
    : e(c_d,a_d,st,sz) {
    unsigned long int p = 0;
    Space* c = (s->status(p) == SS_FAILED) ? NULL : 
                s->clone(true);
    e.init(c);
    e.propagate += p;
    e.current(s);
    e.current(NULL);
    e.current(c);
    if (c == NULL)
      e.fail += 1;
  }

  DDSEngine::ExploreStatus
  DDS::next(Space*& s, int& closedDecomps,
            std::vector<int>& significantVars) {
    return e.explore(s, closedDecomps, significantVars);
  }

  void
  DDS::closeComponent(void) { e.closeComponent(); }

  void
  DDS::closeSingletonBranch(void) { e.closeSingletonBranch(); }

  bool
  DDS::stopped(void) const {
    return e.stopped();
  }

  Statistics
  DDS::statistics(void) const {
    Statistics s = e;
    s.memory += e.stacksize();
    return s;
  }

}}

// STATISTICS: dds-any
