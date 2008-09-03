/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file
 *      gecode/cpltset/branch/post-view.bs
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

#include <gecode/cpltset/branch.hh>

namespace Gecode { namespace CpltSet { namespace Branch { 

  /// Create virtual view selector for tie-breaking
  void
  virtualize(Gecode::Space& home, CpltSetVarBranch vars,
             const Gecode::VarBranchOptions& o_vars,
             Gecode::ViewSelVirtualBase<CpltSetView>*& v) {
    switch (vars) {
     case CPLTSET_VAR_RND:
       v = new (home) ViewSelVirtual<ViewSelRnd<CpltSetView> >(home,o_vars);
       break;
     case CPLTSET_VAR_DEGREE_MIN:
       v = new (home) ViewSelVirtual<ViewSelDegreeMin<CpltSetView> >(home,o_vars);
       break;
     case CPLTSET_VAR_DEGREE_MAX:
       v = new (home) ViewSelVirtual<ViewSelDegreeMax<CpltSetView> >(home,o_vars);
       break;
     case CPLTSET_VAR_MIN_MIN:
       v = new (home) ViewSelVirtual<ByMinMin>(home,o_vars);
       break;
     case CPLTSET_VAR_MIN_MAX:
       v = new (home) ViewSelVirtual<ByMinMax>(home,o_vars);
       break;
     case CPLTSET_VAR_MAX_MIN:
       v = new (home) ViewSelVirtual<ByMaxMin>(home,o_vars);
       break;
     case CPLTSET_VAR_MAX_MAX:
       v = new (home) ViewSelVirtual<ByMaxMax>(home,o_vars);
       break;
     case CPLTSET_VAR_SIZE_MIN:
       v = new (home) ViewSelVirtual<BySizeMin>(home,o_vars);
       break;
     case CPLTSET_VAR_SIZE_MAX:
       v = new (home) ViewSelVirtual<BySizeMax>(home,o_vars);
       break;
    default:
      throw UnknownBranching("CpltSet::branch");
    }
  }

}}}

namespace Gecode { 

  void
  branch(Gecode::Space& home, const CpltSetVarArgs& x,
         CpltSetVarBranch vars, CpltSetValBranch vals,
         const Gecode::VarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::CpltSet;
    using namespace Gecode::CpltSet::Branch;


    if (home.failed()) return;
    ViewArray<CpltSetView> xv(home,x);
    switch (vars) {
    case CPLTSET_VAR_NONE:
      {
        ViewSelNone<CpltSetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_RND:
      {
        ViewSelRnd<CpltSetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<CpltSetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_DEGREE_MAX:
      {
        ViewSelDegreeMax<CpltSetView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MIN_MIN:
      {
        ByMinMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MIN_MAX:
      {
        ByMinMax v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MAX_MIN:
      {
        ByMaxMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MAX_MAX:
      {
        ByMaxMax v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_SIZE_MIN:
      {
        BySizeMin v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_SIZE_MAX:
      {
        BySizeMax v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("CpltSet::branch");
    }
  }

  void
  branch(Gecode::Space& home, const CpltSetVarArgs& x,
         const Gecode::TieBreakVarBranch<CpltSetVarBranch>& vars,
         CpltSetValBranch vals,
         const Gecode::TieBreakVarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::CpltSet;
    using namespace Gecode::CpltSet::Branch;


    if (home.failed()) return;
    if ((vars.a == CPLTSET_VAR_NONE) || (vars.a == CPLTSET_VAR_RND) ||
        ((vars.b == CPLTSET_VAR_NONE) && (vars.c == CPLTSET_VAR_NONE) && (vars.d == CPLTSET_VAR_NONE))) {
      branch(home,x,vars.a,vals,o_vars.a,o_vals);
      return;
    }
    ViewArray<CpltSetView> xv(home,x);
    Gecode::ViewSelVirtualBase<CpltSetView>* tb[3];
    int n=0;
    if (vars.b != CPLTSET_VAR_NONE)
      virtualize(home,vars.b,o_vars.b,tb[n++]);
    if (vars.c != CPLTSET_VAR_NONE)
      virtualize(home,vars.c,o_vars.c,tb[n++]);
    if (vars.d != CPLTSET_VAR_NONE)
      virtualize(home,vars.d,o_vars.d,tb[n++]);
    assert(n > 0);
    ViewSelTieBreakDynamic<CpltSetView> vbcd(home,tb,n);
    switch (vars.a) {
    case CPLTSET_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<CpltSetView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<CpltSetView>,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_DEGREE_MAX:
      {
        ViewSelDegreeMax<CpltSetView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMax<CpltSetView>,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MIN_MIN:
      {
        ByMinMin va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMinMin,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MIN_MAX:
      {
        ByMinMax va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMinMax,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MAX_MIN:
      {
        ByMaxMin va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMaxMin,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_MAX_MAX:
      {
        ByMaxMax va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMaxMax,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_SIZE_MIN:
      {
        BySizeMin va(home,o_vars.a);
        ViewSelTieBreakStatic<BySizeMin,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case CPLTSET_VAR_SIZE_MAX:
      {
        BySizeMax va(home,o_vars.a);
        ViewSelTieBreakStatic<BySizeMax,
          ViewSelTieBreakDynamic<CpltSetView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("CpltSet::branch");
    }
  }

}


// STATISTICS: cpltset-branch

