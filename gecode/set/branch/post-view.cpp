/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file
 *      gecode/set/branch/post-view.bs
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

#include <gecode/set/branch.hh>

namespace Gecode { namespace Set { namespace Branch {

  /// Create virtual view selector for tie-breaking
  void
  virtualize(Gecode::Home home, const SetVarBranch& vars,
             Gecode::ViewSelVirtualBase<SetView>*& v) {
    switch (vars.select()) {
    case SetVarBranch::SEL_RND:
      v = new (home) ViewSelVirtual<ViewSelRnd<SetView> >(home,vars);
      break;
    case SetVarBranch::SEL_MERIT_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritFunction<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_MERIT_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritFunction<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_MIN_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritMin> >(home,vars);
      break;
    case SetVarBranch::SEL_MIN_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritMin> >(home,vars);
      break;
    case SetVarBranch::SEL_MAX_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritMax> >(home,vars);
      break;
    case SetVarBranch::SEL_MAX_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritMax> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritSize> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritSize> >(home,vars);
      break;
    case SetVarBranch::SEL_DEGREE_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritDegree<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_DEGREE_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritDegree<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_AFC_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritAfc<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_AFC_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritAfc<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_ACTIVITY_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritActivity<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_ACTIVITY_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritActivity<SetView> > >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_DEGREE_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritSizeDegree> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_DEGREE_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritSizeDegree> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_AFC_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritSizeAfc> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_AFC_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritSizeAfc> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_ACTIVITY_MIN:
      v = new (home) ViewSelVirtual<ViewSelMin<MeritSizeActivity> >(home,vars);
      break;
    case SetVarBranch::SEL_SIZE_ACTIVITY_MAX:
      v = new (home) ViewSelVirtual<ViewSelMax<MeritSizeActivity> >(home,vars);
      break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

}}}

namespace Gecode {

  void
  branch(Gecode::Home home, const SetVarArgs& x,
         SetVarBranch vars, SetValBranch vals,
         SetBranchFilter bf) {
    using namespace Gecode;
    using namespace Gecode::Set;
    using namespace Gecode::Set::Branch;


    if (home.failed()) return;
    if (vars.activity().initialized() &&
        (vars.activity().size() != x.size()))
      throw ActivityWrongArity("branch");
    ViewArray<SetView> xv(home,x);
    switch (vars.select()) {
    case SetVarBranch::SEL_NONE:
      {
        ViewSelNone<SetView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_RND:
      {
        ViewSelRnd<SetView> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MERIT_MIN:
      {
        ViewSelMin<MeritFunction<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MERIT_MAX:
      {
        ViewSelMax<MeritFunction<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MIN_MIN:
      {
        ViewSelMin<MeritMin> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MIN_MAX:
      {
        ViewSelMax<MeritMin> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MAX_MIN:
      {
        ViewSelMin<MeritMax> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MAX_MAX:
      {
        ViewSelMax<MeritMax> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_MIN:
      {
        ViewSelMin<MeritSize> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_MAX:
      {
        ViewSelMax<MeritSize> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_DEGREE_MIN:
      {
        ViewSelMin<MeritDegree<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_DEGREE_MAX:
      {
        ViewSelMax<MeritDegree<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_AFC_MIN:
      {
        ViewSelMin<MeritAfc<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_AFC_MAX:
      {
        ViewSelMax<MeritAfc<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_ACTIVITY_MIN:
      {
        ViewSelMin<MeritActivity<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_ACTIVITY_MAX:
      {
        ViewSelMax<MeritActivity<SetView> > v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_DEGREE_MIN:
      {
        ViewSelMin<MeritSizeDegree> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_DEGREE_MAX:
      {
        ViewSelMax<MeritSizeDegree> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_AFC_MIN:
      {
        ViewSelMin<MeritSizeAfc> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_AFC_MAX:
      {
        ViewSelMax<MeritSizeAfc> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_ACTIVITY_MIN:
      {
        ViewSelMin<MeritSizeActivity> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_ACTIVITY_MAX:
      {
        ViewSelMax<MeritSizeActivity> v(home,vars);
        post(home,xv,v,vals,bf);
      }
      break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

  void
  branch(Gecode::Home home, const SetVarArgs& x,
         const Gecode::TieBreakVarBranch<SetVarBranch>& vars,
         SetValBranch vals,
         SetBranchFilter bf) {
    using namespace Gecode;
    using namespace Gecode::Set;
    using namespace Gecode::Set::Branch;


    if (home.failed()) return;
    if ((vars.a.select() == SetVarBranch::SEL_NONE) || (vars.a.select() == SetVarBranch::SEL_RND) ||
        ((vars.b.select() == SetVarBranch::SEL_NONE) &&
         (vars.c.select() == SetVarBranch::SEL_NONE) &&
         (vars.d.select() == SetVarBranch::SEL_NONE))) {
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
    ViewArray<SetView> xv(home,x);
    Gecode::ViewSelVirtualBase<SetView>* tb[3];
    int n=0;
    if (vars.b.select() != SetVarBranch::SEL_NONE)
      virtualize(home,vars.b,tb[n++]);
    if (vars.c.select() != SetVarBranch::SEL_NONE)
      virtualize(home,vars.c,tb[n++]);
    if (vars.d.select() != SetVarBranch::SEL_NONE)
      virtualize(home,vars.d,tb[n++]);
    assert(n > 0);
    ViewSelTieBreakDynamic<SetView> vbcd(home,tb,n);
    switch (vars.a.select()) {
    case SetVarBranch::SEL_MERIT_MIN:
      {
        ViewSelMin<MeritFunction<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritFunction<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MERIT_MAX:
      {
        ViewSelMax<MeritFunction<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritFunction<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MIN_MIN:
      {
        ViewSelMin<MeritMin> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritMin>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MIN_MAX:
      {
        ViewSelMax<MeritMin> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritMin>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MAX_MIN:
      {
        ViewSelMin<MeritMax> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritMax>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_MAX_MAX:
      {
        ViewSelMax<MeritMax> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritMax>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_MIN:
      {
        ViewSelMin<MeritSize> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritSize>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_MAX:
      {
        ViewSelMax<MeritSize> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritSize>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_DEGREE_MIN:
      {
        ViewSelMin<MeritDegree<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritDegree<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_DEGREE_MAX:
      {
        ViewSelMax<MeritDegree<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritDegree<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_AFC_MIN:
      {
        ViewSelMin<MeritAfc<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritAfc<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_AFC_MAX:
      {
        ViewSelMax<MeritAfc<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritAfc<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_ACTIVITY_MIN:
      {
        ViewSelMin<MeritActivity<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritActivity<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_ACTIVITY_MAX:
      {
        ViewSelMax<MeritActivity<SetView> > va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritActivity<SetView> >,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_DEGREE_MIN:
      {
        ViewSelMin<MeritSizeDegree> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritSizeDegree>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_DEGREE_MAX:
      {
        ViewSelMax<MeritSizeDegree> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritSizeDegree>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_AFC_MIN:
      {
        ViewSelMin<MeritSizeAfc> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritSizeAfc>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_AFC_MAX:
      {
        ViewSelMax<MeritSizeAfc> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritSizeAfc>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_ACTIVITY_MIN:
      {
        ViewSelMin<MeritSizeActivity> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMin<MeritSizeActivity>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    case SetVarBranch::SEL_SIZE_ACTIVITY_MAX:
      {
        ViewSelMax<MeritSizeActivity> va(home,vars.a);
        ViewSelTieBreakStatic<ViewSelMax<MeritSizeActivity>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,bf);
      }
      break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

}


// STATISTICS: set-branch

