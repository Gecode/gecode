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

#include <gecode/int/branch.hh>

namespace Gecode {

  void
  branch(Home home, const IntVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         IntBranchFilter bf,
         IntVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    vars.expand(home,x);
    ViewArray<IntView> xv(home,x);
    ViewSel<IntView>* vs[1] = {
      Branch::viewsel(home,vars)
    };
    switch (vals.select()) {
    case IntValBranch::SEL_VALUES_MIN:
      Branch::postviewvaluesbrancher<1,true>(home,xv,vs,bf,vvp);
      break;
    case IntValBranch::SEL_VALUES_MAX:
      Branch::postviewvaluesbrancher<1,false>(home,xv,vs,bf,vvp);
      break;
    default:
      postviewvalbrancher<IntView,1,int,2>
        (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
      break;
    }
  }

  void
  branch(Home home, const IntVarArgs& x,
         TieBreak<IntVarBranch> vars, IntValBranch vals,
         IntBranchFilter bf,
         IntVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    vars.a.expand(home,x);
    if ((vars.a.select() == IntVarBranch::SEL_NONE) ||
        (vars.a.select() == IntVarBranch::SEL_RND))
      vars.b = INT_VAR_NONE();
    vars.b.expand(home,x);
    if ((vars.b.select() == IntVarBranch::SEL_NONE) ||
        (vars.b.select() == IntVarBranch::SEL_RND))
      vars.c = INT_VAR_NONE();
    vars.c.expand(home,x);
    if ((vars.c.select() == IntVarBranch::SEL_NONE) ||
        (vars.c.select() == IntVarBranch::SEL_RND))
      vars.d = INT_VAR_NONE();
    vars.d.expand(home,x);
    if (vars.b.select() == IntVarBranch::SEL_NONE) {
      branch(home,x,vars.a,vals,bf,vvp);
    } else {
      ViewArray<IntView> xv(home,x);
      if (vars.c.select() == IntVarBranch::SEL_NONE) {
        ViewSel<IntView>* vs[2] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b)
        };
        switch (vals.select()) {
        case IntValBranch::SEL_VALUES_MIN:
          Branch::postviewvaluesbrancher<2,true>(home,xv,vs,bf,vvp);
          break;
        case IntValBranch::SEL_VALUES_MAX:
          Branch::postviewvaluesbrancher<2,false>(home,xv,vs,bf,vvp);
          break;
        default:
          postviewvalbrancher<IntView,2,int,2>
            (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
        }
      } else if (vars.d.select() == IntVarBranch::SEL_NONE) {
        ViewSel<IntView>* vs[3] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c)
        };
        switch (vals.select()) {
        case IntValBranch::SEL_VALUES_MIN:
          Branch::postviewvaluesbrancher<3,true>(home,xv,vs,bf,vvp);
          break;
        case IntValBranch::SEL_VALUES_MAX:
          Branch::postviewvaluesbrancher<3,false>(home,xv,vs,bf,vvp);
          break;
        default:
          postviewvalbrancher<IntView,3,int,2>
            (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
        }
      } else {
        ViewSel<IntView>* vs[4] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c),Branch::viewsel(home,vars.d)
        };
        switch (vals.select()) {
        case IntValBranch::SEL_VALUES_MIN:
          Branch::postviewvaluesbrancher<4,true>(home,xv,vs,bf,vvp);
          break;
        case IntValBranch::SEL_VALUES_MAX:
          Branch::postviewvaluesbrancher<4,false>(home,xv,vs,bf,vvp);
          break;
        default:
          postviewvalbrancher<IntView,4,int,2>
            (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
        }
      }
    }
  }

  void
  branch(Home home, IntVar x, IntValBranch vals, IntVarValPrint vvp) {
    IntVarArgs xv(1); xv[0]=x;
    branch(home, xv, INT_VAR_NONE(), vals, nullptr, vvp);
  }


  void
  assign(Home home, const IntVarArgs& x,
         IntVarBranch vars, IntAssign vals,
         IntBranchFilter bf,
         IntVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    ViewArray<IntView> xv(home,x);
    ViewSel<IntView>* vs[1] = {
      new (home) ViewSelNone<IntView>(home,vars)
    };
    postviewvalbrancher<IntView,1,int,1>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  branch(Home home, const IntVarArgs& x,
         TieBreak<IntVarBranch> vars, IntAssign vals,
         IntBranchFilter bf,
         IntVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    vars.a.expand(home,x);
    if ((vars.a.select() == IntVarBranch::SEL_NONE) ||
        (vars.a.select() == IntVarBranch::SEL_RND))
      vars.b = INT_VAR_NONE();
    vars.b.expand(home,x);
    if ((vars.b.select() == IntVarBranch::SEL_NONE) ||
        (vars.b.select() == IntVarBranch::SEL_RND))
      vars.c = INT_VAR_NONE();
    vars.c.expand(home,x);
    if ((vars.c.select() == IntVarBranch::SEL_NONE) ||
        (vars.c.select() == IntVarBranch::SEL_RND))
      vars.d = INT_VAR_NONE();
    vars.d.expand(home,x);
    if (vars.b.select() == IntVarBranch::SEL_NONE) {
      assign(home,x,vars.a,vals,bf,vvp);
    } else {
      ViewArray<IntView> xv(home,x);
      if (vars.c.select() == IntVarBranch::SEL_NONE) {
        ViewSel<IntView>* vs[2] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b)
        };
        postviewvalbrancher<IntView,2,int,1>
          (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
    } else if (vars.d.select() == IntVarBranch::SEL_NONE) {
        ViewSel<IntView>* vs[3] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c)
        };
        postviewvalbrancher<IntView,3,int,1>
          (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
      } else {
        ViewSel<IntView>* vs[4] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c),Branch::viewsel(home,vars.d)
        };
        postviewvalbrancher<IntView,4,int,1>
          (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
      }
    }
  }

  void
  assign(Home home, IntVar x, IntAssign ia, IntVarValPrint vvp) {
    IntVarArgs xv(1); xv[0]=x;
    assign(home, xv, INT_VAR_NONE(), ia, nullptr, vvp);
  }


  void
  branch(Home home, const BoolVarArgs& x,
         BoolVarBranch vars, BoolValBranch vals,
         BoolBranchFilter bf,
         BoolVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    vars.expand(home,x);
    ViewArray<BoolView> xv(home,x);
    ViewSel<BoolView>* vs[1] = {
      Branch::viewsel(home,vars)
    };
    postviewvalbrancher<BoolView,1,int,2>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  branch(Home home, const BoolVarArgs& x,
         TieBreak<BoolVarBranch> vars, BoolValBranch vals,
         BoolBranchFilter bf,
         BoolVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    vars.a.expand(home,x);
    if ((vars.a.select() == BoolVarBranch::SEL_NONE) ||
        (vars.a.select() == BoolVarBranch::SEL_RND))
      vars.b = BOOL_VAR_NONE();
    vars.b.expand(home,x);
    if ((vars.b.select() == BoolVarBranch::SEL_NONE) ||
        (vars.b.select() == BoolVarBranch::SEL_RND))
      vars.c = BOOL_VAR_NONE();
    vars.c.expand(home,x);
    if ((vars.c.select() == BoolVarBranch::SEL_NONE) ||
        (vars.c.select() == BoolVarBranch::SEL_RND))
      vars.d = BOOL_VAR_NONE();
    vars.d.expand(home,x);
    if (vars.b.select() == BoolVarBranch::SEL_NONE) {
      branch(home,x,vars.a,vals,bf,vvp);
    } else {
      ViewArray<BoolView> xv(home,x);
      ValSelCommitBase<BoolView,int>*
        vsc = Branch::valselcommit(home,vals);
      if (vars.c.select() == BoolVarBranch::SEL_NONE) {
        ViewSel<BoolView>* vs[2] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b)
        };
        postviewvalbrancher<BoolView,2,int,2>(home,xv,vs,vsc,bf,vvp);
      } else if (vars.d.select() == BoolVarBranch::SEL_NONE) {
        ViewSel<BoolView>* vs[3] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c)
        };
        postviewvalbrancher<BoolView,3,int,2>(home,xv,vs,vsc,bf,vvp);
      } else {
        ViewSel<BoolView>* vs[4] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c),Branch::viewsel(home,vars.d)
        };
        postviewvalbrancher<BoolView,4,int,2>(home,xv,vs,vsc,bf,vvp);
      }
    }
  }

  void
  branch(Home home, BoolVar x, BoolValBranch vals, BoolVarValPrint vvp) {
    BoolVarArgs xv(1); xv[0]=x;
    branch(home, xv, BOOL_VAR_NONE(), vals, nullptr, vvp);
  }

  void
  assign(Home home, const BoolVarArgs& x,
         BoolVarBranch vars, BoolAssign vals,
         BoolBranchFilter bf,
         BoolVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    ViewArray<BoolView> xv(home,x);
    ViewSel<BoolView>* vs[1] = {
      new (home) ViewSelNone<BoolView>(home,vars)
    };
    postviewvalbrancher<BoolView,1,int,1>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  assign(Home home, const BoolVarArgs& x,
         TieBreak<BoolVarBranch> vars, BoolAssign vals,
         BoolBranchFilter bf,
         BoolVarValPrint vvp) {
    using namespace Int;
    if (home.failed()) return;
    vars.a.expand(home,x);
    if ((vars.a.select() == BoolVarBranch::SEL_NONE) ||
        (vars.a.select() == BoolVarBranch::SEL_RND))
      vars.b = BOOL_VAR_NONE();
    vars.b.expand(home,x);
    if ((vars.b.select() == BoolVarBranch::SEL_NONE) ||
        (vars.b.select() == BoolVarBranch::SEL_RND))
      vars.c = BOOL_VAR_NONE();
    vars.c.expand(home,x);
    if ((vars.c.select() == BoolVarBranch::SEL_NONE) ||
        (vars.c.select() == BoolVarBranch::SEL_RND))
      vars.d = BOOL_VAR_NONE();
    vars.d.expand(home,x);
    if (vars.b.select() == BoolVarBranch::SEL_NONE) {
      assign(home,x,vars.a,vals,bf,vvp);
    } else {
      ViewArray<BoolView> xv(home,x);
      ValSelCommitBase<BoolView,int>*
        vsc = Branch::valselcommit(home,vals);
      if (vars.c.select() == BoolVarBranch::SEL_NONE) {
        ViewSel<BoolView>* vs[2] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b)
        };
        postviewvalbrancher<BoolView,2,int,1>(home,xv,vs,vsc,bf,vvp);
      } else if (vars.d.select() == BoolVarBranch::SEL_NONE) {
        ViewSel<BoolView>* vs[3] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c)
        };
        postviewvalbrancher<BoolView,3,int,1>(home,xv,vs,vsc,bf,vvp);
      } else {
        ViewSel<BoolView>* vs[4] = {
          Branch::viewsel(home,vars.a),Branch::viewsel(home,vars.b),
          Branch::viewsel(home,vars.c),Branch::viewsel(home,vars.d)
        };
        postviewvalbrancher<BoolView,4,int,1>(home,xv,vs,vsc,bf,vvp);
      }
    }
  }

  void
  assign(Home home, BoolVar x, BoolAssign ba, BoolVarValPrint vvp) {
    BoolVarArgs xv(1); xv[0]=x;
    assign(home, xv, BOOL_VAR_NONE(), ba, nullptr, vvp);
  }

#ifdef GECODE_HAS_CBS

  void
  cbsbranch(Home home, const IntVarArgs& x) {
    using namespace Int;
    if (home.failed()) return;
    ViewArray<IntView> y(home,x);
    Branch::CBSBrancher<IntView>::post(home,y);
  }

  void
  cbsbranch(Home home, const BoolVarArgs& x) {
    using namespace Int;
    if (home.failed()) return;
    ViewArray<BoolView> y(home,x);
    Branch::CBSBrancher<BoolView>::post(home,y);
  }

#endif

}

// STATISTICS: int-post
