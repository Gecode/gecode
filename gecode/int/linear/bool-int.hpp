/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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
  template <class VX>
  forceinline
  MemoryLinBoolInt<VX>::MemoryLinBoolInt(Space& home, ViewArray<VX>& x0,
                                         int n_s0, int c0)
    : Propagator(home), x(x0), n_s(n_s0), c(c0) {
    for (int i=n_s; i--; )
      x[i].subscribe(home,*this,PC_INT_VAL);
  }

  template <class VX>
  forceinline size_t
  MemoryLinBoolInt<VX>::dispose(Space& home) {
    assert(!home.failed());
    for (int i=n_s; i--; )
      x[i].cancel(home,*this,PC_INT_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class VX>
  forceinline
  MemoryLinBoolInt<VX>::MemoryLinBoolInt(Space& home, bool share, 
                                         MemoryLinBoolInt<VX>& p)
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

  template <class VX>
  PropCost
  MemoryLinBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return cost_lo(x.size(),PC_LINEAR_LO);
  }

  /*
   * Baseclass for integer Boolean sum using advisors
   *
   */
  template <class VX>
  forceinline
  SpeedLinBoolInt<VX>::SpeedLinBoolInt(Space& home, ViewArray<VX>& x0,
                                       int n_s0, int c0)
    : Propagator(home), x(x0), n_s(n_s0), c(c0), co(home) {
    int n_x = x.size() - n_s;
    for (int i=n_x; i<x.size(); i++)
      (void) new (home) ViewAdvisor<VX>(home,*this,co,x[i]);
    x.size(n_x);
  }

  template <class VX>
  forceinline size_t
  SpeedLinBoolInt<VX>::dispose(Space& home) {
    co.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class VX>
  forceinline
  SpeedLinBoolInt<VX>::SpeedLinBoolInt(Space& home, bool share, 
                                       SpeedLinBoolInt<VX>& p)
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
  
  template <class VX>
  PropCost
  SpeedLinBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return PC_UNARY_HI;
  }

  template <class VX>
  Reflection::ActorSpec
  SpeedLinBoolInt<VX>::spec(const Space& home, Reflection::VarMap& m,
                            const Support::Symbol& ati) const {
    Reflection::ActorSpec s(ati);
    Reflection::ArrayArg* a = 
      Reflection::Arg::newArray(x.size()+n_s);
    int i = 0;
    for (; i<x.size(); i++)
      (*a)[i] = x[i].spec(home, m);
    for (Advisors<ViewAdvisor<VX> > as(co); as(); ++as, ++i)
      (*a)[i] = as.advisor().view().spec(home, m);
    assert ( i == x.size() + n_s );
    return s << a << c;
  }

  /*
   * Greater or equal propagator (integer rhs)
   *
   */

  template <class VX>
  forceinline
  GqBoolInt<VX>::Memory::Memory(Space& home, ViewArray<VX>& x, int c)
    : MemoryLinBoolInt<VX>(home,x,c+1,c) {}

  template <class VX>
  forceinline
  GqBoolInt<VX>::Memory::Memory(Space& home, bool share, 
                                typename GqBoolInt<VX>::Memory& p)
    : MemoryLinBoolInt<VX>(home,share,p) {}

  template <class VX>
  Actor*
  GqBoolInt<VX>::Memory::copy(Space& home, bool share) {
    return new (home) Memory(home,share,*this);
  }

  template <class VX>
  ExecStatus
  GqBoolInt<VX>::Memory::propagate(Space& home, const ModEventDelta&) {
    // Eliminate assigned views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      if (x[i].zero()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
      } else if (x[i].one()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
      }
    x.size(n_x);
    if (n_x < c)
      return ES_FAILED;
    if (c <= 0)
      return ES_SUBSUMED(*this,home);
    // Find unassigned variables to subscribe to
    while ((n_s < n_x) && (n_s <= c)) 
      if (x[n_s].zero()) {
        x[n_s]=x[--n_x];
      } else if (x[n_s].one()) {
        x[n_s]=x[--n_x]; c--;
      } else {
        x[n_s++].subscribe(home,*this,PC_INT_VAL,false);
      }
    x.size(n_x);
    if (n_x < c)
      return ES_FAILED;
    if (c <= 0)
      return ES_SUBSUMED(*this,home);
    if (c == n_x) {
      // These are known to be not assigned
      for (int i=n_s; i--; )
        GECODE_ME_CHECK(x[i].one_none(home));
      // These are not known to be assigned
      for (int i=n_s+1; i<n_x; i++)
        GECODE_ME_CHECK(x[i].one(home));
      // Fix number of subscriptions such that cancelling subscriptions work
      n_s = 0;
      return ES_SUBSUMED(*this,sizeof(*this));
    }
    return ES_FIX;
  }


  template <class VX>
  forceinline
  GqBoolInt<VX>::Speed::Speed(Space& home, ViewArray<VX>& x, int c)
    : SpeedLinBoolInt<VX>(home,x,c+1,c) {}

  template <class VX>
  forceinline
  GqBoolInt<VX>::Speed::Speed(Space& home, bool share, 
                              typename GqBoolInt<VX>::Speed& p)
    : SpeedLinBoolInt<VX>(home,share,p) {}
  
  template <class VX>
  Actor*
  GqBoolInt<VX>::Speed::copy(Space& home, bool share) {
    return new (home) Speed(home,share,*this);
  }

  template <class VX>
  Reflection::ActorSpec
  GqBoolInt<VX>::Speed::spec(const Space& home, Reflection::VarMap& m) const {
    return SpeedLinBoolInt<VX>::spec(home, m, ati());
  }

  template <class VX>
  void
  GqBoolInt<VX>::Speed::post(Space& home, Reflection::VarMap& vars,
  const Reflection::ActorSpec& spec) {
    spec.checkArity(2);
    ViewArray<VX> x(home, vars, spec[0]);
    int c = spec[1]->toInt();
    GqBoolInt<VX>::post(home,x,c);
  }

  template <class VX>
  ExecStatus
  GqBoolInt<VX>::Speed::advise(Space& home, Advisor& _a, const Delta&) {
    ViewAdvisor<VX>& a = static_cast<ViewAdvisor<VX>&>(_a);
    if (n_s == 0)
      return ES_SUBSUMED_FIX(a,home,co);
    assert(!a.view().none());
    if (a.view().one()) {
      c--; goto subsume;
    }
    if (c+1 < n_s) 
      goto subsume;
    // Find a new subscription
    for (int i = x.size(); i--; )
      if (x[i].none()) {
        a.view(home,x[i]);
        x.size(i);
        return ES_FIX;
      } else if (x[i].one()) {
        c--;
        if (c+1 < n_s) {
          x.size(i);
          goto subsume;
        }
      }
    // No view left
    x.size(0);
  subsume:
    // Do not update subscription
    n_s--;
    int n = n_s+x.size();
    if (n < c)
      return ES_FAILED;
    if ((c <= 0) || (c == n))
      return ES_SUBSUMED_NOFIX(a,home,co);
    else
      return ES_SUBSUMED_FIX(a,home,co);
  }

  template <class VX>
  ExecStatus
  GqBoolInt<VX>::Speed::propagate(Space& home, const ModEventDelta&) {
    if (c > 0) {
      assert((n_s == c) && (x.size() == 0));
      // Signal to advisors that propagator runs
      n_s = 0;
      // All views must be one to satisfy inequality
      for (Advisors<ViewAdvisor<VX> > as(co); as(); ++as)
        GECODE_ME_CHECK(as.advisor().view().one_none(home));
    }
    return ES_SUBSUMED(*this,home);
  }

  template <class VX>
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
    if (x.size() > threshold)
      (void) new (home) Speed(home,x,c);
    else
      (void) new (home) Memory(home,x,c);
    return ES_OK;
  }

  template <class VX>
  ExecStatus
  GqBoolInt<VX>::Memory::post(Space& home, ViewArray<VX>& x, int c) {
    return GqBoolInt<VX>::post(home,x,c);
  }


  /*
   * Equal propagator (integer rhs)
   *
   */
  template <class VX>
  forceinline
  EqBoolInt<VX>::Memory::Memory(Space& home, ViewArray<VX>& x,
                                int c)
    : MemoryLinBoolInt<VX>(home,x,std::max(c,x.size()-c)+1,c) {}

  template <class VX>
  forceinline
  EqBoolInt<VX>::Memory::Memory(Space& home, bool share, 
                                typename EqBoolInt<VX>::Memory& p)
    : MemoryLinBoolInt<VX>(home,share,p) {}

  template <class VX>
  Actor*
  EqBoolInt<VX>::Memory::copy(Space& home, bool share) {
    return new (home) Memory(home,share,*this);
  }

  template <class VX>
  ExecStatus
  EqBoolInt<VX>::Memory::propagate(Space& home, const ModEventDelta&) {
    // Eliminate assigned views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      if (x[i].zero()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
      } else if (x[i].one()) {
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
      }
    x.size(n_x);
    if ((c < 0) || (c > n_x))
      return ES_FAILED;
    // Find unassigned variables to subscribe to
    while ((n_s < n_x) && ((n_s <= c) || (n_s <= n_x-c))) 
      if (x[n_s].zero()) {
        x[n_s]=x[--n_x];
      } else if (x[n_s].one()) {
        x[n_s]=x[--n_x]; c--;
      } else {
        x[n_s++].subscribe(home,*this,PC_INT_VAL,false);
      }
    x.size(n_x);
    if ((c < 0) || (c > n_x))
      return ES_FAILED;
    if (c == 0) {
      // These are known to be not assigned
      for (int i=n_s; i--; )
        GECODE_ME_CHECK(x[i].zero_none(home));
      // These are not known to be assigned
      for (int i=n_s+1; i<n_x; i++)
        GECODE_ME_CHECK(x[i].zero(home));
      // Fix number of subscriptions such that cancelling subscriptions work
      n_s = 0;
      return ES_SUBSUMED(*this,sizeof(*this));
    }
    if (c == n_x) {
      // These are known to be not assigned
      for (int i=n_s; i--; )
        GECODE_ME_CHECK(x[i].one_none(home));
      // These are not known to be assigned
      for (int i=n_s+1; i<n_x; i++)
        GECODE_ME_CHECK(x[i].one(home));
      // Fix number of subscriptions such that cancelling subscriptions work
      n_s = 0;
      return ES_SUBSUMED(*this,sizeof(*this));
    }
    return ES_FIX;
  }



  template <class VX>
  forceinline
  EqBoolInt<VX>::Speed::Speed(Space& home, ViewArray<VX>& x, int c)
    : SpeedLinBoolInt<VX>(home,x,std::max(c,x.size()-c)+1,c) {}

  template <class VX>
  forceinline
  EqBoolInt<VX>::Speed::Speed(Space& home, bool share, 
                              typename EqBoolInt<VX>::Speed& p)
    : SpeedLinBoolInt<VX>(home,share,p) {}
  
  template <class VX>
  Actor*
  EqBoolInt<VX>::Speed::copy(Space& home, bool share) {
    return new (home) Speed(home,share,*this);
  }

  template <class VX>
  Reflection::ActorSpec
  EqBoolInt<VX>::Speed::spec(const Space& home, Reflection::VarMap& m) const {
    return SpeedLinBoolInt<VX>::spec(home, m, ati());
  }

  template <class VX>
  void
  EqBoolInt<VX>::Speed::post(Space& home, Reflection::VarMap& vars,
                             const Reflection::ActorSpec& spec) {
    spec.checkArity(2);
    ViewArray<VX> x(home, vars, spec[0]);
    int c = spec[1]->toInt();
    EqBoolInt<VX>::post(home,x,c);
  }

  template <class VX>
  ExecStatus
  EqBoolInt<VX>::Speed::advise(Space& home, Advisor& _a, const Delta&) {
    ViewAdvisor<VX>& a = static_cast<ViewAdvisor<VX>&>(_a);
    if (n_s == 0)
      return ES_SUBSUMED_FIX(a,home,co);
    assert(!a.view().none());
    if (a.view().one())
      c--;
    //    std::max(c,x.size()-c)+1;
    if ((c+1 < n_s) && (x.size() < c))
      goto subsume;
    // Find a new subscription
    for (int i = x.size(); i--; )
      if (x[i].none()) {
        a.view(home,x[i]);
        x.size(i);
        return ES_FIX;
      } else if (x[i].one()) {
        c--;
      }
    // No view left
    x.size(0);
  subsume:
    // Do not update subscription
    n_s--;
    int n = n_s+x.size();
    if ((c<0) || (c > n))
      return ES_FAILED;
    if ((c == 0) || (c == n))
      return ES_SUBSUMED_NOFIX(a,home,co);
    else
      return ES_SUBSUMED_FIX(a,home,co);
  }

  template <class VX>
  ExecStatus
  EqBoolInt<VX>::Speed::propagate(Space& home, const ModEventDelta&) {
    // Signal to advisors that propagator runs
    assert(x.size() == 0);
    n_s = 0;
    if (c == 0) {
      // All views must be zero to satisfy equality
      for (Advisors<ViewAdvisor<VX> > as(co); as(); ++as)
        GECODE_ME_CHECK(as.advisor().view().zero_none(home));
    } else {
      // All views must be one to satisfy equality
      for (Advisors<ViewAdvisor<VX> > as(co); as(); ++as)
        GECODE_ME_CHECK(as.advisor().view().one_none(home));
    }
    return ES_SUBSUMED(*this,home);
  }

  template <class VX>
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
    if (x.size() > threshold)
      (void) new (home) Speed(home,x,c);
    else
      (void) new (home) Memory(home,x,c);
    return ES_OK;
  }

  template <class VX>
  ExecStatus
  EqBoolInt<VX>::Memory::post(Space& home, ViewArray<VX>& x, int c) {
    return EqBoolInt<VX>::post(home,x,c);
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
  forceinline ExecStatus
  NqBoolInt<VX>::post(Space& home, VX x0, VX x1, ViewArray<VX>& x, int c) {
    ViewArray<VX> xx(home, x.size()+2);
    for (int i=x.size(); i--;)
      xx[i] = x[i];
    xx[x.size()] = x0; xx[x.size()+1] = x1;
    return post(home, x, c);
  }

  template<class VX>
  Actor*
  NqBoolInt<VX>::copy(Space& home, bool share) {
    return new (home) NqBoolInt<VX>(home,share,*this);
  }

  template<class VX>
  PropCost
  NqBoolInt<VX>::cost(const Space&, const ModEventDelta&) const {
    return PC_LINEAR_LO;
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


}}}

// STATISTICS: int-prop

