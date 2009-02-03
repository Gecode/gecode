/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

namespace Gecode { namespace Set { namespace Element {

  template <class SView, class RView>
  forceinline
  ElementIntersection<SView,RView>::
  ElementIntersection(Space& home, SView y0,
                     IdxViewArray& iv0,
                     RView y1,
                     const IntSet& theUniverse)
    : Propagator(home), universe(theUniverse), x0(y0), iv(iv0), x1(y1) {
    home.notice(*this,AP_DISPOSE);
    x0.subscribe(home,*this, PC_SET_ANY);
    x1.subscribe(home,*this, PC_SET_ANY);
    iv.subscribe(home,*this, PC_SET_ANY);
  }

  template <class SView, class RView>
  forceinline
  ElementIntersection<SView,RView>::
  ElementIntersection(Space& home, bool share,
                     ElementIntersection<SView,RView>& p)
    : Propagator(home,share,p) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
    iv.update(home,share,p.iv);
    universe.update(home,share,p.universe);
  }

  template <class SView, class RView>
  PropCost
  ElementIntersection<SView,RView>::cost(const Space&,
                                         const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI, iv.size()+2);
  }

  template <class SView, class RView>
  size_t
  ElementIntersection<SView,RView>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    if (!home.failed()) {
      x0.cancel(home,*this, PC_SET_ANY);
      x1.cancel(home,*this, PC_SET_ANY);
      iv.cancel(home,*this,PC_SET_ANY);
    }
    universe.~IntSet();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class SView, class RView>
  ExecStatus
  ElementIntersection<SView,RView>::
  post(Space& home, SView x0, IdxViewArray& xs,
       RView x1, const IntSet& universe) {
    int n = xs.size();

    // s2 \subseteq {1,...,n}
    Iter::Ranges::Singleton s(0, n-1);
    GECODE_ME_CHECK(x1.intersectI(home,s));
    (void) new (home)
      ElementIntersection<SView,RView>(home,x0,xs,x1,universe);
    return ES_OK;
  }

  template <class SView, class RView>
  Actor*
  ElementIntersection<SView,RView>::copy(Space& home, bool share) {
    return new (home) ElementIntersection<SView,RView>(home,share,*this);
  }

