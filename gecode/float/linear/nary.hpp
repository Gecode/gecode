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

namespace Gecode { namespace Float { namespace Linear {

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
   * Computing bounds
   *
   */
  template<class View>
  void
  bounds_p(Rounding& r, ModEventDelta med, ViewArray<View>& x, FloatVal& c, FloatNum& sl, FloatNum& su) {
    int n = x.size();
    if (FloatView::me(med) == ME_FLOAT_VAL) {
      for (int i = n; i--; ) {
        if (x[i].assigned()) {
          c -= x[i].val(); x[i] = x[--n];
        } else {
          sl = r.sub_up(sl,x[i].min()); su = r.sub_down(su,x[i].max());
        }
      }
      x.size(n);
    } else {
      for (int i = n; i--; ) {
        sl = r.sub_up(sl,x[i].min()); su = r.sub_down(su,x[i].max());
      }
    }
  }

  template<class View>
  void
  bounds_n(Rounding& r, ModEventDelta med, ViewArray<View>& y, FloatVal& c, FloatNum& sl, FloatNum& su) {
    int n = y.size();
    if (FloatView::me(med) == ME_FLOAT_VAL) {
      for (int i = n; i--; ) {
        if (y[i].assigned()) {
          c += y[i].val(); y[i] = y[--n];
        } else {
          sl = r.add_up(sl,y[i].max()); su = r.add_down(su,y[i].min());
        }
      }
      y.size(n);
    } else {
      for (int i = n; i--; ) {
        sl = r.add_up(sl,y[i].max()); su = r.add_down(su,y[i].min());
      }
    }
  }

  forceinline bool 
  infty(const FloatNum& n) {
    return ((n == std::numeric_limits<FloatNum>::infinity()) || 
            (n == -std::numeric_limits<FloatNum>::infinity()));
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
    (void) new (home) Eq<P,N>(home,x,y,c);
    return ES_OK;
  }


  template<class P, class N>
  forceinline
  Eq<P,N>::Eq(Space& home, bool share, Eq<P,N>& p)
    : Lin<P,N,PC_FLOAT_BND>(home,share,p) {}

  template<class P, class N>
  Actor*
  Eq<P,N>::copy(Space& home, bool share) {
    return new (home) Eq<P,N>(home,share,*this);
  }

