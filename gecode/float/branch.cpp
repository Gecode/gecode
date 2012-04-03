/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Vincent Barichard, 2012
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include <gecode/float/branch.hh>

namespace Gecode {

  void
  assign(Home home, const FloatVarArgs& x, FloatAssign vals,
         const ValBranchOptions& o_vals) {
    using namespace Float;
    if (home.failed()) return;
    ViewArray<FloatView> xv(home,x);
    ViewSelNone<FloatView> v(home,VarBranchOptions::def);
    switch (vals) {
    case FLOAT_ASSIGN_MIN:
      {
        Branch::AssignValMin<FloatView> a(home,o_vals);
        ViewValBrancher
          <ViewSelNone<FloatView>,Branch::AssignValMin<FloatView> >
          ::post(home,xv,v,a);
      }
      break;
    case FLOAT_ASSIGN_MAX:
      {
        Branch::AssignValMax<FloatView> a(home,o_vals);
        ViewValBrancher
          <ViewSelNone<FloatView>,Branch::AssignValMax<FloatView> >
          ::post(home,xv,v,a);
      }
      break;
    case FLOAT_ASSIGN_RND:
      {
        Branch::AssignValRnd<FloatView> a(home,o_vals);
        ViewValBrancher
          <ViewSelNone<FloatView>,Branch::AssignValRnd<FloatView> >
          ::post(home,xv,v,a);
      }
      break;
    default:
      throw UnknownBranching("Float::assign");
    }
  }

  void
  branch(Home home, FloatVar x, FloatValBranch vals,
         const ValBranchOptions& o_vals) {
    FloatVarArgs xv(1); xv[0]=x;
    branch(home, xv, FLOAT_VAR_NONE, vals, VarBranchOptions::def, o_vals);
  }
  
  void
  assign(Home home, FloatVar x, FloatAssign vals,
         const ValBranchOptions& o_vals) {
    FloatVarArgs xv(1); xv[0]=x;
    assign(home, xv, vals, o_vals);
  }
  
}

// STATISTICS: float-post
