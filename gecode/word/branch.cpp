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

#include <gecode/word/branch.hh>

namespace Gecode { namespace Word { namespace Branch {

  MeritSize::MeritSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<WordView,unsigned int>(home,vb) {}
  MeritSize::MeritSize(Space& home, MeritSize& m)
    : MeritBase<WordView,unsigned int>(home,m) {}
  unsigned int
  MeritSize::operator ()(const Space&, WordView x, int) {
    return x.unknown_size();
  }

  MeritDegreeSize::MeritDegreeSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<WordView,double>(home,vb) {}
  MeritDegreeSize::MeritDegreeSize(Space& home, MeritDegreeSize& m)
    : MeritBase<WordView,double>(home,m) {}
  double
  MeritDegreeSize::operator ()(const Space&, WordView x, int) {
    return static_cast<double>(x.unknown_size()) /
      static_cast<double>(x.degree());
  }

  MeritAFCSize::MeritAFCSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<WordView,double>(home,vb), afc(vb.afc()) {}
  MeritAFCSize::MeritAFCSize(Space& home, MeritAFCSize& m)
    : MeritBase<WordView,double>(home,m), afc(m.afc) {}
  double
  MeritAFCSize::operator ()(const Space&, WordView x, int) {
    return x.afc() / static_cast<double>(x.unknown_size());
  }
  bool MeritAFCSize::notice(void) const { return false; }
  void MeritAFCSize::dispose(Space&) { afc.~AFC(); }

  MeritActionSize::MeritActionSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<WordView,double>(home,vb), action(vb.action()) {}
  MeritActionSize::MeritActionSize(Space& home, MeritActionSize& m)
    : MeritBase<WordView,double>(home,m), action(m.action) {}
  double
  MeritActionSize::operator ()(const Space&, WordView x, int i) {
    return action[i] / static_cast<double>(x.unknown_size());
  }
  bool MeritActionSize::notice(void) const { return true; }
  void MeritActionSize::dispose(Space&) { action.~Action(); }

  MeritCHBSize::MeritCHBSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<WordView,double>(home,vb), chb(vb.chb()) {}
  MeritCHBSize::MeritCHBSize(Space& home, MeritCHBSize& m)
    : MeritBase<WordView,double>(home,m), chb(m.chb) {}
  double
  MeritCHBSize::operator ()(const Space&, WordView x, int i) {
    return chb[i] / static_cast<double>(x.unknown_size());
  }
  bool MeritCHBSize::notice(void) const { return true; }
  void MeritCHBSize::dispose(Space&) { chb.~CHB(); }

  template<template<class> class Selector, class Merit>
  static ViewSel<WordView>*
  select_merit(Space& home, const WordVarBranch& wvb) {
    return new (home) Selector<Merit>(home,wvb);
  }

  ViewSel<WordView>*
  viewsel(Space& home, const WordVarBranch& wvb) {
    switch (wvb.select()) {
    case WordVarBranch::SEL_NONE:
      return new (home) ViewSelNone<WordView>(home,wvb);
    case WordVarBranch::SEL_RND:
      return new (home) ViewSelRnd<WordView>(home,wvb);
    default: break;
    }
#define GECODE_WORD_VIEWSEL_CASE(Select, Merit) \
    case WordVarBranch::Select##_MIN: \
      return select_merit<ViewSelMin,Merit>(home,wvb); \
    case WordVarBranch::Select##_MAX: \
      return select_merit<ViewSelMax,Merit>(home,wvb)
    if (wvb.tbl() == nullptr) {
      switch (wvb.select()) {
      GECODE_WORD_VIEWSEL_CASE(SEL_MERIT,MeritFunction<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_DEGREE,MeritDegree<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_AFC,MeritAFC<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_ACTION,MeritAction<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_CHB,MeritCHB<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_SIZE,MeritSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_DEGREE_SIZE,MeritDegreeSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_AFC_SIZE,MeritAFCSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_ACTION_SIZE,MeritActionSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_CHB_SIZE,MeritCHBSize);
      default: throw UnknownBranching("Word::branch");
      }
    } else {
#undef GECODE_WORD_VIEWSEL_CASE
#define GECODE_WORD_VIEWSEL_CASE(Select, Merit) \
    case WordVarBranch::Select##_MIN: \
      return new (home) ViewSelMinTbl<Merit>(home,wvb); \
    case WordVarBranch::Select##_MAX: \
      return new (home) ViewSelMaxTbl<Merit>(home,wvb)
      switch (wvb.select()) {
      GECODE_WORD_VIEWSEL_CASE(SEL_MERIT,MeritFunction<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_DEGREE,MeritDegree<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_AFC,MeritAFC<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_ACTION,MeritAction<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_CHB,MeritCHB<WordView>);
      GECODE_WORD_VIEWSEL_CASE(SEL_SIZE,MeritSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_DEGREE_SIZE,MeritDegreeSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_AFC_SIZE,MeritAFCSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_ACTION_SIZE,MeritActionSize);
      GECODE_WORD_VIEWSEL_CASE(SEL_CHB_SIZE,MeritCHBSize);
      default: throw UnknownBranching("Word::branch");
      }
    }
#undef GECODE_WORD_VIEWSEL_CASE
  }

