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
    : VarBranch(0.0), s(SEL_NONE) {}

  forceinline 
  SetVarBranch::SetVarBranch(Select s0, double t)
    : VarBranch(t), s(s0) {}

  forceinline 
  SetVarBranch::SetVarBranch(Rnd r)
    : VarBranch(r), s(SEL_RND) {}

  forceinline 
  SetVarBranch::SetVarBranch(Select s0, Activity a, double t)
    : VarBranch(a,t), s(s0) {}

  forceinline 
  SetVarBranch::SetVarBranch(Select s0, void* mf, double t)
    : VarBranch(mf,t), s(s0) {}

  forceinline SetVarBranch::Select
  SetVarBranch::select(void) const {
    return s;
  }


  inline SetVarBranch
  SET_VAR_NONE(void) {
    return SetVarBranch(SetVarBranch::SEL_NONE,0.0);
  }

  inline SetVarBranch
  SET_VAR_RND(Rnd r) {
    return SetVarBranch(r);
  }

  inline SetVarBranch
  SET_VAR_MERIT_MIN(SetBranchMerit bm, double tbt) {
    return SetVarBranch(SetVarBranch::SEL_MERIT_MIN,bm,tbt);
  }

  inline SetVarBranch
  SET_VAR_MERIT_MAX(SetBranchMerit bm, double tbt) {
    return SetVarBranch(SetVarBranch::SEL_MERIT_MAX,bm,tbt);
  }

  inline SetVarBranch
  SET_VAR_DEGREE_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_DEGREE_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_DEGREE_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_DEGREE_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_AFC_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_AFC_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_ACTIVITY_MIN(SetActivity a, double tbt) {
    return SetVarBranch(SetVarBranch::SEL_ACTIVITY_MIN,a,tbt);
  }

  inline SetVarBranch
  SET_VAR_ACTIVITY_MAX(SetActivity a, double tbt) {
    return SetVarBranch(SetVarBranch::SEL_ACTIVITY_MAX,a,tbt);
  }

  inline SetVarBranch
  SET_VAR_MIN_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_MIN_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_MIN_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_MIN_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_MAX_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_MAX_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_MAX_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_MAX_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_DEGREE_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_DEGREE_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_DEGREE_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_DEGREE_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_AFC_MIN(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_AFC_MIN,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_AFC_MAX(double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_AFC_MAX,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_ACTIVITY_MIN(SetActivity a, double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_ACTIVITY_MIN,a,tbt);
  }

  inline SetVarBranch
  SET_VAR_SIZE_ACTIVITY_MAX(SetActivity a, double tbt) {
    return SetVarBranch(SetVarBranch::SEL_SIZE_ACTIVITY_MAX,a,tbt);
  }

}

// STATISTICS: set-branch
