/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

#include <gecode/int.hh>

namespace Gecode { namespace Set { namespace Int {

  template<class View>
  forceinline
  ChannelInt<View>::ChannelInt(Home home,
                             ViewArray<Gecode::Int::IntView>& xs0,
                             ViewArray<View>& ys0)
    : Propagator(home), xs(xs0), ys(ys0) {
    xs.subscribe(home,*this, Gecode::Int::PC_INT_DOM);
    ys.subscribe(home,*this, PC_SET_ANY);
  }

  template<class View>
  forceinline
  ChannelInt<View>::ChannelInt(Space& home, bool share, ChannelInt& p)
    : Propagator(home,share,p) {
    xs.update(home,share,p.xs);
    ys.update(home,share,p.ys);
  }

  template<class View>
  forceinline ExecStatus
  ChannelInt<View>::post(Home home, ViewArray<Gecode::Int::IntView>& xs,
                          ViewArray<View>& ys) {
    // Sharing of ys is taken care of in the propagator:
    // The ys are propagated to be disjoint, so shared variables
    // result in failure.
    unsigned int xssize = xs.size();
    for (int i=ys.size(); i--;) {
      GECODE_ME_CHECK(ys[i].exclude(home, xssize, Limits::max));
      GECODE_ME_CHECK(ys[i].exclude(home, Limits::min, -1));
    }
    unsigned int yssize = ys.size();
    if (yssize > static_cast<unsigned int>(Gecode::Int::Limits::max))
      return ES_FAILED;
    for (int i=xs.size(); i--;) {
      GECODE_ME_CHECK(xs[i].gq(home, 0));
      GECODE_ME_CHECK(xs[i].le(home, static_cast<int>(yssize)));
    }

    (void) new (home) ChannelInt(home,xs,ys);
    return ES_OK;
  }

  template<class View>
  PropCost
  ChannelInt<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO, xs.size()+ys.size());
  }

  template<class View>
  size_t
  ChannelInt<View>::dispose(Space& home) {
    assert(!home.failed());
    xs.cancel(home,*this, Gecode::Int::PC_INT_DOM);
    ys.cancel(home,*this, PC_SET_ANY);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  Actor*
  ChannelInt<View>::copy(Space& home, bool share) {
    return new (home) ChannelInt(home,share,*this);
  }

  template<class View>
  ExecStatus
  ChannelInt<View>::propagate(Space& home, const ModEventDelta&) {
    int assigned = 0;
    for (int v=xs.size(); v--;) {
      if (xs[v].assigned()) {
        assigned += 1;
        for (int i=ys.size(); i--;) {
          if (i==xs[v].val()) {
            GECODE_ME_CHECK(ys[i].include(home, v));
          }
          else {
            GECODE_ME_CHECK(ys[i].exclude(home, v));
          }
        }
      } else {

        for (int i=ys.size(); i--;) {
          if (ys[i].notContains(v)) {
            GECODE_ME_CHECK(xs[v].nq(home, i));
          }
          if (ys[i].contains(v)) {
            GECODE_ME_CHECK(xs[v].eq(home, i));
          }
        }

        Gecode::Int::ViewRanges<Gecode::Int::IntView> xsv(xs[v]);
        int min = 0;
        for (; xsv(); ++xsv) {
          for (int i=min; i<xsv.min(); i++) {
            GECODE_ME_CHECK(ys[i].exclude(home, v));
          }
          min = xsv.max() + 1;
        }

      }
    }

    return (assigned==xs.size()) ? ES_SUBSUMED(*this,home) : ES_NOFIX;
  }

}}}

// STATISTICS: set-prop
