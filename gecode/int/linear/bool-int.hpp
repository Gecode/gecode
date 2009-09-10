/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Tias Guns <tias.guns@cs.kuleuven.be>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *     Tias Guns, 2009
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include <algorithm>

namespace Gecode { namespace Int { namespace Linear {

  /*
   * Baseclass for integer Boolean sum using dependencies
   *
   */
  template<class VX>
  forceinline
  LinBoolInt<VX>::LinBoolInt(Space& home, ViewArray<VX>& x0,
                             int n_s, int c0)
    : Propagator(home), co(home), x(x0), n_as(n_s), n_hs(n_s), c(c0) {
    Advisor* a = new (home) Advisor(home,*this,co);
    for (int i=n_as; i--; )
      x[i].subscribe(home,*a);
  }

  template<class VX>
  forceinline void
  LinBoolInt<VX>::normalize(void) {
    if (n_as != n_hs) {
      // Remove views for which no more subscriptions exist
      int n_x = x.size();
      for (int i=n_hs; i--; )
        if (!x[i].none()) {
          x[i]=x[--n_hs]; x[n_hs]=x[--n_x];
        }
      n_hs = n_as; x.size(n_x);
    }
    // Remove assigned yet unsubscribed views
    int n_x = x.size();
    for (int i=n_x-1; i>=n_hs; i--)
      if (x[i].one()) {
        c--; x[i]=x[--n_x];
      } else if (x[i].zero()) {
        x[i]=x[--n_x];
      }
    x.size(n_x);
  }

  template<class VX>
  forceinline
  LinBoolInt<VX>::LinBoolInt(Space& home, bool share, LinBoolInt<VX>& p)
    : Propagator(home,share,p), n_as(p.n_as), n_hs(n_as) {
    p.normalize();
    c=p.c;
    co.update(home,share,p.co);
    x.update(home,share,p.x);
  }

  template<class VX>
  PropCost
  LinBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::HI);
  }

  template<class VX>
  forceinline size_t
  LinBoolInt<VX>::dispose(Space& home) {
    assert(!home.failed());
    Advisors<Advisor> as(co);
    for (int i=n_hs; i--; )
      if (x[i].none())
        x[i].cancel(home,as.advisor());
    co.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Baseclass for integer Boolean sum using dependencies
   *
   */
  template<class VX>
  forceinline
  SmallLinBoolInt<VX>::SmallLinBoolInt(Space& home, ViewArray<VX>& x0,
                                         int n_s0, int c0)
    : Propagator(home), x(x0), n_s(n_s0), c(c0) {
    for (int i=n_s; i--; )
      x[i].subscribe(home,*this,PC_INT_VAL);
  }

  template<class VX>
  forceinline size_t
  SmallLinBoolInt<VX>::dispose(Space& home) {
    assert(!home.failed());
    for (int i=n_s; i--; )
      x[i].cancel(home,*this,PC_INT_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class VX>
  forceinline
  SmallLinBoolInt<VX>::SmallLinBoolInt(Space& home, bool share,
                                         SmallLinBoolInt<VX>& p)
    : Propagator(home,share,p), x(home,p.x.size()), n_s(p.n_s) {
    // Update views not assigned and subscribed to
    for (int i=n_s; i--; )
      x[i].update(home,share,p.x[i]);
    // Eliminate assigned but not subscribed views in original
    // and update remaining ones
    int n_x = p.x.size();
    int p_c = p.c;
    for (int i=n_x; i-- > n_s; )
      if (p.x[i].zero()) {
        n_x--;
        p.x[i]=p.x[n_x]; x[i]=x[n_x];
      } else if (p.x[i].one()) {
        n_x--; p_c--;
        p.x[i]=p.x[n_x]; x[i]=x[n_x];
      } else {
        x[i].update(home,share,p.x[i]);
      }
    p.c = p_c; c = p_c;
    p.x.size(n_x); x.size(n_x);
  }

  template<class VX>
  PropCost
  SmallLinBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO, x.size());
  }

  /*
   * Baseclass for integer Boolean sum using advisors
   *
   */
  template<class VX>
  forceinline
  LargeLinBoolInt<VX>::LargeLinBoolInt(Space& home, ViewArray<VX>& x0,
                                       int n_s0, int c0)
    : Propagator(home), x(x0), n_s(n_s0), c(c0), co(home) {
    int n_x = x.size() - n_s;
    for (int i=n_x; i<x.size(); i++)
      (void) new (home) ViewAdvisor<VX>(home,*this,co,x[i]);
    x.size(n_x);
  }

  template<class VX>
  forceinline size_t
  LargeLinBoolInt<VX>::dispose(Space& home) {
    co.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class VX>
  forceinline
  LargeLinBoolInt<VX>::LargeLinBoolInt(Space& home, bool share,
                                       LargeLinBoolInt<VX>& p)
    : Propagator(home,share,p), x(home,p.x.size()), n_s(p.n_s) {
    // Eliminate assigned views in original and update remaining ones
    int n_x = p.x.size();
    int p_c = p.c;
    for (int i=n_x; i--; )
      if (p.x[i].zero()) {
        n_x--;
        p.x[i]=p.x[n_x]; x[i]=x[n_x];
      } else if (p.x[i].one()) {
        n_x--; p_c--;
        p.x[i]=p.x[n_x]; x[i]=x[n_x];
      } else {
        x[i].update(home,share,p.x[i]);
      }
    p.c = p_c; c = p_c;
    p.x.size(n_x); x.size(n_x);
    co.update(home,share,p.co);
  }

  template<class VX>
  PropCost
  LargeLinBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::HI);
  }

