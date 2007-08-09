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
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
    case AVAL_MIN:
      (void) new (home) 
        ViewValAssignment<IntView,int,
                          Branch::ValMin<IntView> >(home,xv);
      break;
    case AVAL_MED:
      (void) new (home) 
        ViewValAssignment<IntView,int,
                          Branch::ValMed<IntView> >(home,xv);
      break;
    case AVAL_MAX:
      (void) new (home) 
        ViewValAssignment<IntView,int,
                          Branch::ValMax<IntView> >(home,xv);
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  void
  assign(Space* home, const BoolVarArgs& x, AvalSel vals) {
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    switch (vals) {
    case AVAL_MIN:
    case AVAL_MED:
      (void) new (home) 
        ViewValAssignment<BoolView,Branch::NoValue,
                          Branch::ValZeroOne<BoolView> >(home,xv);
      break;
    case AVAL_MAX:
      (void) new (home) 
        ViewValAssignment<BoolView,Branch::NoValue,
                          Branch::ValOneZero<BoolView> >(home,xv);
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

}


// STATISTICS: int-post

