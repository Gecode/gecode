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

#include "gecode/int/branch.hh"

namespace Gecode { namespace Int { namespace Branch { 

  Gecode::ViewSelVirtualBase<BoolView>*
  virtualize(Gecode::Space* home, IntVarBranch vars,
             const Gecode::VarBranchOptions& o_vars) {
    switch (vars) {
     case INT_VAR_NONE:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_RND:
       return new (home) ViewSelVirtual<ViewSelRnd<BoolView> >(home,o_vars);
     case INT_VAR_MIN_MIN:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_MIN_MAX:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_MAX_MIN:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_MAX_MAX:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_SIZE_MIN:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_SIZE_MAX:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_DEGREE_MIN:
       return new (home) ViewSelVirtual<ViewSelDegreeMin<BoolView> >(home,o_vars);
     case INT_VAR_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<ViewSelDegreeMin<BoolView> > >(home,o_vars);
     case INT_VAR_SIZE_DEGREE_MIN:
       return new (home) ViewSelVirtual<ViewSelInvert<ViewSelDegreeMin<BoolView> > >(home,o_vars);
     case INT_VAR_SIZE_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelDegreeMin<BoolView> >(home,o_vars);
     case INT_VAR_REGRET_MIN_MIN:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_REGRET_MIN_MAX:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_REGRET_MAX_MIN:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
     case INT_VAR_REGRET_MAX_MAX:
       return new (home) ViewSelVirtual<ViewSelNone<BoolView> >(home,o_vars);
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}}}

namespace Gecode { 

  void
  branch(Gecode::Space* home, const BoolVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         const Gecode::VarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    switch (vars) {
    case INT_VAR_NONE:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_RND:
      {
        ViewSelRnd<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX:
      {
        ViewSelInvert<ViewSelDegreeMin<BoolView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      {
        ViewSelInvert<ViewSelDegreeMin<BoolView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      {
        ViewSelDegreeMin<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX:
      {
        ViewSelNone<BoolView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  branch(Gecode::Space* home, const BoolVarArgs& x,
         const Gecode::TieBreakVarBranch<IntVarBranch>& vars,
         IntValBranch vals,
         const Gecode::TieBreakVarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home->failed()) return;
    if ((vars.a == INT_VAR_NONE) || (vars.a == INT_VAR_RND) ||
        ((vars.b == INT_VAR_NONE) && (vars.c == INT_VAR_NONE) && (vars.d == INT_VAR_NONE))) {
      branch(home,x,vars.a,vals,o_vars.a,o_vals);
      return;
    }
    ViewArray<BoolView> xv(home,x);
    Gecode::ViewSelVirtualBase<BoolView>* tb[3];
    int n=0;
    tb[n++] = virtualize(home,vars.b,o_vars.b);
    if (vars.c != INT_VAR_NONE)
      tb[n++] = virtualize(home,vars.c,o_vars.c);
    if (vars.d != INT_VAR_NONE)
      tb[n++] = virtualize(home,vars.d,o_vars.d);
    ViewSelTieBreakDynamic<BoolView> vbcd(home,tb,n);
    switch (vars.a) {
    case INT_VAR_MIN_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX:
      {
        ViewSelInvert<ViewSelDegreeMin<BoolView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ViewSelDegreeMin<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      {
        ViewSelInvert<ViewSelDegreeMin<BoolView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ViewSelDegreeMin<BoolView> >,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      {
        ViewSelDegreeMin<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX:
      {
        ViewSelNone<BoolView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelNone<BoolView>,
          ViewSelTieBreakDynamic<BoolView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}


// STATISTICS: int-branch