  /*
   * Greater or equal propagator (integer rhs)
   *
   */
  template<class VX>
  forceinline
  GqBoolInt<VX>::GqBoolInt(Space& home, ViewArray<VX>& x, int c)
    : LinBoolInt<VX>(home,x,c+1,c) {}

  template<class VX>
  forceinline
  GqBoolInt<VX>::GqBoolInt(Space& home, bool share, GqBoolInt<VX>& p)
    : LinBoolInt<VX>(home,share,p) {}

  template<class VX>
  Actor*
  GqBoolInt<VX>::copy(Space& home, bool share) {
    return new (home) GqBoolInt<VX>(home,share,*this);
  }

  template<class VX>
  ExecStatus
  GqBoolInt<VX>::advise(Space& home, Advisor& a, const Delta& d) {
    // Check whether propagator is running
    if (n_as == 0)
      return ES_FIX;

    if (VX::one(d)) {
      c--; goto check;
    }
    if (c+1 < n_as)
      goto check;
    // Find a new subscription
    for (int i = x.size()-1; i>=n_hs; i--)
      if (x[i].none()) {
        std::swap(x[i],x[n_hs]);
        x[n_hs++].subscribe(home,a);
        x.size(i+1);
        return ES_FIX;
      } else if (x[i].one()) {
        c--;
        if (c+1 < n_as) {
          x.size(i);
          assert(n_hs <= x.size());
          goto check;
        }
      }
    // No view left for subscription
    x.size(n_hs);
  check:
    // Do not update subscription
    n_as--;
    int n = x.size()-n_hs+n_as;
    if (n < c)
      return ES_FAILED;
    if ((c <= 0) || (c == n))
      return ES_NOFIX;
    else
      return ES_FIX;
  }

  template<class VX>
  ExecStatus
  GqBoolInt<VX>::propagate(Space& home, const ModEventDelta&) {
    if (c > 0) {
      assert((n_as == c) && (x.size() == n_hs));
      // Signal that propagator is running
      n_as = 0;
      // All views must be one to satisfy inequality
      for (int i=n_hs; i--; )
        if (x[i].none())
          GECODE_ME_CHECK(x[i].one_none(home));
    } else {
      Advisors<Advisor> as(co);
      for (int i=n_hs; i--; )
        if (x[i].none())
          x[i].cancel(home,as.advisor());
    }
    co.dispose(home);
    return ES_SUBSUMED(*this,sizeof(*this));
  }

