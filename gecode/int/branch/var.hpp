/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode {

  forceinline 
  IntVarBranch::IntVarBranch(void)
    : VarBranch(0.0), s(SEL_NONE) {}

  forceinline 
  IntVarBranch::IntVarBranch(Select s0, double t)
    : VarBranch(t), s(s0) {}

  forceinline 
  IntVarBranch::IntVarBranch(Rnd r)
    : VarBranch(r), s(SEL_RND) {}

  forceinline 
  IntVarBranch::IntVarBranch(Select s0, Activity a, double t)
    : VarBranch(a,t), s(s0) {}

  forceinline 
  IntVarBranch::IntVarBranch(Select s0, void* mf, double t)
    : VarBranch(mf,t), s(s0) {}

  forceinline IntVarBranch::Select
  IntVarBranch::select(void) const {
    return s;
  }

  inline IntVarBranch
  INT_VAR_NONE(void) {
    return IntVarBranch(IntVarBranch::SEL_NONE,0.0);
  }

  inline IntVarBranch
  INT_VAR_RND(Rnd r) {
    return IntVarBranch(r);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MIN(IntBranchMerit bm, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MIN,bm,tbt);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MIN(BoolBranchMerit bm, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MIN,bm,tbt);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MAX(IntBranchMerit bm, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MAX,bm,tbt);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MAX(BoolBranchMerit bm, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MAX,bm,tbt);
  }

  inline IntVarBranch
  INT_VAR_DEGREE_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_DEGREE_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_DEGREE_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_DEGREE_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_AFC_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_AFC_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MIN(IntActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MIN,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MIN(BoolActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MIN,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MAX(IntActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MAX,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MAX(BoolActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MAX,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_MIN_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MIN_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_MIN_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MIN_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_MAX_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MAX_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_MAX_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_MAX_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_DEGREE_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_DEGREE_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_DEGREE_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_DEGREE_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_AFC_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_AFC_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_AFC_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_AFC_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MIN(IntActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MIN,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MIN(BoolActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MIN,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MAX(IntActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MAX,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MAX(BoolActivity a, double tbt) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MAX,a,tbt);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MIN_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MIN_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MIN_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MIN_MAX,tbt);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MAX_MIN(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MAX_MIN,tbt);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MAX_MAX(double tbt) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MAX_MAX,tbt);
  }

}

// STATISTICS: int-branch
