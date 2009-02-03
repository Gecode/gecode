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
  virtualize(Gecode::Space& home, SetVarBranch vars,
             const Gecode::VarBranchOptions& o_vars,
             Gecode::ViewSelVirtualBase<SetView>*& v) {
    switch (vars) {
     case SET_VAR_RND:
       v = new (home) ViewSelVirtual<ViewSelRnd<SetView> >(home,o_vars);
       break;
     case SET_VAR_DEGREE_MIN:
       v = new (home) ViewSelVirtual<ViewSelDegreeMin<SetView> >(home,o_vars);
       break;
     case SET_VAR_DEGREE_MAX:
       v = new (home) ViewSelVirtual<ViewSelDegreeMin<SetView> >(home,o_vars);
       break;
     case SET_VAR_MIN_MIN:
       v = new (home) ViewSelVirtual<ByMinMin>(home,o_vars);
       break;
     case SET_VAR_MIN_MAX:
       v = new (home) ViewSelVirtual<ByMinMin>(home,o_vars);
       break;
     case SET_VAR_MAX_MIN:
       v = new (home) ViewSelVirtual<ByMaxMin>(home,o_vars);
       break;
     case SET_VAR_MAX_MAX:
       v = new (home) ViewSelVirtual<ByMaxMax>(home,o_vars);
       break;
     case SET_VAR_SIZE_MIN:
       v = new (home) ViewSelVirtual<BySizeMin>(home,o_vars);
       break;
     case SET_VAR_SIZE_MAX:
       v = new (home) ViewSelVirtual<BySizeMax>(home,o_vars);
       break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

}}}

namespace Gecode {

  void
  branch(Gecode::Space& home, const SetVarArgs& x,
         SetVarBranch vars, SetValBranch vals,
         const Gecode::VarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Set;
    using namespace Gecode::Set::Branch;


    if (home.failed()) return;
    ViewArray<SetView> xv(home,x);
    switch (vars) {
    case SET_VAR_NONE:
      {
        ViewSelNone<SetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_RND:
      {
        ViewSelRnd<SetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<SetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_DEGREE_MAX:
      {
        ViewSelDegreeMin<SetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MIN_MIN:
      {
        ByMinMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MIN_MAX:
      {
        ByMinMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MAX_MIN:
      {
        ByMaxMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MAX_MAX:
      {
        ByMaxMax v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_SIZE_MIN:
      {
        BySizeMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_SIZE_MAX:
      {
        BySizeMax v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

  void
  branch(Gecode::Space& home, const SetVarArgs& x,
         const Gecode::TieBreakVarBranch<SetVarBranch>& vars,
         SetValBranch vals,
         const Gecode::TieBreakVarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Set;
    using namespace Gecode::Set::Branch;


    if (home.failed()) return;
    if ((vars.a == SET_VAR_NONE) || (vars.a == SET_VAR_RND) ||
        ((vars.b == SET_VAR_NONE) && (vars.c == SET_VAR_NONE) && (vars.d == SET_VAR_NONE))) {
      branch(home,x,vars.a,vals,o_vars.a,o_vals);
      return;
    }
    ViewArray<SetView> xv(home,x);
    Gecode::ViewSelVirtualBase<SetView>* tb[3];
    int n=0;
    if (vars.b != SET_VAR_NONE)
      virtualize(home,vars.b,o_vars.b,tb[n++]);
    if (vars.c != SET_VAR_NONE)
      virtualize(home,vars.c,o_vars.c,tb[n++]);
    if (vars.d != SET_VAR_NONE)
      virtualize(home,vars.d,o_vars.d,tb[n++]);
    assert(n > 0);
    ViewSelTieBreakDynamic<SetView> vbcd(home,tb,n);
    switch (vars.a) {
    case SET_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<SetView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<SetView>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_DEGREE_MAX:
      {
        ViewSelDegreeMin<SetView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<SetView>,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MIN_MIN:
      {
        ByMinMin va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMinMin,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MIN_MAX:
      {
        ByMinMin va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMinMin,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MAX_MIN:
      {
        ByMaxMin va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMaxMin,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_MAX_MAX:
      {
        ByMaxMax va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMaxMax,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_SIZE_MIN:
      {
        BySizeMin va(home,o_vars.a);
        ViewSelTieBreakStatic<BySizeMin,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case SET_VAR_SIZE_MAX:
      {
        BySizeMax va(home,o_vars.a);
        ViewSelTieBreakStatic<BySizeMax,
          ViewSelTieBreakDynamic<SetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

  namespace {
    class BranchingRegistrar {
    public:
      BranchingRegistrar(void) {
        using namespace Gecode;
        using namespace Gecode::Set;
        using namespace Gecode::Set::Branch;
        registerAllSetView< ViewSelNone<SetView> >();
        registerAllSetView< ViewSelRnd<SetView> >();
        registerAllSetView< ViewSelDegreeMin<SetView> >();
        registerAllSetView< ViewSelDegreeMin<SetView> >();
        registerAllSetView< ByMinMin >();
        registerAllSetView< ByMinMin >();
        registerAllSetView< ByMaxMin >();
        registerAllSetView< ByMaxMax >();
        registerAllSetView< BySizeMin >();
        registerAllSetView< BySizeMax >();
        registerAllSetView<ViewSelTieBreakStatic<ViewSelDegreeMin<SetView>,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<ViewSelDegreeMin<SetView>,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<ByMinMin,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<ByMinMin,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<ByMaxMin,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<ByMaxMax,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<BySizeMin,
                           ViewSelTieBreakDynamic<SetView> > >();
        registerAllSetView<ViewSelTieBreakStatic<BySizeMax,
                           ViewSelTieBreakDynamic<SetView> > >();
      }
    };
    BranchingRegistrar r;
  }
}


// STATISTICS: set-branch