  ValSelLsb::ValSelLsb(Space& home, const ValBranch<Var>& vb)
    : ValSel<WordView,unsigned int>(home,vb) {}
  ValSelLsb::ValSelLsb(Space& home, ValSelLsb& vs)
    : ValSel<WordView,unsigned int>(home,vs) {}
  unsigned int
  ValSelLsb::val(const Space&, WordView x, int) {
    WordValue u = x.unknown();
    unsigned int bit = 0;
    while ((u & 1U) == 0) {
      u >>= 1;
      bit++;
    }
    return bit;
  }

  ValSelMsb::ValSelMsb(Space& home, const ValBranch<Var>& vb)
    : ValSel<WordView,unsigned int>(home,vb) {}
  ValSelMsb::ValSelMsb(Space& home, ValSelMsb& vs)
    : ValSel<WordView,unsigned int>(home,vs) {}
  unsigned int
  ValSelMsb::val(const Space&, WordView x, int) {
    WordValue u = x.unknown();
    unsigned int bit = 0;
    while (u >>= 1)
      bit++;
    return bit;
  }

  ValSelRnd::ValSelRnd(Space& home, const ValBranch<Var>& vb)
    : ValSel<WordView,unsigned int>(home,vb), r(vb.rnd()) {}
  ValSelRnd::ValSelRnd(Space& home, ValSelRnd& vs)
    : ValSel<WordView,unsigned int>(home,vs), r(vs.r) {}
  unsigned int
  ValSelRnd::val(const Space&, WordView x, int) {
    unsigned int n = r(x.unknown_size());
    WordValue u = x.unknown();
    for (unsigned int bit=0; bit<x.width(); bit++) {
      if ((u & (WordValue(1) << bit)) != 0) {
        if (n == 0)
          return bit;
        n--;
      }
    }
    GECODE_NEVER;
    return 0;
  }
  bool ValSelRnd::notice(void) const { return true; }
  void ValSelRnd::dispose(Space&) { r.~Rnd(); }

  ZeroNGL::ZeroNGL(Space& home, WordView x, unsigned int bit)
    : ViewValNGL<WordView,unsigned int,PC_WORD_BITS>(home,x,bit) {}
  ZeroNGL::ZeroNGL(Space& home, ZeroNGL& ngl)
    : ViewValNGL<WordView,unsigned int,PC_WORD_BITS>(home,ngl) {}
  NGL*
  ZeroNGL::copy(Space& home) {
    return new (home) ZeroNGL(home,*this);
  }
  NGL::Status
  ZeroNGL::status(const Space&) const {
    const WordValue m = WordValue(1) << n;
    if ((x.hi() & m) == 0)
      return NGL::SUBSUMED;
    return ((x.lo() & m) != 0) ? NGL::FAILED : NGL::NONE;
  }
  ExecStatus
  ZeroNGL::prune(Space& home) {
    const WordValue m = WordValue(1) << n;
    return me_failed(x.narrow(home,x.lo() | m,x.hi())) ? ES_FAILED : ES_OK;
  }

  ValCommitZero::ValCommitZero(Space& home, const ValBranch<Var>& vb)
    : ValCommit<WordView,unsigned int>(home,vb) {}
  ValCommitZero::ValCommitZero(Space& home, ValCommitZero& vc)
    : ValCommit<WordView,unsigned int>(home,vc) {}
  ModEvent
  ValCommitZero::commit(Space& home, unsigned int a, WordView x, int,
                        unsigned int bit) {
    const WordValue m = WordValue(1) << bit;
    return (a == 0) ? x.narrow(home,x.lo(),x.hi() & ~m)
                    : x.narrow(home,x.lo() | m,x.hi());
  }
  NGL*
  ValCommitZero::ngl(Space& home, unsigned int a, WordView x,
                     unsigned int bit) const {
    return (a == 0) ? new (home) ZeroNGL(home,x,bit) : nullptr;
  }
  void
  ValCommitZero::print(const Space&, unsigned int a, WordView, int i,
                       unsigned int bit, std::ostream& o) const {
    o << "var[" << i << "].bit(" << bit << ")=" << ((a == 0) ? 0 : 1);
  }

