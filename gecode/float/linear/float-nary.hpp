/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2003
 *     Vincent Barichard, 2012
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

#include <gecode/float/linear/float-noview.hpp>

namespace Gecode { namespace Float { namespace Linear {

  /**
   * \brief Test if only unit-coefficient arrays used
   *
   */
  template<class P, class N>
  forceinline bool
  isunit(ViewArray<P>&, ViewArray<N>&) { return false; }
  template<>
  forceinline bool
  isunit(ViewArray<FloatView>&, ViewArray<FloatView>&) { return true; }
  template<>
  forceinline bool
  isunit(ViewArray<FloatView>&, ViewArray<NoView>&) { return true; }
  template<>
  forceinline bool
  isunit(ViewArray<NoView>&, ViewArray<FloatView>&) { return true; }

  /*
   * Linear propagators
   *
   */
  template<class P, class N, PropCond pc>
  forceinline
  Lin<P,N,pc>::Lin(Home home, ViewArray<P>& x0, ViewArray<N>& y0, FloatVal c0)
    : Propagator(home), x(x0), y(y0), c(c0) {
    x.subscribe(home,*this,pc);
    y.subscribe(home,*this,pc);
  }

  template<class P, class N, PropCond pc>
  forceinline
  Lin<P,N,pc>::Lin(Space& home, bool share, Lin<P,N,pc>& p)
    : Propagator(home,share,p), c(p.c) {
    x.update(home,share,p.x);
    y.update(home,share,p.y);
  }

  template<class P, class N, PropCond pc>
  PropCost
  Lin<P,N,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO, x.size()+y.size());
  }

