/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "gecode/int/branch.hh"

namespace Gecode {

  using namespace Int;

  void
  branch(Space* home, const IntVarArgs& x, BvarSel vars, BvalSel vals) {
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (vars) {
    case BVAR_NONE:
      Branch::create<Branch::ByNone>(home,xv,vals); break;
    case BVAR_MIN_MIN:
      Branch::create<Branch::ByMinMin>(home,xv,vals); break;
    case BVAR_MIN_MAX:
      Branch::create<Branch::ByMinMax>(home,xv,vals); break;
    case BVAR_MAX_MIN:
      Branch::create<Branch::ByMaxMin>(home,xv,vals); break;
    case BVAR_MAX_MAX:
      Branch::create<Branch::ByMaxMax>(home,xv,vals); break;
    case BVAR_SIZE_MIN:
      Branch::create<Branch::BySizeMin>(home,xv,vals); break;
    case BVAR_SIZE_MAX:
      Branch::create<Branch::BySizeMax>(home,xv,vals); break;
    case BVAR_DEGREE_MIN:
      Branch::create<Branch::ByDegreeMin>(home,xv,vals); break;
    case BVAR_DEGREE_MAX:
      Branch::create<Branch::ByDegreeMax>(home,xv,vals); break;
    case BVAR_REGRET_MIN_MIN:
      Branch::create<Branch::ByRegretMinMin>(home,xv,vals); break;
    case BVAR_REGRET_MIN_MAX:
      Branch::create<Branch::ByRegretMinMax>(home,xv,vals); break;
    case BVAR_REGRET_MAX_MIN:
      Branch::create<Branch::ByRegretMaxMin>(home,xv,vals); break;
    case BVAR_REGRET_MAX_MAX:
      Branch::create<Branch::ByRegretMaxMax>(home,xv,vals); break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  branch(Space* home, const BoolVarArgs& x, BvarSel vars, BvalSel vals) {
    if (home->failed()) return;
    IntVarArgs y(x.size());
    for (int i=x.size(); i--; ) {
      IntVar yi(home,0,1); link(home,x[i],yi); y[i]=yi;
    }
    branch(home,x,vars,vals);
  }

  void
  assign(Space* home, const IntVarArgs& x, AvalSel vals) {
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (vals) {
    case AVAL_MIN: (void) new (home) Branch::AssignMin(home,xv); break;
    case AVAL_MED: (void) new (home) Branch::AssignMed(home,xv); break;
    case AVAL_MAX: (void) new (home) Branch::AssignMax(home,xv); break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  void
  assign(Space* home, const BoolVarArgs& x, AvalSel vals) {
    if (home->failed()) return;
    IntVarArgs y(x.size());
    for (int i=x.size(); i--; ) {
      IntVar yi(home,0,1); link(home,x[i],yi); y[i]=yi;
    }
    assign(home,x,vals);
  }


}


// STATISTICS: int-post

