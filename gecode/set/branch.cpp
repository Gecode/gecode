/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include <gecode/set/branch.hh>

namespace Gecode {

  void
  assign(Space& home, const SetVarArgs& x, SetAssign vals,
         const ValBranchOptions& o_vals) {
    using namespace Set;
    if (home.failed()) return;
    ViewArray<SetView> xv(home,x);
    ViewSelNone<SetView> v(home,VarBranchOptions::def);
    switch (vals) {
    case SET_ASSIGN_MIN_INC:
      {
        Branch::AssignValMin<true> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<SetView>,Branch::AssignValMin<true> >
          (home,xv,v,a);
      }
      break;
    case SET_ASSIGN_MIN_EXC:
      {
        Branch::AssignValMin<false> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<SetView>,Branch::AssignValMin<false> >
          (home,xv,v,a);
      }
      break;
    case SET_ASSIGN_MAX_INC:
      {
        Branch::AssignValMax<true> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<SetView>,Branch::AssignValMax<true> >
          (home,xv,v,a);
      }
      break;
    case SET_ASSIGN_MAX_EXC:
      {
        Branch::AssignValMax<false> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<SetView>,Branch::AssignValMax<false> >
          (home,xv,v,a);
      }
      break;
    case SET_ASSIGN_RND_INC:
      {
        Branch::AssignValRnd<true> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<SetView>,Branch::AssignValRnd<true> >
          (home,xv,v,a);
      }
      break;
    case SET_ASSIGN_RND_EXC:
      {
        Branch::AssignValRnd<false> a(home,o_vals);
        (void) new (home) ViewValBranching
          <ViewSelNone<SetView>,Branch::AssignValRnd<false> >
          (home,xv,v,a);
      }
      break;
    default:
      throw UnknownBranching("Set::assign");
    }
  }

}

// STATISTICS: set-branch

