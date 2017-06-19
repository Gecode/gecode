/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date: 2017-05-10 14:58:42 +0200 (Wed, 10 May 2017) $ by $Author: schulte $
 *     $Revision: 15697 $
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


#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/int/branch.hh>
#include <gecode/minimodel.hh>

namespace Gecode { namespace FlatZinc {

  /// Which integer or Boolean variable to select for branching
  class IntBoolVarBranch : public VarBranch<IntVar> {
  public:
    /// Which variable selection
    enum Select {
      SEL_AFC_MAX,         ///< With largest accumulated failure count
      SEL_ACTION_MAX,      ///< With highest action
      SEL_CHB_MAX,         ///< With highest CHB Q-score
      SEL_AFC_SIZE_MAX,    ///< With largest accumulated failure count divided by domain size
      SEL_ACTION_SIZE_MAX, ///< With largest action divided by domain size
      SEL_CHB_SIZE_MAX     ///< With largest CHB Q-score divided by domain size
    };
  protected:
    /// Which variable to select
    Select s;
    /// Integer AFC
    IntAFC iafc;
    /// Boolean AFC
    BoolAFC bafc;
    /// Integer action
    IntAction iaction;
    /// Boolean action
    BoolAction baction;
    /// Integer CHB
    IntCHB ichb;
    /// Boolean CHB
    BoolCHB bchb;
  public:
    /// Initialize with selection strategy \a s and AFC \a i and \a b
    IntBoolVarBranch(Select s, IntAFC i, BoolAFC b);
    /// Initialize with selection strategy \a s and action \a i and \a b
    IntBoolVarBranch(Select s, IntAction i, BoolAction b);
    /// Initialize with selection strategy \a s and CHB \a i and \a b
    IntBoolVarBranch(Select s, IntCHB i, BoolCHB b);
    /// Return selection strategy
    Select select(void) const;
    /// Return integer AFC
    IntAFC intafc(void) const;
    /// Return Boolean AFC
    BoolAFC boolafc(void) const;
    /// Return integer action
    IntAction intaction(void) const;
    /// Return Boolean action
    BoolAction boolaction(void) const;
    /// Return integer CHB
    IntCHB intchb(void) const;
    /// Return Boolean AFC
    BoolCHB boolchb(void) const;
  };

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

  /// Variable selection for both integer and Boolean variables
  //@{
  /// Select variable with largest accumulated failure count
  IntBoolVarBranch INTBOOL_VAR_AFC_MAX(IntAFC ia, BoolAFC ba);
  /// Select variable with highest action
  IntBoolVarBranch INTBOOL_VAR_ACTION_MAX(IntAction ia, BoolAction ba);
  /// Select variable with largest CHB Q-score
  IntBoolVarBranch INTBOOL_VAR_CHB_MAX(IntCHB ic, BoolCHB bc);
  /// Select variable with largest accumulated failure count divided by domain size
  IntBoolVarBranch INTBOOL_VAR_AFC_SIZE_MAX(IntAFC ia, BoolAFC ba);
  /// Select variable with largest action divided by domain size
  IntBoolVarBranch INTBOOL_VAR_ACTION_SIZE_MAX(IntAction ia, BoolAction ba);
  /// Select variable with largest CHB Q-score divided by domain size
  IntBoolVarBranch INTBOOL_VAR_CHB_SIZE_MAX(IntCHB ic, BoolCHB bc);
  //@}
  
  inline IntBoolVarBranch
  INTBOOL_VAR_AFC_MAX(IntAFC ia, BoolAFC ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_ACTION_MAX(IntAction ia, BoolAction ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_CHB_MAX(IntCHB ic, BoolCHB bc) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_MAX,ic,bc);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_AFC_SIZE_MAX(IntAFC ia, BoolAFC ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_SIZE_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_ACTION_SIZE_MAX(IntAction ia, BoolAction ba) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_SIZE_MAX,ia,ba);
  }
  inline IntBoolVarBranch
  INTBOOL_VAR_CHB_SIZE_MAX(IntCHB ic, BoolCHB bc) {
    return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_SIZE_MAX,ic,bc);
  }

