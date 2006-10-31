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
      Branch::create<IntView,Branch::ByNone>(home,xv,vals); break;
    case BVAR_MIN_MIN:
      Branch::create<IntView,Branch::ByMinMin>(home,xv,vals); break;
    case BVAR_MIN_MAX:
      Branch::create<IntView,Branch::ByMinMax>(home,xv,vals); break;
    case BVAR_MAX_MIN:
      Branch::create<IntView,Branch::ByMaxMin>(home,xv,vals); break;
    case BVAR_MAX_MAX:
      Branch::create<IntView,Branch::ByMaxMax>(home,xv,vals); break;
    case BVAR_SIZE_MIN:
      Branch::create<IntView,Branch::BySizeMin>(home,xv,vals); break;
    case BVAR_SIZE_MAX:
      Branch::create<IntView,Branch::BySizeMax>(home,xv,vals); break;
    case BVAR_DEGREE_MIN:
      Branch::create<IntView,Branch::ByDegreeMin>(home,xv,vals); break;
    case BVAR_DEGREE_MAX:
      Branch::create<IntView,Branch::ByDegreeMax>(home,xv,vals); break;
    case BVAR_REGRET_MIN_MIN:
      Branch::create<IntView,Branch::ByRegretMinMin>(home,xv,vals); break;
    case BVAR_REGRET_MIN_MAX:
      Branch::create<IntView,Branch::ByRegretMinMax>(home,xv,vals); break;
    case BVAR_REGRET_MAX_MIN:
      Branch::create<IntView,Branch::ByRegretMaxMin>(home,xv,vals); break;
    case BVAR_REGRET_MAX_MAX:
      Branch::create<IntView,Branch::ByRegretMaxMax>(home,xv,vals); break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  branch(Space* home, const BoolVarArgs& x, BvarSel vars, BvalSel vals) {
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    switch (vars) {
    case BVAR_NONE:
    case BVAR_MIN_MIN:
    case BVAR_MIN_MAX:
    case BVAR_MAX_MIN:
    case BVAR_MAX_MAX:
    case BVAR_SIZE_MIN:
    case BVAR_SIZE_MAX:
    case BVAR_REGRET_MIN_MIN:
    case BVAR_REGRET_MIN_MAX:
    case BVAR_REGRET_MAX_MIN:
    case BVAR_REGRET_MAX_MAX:
      switch (vals) {
      case BVAL_MIN:
      case BVAL_MED:
      case BVAL_SPLIT_MIN:
        (void) new (home) 
          ViewValBranching<BoolView,Branch::NoValue,
                           Branch::ByNone<BoolView>,
                           Branch::ValZeroOne<BoolView> >(home,xv);
        break;
      case BVAL_MAX:
      case BVAL_SPLIT_MAX:
        (void) new (home) 
          ViewValBranching<BoolView,Branch::NoValue,
                           Branch::ByNone<BoolView>,
                           Branch::ValOneZero<BoolView> >(home,xv);
        break;
      default:
        throw UnknownBranching("Int::branch");
      }
      break;
    case BVAR_DEGREE_MIN:
      switch (vals) {
      case BVAL_MIN:
      case BVAL_MED:
      case BVAL_SPLIT_MIN:
        (void) new (home) 
          ViewValBranching<BoolView,Branch::NoValue,
                           Branch::ByDegreeMinNoTies<BoolView>,
                           Branch::ValZeroOne<BoolView> >(home,xv);
        break;
      case BVAL_MAX:
      case BVAL_SPLIT_MAX:
        (void) new (home) 
          ViewValBranching<BoolView,Branch::NoValue,
                           Branch::ByDegreeMinNoTies<BoolView>,
                           Branch::ValOneZero<BoolView> >(home,xv);
        break;
      default:
        throw UnknownBranching("Int::branch");
      }
      break;
    case BVAR_DEGREE_MAX:
      switch (vals) {
      case BVAL_MIN:
      case BVAL_MED:
      case BVAL_SPLIT_MIN:
        (void) new (home) 
          ViewValBranching<BoolView,Branch::NoValue,
                           Branch::ByDegreeMaxNoTies<BoolView>,
                           Branch::ValZeroOne<BoolView> >(home,xv);
        break;
      case BVAL_MAX:
      case BVAL_SPLIT_MAX:
        (void) new (home) 
          ViewValBranching<BoolView,Branch::NoValue,
                           Branch::ByDegreeMaxNoTies<BoolView>,
                           Branch::ValOneZero<BoolView> >(home,xv);
        break;
      default:
        throw UnknownBranching("Int::branch");
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
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
    assign(home,y,vals);
  }


}


// STATISTICS: int-post

