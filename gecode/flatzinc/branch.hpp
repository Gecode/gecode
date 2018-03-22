/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

namespace Gecode { namespace FlatZinc {

  forceinline
  IntBoolVarBranch::IntBoolVarBranch(Select s0, double d)
    : VarBranch<IntVar>(d), s(s0) {}

  forceinline
  IntBoolVarBranch::IntBoolVarBranch(Select s0, IntAFC i, BoolAFC b)
    : s(s0), iafc(i), bafc(b) {}

  forceinline
  IntBoolVarBranch::IntBoolVarBranch(Select s0, IntAction i, BoolAction b)
    : s(s0), iaction(i), baction(b) {}

  forceinline
  IntBoolVarBranch::IntBoolVarBranch(Select s0, IntCHB i, BoolCHB b)
    : s(s0), ichb(i), bchb(b) {}

  forceinline IntBoolVarBranch::Select
  IntBoolVarBranch::select(void) const {
    return s;
  }

  forceinline IntAFC
  IntBoolVarBranch::intafc(void) const {
    return iafc;
  }
  forceinline BoolAFC
  IntBoolVarBranch::boolafc(void) const {
    return bafc;
  }

  forceinline IntAction
  IntBoolVarBranch::intaction(void) const {
    return iaction;
  }
  forceinline BoolAction
  IntBoolVarBranch::boolaction(void) const {
    return baction;
  }

  forceinline IntCHB
  IntBoolVarBranch::intchb(void) const {
    return ichb;
  }
  forceinline BoolCHB
  IntBoolVarBranch::boolchb(void) const {
    return bchb;
  }
  forceinline void
  IntBoolVarBranch::expand(Home home, const IntVarArgs& x, const BoolVarArgs& y) {
    switch (select()) {
    case IntBoolVarBranch::SEL_AFC_MAX:
    case IntBoolVarBranch::SEL_AFC_SIZE_MAX:
      if (!iafc)
        iafc = IntAFC(home,x,decay());
      if (!bafc)
        bafc = BoolAFC(home,y,decay());
      break;
    case IntBoolVarBranch::SEL_ACTION_MAX:
    case IntBoolVarBranch::SEL_ACTION_SIZE_MAX:
      if (!iaction)
        iaction = IntAction(home,x,decay());
      if (!baction)
        baction = BoolAction(home,y,decay());
      break;
    case IntBoolVarBranch::SEL_CHB_MAX:
    case IntBoolVarBranch::SEL_CHB_SIZE_MAX:
      if (!ichb)
        ichb = IntCHB(home,x);
      if (!bchb)
        bchb = BoolCHB(home,y);
      break;
    default: ;
    }
  }