  template<class P, class N>
  ExecStatus
  Eq<P,N>::propagate(Space& home, const ModEventDelta& med) {
    // Eliminate singletons
    FloatNum sl = 0.0;
    FloatNum su = 0.0;

    Rounding r;

    bounds_p<P>(r, med, x, c, sl, su);
    bounds_n<N>(r, med, y, c, sl, su);

    if ((FloatView::me(med) == ME_FLOAT_VAL) && ((x.size() + y.size()) <= 1)) {
      if (x.size() == 1) {
        GECODE_ME_CHECK(x[0].eq(home,c));
        return home.ES_SUBSUMED(*this);
      }
      if (y.size() == 1) {
        GECODE_ME_CHECK(y[0].eq(home,-c));
        return home.ES_SUBSUMED(*this);
      }
      return (c.in(0.0)) ? home.ES_SUBSUMED(*this) : ES_FAILED;
    }


    sl = r.add_up(sl,c.max()); 
    su = r.add_down(su,c.min());

    const int mod_sl = 1;
    const int mod_su = 2;

    int mod = mod_sl | mod_su;

    do {
      if (mod & mod_sl) {
        mod -= mod_sl;
        // Propagate max bound for positive variables
        for (int i = x.size(); i--; ) {
          const FloatNum xi_max = x[i].max();
          ModEvent me = x[i].lq(home,r.add_up(sl,x[i].min()));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(su)) 
              su = r.add_down(su,r.sub_down(xi_max,x[i].max()));
            mod |= mod_su;
          }
        }
        // Propagate min bound for negative variables
        for (int i = y.size(); i--; ) {
          const FloatNum yi_min = y[i].min();
          ModEvent me = y[i].gq(home,r.sub_down(y[i].max(),sl));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(su)) 
              su = r.add_down(su,r.sub_down(y[i].min(),yi_min));
            mod |= mod_su;
          }
        }
      }
      if (mod & mod_su) {
        mod -= mod_su;
        // Propagate min bound for positive variables
        for (int i = x.size(); i--; ) {
          const FloatNum xi_min = x[i].min();
          ModEvent me = x[i].gq(home,r.add_down(su,x[i].max()));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(sl)) 
              sl = r.add_up(sl,r.sub_up(xi_min,x[i].min()));
            mod |= mod_sl;
          }
        }
        // Propagate max bound for negative variables
        for (int i = y.size(); i--; ) {
          const FloatNum yi_max = y[i].max();
          ModEvent me = y[i].lq(home,r.sub_up(y[i].min(),su));
          if (me_failed(me))
            return ES_FAILED;
          if (me_modified(me)) {
            if (!infty(sl)) 
              sl = r.add_up(sl,r.sub_up(y[i].max(),yi_max));
            mod |= mod_sl;
          }
        }
      }
    } while (mod);

    return (sl == su) ? home.ES_SUBSUMED(*this) : ES_NOFIX;
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
    (void) new (home) Lq<P,N>(home,x,y,c);
    return ES_OK;
  }

  template<class P, class N>
  forceinline
  Lq<P,N>::Lq(Space& home, bool share, Lq<P,N>& p)
    : Lin<P,N,PC_FLOAT_BND>(home,share,p) {}

  template<class P, class N>
  Actor*
  Lq<P,N>::copy(Space& home, bool share) {
    return new (home) Lq<P,N>(home,share,*this);
  }

  template<class P, class N>
  ExecStatus
  Lq<P,N>::propagate(Space& home, const ModEventDelta& med) {
    // Eliminate singletons
    FloatNum sl = 0.0;

    Rounding r;

    if (FloatView::me(med) == ME_FLOAT_VAL) {
      for (int i = x.size(); i--; ) {
        if (x[i].assigned()) {
          c  -= x[i].val();  x.move_lst(i);
        } else {
          sl = r.sub_up(sl,x[i].min());
        }
      }
      for (int i = y.size(); i--; ) {
        if (y[i].assigned()) {
          c  += y[i].val();  y.move_lst(i);
        } else {
          sl = r.add_up(sl,y[i].max());
        }
      }
      if ((x.size() + y.size()) <= 1) {
        if (x.size() == 1) {
          GECODE_ME_CHECK(x[0].lq(home,c.max()));
          return home.ES_SUBSUMED(*this);
        }
        if (y.size() == 1) {
          GECODE_ME_CHECK(y[0].gq(home,(-c).min()));
          return home.ES_SUBSUMED(*this);
        }
        return (c.max() >= 0) ? home.ES_SUBSUMED(*this) : ES_FAILED;
      }
    } else {
      for (int i = x.size(); i--; )
        sl = r.sub_up(sl,x[i].min());
      for (int i = y.size(); i--; )
        sl = r.add_up(sl,y[i].max());
    }

    sl = r.add_up(sl,c.max());

    ExecStatus es = ES_FIX;
    bool assigned = true;
    for (int i = x.size(); i--; ) {
      assert(!x[i].assigned());
      FloatNum slx = r.add_up(sl,x[i].min());
      ModEvent me = x[i].lq(home,slx);
      if (me == ME_FLOAT_FAILED)
        return ES_FAILED;
      if (me != ME_FLOAT_VAL)
        assigned = false;
      if (me_modified(me))
        es = ES_NOFIX;
    }

    for (int i = y.size(); i--; ) {
      assert(!y[i].assigned());
      FloatNum sly = r.sub_up(y[i].max(),sl);
      ModEvent me = y[i].gq(home,sly);
      if (me == ME_FLOAT_FAILED)
        return ES_FAILED;
      if (me != ME_FLOAT_VAL)
        assigned = false;
      if (me_modified(me))
        es = ES_NOFIX;
    }

    return assigned ? home.ES_SUBSUMED(*this) : es;
  }

}}}

// STATISTICS: float-prop

