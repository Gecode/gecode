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

  void
  branch(Space* home, const IntVarArgs& x,
         const TieBreakIntVarBranch& vars, IntValBranch vals) {
    Int::Branch::post(home,x,vars.a,vals);
  }

  void
  branch(Space* home, const BoolVarArgs& x,
         const TieBreakIntVarBranch& vars, IntValBranch vals) {
    Int::Branch::post(home,x,vars.a,vals);
  }


  void
  assign(Space* home, const IntVarArgs& x, IntAssign vals) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (vals) {
    case INT_ASSIGN_MIN:
      (void) new (home) ViewValBranching
        <ViewSelByNone<IntView>,Branch::AssignValMin<IntView> >(home,xv);
      break;
    case INT_ASSIGN_MED:
      (void) new (home) ViewValBranching
        <ViewSelByNone<IntView>,Branch::AssignValMed<IntView> >(home,xv);
      break;
    case INT_ASSIGN_MAX:
      (void) new (home) ViewValBranching
        <ViewSelByNone<IntView>,Branch::AssignValMin<MinusView> >(home,xv);
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  void
  assign(Space* home, const BoolVarArgs& x, IntAssign vals) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    switch (vals) {
    case INT_ASSIGN_MIN:
    case INT_ASSIGN_MED:
      (void) new (home) ViewValBranching
        <ViewSelByNone<BoolView>,Branch::AssignValZeroOne<BoolView> >
        (home,xv);
      break;
    case INT_ASSIGN_MAX:
      (void) new (home) ViewValBranching
        <ViewSelByNone<BoolView>,Branch::AssignValZeroOne<NegBoolView> >
        (home,xv);
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  /*

  namespace Int { namespace Branch {

    template<class View, template <class> class SelView>
    /// Register reflection for branchings
    class Creator {
    public:
      Reflection::ActorRegistrar<ViewValBranching<SelView<View>,
        Branch::ValMin<View> > > r1;
      Reflection::ActorRegistrar<ViewValBranching<SelView<View>,
        Branch::ValMed<View> > > r2;
      Reflection::ActorRegistrar<ViewValBranching<SelView<View>,
        Branch::ValMax<View> > > r3;
      Reflection::ActorRegistrar<ViewValBranching<SelView<View>,
        Branch::ValSplitMin<View> > > r4;
      Reflection::ActorRegistrar<ViewValBranching<SelView<View>,
        Branch::ValSplitMax<View> > > r5;
    };

    Creator<IntView,BySizeMin> c1;
    Creator<IntView,BySizeMax> c2;
    Creator<IntView,ByMinMin> c3;
    Creator<IntView,ByMinMax> c4;
    Creator<IntView,ByMaxMin> c5;
    Creator<IntView,ByMaxMax> c6;
    Creator<IntView,ByNone> c7;
    Creator<IntView,ByRegretMinMin> c8;
    Creator<IntView,ByRegretMinMax> c9;
    Creator<IntView,ByRegretMaxMin> c10;
    Creator<IntView,ByRegretMaxMax> c11;
    Creator<IntView,ByDegreeMin> c12;
    Creator<IntView,ByDegreeMax> c13;

    Reflection::ActorRegistrar<ViewValBranching<
      ByDegreeMaxNoTies<BoolView>,ValOneZero<BoolView> > > r1;
    Reflection::ActorRegistrar<ViewValBranching<
      ByDegreeMaxNoTies<BoolView>,ValZeroOne<BoolView> > > r2;
    Reflection::ActorRegistrar<ViewValBranching<
      ByDegreeMinNoTies<BoolView>,ValOneZero<BoolView> > > r3;
    Reflection::ActorRegistrar<ViewValBranching<
      ByDegreeMinNoTies<BoolView>,ValZeroOne<BoolView> > > r4;
    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<BoolView>,ValOneZero<BoolView> > > r5;
    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<BoolView>,ValZeroOne<BoolView> > > r6;

    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<IntView>,AssignValMax<IntView> > > r7;
    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<IntView>,AssignValMed<IntView> > > r8;
    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<IntView>,AssignValMin<IntView> > > r9;
    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<BoolView>,AssignValOneZero<BoolView> > > r10;
    Reflection::ActorRegistrar<ViewValBranching<
      ByNone<BoolView>,AssignValZeroOne<BoolView> > > r11;

  }}

  */

}


// STATISTICS: int-post