  inline IntBoolVarBranch
  INTBOOL_VAR_AFC_MAX(double d) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_MAX,d);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_AFC_MAX(IntAFC ia, BoolAFC ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_ACTION_MAX(double d) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_MAX,d);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_ACTION_MAX(IntAction ia, BoolAction ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_CHB_MAX(double d) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_MAX,d);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_CHB_MAX(IntCHB ic, BoolCHB bc) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_MAX,ic,bc);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_AFC_SIZE_MAX(double d) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_SIZE_MAX,d);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_AFC_SIZE_MAX(IntAFC ia, BoolAFC ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_SIZE_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_ACTION_SIZE_MAX(double d) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_SIZE_MAX,d);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_ACTION_SIZE_MAX(IntAction ia, BoolAction ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_SIZE_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_CHB_SIZE_MAX(double d) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_SIZE_MAX,d);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_CHB_SIZE_MAX(IntCHB ic, BoolCHB bc) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_SIZE_MAX,ic,bc);
  }



  forceinline
  MeritMaxAFC::MeritMaxAFC(Space&, const IntBoolVarBranch& ibvb)
    : iafc(ibvb.intafc()), bafc(ibvb.boolafc()) {}
  forceinline
  MeritMaxAFC::MeritMaxAFC(Space&, MeritMaxAFC& m)
    : iafc(m.iafc), bafc(m.bafc) {}
  forceinline double
  MeritMaxAFC::operator ()(Int::IntView x, int) const {
    return x.afc();
  }
  forceinline double
  MeritMaxAFC::operator ()(Int::BoolView x, int) const {
    return x.afc();
  }
  forceinline void
  MeritMaxAFC::dispose(void) {
    iafc.~IntAFC();
    bafc.~BoolAFC();
  }


  forceinline
  MeritMaxAFCSize::MeritMaxAFCSize(Space&, const IntBoolVarBranch& ibvb)
    : iafc(ibvb.intafc()), bafc(ibvb.boolafc()) {}
  forceinline
  MeritMaxAFCSize::MeritMaxAFCSize(Space&, MeritMaxAFCSize& m)
    : iafc(m.iafc), bafc(m.bafc) {}
  forceinline double
  MeritMaxAFCSize::operator ()(Int::IntView x, int) const {
    return x.afc() / x.size();
  }
  forceinline double
  MeritMaxAFCSize::operator ()(Int::BoolView x, int) const {
    return x.afc() / 2.0;
  }
  forceinline void
  MeritMaxAFCSize::dispose(void) {
    iafc.~IntAFC();
    bafc.~BoolAFC();
  }


  forceinline
  MeritMaxAction::MeritMaxAction(Space&, const IntBoolVarBranch& ibvb)
    : iaction(ibvb.intaction()), baction(ibvb.boolaction()) {}
  forceinline
  MeritMaxAction::MeritMaxAction(Space&, MeritMaxAction& m)
    : iaction(m.iaction), baction(m.baction) {}
  forceinline double
  MeritMaxAction::operator ()(Int::IntView, int i) const {
    return iaction[i];
  }
  forceinline double
  MeritMaxAction::operator ()(Int::BoolView, int i) const {
    return baction[i];
  }
  forceinline void
  MeritMaxAction::dispose(void) {
    iaction.~IntAction();
    baction.~BoolAction();
  }


  forceinline
  MeritMaxActionSize::MeritMaxActionSize(Space&, const IntBoolVarBranch& ibvb)
    : iaction(ibvb.intaction()), baction(ibvb.boolaction()) {}
  forceinline
  MeritMaxActionSize::MeritMaxActionSize(Space&, MeritMaxActionSize& m)
    : iaction(m.iaction), baction(m.baction) {}
  forceinline double
  MeritMaxActionSize::operator ()(Int::IntView x, int i) const {
    return iaction[i] / x.size();
  }
  forceinline double
  MeritMaxActionSize::operator ()(Int::BoolView, int i) const {
    return baction[i] / 2.0;
  }
  forceinline void
  MeritMaxActionSize::dispose(void) {
    iaction.~IntAction();
    baction.~BoolAction();
  }


  forceinline
  MeritMaxCHB::MeritMaxCHB(Space&, const IntBoolVarBranch& ibvb)
    : ichb(ibvb.intchb()), bchb(ibvb.boolchb()) {}
  forceinline
  MeritMaxCHB::MeritMaxCHB(Space&, MeritMaxCHB& m)
    : ichb(m.ichb), bchb(m.bchb) {}
  forceinline double
  MeritMaxCHB::operator ()(Int::IntView, int i) const {
    return ichb[i];
  }
  forceinline double
  MeritMaxCHB::operator ()(Int::BoolView, int i) const {
    return bchb[i];
  }
  forceinline void
  MeritMaxCHB::dispose(void) {
    ichb.~IntCHB();
    bchb.~BoolCHB();
  }


  forceinline
  MeritMaxCHBSize::MeritMaxCHBSize(Space&, const IntBoolVarBranch& ibvb)
    : ichb(ibvb.intchb()), bchb(ibvb.boolchb()) {}
  forceinline
  MeritMaxCHBSize::MeritMaxCHBSize(Space&, MeritMaxCHBSize& m)
    : ichb(m.ichb), bchb(m.bchb) {}
  forceinline double
  MeritMaxCHBSize::operator ()(Int::IntView x, int i) const {
    return ichb[i] / x.size();
  }
  forceinline double
  MeritMaxCHBSize::operator ()(Int::BoolView, int i) const {
    return bchb[i] / 2.0;
  }
  forceinline void
  MeritMaxCHBSize::dispose(void) {
    ichb.~IntCHB();
    bchb.~BoolCHB();
  }


  forceinline
  PosIntChoice::PosIntChoice(const Brancher& b, unsigned int a, int p, int n)
    : Choice(b,a), _pos(p), _val(n) {}
  forceinline int
  PosIntChoice::pos(void) const {
    return _pos;
  }
  forceinline int
  PosIntChoice::val(void) const {
    return _val;
  }


  forceinline
  IntBoolBrancherBase::
  IntBoolBrancherBase(Home home,
                  ViewArray<Int::IntView> x0,
                  ViewArray<Int::BoolView> y0,
                  ValSelCommitBase<Int::IntView,int>* xvsc0,
                  ValSelCommitBase<Int::BoolView,int>* yvsc0)
    : Brancher(home), x(x0), y(y0), start(0), xvsc(xvsc0), yvsc(yvsc0) {
    home.notice(*this,AP_DISPOSE,true);
  }

  forceinline
  IntBoolBrancherBase::
  IntBoolBrancherBase(Space& home, IntBoolBrancherBase& b)
    : Brancher(home,b), start(b.start),
      xvsc(b.xvsc->copy(home)), yvsc(b.yvsc->copy(home)) {
    x.update(home,b.x);
    y.update(home,b.y);
  }

  forceinline size_t
  IntBoolBrancherBase::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE,true);
    xvsc->dispose(home);
    yvsc->dispose(home);
    (void) Brancher::dispose(home);
    return sizeof(IntBoolBrancherBase);
  }


  template<class Merit>
  forceinline
  IntBoolBrancher<Merit>::
  IntBoolBrancher(Home home,
                  ViewArray<Int::IntView> x,
                  ViewArray<Int::BoolView> y,
                  Merit& m,
                  ValSelCommitBase<Int::IntView,int>* xvsc,
                  ValSelCommitBase<Int::BoolView,int>* yvsc)
    : IntBoolBrancherBase(home,x,y,xvsc,yvsc), merit(m) {}

  template<class Merit>
  forceinline void
  IntBoolBrancher<Merit>::
  post(Home home,
       ViewArray<Int::IntView> x,
       ViewArray<Int::BoolView> y,
       Merit& m,
       ValSelCommitBase<Int::IntView,int>* xvsc,
       ValSelCommitBase<Int::BoolView,int>* yvsc) {
    (void) new (home) IntBoolBrancher<Merit>(home, x, y, m, xvsc, yvsc);
  }

  template<class Merit>
  forceinline
  IntBoolBrancher<Merit>::
  IntBoolBrancher(Space& home, IntBoolBrancher<Merit>& b)
    : IntBoolBrancherBase(home,b), merit(home, b.merit) {
  }

  template<class Merit>
  Actor*
  IntBoolBrancher<Merit>::copy(Space& home) {
    return new (home) IntBoolBrancher<Merit>(home,*this);
  }

  template<class Merit>
  const Choice*
  IntBoolBrancher<Merit>::choice(Space& home) {
    int p = start;
    double m;
    if (p < x.size()) {
      assert(!x[p].assigned());
      m=merit(x[p],p);
      for (int i=p+1; i<x.size(); i++)
        if (!x[i].assigned()) {
          double mi = merit(x[i],i);
          if (mi > m) {
            m=mi; p=i;
          }
        }
      for (int i=0; i<y.size(); i++)
        if (!y[i].assigned()) {
          double mi = merit(y[i],i);
          if (mi > m) {
            m=mi; p=i+x.size();
          }
        }
    } else {
      assert(!y[p-x.size()].assigned());
      m=merit(y[p-x.size()],p-x.size());
      for (int i=p-x.size()+1; i<y.size(); i++)
        if (!y[i].assigned()) {
          double mi = merit(y[i],i);
          if (mi > m) {
            m=mi; p=i+x.size();
          }
        }
    }
    int v;
    if (p < x.size()) {
      v=xvsc->val(home,x[p],p);
    } else {
      v=yvsc->val(home,y[p-x.size()],p-x.size());
    }
    return new PosIntChoice(*this,2,p,v);
  }

  template<class Merit>
  size_t
  IntBoolBrancher<Merit>::dispose(Space& home) {
    merit.dispose();
    (void) IntBoolBrancherBase::dispose(home);
    return sizeof(IntBoolBrancher<Merit>);
  }


  forceinline BoolValBranch
  i2b(const IntValBranch& ivb) {
    switch (ivb.select()) {
    case IntValBranch::SEL_MIN:
    case IntValBranch::SEL_MED:
    case IntValBranch::SEL_SPLIT_MIN:
    case IntValBranch::SEL_RANGE_MIN:
      return BOOL_VAL_MIN();
    case IntValBranch::SEL_MAX:
    case IntValBranch::SEL_SPLIT_MAX:
    case IntValBranch::SEL_RANGE_MAX:
      return BOOL_VAL_MAX();
    case IntValBranch::SEL_RND:
      return BOOL_VAL_RND(ivb.rnd());
    case IntValBranch::SEL_VAL_COMMIT:
    default:
    GECODE_NEVER;
    }
    GECODE_NEVER;
    return BOOL_VAL_MIN();
  }

}}

// STATISTICS: flatzinc-branch

