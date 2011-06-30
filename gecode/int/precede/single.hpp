/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <Chris.Mears@monash.edu>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christopher Mear, 2011
 *     Christian Schulte, 2011
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

namespace Gecode { namespace Int { namespace Precede {

  template<class View>
  forceinline bool
  fixedTo(View x, int v) {
    return x.assigned() && (x.val() == v);
  }
    
  template<class View>
  forceinline
  Single<View>::Index::Index(Space& home, Propagator& p,
                             Council<Index>& c, int i0)
    : Advisor(home,p,c), i(i0) {}

  template<class View>
  forceinline
  Single<View>::Index::Index(Space& home, bool share, Index& a)
    : Advisor(home,share,a), i(a.i) {}

  
  template<class View>
  forceinline ExecStatus 
  Single<View>::updateAlpha(Space& home) {
    int n = x.size();
    while ((alpha < n) && !x[alpha].in(s))
      GECODE_ME_CHECK(x[alpha++].nq(home, t));
    if (alpha < n)
      GECODE_ME_CHECK(x[alpha].nq(home, t));
    return ES_OK;
  }

  template<class View>
  forceinline ExecStatus
  Single<View>::updateBeta(Space& home) {
    int n = x.size();
    do {
      beta++;
    } while ((beta < n) && !x[beta].in(s));
    if (beta > gamma)
      GECODE_ME_CHECK(x[alpha].eq(home, s));
    return ES_OK;
  }
  
  template<class View>
  forceinline
  Single<View>::Single(Home home, ViewArray<View>& x0, 
                       int s0, int t0, int b, int g)
    : NaryPropagator<View, PC_INT_NONE>(home,x0), 
      c(home), s(s0), t(t0), alpha(0), beta(b), gamma(g) {
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        x[i].subscribe(home,*new (home) Index(home,*this,c,i));
    View::schedule(home, *this, ME_INT_BND);
  }

  template<class View>
  inline ExecStatus
  Single<View>::post(Home home, ViewArray<View>& x, int s, int t) {
    {
      int alpha = 0;
      while ((alpha < x.size()) && !x[alpha].in(s))
        GECODE_ME_CHECK(x[alpha++].nq(home,t));
      x.drop_fst(alpha);
      if (x.size() == 0)
        return ES_OK;
    }
    int beta = 0, gamma = 0;
    assert(x.size() > 0);
    GECODE_ME_CHECK(x[0].nq(home,t));
    do {
      gamma++;
    } while ((gamma < x.size()) && !fixedTo(x[gamma],t));
    do {
      beta++;
    } while ((beta < x.size()) && !x[beta].in(s));
    if (beta > gamma) {
      GECODE_ME_CHECK(x[0].eq(home, s));
      return ES_OK;
    }
    (void) new (home) Single<View>(home, x, s, t, beta, gamma);
    return ES_OK;
  }
    


  template<class View>
  forceinline
  Single<View>::Single(Space& home, bool share, Single& p)
    : NaryPropagator<View,PC_INT_NONE>(home, share, p),
      s(p.s), t(p.t),
      alpha(p.alpha), beta(p.beta), gamma(p.gamma) {
    c.update(home, share, p.c);
  }
  template<class View>
  Propagator*
  Single<View>::copy(Space& home, bool share) {
    return new (home) Single<View>(home, share, *this);
  }


  template<class View>
  inline size_t
  Single<View>::dispose(Space& home) {
    for (Advisors<Index> as(c); as(); ++as)
      x[as.advisor().i].cancel(home,as.advisor());
    c.dispose(home);
    (void) NaryPropagator<View,PC_INT_NONE>::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  PropCost 
  Single<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO, x.size());
  }

  template<class View>
  ExecStatus
  Single<View>::advise(Space&, Advisor& a0, const Delta&) {
    Index& a(static_cast<Index&>(a0));
    // Check for gamma
    if ((beta <= gamma) && (a.i < gamma) && fixedTo(x[a.i],t))
      gamma = a.i;
    return ES_NOFIX;
  }
  
  template<class View>
  ExecStatus
  Single<View>::propagate(Space& home, const ModEventDelta&) {
    int n = x.size();
    if (beta > gamma) {
      GECODE_ME_CHECK(x[alpha].eq(home, s));
      return home.ES_SUBSUMED(*this);
    }
    if ((alpha < n) && !x[alpha].in(s)) {
      alpha++;
      while (alpha < beta)
        GECODE_ME_CHECK(x[alpha++].nq(home, t));
      GECODE_ES_CHECK(updateAlpha(home));
      beta = alpha;
      if (alpha < n)
        GECODE_ES_CHECK(updateBeta(home));
    } else if ((beta < n) && !x[beta].in(s)) {
      GECODE_ES_CHECK(updateBeta(home));
    }
    
    if (x.assigned())
      return home.ES_SUBSUMED(*this);
    else
      return ES_FIX;
  }
  
}}}

// STATISTICS: int-prop