  template<class P, class N, PropCond pc>
  forceinline size_t
  Lin<P,N,pc>::dispose(Space& home) {
    x.cancel(home,*this,pc);
    y.cancel(home,*this,pc);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Reified linear propagators
   *
   */
  template<class P, class N, PropCond pc, class Ctrl>
  forceinline
  ReLin<P,N,pc,Ctrl>::ReLin
  (Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b0)
    : Lin<P,N,pc>(home,x,y,c), b(b0) {
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<class P, class N, PropCond pc, class Ctrl>
  forceinline
  ReLin<P,N,pc,Ctrl>::ReLin
  (Space& home, bool share, ReLin<P,N,pc,Ctrl>& p)
    : Lin<P,N,pc>(home,share,p) {
    b.update(home,share,p.b);
  }

  template<class P, class N, PropCond pc, class Ctrl>
  forceinline size_t
  ReLin<P,N,pc,Ctrl>::dispose(Space& home) {
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Lin<P,N,pc>::dispose(home);
    return sizeof(*this);
  }

  /*
   * Computing bounds
   *
   */
  template<class View>
  void
  bounds_p(ModEventDelta med, ViewArray<View>& x, FloatVal& c, FloatNum& sl, FloatNum& su) {
    int n = x.size();
    if (FloatView::me(med) == ME_FLOAT_VAL) {
      for (int i = n; i--; ) {
        FloatNum m = x[i].min();
        if (x[i].assigned()) {
          c -= m; x[i] = x[--n];
        } else {
          sl = Round.sub_up(sl,m); su = Round.sub_down(su,x[i].max());
        }
      }
      x.size(n);
    } else {
      for (int i = n; i--; ) {
        sl = Round.sub_up(sl,x[i].min()); su = Round.sub_down(su,x[i].max());
      }
    }
  }

  template<class View>
  void
  bounds_n(ModEventDelta med, ViewArray<View>& y, FloatVal& c, FloatNum& sl, FloatNum& su) {
    int n = y.size();
    if (FloatView::me(med) == ME_FLOAT_VAL) {
      for (int i = n; i--; ) {
        FloatNum m = y[i].max();
        if (y[i].assigned()) {
          c += m; y[i] = y[--n];
        } else {
          sl = Round.add_up(sl,m); 
          su = Round.add_down(su,y[i].min());
        }
      }
      y.size(n);
    } else {
      for (int i = n; i--; ) {
        sl = Round.add_up(sl,y[i].max()); 
        su = Round.add_down(su,y[i].min());
      }
    }
  }

  forceinline bool infty(const FloatNum& n) {
    return (  (n == std::numeric_limits<FloatNum>::infinity())
           || (n == -std::numeric_limits<FloatNum>::infinity()) );
  }

  template<class P, class N>
  ExecStatus
  prop_bnd(Space& home, ModEventDelta med, Propagator& p,
           ViewArray<P>& x, ViewArray<N>& y, FloatVal& c) {
    // Eliminate singletons
    FloatNum sl = 0.0;
    FloatNum su = 0.0;

    bounds_p<P>(med, x, c, sl, su);
    bounds_n<N>(med, y, c, sl, su);

    if ((FloatView::me(med) == ME_FLOAT_VAL) && ((x.size() + y.size()) <= 1)) {
      if (x.size() == 1) {
        GECODE_ME_CHECK(x[0].eq(home,c));
        return home.ES_SUBSUMED(p);
      }
      if (y.size() == 1) {
        GECODE_ME_CHECK(y[0].eq(home,-c));
        return home.ES_SUBSUMED(p);
      }
      return (c == 0.0) ? home.ES_SUBSUMED(p) : ES_FAILED;
    }

    sl = Round.add_up(sl,upper(c)); 
    su = Round.add_down(su,lower(c));

    const int mod_sl = 1;
    const int mod_su = 2;

    int mod = mod_sl | mod_su;

    do {
      if (mod & mod_sl) {
        mod -= mod_sl;
        // Propagate upper bound for positive variables
        for (int i = x.size(); i--; ) {
          const FloatNum xi_max = x[i].max();
          ModEvent me = x[i].lq(home,Round.add_up(sl,x[i].min()));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(su)) 
              su = Round.add_down(su,Round.sub_down(xi_max,x[i].max()));
            mod |= mod_su;
          }
        }
        // Propagate lower bound for negative variables
        for (int i = y.size(); i--; ) {
          const FloatNum yi_min = y[i].min();
          ModEvent me = y[i].gq(home,Round.sub_down(y[i].max(),sl));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(su)) 
              su = Round.add_down(su,Round.sub_down(y[i].min(),yi_min));
            mod |= mod_su;
          }
        }
      }
      if (mod & mod_su) {
        mod -= mod_su;
        // Propagate lower bound for positive variables
        for (int i = x.size(); i--; ) {
          const FloatNum xi_min = x[i].min();
          ModEvent me = x[i].gq(home,Round.add_down(su,x[i].max()));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(sl)) 
              sl = Round.add_up(sl,Round.sub_up(xi_min,x[i].min()));
            mod |= mod_sl;
          }
        }
        // Propagate upper bound for negative variables
        for (int i = y.size(); i--; ) {
          const FloatNum yi_max = y[i].max();
          ModEvent me = y[i].lq(home,Round.sub_up(y[i].min(),su));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(sl)) 
              sl = Round.add_up(sl,Round.sub_up(y[i].max(),yi_max));
            mod |= mod_sl;
          }
        }
      }
    } while (mod);

    return (sl == su) ? home.ES_SUBSUMED(p) : ES_FIX;
  }

  /*
   * Bound consistent linear equation
   *
   */

  template<class P, class N>
  forceinline
  Eq<P,N>::Eq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c)
    : Lin<P,N,PC_FLOAT_BND>(home,x,y,c) {}

  template<class P, class N>
  ExecStatus
  Eq<P,N>::post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c) {
    ViewArray<NoView> nva;
    if (y.size() == 0) {
      (void) new (home) Eq<P,NoView>(home,x,nva,c);
    } else if (x.size() == 0) {
      (void) new (home) Eq<N,NoView>(home,y,nva,-c);
    } else {
      (void) new (home) Eq<P,N>(home,x,y,c);
    }
    return ES_OK;
  }


  template<class P, class N>
  forceinline
  Eq<P,N>::Eq(Space& home, bool share, Eq<P,N>& p)
    : Lin<P,N,PC_FLOAT_BND>(home,share,p) {}

  /**
   * \brief Rewriting of equality to binary propagators
   *
   */
  template<class P, class N>
  forceinline Actor*
  eqtobin(Space&, bool, Propagator&, ViewArray<P>&, ViewArray<N>&, FloatVal) {
    return NULL;
  }
  template<class Val>
  forceinline Actor*
  eqtobin(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<NoView>&, FloatVal c) {
    assert(x.size() == 2);
    return new (home) EqBin<FloatView,FloatView>
      (home,share,p,x[0],x[1],c);
  }
  template<class Val>
  forceinline Actor*
  eqtobin(Space& home, bool share, Propagator& p,
          ViewArray<NoView>&, ViewArray<FloatView>& y, FloatVal c) {
    assert(y.size() == 2);
    return new (home) EqBin<FloatView,FloatView>
      (home,share,p,y[0],y[1],-c);
  }
  template<class Val>
  forceinline Actor*
  eqtobin(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<FloatView>& y, FloatVal c) {
    if (x.size() == 2)
      return new (home) EqBin<FloatView,FloatView>
        (home,share,p,x[0],x[1],c);
    if (x.size() == 1)
      return new (home) EqBin<FloatView,MinusView>
        (home,share,p,x[0],MinusView(y[0]),c);
    return new (home) EqBin<FloatView,FloatView>
      (home,share,p,y[0],y[1],-c);
  }

  /**
   * \brief Rewriting of equality to ternary propagators
   *
   */
  template<class P, class N>
  forceinline Actor*
  eqtoter(Space&, bool, Propagator&, ViewArray<P>&, ViewArray<N>&, FloatVal) {
    return NULL;
  }
  template<class Val>
  forceinline Actor*
  eqtoter(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<NoView>&, FloatVal c) {
    assert(x.size() == 3);
    return new (home) EqTer<FloatView,FloatView,FloatView>
      (home,share,p,x[0],x[1],x[2],c);
  }
  template<class Val>
  forceinline Actor*
  eqtoter(Space& home, bool share, Propagator& p,
          ViewArray<NoView>&, ViewArray<FloatView>& y, FloatVal c) {
    assert(y.size() == 3);
    return new (home) EqTer<FloatView,FloatView,FloatView>
      (home,share,p,y[0],y[1],y[2],-c);
  }
  template<class Val>
  forceinline Actor*
  eqtoter(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<FloatView>& y, FloatVal c) {
    if (x.size() == 3)
      return new (home) EqTer<FloatView,FloatView,FloatView>
        (home,share,p,x[0],x[1],x[2],c);
    if (x.size() == 2)
      return new (home) EqTer<FloatView,FloatView,MinusView>
        (home,share,p,x[0],x[1],MinusView(y[0]),c);
    if (x.size() == 1)
      return new (home) EqTer<FloatView,FloatView,MinusView>
        (home,share,p,y[0],y[1],MinusView(x[0]),-c);
    return new (home) EqTer<FloatView,FloatView,FloatView>
      (home,share,p,y[0],y[1],y[2],-c);
  }

  template<class P, class N>
  Actor*
  Eq<P,N>::copy(Space& home, bool share) {
    if (isunit(x,y)) {
      // Check whether rewriting is possible
      if (x.size() + y.size() == 2)
        return eqtobin(home,share,*this,x,y,c);
      if (x.size() + y.size() == 3)
        return eqtoter(home,share,*this,x,y,c);
    }
    return new (home) Eq<P,N>(home,share,*this);
  }

  template<class P, class N>
  ExecStatus
  Eq<P,N>::propagate(Space& home, const ModEventDelta& med) {
    return prop_bnd<P,N>(home,med,*this,x,y,c);
  }

  /*
   * Reified bound consistent linear equation
   *
   */

  template<class P, class N, class Ctrl, ReifyMode rm>
  forceinline
  ReEq<P,N,Ctrl,rm>::ReEq(Home home,
                          ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b)
    : ReLin<P,N,PC_FLOAT_BND,Ctrl>(home,x,y,c,b) {}

  template<class P, class N, class Ctrl, ReifyMode rm>
  ExecStatus
  ReEq<P,N,Ctrl,rm>::post(Home home,
                          ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b) {
    ViewArray<NoView> nva;
    if (y.size() == 0) {
      (void) new (home) ReEq<P,NoView,Ctrl,rm>(home,x,nva,c,b);
    } else if (x.size() == 0) {
      (void) new (home) ReEq<N,NoView,Ctrl,rm>(home,y,nva,-c,b);
    } else {
      (void) new (home) ReEq<P,N,Ctrl,rm>(home,x,y,c,b);
    }
    return ES_OK;
  }


  template<class P, class N, class Ctrl, ReifyMode rm>
  forceinline
  ReEq<P,N,Ctrl,rm>::ReEq(Space& home, bool share, 
                          ReEq<P,N,Ctrl,rm>& p)
    : ReLin<P,N,PC_FLOAT_BND,Ctrl>(home,share,p) {}

  template<class P, class N, class Ctrl, ReifyMode rm>
  Actor*
  ReEq<P,N,Ctrl,rm>::copy(Space& home, bool share) {
    return new (home) ReEq<P,N,Ctrl,rm>(home,share,*this);
  }

  template<class P, class N, class Ctrl, ReifyMode rm>
  ExecStatus
  ReEq<P,N,Ctrl,rm>::propagate(Space& home, const ModEventDelta& med) {
    if (b.zero())
      GECODE_REWRITE(*this,(Nq<P,N>::post(home(*this),x,y,c)));
    if (b.one())
      GECODE_REWRITE(*this,(Eq<P,N>::post(home(*this),x,y,c)));

    FloatNum sl = 0;
    FloatNum su = 0;

    bounds_p<P>(med, x, c, sl, su);
    bounds_n<N>(med, y, c, sl, su);

    if ((c == -sl) && (c == -su)) {
      GECODE_ME_CHECK(b.one_none(home));
      return home.ES_SUBSUMED(*this);
    }
    if ((c < -sl) || (c > -su)) {
      GECODE_ME_CHECK(b.zero_none(home));
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }


  /*
   * Domain consistent linear disequation
   *
   */

  template<class P, class N>
  forceinline
  Nq<P,N>::Nq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c)
    : Lin<P,N,PC_FLOAT_VAL>(home,x,y,c) {}

  template<class P, class N>
  ExecStatus
  Nq<P,N>::post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c) {
    ViewArray<NoView> nva;
    if (y.size() == 0) {
      (void) new (home) Nq<P,NoView>(home,x,nva,c);
    } else if (x.size() == 0) {
      (void) new (home) Nq<N,NoView>(home,y,nva,-c);
    } else {
      (void) new (home) Nq<P,N>(home,x,y,c);
    }
    return ES_OK;
  }


  template<class P, class N>
  forceinline
  Nq<P,N>::Nq(Space& home, bool share, Nq<P,N>& p)
    : Lin<P,N,PC_FLOAT_VAL>(home,share,p) {}

  /**
   * \brief Rewriting of disequality to binary propagators
   *
   */
  template<class P, class N>
  forceinline Actor*
  nqtobin(Space&, bool, Propagator&, ViewArray<P>&, ViewArray<N>&, FloatVal) {
    return NULL;
  }
  forceinline Actor*
  nqtobin(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<NoView>&, FloatVal c) {
    assert(x.size() == 2);
    return new (home) NqBin<FloatView,FloatView>
      (home,share,p,x[0],x[1],c);
  }
  forceinline Actor*
  nqtobin(Space& home, bool share, Propagator& p,
          ViewArray<NoView>&, ViewArray<FloatView>& y, FloatVal c) {
    assert(y.size() == 2);
    return new (home) NqBin<FloatView,FloatView>
      (home,share,p,y[0],y[1],-c);
  }
  forceinline Actor*
  nqtobin(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<FloatView>& y, FloatVal c) {
    if (x.size() == 2)
      return new (home) NqBin<FloatView,FloatView>
        (home,share,p,x[0],x[1],c);
    if (x.size() == 1)
      return new (home) NqBin<FloatView,MinusView>
        (home,share,p,x[0],MinusView(y[0]),c);
    return new (home) NqBin<FloatView,FloatView>
      (home,share,p,y[0],y[1],-c);
  }

  /**
   * \brief Rewriting of disequality to ternary propagators
   *
   */
  template<class P, class N>
  forceinline Actor*
  nqtoter(Space&, bool, Propagator&,ViewArray<P>&, ViewArray<N>&, FloatVal) {
    return NULL;
  }
  forceinline Actor*
  nqtoter(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<NoView>&, FloatVal c) {
    assert(x.size() == 3);
    return new (home) NqTer<FloatView,FloatView,FloatView>
      (home,share,p,x[0],x[1],x[2],c);
  }
  forceinline Actor*
  nqtoter(Space& home, bool share, Propagator& p,
          ViewArray<NoView>&, ViewArray<FloatView>& y, FloatVal c) {
    assert(y.size() == 3);
    return new (home) NqTer<FloatView,FloatView,FloatView>
      (home,share,p,y[0],y[1],y[2],-c);
  }
  forceinline Actor*
  nqtoter(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<FloatView>& y, FloatVal c) {
    if (x.size() == 3)
      return new (home) NqTer<FloatView,FloatView,FloatView>
        (home,share,p,x[0],x[1],x[2],c);
    if (x.size() == 2)
      return new (home) NqTer<FloatView,FloatView,MinusView>
        (home,share,p,x[0],x[1],MinusView(y[0]),c);
    if (x.size() == 1)
      return new (home) NqTer<FloatView,FloatView,MinusView>
        (home,share,p,y[0],y[1],MinusView(x[0]),-c);
    return new (home) NqTer<FloatView,FloatView,FloatView>
      (home,share,p,y[0],y[1],y[2],-c);
  }

  template<class P, class N>
  Actor*
  Nq<P,N>::copy(Space& home, bool share) {
    if (isunit(x,y)) {
      // Check whether rewriting is possible
      if (x.size() + y.size() == 2)
        return nqtobin(home,share,*this,x,y,c);
      if (x.size() + y.size() == 3)
        return nqtoter(home,share,*this,x,y,c);
    }
    return new (home) Nq<P,N>(home,share,*this);
  }

  template<class P, class N>
  ExecStatus
  Nq<P,N>::propagate(Space& home, const ModEventDelta&) {
    for (int i = x.size(); i--; )
      if (x[i].assigned()) {
        c -= x[i].val();  x.move_lst(i);
      }
    for (int i = y.size(); i--; )
      if (y[i].assigned()) {
        c += y[i].val();  y.move_lst(i);
      }
    if (x.size() + y.size() <= 1) {
      if (x.size() == 1 && x[0].assigned()) {
        return (x[0].val() == c) ? ES_FAILED : home.ES_SUBSUMED(*this);
      }
      if (y.size() == 1 && y[0].assigned()) {
        return (y[0].val() == -c) ? ES_FAILED : home.ES_SUBSUMED(*this);
      }
      return (c == 0.0) ?
        ES_FAILED : home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }



  /*
   * Bound consistent linear inequation
   *
   */

  template<class P, class N>
  forceinline
  Lq<P,N>::Lq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c)
    : Lin<P,N,PC_FLOAT_BND>(home,x,y,c) {}

  template<class P, class N>
  ExecStatus
  Lq<P,N>::post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c) {
    ViewArray<NoView> nva;
    if (y.size() == 0) {
      (void) new (home) Lq<P,NoView>(home,x,nva,c);
    } else if (x.size() == 0) {
      (void) new (home) Lq<NoView,N>(home,nva,y,c);
    } else {
      (void) new (home) Lq<P,N>(home,x,y,c);
    }
    return ES_OK;
  }


  template<class P, class N>
  forceinline
  Lq<P,N>::Lq(Space& home, bool share, Lq<P,N>& p)
    : Lin<P,N,PC_FLOAT_BND>(home,share,p) {}

  /**
   * \brief Rewriting of inequality to binary propagators
   *
   */
  template<class P, class N>
  forceinline Actor*
  lqtobin(Space&, bool, Propagator&,ViewArray<P>&, ViewArray<N>&, FloatVal) {
    return NULL;
  }
  template<class Val>
  forceinline Actor*
  lqtobin(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<NoView>&, FloatVal c) {
    assert(x.size() == 2);
    return new (home) LqBin<FloatView,FloatView>
      (home,share,p,x[0],x[1],c);
  }
  template<class Val>
  forceinline Actor*
  lqtobin(Space& home, bool share, Propagator& p,
          ViewArray<NoView>&, ViewArray<FloatView>& y, FloatVal c) {
    assert(y.size() == 2);
    return new (home) LqBin<MinusView,MinusView>
      (home,share,p,MinusView(y[0]),MinusView(y[1]),c);
  }
  template<class Val>
  forceinline Actor*
  lqtobin(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<FloatView>& y, FloatVal c) {
    if (x.size() == 2)
      return new (home) LqBin<FloatView,FloatView>
        (home,share,p,x[0],x[1],c);
    if (x.size() == 1)
      return new (home) LqBin<FloatView,MinusView>
        (home,share,p,x[0],MinusView(y[0]),c);
    return new (home) LqBin<MinusView,MinusView>
      (home,share,p,MinusView(y[0]),MinusView(y[1]),c);
  }

  /**
   * \brief Rewriting of inequality to ternary propagators
   *
   */
  template<class P, class N>
  forceinline Actor*
  lqtoter(Space&, bool, Propagator&, ViewArray<P>&, ViewArray<N>&, FloatVal) {
    return NULL;
  }
  template<class Val>
  forceinline Actor*
  lqtoter(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<NoView>&, FloatVal c) {
    assert(x.size() == 3);
    return new (home) LqTer<FloatView,FloatView,FloatView>
      (home,share,p,x[0],x[1],x[2],c);
  }
  template<class Val>
  forceinline Actor*
  lqtoter(Space& home, bool share, Propagator& p,
          ViewArray<NoView>&, ViewArray<FloatView>& y, FloatVal c) {
    assert(y.size() == 3);
    return new (home) LqTer<MinusView,MinusView,MinusView>
      (home,share,p,MinusView(y[0]),MinusView(y[1]),MinusView(y[2]),c);
  }
  template<class Val>
  forceinline Actor*
  lqtoter(Space& home, bool share, Propagator& p,
          ViewArray<FloatView>& x, ViewArray<FloatView>& y, FloatVal c) {
    if (x.size() == 3)
      return new (home) LqTer<FloatView,FloatView,FloatView>
        (home,share,p,x[0],x[1],x[2],c);
    if (x.size() == 2)
      return new (home) LqTer<FloatView,FloatView,MinusView>
        (home,share,p,x[0],x[1],MinusView(y[0]),c);
    if (x.size() == 1)
      return new (home) LqTer<FloatView,MinusView,MinusView>
        (home,share,p,x[0],MinusView(y[0]),MinusView(y[1]),c);
    return new (home) LqTer<MinusView,MinusView,MinusView>
      (home,share,p,MinusView(y[0]),MinusView(y[1]),MinusView(y[2]),c);
  }

  template<class P, class N>
  Actor*
  Lq<P,N>::copy(Space& home, bool share) {
    if (isunit(x,y)) {
      // Check whether rewriting is possible
      if (x.size() + y.size() == 2)
        return lqtobin(home,share,*this,x,y,c);
      if (x.size() + y.size() == 3)
        return lqtoter(home,share,*this,x,y,c);
    }
    return new (home) Lq<P,N>(home,share,*this);
  }

  template<class P, class N>
  ExecStatus
  Lq<P,N>::propagate(Space& home, const ModEventDelta& med) {
    // Eliminate singletons
    FloatNum sl = 0;

    if (FloatView::me(med) == ME_FLOAT_VAL) {
      for (int i = x.size(); i--; ) {
        FloatNum m = x[i].min();
        if (x[i].assigned()) {
          c  -= m;  x.move_lst(i);
        } else {
          sl = Round.sub_up(sl,m);
        }
      }
      for (int i = y.size(); i--; ) {
        FloatNum m = y[i].max();
        if (y[i].assigned()) {
          c  += m;  y.move_lst(i);
        } else {
          sl = Round.add_up(sl,m);
        }
      }
      if ((x.size() + y.size()) <= 1) {
        if (x.size() == 1) {
          GECODE_ME_CHECK(x[0].lq(home,upper(c)));
          return home.ES_SUBSUMED(*this);
        }
        if (y.size() == 1) {
          GECODE_ME_CHECK(y[0].gq(home,lower(-c)));
          return home.ES_SUBSUMED(*this);
        }
        return (c >= 0.0) ? home.ES_SUBSUMED(*this) : ES_FAILED;
      }
    } else {
      for (int i = x.size(); i--; )
        sl = Round.sub_up(sl,x[i].min());
      for (int i = y.size(); i--; )
        sl = Round.add_up(sl,y[i].max());
    }

    sl = Round.add_up(sl,upper(c));

    ExecStatus es = ES_FIX;
    bool assigned = true;
    for (int i = x.size(); i--; ) {
      assert(!x[i].assigned());
      FloatNum slx = Round.add_up(sl,x[i].min());
      ModEvent me = x[i].lq(home,slx);
      if (me == ME_FLOAT_FAILED)
        return ES_FAILED;
      if (me != ME_FLOAT_VAL)
        assigned = false;
      if (me_modified(me) && (slx != x[i].max()))
        es = ES_NOFIX;
    }

    for (int i = y.size(); i--; ) {
      assert(!y[i].assigned());
      FloatNum sly = Round.sub_up(y[i].max(),sl);
      ModEvent me = y[i].gq(home,sly);
      if (me == ME_FLOAT_FAILED)
        return ES_FAILED;
      if (me != ME_FLOAT_VAL)
        assigned = false;
      if (me_modified(me) && (sly != y[i].min()))
        es = ES_NOFIX;
    }
    return assigned ? home.ES_SUBSUMED(*this) : es;
  }

  /*
   * Reified bound consistent linear inequation
   *
   */

  template<class P, class N, class Ctrl, ReifyMode rm>
  forceinline
  ReLq<P,N,Ctrl,rm>::ReLq(Home home, 
                          ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b)
    : ReLin<P,N,PC_FLOAT_BND,Ctrl>(home,x,y,c,b) {}

  template<class P, class N, class Ctrl, ReifyMode rm>
  ExecStatus
  ReLq<P,N,Ctrl,rm>::post(Home home, 
                          ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b) {
    ViewArray<NoView> nva;
    if (y.size() == 0) {
      (void) new (home) ReLq<P,NoView,Ctrl,rm>(home,x,nva,c,b);
    } else if (x.size() == 0) {
      (void) new (home) ReLq<NoView,N,Ctrl,rm>(home,nva,y,c,b);
    } else {
      (void) new (home) ReLq<P,N,Ctrl,rm>(home,x,y,c,b);
    }
    return ES_OK;
  }


  template<class P, class N, class Ctrl, ReifyMode rm>
  forceinline
  ReLq<P,N,Ctrl,rm>::ReLq(Space& home, bool share, ReLq<P,N,Ctrl,rm>& p)
    : ReLin<P,N,PC_FLOAT_BND,Ctrl>(home,share,p) {}

  template<class P, class N, class Ctrl, ReifyMode rm>
  Actor*
  ReLq<P,N,Ctrl,rm>::copy(Space& home, bool share) {
    return new (home) ReLq<P,N,Ctrl,rm>(home,share,*this);
  }

  template<class P, class N, class Ctrl, ReifyMode rm>
  ExecStatus
  ReLq<P,N,Ctrl,rm>::propagate(Space& home, const ModEventDelta& med) {
    if (b.zero())
    {
      GECODE_ES_CHECK((Nq<P,N>::post(home(*this),x,y,c)));
      GECODE_REWRITE(*this,(Lq<P,N>::post(home(*this),x,y,c)));
    }
    if (b.one())
      GECODE_REWRITE(*this,(Lq<P,N>::post(home(*this),x,y,c)));

    // Eliminate singletons
    FloatNum sl = 0;
    FloatNum su = 0;

    bounds_p<P>(med,x,c,sl,su);
    bounds_n<N>(med,y,c,sl,su);

    if (c < -sl) {
      GECODE_ME_CHECK(b.zero_none(home));
      return home.ES_SUBSUMED(*this);
    }
    if (c >= -su) {
      GECODE_ME_CHECK(b.one_none(home));
      return home.ES_SUBSUMED(*this);
    }

    return ES_FIX;
  }

}}}

// STATISTICS: float-prop

