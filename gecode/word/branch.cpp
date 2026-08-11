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

  void
  branch(Home home, const WordVarArgs& x, WordValBranch vals) {
    using namespace Word;
    if (home.failed()) return;
    ViewArray<WordView> xv(home,x);
    ViewSel<WordView>* vs[1] = {
      new (home) ViewSelNone<WordView>(home,VarBranch<WordVar>())
    };
    postviewvalbrancher<WordView,1,unsigned int,2>
      (home,xv,vs,Branch::valselcommit(home,vals),nullptr,nullptr);
  }

  void
  branch(Home home, WordVar x, WordValBranch vals) {
    WordVarArgs xv(1); xv[0] = x;
    branch(home,xv,vals);
  }

  void
  assign(Home home, const WordVarArgs& x, WordAssign vals) {
    using namespace Word;
    if (home.failed()) return;
    ViewArray<WordView> xv(home,x);
    ViewSel<WordView>* vs[1] = {
      new (home) ViewSelNone<WordView>(home,VarBranch<WordVar>())
    };
    postviewvalbrancher<WordView,1,unsigned int,1>
      (home,xv,vs,Branch::valselcommit(home,vals),nullptr,nullptr);
  }

  void
  assign(Home home, WordVar x, WordAssign vals) {
    WordVarArgs xv(1); xv[0] = x;
    assign(home,xv,vals);
  }

}

