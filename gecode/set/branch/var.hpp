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
  SetVarBranch::SetVarBranch(void)
    : VarBranch(NULL), s(SEL_NONE) {}

  forceinline 
  SetVarBranch::SetVarBranch(Select s0, BranchTbl t)
    : VarBranch(t), s(s0) {}

  forceinline 
  SetVarBranch::SetVarBranch(Rnd r)
    : VarBranch(r), s(SEL_RND) {}

  forceinline 
  SetVarBranch::SetVarBranch(Select s0, Activity a, BranchTbl t)
    : VarBranch(a,t), s(s0) {}

  forceinline 
  SetVarBranch::SetVarBranch(Select s0, void* mf, BranchTbl t)
    : VarBranch(mf,t), s(s0) {}

  forceinline SetVarBranch::Select
  SetVarBranch::select(void) const {
    return s;
  }


  inline SetVarBranch
  SET_VAR_NONE(void) {
    return SetVarBranch(SetVarBranch::SEL_NONE,NULL);
  }

  inline SetVarBranch
  SET_VAR_RND(Rnd r) {
    return SetVarBranch(r);
  }

  inline SetVarBranch
  SET_VAR_MERIT_MIN(SetBranchMerit bm, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_MERIT_MIN,bm,tbl);
  }

  inline SetVarBranch
  SET_VAR_MERIT_MAX(SetBranchMerit bm, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_MERIT_MAX,bm,tbl);
  }

  inline SetVarBranch
  SET_VAR_DEGREE_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_DEGREE_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_DEGREE_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_DEGREE_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTIVITY_MIN(SetActivity a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTIVITY_MIN,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTIVITY_MAX(SetActivity a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTIVITY_MAX,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_MIN_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_MIN_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_MIN_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_MIN_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_MAX_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_MAX_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_MAX_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_MAX_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_DEGREE_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_DEGREE_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_DEGREE_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_DEGREE_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_AFC_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_AFC_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_AFC_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_AFC_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_ACTIVITY_MIN(SetActivity a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_ACTIVITY_MIN,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_SIZE_ACTIVITY_MAX(SetActivity a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_ACTIVITY_MAX,a,tbl);
  }

}

// STATISTICS: set-branch
