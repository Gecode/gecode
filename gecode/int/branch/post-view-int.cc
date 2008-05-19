/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file
 *      gecode/int/branch/post-view-int.bs
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

  Gecode::ViewSelVirtualBase<IntView>*
  virtualize(Gecode::Space* home, IntVarBranch vars,
             const Gecode::VarBranchOptions& o_vars) {
    switch (vars) {
     case INT_VAR_NONE:
       return new (home) ViewSelVirtual<ViewSelNone<IntView> >(home,o_vars);
     case INT_VAR_RND:
       return new (home) ViewSelVirtual<ViewSelRnd<IntView> >(home,o_vars);
     case INT_VAR_MIN_MIN:
       return new (home) ViewSelVirtual<ByMinMin<IntView> >(home,o_vars);
     case INT_VAR_MIN_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<ByMinMin<IntView> > >(home,o_vars);
     case INT_VAR_MAX_MIN:
       return new (home) ViewSelVirtual<ByMaxMin<IntView> >(home,o_vars);
     case INT_VAR_MAX_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<ByMaxMin<IntView> > >(home,o_vars);
     case INT_VAR_SIZE_MIN:
       return new (home) ViewSelVirtual<BySizeMin<IntView> >(home,o_vars);
     case INT_VAR_SIZE_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<BySizeMin<IntView> > >(home,o_vars);
     case INT_VAR_DEGREE_MIN:
       return new (home) ViewSelVirtual<ViewSelDegreeMin<IntView> >(home,o_vars);
     case INT_VAR_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<ViewSelDegreeMin<IntView> > >(home,o_vars);
     case INT_VAR_SIZE_DEGREE_MIN:
       return new (home) ViewSelVirtual<BySizeDegreeMin<IntView> >(home,o_vars);
     case INT_VAR_SIZE_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<BySizeDegreeMin<IntView> > >(home,o_vars);
     case INT_VAR_REGRET_MIN_MIN:
       return new (home) ViewSelVirtual<ByRegretMinMin<IntView> >(home,o_vars);
     case INT_VAR_REGRET_MIN_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<ByRegretMinMin<IntView> > >(home,o_vars);
     case INT_VAR_REGRET_MAX_MIN:
       return new (home) ViewSelVirtual<ByRegretMaxMin<IntView> >(home,o_vars);
     case INT_VAR_REGRET_MAX_MAX:
       return new (home) ViewSelVirtual<ViewSelInvert<ByRegretMaxMin<IntView> > >(home,o_vars);
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}}}

namespace Gecode { 

  void
  branch(Gecode::Space* home, const IntVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         const Gecode::VarBranchOptions& o_vars,
         const Gecode::ValBranchOptions& o_vals) {
    using namespace Gecode;
    using namespace Gecode::Int;
    using namespace Gecode::Int::Branch;


    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (vars) {
    case INT_VAR_NONE:
      {
        ViewSelNone<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_RND:
      {
        ViewSelRnd<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MIN:
      {
        ByMinMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX:
      {
        ViewSelInvert<ByMinMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN:
      {
        ByMaxMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX:
      {
        ViewSelInvert<ByMaxMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN:
      {
        BySizeMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX:
      {
        ViewSelInvert<BySizeMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX:
      {
        ViewSelInvert<ViewSelDegreeMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      {
        BySizeDegreeMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      {
        ViewSelInvert<BySizeDegreeMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN:
      {
        ByRegretMinMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX:
      {
        ViewSelInvert<ByRegretMinMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN:
      {
        ByRegretMaxMin<IntView> v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX:
      {
        ViewSelInvert<ByRegretMaxMin<IntView> > v(home,o_vars);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  branch(Gecode::Space* home, const IntVarArgs& x,
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
    ViewArray<IntView> xv(home,x);
    Gecode::ViewSelVirtualBase<IntView>* tb[3];
    int n=0;
    tb[n++] = virtualize(home,vars.b,o_vars.b);
    if (vars.c != INT_VAR_NONE)
      tb[n++] = virtualize(home,vars.c,o_vars.c);
    if (vars.d != INT_VAR_NONE)
      tb[n++] = virtualize(home,vars.d,o_vars.d);
    ViewSelTieBreakDynamic<IntView> vbcd(home,tb,n);
    switch (vars.a) {
    case INT_VAR_MIN_MIN:
      {
        ByMinMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMinMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX:
      {
        ViewSelInvert<ByMinMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ByMinMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN:
      {
        ByMaxMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ByMaxMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX:
      {
        ViewSelInvert<ByMaxMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ByMaxMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN:
      {
        BySizeMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<BySizeMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX:
      {
        ViewSelInvert<BySizeMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<BySizeMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN:
      {
        ViewSelDegreeMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelDegreeMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX:
      {
        ViewSelInvert<ViewSelDegreeMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ViewSelDegreeMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN:
      {
        BySizeDegreeMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<BySizeDegreeMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX:
      {
        ViewSelInvert<BySizeDegreeMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<BySizeDegreeMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN:
      {
        ByRegretMinMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ByRegretMinMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX:
      {
        ViewSelInvert<ByRegretMinMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ByRegretMinMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN:
      {
        ByRegretMaxMin<IntView> va(home,o_vars.a);
        ViewSelTieBreakStatic<ByRegretMaxMin<IntView>,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX:
      {
        ViewSelInvert<ByRegretMaxMin<IntView> > va(home,o_vars.a);
        ViewSelTieBreakStatic<ViewSelInvert<ByRegretMaxMin<IntView> >,
          ViewSelTieBreakDynamic<IntView> > v(home,va,vbcd);
        post(home,xv,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}


// STATISTICS: int-branch

