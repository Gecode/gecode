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

namespace Gecode { namespace Int { namespace Count {

  /*
   * General baseclass
   *
   */

  template<class VX, class VY>
  forceinline
  BaseInt<VX,VY>::BaseInt(Home home,
                          ViewArray<VX>& x0, int n_s0, VY y0, int c0)
    : Propagator(home), x(x0), n_s(n_s0), y(y0), c(c0) {
    if (notice(y))
      home.notice(*this,AP_DISPOSE);
    for (int i=n_s; i--; )
      x[i].subscribe(home,*this,PC_INT_DOM);
    subscribe(home,*this,y);
  }

  template<class VX, class VY>
  forceinline size_t
  BaseInt<VX,VY>::dispose(Space& home) {
    if (notice(y))
      home.ignore(*this,AP_DISPOSE);
    for (int i=n_s; i--; )
      x[i].cancel(home,*this,PC_INT_DOM);
    cancel(home,*this,y);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class VX, class VY>
  forceinline
  BaseInt<VX,VY>::BaseInt(Space& home, bool share, BaseInt<VX,VY>& p)
    : Propagator(home,share,p), n_s(p.n_s), c(p.c) {
    x.update(home,share,p.x);
    y.update(home,share,p.y);
  }

  template<class VX, class VY>
  PropCost
  BaseInt<VX,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  /*
   * Equal propagator (integer rhs)
   *
   */
  template<class VX, class VY>
  forceinline
  EqInt<VX,VY>::EqInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c)
    : BaseInt<VX,VY>(home,x,n_s,y,c) {}

  template<class VX, class VY>
  ExecStatus
  EqInt<VX,VY>::post(Home home, ViewArray<VX>& x, VY y, int c) {
    // Eliminate decided views
    int n_x = x.size();
    for (int i=n_x; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i] = x[--n_x]; break;
      case RT_TRUE:
        x[i] = x[--n_x]; c--; break;
      case RT_MAYBE:
        break;
      default:
        GECODE_NEVER;
      }
    x.size(n_x);
    // RHS too small or too large
    if ((c < 0) || (c > n_x))
      return ES_FAILED;
    // All views must be different
    if (c == 0)
      return post_false(home,x,y);
    // All views must be equal
    if (c == n_x)
      return post_true(home,x,y);
    // Compute how many subscriptions must be created
    int n_s = std::max(c,n_x-c)+1;
    assert(n_s <= n_x);
    (void) new (home) EqInt<VX,VY>(home,x,n_s,y,c);
    return ES_OK;
  }

  template<class VX, class VY>
  forceinline
  EqInt<VX,VY>::EqInt(Space& home, bool share, EqInt<VX,VY>& p)
    : BaseInt<VX,VY>(home,share,p) {}

  template<class VX, class VY>
  Actor*
  EqInt<VX,VY>::copy(Space& home, bool share) {
    return new (home) EqInt<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  ExecStatus
  EqInt<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    // Eliminate decided views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i].cancel(home,*this,PC_INT_DOM);
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
        break;
      case RT_TRUE:
        x[i].cancel(home,*this,PC_INT_DOM);
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
        break;
      case RT_MAYBE:
        break;
      default:
        GECODE_NEVER;
      }
    x.size(n_x);
    if ((c < 0) || (c > n_x))
      return ES_FAILED;
    // Eliminate decided views from unsubscribed views
    for (int i=n_x; i-- > n_s; )
      switch (holds(x[i],y)) {
      case RT_FALSE: x[i]=x[--n_x]; break;
      case RT_TRUE:  x[i]=x[--n_x]; c--; break;
      case RT_MAYBE: break;
      default:       GECODE_NEVER;
      }
    x.size(n_x);
    if ((c < 0) || (c > n_x))
      return ES_FAILED;
    if (c == 0) {
      // All views must be different
      GECODE_ES_CHECK(post_false(home,x,y));
      return home.ES_SUBSUMED(*this);
    }
    if (c == n_x) {
      // All views must be equal
      GECODE_ES_CHECK(post_true(home,x,y));
      return home.ES_SUBSUMED(*this);
    }
    int m = std::max(c,n_x-c)+1;
    assert(m <= n_x);
    // Now, there must be new subscriptions from x[n_s] up to x[m-1]
    while (n_s < m)
      x[n_s++].subscribe(home,*this,PC_INT_DOM,false);
    return ES_FIX;
  }