  template<class VX>
  ExecStatus
  GqBoolInt<VX>::post(Space& home, ViewArray<VX>& x, int c) {
    // Eliminate assigned views
    int n_x = x.size();
    for (int i=n_x; i--; )
      if (x[i].zero()) {
        x[i] = x[--n_x];
      } else if (x[i].one()) {
        x[i] = x[--n_x]; c--;
      }
    // RHS too large
    if (n_x < c)
      return ES_FAILED;
    // Whatever the x[i] take for values, the inequality is subsumed
    if (c <= 0)
      return ES_OK;
    // All views must be one to satisfy inequality
    if (c == n_x) {
      for (int i=n_x; i--; )
        GECODE_ME_CHECK(x[i].one_none(home));
      return ES_OK;
    }
    // This is the needed invariant as c+1 subscriptions must be created
    assert(n_x > c);
    x.size(n_x);
    (void) new (home) GqBoolInt<VX>(home,x,c);
    return ES_OK;
  }




  /*
   * Equal propagator (integer rhs)
   *
   */
  template<class VX>
  forceinline
  EqBoolInt<VX>::EqBoolInt(Space& home, ViewArray<VX>& x, int c)
    : LinBoolInt<VX>(home,x,std::max(c,x.size()-c)+1,c) {}

  template<class VX>
  forceinline
  EqBoolInt<VX>::EqBoolInt(Space& home, bool share, EqBoolInt<VX>& p)
    : LinBoolInt<VX>(home,share,p) {}

  template<class VX>
  Actor*
  EqBoolInt<VX>::copy(Space& home, bool share) {
    return new (home) EqBoolInt<VX>(home,share,*this);
  }

  template<class VX>
  ExecStatus
  EqBoolInt<VX>::advise(Space& home, Advisor& a, const Delta& d) {
    // Check whether propagator is running
    if (n_as == 0)
      return ES_FIX;

    if (VX::one(d))
      c--;
    if ((c+1 < n_as) && (x.size()-n_hs < c))
      goto check;
    // Find a new subscription
    for (int i = x.size()-1; i>=n_hs; i--)
      if (x[i].none()) {
        std::swap(x[i],x[n_hs]);
        x[n_hs++].subscribe(home,a);
        x.size(i+1);
        return ES_FIX;
      } else if (x[i].one()) {
        c--;
      }
    // No view left for subscription
    x.size(n_hs);
  check:
    // Do not update subscription
    n_as--;
    int n = x.size()-n_hs+n_as;
    if ((c < 0) || (c > n))
      return ES_FAILED;
    if ((c == 0) || (c == n))
      return ES_NOFIX;
    else
      return ES_FIX;
  }

  template<class VX>
  ExecStatus
  EqBoolInt<VX>::propagate(Space& home, const ModEventDelta&) {
    assert(x.size() == n_hs);
    // Signal that propagator is running
    n_as = 0;
    if (c == 0) {
      // All views must be zero to satisfy equality
      for (int i=n_hs; i--; )
        if (x[i].none())
          GECODE_ME_CHECK(x[i].zero_none(home));
    } else {
      // All views must be one to satisfy equality
      for (int i=n_hs; i--; )
        if (x[i].none())
          GECODE_ME_CHECK(x[i].one_none(home));
    }
    co.dispose(home);
    return ES_SUBSUMED(*this,sizeof(*this));
  }

