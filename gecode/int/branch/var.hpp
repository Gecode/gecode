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
    : VarBranch(NULL), s(SEL_NONE) {}

  forceinline 
  IntVarBranch::IntVarBranch(Select s0, BranchTbl t)
    : VarBranch(t), s(s0) {}

  forceinline 
  IntVarBranch::IntVarBranch(Rnd r)
    : VarBranch(r), s(SEL_RND) {}

  forceinline 
  IntVarBranch::IntVarBranch(Select s0, Activity a, BranchTbl t)
    : VarBranch(a,t), s(s0) {}

  forceinline 
  IntVarBranch::IntVarBranch(Select s0, void* mf, BranchTbl t)
    : VarBranch(mf,t), s(s0) {}

  forceinline IntVarBranch::Select
  IntVarBranch::select(void) const {
    return s;
  }

  inline IntVarBranch
  INT_VAR_NONE(void) {
    return IntVarBranch(IntVarBranch::SEL_NONE,NULL);
  }

  inline IntVarBranch
  INT_VAR_RND(Rnd r) {
    return IntVarBranch(r);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MIN(IntBranchMerit bm, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MIN,bm,tbl);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MIN(BoolBranchMerit bm, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MIN,bm,tbl);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MAX(IntBranchMerit bm, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MAX,bm,tbl);
  }

  inline IntVarBranch
  INT_VAR_MERIT_MAX(BoolBranchMerit bm, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MERIT_MAX,bm,tbl);
  }

  inline IntVarBranch
  INT_VAR_DEGREE_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_DEGREE_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_DEGREE_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_DEGREE_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MIN(IntActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MIN(BoolActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MAX(IntActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTIVITY_MAX(BoolActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTIVITY_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_MIN_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MIN_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_MIN_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MIN_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_MAX_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MAX_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_MAX_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_MAX_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_DEGREE_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_DEGREE_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_DEGREE_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_DEGREE_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_AFC_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_AFC_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_AFC_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_AFC_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MIN(IntActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MIN(BoolActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MAX(IntActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_SIZE_ACTIVITY_MAX(BoolActivity a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_SIZE_ACTIVITY_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MIN_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MIN_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MIN_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MIN_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MAX_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MAX_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_REGRET_MAX_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_REGRET_MAX_MAX,tbl);
  }

}

// STATISTICS: int-branch
