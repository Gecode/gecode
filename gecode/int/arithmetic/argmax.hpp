/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <gecode/int/rel.hh>

namespace Gecode { namespace Int { namespace Arithmetic {

  template<class VA, class VB, bool tiebreak>
  forceinline
  ArgMax<VA,VB,tiebreak>::ArgMax(Home home, IdxViewArray<VA>& x0, VB y0)
    : Propagator(home), x(x0), y(y0) {
    x.subscribe(home,*this,PC_INT_BND);
    y.subscribe(home,*this,PC_INT_DOM);
  }

  template<class VA, class VB, bool tiebreak>
  ExecStatus
  ArgMax<VA,VB,tiebreak>::post(Home home, IdxViewArray<VA>& x, VB y) {
    assert(x.size() > 0);
    if (x.size() == 1) {
      GECODE_ME_CHECK(y.eq(home,x[0].idx));
    } else if (y.assigned()) {
      int max=0;
      while (x[max].idx < y.val())
        max++;
      assert(x[max].idx == y.val());
      if (tiebreak)
        for (int i=0; i<max; i++)
          GECODE_ES_CHECK((Rel::Le<VA,VA>::post(home,
                                                x[i].view,x[max].view)));
      else
        for (int i=0; i<max; i++)
          GECODE_ES_CHECK((Rel::Lq<VA,VA>::post(home,
                                                x[i].view,x[max].view)));
      for (int i=max+1; i<x.size(); i++)
        GECODE_ES_CHECK((Rel::Lq<VA,VA>::post(home,
                                              x[i].view,x[max].view)));
    } else {
      (void) new (home) ArgMax<VA,VB,tiebreak>(home,x,y);
    }
    return ES_OK;
  }

  template<class VA, class VB, bool tiebreak>
  forceinline
  ArgMax<VA,VB,tiebreak>::ArgMax(Space& home, ArgMax<VA,VB,tiebreak>& p)
    : Propagator(home,p) {
    x.update(home,p.x);
    y.update(home,p.y);
  }

  template<class VA, class VB, bool tiebreak>
  Actor*
  ArgMax<VA,VB,tiebreak>::copy(Space& home) {
    return new (home) ArgMax<VA,VB,tiebreak>(home,*this);
  }

  template<class VA, class VB, bool tiebreak>
  PropCost
  ArgMax<VA,VB,tiebreak>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size()+1);
  }

  template<class VA, class VB, bool tiebreak>
  void
  ArgMax<VA,VB,tiebreak>::reschedule(Space& home) {
    x.reschedule(home,*this,PC_INT_BND);
    y.reschedule(home,*this,PC_INT_DOM);
  }

  template<class VA, class VB, bool tiebreak>
  ExecStatus
  ArgMax<VA,VB,tiebreak>::propagate(Space& home, const ModEventDelta&) {
    /*
     * A key invariant is as follows: for each value i in the domain
     * of y there is an index-value pair with index i in x.
     */

    // Compute lower and upper bounds for the maximum and its first position.
    int p = x[0].idx;
    int l = x[0].view.min();
    int u = x[0].view.max();
    for (int i=1; i<x.size(); i++) {
      if (l < x[i].view.min()) {
        p = x[i].idx; l = x[i].view.min();
      }
      if (u < x[i].view.max())
        u = x[i].view.max();
    }

    // Eliminate elements from x and y that are too small
    {
      Region r;

      // Values to delete from y
      int* d=r.alloc<int>(y.size());
      // Number of values to delete
      int  n = 0;

      int i=0, j=0;
      ViewValues<VB> iy(y);

      while ((i < x.size()) && iy()) {
        if (x[i].idx == iy.val()) {
          if (x[i].view.max() < l) {
            x[i].view.cancel(home,*this,PC_INT_BND);
            d[n++]=x[i].idx;
          } else {
            x[j++]=x[i];
          }
          ++iy;
        } else {
          assert(x[i].idx < iy.val());
          if (x[i].view.max() < l) {
            x[i].view.cancel(home,*this,PC_INT_BND);
          } else {
            x[j++]=x[i];
          }
        }
        i++;
      }
      while (i < x.size())
        if (x[i].view.max() < l) {
          x[i].view.cancel(home,*this,PC_INT_BND); i++;
        } else {
          x[j++]=x[i++];
        }
      x.size(j);

      if (static_cast<unsigned int>(n) == y.size())
        return ES_FAILED;
      Iter::Values::Array id(d,n);
      GECODE_ME_CHECK(y.minus_v(home,id,false));
    }

    /*
     * Now the following invariant holds:
     * - for all q in y: u >= x(q).max() >= l
     * - if l==u, then exists q in y: x(q).max = u
     */

    if (tiebreak && (l == u))
      GECODE_ME_CHECK(y.lq(home,p));

    if (y.assigned()) {
      int max=0;
      while (x[max].idx < y.val())
        max++;
      assert(x[max].idx == y.val());
      if (tiebreak)
        for (int i=0; i<max; i++)
          GECODE_ES_CHECK((Rel::Le<VA,VA>::post(home(*this),
                                                x[i].view,x[max].view)));
      else
        for (int i=0; i<max; i++)
          GECODE_ES_CHECK((Rel::Lq<VA,VA>::post(home(*this),
                                               x[i].view,x[max].view)));
      for (int i=max+1; i<x.size(); i++)
        GECODE_ES_CHECK((Rel::Lq<VA,VA>::post(home(*this),
                                              x[i].view,x[max].view)));
      return home.ES_SUBSUMED(*this);
    }

    // Recompute the upper bound for elements in y
    {
      ViewValues<VB> iy(y);
      int i=0;
      // Skip smaller elements
      while (x[i].idx < y.min())
        i++;
      u=x[i].view.max();
      // Skip the minimal element
      i++; ++iy;
      while (iy()) {
        if (x[i].idx == iy.val()) {
          u = std::max(u,x[i].view.max());
          ++iy;
        } else {
          assert(x[i].idx < iy.val());
        }
        i++;
      }
    }

    // Constrain elements in x but not in y
    {
      int i = 0;
      // Elements which must be smaller (for tiebreaking)
      if (tiebreak)
        while ((i < x.size()) && (x[i].idx < y.min())) {
          GECODE_ME_CHECK(x[i].view.le(home,u));
          i++;
        }
      else
        while ((i < x.size()) && (x[i].idx < y.min())) {
          GECODE_ME_CHECK(x[i].view.lq(home,u));
          i++;
        }
      assert(x[i].idx == y.min());

      // Elements which cannot be larger
      ViewValues<VB> iy(y);
      // Skip the minimal element
      i++; ++iy;
      while ((i < x.size()) && iy()) {
        if (x[i].idx == iy.val()) {
          ++iy;
        } else {
          assert(x[i].idx < iy.val());
          GECODE_ME_CHECK(x[i].view.lq(home,u));
        }
        i++;
      }
      while (i < x.size()) {
        assert(x[i].idx > y.max());
        GECODE_ME_CHECK(x[i].view.lq(home,u));
        i++;
      }
    }
    return tiebreak ? ES_NOFIX : ES_FIX;
  }

  template<class VA, class VB, bool tiebreak>
  forceinline size_t
  ArgMax<VA,VB,tiebreak>::dispose(Space& home) {
    x.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_DOM);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: int-prop

