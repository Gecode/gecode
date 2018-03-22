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
  IntVarBranch::IntVarBranch(void)
    : s(SEL_NONE) {}

  forceinline
  IntVarBranch::IntVarBranch(Select s0, BranchTbl t)
    : VarBranch<IntVar>(t), s(s0) {}

  forceinline
  IntVarBranch::IntVarBranch(Rnd r)
    : VarBranch<IntVar>(r), s(SEL_RND) {}

  forceinline
  IntVarBranch::IntVarBranch(Select s0, double d, BranchTbl t)
    : VarBranch<IntVar>(d,t), s(s0) {}

  forceinline
  IntVarBranch::IntVarBranch(Select s0, IntAFC a, BranchTbl t)
    : VarBranch<IntVar>(a,t), s(s0) {}

  forceinline
  IntVarBranch::IntVarBranch(Select s0, IntAction a, BranchTbl t)
    : VarBranch<IntVar>(a,t), s(s0) {}

  forceinline
  IntVarBranch::IntVarBranch(Select s0, IntCHB c, BranchTbl t)
    : VarBranch<IntVar>(c,t), s(s0) {}

  forceinline
  IntVarBranch::IntVarBranch(Select s0, IntBranchMerit mf, BranchTbl t)
    : VarBranch<IntVar>(mf,t), s(s0) {}

  forceinline IntVarBranch::Select
  IntVarBranch::select(void) const {
    return s;
  }

  forceinline void
  IntVarBranch::expand(Home home, const IntVarArgs& x) {
    switch (select()) {
    case SEL_AFC_MIN: case SEL_AFC_MAX:
    case SEL_AFC_SIZE_MIN: case SEL_AFC_SIZE_MAX:
      if (!_afc)
        _afc = IntAFC(home,x,decay());
      break;
    case SEL_ACTION_MIN: case SEL_ACTION_MAX:
    case SEL_ACTION_SIZE_MIN: case SEL_ACTION_SIZE_MAX:
      if (!_act)
        _act = IntAction(home,x,decay());
      break;
    case SEL_CHB_MIN: case SEL_CHB_MAX:
    case SEL_CHB_SIZE_MIN: case SEL_CHB_SIZE_MAX:
      if (!_chb)
        _chb = IntCHB(home,x);
      break;
    default: ;
    }
  }

  inline IntVarBranch
  INT_VAR_NONE(void) {
    return IntVarBranch(IntVarBranch::SEL_NONE,nullptr);
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
  INT_VAR_MERIT_MAX(IntBranchMerit bm, BranchTbl tbl) {
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
  INT_VAR_AFC_MIN(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MIN,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_MIN(IntAFC a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_MAX(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MAX,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_MAX(IntAFC a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_MIN(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_MIN,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_MIN(IntAction a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_MAX(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_MAX,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_MAX(IntAction a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_MIN(IntCHB c, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_MIN,c,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_MAX(IntCHB c, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_MAX,c,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_MAX,tbl);
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
  INT_VAR_DEGREE_SIZE_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_DEGREE_SIZE_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_DEGREE_SIZE_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_DEGREE_SIZE_MAX,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_SIZE_MIN(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_SIZE_MIN,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_SIZE_MIN(IntAFC a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_SIZE_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_SIZE_MAX(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_SIZE_MAX,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_AFC_SIZE_MAX(IntAFC a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_AFC_SIZE_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_SIZE_MIN(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_SIZE_MIN,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_SIZE_MIN(IntAction a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_SIZE_MIN,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_SIZE_MAX(double d, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_SIZE_MAX,d,tbl);
  }

  inline IntVarBranch
  INT_VAR_ACTION_SIZE_MAX(IntAction a, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_ACTION_SIZE_MAX,a,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_SIZE_MIN(IntCHB c, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_SIZE_MIN,c,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_SIZE_MIN(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_SIZE_MIN,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_SIZE_MAX(IntCHB c, BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_SIZE_MAX,c,tbl);
  }

  inline IntVarBranch
  INT_VAR_CHB_SIZE_MAX(BranchTbl tbl) {
    return IntVarBranch(IntVarBranch::SEL_CHB_SIZE_MAX,tbl);
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



  forceinline
  BoolVarBranch::BoolVarBranch(void)
    : s(SEL_NONE) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Select s0, BranchTbl t)
    : VarBranch<BoolVar>(t), s(s0) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Rnd r)
    : VarBranch<BoolVar>(r), s(SEL_RND) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Select s0, double d, BranchTbl t)
    : VarBranch<BoolVar>(d,t), s(s0) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Select s0, BoolAFC a, BranchTbl t)
    : VarBranch<BoolVar>(a,t), s(s0) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Select s0, BoolAction a, BranchTbl t)
    : VarBranch<BoolVar>(a,t), s(s0) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Select s0, BoolCHB c, BranchTbl t)
    : VarBranch<BoolVar>(c,t), s(s0) {}

  forceinline
  BoolVarBranch::BoolVarBranch(Select s0, BoolBranchMerit mf, BranchTbl t)
    : VarBranch<BoolVar>(mf,t), s(s0) {}

  forceinline BoolVarBranch::Select
  BoolVarBranch::select(void) const {
    return s;
  }

  forceinline void
  BoolVarBranch::expand(Home home, const BoolVarArgs& x) {
    switch (select()) {
    case SEL_AFC_MIN: case SEL_AFC_MAX:
      if (!_afc)
        _afc = BoolAFC(home,x,decay());
      break;
    case SEL_ACTION_MIN: case SEL_ACTION_MAX:
      if (!_act)
        _act = BoolAction(home,x,decay());
      break;
    case SEL_CHB_MIN: case SEL_CHB_MAX:
      if (!_chb)
        _chb = BoolCHB(home,x);
      break;
    default: ;
    }
  }

  inline BoolVarBranch
  BOOL_VAR_NONE(void) {
    return BoolVarBranch(BoolVarBranch::SEL_NONE,nullptr);
  }

  inline BoolVarBranch
  BOOL_VAR_RND(Rnd r) {
    return BoolVarBranch(r);
  }

  inline BoolVarBranch
  BOOL_VAR_MERIT_MIN(BoolBranchMerit bm, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_MERIT_MIN,bm,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_MERIT_MAX(BoolBranchMerit bm, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_MERIT_MAX,bm,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_DEGREE_MIN(BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_DEGREE_MIN,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_DEGREE_MAX(BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_DEGREE_MAX,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_AFC_MIN(double d, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_AFC_MIN,d,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_AFC_MIN(BoolAFC a, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_AFC_MIN,a,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_AFC_MAX(double d, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_AFC_MAX,d,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_AFC_MAX(BoolAFC a, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_AFC_MAX,a,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_ACTION_MIN(double d, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_ACTION_MIN,d,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_ACTION_MIN(BoolAction a, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_ACTION_MIN,a,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_ACTION_MAX(double d, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_ACTION_MAX,d,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_ACTION_MAX(BoolAction a, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_ACTION_MAX,a,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_CHB_MIN(BoolCHB c, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_CHB_MIN,c,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_CHB_MIN(BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_CHB_MIN,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_CHB_MAX(BoolCHB c, BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_CHB_MAX,c,tbl);
  }

  inline BoolVarBranch
  BOOL_VAR_CHB_MAX(BranchTbl tbl) {
    return BoolVarBranch(BoolVarBranch::SEL_CHB_MAX,tbl);
  }

}

// STATISTICS: int-branch
