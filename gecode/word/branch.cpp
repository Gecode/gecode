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
    : ValSel<WordView,WordValue>(home,vb) {}
  ValSelLsb::ValSelLsb(Space& home, ValSelLsb& vs)
    : ValSel<WordView,WordValue>(home,vs) {}
  WordValue
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
    : ValSel<WordView,WordValue>(home,vb) {}
  ValSelMsb::ValSelMsb(Space& home, ValSelMsb& vs)
    : ValSel<WordView,WordValue>(home,vs) {}
  WordValue
  ValSelMsb::val(const Space&, WordView x, int) {
    WordValue u = x.unknown();
    unsigned int bit = 0;
    while (u >>= 1)
      bit++;
    return bit;
  }

  ValSelRnd::ValSelRnd(Space& home, const ValBranch<Var>& vb)
    : ValSel<WordView,WordValue>(home,vb), r(vb.rnd()) {}
  ValSelRnd::ValSelRnd(Space& home, ValSelRnd& vs)
    : ValSel<WordView,WordValue>(home,vs), r(vs.r) {}
  WordValue
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
    : ValCommit<WordView,WordValue>(home,vb) {}
  ValCommitZero::ValCommitZero(Space& home, ValCommitZero& vc)
    : ValCommit<WordView,WordValue>(home,vc) {}
  ModEvent
  ValCommitZero::commit(Space& home, unsigned int a, WordView x, int,
                        WordValue bit) {
    const WordValue m = WordValue(1) << bit;
    return (a == 0) ? x.narrow(home,x.lo(),x.hi() & ~m)
                    : x.narrow(home,x.lo() | m,x.hi());
  }
  NGL*
  ValCommitZero::ngl(Space& home, unsigned int a, WordView x,
                     WordValue bit) const {
    return (a == 0) ? new (home) ZeroNGL(home,x,bit) : nullptr;
  }
  void
  ValCommitZero::print(const Space&, unsigned int a, WordView, int i,
                       WordValue bit, std::ostream& o) const {
    o << "var[" << i << "].bit(" << bit << ")=" << ((a == 0) ? 0 : 1);
  }

  ValSelRank::ValSelRank(Space& home, const ValBranch<WordVar>& wa)
    : ValSel<WordView,WordValue>(home,wa),
      select(static_cast<const WordAssign&>(wa).select()) {}
  ValSelRank::ValSelRank(Space& home, ValSelRank& vs)
    : ValSel<WordView,WordValue>(home,vs), select(vs.select) {}
  WordValue
  ValSelRank::val(const Space&, WordView x, int) {
    assert(x.bounded());
    if (select == WordAssign::SEL_MIN)
      return x.rank_minimum();
    if (select == WordAssign::SEL_MAX)
      return x.rank_maximum();
    const WordValue minimum = x.rank_minimum();
    const WordValue maximum = x.rank_maximum();
    const WordValue middle = minimum + ((maximum-minimum) >> 1);
    WordValue ordered_lo, ordered_hi, value;
    ordered_cube(x.domain_type(),x.width(),x.lo(),x.hi(),
                 ordered_lo,ordered_hi);
    if (!cube_successor(ordered_lo,ordered_hi,middle,x.mask(),value) ||
        (value > maximum)) {
      const bool found = cube_predecessor(ordered_lo,ordered_hi,middle,
                                          x.mask(),value);
      (void) found;
      assert(found && (value >= minimum));
    }
    return value;
  }

  ValSelSplit::ValSelSplit(Space& home, const ValBranch<WordVar>& wvb)
    : ValSel<WordView,WordValue>(home,wvb) {}
  ValSelSplit::ValSelSplit(Space& home, ValSelSplit& vs)
    : ValSel<WordView,WordValue>(home,vs) {}
  WordValue
  ValSelSplit::val(const Space&, WordView x, int) {
    assert(x.bounded() && !x.assigned());
    const WordValue minimum = x.rank_minimum();
    const WordValue maximum = x.rank_maximum();
    const WordValue middle = minimum + ((maximum-minimum) >> 1);
    WordValue ordered_lo, ordered_hi, value;
    ordered_cube(x.domain_type(),x.width(),x.lo(),x.hi(),
                 ordered_lo,ordered_hi);
    bool found = cube_successor(ordered_lo,ordered_hi,middle,x.mask(),value);
    if (!found || (value >= maximum))
      found = cube_predecessor(ordered_lo,ordered_hi,middle,x.mask(),value);
    assert(found && (value >= minimum) && (value < maximum));
    return value;
  }

  RankLqNGL::RankLqNGL(Space& home, WordView x, WordValue rank)
    : ViewValNGL<WordView,WordValue,PC_WORD_BND>(home,x,rank) {}
  RankLqNGL::RankLqNGL(Space& home, RankLqNGL& ngl)
    : ViewValNGL<WordView,WordValue,PC_WORD_BND>(home,ngl) {}
  NGL* RankLqNGL::copy(Space& home) {
    return new (home) RankLqNGL(home,*this);
  }
  NGL::Status RankLqNGL::status(const Space&) const {
    if (x.rank_maximum() <= n)
      return NGL::SUBSUMED;
    return (x.rank_minimum() > n) ? NGL::FAILED : NGL::NONE;
  }
  ExecStatus RankLqNGL::prune(Space& home) {
    return me_failed(x.narrow_rank_range(home,n+1,x.rank_maximum()))
      ? ES_FAILED : ES_OK;
  }

  RankGrNGL::RankGrNGL(Space& home, WordView x, WordValue rank)
    : ViewValNGL<WordView,WordValue,PC_WORD_BND>(home,x,rank) {}
  RankGrNGL::RankGrNGL(Space& home, RankGrNGL& ngl)
    : ViewValNGL<WordView,WordValue,PC_WORD_BND>(home,ngl) {}
  NGL* RankGrNGL::copy(Space& home) {
    return new (home) RankGrNGL(home,*this);
  }
  NGL::Status RankGrNGL::status(const Space&) const {
    if (x.rank_minimum() > n)
      return NGL::SUBSUMED;
    return (x.rank_maximum() <= n) ? NGL::FAILED : NGL::NONE;
  }
  ExecStatus RankGrNGL::prune(Space& home) {
    return me_failed(x.narrow_rank_range(home,x.rank_minimum(),n))
      ? ES_FAILED : ES_OK;
  }

  ValCommitSplit::ValCommitSplit(Space& home,
                                 const ValBranch<WordVar>& wvb)
    : ValCommit<WordView,WordValue>(home,wvb),
      lower_first(static_cast<const WordValBranch&>(wvb).select() ==
                  WordValBranch::SEL_SPLIT_MIN) {}
  ValCommitSplit::ValCommitSplit(Space& home, ValCommitSplit& vc)
    : ValCommit<WordView,WordValue>(home,vc),
      lower_first(vc.lower_first) {}
  ModEvent
  ValCommitSplit::commit(Space& home, unsigned int a, WordView x, int,
                         WordValue rank) {
    const bool lower = (a == 0) == lower_first;
    return lower
      ? x.narrow_rank_range(home,x.rank_minimum(),rank)
      : x.narrow_rank_range(home,rank+1,x.rank_maximum());
  }
  NGL*
  ValCommitSplit::ngl(Space& home, unsigned int a, WordView x,
                      WordValue rank) const {
    if (a != 0)
      return nullptr;
    return lower_first
      ? static_cast<NGL*>(new (home) RankLqNGL(home,x,rank))
      : static_cast<NGL*>(new (home) RankGrNGL(home,x,rank));
  }
  void
  ValCommitSplit::print(const Space&, unsigned int a, WordView, int i,
                        WordValue rank, std::ostream& o) const {
    const bool lower = (a == 0) == lower_first;
    o << "var[" << i << "].rank" << (lower ? "<=" : ">") << rank;
  }

  ValCommitRank::ValCommitRank(Space& home, const ValBranch<WordVar>& wa)
    : ValCommit<WordView,WordValue>(home,wa) {}
  ValCommitRank::ValCommitRank(Space& home, ValCommitRank& vc)
    : ValCommit<WordView,WordValue>(home,vc) {}
  ModEvent
  ValCommitRank::commit(Space& home, unsigned int, WordView x, int,
                        WordValue rank) {
    return x.eq(home,x.encode_rank(rank));
  }
  NGL*
  ValCommitRank::ngl(Space&, unsigned int, WordView, WordValue) const {
    return nullptr;
  }
  void
  ValCommitRank::print(const Space&, unsigned int, WordView, int i,
                       WordValue rank, std::ostream& o) const {
    o << "var[" << i << "].rank=" << rank;
  }

  template<class ValSel, class BranchDescription>
  static ValSelCommitBase<WordView,WordValue>*
  make_valselcommit(Space& home, const BranchDescription& bd) {
    return new (home) ValSelCommit<ValSel,ValCommitZero>(home,bd);
  }

  ValSelCommitBase<WordView,WordValue>*
  valselcommit(Space& home, const WordValBranch& wvb) {
    switch (wvb.select()) {
    case WordValBranch::SEL_LSB:
      return make_valselcommit<ValSelLsb>(home,wvb);
    case WordValBranch::SEL_MSB:
      return make_valselcommit<ValSelMsb>(home,wvb);
    case WordValBranch::SEL_RND:
      return make_valselcommit<ValSelRnd>(home,wvb);
    case WordValBranch::SEL_SPLIT_MIN:
    case WordValBranch::SEL_SPLIT_MAX:
      return new (home)
        ValSelCommit<ValSelSplit,ValCommitSplit>(home,wvb);
    case WordValBranch::SEL_VAL_COMMIT:
      if (!wvb.commit())
        return new (home)
          ValSelCommit<ValSelFunction<WordView>,ValCommitZero>(home,wvb);
      return new (home)
        ValSelCommit<ValSelFunction<WordView>,ValCommitFunction<WordView> >
          (home,wvb);
    default:
      throw UnknownBranching("Word::branch");
    }
  }

  ValSelCommitBase<WordView,WordValue>*
  valselcommit(Space& home, const WordAssign& wa) {
    switch (wa.select()) {
    case WordAssign::SEL_LSB:
      return make_valselcommit<ValSelLsb>(home,wa);
    case WordAssign::SEL_MSB:
      return make_valselcommit<ValSelMsb>(home,wa);
    case WordAssign::SEL_RND:
      return make_valselcommit<ValSelRnd>(home,wa);
    case WordAssign::SEL_MIN:
    case WordAssign::SEL_MED:
    case WordAssign::SEL_MAX:
      return new (home)
        ValSelCommit<ValSelRank,ValCommitRank>(home,wa);
    case WordAssign::SEL_VAL_COMMIT:
      if (!wa.commit())
        return new (home)
          ValSelCommit<ValSelFunction<WordView>,ValCommitZero>(home,wa);
      return new (home)
        ValSelCommit<ValSelFunction<WordView>,ValCommitFunction<WordView> >
          (home,wa);
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
    if ((vals.select() == WordValBranch::SEL_SPLIT_MIN) ||
        (vals.select() == WordValBranch::SEL_SPLIT_MAX))
      for (int i=0; i<xv.size(); i++)
        if (!xv[i].assigned() && !xv[i].bounded())
          throw UnknownBranching("Word::branch");
    ViewSel<WordView>* vs[1] = { Branch::viewsel(home,vars) };
    postviewvalbrancher<WordView,1,WordValue,2>
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
    if ((vals.select() == WordAssign::SEL_MIN) ||
        (vals.select() == WordAssign::SEL_MED) ||
        (vals.select() == WordAssign::SEL_MAX))
      for (int i=0; i<xv.size(); i++)
        if (!xv[i].assigned() && !xv[i].bounded())
          throw UnknownBranching("Word::assign");
    ViewSel<WordView>* vs[1] = { Branch::viewsel(home,vars) };
    postviewvalbrancher<WordView,1,WordValue,1>
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