  /*
   * Greater or equal propagator (integer rhs)
   *
   */
  template<class VX, class VY>
  forceinline
  GqInt<VX,VY>::GqInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c)
    : BaseInt<VX,VY>(home,x,n_s,y,c) {}

  template<class VX, class VY>
  ExecStatus
  GqInt<VX,VY>::post(Home home, ViewArray<VX>& x, VY y, int c) {
    // Eliminate decided views
    int n_x = x.size();
    for (int i=n_x; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i] = x[--n_x]; break;
      case RT_TRUE:
        x[i] = x[--n_x]; c--; break;
      case RT_MAYBE:
        break;
      default:
        GECODE_NEVER;
      }
    x.size(n_x);
    // RHS too large
    if (n_x < c)
      return ES_FAILED;
    // Whatever the x[i] take for values, the inequality is subsumed
    if (c <= 0)
      return ES_OK;
    // All views must be equal
    if (c == n_x)
      return post_true(home,x,y);
    (void) new (home) GqInt<VX,VY>(home,x,c+1,y,c);
    return ES_OK;
  }

  template<class VX, class VY>
  forceinline
  GqInt<VX,VY>::GqInt(Space& home, bool share, GqInt<VX,VY>& p)
    : BaseInt<VX,VY>(home,share,p) {}

  template<class VX, class VY>
  Actor*
  GqInt<VX,VY>::copy(Space& home, bool share) {
    return new (home) GqInt<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  ExecStatus
  GqInt<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    // Eliminate decided views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i].cancel(home,*this,PC_INT_DOM);
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
        break;
      case RT_TRUE:
        x[i].cancel(home,*this,PC_INT_DOM);
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
        break;
      case RT_MAYBE:
        break;
      default:
        GECODE_NEVER;
      }
    x.size(n_x);
    if (n_x < c)
      return ES_FAILED;
    if (c <= 0)
      return home.ES_SUBSUMED(*this);
    // Eliminate decided views from unsubscribed views
    for (int i=n_x; i-- > n_s; )
      switch (holds(x[i],y)) {
      case RT_FALSE: x[i]=x[--n_x]; break;
      case RT_TRUE:  x[i]=x[--n_x]; c--; break;
      case RT_MAYBE: break;
      default:       GECODE_NEVER;
      }
    x.size(n_x);
    if (n_x < c)
      return ES_FAILED;
    if (c <= 0)
      return home.ES_SUBSUMED(*this);
    if (c == n_x) {
      // All views must be equal
      GECODE_ES_CHECK(post_true(home,x,y)); 
      return home.ES_SUBSUMED(*this);
    }
    // Now, there must be new subscriptions from x[n_s] up to x[c+1]
    while (n_s <= c)
      x[n_s++].subscribe(home,*this,PC_INT_DOM,false);
    return ES_FIX;
  }

  /*
   * Less or equal propagator (integer rhs)
   *
   */
  template<class VX, class VY>
  forceinline
  LqInt<VX,VY>::LqInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c)
    : BaseInt<VX,VY>(home,x,n_s,y,c) {}

  template<class VX, class VY>
  ExecStatus
  LqInt<VX,VY>::post(Home home, ViewArray<VX>& x, VY y, int c) {
    // Eliminate decided views
    int n_x = x.size();
    for (int i=n_x; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i] = x[--n_x]; break;
      case RT_TRUE:
        x[i] = x[--n_x]; c--; break;
      case RT_MAYBE:
        break;
      default:
        GECODE_NEVER;
      }
    x.size(n_x);
    if (c < 0)
      return ES_FAILED;
    if (c >= n_x)
      return ES_OK;
    // All views must be different
    if (c == 0)
      return post_false(home,x,y);
    (void) new (home) LqInt<VX,VY>(home,x,n_x-c+1,y,c);
    return ES_OK;
  }

  template<class VX, class VY>
  forceinline
  LqInt<VX,VY>::LqInt(Space& home, bool share, LqInt<VX,VY>& p)
    : BaseInt<VX,VY>(home,share,p) {}

  template<class VX, class VY>
  Actor*
  LqInt<VX,VY>::copy(Space& home, bool share) {
    return new (home) LqInt<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  ExecStatus
  LqInt<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    // Eliminate decided views from subscribed views
    int n_x = x.size();
    for (int i=n_s; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i].cancel(home,*this,PC_INT_DOM);
        x[i]=x[--n_s]; x[n_s]=x[--n_x];
        break;
      case RT_TRUE:
        x[i].cancel(home,*this,PC_INT_DOM);
        x[i]=x[--n_s]; x[n_s]=x[--n_x]; c--;
        break;
      case RT_MAYBE:
        break;
      default:
        GECODE_NEVER;
      }
    x.size(n_x);
    if (c < 0)
      return ES_FAILED;
    if (c >= n_x)
      return home.ES_SUBSUMED(*this);
    // Eliminate decided views from unsubscribed views
    for (int i=n_x; i-- > n_s; )
      switch (holds(x[i],y)) {
      case RT_FALSE: x[i]=x[--n_x]; break;
      case RT_TRUE:  x[i]=x[--n_x]; c--; break;
      case RT_MAYBE: break;
      default:       GECODE_NEVER;
      }
    x.size(n_x);
    if (c < 0)
      return ES_FAILED;
    if (c >= n_x)
      return home.ES_SUBSUMED(*this);
    if (c == 0) {
      // All views must be different
      GECODE_ES_CHECK(post_false(home,x,y));
      return home.ES_SUBSUMED(*this);
    }
    // Now, there must be new subscriptions from x[n_s] up to x[n_x-c+1]
    int m = n_x-c;
    while (n_s <= m)
      x[n_s++].subscribe(home,*this,PC_INT_DOM,false);
    return ES_FIX;
  }

  /*
   * Not-equal propagator (integer rhs)
   *
   */
  template<class VX, class VY>
  forceinline
  NqInt<VX,VY>::NqInt(Home home, ViewArray<VX>& x0, VY y0, int c0)
    : BinaryPropagator<VX,PC_INT_DOM>(home,
                                      x0[x0.size()-2],
                                      x0[x0.size()-1]), x(x0), y(y0), c(c0) {
    assert(x.size() >= 2);
    x.size(x.size()-2);
    if (notice(y))
      home.notice(*this,AP_DISPOSE);
    subscribe(home,*this,y);
  }

  template<class VX, class VY>
  forceinline size_t
  NqInt<VX,VY>::dispose(Space& home) {
    if (notice(y))
      home.ignore(*this,AP_DISPOSE);
    cancel(home,*this,y);
    (void) BinaryPropagator<VX,PC_INT_DOM>::dispose(home);
    return sizeof(*this);
  }

  template<class VX, class VY>
  forceinline
  NqInt<VX,VY>::NqInt(Space& home, bool share, NqInt<VX,VY>& p)
    : BinaryPropagator<VX,PC_INT_DOM>(home,share,p), c(p.c) {
    x.update(home,share,p.x);
    y.update(home,share,p.y);
  }

  template<class VX, class VY>
  forceinline ExecStatus
  NqInt<VX,VY>::post(Home home, ViewArray<VX>& x, VY y, int c) {
    int n = x.size();
    for (int i=n; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE: x[i]=x[--n]; break;
      case RT_TRUE:  x[i]=x[--n]; c--; break;
      case RT_MAYBE: break;
      default:       GECODE_NEVER;
      }
    x.size(n);
    if ((n < c) || (c < 0))
      return ES_OK;
    if (n == 0)
      return (c == 0) ? ES_FAILED : ES_OK;
    if (n == 1) {
      if (c == 1)
        return post_false(home,x[0],y);
      else
        return post_true(home,x[0],y);
    }
    (void) new (home) NqInt(home,x,y,c);
    return ES_OK;
  }

  template<class VX, class VY>
  Actor*
  NqInt<VX,VY>::copy(Space& home, bool share) {
    return new (home) NqInt<VX,VY>(home,share,*this);
  }

  template<class VX, class VY>
  PropCost
  NqInt<VX,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  template<class VX, class VY>
  forceinline bool
  NqInt<VX,VY>::resubscribe(Space& home, VX& z) {
    switch (holds(z,y)) {
    case RT_FALSE: break;
    case RT_TRUE:  c--; break;
    case RT_MAYBE: return true;
    default:       GECODE_NEVER;
    }
    int n = x.size();
    for (int i=n; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i]=x[--n];
        break;
      case RT_TRUE:
        x[i]=x[--n]; c--;
        break;
      case RT_MAYBE:
        // New undecided view found
        z.cancel(home,*this,PC_INT_DOM);
        z=x[i]; x[i]=x[--n];
        x.size(n);
        z.subscribe(home,*this,PC_INT_DOM,false);
        return true;
      default:
        GECODE_NEVER;
      }
    // All views have been decided
    x.size(0);
    return false;
  }

  template<class VX, class VY>
  ExecStatus
  NqInt<VX,VY>::propagate(Space& home, const ModEventDelta&) {
    bool s0 = resubscribe(home,x0);
    bool s1 = resubscribe(home,x1);
    int n = x.size() + s0 + s1;
    if ((n < c) || (c < 0))
      return home.ES_SUBSUMED(*this);
    if (n == 0)
      return (c == 0) ? ES_FAILED : home.ES_SUBSUMED(*this);
    if (n == 1) {
      if (s0) {
        if (c == 1) {
          GECODE_ES_CHECK(post_false(home,x0,y));
          return home.ES_SUBSUMED(*this);
        } else {
          GECODE_ES_CHECK(post_true(home,x0,y));
          return home.ES_SUBSUMED(*this);
        }
      } else {
        if (c == 1) {
          GECODE_ES_CHECK(post_false(home,x1,y));
          return home.ES_SUBSUMED(*this);
        } else {
          GECODE_ES_CHECK(post_true(home,x1,y));
          return home.ES_SUBSUMED(*this);
        }
      }
    }
    return ES_FIX;
  }

}}}

// STATISTICS: int-prop