  template<class VX>
  ExecStatus
  EqBoolInt<VX>::post(Space& home, ViewArray<VX>& x, int c) {
    // Eliminate assigned views
    int n_x = x.size();
    for (int i=n_x; i--; )
      if (x[i].zero()) {
        x[i] = x[--n_x];
      } else if (x[i].one()) {
        x[i] = x[--n_x]; c--;
      }
    // RHS too small or too large
    if ((c < 0) || (c > n_x))
      return ES_FAILED;
    // All views must be zero to satisfy equality
    if (c == 0) {
      for (int i=n_x; i--; )
        GECODE_ME_CHECK(x[i].zero_none(home));
      return ES_OK;
    }
    // All views must be one to satisfy equality
    if (c == n_x) {
      for (int i=n_x; i--; )
        GECODE_ME_CHECK(x[i].one_none(home));
      return ES_OK;
    }
    x.size(n_x);
    (void) new (home) EqBoolInt<VX>(home,x,c);
    return ES_OK;
  }


  /*
   * Integer disequal to Boolean sum
   *
   */

  template<class VX>
  forceinline
  NqBoolInt<VX>::NqBoolInt(Space& home, ViewArray<VX>& b, int c0)
    : BinaryPropagator<VX,PC_INT_VAL>(home,
                                      b[b.size()-2],
                                      b[b.size()-1]), x(b), c(c0) {
    assert(x.size() >= 2);
    x.size(x.size()-2);
  }

  template<class VX>
  forceinline
  NqBoolInt<VX>::NqBoolInt(Space& home, bool share, NqBoolInt<VX>& p)
    : BinaryPropagator<VX,PC_INT_VAL>(home,share,p), x(home,p.x.size()) {
    // Eliminate all zeros and ones in original and update
    int n = p.x.size();
    int p_c = p.c;
    for (int i=n; i--; )
      if (p.x[i].zero()) {
        n--; p.x[i]=p.x[n]; x[i]=x[n];
      } else if (p.x[i].one()) {
        n--; p_c--; p.x[i]=p.x[n]; x[i]=x[n];
      } else {
        x[i].update(home,share,p.x[i]);
      }
    c = p_c; p.c = p_c;
    x.size(n); p.x.size(n);
  }

  template<class VX>
  forceinline ExecStatus
  NqBoolInt<VX>::post(Space& home, ViewArray<VX>& x, int c) {
    int n = x.size();
    for (int i=n; i--; )
      if (x[i].one()) {
        x[i]=x[--n]; c--;
      } else if (x[i].zero()) {
        x[i]=x[--n];
      }
    x.size(n);
    if ((n < c) || (c < 0))
      return ES_OK;
    if (n == 0)
      return (c == 0) ? ES_FAILED : ES_OK;
    if (n == 1) {
      if (c == 1) {
        GECODE_ME_CHECK(x[0].zero_none(home));
      } else {
        GECODE_ME_CHECK(x[0].one_none(home));
      }
      return ES_OK;
    }
    (void) new (home) NqBoolInt(home,x,c);
    return ES_OK;
  }

  template<class VX>
  Actor*
  NqBoolInt<VX>::copy(Space& home, bool share) {
    return new (home) NqBoolInt<VX>(home,share,*this);
  }

