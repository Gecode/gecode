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
 * This file is based on gecode/int/bool/clause.hpp
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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
   * Quantified Boolean clause propagator (disjunctive, true)
   *
   */

  template<class VX, class VY>
  forceinline
  QClauseTrue<VX,VY>::QClauseTrue(Home home,
                                  ViewArray<VX>& x0, const QuantArgs& qx0, const IntArgs& rx0,
                                  ViewArray<VY>& y0, const QuantArgs& qy0, const IntArgs& ry0)
    : MixBinaryPropagator<VX,PC_BOOL_VAL,VY,PC_BOOL_VAL>
  (home,x0[x0.size()-1],y0[y0.size()-1]), x(x0), c_qx(qx0[x0.size()-1]), qx(NULL), c_rx(rx0[x0.size()-1]), rx(NULL),
                                          y(y0), c_qy(qy0[y0.size()-1]), qy(NULL), c_ry(ry0[y0.size()-1]), ry(NULL) {
    assert((x.size() > 0) && (y.size() > 0));
    x.size(x.size()-1); y.size(y.size()-1);
    if (x.size() > 0) {
      qx = static_cast<Space&>(home).alloc<TQuantifier>(x.size());
      rx = static_cast<Space&>(home).alloc<int>(x.size());
      for (int n=x.size(); n-- ; ) { qx[n] = qx0[n]; rx[n] = rx0[n]; }
    }
    if (y.size() > 0) {
      qy = static_cast<Space&>(home).alloc<TQuantifier>(y.size());
      ry = static_cast<Space&>(home).alloc<int>(y.size());
      for (int n=y.size(); n-- ; ) { qy[n] = qy0[n]; ry[n] = ry0[n]; }
    }
  }

  template<class VX, class VY>
  PropCost
  QClauseTrue<VX,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<class VX, class VY>
  forceinline
  QClauseTrue<VX,VY>::QClauseTrue(Space& home, bool share, QClauseTrue<VX,VY>& p)
    : MixBinaryPropagator<VX,PC_BOOL_VAL,VY,PC_BOOL_VAL>(home,share,p), c_qx(p.c_qx), qx(NULL), c_rx(p.c_rx), rx(NULL),
                                                                        c_qy(p.c_qy), qy(NULL), c_ry(p.c_ry), ry(NULL) {
    if (p.x.size() > 0) {
      qx = home.alloc<TQuantifier>(p.x.size());
      rx = home.alloc<int>(p.x.size());
      for (int n=p.x.size(); n-- ; ) { qx[n] = p.qx[n]; rx[n] = p.rx[n]; }
    }
    if (p.y.size() > 0) {
      qy = home.alloc<TQuantifier>(p.y.size());
      ry = home.alloc<int>(p.y.size());
      for (int n=p.y.size(); n-- ; ) { qy[n] = p.qy[n]; ry[n] = p.ry[n]; }
    }
    x.update(home,share,p.x);
    y.update(home,share,p.y);
  }

  template<class VX, class VY>
  Actor*
  QClauseTrue<VX,VY>::copy(Space& home, bool share) {
    {
      int n = x.size();
      if (n > 0) {
        // Eliminate all zeros and find a one
        for (int i=n; i--; )
          if (x[i].one()) {
            // Only keep the one
            x[0]=x[i]; qx[0]=qx[i]; rx[0]=rx[i]; n=1; break;
          } else if (x[i].zero()) {
            // Eliminate the zero
            x[i]=x[--n];
            qx[i]=qx[n];
            rx[i]=rx[n];
          }
        x.size(n);
      }
    }
    {
      int n = y.size();
      if (n > 0) {
        // Eliminate all zeros and find a one
        for (int i=n; i--; )
          if (y[i].one()) {
            // Only keep the one
            y[0]=y[i]; qy[0]=qy[i]; ry[0]=ry[i]; n=1; break;
          } else if (y[i].zero()) {
            // Eliminate the zero
            y[i]=y[--n];
            qy[i]=qy[n];
            ry[i]=ry[n];
          }
        y.size(n);
      }
    }
    if ((x.size() == 0) && (y.size() == 0))
      return new (home) QBinOrTrue<VX,VY>(home,share,*this,x0,c_qx,c_rx,x1,c_qy,c_ry);
    else
      return new (home) QClauseTrue<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  inline ExecStatus
  QClauseTrue<VX,VY>::post(Home home, ViewArray<VX>& x, QuantArgs qx, IntArgs rx, ViewArray<VY>& y, QuantArgs qy, IntArgs ry) {
    int nbExists=0;
    for (int i=x.size(); i--; )
      if (x[i].one())
        return ES_OK;
      else if (x[i].zero())
      {
        x.move_lst(i);
        qx[i] = qx[x.size()-1];
        rx[i] = rx[x.size()-1];
      }
      else if (qx[i] == EXISTS)
        nbExists++;
    if (x.size() == 0)
      return QNaryOrTrue<VY>::post(home,y,qy,ry);
    for (int i=y.size(); i--; )
      if (y[i].one())
        return ES_OK;
      else if (y[i].zero())
      {
        y.move_lst(i);
        qy[i] = qy[y.size()];
        ry[i] = ry[y.size()];
      }
      else if (qy[i] == EXISTS)
        nbExists++;
    if (nbExists == 0) return ES_FAILED;
    if (y.size() == 0)
      return QNaryOrTrue<VX>::post(home,x,qx,rx);
    if ((x.size() == 1) && (y.size() == 1)) {
      return QBinOrTrue<VX,VY>::post(home,x[0],rx[0],qx[0],y[0],qy[0],ry[0]);
    } else if (!x.shared(home,y)) {
      (void) new (home) QClauseTrue(home,x,qx,rx,y,qy,ry);
    }
    return ES_OK;
  }

  template<class VX, class VY>
  forceinline size_t
  QClauseTrue<VX,VY>::dispose(Space& home) {
    (void) MixBinaryPropagator<VX,PC_BOOL_VAL,VY,PC_BOOL_VAL>::dispose(home);
    return sizeof(*this);
  }

  template<class VX, class VY>
  forceinline ExecStatus
  resubscribe(Space& home, Propagator& p,
              VX& x0, ViewArray<VX>& x, TQuantifier& c_qx, TQuantifier* qx, int& c_rx, int* rx,
              VY& x1, ViewArray<VY>& y, TQuantifier& c_qy, TQuantifier* qy, int& c_ry, int* ry) {
    if (x0.zero()) {
      int n = x.size();
      for (int i=n; i--; )
        if (x[i].one()) {
          x.size(n);
          return home.ES_SUBSUMED(p);
        } else if (x[i].zero()) {
          x[i] = x[--n]; qx[i] = qx[n]; rx[i] = rx[n];
        } else {
          // Rewrite if there is just one view left
          if ((i == 0) && (y.size() == 0)) {
            VX z = x[0]; x.size(0);
            GECODE_REWRITE(p,(QBinOrTrue<VX,VY>::post(home(p),z,qx[0],rx[0],x1,c_qy,c_ry)));
          }
          // Move to x0 and subscribe
          x0=x[i];c_qx=qx[i];c_rx=rx[i];
          x[i]=x[--n];qx[i]=qx[n];rx[i]=rx[n];
          x.size(n);
          x0.subscribe(home,p,PC_BOOL_VAL,false);
          return ES_FIX;
        }
      // All x-views have been assigned!
      ViewArray<VY> z(home,y.size()+1);
      QuantArgs qz(z.size());
      IntArgs rz(z.size());
      for (int i=y.size(); i--; )
      {
        z[i]=y[i]; qz[i]=qy[i]; rz[i]=ry[i];
      }
      z[y.size()] = x1; qz[y.size()] = c_qy;; rz[y.size()] = c_ry;
      GECODE_REWRITE(p,(QNaryOrTrue<VY>::post(home(p),z,qz,rz)));
    }
    return ES_FIX;
  }

  template<class VX, class VY>
  ExecStatus
  QClauseTrue<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    if (x0.one() || x1.one())
      return home.ES_SUBSUMED(*this);
    GECODE_ES_CHECK(resubscribe(home,*this,x0,x,c_qx,qx,c_rx,rx,x1,y,c_qy,qy,c_ry,ry));
    GECODE_ES_CHECK(resubscribe(home,*this,x1,y,c_qy,qy,c_ry,ry,x0,x,c_qx,qx,c_rx,rx));
    return ES_FIX;
  }


  /*
   * Quantified boolean clause propagator (disjunctive)
   *
   */

  /*
   * Index advisors
   *
   */
  template<class VX, class VY>
  forceinline
  QClause<VX,VY>::Tagged::Tagged(Space& home, Propagator& p,
                                Council<Tagged>& c, bool x0)
    : Advisor(home,p,c), x(x0) {}

  template<class VX, class VY>
  forceinline
  QClause<VX,VY>::Tagged::Tagged(Space& home, bool share, Tagged& a)
    : Advisor(home,share,a), x(a.x) {}

  template<class VX, class VY>
  forceinline
  QClause<VX,VY>::QClause(Home home, ViewArray<VX>& x0, const QuantArgs& qx0, const IntArgs& rx0, ViewArray<VY>& y0, const QuantArgs& qy0, const IntArgs& ry0,
                          VX z0)
    : Propagator(home), x(x0), qx(NULL), rx(NULL), y(y0), qy(NULL), ry(NULL), z(z0), n_zero(0), c(home) {
    if (x.size() > 0) {
      qx = static_cast<Space&>(home).alloc<TQuantifier>(x.size());
      rx = static_cast<Space&>(home).alloc<int>(x.size());
      for (int n=x.size(); n-- ; ) { qx[n] = qx0[n]; rx[n] = rx0[n]; }
    }
    if (y.size() > 0) {
      qy = static_cast<Space&>(home).alloc<TQuantifier>(y.size());
      ry = static_cast<Space&>(home).alloc<int>(y.size());
      for (int n=y.size(); n-- ; ) { qy[n] = qy0[n]; ry[n] = ry0[n]; }
    }
    x.subscribe(home,*new (home) Tagged(home,*this,c,true));
    y.subscribe(home,*new (home) Tagged(home,*this,c,false));
    z.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<class VX, class VY>
  forceinline
  QClause<VX,VY>::QClause(Space& home, bool share, QClause<VX,VY>& p)
    : Propagator(home,share,p), n_zero(p.n_zero) {
    if (p.x.size() > 0) {
      qx = home.alloc<TQuantifier>(p.x.size());
      rx = home.alloc<int>(p.x.size());
      for (int n=p.x.size(); n-- ; ) { qx[n] = p.qx[n]; rx[n] = p.rx[n]; }
    }
    if (p.y.size() > 0) {
      qy = home.alloc<TQuantifier>(p.y.size());
      ry = home.alloc<int>(p.y.size());
      for (int n=p.y.size(); n-- ; ) { qy[n] = p.qy[n]; ry[n] = p.ry[n]; }
    }
    x.update(home,share,p.x);
    y.update(home,share,p.y);
    z.update(home,share,p.z);
    c.update(home,share,p.c);
  }

  template<class VX>
  forceinline void
  eliminate_zero(ViewArray<VX>& x, TQuantifier *qx, int *rx, int& n_zero) {
    if (n_zero > 0) {
      int n=x.size();
      // Eliminate all zeros
      for (int i=n; i--; )
        if (x[i].zero()) {
          x[i]=x[--n]; qx[i]=qx[n]; rx[i]=rx[n]; n_zero--;
        }
      x.size(n);
    }
  }

  template<class VX, class VY>
  Actor*
  QClause<VX,VY>::copy(Space& home, bool share) {
    eliminate_zero(x,qx,rx,n_zero);
    eliminate_zero(y,qy,ry,n_zero);
    return new (home) QClause<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  inline ExecStatus
  QClause<VX,VY>::post(Home home, ViewArray<VX>& x, QuantArgs qx, IntArgs rx, ViewArray<VY>& y, QuantArgs qy, IntArgs ry, VX z) {
    assert(!x.shared(home) && !y.shared(home));
    if (z.one())
      return QClauseTrue<VX,VY>::post(home,x,qx,rx,y,qy,ry);
    if (z.zero()) {
      for (int i=x.size(); i--; )
      {
        if (qx[i] == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(x[i].zero(home));
      }
      for (int i=y.size(); i--; )
      {
        if (qy[i] == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(y[i].zero(home));
      }
      return ES_OK;
    }
    for (int i=x.size(); i--; )
      if (x[i].one()) {
        GECODE_ME_CHECK(z.one_none(home));
        return ES_OK;
      } else if (x[i].zero()) {
        x.move_lst(i);
        qx[i] = qx[x.size()];
        rx[i] = rx[x.size()];
      }
    if (x.size() == 0)
      return QNaryOr<VY,VX>::post(home,y,qy,ry,z);
    for (int i=y.size(); i--; )
      if (y[i].one()) {
        GECODE_ME_CHECK(z.one_none(home));
        return ES_OK;
      } else if (y[i].zero()) {
        y.move_lst(i);
        qy[i] = qy[y.size()];
        ry[i] = ry[y.size()];
      }
    if (y.size() == 0)
      return QNaryOr<VX,VX>::post(home,x,qx,rx,z);
    if ((x.size() == 1) && (y.size() == 1)) {
      return QOr<VX,VY,VX>::post(home,x[0],qx[0],rx[0],y[0],qy[0],ry[0],z);
    } else if (x.shared(home,y)) {
      GECODE_ME_CHECK(z.one_none(home));
    } else {
      (void) new (home) QClause<VX,VY>(home,x,qx,rx,y,qy,ry,z);
    }
    return ES_OK;
  }

  template<class VX, class VY>
  PropCost
  QClause<VX,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::LO);
  }

  template<class VX, class VY>
  forceinline void
  QClause<VX,VY>::cancel(Space& home) {
    for (Advisors<Tagged> as(c); as(); ++as) {
      if (as.advisor().x)
        x.cancel(home,as.advisor());
      else
        y.cancel(home,as.advisor());
      as.advisor().dispose(home,c);
    }
    c.dispose(home);
    z.cancel(home,*this,PC_BOOL_VAL);
  }

  template<class VX, class VY>
  forceinline size_t
  QClause<VX,VY>::dispose(Space& home) {
    cancel(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  template<class VX, class VY>
  ExecStatus
  QClause<VX,VY>::advise(Space&, Advisor& _a, const Delta& d) {
    Tagged& a = static_cast<Tagged&>(_a);
    // Decides whether the propagator must be run
    if ((a.x && VX::zero(d)) || (!a.x && VY::zero(d)))
      if (++n_zero < x.size() + y.size())
        return ES_FIX;
    return ES_NOFIX;
  }

  template<class VX, class VY>
  ExecStatus
  QClause<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    if (z.one())
      GECODE_REWRITE(*this,(QClauseTrue<VX,VY>::post(home(*this),x,QuantArgs(x.size(),qx),IntArgs(x.size(),rx),y,QuantArgs(y.size(),qy),IntArgs(y.size(),ry))));
    if (z.zero()) {
      for (int i = x.size(); i--; )
      {
        if (qx[i] == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(x[i].zero(home));
      }
      for (int i = y.size(); i--; )
      {
        if (qy[i] == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(y[i].zero(home));
      }
      c.dispose(home);
    } else if (n_zero == x.size() + y.size()) {
      GECODE_ME_CHECK(z.zero_none(home));
      c.dispose(home);
    } else {
      // There is exactly one view which is one
      GECODE_ME_CHECK(z.one_none(home));
    }
    return home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: int-prop
