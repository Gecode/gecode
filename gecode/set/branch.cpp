/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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
  branch(Home home, const SetVarArgs& x,
         SetVarBranch vars, SetValBranch vals,
         SetBranchFilter bf,
         SetVarValPrint vvp) {
    using namespace Set;
    if (home.failed()) return;
    vars.expand(home,x);
    ViewArray<SetView> xv(home,x);
    ViewSel<SetView>* vs[1] = {
      Branch::viewsel(home,vars)
    };
    postviewvalbrancher<SetView,1,int,2>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  branch(Home home, const SetVarArgs& x,
         TieBreak<SetVarBranch> vars, SetValBranch vals,
         SetBranchFilter bf,
         SetVarValPrint vvp) {
    using namespace Set;
    if (home.failed()) return;
    vars.a.expand(home,x);
    if ((vars.a.select() == SetVarBranch::SEL_NONE) ||
        (vars.a.select() == SetVarBranch::SEL_RND))
      vars.b = SET_VAR_NONE();
    vars.b.expand(home,x);
    if ((vars.b.select() == SetVarBranch::SEL_NONE) ||
        (vars.b.select() == SetVarBranch::SEL_RND))
      vars.c = SET_VAR_NONE();
    vars.c.expand(home,x);
    if ((vars.c.select() == SetVarBranch::SEL_NONE) ||
        (vars.c.select() == SetVarBranch::SEL_RND))
      vars.d = SET_VAR_NONE();
    vars.d.expand(home,x);
    if (vars.b.select() == SetVarBranch::SEL_NONE) {
      branch(home,x,vars.a,vals,bf,vvp);
    } else {
      ViewArray<SetView> xv(home,x);
      ValSelCommitBase<SetView,int>* vsc = Branch::valselcommit(home,vals);
      if (vars.c.select() == SetVarBranch::SEL_NONE) {
        ViewSel<SetView>* vs[2] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b)
        };
        postviewvalbrancher<SetView,2,int,2>(home,xv,vs,vsc,bf,vvp);
      } else if (vars.d.select() == SetVarBranch::SEL_NONE) {
        ViewSel<SetView>* vs[3] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c)
        };
        postviewvalbrancher<SetView,3,int,2>(home,xv,vs,vsc,bf,vvp);
      } else {
        ViewSel<SetView>* vs[4] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c),Branch::viewsel(home,vars.d)
        };
        postviewvalbrancher<SetView,4,int,2>(home,xv,vs,vsc,bf,vvp);
      }
    }
  }

  void
  branch(Home home, SetVar x, SetValBranch vals, SetVarValPrint vvp) {
    SetVarArgs xv(1); xv[0]=x;
    branch(home, xv, SET_VAR_NONE(), vals, nullptr, vvp);
  }


  void
  assign(Home home, const SetVarArgs& x,
         SetVarBranch vars, SetAssign vals,
         SetBranchFilter bf,
         SetVarValPrint vvp) {
    using namespace Set;
    if (home.failed()) return;
    ViewArray<SetView> xv(home,x);
    ViewSel<SetView>* vs[1] = {
      new (home) ViewSelNone<SetView>(home,vars)
    };
    postviewvalbrancher<SetView,1,int,1>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  assign(Home home, const SetVarArgs& x,
         TieBreak<SetVarBranch> vars, SetAssign vals,
         SetBranchFilter bf,
         SetVarValPrint vvp) {
    using namespace Set;
    if (home.failed()) return;
    vars.a.expand(home,x);
    if ((vars.a.select() == SetVarBranch::SEL_NONE) ||
        (vars.a.select() == SetVarBranch::SEL_RND))
      vars.b = SET_VAR_NONE();
    vars.b.expand(home,x);
    if ((vars.b.select() == SetVarBranch::SEL_NONE) ||
        (vars.b.select() == SetVarBranch::SEL_RND))
      vars.c = SET_VAR_NONE();
    vars.c.expand(home,x);
    if ((vars.c.select() == SetVarBranch::SEL_NONE) ||
        (vars.c.select() == SetVarBranch::SEL_RND))
      vars.d = SET_VAR_NONE();
    vars.d.expand(home,x);
    if (vars.b.select() == SetVarBranch::SEL_NONE) {
      assign(home,x,vars.a,vals,bf,vvp);
    } else {
      ViewArray<SetView> xv(home,x);
      ValSelCommitBase<SetView,int>* vsc = Branch::valselcommit(home,vals);
      if (vars.c.select() == SetVarBranch::SEL_NONE) {
        ViewSel<SetView>* vs[2] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b)
        };
        postviewvalbrancher<SetView,2,int,1>(home,xv,vs,vsc,bf,vvp);
      } else if (vars.d.select() == SetVarBranch::SEL_NONE) {
        ViewSel<SetView>* vs[3] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c)
        };
        postviewvalbrancher<SetView,3,int,1>(home,xv,vs,vsc,bf,vvp);
      } else {
        ViewSel<SetView>* vs[4] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c),Branch::viewsel(home,vars.d)
        };
        postviewvalbrancher<SetView,4,int,1>(home,xv,vs,vsc,bf,vvp);
      }
    }
  }

  void
  assign(Home home, SetVar x, SetAssign vars, SetVarValPrint vvp) {
    SetVarArgs xv(1); xv[0]=x;
    assign(home, xv, SET_VAR_NONE(), vars, nullptr, vvp);
  }

}

// STATISTICS: set-post
