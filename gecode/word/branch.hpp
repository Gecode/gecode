/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

#ifndef GECODE_WORD_BRANCH_HPP
#define GECODE_WORD_BRANCH_HPP

namespace Gecode {

  forceinline WordAFC::WordAFC(void) {}
  forceinline WordAFC::WordAFC(const WordAFC& a) : AFC(a) {}
  forceinline WordAFC& WordAFC::operator =(const WordAFC& a) {
    return static_cast<WordAFC&>(AFC::operator =(a));
  }
  forceinline
  WordAFC::WordAFC(Home home, const WordVarArgs& x, double d, bool share) {
    AFC::init(home,x,d,share);
  }
  forceinline void
  WordAFC::init(Home home, const WordVarArgs& x, double d, bool share) {
    AFC::init(home,x,d,share);
  }

  forceinline WordAction::WordAction(void) {}
  forceinline WordAction::WordAction(const WordAction& a) : Action(a) {}
  forceinline WordAction& WordAction::operator =(const WordAction& a) {
    return static_cast<WordAction&>(Action::operator =(a));
  }

  forceinline WordCHB::WordCHB(void) {}
  forceinline WordCHB::WordCHB(const WordCHB& c) : CHB(c) {}
  forceinline WordCHB& WordCHB::operator =(const WordCHB& c) {
    return static_cast<WordCHB&>(CHB::operator =(c));
  }

  forceinline WordVarBranch::WordVarBranch(void) : s(SEL_NONE) {}
  forceinline WordVarBranch::WordVarBranch(Rnd r)
    : VarBranch<WordVar>(r), s(SEL_RND) {}
  forceinline WordVarBranch::WordVarBranch(Select s0, BranchTbl t)
    : VarBranch<WordVar>(t), s(s0) {}
  forceinline WordVarBranch::WordVarBranch(Select s0, double d, BranchTbl t)
    : VarBranch<WordVar>(d,t), s(s0) {}
  forceinline WordVarBranch::WordVarBranch(Select s0, WordAFC a, BranchTbl t)
    : VarBranch<WordVar>(a,t), s(s0) {}
  forceinline WordVarBranch::WordVarBranch(Select s0, WordAction a,
                                           BranchTbl t)
    : VarBranch<WordVar>(a,t), s(s0) {}
  forceinline WordVarBranch::WordVarBranch(Select s0, WordCHB c, BranchTbl t)
    : VarBranch<WordVar>(c,t), s(s0) {}
  forceinline WordVarBranch::WordVarBranch(Select s0, WordBranchMerit mf,
                                           BranchTbl t)
    : VarBranch<WordVar>(mf,t), s(s0) {}
  forceinline WordVarBranch::Select WordVarBranch::select(void) const {
    return s;
  }
  forceinline void
  WordVarBranch::expand(Home home, const WordVarArgs& x) {
    switch (select()) {
    case SEL_AFC_MIN: case SEL_AFC_MAX:
    case SEL_AFC_SIZE_MIN: case SEL_AFC_SIZE_MAX:
      if (!_afc) _afc = WordAFC(home,x,decay());
      break;
    case SEL_ACTION_MIN: case SEL_ACTION_MAX:
    case SEL_ACTION_SIZE_MIN: case SEL_ACTION_SIZE_MAX:
      if (!_act) _act = WordAction(home,x,decay());
      break;
    case SEL_CHB_MIN: case SEL_CHB_MAX:
    case SEL_CHB_SIZE_MIN: case SEL_CHB_SIZE_MAX:
      if (!_chb) _chb = WordCHB(home,x);
      break;
    default: ;
    }
  }

