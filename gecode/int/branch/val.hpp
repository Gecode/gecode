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
  IntValBranch::IntValBranch(Select s0)
    : s(s0) {}

  forceinline
  IntValBranch::IntValBranch(Rnd r)
    : ValBranch<IntVar>(r), s(SEL_RND) {}

  forceinline
  IntValBranch::IntValBranch(IntBranchVal v, IntBranchCommit c)
    : ValBranch<IntVar>(v,c), s(SEL_VAL_COMMIT) {}

  forceinline IntValBranch::Select
  IntValBranch::select(void) const {
    return s;
  }


  inline IntValBranch
  INT_VAL_MIN(void) {
    return IntValBranch(IntValBranch::SEL_MIN);
  }

  inline IntValBranch
  INT_VAL_MED(void) {
    return IntValBranch(IntValBranch::SEL_MED);
  }

  inline IntValBranch
  INT_VAL_MAX(void) {
    return IntValBranch(IntValBranch::SEL_MAX);
  }

  inline IntValBranch
  INT_VAL_RND(Rnd r) {
    return IntValBranch(r);
  }

  inline IntValBranch
  INT_VAL_SPLIT_MIN(void) {
    return IntValBranch(IntValBranch::SEL_SPLIT_MIN);
  }

  inline IntValBranch
  INT_VAL_SPLIT_MAX(void) {
    return IntValBranch(IntValBranch::SEL_SPLIT_MAX);
  }

  inline IntValBranch
  INT_VAL_RANGE_MIN(void) {
    return IntValBranch(IntValBranch::SEL_RANGE_MIN);
  }

  inline IntValBranch
  INT_VAL_RANGE_MAX(void) {
    return IntValBranch(IntValBranch::SEL_RANGE_MAX);
  }

  inline IntValBranch
  INT_VAL(IntBranchVal v, IntBranchCommit c) {
    return IntValBranch(v,c);
  }

  inline IntValBranch
  INT_VALUES_MIN(void) {
    return IntValBranch(IntValBranch::SEL_VALUES_MIN);
  }

  inline IntValBranch
  INT_VALUES_MAX(void) {
    return IntValBranch(IntValBranch::SEL_VALUES_MAX);
  }



  forceinline
  BoolValBranch::BoolValBranch(Select s0)
    : s(s0) {}

  forceinline
  BoolValBranch::BoolValBranch(Rnd r)
    : ValBranch<BoolVar>(r), s(SEL_RND) {}

  forceinline
  BoolValBranch::BoolValBranch(BoolBranchVal v, BoolBranchCommit c)
    : ValBranch<BoolVar>(v,c), s(SEL_VAL_COMMIT) {}

  forceinline BoolValBranch::Select
  BoolValBranch::select(void) const {
    return s;
  }


  inline BoolValBranch
  BOOL_VAL_MIN(void) {
    return BoolValBranch(BoolValBranch::SEL_MIN);
  }

  inline BoolValBranch
  BOOL_VAL_MAX(void) {
    return BoolValBranch(BoolValBranch::SEL_MAX);
  }

  inline BoolValBranch
  BOOL_VAL_RND(Rnd r) {
    return BoolValBranch(r);
  }

  inline BoolValBranch
  BOOL_VAL(BoolBranchVal v, BoolBranchCommit c) {
    return BoolValBranch(v,c);
  }

}

// STATISTICS: int-branch
