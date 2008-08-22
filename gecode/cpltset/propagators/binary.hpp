/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

namespace Gecode { namespace CpltSet {

  template <class View0, class View1>
  forceinline
  BinaryCpltSetPropagator<View0,View1>
  ::BinaryCpltSetPropagator(Space& home, View0& x0, View1& y0, bdd& d0)
    : Propagator(home), x(x0), y(y0), d(d0) {
    home.notice(*this,AP_DISPOSE);
    x.subscribe(home, *this, PC_CPLTSET_DOM);
    y.subscribe(home, *this, PC_CPLTSET_DOM);
  }

  template <class View0, class View1>
  forceinline
  BinaryCpltSetPropagator<View0,View1>
  ::BinaryCpltSetPropagator(Space& home, bool share,
                            BinaryCpltSetPropagator& p)
    : Propagator(home,share,p) {
    d = p.d;
    x.update(home, share, p.x);
    y.update(home, share, p.y);
  }
  
  template <class View0, class View1>
  forceinline PropCost
  BinaryCpltSetPropagator<View0,View1>::cost(const Space&, const ModEventDelta&) const {
    if (manager.ctrue(x.dom()) || manager.ctrue(d)) {
      return PC_LINEAR_LO;
    } else {
      return PC_QUADRATIC_HI;
    }
  }

  template <class View0, class View1>
  Support::Symbol
  BinaryCpltSetPropagator<View0,View1>::ati(void) {
    return 
      Reflection::mangle<View0,View1>("Gecode::CpltSet::BinaryCpltSetPropagator");
  }

  template <class View0, class View1>
  Reflection::ActorSpec
  BinaryCpltSetPropagator<View0,View1>::spec(const Space&,
                                             Reflection::VarMap&) const {
    throw Reflection::ReflectionException("Not implemented");
  } 
  
  template <class View0, class View1>
  size_t
  BinaryCpltSetPropagator<View0,View1>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    if (!home.failed()) {
      x.cancel(home, *this, PC_CPLTSET_DOM);
      y.cancel(home, *this, PC_CPLTSET_DOM);
    }
    manager.dispose(d);
    Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class View0, class View1>
  forceinline ExecStatus
  BinaryCpltSetPropagator<View0,View1>::post(Space& home,
                                             View0& x0, View1& y0, bdd& d0) {
    (void) new (home) BinaryCpltSetPropagator(home, x0, y0, d0);
    return ES_OK;
  }

  template <class View0, class View1>
  forceinline Actor*
  BinaryCpltSetPropagator<View0,View1>::copy(Space& home, bool share) {
    return new (home) BinaryCpltSetPropagator(home, share, *this);
  }

  template <class View0, class View1>
  forceinline ExecStatus 
  BinaryCpltSetPropagator<View0,View1>::propagate(Space& home, const ModEventDelta&) {
    bool assigned = true;
    {
      bdd dom = y.dom();
      int s = y.offset();
      int w = s + y.tableWidth() - 1;
      manager.existquant(dom, d, s, w);
      ModEvent me = x.intersect(home, dom);
      GECODE_ME_CHECK(me);
    }
    {
      bdd dom = x.dom();
      int s = x.offset();
      int w = s + x.tableWidth() - 1;
      manager.existquant(dom, d, s, w);
      ModEvent me = y.intersect(home, dom);
      GECODE_ME_CHECK(me);
    }

    assigned = true;
    assigned &= (x.assigned() && y.assigned());

    if (assigned) {
      return ES_SUBSUMED(*this, home);
    }
    return ES_FIX;
  }



  template <class View0, class View1>
  forceinline
  BinRelDisj<View0,View1>::BinRelDisj(Space& home, View0& x0, View1& y0, 
                                      bdd& d0)
    : BinaryCpltSetPropagator<View0,View1>(home, x0, y0, d0) { }

  template <class View0, class View1>
  forceinline
  BinRelDisj<View0,View1>::BinRelDisj(Space& home, bool share, BinRelDisj& p)
    : BinaryCpltSetPropagator<View0,View1>(home,share,p) { }
  

  template <class View0, class View1>
  size_t
  BinRelDisj<View0,View1>::dispose(Space& home) {
    BinaryCpltSetPropagator<View0,View1>::dispose(home);
    return sizeof(*this);
  }

  template <class View0, class View1>
  forceinline ExecStatus
  BinRelDisj<View0,View1>::post(Space& home, View0& x0, View1& y0, 
                                bdd& d0) {
    (void) new (home) BinRelDisj(home, x0, y0, d0);
    return ES_OK;
  }

  template <class View0, class View1>
  forceinline Actor*
  BinRelDisj<View0,View1>::copy(Space& home, bool share) {
    return new (home) BinRelDisj(home, share, *this);
  }

  template <class View0, class View1>
  forceinline ExecStatus 
  BinRelDisj<View0,View1>::propagate(Space& home, const ModEventDelta&) {
    bool assigned = true;

    if (x.assigned()) {
      Set::GlbRanges<View0> glbx(x);
      if (y.assigned()) {
        Set::GlbRanges<View1> glby(y);
        Iter::Ranges::Inter<Set::GlbRanges<View0>, Set::GlbRanges<View1> > 
          inter(glbx, glby);
        if (inter())
          return ES_FAILED;
        return ES_SUBSUMED(*this, home);
      }
      ModEvent me = y.excludeI(home, glbx);
      GECODE_ME_CHECK(me);
      return ES_SUBSUMED(*this, home);
    }

    if (y.assigned()) {
      Set::GlbRanges<View1> glby(y);
      ModEvent me = x.excludeI(home, glby);
      GECODE_ME_CHECK(me);
    }

    Set::LubRanges<View0> lubx(x);
    Set::LubRanges<View1> luby(y);
    Iter::Ranges::Inter<Set::LubRanges<View0>, Set::LubRanges<View1> > 
      inter(lubx, luby);
    Iter::Ranges::ToValues<
      Iter::Ranges::Inter<Set::LubRanges<View0>, Set::LubRanges<View1> > 
      > ival(inter);

    Iter::Ranges::ValCache<
      Iter::Ranges::ToValues<
         Iter::Ranges::Inter<Set::LubRanges<View0>, Set::LubRanges<View1> > 
      >
      > cache(ival);

    if (!cache()) {
      return ES_SUBSUMED(*this, home);
    } else {
      d = bdd_true();
      cache.last();
      for (; cache(); --cache) {
        int v = cache.min();
        int xmin = x.initialLubMin();
        int ymin = y.initialLubMin();
        d &= (!(x.element(v - xmin) & y.element(v - ymin)));
      }
    }

    {
      bdd dom = y.dom();
      int s = y.offset();
      int w = s + y.tableWidth() - 1;
      manager.existquant(dom, d, s, w);
      ModEvent me = x.intersect(home, dom);
      GECODE_ME_CHECK(me);
    }
    {
      bdd dom = x.dom();
      int s = x.offset();
      int w = s + x.tableWidth() - 1;
      manager.existquant(dom, d, s, w);
      ModEvent me = y.intersect(home, dom);
      GECODE_ME_CHECK(me);
    }

    assigned = true;
    assigned &= (x.assigned() && y.assigned());

    if (assigned) {
      return ES_SUBSUMED(*this, home);
    }

    return ES_FIX;
  }


}}

// STATISTICS: cpltset-prop
