/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
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

namespace Gecode { namespace Int { namespace NoOverlap {

  /*
   * Dimension with integer size
   *
   */
  forceinline
  IntDim::IntDim(void) 
    : s(0) {}
  forceinline
  IntDim::IntDim(IntView c0, int s0)
    : c(c0), s(s0) {}

  forceinline int 
  IntDim::ssc(void) const {
    return c.min();
  }
  forceinline int 
  IntDim::lsc(void) const {
    return c.max();
  }
  forceinline int 
  IntDim::sec(void) const {
    return c.min() + s;
  }
  forceinline int 
  IntDim::lec(void) const {
    return c.max() + s;
  }

  forceinline ExecStatus
  IntDim::ssc(Space& home, int n) {
    GECODE_ME_CHECK(c.gq(home, n));
    return ES_OK;
  }
  forceinline ExecStatus
  IntDim::lec(Space& home, int n) {
    GECODE_ME_CHECK(c.lq(home, n - s));
    return ES_OK;
  }
  forceinline ExecStatus
  IntDim::nooverlap(Space& home, int n, int m) {
    if (n <= m) {
      Iter::Ranges::Singleton r(n-s+1,m);
      GECODE_ME_CHECK(c.minus_r(home,r,false));
    }
    return ES_OK;
  }
  forceinline ExecStatus
  IntDim::nooverlap(Space& home, IntDim& d) {
    if (d.sec() > lsc()) {
      // Propagate that d must be after this
      GECODE_ES_CHECK(lec(home,d.lsc()));
      GECODE_ES_CHECK(d.ssc(home,sec()));
    } else {
      nooverlap(home, d.lsc(), d.sec()-1);
    }
    return ES_OK;
  }

  forceinline void
  IntDim::update(Space& home, bool share, IntDim& d) {
    c.update(home,share,d.c);
    s = d.s;
  }

  forceinline void
  IntDim::subscribe(Space& home, Propagator& p) {
    c.subscribe(home,p,PC_INT_DOM);
  }
  forceinline void
  IntDim::cancel(Space& home, Propagator& p) {
    c.cancel(home,p,PC_INT_DOM);
  }


  /*
   * Dimension with integer view size
   *
   */
  forceinline
  ViewDim::ViewDim(void) {}
  forceinline
  ViewDim::ViewDim(IntView c0, IntView s0)
    : c(c0), s(s0) {}

  forceinline int 
  ViewDim::ssc(void) const {
    return c.min();
  }
  forceinline int 
  ViewDim::lsc(void) const {
    return c.max();
  }
  forceinline int 
  ViewDim::sec(void) const {
    return c.min() + s.min();
  }
  forceinline int 
  ViewDim::lec(void) const {
    return c.max() + s.max();
  }

  forceinline ExecStatus
  ViewDim::ssc(Space& home, int n) {
    GECODE_ME_CHECK(c.gq(home, n));
    return ES_OK;
  }
  forceinline ExecStatus
  ViewDim::lec(Space& home, int n) {
    GECODE_ME_CHECK(c.lq(home, n - s.min()));
    GECODE_ME_CHECK(s.lq(home, n - c.min()));
    return ES_OK;
  }
  forceinline ExecStatus
  ViewDim::nooverlap(Space& home, int n, int m) {
    if (n <= m) {
      Iter::Ranges::Singleton r(n-s.min()+1,m);
      GECODE_ME_CHECK(c.minus_r(home,r,false));
    }
    return ES_OK;
  }
  forceinline ExecStatus
  ViewDim::nooverlap(Space& home, ViewDim& d) {
    if (d.sec() > lsc()) {
      // Propagate that d must be after this
      GECODE_ES_CHECK(lec(home,d.lsc()));
      GECODE_ES_CHECK(d.ssc(home,sec()));
    } else {
      nooverlap(home, d.lsc(), d.sec()-1);
    }
    return ES_OK;
  }


  forceinline void
  ViewDim::update(Space& home, bool share, ViewDim& d) {
    c.update(home,share,d.c);
    s.update(home,share,d.s);
  }

  forceinline void
  ViewDim::subscribe(Space& home, Propagator& p) {
    c.subscribe(home,p,PC_INT_DOM);
    s.subscribe(home,p,PC_INT_BND);
  }
  forceinline void
  ViewDim::cancel(Space& home, Propagator& p) {
    c.cancel(home,p,PC_INT_DOM);
    s.cancel(home,p,PC_INT_BND);
  }

}}}

// STATISTICS: int-prop

