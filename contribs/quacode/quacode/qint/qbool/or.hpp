/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/int/bool/or.hpp
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

namespace Gecode { namespace Int { namespace Bool {

  /*
   * Quantified Binary Boolean disjunction propagator (true)
   *
   */

  template<class BVA, class BVB>
  forceinline
  QBinOrTrue<BVA,BVB>::QBinOrTrue(Home home,
                                  BVA b0, TQuantifier _q0, int _r0,
                                  BVB b1, TQuantifier _q1, int _r1)
    : BoolBinary<BVA,BVB>(home,b0,b1), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB>
  forceinline
  QBinOrTrue<BVA,BVB>::QBinOrTrue(Space& home, bool share, QBinOrTrue<BVA,BVB>& p)
    : BoolBinary<BVA,BVB>(home,share,p) {}

  template<class BVA, class BVB>
  forceinline
  QBinOrTrue<BVA,BVB>::QBinOrTrue(Space& home, bool share, Propagator& p,
                                  BVA b0, TQuantifier _q0, int _r0,
                                  BVB b1, TQuantifier _q1, int _r1)
    : BoolBinary<BVA,BVB>(home,share,p,b0,b1), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB>
  Actor*
  QBinOrTrue<BVA,BVB>::copy(Space& home, bool share) {
    return new (home) QBinOrTrue<BVA,BVB>(home,share,*this);
  }

