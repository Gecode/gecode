/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#include <gecode/int/rel.hh>

namespace Gecode { namespace Int { namespace Count {

  /*
   * Counting domain consistent equality
   *
   */

  template<class VX>
  forceinline RelTest
  holds(VX x, ConstIntView y) {
    return rtest_eq_dom(x,y.val());
  }
  template<class VX>
  forceinline RelTest
  holds(VX x, ZeroIntView) {
    return rtest_eq_dom(x,0);
  }
  template<class VX>
  forceinline RelTest
  holds(VX x, VX y) {
    return rtest_eq_dom(x,y);
  }
  template<class VX>
  forceinline ExecStatus
  post_true(Home home, VX x, ConstIntView y) {
    GECODE_ME_CHECK(x.eq(home,y.val()));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_true(Home home, VX x, ZeroIntView) {
    GECODE_ME_CHECK(x.eq(home,0));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_true(Home home, ViewArray<VX>& x, ConstIntView y) {
    for (int i = x.size(); i--; )
      GECODE_ME_CHECK(x[i].eq(home,y.val()));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_true(Home home, ViewArray<VX>& x, ZeroIntView) {
    for (int i = x.size(); i--; )
      GECODE_ME_CHECK(x[i].eq(home,0));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_false(Home home, VX x, ConstIntView y) {
    GECODE_ME_CHECK(x.nq(home,y.val()));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_false(Home home, VX x, ZeroIntView) {
    GECODE_ME_CHECK(x.nq(home,0));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_false(Home home, ViewArray<VX>& x, ConstIntView y) {
    for (int i = x.size(); i--; )
      GECODE_ME_CHECK(x[i].nq(home,y.val()));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_false(Home home, ViewArray<VX>& x, ZeroIntView) {
    for (int i = x.size(); i--; )
      GECODE_ME_CHECK(x[i].nq(home,0));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_true(Home home, ViewArray<VX>& x, VX y) {
    ViewArray<VX> z(home,x.size()+1);
    z[x.size()] = y;
    for (int i = x.size(); i--; )
      z[i] = x[i];
    return Rel::NaryEqDom<VX>::post(home,z);
  }
  template<class VX>
  forceinline ExecStatus
  post_true(Home home, VX x, VX y) {
    return Rel::EqDom<VX,VX>::post(home,x,y);
  }
  template<class VX>
  forceinline ExecStatus
  post_false(Home home, ViewArray<VX>& x, VX y) {
    for (int i = x.size(); i--; )
      GECODE_ES_CHECK(Rel::Nq<VX>::post(home,x[i],y));
    return ES_OK;
  }
  template<class VX>
  forceinline ExecStatus
  post_false(Home home, VX x, VX y) {
    return Rel::Nq<VX>::post(home,x,y);
  }

}}}

// STATISTICS: int-prop
