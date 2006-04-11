/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

#include "gecode/set/branch.hh"

namespace Gecode {

  using namespace Set;

  void
  branch(Space* home, const SetVarArgs& xa, SetBvarSel vars, SetBvalSel vals) {
    if (home->failed()) return;
    ViewArray<SetView> x(home,xa);
    switch (vars) {
    case SETBVAR_NONE:
      Branch::create<Branch::ByNone>(home,x,vals); break;
    case SETBVAR_MIN_CARD:
      Branch::create<Branch::ByMinCard>(home,x,vals); break;
    case SETBVAR_MAX_CARD:
      Branch::create<Branch::ByMaxCard>(home,x,vals); break;
    case SETBVAR_MIN_UNKNOWN_ELEM:
      Branch::create<Branch::ByMinUnknown>(home,x,vals); break;
    case SETBVAR_MAX_UNKNOWN_ELEM:
      Branch::create<Branch::ByMaxUnknown>(home,x,vals); break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

}

// STATISTICS: set-post