  template<class VX>
  PropCost
  NqBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO, x.size());
  }

  template<class VX>
  forceinline bool
  NqBoolInt<VX>::resubscribe(Space& home, VX& y) {
    if (y.one())
      c--;
    int n = x.size();
    for (int i=n; i--; )
      if (x[i].one()) {
        c--; x[i]=x[--n];
      } else if (x[i].zero()) {
        x[i] = x[--n];
      } else {
        // New unassigned view found
        assert(!x[i].zero() && !x[i].one());
        // Move to y and subscribe
        y=x[i]; x[i]=x[--n];
        x.size(n);
        y.subscribe(home,*this,PC_INT_VAL,false);
        return true;
      }
    // All views have been assigned!
    x.size(0);
    return false;
  }

  template<class VX>
  ExecStatus
  NqBoolInt<VX>::propagate(Space& home, const ModEventDelta&) {
    bool s0 = true;
    if (x0.zero() || x0.one())
      s0 = resubscribe(home,x0);
    bool s1 = true;
    if (x1.zero() || x1.one())
      s1 = resubscribe(home,x1);
    int n = x.size() + s0 + s1;
    if ((n < c) || (c < 0))
      return ES_SUBSUMED(*this,home);
    if (n == 0)
      return (c == 0) ? ES_FAILED : ES_SUBSUMED(*this,sizeof(*this));
    if (n == 1) {
      if (s0) {
        if (c == 1) {
          GECODE_ME_CHECK(x0.zero_none(home));
        } else {
          GECODE_ME_CHECK(x0.one_none(home));
        }
      } else {
        assert(s1);
        if (c == 1) {
          GECODE_ME_CHECK(x1.zero_none(home));
        } else {
          GECODE_ME_CHECK(x1.one_none(home));
        }
      }
      return ES_SUBSUMED(*this,sizeof(*this));
    }
    return ES_FIX;
  }

  //** REIFIED **//

  /*
   * Baseclass for reified integer Boolean sum using dependencies
   *
   */
  template<class VX, class VB>
  forceinline
  SmallReLinBoolInt<VX,VB>::SmallReLinBoolInt(Space& home, ViewArray<VX>& x0,
                                         int n_s0, int c0, VB b0)
    : SmallLinBoolInt<VX>(home,x0,n_s0,c0), b(b0) {
    b.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<class VX, class VB>
  forceinline size_t
  SmallReLinBoolInt<VX,VB>::dispose(Space& home) {
    assert(!home.failed());
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) SmallLinBoolInt<VX>::dispose(home);
    return sizeof(*this);
  }

  template<class VX, class VB>
  forceinline
  SmallReLinBoolInt<VX,VB>::SmallReLinBoolInt(Space& home, bool share, 
                                         SmallReLinBoolInt<VX,VB>& p)
    : SmallLinBoolInt<VX>(home,share,p) {
    b.update(home,share,p.b);
  }


  /*
   * Baseclass for reified integer Boolean sum using advisors
   *
   */
  template<class VX, class VB>
  forceinline
  LargeReLinBoolInt<VX,VB>::LargeReLinBoolInt(Space& home, ViewArray<VX>& x,
                                              int c0, VB b0)
    : Propagator(home), n(x.size()), c(c0), co(home), b(b0) {
    for (int i=n; i--; )
      (void) new (home) ViewAdvisor<VX>(home,*this,co,x[i]);
    b.subscribe(home,*this,PC_BOOL_VAL);
  }

  template<class VX, class VB>
  forceinline
  LargeReLinBoolInt<VX,VB>::LargeReLinBoolInt(Space& home, bool share, 
                                              LargeReLinBoolInt<VX,VB>& p)
    : Propagator(home,share,p), n(p.n), c(p.c) {
    co.update(home,share,p.co);
    b.update(home,share,p.b);
  }

  template<class VX, class VB>
  forceinline size_t
  LargeReLinBoolInt<VX,VB>::dispose(Space& home) {
    assert(!home.failed());
    co.dispose(home);
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class VX, class VB>
  PropCost
  LargeReLinBoolInt<VX,VB>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::HI);
  }


  /*
   * Reified greater propagator (integer rhs)
   * 
   */

  template<class VX, class VB>
  forceinline
  ReGqBoolInt<VX,VB>::Small::Small(Space& home, ViewArray<VX>& x, int c, VB b)
  // max(c,|x|-c)+1 because c might be a low number and we need to propagate to 'b' as fast as possible. min(|x|,val) to avoid overflow
    : SmallReLinBoolInt<VX,VB>
        (home,x,std::min(x.size(),std::max(c,x.size()-c)+1),c,b) {}


  template<class VX, class VB>
  forceinline
  ReGqBoolInt<VX,VB>::Small::Small(Space& home, bool share, 
                                typename ReGqBoolInt<VX,VB>::Small& p)
    : SmallReLinBoolInt<VX,VB>(home,share,p) {}

  template<class VX, class VB>
  Actor*
  ReGqBoolInt<VX,VB>::Small::copy(Space& home, bool share) {
    return new (home) Small(home,share,*this);
  }

  template<class VX, class VB>
  inline ExecStatus
  ReGqBoolInt<VX,VB>::Small::rewrite_inverse(Space& home, 
                                              ViewArray<BoolView>& x, int c) {
    ViewArray<NegBoolView> y(home,x.size());
    for (int i=x.size(); i--; )
      y[i]=x[i];
    GECODE_REWRITE(*this,(GqBoolInt<NegBoolView>::post(home,y,x.size()-c+1)));
  }

  template<class VX, class VB>
  inline ExecStatus
  ReGqBoolInt<VX,VB>::Small::rewrite_inverse(Space& home, 
                                              ViewArray<NegBoolView>& x, int c) {
    ViewArray<BoolView> y(home,x.size());
    for (int i=x.size(); i--; )
      y[i]=x[i].base();
    GECODE_REWRITE(*this,(GqBoolInt<BoolView>::post(home,y,x.size()-c+1)));
  }

  template<class VX, class VB>
  ExecStatus
  ReGqBoolInt<VX,VB>::Small::propagate(Space& home, const ModEventDelta&) {
    // Eliminate assigned views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      if (x[i].zero()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
      } else if (x[i].one()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
      }
    x.size(n_x);

    // b got assigned, subsume or rewrite
    if (b.one())
      GECODE_REWRITE(*this,(GqBoolInt<VX>::post(home,x,c)));
    if (b.zero())
      return rewrite_inverse(home, x, c);

    if (n_x < c) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(*this,home);
    }
    if (c <= 0) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_SUBSUMED(*this,home);
    }
    // Find unassigned variables to subscribe to
    int n_s0 = c; // minimal number of subscriptions !
    // max(c,|x|-c)+1 because c might be a low number and we need to propagate to 'b' as fast as possible. min(|x|,val) to avoid overflow
    n_s0 = std::min(x.size(),std::max(c,x.size()-c)+1);

    while ((n_s < n_x) && (n_s <= n_s0)) 
      if (x[n_s].zero()) {
        x[n_s]=x[--n_x];
      } else if (x[n_s].one()) {
        x[n_s]=x[--n_x]; c--;
      } else {
        x[n_s++].subscribe(home,*this,PC_INT_VAL,false);
      }
    x.size(n_x);
    if (n_x < c) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(*this,home);
    }
    if (c <= 0) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_SUBSUMED(*this,home);
    }
    return ES_FIX;
  }

  template<class VX, class VB>
  forceinline
  ReGqBoolInt<VX,VB>::Large::Large(Space& home, ViewArray<VX>& x, int c, VB b)
    : LargeReLinBoolInt<VX,VB>(home,x,c,b) {}

  template<class VX, class VB>
  forceinline
  ReGqBoolInt<VX,VB>::Large::Large(Space& home, bool share, 
                                   typename ReGqBoolInt<VX,VB>::Large& p)
    : LargeReLinBoolInt<VX,VB>(home,share,p) {}

  template<class VX, class VB>
  Actor*
  ReGqBoolInt<VX,VB>::Large::copy(Space& home, bool share) {
    return new (home) Large(home,share,*this);
  }


  template<class VX, class VB>
  inline ExecStatus
  ReGqBoolInt<VX,VB>::Large::rewrite_inverse(Space& home, 
                                             ViewArray<BoolView>& x, int c) {
    ViewArray<NegBoolView> y(home,x.size());
    for (int i=x.size(); i--; )
      y[i]=x[i];
    GECODE_REWRITE(*this,(GqBoolInt<NegBoolView>::post(home,y,x.size()-c+1)));
  }

  template<class VX, class VB>
  inline ExecStatus
  ReGqBoolInt<VX,VB>::Large::rewrite_inverse(Space& home, 
                                             ViewArray<NegBoolView>& x, int c) {
    ViewArray<BoolView> y(home,x.size());
    for (int i=x.size(); i--; )
      y[i]=x[i].base();
    GECODE_REWRITE(*this,(GqBoolInt<BoolView>::post(home,y,x.size()-c+1)));
  }


  template<class VX, class VB>
  ExecStatus
  ReGqBoolInt<VX,VB>::Large::advise(Space& home, Advisor& _a, const Delta&) {
    ViewAdvisor<VX>& a = static_cast<ViewAdvisor<VX>&>(_a);

    // eliminate view
    assert(!a.view().none());
    if (a.view().one())
      c--;
    n--;
    if ((n < c) || (c <= 0))
      return ES_SUBSUMED_NOFIX(a,home,co);
    else
      return ES_SUBSUMED_FIX(a,home,co);
  }


  template<class VX, class VB>
  ExecStatus
  ReGqBoolInt<VX,VB>::Large::propagate(Space& home, const ModEventDelta&) {
    if (b.none()) {
      if (c <= 0) {
        GECODE_ME_CHECK(b.one_none(home));
      } else {
        GECODE_ME_CHECK(b.zero_none(home));
      }
      return ES_SUBSUMED(*this,home);
    }
    
    ViewArray<VX> x(home, n);
    int i=0;
    for (Advisors<ViewAdvisor<VX> > as(co); as(); ++as)
      x[i++] = as.advisor().view();
    assert(i == n);

    if (b.one()) {
      GECODE_REWRITE(*this,(GqBoolInt<VX>::post(home,x,c)));
    } else {
      assert(b.zero());
      return rewrite_inverse(home,x,c);
    }      
  }

  template<class VX, class VB>
  ExecStatus
  ReGqBoolInt<VX,VB>::post(Space& home, ViewArray<VX>& x, int c, VB b) {
    assert(!b.assigned()); // checked before posting

    // Eliminate assigned views
    int n_x = x.size();
    for (int i=n_x; i--; )
      if (x[i].zero()) {
        x[i] = x[--n_x];
      } else if (x[i].one()) {
        x[i] = x[--n_x]; c--;
      }
    // RHS too large
    if (n_x < c) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_OK;
    }
    // Whatever the x[i] take for values, the inequality is subsumed
    if (c <= 0) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_OK;
    }
    // This is the needed invariant as c subscriptions must be created (c+1 in non-reified)
    assert(n_x >= c);
    x.size(n_x);

    if (x.size() >= threshold)
      (void) new (home) Large(home,x,c,b);
    else
      (void) new (home) Small(home,x,c,b);
    return ES_OK;
  }


  /*
   * Reified equal propagator (integer rhs)
   * 
   */

  template<class VX, class VB>
  forceinline
  ReEqBoolInt<VX,VB>::Small::Small(Space& home, ViewArray<VX>& x, int c, VB b)
  // max(c,|x|-c)+1 because c might be a low number and we need to propagate to 'b' as fast as possible. min(|x|,val) to avoid overflow
    : SmallReLinBoolInt<VX,VB>
        (home,x,std::min(x.size(),std::max(c,x.size()-c)+1),c,b) {}

  template<class VX, class VB>
  forceinline
  ReEqBoolInt<VX,VB>::Small::Small(Space& home, bool share, 
                                typename ReEqBoolInt<VX,VB>::Small& p)
    : SmallReLinBoolInt<VX,VB>(home,share,p) {}

  template<class VX, class VB>
  Actor*
  ReEqBoolInt<VX,VB>::Small::copy(Space& home, bool share) {
    return new (home) Small(home,share,*this);
  }

  template<class VX, class VB>
  ExecStatus
  ReEqBoolInt<VX,VB>::Small::propagate(Space& home, const ModEventDelta&) {
    // Eliminate assigned views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      if (x[i].zero()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
      } else if (x[i].one()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
      }
    x.size(n_x);

    // b got assigned, subsume or rewrite
    if (b.one())
      GECODE_REWRITE(*this,(EqBoolInt<VX>::post(home,x,c)));
    if (b.zero())
      GECODE_REWRITE(*this,(NqBoolInt<VX>::post(home,x,c)));

    if ((c < 0) || (c > n_x)) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(*this,home);
    }
    // Find unassigned variables to subscribe to
    int n_s0 = c; // minimal number of subscriptions !
    // max(c,|x|-c)+1 because c might be a low number and we need to propagate to 'b' as fast as possible. min(|x|,val) to avoid overflow
    n_s0 = std::min(x.size(),std::max(c,x.size()-c)+1);

    while ((n_s < n_x) && (n_s <= n_s0)) 
      if (x[n_s].zero()) {
        x[n_s]=x[--n_x];
      } else if (x[n_s].one()) {
        x[n_s]=x[--n_x]; c--;
      } else {
        x[n_s++].subscribe(home,*this,PC_INT_VAL,false);
      }
    x.size(n_x);
    if ((c < 0) || (c > n_x)) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(*this,home);
    }
    if (c == 0 && n_x == 0) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_SUBSUMED(*this,home);
    }
    return ES_FIX;
  }



  template<class VX, class VB>
  forceinline
  ReEqBoolInt<VX,VB>::Large::Large(Space& home, ViewArray<VX>& x, int c, VB b)
    : LargeReLinBoolInt<VX,VB>(home,x,c,b) {}

  template<class VX, class VB>
  forceinline
  ReEqBoolInt<VX,VB>::Large::Large(Space& home, bool share, 
                                   typename ReEqBoolInt<VX,VB>::Large& p)
    : LargeReLinBoolInt<VX,VB>(home,share,p) {}

  template<class VX, class VB>
  Actor*
  ReEqBoolInt<VX,VB>::Large::copy(Space& home, bool share) {
    return new (home) Large(home,share,*this);
  }

  template<class VX, class VB>
  ExecStatus
  ReEqBoolInt<VX,VB>::Large::advise(Space& home, Advisor& _a, const Delta&) {
    ViewAdvisor<VX>& a = static_cast<ViewAdvisor<VX>&>(_a);

    // Eliminate view
    assert(!a.view().none());
    if (a.view().one())
      c--;
    n--;

    if ((c < 0) || (c > n) || (n == 0))
      return ES_SUBSUMED_NOFIX(a,home,co);
    else
      return ES_SUBSUMED_FIX(a,home,co);
  }

  template<class VX, class VB>
  ExecStatus
  ReEqBoolInt<VX,VB>::Large::propagate(Space& home, const ModEventDelta&) {
    if (b.none()) {
      if ((c == 0) && (n == 0)) {
        GECODE_ME_CHECK(b.one_none(home));
      } else {
        GECODE_ME_CHECK(b.zero_none(home));
      }
      return ES_SUBSUMED(*this,home);
    }
     
    ViewArray<VX> x(home, n);
    int i = 0;
    for (Advisors<ViewAdvisor<VX> > as(co); as(); ++as)
      x[i++] = as.advisor().view();
    assert(i == n);
    
    if (b.one()) {
      GECODE_REWRITE(*this,(EqBoolInt<VX>::post(home,x,c)));
    } else {
      assert(b.zero());
      GECODE_REWRITE(*this,(NqBoolInt<VX>::post(home,x,c)));
    }
  }

  template<class VX, class VB>
  ExecStatus
  ReEqBoolInt<VX,VB>::post(Space& home, ViewArray<VX>& x, int c, VB b) {
    assert(!b.assigned()); // checked before posting

    // Eliminate assigned views
    int n_x = x.size();
    for (int i=n_x; i--; )
      if (x[i].zero()) {
        x[i] = x[--n_x];
      } else if (x[i].one()) {
        x[i] = x[--n_x]; c--;
      }
    // RHS too large
    if ((n_x < c) || (c < 0)) {
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_OK;
    }
    // all variables set, and c == 0: equality
    if ((c == 0) && (n_x == 0)) {
      GECODE_ME_CHECK(b.one_none(home));
      return ES_OK;
    }

    x.size(n_x);

    if (x.size() >= threshold)
      (void) new (home) Large(home,x,c,b);
    else
      (void) new (home) Small(home,x,c,b);
    return ES_OK;
  }


}}}

// STATISTICS: int-prop