  /// Select by maximal AFC
  class MeritMaxAFC {
  protected:
    /// Integer AFC information
    IntAFC iafc;
    /// Boolean AFC information
    BoolAFC bafc;
  public:
    /// Constructor for initialization
    MeritMaxAFC(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxAFC(Space& home, MeritMaxAFC& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  forceinline
  MeritMaxAFC::MeritMaxAFC(Space& home, const IntBoolVarBranch& ibvb)
    : iafc(ibvb.intafc()), bafc(ibvb.boolafc()) {}
  forceinline
  MeritMaxAFC::MeritMaxAFC(Space& home, MeritMaxAFC& m)
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

  /// Select by maximal AFC over size
  class MeritMaxAFCSize {
  protected:
    /// Integer AFC information
    IntAFC iafc;
    /// Boolean AFC information
    BoolAFC bafc;
  public:
    /// Constructor for initialization
    MeritMaxAFCSize(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxAFCSize(Space& home, MeritMaxAFCSize& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  forceinline
  MeritMaxAFCSize::MeritMaxAFCSize(Space& home, const IntBoolVarBranch& ibvb)
    : iafc(ibvb.intafc()), bafc(ibvb.boolafc()) {}
  forceinline
  MeritMaxAFCSize::MeritMaxAFCSize(Space& home, MeritMaxAFCSize& m)
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


  /// Select by maximal Action
  class MeritMaxAction {
  protected:
    /// Integer Action information
    IntAction iaction;
    /// Boolean Action information
    BoolAction baction;
  public:
    /// Constructor for initialization
    MeritMaxAction(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxAction(Space& home, MeritMaxAction& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  forceinline
  MeritMaxAction::MeritMaxAction(Space& home, const IntBoolVarBranch& ibvb)
    : iaction(ibvb.intaction()), baction(ibvb.boolaction()) {}
  forceinline
  MeritMaxAction::MeritMaxAction(Space& home, MeritMaxAction& m)
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

  /// Select by maximal Action over size
  class MeritMaxActionSize {
  protected:
    /// Integer Action information
    IntAction iaction;
    /// Boolean Action information
    BoolAction baction;
  public:
    /// Constructor for initialization
    MeritMaxActionSize(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxActionSize(Space& home, MeritMaxActionSize& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  forceinline
  MeritMaxActionSize::MeritMaxActionSize(Space& home, const IntBoolVarBranch& ibvb)
    : iaction(ibvb.intaction()), baction(ibvb.boolaction()) {}
  forceinline
  MeritMaxActionSize::MeritMaxActionSize(Space& home, MeritMaxActionSize& m)
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


  /// Select by maximal CHB
  class MeritMaxCHB {
  protected:
    /// Integer CHB information
    IntCHB ichb;
    /// Boolean CHB information
    BoolCHB bchb;
  public:
    /// Constructor for initialization
    MeritMaxCHB(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxCHB(Space& home, MeritMaxCHB& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  forceinline
  MeritMaxCHB::MeritMaxCHB(Space& home, const IntBoolVarBranch& ibvb)
    : ichb(ibvb.intchb()), bchb(ibvb.boolchb()) {}
  forceinline
  MeritMaxCHB::MeritMaxCHB(Space& home, MeritMaxCHB& m)
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

  /// Select by maximal CHB over size
  class MeritMaxCHBSize {
  protected:
    /// Integer CHB information
    IntCHB ichb;
    /// Boolean CHB information
    BoolCHB bchb;
  public:
    /// Constructor for initialization
    MeritMaxCHBSize(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxCHBSize(Space& home, MeritMaxCHBSize& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  forceinline
  MeritMaxCHBSize::MeritMaxCHBSize(Space& home, const IntBoolVarBranch& ibvb)
    : ichb(ibvb.intchb()), bchb(ibvb.boolchb()) {}
  forceinline
  MeritMaxCHBSize::MeritMaxCHBSize(Space& home, MeritMaxCHBSize& m)
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


  /// Brancher for integer and Boolean views
  template<class Merit>
  class IntBoolBrancher : public Brancher {
  protected:
    /// Integer views to branch on
    ViewArray<Int::IntView> x;
    /// Boolean views to branch on
    ViewArray<Int::BoolView> y;
    /// Unassigned views start here (might be in \a x or \a y)
    mutable int start;
    /// Selection by maximal merit
    Merit merit;
    /// Integer value selection and commit object
    ValSelCommitBase<Int::IntView,int>* xvsc;
    /// Boolean value selection and commit object
    ValSelCommitBase<Int::BoolView,int>* yvsc;
    /// Constructor for cloning \a b
    IntBoolBrancher(Space& home, IntBoolBrancher& b);
    /// Constructor for creation
    IntBoolBrancher(Home home,
                    ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y,
                    Merit& m,
                    ValSelCommitBase<Int::IntView,int>* xvsc,
                    ValSelCommitBase<Int::BoolView,int>* yvsc);
  public:
    /// Check status of brancher, return true if alternatives left
    virtual bool status(const Space& home) const;
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a b
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int b);
    /// Create no-good literal for choice \a c and alternative \a b
    virtual NGL* ngl(Space& home, const Choice& c, unsigned int b) const;
    /// Print branch for choice \a c and alternative \a b
    virtual void print(const Space& home, const Choice& c, unsigned int b,
                       std::ostream& o) const;
    /// Perform cloning
    virtual Actor* copy(Space& home);
    /// Post barncher
    static void post(Home home,
                     ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y,
                     Merit& m,
                     ValSelCommitBase<Int::IntView,int>* xvsc,
                     ValSelCommitBase<Int::BoolView,int>* yvsc);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
  };

  /// %Choice storing position and value
  class GECODE_VTABLE_EXPORT PosIntChoice : public Choice {
  private:
    /// Position of view to assign
    int _pos;
    /// Value to assign to
    int _val;
  public:
    /// Initialize choice for brancher \a b, number of alternatives \a a, position \a p, and value \a n
    PosIntChoice(const Brancher& b, unsigned int a, int p, int n);
    /// Return position of view to assign
    int pos(void) const;
    /// Return value to assign to
    int val(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };

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
  void
  PosIntChoice::archive(Archive& e) const {
    Choice::archive(e);
    e << _pos;
    e << _val;
  }


  template<class Merit>
  forceinline
  IntBoolBrancher<Merit>::
  IntBoolBrancher(Home home,
                  ViewArray<Int::IntView> x0,
                  ViewArray<Int::BoolView> y0,
                  Merit& m,
                  ValSelCommitBase<Int::IntView,int>* xvsc0,
                  ValSelCommitBase<Int::BoolView,int>* yvsc0)
    : Brancher(home), x(x0), y(y0), start(0),
      merit(m), xvsc(xvsc0), yvsc(yvsc0) {
    home.notice(*this,AP_DISPOSE,true);
  }

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
    : Brancher(home,b), start(b.start),
      merit(home, b.merit),
      xvsc(b.xvsc->copy(home)), yvsc(b.yvsc->copy(home)) {
    x.update(home,b.x);
    y.update(home,b.y);
  }

  template<class Merit>
  Actor*
  IntBoolBrancher<Merit>::copy(Space& home) {
    return new (home) IntBoolBrancher<Merit>(home,*this);
  }

  template<class Merit>
  bool
  IntBoolBrancher<Merit>::status(const Space& home) const {
    if (start < x.size()) {
      for (int i=start; i < x.size(); i++)
        if (!x[i].assigned()) {
          start = i;
          return true;
        }
      start = x.size();
    }
    for (int i=start-x.size(); i < y.size(); i++)
      if (!y[i].assigned()) {
        start = x.size() + i;
        return true;
      }
    return false;
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
  ExecStatus
  IntBoolBrancher<Merit>::commit(Space& home, const Choice& _c,
                                 unsigned int b) {
    const PosIntChoice& c
      = static_cast<const PosIntChoice&>(_c);
    int p=c.pos(); int n=c.val();
    if (p < x.size()) {
      return me_failed(xvsc->commit(home,b,x[p],p,n)) ?
        ES_FAILED : ES_OK;
    } else {
      p -= x.size();
      return me_failed(yvsc->commit(home,b,y[p],p,n)) ?
        ES_FAILED : ES_OK;
    }
  }

  template<class Merit>
  NGL*
  IntBoolBrancher<Merit>::ngl(Space& home, const Choice& _c,
                              unsigned int b) const {
    const PosIntChoice& c
      = static_cast<const PosIntChoice&>(_c);
    int p=c.pos(); int n=c.val();
    if (p < x.size()) {
      return xvsc->ngl(home,b,x[p],n);
    } else {
      p -= x.size();
      return yvsc->ngl(home,b,y[p],n);
    }
  }

  template<class Merit>
  void
  IntBoolBrancher<Merit>::print(const Space& home, const Choice& _c,
                                unsigned int b,
                                std::ostream& o) const {
    const PosIntChoice& c
      = static_cast<const PosIntChoice&>(_c);
    int p=c.pos(); int n=c.val();
    if (p < x.size()) {
      xvsc->print(home,b,x[p],p,n,o);
    } else {
      p -= x.size();
      yvsc->print(home,b,y[p],p,n,o);
    }
  }

  template<class Merit>
  const Choice*
  IntBoolBrancher<Merit>::choice(const Space& home, Archive& e) {
    (void) home;
    int p; e >> p;
    int v; e >> v;
    return new PosIntChoice(*this,2,p,v);
  }

  template<class Merit>
  size_t
  IntBoolBrancher<Merit>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE,true);
    merit.dispose();
    xvsc->dispose(home);
    yvsc->dispose(home);
    (void) Brancher::dispose(home);
    return sizeof(IntBoolBrancher);
  }

  /// Map respective integer value selection to Boolean value selection
  BoolValBranch
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

  /// Branch function for integer and Boolean variables
  void
  branch(Home home, const IntVarArgs& x, const BoolVarArgs& y,
         IntBoolVarBranch vars, IntValBranch vals) {
    using namespace Int;
    if (home.failed()) return;
    ViewArray<IntView> xv(home,x);
    ViewArray<BoolView> yv(home,y);
    ValSelCommitBase<IntView,int>* xvsc =
      Branch::valselcommit(home,vals);
    ValSelCommitBase<BoolView,int>* yvsc =
      Branch::valselcommit(home,i2b(vals));
    switch (vars.select()) {
    case IntBoolVarBranch::SEL_AFC_MAX:
      {
        MeritMaxAFC m(home,vars);
        IntBoolBrancher<MeritMaxAFC>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_ACTION_MAX:
      {
        MeritMaxAction m(home,vars);
        IntBoolBrancher<MeritMaxAction>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_CHB_MAX:
      {
        MeritMaxCHB m(home,vars);
        IntBoolBrancher<MeritMaxCHB>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_AFC_SIZE_MAX:
      {
        MeritMaxAFCSize m(home,vars);
        IntBoolBrancher<MeritMaxAFCSize>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_ACTION_SIZE_MAX:
      {
        MeritMaxActionSize m(home,vars);
        IntBoolBrancher<MeritMaxActionSize>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    case IntBoolVarBranch::SEL_CHB_SIZE_MAX:
      {
        MeritMaxCHBSize m(home,vars);
        IntBoolBrancher<MeritMaxCHBSize>::post(home,xv,yv,m,xvsc,yvsc);
      }
      break;
    default:
      GECODE_NEVER;
    }
  }

}}

using namespace Gecode;

/** The maximum number of knights placeable
 *
 * \relates QueensArmies
 */
const int kval[] = {
  0,   0,  0,  0,  5,
  9,  15, 21, 29, 37,
  47, 57, 69, 81, 94,
  109
};

namespace {
  /** \brief Set of valid positions for the bishops
   * \relates CrowdedChess
   */
  TupleSet bishops;
  /** \brief Class for solving the bishops sub-problem
   * \relates CrowdedChess
   */
  class Bishops : public Space {
  public:
    const int n;
    BoolVarArray k;
    bool valid_pos(int i, int j) {
      return (i >= 0 && i < n) && (j >= 0 && j < n);
    }
    Bishops(int size)
      : n(size), k(*this,n*n,0,1) {
      Matrix<BoolVarArray> kb(k, n, n);
      for (int l = n; l--; ) {
        const int il = (n-1) - l;
        BoolVarArgs d1(l+1), d2(l+1), d3(l+1), d4(l+1);
        for (int i = 0; i <= l; ++i) {
          d1[i] = kb(i+il, i);
          d2[i] = kb(i, i+il);
          d3[i] = kb((n-1)-i-il, i);
          d4[i] = kb((n-1)-i, i+il);
        }

        linear(*this, d1, IRT_LQ, 1);
        linear(*this, d2, IRT_LQ, 1);
        linear(*this, d3, IRT_LQ, 1);
        linear(*this, d4, IRT_LQ, 1);
      }

      linear(*this, k, IRT_EQ, 2*n - 2);
      // Forced bishop placement from crowded chess model
      rel(*this, kb(n-1,   0), IRT_EQ, 1);
      rel(*this, kb(n-1, n-1), IRT_EQ, 1);
      branch(*this, k, BOOL_VAR_DEGREE_MAX(), BOOL_VAL_MAX());
    }
    Bishops(Bishops& s) : Space(s), n(s.n) {
      k.update(*this, s.k);
    }
    virtual Space* copy(void) {
      return new Bishops(*this);
    }
  };
  /** \brief Initialize bishops
   * \relates CrowdedChess
   */
  void init_bishops(int size) {
    Bishops* prob = new Bishops(size);
    DFS<Bishops> e(prob); IntArgs ia(size*size);
    delete prob;

    while (Bishops* s = e.next()) {
      for (int i = size*size; i--; )
        ia[i] = s->k[i].val();
      bishops.add(ia);
      delete s;
    }

    bishops.finalize();
  }
}
/**
   \brief %Example: Crowded chessboard

   You are given a chessboard together with 8 queens, 8 rooks, 14
   bishops, and 21 knights. The puzzle is to arrange the 51 pieces on
   the chessboard so that no queen shall attack another queen, no rook
   attack another rook, no bishop attack another bishop, and no knight
   attack another knight. No notice is to be taken of the intervention
   of pieces of another type from that under consideration - that is,
   two queens will be considered to attack one another although there
   may be, say, a rook, a bishop, and a knight between them. It is not
   difficult to dispose of each type of piece separately; the
   difficulty comes in when you have to find room for all the
   arrangements on the board simultaneously.
   <em>Dudeney, H.E., (1917), Amusements in Mathematics,
   Thomas Nelson and Sons.</em>

   This puzzle can be generalized to chess-boards of size \f$n\f$, where the
   number of pieces to place are:
     - \f$ n \f$ queens
     - \f$ n \f$ rooks
     - \f$ 2n-1 \f$ bishops
     - \f$ k \f$ knights
   where k is a number to maximize.

   The maximum k for some different values of \f$ n \f$ are presented
   below (from Jesper Hansen and Joachim Schimpf, <a
   href="http://www.icparc.ic.ac.uk/eclipse/examples/crowded_chess.ecl.txt">
   ECLiPSe solution</a>
   <TABLE>
   <TR> <TD> n </TD> <TD> k </TD> </TR>
   <TR> <TD> 8 </TD> <TD> 21 </TD> </TR>
   <TR> <TD> 9 </TD> <TD> 29 </TD> </TR>
   <TR> <TD> 10 </TD> <TD> 37 </TD> </TR>
   <TR> <TD> 11 </TD> <TD> 47 </TD> </TR>
   <TR> <TD> 12 </TD> <TD> 57 </TD> </TR>
   <TR> <TD> 13 </TD> <TD> 69 </TD> </TR>
   <TR> <TD> 14 </TD> <TD> 81 </TD> </TR>
   <TR> <TD> 15 </TD> <TD> 94 </TD> </TR>
   <TR> <TD> 16 </TD> <TD> 109 </TD> </TR>
   </TABLE>

   A solution for n = 8 is:
   <TABLE>
   <TR><TD>Q</TD><TD>B</TD><TD>K</TD><TD>.</TD>
   <TD>K</TD><TD>B</TD><TD>K</TD><TD>R</TD></TR>
   <TR><TD>.</TD><TD>K</TD><TD>.</TD><TD>K</TD>
   <TD>Q</TD><TD>K</TD><TD>R</TD><TD>B</TD></TR>
   <TR><TD>B</TD><TD>.</TD><TD>K</TD><TD>R</TD>
   <TD>K</TD><TD>.</TD><TD>K</TD><TD>Q</TD></TR>
   <TR><TD>B</TD><TD>K</TD><TD>R</TD><TD>K</TD>
   <TD>.</TD><TD>Q</TD><TD>.</TD><TD>B</TD></TR>
   <TR><TD>B</TD><TD>R</TD><TD>Q</TD><TD>.</TD>
   <TD>K</TD><TD>.</TD><TD>K</TD><TD>B</TD></TR>
   <TR><TD>R</TD><TD>K</TD><TD>.</TD><TD>K</TD>
   <TD>.</TD><TD>K</TD><TD>Q</TD><TD>B</TD></TR>
   <TR><TD>B</TD><TD>Q</TD><TD>K</TD><TD>.</TD>
   <TD>K</TD><TD>R</TD><TD>K</TD><TD>.</TD></TR>
   <TR><TD>B</TD><TD>K</TD><TD>B</TD><TD>Q</TD>
   <TD>R</TD><TD>K</TD><TD>B</TD><TD>B</TD></TR>
 </TABLE>

   \ingroup Example
*/
class CrowdedChess : public Script {
protected:
  const int n;          ///< Board-size
  IntVarArray s;        ///< The board
  IntVarArray queens,   ///< Row of queen in column x
    rooks;              ///< Row of rook in column x
  BoolVarArray knights; ///< True iff the corresponding place has a knight

  /** Symbolic names of pieces. The order determines which piece will
   * be placed first.
   */
  enum
    {Q,   ///< Queen
     R,   ///< Rook
     B,   ///< Bishop
     K,   ///< Knight
     E,   ///< Empty square
     PMAX ///< Number of pieces (including empty squares)
    } piece;

  // Is (i,j) a valid position on the board?
  bool valid_pos(int i, int j) {
    return (i >= 0 && i < n) &&
      (j >= 0 && j < n);
  }

  /// Post knight-constraints
  void knight_constraints(void) {
    static const int kmoves[4][2] = {
      {-1,2}, {1,2}, {2,-1}, {2,1}
    };
    Matrix<BoolVarArray> kb(knights, n, n);
    for (int x = n; x--; )
      for (int y = n; y--; )
        for (int i = 4; i--; )
          if (valid_pos(x+kmoves[i][0], y+kmoves[i][1]))
            rel(*this,
                kb(x, y),
                BOT_AND,
                kb(x+kmoves[i][0], y+kmoves[i][1]),
                0);
  }


public:
  enum {
    PROP_TUPLE_SET, ///< Propagate bishops placement extensionally
    PROP_DECOMPOSE  ///< Propagate bishops placement with decomposition
  };

  /// The model of the problem
  CrowdedChess(const SizeOptions& opt)
    : Script(opt),
      n(opt.size()),
      s(*this, n*n, 0, PMAX-1),
      queens(*this, n, 0, n-1),
      rooks(*this, n, 0, n-1),
      knights(*this, n*n, 0, 1) {
    const int nkval = sizeof(kval)/sizeof(int);
    const int nn = n*n, q = n, r = n, b = (2*n)-2,
      k = n <= nkval ? kval[n-1] : kval[nkval-1];
    const int e = nn - (q + r + b + k);

    assert(nn == (e + q + r + b + k));

    Matrix<IntVarArray> m(s, n);

    // ***********************
    // Basic model
    // ***********************

    count(*this, s, E, IRT_EQ, e, opt.ipl());
    count(*this, s, Q, IRT_EQ, q, opt.ipl());
    count(*this, s, R, IRT_EQ, r, opt.ipl());
    count(*this, s, B, IRT_EQ, b, opt.ipl());
    count(*this, s, K, IRT_EQ, k, opt.ipl());

    // Collect rows and columns for handling rooks and queens
    for (int i = 0; i < n; ++i) {
      IntVarArgs aa = m.row(i), bb = m.col(i);

      count(*this, aa, Q, IRT_EQ, 1, opt.ipl());
      count(*this, bb, Q, IRT_EQ, 1, opt.ipl());
      count(*this, aa, R, IRT_EQ, 1, opt.ipl());
      count(*this, bb, R, IRT_EQ, 1, opt.ipl());

      // Connect (queens|rooks)[i] to the row it is in
      element(*this, aa, queens[i], Q, IPL_DOM);
      element(*this, aa,  rooks[i], R, IPL_DOM);
    }

    // N-queens constraints
    distinct(*this, queens, IPL_DOM);
    distinct(*this, IntArgs::create(n,0,1), queens, IPL_DOM);
    distinct(*this, IntArgs::create(n,0,-1), queens, IPL_DOM);

    // N-rooks constraints
    distinct(*this,  rooks, IPL_DOM);

    // Collect diagonals for handling queens and bishops
    for (int l = n; l--; ) {
      const int il = (n-1) - l;
      IntVarArgs d1(l+1), d2(l+1), d3(l+1), d4(l+1);
      for (int i = 0; i <= l; ++i) {
        d1[i] = m(i+il, i);
        d2[i] = m(i, i+il);
        d3[i] = m((n-1)-i-il, i);
        d4[i] = m((n-1)-i, i+il);
      }

      count(*this, d1, Q, IRT_LQ, 1, opt.ipl());
      count(*this, d2, Q, IRT_LQ, 1, opt.ipl());
      count(*this, d3, Q, IRT_LQ, 1, opt.ipl());
      count(*this, d4, Q, IRT_LQ, 1, opt.ipl());
      if (opt.propagation() == PROP_DECOMPOSE) {
        count(*this, d1, B, IRT_LQ, 1, opt.ipl());
        count(*this, d2, B, IRT_LQ, 1, opt.ipl());
        count(*this, d3, B, IRT_LQ, 1, opt.ipl());
        count(*this, d4, B, IRT_LQ, 1, opt.ipl());
      }
    }
    if (opt.propagation() == PROP_TUPLE_SET) {
      IntVarArgs b(s.size());
      for (int i = s.size(); i--; )
        b[i] = channel(*this, expr(*this, (s[i] == B)));
      extensional(*this, b, bishops, opt.ipl());
    }

    // Handle knigths
    // Connect knigths to board
    for(int i = n*n; i--; )
      knights[i] = expr(*this, (s[i] == K));
    knight_constraints();


    // ***********************
    // Redundant constraints
    // ***********************

    // Queens and rooks not in the same place
    // Faster than going through the channelled board-connection
    for (int i = n; i--; )
      rel(*this, queens[i], IRT_NQ, rooks[i]);

    // Place bishops in two corners (from Schimpf and Hansens solution)
    // Avoids some symmetries of the problem
    rel(*this, m(n-1,   0), IRT_EQ, B);
    rel(*this, m(n-1, n-1), IRT_EQ, B);


    // ***********************
    // Branching
    // ***********************
    // Place each piece in turn
    //    branch(*this, s, INT_VAR_MIN_MIN(), INT_VAL_MIN());
    IntAFC iafc(*this, s);
    BoolAFC bafc(*this, knights);
    IntAction iaction(*this, s);
    BoolAction baction(*this, knights);
    FlatZinc::branch(*this, s, knights, 
                     FlatZinc::INTBOOL_VAR_ACTION_MAX(iaction,baction),
                     INT_VAL_MIN());
  }

  /// Constructor for cloning e
  CrowdedChess(CrowdedChess& e)
    : Script(e), n(e.n) {
    s.update(*this, e.s);
    queens.update(*this, e.queens);
    rooks.update(*this, e.rooks);
    knights.update(*this, e.knights);
  }

  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new CrowdedChess(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m(s, n);
    char names[PMAX];
    names[E] = '.'; names[Q] = 'Q'; names[R] = 'R';
    names[B] = 'B'; names[K] = 'K';
    const char* sep   = n < 8 ? "\t\t" : "\t";

    for (int r = 0; r < n; ++r){
      // Print main board
      os << '\t';
      for (int c = 0; c < n; ++c) {
        if (m(r, c).assigned()) {
          os << names[m(r, c).val()];
        } else {
          os << " ";
        }
      }
      // Print each piece on its own
      for (int p = 0; p < PMAX; ++p) {
        if (p == E) continue;
        os << sep;
        for (int c = 0; c < n; ++c) {
          if (m(r, c).assigned()) {
            if (m(r, c).val() == p)
              os << names[p];
            else
              os << names[E];
          } else {
            os << " ";
          }
        }
      }
      os << std::endl;
    }
    os << std::endl;
  }
};

/** \brief Main function
 * \relates CrowdedChess
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("CrowdedChess");
  opt.propagation(CrowdedChess::PROP_TUPLE_SET);
  opt.propagation(CrowdedChess::PROP_TUPLE_SET,
                  "extensional",
                  "Use extensional propagation for bishops-placement");
  opt.propagation(CrowdedChess::PROP_DECOMPOSE,
                  "decompose",
                  "Use decomposed propagation for bishops-placement");
  opt.ipl(IPL_DOM);
  opt.size(8);
  opt.parse(argc,argv);
  if (opt.size() < 5) {
    std::cerr << "Error: size must be at least 5" << std::endl;
    return 1;
  }
  init_bishops(opt.size());
  Script::run<CrowdedChess,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