  template <class SView, class RView>
  ExecStatus
  ElementIntersection<SView,RView>::propagate(Space& home, const ModEventDelta&) {
    Region r(home);
    int n = iv.size();

    bool loopVar;
    do {
      loopVar = false;

      // Cache the upper bound iterator, as we have to
      // modify the upper bound while iterating
      LubRanges<RView> x1ub(x1);
      Iter::Ranges::Cache<LubRanges<RView> > x1ubc(x1ub);
      Iter::Ranges::ToValues<Iter::Ranges::Cache<LubRanges<RView> > >
        vx1ub(x1ubc);

      GlbRanges<RView> x1lb(x1);
      Iter::Ranges::Cache<GlbRanges<RView> > x1lbc(x1lb);
      Iter::Ranges::ToValues<Iter::Ranges::Cache<GlbRanges<RView> > >
        vx1(x1lbc);

      // In the first iteration, compute in before[i] the intersection
      // of all the lower bounds of the x_i. At the same time,
      // exclude inconsistent x_i from x1 and remove them from
      // the list, cancel their dependencies.

      LUBndSet sofarBefore(home,universe);
      LUBndSet* before = r.alloc<LUBndSet>(n);

      int j = 0;
      int i = 0;
      while ( vx1ub() ) {

        // Remove vars at indices not in the upper bound
        if (iv[i].idx < vx1ub.val()) {
          iv[i].view.cancel(home,*this, PC_SET_ANY);
          ++i;
          continue;
        }
        assert(iv[i].idx == vx1ub.val());
        iv[j] = iv[i];

        SView candidate = iv[j].view;
        int candidateInd = iv[j].idx;

        // inter = glb(x0) & complement(lub(candidate))
        GlbRanges<SView> x0lb(x0);
        LubRanges<SView> candub(candidate);
        Iter::Ranges::Diff<GlbRanges<SView>,LubRanges<SView> >
          inter(x0lb, candub);

        // exclude inconsistent x_i
        // an x_i is inconsistent if
        //  * its max cardinality is less than minCard of x0
        //  * inter is not empty (there are elements in x_0
        //    that can't be in x_i)
        if (candidate.cardMax() < x0.cardMin() ||
            inter()) {
          ModEvent me = (x1.exclude(home,candidateInd));
          loopVar |= me_modified(me);
          GECODE_ME_CHECK(me);

          iv[j].view.cancel(home,*this, PC_SET_ANY);
          ++i;
          ++vx1ub;
          continue;
        } else {
          // if x_i is consistent, check whether we know
          // that its index is in x1
          if (vx1() && vx1.val()==candidateInd) {
            // x0 <= candidate, candidate >= x0
            GlbRanges<SView> x0lb(x0);
            ModEvent me = candidate.includeI(home,x0lb);
            loopVar |= me_modified(me);
            GECODE_ME_CHECK(me);

            LubRanges<SView> candub(candidate);
            me = x0.intersectI(home,candub);
            loopVar |= me_modified(me);
            GECODE_ME_CHECK(me);
            ++vx1;
          }
          new (&before[j]) LUBndSet(home);
          before[j].update(home,sofarBefore);
          GlbRanges<SView> clb(candidate);
          sofarBefore.intersectI(home,clb);
        }

        ++vx1ub;
        ++i; ++j;
      }

      // cancel the variables with index greater than
      // max of lub(x1)
      for (int k=i; k<n; k++) {
        iv[k].view.cancel(home,*this, PC_SET_ANY);
      }
      n = j;
      iv.size(n);

      if (x1.cardMax()==0) {
        // Elementor is empty, hence the result must be universe
        {
          IntSetRanges uniI(universe);
          GECODE_ME_CHECK(x0.includeI(home,uniI));
        }
        {
          IntSetRanges uniI(universe);
          GECODE_ME_CHECK(x0.intersectI(home,uniI));
        }
        for (int i=n; i--;)
          before[i].dispose(home);
        return ES_SUBSUMED(*this,home);
      }

      {
        // x0 >= sofarBefore
        BndSetRanges sfB(sofarBefore);
        ModEvent me = x0.includeI(home,sfB);
        loopVar |= me_modified(me);
        GECODE_ME_CHECK(me);
      }

      sofarBefore.dispose(home);

      LUBndSet sofarAfter(home, universe);

      // In the second iteration, this time backwards, compute
      // sofarAfter as the intersection of all glb(x_j) with j>i
      for (int i=n; i--;) {
        if (sofarAfter.size() == 0) break;

        // extra = inter(before[i], sofarAfter) - lub(x0)
        BndSetRanges b(before[i]);
        BndSetRanges s(sofarAfter);
        LubRanges<SView> x0ub(x0);
        Iter::Ranges::Inter<BndSetRanges, BndSetRanges> inter(b,s);
        Iter::Ranges::Diff<Iter::Ranges::Inter<BndSetRanges,
          BndSetRanges>, LubRanges<SView> > diff(inter, x0ub);
        if (diff()) {
          ModEvent me = (x1.include(home,iv[i].idx));
          loopVar |= me_modified(me);
          GECODE_ME_CHECK(me);

          // candidate != extra
          me = iv[i].view.excludeI(home,diff);
          loopVar |= me_modified(me);
          GECODE_ME_CHECK(me);
        }

        GlbRanges<SView> ivilb(iv[i].view);
        sofarAfter.intersectI(home,ivilb);
        before[i].dispose(home);
      }
      sofarAfter.dispose(home);

    } while (loopVar);

    // Test whether we determined x1 without determining x0
    if (x1.assigned() && !x0.assigned()) {
      int ubsize = x1.lubSize();
      if (ubsize > 2) {
        assert(ubsize==n);
        ViewArray<SView> is(home,ubsize);
        for (int i=n; i--;)
          is[i]=iv[i].view;
        GECODE_REWRITE(*this,(RelOp::IntersectionN<SView, SView>
                        ::post(home,is,x0)));
      } else if (ubsize == 2) {
        assert(n==2);
        SView a = iv[0].view;
        SView b = iv[1].view;
        GECODE_REWRITE(*this,(RelOp::Intersection<SView, SView, SView>
                       ::post(home,a,b,x0)));
      } else if (ubsize == 1) {
        assert(n==1);
        GECODE_REWRITE(*this,(Rel::Eq<SView,SView>::post(home,x0,iv[0].view)));
      } else {
        GECODE_ME_CHECK(x0.cardMax(home, 0));
        return ES_SUBSUMED(*this,home);
      }
    }

    bool allAssigned = true;
    for (int i=iv.size(); i--;) {
      if (!iv[i].view.assigned()) {
        allAssigned = false;
        break;
      }
    }
    if (x0.assigned() && x1.assigned() && allAssigned) {
      return ES_SUBSUMED(*this,home);
    }

    return ES_FIX;
  }

}}}

// STATISTICS: set-prop
