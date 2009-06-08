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

#include <gecode/int/branch.hh>

namespace Gecode {

  void
  assign(Space& home, const IntVarArgs& x, IntAssign vals,
         const ValBranchOptions& o_vals) {
    using namespace Int;
    if (home.failed()) return;
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
    case INT_ASSIGN_RND:
      {
        Branch::AssignValRnd<IntView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<IntView>,Branch::AssignValRnd<IntView> >
          (home,xv,v,a);
      }
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  void
  assign(Space& home, const BoolVarArgs& x, IntAssign vals,
         const ValBranchOptions& o_vals) {
    using namespace Int;
    if (home.failed()) return;
    ViewArray<BoolView> xv(home,x);
    ViewSelNone<BoolView> v(home,VarBranchOptions::def);
    switch (vals) {
    case INT_ASSIGN_MIN:
    case INT_ASSIGN_MED: {
        Branch::AssignValZero<BoolView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<BoolView>,Branch::AssignValZero<BoolView> >
          (home,xv,v,a);
      }
      break;
    case INT_ASSIGN_MAX: {
        Branch::AssignValZero<NegBoolView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<BoolView>,Branch::AssignValZero<NegBoolView> >
          (home,xv,v,a);
      }
      break;
    case INT_ASSIGN_RND: {
        Branch::AssignValRnd<BoolView> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<BoolView>,Branch::AssignValRnd<BoolView> >
          (home,xv,v,a);
      }
      break;
    default:
      throw UnknownBranching("Int::assign");
    }
  }

  void
  branch(Space& home, IntVar x, IntValBranch vals,
         const ValBranchOptions& o_vals) {
    IntVarArgs xv(1); xv[0]=x;
    branch(home, xv, INT_VAR_NONE, vals, VarBranchOptions::def, o_vals);
  }
  
  void
  branch(Space& home, BoolVar x, IntValBranch vals,
         const ValBranchOptions& o_vals) {
    BoolVarArgs xv(1); xv[0]=x;
    branch(home, xv, INT_VAR_NONE, vals, VarBranchOptions::def, o_vals);
  }
  
  void
  assign(Space& home, IntVar x, IntAssign vals,
         const ValBranchOptions& o_vals) {
    IntVarArgs xv(1); xv[0]=x;
    assign(home, xv, vals, o_vals);
  }
  
  void
  assign(Space& home, BoolVar x, IntAssign vals,
         const ValBranchOptions& o_vals) {
    BoolVarArgs xv(1); xv[0]=x;
    assign(home, xv, vals, o_vals);
  }
  
}

// STATISTICS: int-post
