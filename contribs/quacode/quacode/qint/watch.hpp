/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
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
namespace Gecode { namespace Int {

  template<class View>
  forceinline
  Watch<View>::Watch(Home home, View x0, View x1, unsigned int _x0Size)
    : BinaryPropagator<View,PC_INT_DOM>(home,x0,x1), x0Size(_x0Size) {
    dynamic_cast<QSpaceInfo&>((Space&)home).addWatchConstraint();
  }

  template<class View>
  ExecStatus
  Watch<View>::post(Home home, View x0, View x1, unsigned int _x0Size) {
    if (x0.size() != _x0Size) {
      return ES_FAILED;
    } else if (x1.assigned()) {
      GECODE_ME_CHECK(x0.eq(home,x1.val()));
    } else {
      assert(!same(x0,x1));
      GECODE_ME_CHECK(x0.lq(home,x1.max()));
      GECODE_ME_CHECK(x1.lq(home,x0.max()));
      GECODE_ME_CHECK(x0.gq(home,x1.min()));
      GECODE_ME_CHECK(x1.gq(home,x0.min()));
      assert(x0.size() == x1.size());
      (void) new (home) Watch<View>(home,x0,x1,_x0Size);
    }
    return ES_OK;
  }

  template<class View>
  forceinline
  Watch<View>::Watch(Space& home, bool share, Watch<View>& p)
    : BinaryPropagator<View,PC_INT_DOM>(home,share,p), x0Size(p.x0Size) {}

  template<class View>
  Actor*
  Watch<View>::copy(Space& home,bool share) {
    return new (home) Watch<View>(home,share,*this);
  }

  template<class View>
  PropCost
  Watch<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::unary(PropCost::LO);
  }

  template<class View>
  ExecStatus
  Watch<View>::propagate(Space& home, const ModEventDelta&) {
    if (x0.size() != x0Size) return ES_FAILED;
    if (x1.assigned()) {
      GECODE_ME_CHECK(x0.eq(home,x1.val()));
      dynamic_cast<QSpaceInfo&>((Space&)home).delWatchConstraint();
      return home.ES_SUBSUMED(*this);
    }
    ViewRanges<View> r0(x0);
    GECODE_ME_CHECK(x1.inter_r(home,r0,shared(x0,x1)));
    ViewRanges<View> r1(x1);
    GECODE_ME_CHECK(x0.narrow_r(home,r1,shared(x0,x1)));
    if (x0.assigned()) {
      dynamic_cast<QSpaceInfo&>((Space&)home).delWatchConstraint();
      return home.ES_SUBSUMED(*this);
    }
    assert(x0.size() == x1.size());
    x0Size = x0.size();
    return ES_FIX;
  }

}}

// STATISTICS: int-prop