  template<class BVA, class BVB>
  inline ExecStatus
  QBinOrTrue<BVA,BVB>::post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1) {
    switch (bool_test(b0,b1)) {
    case BT_SAME:
      assert((_q0 == _q1) && (_r0 == _r1));
      if (_q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(b0.one(home));
      break;
    case BT_COMP:
      break;
    case BT_NONE:
      if (b0.zero()) {
        if (_q1 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b1.one(home));
      } else if (b1.zero()) {
        if (_q0 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b0.one(home));
      } else if (!b0.one() && !b1.one()) {
        if ((_q0 == FORALL) && (_q1 == FORALL)) return ES_FAILED;
        if ((_q0 == FORALL) && (_r0 > _r1)) { GECODE_ME_CHECK(b1.one(home)); break; }
        if ((_q1 == FORALL) && (_r1 > _r0)) { GECODE_ME_CHECK(b0.one(home)); break; }
        (void) new (home) QBinOrTrue<BVA,BVB>(home,b0,_q0,_r0,b1,_q1,_r1);
      }
      break;
    default: GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class BVA, class BVB>
  ExecStatus
  QBinOrTrue<BVA,BVB>::propagate(Space& home, const ModEventDelta&) {
#define GECODE_INT_STATUS(S0,S1) \
  ((BVA::S0<<(1*BVA::BITS))|(BVB::S1<<(0*BVB::BITS)))
    switch ((x0.status() << (1*BVA::BITS)) | (x1.status() << (0*BVB::BITS))) {
    case GECODE_INT_STATUS(NONE,NONE):
      GECODE_NEVER;
    case GECODE_INT_STATUS(NONE,ZERO):
      GECODE_ME_CHECK(x0.one_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE):
      break;
    case GECODE_INT_STATUS(ZERO,NONE):
      GECODE_ME_CHECK(x1.one_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ZERO,ONE):
    case GECODE_INT_STATUS(ONE,NONE):
    case GECODE_INT_STATUS(ONE,ZERO):
    case GECODE_INT_STATUS(ONE,ONE):
      break;
    default:
        GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
#undef GECODE_INT_STATUS
  }

  /*
   * Quantified boolean disjunction propagator
   *
   */

  template<class BVA, class BVB, class BVC>
  forceinline
  QOr<BVA,BVB,BVC>::QOr(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2)
    : BoolTernary<BVA,BVB,BVC>(home,b0,b1,b2), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB, class BVC>
  forceinline
  QOr<BVA,BVB,BVC>::QOr(Space& home, bool share, QOr<BVA,BVB,BVC>& p)
    : BoolTernary<BVA,BVB,BVC>(home,share,p), q0(p.q0), r0(p.r0), q1(p.q1), r1(p.r1) {}

  template<class BVA, class BVB, class BVC>
  forceinline
  QOr<BVA,BVB,BVC>::QOr(Space& home, bool share, Propagator& p,
                         BVA b0, TQuantifier _q0, int _r0,
                         BVB b1, TQuantifier _q1, int _r1, BVC b2)
    : BoolTernary<BVA,BVB,BVC>(home,share,p,b0,b1,b2), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB, class BVC>
  Actor*
  QOr<BVA,BVB,BVC>::copy(Space& home, bool share) {
    if (x2.one()) {
      assert(x0.none() && x1.none());
      return new (home) QBinOrTrue<BVA,BVB>(home,share,*this,x0,q0,r0,x1,q1,r1);
    } else if (x0.zero()) {
      assert(x1.none() && x2.none());
      return new (home) QEq<BVB,BVC>(home,share,*this,x1,q1,r1,x2,EXISTS,Int::Limits::max);
    } else if (x1.zero()) {
      assert(x0.none() && x2.none());
      return new (home) QEq<BVA,BVC>(home,share,*this,x0,q0,r0,x2,EXISTS,Int::Limits::max);
    } else {
      return new (home) QOr<BVA,BVB,BVC>(home,share,*this);
    }
  }

  template<class BVA, class BVB, class BVC>
  inline ExecStatus
  QOr<BVA,BVB,BVC>::post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2) {
    if (b2.zero()) {
      if ((_q0 == FORALL) || (_q1 == FORALL)) return ES_FAILED;
      GECODE_ME_CHECK(b0.zero(home));
      GECODE_ME_CHECK(b1.zero(home));
    } else if (b2.one()) {
      return QBinOrTrue<BVA,BVB>::post(home,b0,_q0,_r0,b1,_q1,_r1);
    } else {
      switch (bool_test(b0,b1)) {
      case BT_SAME:
        return QEq<BVA,BVC>::post(home,b0,_q0,_r0,b2,EXISTS,Int::Limits::max);
      case BT_COMP:
        if (_q1 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b2.one(home));
        break;
      case BT_NONE:
        if (b0.one() || b1.one()) {
          GECODE_ME_CHECK(b2.one(home));
        } else if (b0.zero()) {
          return QEq<BVB,BVC>::post(home,b1,_q1,_r1,b2,EXISTS,Int::Limits::max);
        } else if (b1.zero()) {
          return QEq<BVA,BVC>::post(home,b0,_q0,_r0,b2,EXISTS,Int::Limits::max);
        } else {
          if ((_q0 == FORALL) && (_q1 == FORALL)) {GECODE_ME_CHECK(b2.zero(home)); break; }
          (void) new (home) QOr<BVA,BVB,BVC>(home,b0,_q0,_r0,b1,_q1,_r1,b2);
        }
        break;
      default: GECODE_NEVER;
      }
    }
    return ES_OK;
  }

  template<class BVA, class BVB, class BVC>
  ExecStatus
  QOr<BVA,BVB,BVC>::propagate(Space& home, const ModEventDelta&) {
    assert((q0 == EXISTS) || (q1 == EXISTS));
#define GECODE_INT_STATUS(S0,S1,S2) \
  ((BVA::S0<<(2*BVA::BITS))|(BVB::S1<<(1*BVB::BITS))|(BVC::S2<<(0*BVC::BITS)))
    switch ((x0.status() << (2*BVA::BITS)) | (x1.status() << (1*BVB::BITS)) |
            (x2.status() << (0*BVC::BITS))) {
    case GECODE_INT_STATUS(NONE,NONE,NONE):
      GECODE_NEVER;
    case GECODE_INT_STATUS(NONE,NONE,ZERO):
      if ((q0 == FORALL) || (q1 == FORALL)) return ES_FAILED;
      GECODE_ME_CHECK(x0.zero_none(home));
      GECODE_ME_CHECK(x1.zero_none(home));
      break;
    case GECODE_INT_STATUS(NONE,NONE,ONE):
      if ((q0 == FORALL) && (r0 > r1)) { GECODE_ME_CHECK(x1.one_none(home)); break; }
      if ((q1 == FORALL) && (r1 > r0)) { GECODE_ME_CHECK(x0.one_none(home)); break; }
      return ES_FIX;
    case GECODE_INT_STATUS(NONE,ZERO,NONE):
      switch (bool_test(x0,x2)) {
      case BT_SAME: return home.ES_SUBSUMED(*this);
      case BT_COMP: return ES_FAILED;
      case BT_NONE: return ES_FIX;
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
    case GECODE_INT_STATUS(NONE,ZERO,ZERO):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.zero_none(home)); break;
    case GECODE_INT_STATUS(NONE,ZERO,ONE):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.one_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(NONE,ONE,ONE):
      break;
    case GECODE_INT_STATUS(ZERO,NONE,NONE):
      switch (bool_test(x1,x2)) {
      case BT_SAME: return home.ES_SUBSUMED(*this);
      case BT_COMP: return ES_FAILED;
      case BT_NONE: return ES_FIX;
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
    case GECODE_INT_STATUS(ZERO,NONE,ZERO):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.zero_none(home)); break;
    case GECODE_INT_STATUS(ZERO,NONE,ONE):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.one_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ZERO,NONE):
      GECODE_ME_CHECK(x2.zero_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ZERO,ZERO):
      break;
    case GECODE_INT_STATUS(ZERO,ZERO,ONE):
      return ES_FAILED;
    case GECODE_INT_STATUS(ZERO,ONE,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ONE,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ZERO,ONE,ONE):
      break;
    case GECODE_INT_STATUS(ONE,NONE,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,NONE,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ONE,NONE,ONE):
      break;
    case GECODE_INT_STATUS(ONE,ZERO,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,ZERO,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ONE,ZERO,ONE):
      break;
    case GECODE_INT_STATUS(ONE,ONE,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,ONE,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ONE,ONE,ONE):
      break;
    default:
      GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
#undef GECODE_INT_STATUS
  }


  /*
   * Quantified N-ary Boolean disjunction propagator (true)
   *
   */

  template<class BV>
  forceinline
  QNaryOrTrue<BV>::QNaryOrTrue(Home home, ViewArray<BV>& b, const QuantArgs& _q, const IntArgs& _r)
    : BinaryPropagator<BV,PC_BOOL_VAL>(home,b[0],b[1]), x(b), qx0(_q[0]), qx1(_q[1]), qx(NULL), rx0(_r[0]), rx1(_r[1]), rx(NULL)  {
    assert(x.size() > 2);
    x.drop_fst(2);
    qx = static_cast<Space&>(home).alloc<TQuantifier>(x.size());
    rx = static_cast<Space&>(home).alloc<int>(x.size());
    for (int n=x.size(); --n ; ) { qx[n] = _q[n+2]; rx[n] = _r[n+2]; }
  }

  template<class BV>
  PropCost
  QNaryOrTrue<BV>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<class BV>
  forceinline
  QNaryOrTrue<BV>::QNaryOrTrue(Space& home, bool share, QNaryOrTrue<BV>& p)
    : BinaryPropagator<BV,PC_BOOL_VAL>(home,share,p), qx0(p.qx0), qx1(p.qx1), qx(NULL),
                                                      rx0(p.rx0), rx1(p.rx1), rx(NULL) {
    qx = home.alloc<TQuantifier>(p.x.size());
    rx = home.alloc<int>(p.x.size());
    for (int n=p.x.size(); --n ; ) { qx[n] = p.qx[n]; rx[n] = p.rx[n]; }
    x.update(home,share,p.x);
  }

  template<class BV>
  Actor*
  QNaryOrTrue<BV>::copy(Space& home, bool share) {
    int n = x.size();
    if (n > 0) {
      // Eliminate all zeros and find a one
      for (int i=n; i--; )
        if (x[i].one()) {
          // Only keep the one
          x[0]=x[i]; x.size(1);
          return new (home) OrTrueSubsumed<BV>(home,share,*this,x0,x1);
        } else if (x[i].zero()) {
          // Eliminate the zero
          x[i]=x[--n]; qx[i]=qx[n]; rx[i]=rx[n];
        }
      x.size(n);
    }
    switch (n) {
    case 0:
      return new (home) QBinOrTrue<BV,BV>(home,share,*this,x0,qx0,rx0,x1,qx1,rx1);
    default:
      return new (home) QNaryOrTrue<BV>(home,share,*this);
    }
  }

  template<class BV>
  inline ExecStatus
  QNaryOrTrue<BV>::post(Home home, ViewArray<BV>& b, QuantArgs _q, IntArgs _r) {
    int nbExists=0;
    for (int i=b.size(); i--; )
      if (b[i].one())
        return ES_OK;
      else if (b[i].zero())
      {
        b.move_lst(i); _q[i]=_q[b.size()]; _r[i]=_r[b.size()];
      }
      else if (_q[i] == EXISTS)
        nbExists++;

    if (nbExists == 0) return ES_FAILED;
    if (b.size() == 0) return ES_FAILED;
    if (b.size() == 1) {
      if (_q[0] == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(b[0].one(home));
    } else if (b.size() == 2) {
       return QBinOrTrue<BV,BV>::post(home,b[0],_q[0],_r[0],b[1],_q[1],_r[1]);
    } else {
      (void) new (home) QNaryOrTrue(home,b,_q,_r);
    }
    return ES_OK;
  }

  template<class BV>
  forceinline size_t
  QNaryOrTrue<BV>::dispose(Space& home) {
    (void) BinaryPropagator<BV,PC_BOOL_VAL>::dispose(home);
    return sizeof(*this);
  }

  template<class BV>
  forceinline ExecStatus
  QNaryOrTrue<BV>::resubscribe(Space& home, BV& x0, TQuantifier& _qx0, int& _rx0, BV x1, TQuantifier _qx1, int ) {
    if (x0.zero()) {
      int n = x.size();
      for (int i=n; i--; )
        if (x[i].one()) {
          return home.ES_SUBSUMED(*this);
        } else if (x[i].zero()) {
          x[i] = x[--n]; qx[i]=qx[n]; rx[i]=rx[n];
        } else {
          // Move to x0 and subscribe
          x0=x[i]; _qx0=qx[i]; _rx0=rx[i]; x[i]=x[--n]; qx[i]=qx[n]; rx[i]=rx[n];
          x.size(n);
          x0.subscribe(home,*this,PC_BOOL_VAL,false);
          return ES_FIX;
        }
      // All views have been assigned!
      if (_qx1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.one(home));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

  template<class BV>
  ExecStatus
  QNaryOrTrue<BV>::propagate(Space& home, const ModEventDelta&) {
    if (x0.one())
      return home.ES_SUBSUMED(*this);
    if (x1.one())
      return home.ES_SUBSUMED(*this);
    GECODE_ES_CHECK(resubscribe(home,x0,qx0,rx0,x1,qx1,rx1));
    GECODE_ES_CHECK(resubscribe(home,x1,qx1,rx1,x0,qx0,rx0));
    return ES_FIX;
  }


  /*
   * N-ary Quantified boolean disjunction propagator
   *
   */

  template<class VX, class VY>
  forceinline
  QNaryOr<VX,VY>::QNaryOr(Home home, ViewArray<VX>& x, const QuantArgs& _q, const IntArgs& _r, VY y)
    : MixNaryOnePropagator<VX,PC_BOOL_NONE,VY,PC_BOOL_VAL>(home,x,y), q(NULL), r(NULL),
      n_zero(0), c(home) {
    q = static_cast<Space&>(home).alloc<TQuantifier>(x.size());
    r = static_cast<Space&>(home).alloc<int>(x.size());
    for (int n=x.size(); --n ; ) { q[n] = _q[n]; r[n] = _r[n]; }
    x.subscribe(home,*new (home) Advisor(home,*this,c));
  }

  template<class VX, class VY>
  forceinline
  QNaryOr<VX,VY>::QNaryOr(Space& home, bool share, QNaryOr<VX,VY>& p)
    : MixNaryOnePropagator<VX,PC_BOOL_NONE,VY,PC_BOOL_VAL>(home,share,p),
      q(NULL), r(NULL), n_zero(p.n_zero) {
    q = home.alloc<TQuantifier>(p.x.size());
    r = home.alloc<int>(p.x.size());
    for (int n=p.x.size(); --n ; ) { q[n] = p.q[n]; r[n] = p.r[n]; }
    c.update(home,share,p.c);
  }

  template<class VX, class VY>
  Actor*
  QNaryOr<VX,VY>::copy(Space& home, bool share) {
    assert(n_zero < x.size());
    if (n_zero > 0) {
      int n=x.size();
      // Eliminate all zeros
      for (int i=n; i--; )
        if (x[i].zero())
        {
          x[i]=x[--n]; q[i]=q[n]; r[i]=r[n];
        }
      x.size(n);
      n_zero = 0;
    }
    assert(n_zero < x.size());
    return new (home) QNaryOr<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  inline ExecStatus
  QNaryOr<VX,VY>::post(Home home, ViewArray<VX>& x, QuantArgs _q, IntArgs _r, VY y) {
    assert(!x.shared(home));
    if (y.one())
      return QNaryOrTrue<VX>::post(home,x,_q,_r);
    if (y.zero()) {
      for (int i=x.size(); i--; )
      {
        if (_q[i] == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(x[i].zero(home));
      }
      return ES_OK;
    }
    for (int i=x.size(); i--; )
      if (x[i].one()) {
        GECODE_ME_CHECK(y.one_none(home));
        return ES_OK;
      } else if (x[i].zero()) {
        x.move_lst(i);
        _q[i]=_q[x.size()];
        _r[i]=_r[x.size()];
      }
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.zero_none(home));
    } else if (x.size() == 1) {
      return QEq<VX,VY>::post(home,x[0],_q[0],_r[0],y,EXISTS,Int::Limits::max);
    } else if (x.size() == 2) {
      return QOr<VX,VX,VY>::post(home,x[0],_q[0],_r[0],x[1],_q[1],_r[1],y);
    } else {
      (void) new (home) QNaryOr(home,x,_q,_r,y);
    }
    return ES_OK;
  }

  template<class VX, class VY>
  PropCost
  QNaryOr<VX,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::LO);
  }

  template<class VX, class VY>
  ExecStatus
  QNaryOr<VX,VY>::advise(Space&, Advisor&, const Delta& d) {
    // Decides whether the propagator must be run
    if (VX::zero(d) && (++n_zero < x.size()))
      return ES_FIX;
    else
      return ES_NOFIX;
  }

  template<class VX, class VY>
  forceinline size_t
  QNaryOr<VX,VY>::dispose(Space& home) {
    Advisors<Advisor> as(c);
    x.cancel(home,as.advisor());
    c.dispose(home);
    (void) MixNaryOnePropagator<VX,PC_BOOL_NONE,VY,PC_BOOL_VAL>
      ::dispose(home);
    return sizeof(*this);
  }

  template<class VX, class VY>
  ExecStatus
  QNaryOr<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    if (y.one())
      GECODE_REWRITE(*this,QNaryOrTrue<VX>::post(home(*this),x,QuantArgs(x.size(),q),IntArgs(x.size(),r)));
    if (y.zero()) {
      // Note that this might trigger the advisor of this propagator!
      for (int i = x.size(); i--; )
      {
        if (q[i] == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(x[i].zero(home));
      }
    } else if (n_zero == x.size()) {
      // All views are zero
      GECODE_ME_CHECK(y.zero_none(home));
    } else {
      // There is exactly one view which is one
      GECODE_ME_CHECK(y.one_none(home));
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: int-prop
