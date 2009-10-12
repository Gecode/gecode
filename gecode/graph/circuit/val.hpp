/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

namespace Gecode { namespace Graph { namespace Circuit {

  /*
   * The actual propagator
   *
   */
  template<class View>
  forceinline
  Val<View>::Val(Home home, ViewArray<View>& x)
    : Base<View>(home,x) {}

  template<class View>
  forceinline
  Val<View>::Val(Space& home, bool share, Val<View>& p)
    : Base<View>(home,share,p) {}

  template<class View>
  Actor*
  Val<View>::copy(Space& home, bool share) {
    return new (home) Val<View>(home,share,*this);
  }

  template<class View>
  PropCost
  Val<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI, x.size());
  }

  template<class View>
  ExecStatus
  Val<View>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK((Int::Distinct::prop_val<View,true>(home,y)));
    ExecStatus esc = connected(home);
    if (esc != ES_FIX)
      return esc;
    /*
     * One cannot have a single unassigned view as the views constitute
     * a permutation.
     */
    if (y.size() < 2)
      return ES_SUBSUMED(*this,home);
    return path(home);
  }

  template<class View>
  ExecStatus
  Val<View>::post(Home home, ViewArray<View>& x) {
    int n = x.size();
    if (n == 1) {
      GECODE_ME_CHECK(x[0].eq(home,0));
    } else if (n == 2) {
      GECODE_ME_CHECK(x[0].eq(home,1));
      GECODE_ME_CHECK(x[1].eq(home,0));
    } else {
      for (int i=n; i--; ) {
        GECODE_ME_CHECK(x[i].gq(home,0));
        GECODE_ME_CHECK(x[i].le(home,n));
        GECODE_ME_CHECK(x[i].nq(home,i));
      }
      (void) new (home) Val<View>(home,x);
    }
    return ES_OK;
  }

}}}

// STATISTICS: graph-prop