  template<class ValSel, class BranchDescription>
  static ValSelCommitBase<WordView,unsigned int>*
  make_valselcommit(Space& home, const BranchDescription& bd) {
    return new (home) ValSelCommit<ValSel,ValCommitZero>(home,bd);
  }

  ValSelCommitBase<WordView,unsigned int>*
  valselcommit(Space& home, const WordValBranch& wvb) {
    switch (wvb.select()) {
    case WordValBranch::SEL_LSB:
      return make_valselcommit<ValSelLsb>(home,wvb);
    case WordValBranch::SEL_MSB:
      return make_valselcommit<ValSelMsb>(home,wvb);
    case WordValBranch::SEL_RND:
      return make_valselcommit<ValSelRnd>(home,wvb);
    default:
      throw UnknownBranching("Word::branch");
    }
  }

  ValSelCommitBase<WordView,unsigned int>*
  valselcommit(Space& home, const WordAssign& wa) {
    switch (wa.select()) {
    case WordAssign::SEL_LSB:
      return make_valselcommit<ValSelLsb>(home,wa);
    case WordAssign::SEL_MSB:
      return make_valselcommit<ValSelMsb>(home,wa);
    case WordAssign::SEL_RND:
      return make_valselcommit<ValSelRnd>(home,wa);
    default:
      throw UnknownBranching("Word::assign");
    }
  }

}}}

namespace Gecode {

  WordAction::WordAction(Home home, const WordVarArgs& x, double d,
                         bool p, bool f, WordBranchMerit bm) {
    ViewArray<Word::WordView> y(home,x);
    Action::init(home,y,d,p,f,bm);
  }
  void
  WordAction::init(Home home, const WordVarArgs& x, double d,
                   bool p, bool f, WordBranchMerit bm) {
    ViewArray<Word::WordView> y(home,x);
    Action::init(home,y,d,p,f,bm);
  }
  WordCHB::WordCHB(Home home, const WordVarArgs& x, WordBranchMerit bm) {
    ViewArray<Word::WordView> y(home,x);
    CHB::init(home,y,bm);
  }
  void
  WordCHB::init(Home home, const WordVarArgs& x, WordBranchMerit bm) {
    ViewArray<Word::WordView> y(home,x);
    CHB::init(home,y,bm);
  }

  void
  branch(Home home, const WordVarArgs& x, WordVarBranch vars,
         WordValBranch vals, WordBranchFilter bf, WordVarValPrint vvp) {
    using namespace Word;
    if (home.failed()) return;
    vars.expand(home,x);
    ViewArray<WordView> xv(home,x);
    ViewSel<WordView>* vs[1] = { Branch::viewsel(home,vars) };
    postviewvalbrancher<WordView,1,unsigned int,2>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  branch(Home home, const WordVarArgs& x, WordValBranch vals) {
    branch(home,x,WORD_VAR_NONE(),vals,nullptr,nullptr);
  }

  void
  branch(Home home, WordVar x, WordValBranch vals) {
    WordVarArgs xv(1); xv[0] = x;
    branch(home,xv,vals);
  }

  void
  assign(Home home, const WordVarArgs& x, WordVarBranch vars,
         WordAssign vals, WordBranchFilter bf, WordVarValPrint vvp) {
    using namespace Word;
    if (home.failed()) return;
    vars.expand(home,x);
    ViewArray<WordView> xv(home,x);
    ViewSel<WordView>* vs[1] = { Branch::viewsel(home,vars) };
    postviewvalbrancher<WordView,1,unsigned int,1>
      (home,xv,vs,Branch::valselcommit(home,vals),bf,vvp);
  }

  void
  assign(Home home, const WordVarArgs& x, WordAssign vals) {
    assign(home,x,WORD_VAR_NONE(),vals,nullptr,nullptr);
  }

  void
  assign(Home home, WordVar x, WordAssign vals) {
    WordVarArgs xv(1); xv[0] = x;
    assign(home,xv,vals);
  }

}

// STATISTICS: word-post
