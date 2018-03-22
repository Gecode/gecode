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
  SetVarBranch::SetVarBranch(void)
    : s(SEL_NONE) {}

  forceinline
  SetVarBranch::SetVarBranch(Select s0, BranchTbl t)
    : VarBranch<SetVar>(t), s(s0) {}

  forceinline
  SetVarBranch::SetVarBranch(Rnd r)
    : VarBranch<SetVar>(r), s(SEL_RND) {}

  forceinline
  SetVarBranch::SetVarBranch(Select s0, double d, BranchTbl t)
    : VarBranch<SetVar>(d,t), s(s0) {}

  forceinline
  SetVarBranch::SetVarBranch(Select s0, SetAFC a, BranchTbl t)
    : VarBranch<SetVar>(a,t), s(s0) {}

  forceinline
  SetVarBranch::SetVarBranch(Select s0, SetAction a, BranchTbl t)
    : VarBranch<SetVar>(a,t), s(s0) {}

  forceinline
  SetVarBranch::SetVarBranch(Select s0, SetCHB c, BranchTbl t)
    : VarBranch<SetVar>(c,t), s(s0) {}

  forceinline
  SetVarBranch::SetVarBranch(Select s0, SetBranchMerit mf, BranchTbl t)
    : VarBranch<SetVar>(mf,t), s(s0) {}

  forceinline SetVarBranch::Select
  SetVarBranch::select(void) const {
    return s;
  }

  forceinline void
  SetVarBranch::expand(Home home, const SetVarArgs& x) {
    switch (select()) {
    case SEL_AFC_MIN: case SEL_AFC_MAX:
    case SEL_AFC_SIZE_MIN: case SEL_AFC_SIZE_MAX:
      if (!_afc)
        _afc = SetAFC(home,x,decay());
      break;
    case SEL_ACTION_MIN: case SEL_ACTION_MAX:
    case SEL_ACTION_SIZE_MIN: case SEL_ACTION_SIZE_MAX:
      if (!_act)
        _act = SetAction(home,x,decay());
      break;
    case SEL_CHB_MIN: case SEL_CHB_MAX:
    case SEL_CHB_SIZE_MIN: case SEL_CHB_SIZE_MAX:
      if (!_chb)
        _chb = SetCHB(home,x);
      break;
    default: ;
    }
  }

  inline SetVarBranch
  SET_VAR_NONE(void) {
    return SetVarBranch(SetVarBranch::SEL_NONE,nullptr);
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
  SET_VAR_AFC_MIN(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MIN,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_MIN(SetAFC a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MIN,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_MAX(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MAX,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_MAX(SetAFC a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_MAX,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_MIN(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_MIN,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_MIN(SetAction a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_MIN,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_MAX(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_MAX,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_MAX(SetAction a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_MAX,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_MIN(SetCHB c, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_MIN,c,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_MAX(SetCHB c, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_MAX,c,tbl);
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
  SET_VAR_DEGREE_SIZE_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_DEGREE_SIZE_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_DEGREE_SIZE_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_DEGREE_SIZE_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_SIZE_MIN(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MIN,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_SIZE_MIN(SetAFC a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MIN,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_SIZE_MAX(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MAX,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_AFC_SIZE_MAX(SetAFC a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MAX,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_SIZE_MIN(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MIN,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_SIZE_MIN(SetAction a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MIN,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_SIZE_MAX(double d, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MAX,d,tbl);
  }

  inline SetVarBranch
  SET_VAR_ACTION_SIZE_MAX(SetAction a, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MAX,a,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_SIZE_MIN(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MIN,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_SIZE_MIN(SetCHB c, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MIN,c,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_SIZE_MAX(BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MAX,tbl);
  }

  inline SetVarBranch
  SET_VAR_CHB_SIZE_MAX(SetCHB c, BranchTbl tbl) {
    return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MAX,c,tbl);
  }

}

// STATISTICS: set-branch
