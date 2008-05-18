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
         const TieBreakVarBranch<IntVarBranch>& vars, IntValBranch vals,
         const TieBreakVarBranchOptions& o_vars,
         const ValBranchOptions& o_vals) {
    ViewArray<Int::IntView> xv(home,x);
    Int::Branch::post(home,xv,vars,vals,o_vars,o_vals);
  }

  void
  branch(Space* home, const BoolVarArgs& x,
         const TieBreakVarBranch<IntVarBranch>& vars, IntValBranch vals,
         const TieBreakVarBranchOptions& o_vars,
         const ValBranchOptions& o_vals) {
    ViewArray<Int::BoolView> xv(home,x);
    Int::Branch::post(home,xv,vars,vals,o_vars,o_vals);
  }


  void
  assign(Space* home, const IntVarArgs& x, IntAssign vals,
         const ValBranchOptions& o_vals) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    ViewSelNone<IntView> v(home,VarBranchOptions::def);
    switch (vals) {
    case INT_ASSIGN_MIN:
      {
        Branch::AssignValMin<IntView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<IntView>,Branch::AssignValMin<IntView> >
          (home,xv,v,a);
      }
      break;
    case INT_ASSIGN_MED:
      {
        Branch::AssignValMed<IntView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<IntView>,Branch::AssignValMed<IntView> >
          (home,xv,v,a);
      }
      break;
    case INT_ASSIGN_MAX:
      {
        Branch::AssignValMin<MinusView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<IntView>,Branch::AssignValMin<MinusView> >
          (home,xv,v,a);
      }
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  void
  assign(Space* home, const BoolVarArgs& x, IntAssign vals,
         const ValBranchOptions& o_vals) {
    using namespace Int;
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    ViewSelNone<BoolView> v(home,VarBranchOptions::def);
    switch (vals) {
    case INT_ASSIGN_MIN:
    case INT_ASSIGN_MED: {
        Branch::AssignValZeroOne<BoolView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<BoolView>,Branch::AssignValZeroOne<BoolView> >
          (home,xv,v,a);
      }
      break;
    case INT_ASSIGN_MAX: {
        Branch::AssignValZeroOne<NegBoolView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<BoolView>,Branch::AssignValZeroOne<NegBoolView> >
          (home,xv,v,a);
      }
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

}

// STATISTICS: int-post
