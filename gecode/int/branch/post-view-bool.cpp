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
  virtualize(Gecode::Home home, const IntVarBranch& vars,
             Gecode::ViewSelVirtualBase<BoolView>*& v) {
    switch (vars.select()) {
    case IntVarBranch::SEL_RND:
      v = new (home) ViewSelVirtual<ViewSelRnd<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_MERIT_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritFunction<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_MERIT_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritFunction<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_MIN_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_MIN_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_MAX_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_MAX_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_DEGREE_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritDegree<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_DEGREE_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritDegree<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_AFC_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritAfc<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_AFC_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritAfc<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_ACTIVITY_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritActivity<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_ACTIVITY_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritActivity<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_DEGREE_MIN:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritDegree<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_DEGREE_MAX:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritDegree<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_AFC_MIN:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritAfc<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_AFC_MAX:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritAfc<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_ACTIVITY_MIN:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritActivity<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_SIZE_ACTIVITY_MAX:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritActivity<BoolView> > >(home,vars);
      break;
    case IntVarBranch::SEL_REGRET_MIN_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_REGRET_MIN_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_REGRET_MAX_MIN:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
      break;
    case IntVarBranch::SEL_REGRET_MAX_MAX:
      v = new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,vars);
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
         BoolBranchFilter bf) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home.failed()) return;
    if (vars.activity().initialized() &&
        (vars.activity().size() != x.size()))
      throw ActivityWrongArity("branch");
    ViewArray<BoolView> xv(home,x);
    switch (vars.select()) {
    case IntVarBranch::SEL_NONE:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_RND:
      {
        ViewSelRnd<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MERIT_MIN:
      {
        ViewSelMin<MeritFunction<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MERIT_MAX:
      {
        ViewSelMax<MeritFunction<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MIN_MIN:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MIN_MAX:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MAX_MIN:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MAX_MAX:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_MIN:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_MAX:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_DEGREE_MIN:
      {
        ViewSelMin<MeritDegree<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_DEGREE_MAX:
      {
        ViewSelMax<MeritDegree<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_AFC_MIN:
      {
        ViewSelMin<MeritAfc<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_AFC_MAX:
      {
        ViewSelMax<MeritAfc<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_ACTIVITY_MIN:
      {
        ViewSelMin<MeritActivity<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_ACTIVITY_MAX:
      {
        ViewSelMax<MeritActivity<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_DEGREE_MIN:
      {
        ViewSelMax<MeritDegree<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_DEGREE_MAX:
      {
        ViewSelMin<MeritDegree<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_AFC_MIN:
      {
        ViewSelMax<MeritAfc<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_AFC_MAX:
      {
        ViewSelMin<MeritAfc<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_ACTIVITY_MIN:
      {
        ViewSelMax<MeritActivity<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_ACTIVITY_MAX:
      {
        ViewSelMin<MeritActivity<BoolView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MIN_MIN:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MIN_MAX:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MAX_MIN:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MAX_MAX:
      {
        ViewSelNone<BoolView> v(home,vars);
        post(home,xv,v,vals,bf);
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
         BoolBranchFilter bf) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home.failed()) return;
    if ((vars.a.select() == IntVarBranch::SEL_NONE) || (vars.a.select() == IntVarBranch::SEL_RND) ||
        ((vars.b.select() == IntVarBranch::SEL_NONE) &&
         (vars.c.select() == IntVarBranch::SEL_NONE) &&
         (vars.d.select() == IntVarBranch::SEL_NONE))) {
      branch(home,x,vars.a,vals,bf);
      return;
    }
    if (vars.a.activity().initialized() && (vars.a.activity().size() != x.size()))
      throw ActivityWrongArity("branch (option a)");
    if (vars.b.activity().initialized() && (vars.b.activity().size() != x.size()))
      throw ActivityWrongArity("branch (option b)");
    if (vars.c.activity().initialized() && (vars.c.activity().size() != x.size()))
      throw ActivityWrongArity("branch (option c)");
    if (vars.d.activity().initialized() && (vars.d.activity().size() != x.size()))
      throw ActivityWrongArity("branch (option d)");
    ViewArray<BoolView> xv(home,x);
    Gecode::ViewSelVirtualBase<BoolView>* tb[3];
    int n=0;
    if (vars.b.select() != IntVarBranch::SEL_NONE)
      virtualize(home,vars.b,tb[n++]);
    if (vars.c.select() != IntVarBranch::SEL_NONE)
      virtualize(home,vars.c,tb[n++]);
    if (vars.d.select() != IntVarBranch::SEL_NONE)
      virtualize(home,vars.d,tb[n++]);
    assert(n > 0);
    ViewSelTieBreakDynamic<BoolView> vbcd(home,tb,n);
    switch (vars.a.select()) {
    case IntVarBranch::SEL_MERIT_MIN:
      {
        ViewSelMin<MeritFunction<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritFunction<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MERIT_MAX:
      {
        ViewSelMax<MeritFunction<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritFunction<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MIN_MIN:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MIN_MAX:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MAX_MIN:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_MAX_MAX:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_MIN:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_MAX:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_DEGREE_MIN:
      {
        ViewSelMin<MeritDegree<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritDegree<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_DEGREE_MAX:
      {
        ViewSelMax<MeritDegree<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritDegree<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_AFC_MIN:
      {
        ViewSelMin<MeritAfc<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritAfc<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_AFC_MAX:
      {
        ViewSelMax<MeritAfc<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritAfc<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_ACTIVITY_MIN:
      {
        ViewSelMin<MeritActivity<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritActivity<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_ACTIVITY_MAX:
      {
        ViewSelMax<MeritActivity<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritActivity<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_DEGREE_MIN:
      {
        ViewSelMax<MeritDegree<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritDegree<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_DEGREE_MAX:
      {
        ViewSelMin<MeritDegree<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritDegree<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_AFC_MIN:
      {
        ViewSelMax<MeritAfc<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritAfc<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_AFC_MAX:
      {
        ViewSelMin<MeritAfc<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritAfc<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_ACTIVITY_MIN:
      {
        ViewSelMax<MeritActivity<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritActivity<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_SIZE_ACTIVITY_MAX:
      {
        ViewSelMin<MeritActivity<BoolView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritActivity<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MIN_MIN:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MIN_MAX:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MAX_MIN:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case IntVarBranch::SEL_REGRET_MAX_MAX:
      {
        ViewSelNone<BoolView> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}


// STATISTICS: int-branch