  inline WordVarBranch WORD_VAR_NONE(void) {
    return WordVarBranch(WordVarBranch::SEL_NONE,nullptr);
  }
  inline WordVarBranch WORD_VAR_RND(Rnd r) { return WordVarBranch(r); }
  inline WordVarBranch WORD_VAR_MERIT_MIN(WordBranchMerit bm, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_MERIT_MIN,bm,t);
  }
  inline WordVarBranch WORD_VAR_MERIT_MAX(WordBranchMerit bm, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_MERIT_MAX,bm,t);
  }
  inline WordVarBranch WORD_VAR_DEGREE_MIN(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_DEGREE_MIN,t);
  }
  inline WordVarBranch WORD_VAR_DEGREE_MAX(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_DEGREE_MAX,t);
  }
  inline WordVarBranch WORD_VAR_SIZE_MIN(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_SIZE_MIN,t);
  }
  inline WordVarBranch WORD_VAR_SIZE_MAX(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_SIZE_MAX,t);
  }
  inline WordVarBranch WORD_VAR_DEGREE_SIZE_MIN(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_DEGREE_SIZE_MIN,t);
  }
  inline WordVarBranch WORD_VAR_DEGREE_SIZE_MAX(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_DEGREE_SIZE_MAX,t);
  }
  inline WordVarBranch WORD_VAR_AFC_MIN(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_MIN,d,t);
  }
  inline WordVarBranch WORD_VAR_AFC_MAX(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_MAX,d,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_MIN(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_MIN,d,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_MAX(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_MAX,d,t);
  }
  inline WordVarBranch WORD_VAR_AFC_SIZE_MIN(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_SIZE_MIN,d,t);
  }
  inline WordVarBranch WORD_VAR_AFC_SIZE_MAX(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_SIZE_MAX,d,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_SIZE_MIN(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_SIZE_MIN,d,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_SIZE_MAX(double d, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_SIZE_MAX,d,t);
  }
  inline WordVarBranch WORD_VAR_AFC_MIN(WordAFC a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_MIN,a,t);
  }
  inline WordVarBranch WORD_VAR_AFC_MAX(WordAFC a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_MAX,a,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_MIN(WordAction a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_MIN,a,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_MAX(WordAction a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_MAX,a,t);
  }
  inline WordVarBranch WORD_VAR_CHB_MIN(WordCHB c, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_MIN,c,t);
  }
  inline WordVarBranch WORD_VAR_CHB_MAX(WordCHB c, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_MAX,c,t);
  }
  inline WordVarBranch WORD_VAR_AFC_SIZE_MIN(WordAFC a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_SIZE_MIN,a,t);
  }
  inline WordVarBranch WORD_VAR_AFC_SIZE_MAX(WordAFC a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_AFC_SIZE_MAX,a,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_SIZE_MIN(WordAction a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_SIZE_MIN,a,t);
  }
  inline WordVarBranch WORD_VAR_ACTION_SIZE_MAX(WordAction a, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_ACTION_SIZE_MAX,a,t);
  }
  inline WordVarBranch WORD_VAR_CHB_SIZE_MIN(WordCHB c, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_SIZE_MIN,c,t);
  }
  inline WordVarBranch WORD_VAR_CHB_SIZE_MAX(WordCHB c, BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_SIZE_MAX,c,t);
  }
  inline WordVarBranch WORD_VAR_CHB_MIN(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_MIN,t);
  }
  inline WordVarBranch WORD_VAR_CHB_MAX(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_MAX,t);
  }
  inline WordVarBranch WORD_VAR_CHB_SIZE_MIN(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_SIZE_MIN,t);
  }
  inline WordVarBranch WORD_VAR_CHB_SIZE_MAX(BranchTbl t) {
    return WordVarBranch(WordVarBranch::SEL_CHB_SIZE_MAX,t);
  }

  forceinline
  WordValBranch::WordValBranch(Select s0) : s(s0) {}
  forceinline
  WordValBranch::WordValBranch(Select s0, Rnd r)
    : ValBranch<WordVar>(r), s(s0) {}
  forceinline WordValBranch::Select
  WordValBranch::select(void) const { return s; }

  forceinline WordValBranch WORD_VAL_LSB(void) {
    return WordValBranch(WordValBranch::SEL_LSB);
  }
  forceinline WordValBranch WORD_VAL_MSB(void) {
    return WordValBranch(WordValBranch::SEL_MSB);
  }
  forceinline WordValBranch WORD_VAL_RND(Rnd r) {
    return WordValBranch(WordValBranch::SEL_RND,r);
  }
  forceinline WordValBranch WORD_VAL_SPLIT_MIN(void) {
    return WordValBranch(WordValBranch::SEL_SPLIT_MIN);
  }
  forceinline WordValBranch WORD_VAL_SPLIT_MAX(void) {
    return WordValBranch(WordValBranch::SEL_SPLIT_MAX);
  }

  forceinline
  WordAssign::WordAssign(Select s0) : s(s0) {}
  forceinline
  WordAssign::WordAssign(Select s0, Rnd r)
    : ValBranch<WordVar>(r), s(s0) {}
  forceinline WordAssign::Select
  WordAssign::select(void) const { return s; }

  forceinline WordAssign WORD_ASSIGN_LSB(void) {
    return WordAssign(WordAssign::SEL_LSB);
  }
  forceinline WordAssign WORD_ASSIGN_MSB(void) {
    return WordAssign(WordAssign::SEL_MSB);
  }
  forceinline WordAssign WORD_ASSIGN_RND(Rnd r) {
    return WordAssign(WordAssign::SEL_RND,r);
  }
  forceinline WordAssign WORD_ASSIGN_MIN(void) {
    return WordAssign(WordAssign::SEL_MIN);
  }
  forceinline WordAssign WORD_ASSIGN_MED(void) {
    return WordAssign(WordAssign::SEL_MED);
  }
  forceinline WordAssign WORD_ASSIGN_MAX(void) {
    return WordAssign(WordAssign::SEL_MAX);
  }

}

// STATISTICS: word-branch

#endif
