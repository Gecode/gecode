/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the specification file "gecode/int/branch/post-view-bool.bs" instead.
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
  post(Gecode::Space* home, IntVarBranch vars,
       const Gecode::VarBranchOptions& o_vars) {
    switch (vars) {
     case INT_VAR_NONE:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_MIN_MIN:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_MIN_MAX:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_MAX_MIN:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_MAX_MAX:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_SIZE_MIN:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_SIZE_MAX:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_DEGREE_MIN:
       return new (home) ViewSelVirtual<ViewSelByDegreeMin<BoolView> >();
     case INT_VAR_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelByDegreeMax<BoolView> >();
     case INT_VAR_SIZE_DEGREE_MIN:
       return new (home) ViewSelVirtual<ViewSelByDegreeMax<BoolView> >();
     case INT_VAR_SIZE_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelByDegreeMin<BoolView> >();
     case INT_VAR_REGRET_MIN_MIN:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_REGRET_MIN_MAX:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_REGRET_MAX_MIN:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
     case INT_VAR_REGRET_MAX_MAX:
       return new (home) ViewSelVirtual<ViewSelByNone<BoolView> >();
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  template<int n>
  void
  post(Gecode::Space* home, Gecode::ViewArray<BoolView>& x,
       IntVarBranch vars, Gecode::ViewSelVirtualBase<BoolView>* tb[n],
       IntValBranch vals,
       const Gecode::ValBranchOptions& o_vals) {
    switch (vars) {
    case INT_VAR_NONE: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MIN: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN: {
        ViewSelTieBreak<ViewSelByDegreeMin<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByDegreeMin<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX: {
        ViewSelTieBreak<ViewSelByDegreeMax<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByDegreeMax<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN: {
        ViewSelTieBreak<ViewSelByDegreeMax<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByDegreeMax<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX: {
        ViewSelTieBreak<ViewSelByDegreeMin<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByDegreeMin<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX: {
        ViewSelTieBreak<ViewSelByNone<BoolView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByNone<BoolView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  post(Gecode::Space* home, Gecode::ViewArray<BoolView>& x,
       const Gecode::TieBreakVarBranch<IntVarBranch>& vars,
       IntValBranch vals,
       const Gecode::TieBreakVarBranchOptions& o_vars,
       const Gecode::ValBranchOptions& o_vals) {
    if ((vars.b == INT_VAR_NONE) && 
        (vars.c == INT_VAR_NONE) && 
        (vars.d == INT_VAR_NONE)) {
      switch (vars.a) {
    case INT_VAR_NONE: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MIN: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN: {
        ViewSelByDegreeMin<BoolView> v; // v(o_vars.a)
        post<ViewSelByDegreeMin<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX: {
        ViewSelByDegreeMax<BoolView> v; // v(o_vars.a)
        post<ViewSelByDegreeMax<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN: {
        ViewSelByDegreeMax<BoolView> v; // v(o_vars.a)
        post<ViewSelByDegreeMax<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX: {
        ViewSelByDegreeMin<BoolView> v; // v(o_vars.a)
        post<ViewSelByDegreeMin<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX: {
        ViewSelByNone<BoolView> v; // v(o_vars.a)
        post<ViewSelByNone<BoolView> >(home,x,v,vals,o_vals);
      }
      break;
      default:
        throw UnknownBranching("Int::branch");
      }
    } else if ((vars.c == INT_VAR_NONE) && 
               (vars.d == INT_VAR_NONE)) {
      Gecode::ViewSelVirtualBase<BoolView>* tb[1] = {
        post(home,vars.b,o_vars.b)
      };
      post<1>(home,x,vars.a,tb,vals,o_vals);
    } else if (vars.d == INT_VAR_NONE) {
      Gecode::ViewSelVirtualBase<BoolView>* tb[2] = {
        post(home,vars.b,o_vars.b),
        post(home,vars.c,o_vars.c)
      };
      post<2>(home,x,vars.a,tb,vals,o_vals);
    } else {
      Gecode::ViewSelVirtualBase<BoolView>* tb[3] = {
        post(home,vars.b,o_vars.b),
        post(home,vars.c,o_vars.c),
        post(home,vars.d,o_vars.d)
      };
      post<3>(home,x,vars.a,tb,vals,o_vals);
    }
  }


}}}

// STATISTICS: int-post

