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

#include <gecode/set/branch.hh>

namespace Gecode {

  void
  assign(Home home, const SetVarArgs& x, SetAssign vals,
         SetBranchFilter sbf) {
    using namespace Set;
    if (home.failed()) return;
    ViewArray<SetView> xv(home,x);
    ViewSelNone<SetView> v(home,SetVarBranch());
    switch (vals.select()) {
    case SetAssign::SEL_MIN_INC:
      {
        Branch::AssignValMin<true> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValMin<true> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_MIN_EXC:
      {
        Branch::AssignValMin<false> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValMin<false> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_MED_INC:
      {
        Branch::AssignValMed<true> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValMed<true> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_MED_EXC:
      {
        Branch::AssignValMed<false> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValMed<false> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_MAX_INC:
      {
        Branch::AssignValMax<true> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValMax<true> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_MAX_EXC:
      {
        Branch::AssignValMax<false> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValMax<false> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_RND_INC:
      {
        Branch::AssignValRnd<true> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,
          Branch::AssignValRnd<true> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_RND_EXC:
      {
        Branch::AssignValRnd<false> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,Branch::AssignValRnd<false> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    case SetAssign::SEL_VAL_COMMIT:
      {
        ValSelValCommit<SetView,1> a(home,vals);
        ViewValBrancher<ViewSelNone<SetView>,ValSelValCommit<SetView,1> >
          ::post(home,xv,v,a,sbf);
      }
      break;
    default:
      throw UnknownBranching("Set::assign");
    }
  }

  void
  branch(Home home, SetVar x, SetValBranch vals) {
    SetVarArgs xv(1); xv[0]=x;
    branch(home, xv, SET_VAR_NONE(), vals);
  }
  
  void
  assign(Home home, SetVar x, SetAssign vals) {
    SetVarArgs xv(1); xv[0]=x;
    assign(home, xv, vals);
  }

}

// STATISTICS: set-branch

