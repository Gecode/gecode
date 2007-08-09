/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset/branch.hh"

namespace Gecode {

  using namespace CpltSet;

  void
  branch(Space* home, const CpltSetVarArgs& xa, CpltSetBvarSel vars, CpltSetBvalSel vals, 
         SetConLevel scl) {
    // std::cout << "bdd branching\n";

    if (home->failed()) return;

//     std::cout << xa[0].manager() << "\n";
//     std::cout << "start branching\n";

    ViewArray<CpltSetView> x(home,xa);

    switch (vars) {
    case CPLTSET_BVAR_NONE:
      Branch::create<Branch::ByNone>(home,x,vals); break;
    case CPLTSET_BVAR_MIN_CARD:
      Branch::create<Branch::ByMinCard>(home,x,vals); break;
    case CPLTSET_BVAR_MAX_CARD:
      Branch::create<Branch::ByMaxCard>(home,x,vals); break;
    case CPLTSET_BVAR_MIN_UNKNOWN_ELEM:
      Branch::create<Branch::ByMinUnknown>(home,x,vals); break;
    case CPLTSET_BVAR_MAX_UNKNOWN_ELEM:
      Branch::create<Branch::ByMaxUnknown>(home,x,vals); break;
    default:
      throw CpltSet::UnknownBranching("CpltSet::branch not yet implemented");
    }
  }

}

// STATISTICS: bdd-post
