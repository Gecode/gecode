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
  post(Gecode::Space* home, IntVarBranch vars,
       const Gecode::VarBranchOptions& o_vars) {
    switch (vars) {
     case INT_VAR_NONE:
       return new (home) ViewSelVirtual<ViewSelByNone<IntView> >();
     case INT_VAR_MIN_MIN:
       return new (home) ViewSelVirtual<ByMinMin<IntView> >();
     case INT_VAR_MIN_MAX:
       return new (home) ViewSelVirtual<ByMinMax<IntView> >();
     case INT_VAR_MAX_MIN:
       return new (home) ViewSelVirtual<ByMaxMin<IntView> >();
     case INT_VAR_MAX_MAX:
       return new (home) ViewSelVirtual<ByMaxMax<IntView> >();
     case INT_VAR_SIZE_MIN:
       return new (home) ViewSelVirtual<BySizeMin<IntView> >();
     case INT_VAR_SIZE_MAX:
       return new (home) ViewSelVirtual<BySizeMax<IntView> >();
     case INT_VAR_DEGREE_MIN:
       return new (home) ViewSelVirtual<ViewSelByDegreeMin<IntView> >();
     case INT_VAR_DEGREE_MAX:
       return new (home) ViewSelVirtual<ViewSelByDegreeMax<IntView> >();
     case INT_VAR_SIZE_DEGREE_MIN:
       return new (home) ViewSelVirtual<BySizeDegreeMin<IntView> >();
     case INT_VAR_SIZE_DEGREE_MAX:
       return new (home) ViewSelVirtual<BySizeDegreeMax<IntView> >();
     case INT_VAR_REGRET_MIN_MIN:
       return new (home) ViewSelVirtual<ByRegretMinMin<IntView> >();
     case INT_VAR_REGRET_MIN_MAX:
       return new (home) ViewSelVirtual<ByRegretMinMax<IntView> >();
     case INT_VAR_REGRET_MAX_MIN:
       return new (home) ViewSelVirtual<ByRegretMaxMin<IntView> >();
     case INT_VAR_REGRET_MAX_MAX:
       return new (home) ViewSelVirtual<ByRegretMaxMax<IntView> >();
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  template<int n>
  void
  post(Gecode::Space* home, Gecode::ViewArray<IntView>& x,
       IntVarBranch vars, Gecode::ViewSelVirtualBase<IntView>* tb[n],
       IntValBranch vals,
       const Gecode::ValBranchOptions& o_vals) {
    switch (vars) {
    case INT_VAR_MIN_MIN: {
        ViewSelTieBreak<ByMinMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByMinMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX: {
        ViewSelTieBreak<ByMinMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByMinMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN: {
        ViewSelTieBreak<ByMaxMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByMaxMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX: {
        ViewSelTieBreak<ByMaxMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByMaxMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN: {
        ViewSelTieBreak<BySizeMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<BySizeMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX: {
        ViewSelTieBreak<BySizeMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<BySizeMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN: {
        ViewSelTieBreak<ViewSelByDegreeMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByDegreeMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX: {
        ViewSelTieBreak<ViewSelByDegreeMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<ViewSelByDegreeMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN: {
        ViewSelTieBreak<BySizeDegreeMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<BySizeDegreeMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX: {
        ViewSelTieBreak<BySizeDegreeMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<BySizeDegreeMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN: {
        ViewSelTieBreak<ByRegretMinMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByRegretMinMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX: {
        ViewSelTieBreak<ByRegretMinMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByRegretMinMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN: {
        ViewSelTieBreak<ByRegretMaxMin<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByRegretMaxMin<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX: {
        ViewSelTieBreak<ByRegretMaxMax<IntView>,n> v(tb);
        post<ViewSelTieBreak<ByRegretMaxMax<IntView>,n> >(home,x,v,vals,o_vals);
      }
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

  void
  post(Gecode::Space* home, Gecode::ViewArray<IntView>& x,
       const Gecode::TieBreakVarBranch<IntVarBranch>& vars,
       IntValBranch vals,
       const Gecode::TieBreakVarBranchOptions& o_vars,
       const Gecode::ValBranchOptions& o_vals) {
    if ((vars.a == INT_VAR_NONE) ||
        ((vars.b == INT_VAR_NONE) && 
         (vars.c == INT_VAR_NONE) && 
         (vars.d == INT_VAR_NONE))) {
      switch (vars.a) {
    case INT_VAR_NONE: {
        ViewSelByNone<IntView> v; // v(o_vars.a)
        post<ViewSelByNone<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MIN: {
        ByMinMin<IntView> v; // v(o_vars.a)
        post<ByMinMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MIN_MAX: {
        ByMinMax<IntView> v; // v(o_vars.a)
        post<ByMinMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MIN: {
        ByMaxMin<IntView> v; // v(o_vars.a)
        post<ByMaxMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_MAX_MAX: {
        ByMaxMax<IntView> v; // v(o_vars.a)
        post<ByMaxMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MIN: {
        BySizeMin<IntView> v; // v(o_vars.a)
        post<BySizeMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_MAX: {
        BySizeMax<IntView> v; // v(o_vars.a)
        post<BySizeMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MIN: {
        ViewSelByDegreeMin<IntView> v; // v(o_vars.a)
        post<ViewSelByDegreeMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_DEGREE_MAX: {
        ViewSelByDegreeMax<IntView> v; // v(o_vars.a)
        post<ViewSelByDegreeMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MIN: {
        BySizeDegreeMin<IntView> v; // v(o_vars.a)
        post<BySizeDegreeMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_SIZE_DEGREE_MAX: {
        BySizeDegreeMax<IntView> v; // v(o_vars.a)
        post<BySizeDegreeMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MIN: {
        ByRegretMinMin<IntView> v; // v(o_vars.a)
        post<ByRegretMinMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MIN_MAX: {
        ByRegretMinMax<IntView> v; // v(o_vars.a)
        post<ByRegretMinMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MIN: {
        ByRegretMaxMin<IntView> v; // v(o_vars.a)
        post<ByRegretMaxMin<IntView> >(home,x,v,vals,o_vals);
      }
      break;
    case INT_VAR_REGRET_MAX_MAX: {
        ByRegretMaxMax<IntView> v; // v(o_vars.a)
        post<ByRegretMaxMax<IntView> >(home,x,v,vals,o_vals);
      }
      break;
      default:
        throw UnknownBranching("Int::branch");
      }
    } else if ((vars.c == INT_VAR_NONE) && 
               (vars.d == INT_VAR_NONE)) {
      Gecode::ViewSelVirtualBase<IntView>* tb[1] = {
        post(home,vars.b,o_vars.b)
      };
      post<1>(home,x,vars.a,tb,vals,o_vals);
    } else if (vars.d == INT_VAR_NONE) {
      Gecode::ViewSelVirtualBase<IntView>* tb[2] = {
        post(home,vars.b,o_vars.b),
        post(home,vars.c,o_vars.c)
      };
      post<2>(home,x,vars.a,tb,vals,o_vals);
    } else {
      Gecode::ViewSelVirtualBase<IntView>* tb[3] = {
        post(home,vars.b,o_vars.b),
        post(home,vars.c,o_vars.c),
        post(home,vars.d,o_vars.d)
      };
      post<3>(home,x,vars.a,tb,vals,o_vals);
    }
  }


}}}

// STATISTICS: int-post

