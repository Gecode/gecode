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

  namespace Int { namespace Branch {

    template<class View, class Val, template <class> class SelView>
    class Creator {
    public:
      Reflection::Registrar<ViewValBranching<View,Val,SelView<View>,
        Branch::ValMin<View> > > r1;
      Reflection::Registrar<ViewValBranching<View,Val,SelView<View>,
        ValMed<View> > > r2;
      Reflection::Registrar<ViewValBranching<View,Val,SelView<View>,
        ValMax<View> > > r3;
      Reflection::Registrar<ViewValBranching<View,Val,SelView<View>,
        ValSplitMin<View> > > r4;
      Reflection::Registrar<ViewValBranching<View,Val,SelView<View>,
        ValSplitMax<View> > > r5;
    };

    Creator<IntView,int,BySizeMin> c1;
    Creator<IntView,int,BySizeMax> c2;
    Creator<IntView,int,ByMinMin> c3;
    Creator<IntView,int,ByMinMax> c4;
    Creator<IntView,int,ByMaxMin> c5;
    Creator<IntView,int,ByMaxMax> c6;
    Creator<IntView,int,ByNone> c7;
    Creator<IntView,int,ByRegretMinMin> c8;
    Creator<IntView,int,ByRegretMinMax> c9;
    Creator<IntView,int,ByRegretMaxMin> c10;
    Creator<IntView,int,ByRegretMaxMax> c11;
    Creator<IntView,int,ByDegreeMin> c12;
    Creator<IntView,int,ByDegreeMax> c13;

    Reflection::Registrar<ViewValBranching<BoolView,NoValue,
      ByDegreeMaxNoTies<BoolView>,ValOneZero<BoolView> > > r1;
    Reflection::Registrar<ViewValBranching<BoolView,NoValue,
      ByDegreeMaxNoTies<BoolView>,ValZeroOne<BoolView> > > r2;
    Reflection::Registrar<ViewValBranching<BoolView,NoValue,
      ByDegreeMinNoTies<BoolView>,ValOneZero<BoolView> > > r3;
    Reflection::Registrar<ViewValBranching<BoolView,NoValue,
      ByDegreeMinNoTies<BoolView>,ValZeroOne<BoolView> > > r4;
    Reflection::Registrar<ViewValBranching<BoolView,NoValue,
      ByNone<BoolView>,ValOneZero<BoolView> > > r5;
    Reflection::Registrar<ViewValBranching<BoolView,NoValue,
      ByNone<BoolView>,ValZeroOne<BoolView> > > r6;

    Reflection::Registrar<ViewValAssignment<IntView,int,
      ValMax<IntView> > > r7;
    Reflection::Registrar<ViewValAssignment<IntView,int,
      ValMed<IntView> > > r8;
    Reflection::Registrar<ViewValAssignment<IntView,int,
      ValMin<IntView> > > r9;
    Reflection::Registrar<ViewValAssignment<BoolView,NoValue,
      ValOneZero<BoolView> > > r10;
    Reflection::Registrar<ViewValAssignment<BoolView,NoValue,
      ValZeroOne<BoolView> > > r11;

  }}

}


// STATISTICS: int-post

