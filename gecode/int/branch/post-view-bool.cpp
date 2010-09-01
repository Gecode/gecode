/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file
 *      gecode/int/branch/post-view-bool.bs
 *    instead.
 *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
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

namespace Gecode { namespace Int { namespace Branch {

  /// Create virtual view selector for tie-breaking
  void
  virtualize(Gecode::Home home, IntVarBranch vars,
             const Gecode::VarBranchOptions& o_vars,
             Gecode::ViewSelVirtualBase<BoolView>*& v) {
    switch (vars) {
    case INT_VAR_RND:
      v = new (home) ViewSelVirtual<ViewSelRnd<BoolView> >(home,o_vars);
      break;
    case INT_VAR_MIN_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_MIN_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_MAX_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_MAX_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_SIZE_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_SIZE_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_DEGREE_MIN:
      v = new (home) ViewSelVirtual<ViewSelDegreeMin<BoolView> >(home,o_vars);
      break;
    case INT_VAR_DEGREE_MAX:
      v = new (home) ViewSelVirtual<ViewSelDegreeMax<BoolView> >(home,o_vars);
      break;
    case INT_VAR_AFC_MIN:
      v = new (home) ViewSelVirtual<ViewSelAfcMin<BoolView> >(home,o_vars);
      break;
    case INT_VAR_AFC_MAX:
      v = new (home) ViewSelVirtual<ViewSelAfcMax<BoolView> >(home,o_vars);
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      v = new (home) ViewSelVirtual<ViewSelDegreeMax<BoolView> >(home,o_vars);
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      v = new (home) ViewSelVirtual<ViewSelDegreeMin<BoolView> >(home,o_vars);
      break;
    case INT_VAR_SIZE_AFC_MIN:
      v = new (home) ViewSelVirtual<ViewSelAfcMax<BoolView> >(home,o_vars);
      break;
    case INT_VAR_SIZE_AFC_MAX:
      v = new (home) ViewSelVirtual<ViewSelAfcMin<BoolView> >(home,o_vars);
      break;
    case INT_VAR_REGRET_MIN_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_REGRET_MIN_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_REGRET_MAX_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    case INT_VAR_REGRET_MAX_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}}}

namespace Gecode {

  void
  branch(Gecode::Home home, const BoolVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         const Gecode::VarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home.failed()) return;
    ViewArray<BoolView> xv(home,x);
    switch (vars) {
    case INT_VAR_NONE:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_RND:
      {
        ViewSelRnd<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_MIN_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_MIN_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_MAX_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_MAX_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_SIZE_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_SIZE_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_DEGREE_MAX:
      {
        ViewSelDegreeMax<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_AFC_MIN:
      {
        ViewSelAfcMin<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_AFC_MAX:
      {
        ViewSelAfcMax<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      {
        ViewSelDegreeMax<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      {
        ViewSelDegreeMin<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_SIZE_AFC_MIN:
      {
        ViewSelAfcMax<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_SIZE_AFC_MAX:
      {
        ViewSelAfcMin<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals,o_vars.bf);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  branch(Gecode::Home home, const BoolVarArgs& x,
         const Gecode::TieBreakVarBranch<IntVarBranch>& vars,
         IntValBranch vals,
         const Gecode::TieBreakVarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home.failed()) return;
    if ((vars.a == INT_VAR_NONE) || (vars.a == INT_VAR_RND) ||
        ((vars.b == INT_VAR_NONE) && (vars.c == INT_VAR_NONE) && (vars.d == INT_VAR_NONE))) {
      branch(home,x,vars.a,vals,o_vars.a,o_vals);
      return;
    }
    ViewArray<BoolView> xv(home,x);
    Gecode::ViewSelVirtualBase<BoolView>* tb[3];
    int n=0;
    if (vars.b != INT_VAR_NONE)
      virtualize(home,vars.b,o_vars.b,tb[n++]);
    if (vars.c != INT_VAR_NONE)
      virtualize(home,vars.c,o_vars.c,tb[n++]);
    if (vars.d != INT_VAR_NONE)
      virtualize(home,vars.d,o_vars.d,tb[n++]);
    assert(n > 0);
    ViewSelTieBreakDynamic<BoolView> vbcd(home,tb,n);
    switch (vars.a) {
    case INT_VAR_MIN_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_MIN_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_MAX_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_MAX_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_SIZE_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_SIZE_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_DEGREE_MAX:
      {
        ViewSelDegreeMax<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMax<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_AFC_MIN:
      {
        ViewSelAfcMin<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelAfcMin<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_AFC_MAX:
      {
        ViewSelAfcMax<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelAfcMax<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      {
        ViewSelDegreeMax<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMax<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      {
        ViewSelDegreeMin<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_SIZE_AFC_MIN:
      {
        ViewSelAfcMax<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelAfcMax<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_SIZE_AFC_MAX:
      {
        ViewSelAfcMin<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelAfcMin<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals,o_vars.a.bf);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}


// STATISTICS: int-branch

