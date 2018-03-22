/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode {

  forceinline
  IntAssign::IntAssign(Select s0)
    : s(s0) {}

  forceinline
  IntAssign::IntAssign(Rnd r)
    : ValBranch<IntVar>(r), s(SEL_RND) {}

  forceinline
  IntAssign::IntAssign(IntBranchVal v, IntBranchCommit c)
    : ValBranch<IntVar>(v,c), s(SEL_VAL_COMMIT) {}

  forceinline IntAssign::Select
  IntAssign::select(void) const {
    return s;
  }


  inline IntAssign
  INT_ASSIGN_MIN(void) {
    return IntAssign(IntAssign::SEL_MIN);
  }

  inline IntAssign
  INT_ASSIGN_MED(void) {
    return IntAssign(IntAssign::SEL_MED);
  }

  inline IntAssign
  INT_ASSIGN_MAX(void) {
    return IntAssign(IntAssign::SEL_MAX);
  }

  inline IntAssign
  INT_ASSIGN_RND(Rnd r) {
    return IntAssign(r);
  }

  inline IntAssign
  INT_ASSIGN(IntBranchVal v, IntBranchCommit c) {
    return IntAssign(v,c);
  }



  forceinline
  BoolAssign::BoolAssign(Select s0)
    : s(s0) {}

  forceinline
  BoolAssign::BoolAssign(Rnd r)
    : ValBranch<BoolVar>(r), s(SEL_RND) {}

  forceinline
  BoolAssign::BoolAssign(BoolBranchVal v, BoolBranchCommit c)
    : ValBranch<BoolVar>(v,c), s(SEL_VAL_COMMIT) {}

  forceinline BoolAssign::Select
  BoolAssign::select(void) const {
    return s;
  }


  inline BoolAssign
  BOOL_ASSIGN_MIN(void) {
    return BoolAssign(BoolAssign::SEL_MIN);
  }

  inline BoolAssign
  BOOL_ASSIGN_MAX(void) {
    return BoolAssign(BoolAssign::SEL_MAX);
  }

  inline BoolAssign
  BOOL_ASSIGN_RND(Rnd r) {
    return BoolAssign(r);
  }

  inline BoolAssign
  BOOL_ASSIGN(BoolBranchVal v, BoolBranchCommit c) {
    return BoolAssign(v,c);
  }

}

// STATISTICS: int-branch
