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

#include <gecode/dds.hh>
#include <gecode/int/branch.hh>

namespace Gecode { namespace Decomposition {

  /*
   * Control for DDS
   *
   */

  DDS::DDS(Space* s, unsigned int c_d, unsigned int a_d, Search::Stop* st,
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

  bool
  DDS::stopped(void) const {
    return e.stopped();
  }

  Search::Statistics
  DDS::statistics(void) const {
    Search::Statistics s = e;
    s.memory += e.stacksize();
    return s;
  }

}

  // void
  // decomposingBranch(Space& home, const IntVarArgs& x,
  //                   IntVarBranch vars, IntValBranch vals) {
  //   Int::Branch::createBranch<DecomposingViewValBranching>(home,x,vars,vals);
  // }
  //
  // void
  // decomposingBranch(Space& home, const BoolVarArgs& x,
  //                   IntVarBranch vars, IntValBranch vals) {
  //   Int::Branch::createBranch<DecomposingViewValBranching>(home,x,vars,vals);
  // }

}

// STATISTICS: dds-any
