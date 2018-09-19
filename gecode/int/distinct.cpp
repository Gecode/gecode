/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Contributing authors:
 *     Roberto Castañeda Lozano <rcas@kth.se>
 *
 *  Copyright:
 *     Roberto Castañeda Lozano, 2015
 *     Christian Schulte, 2002
 *     Gabor Szokoli, 2003
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

#include <gecode/int/distinct.hh>
#include <gecode/int/bool.hh>

namespace Gecode {

  void
  distinct(Home home, const IntVarArgs& x, IntPropLevel ipl) {
    using namespace Int;
    if (same(x))
      throw ArgumentSame("Int::distinct");
    GECODE_POST;
    ViewArray<IntView> xv(home,x);
    switch (vbd(ipl)) {
    case IPL_BND:
      GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home,xv));
      break;
    case IPL_DOM:
      GECODE_ES_FAIL(Distinct::Dom<IntView>::post(home,xv));
      break;
    default:
      GECODE_ES_FAIL(Distinct::Val<IntView>::post(home,xv));
    }
  }

  void
  distinct(Home home, const IntArgs& c, const IntVarArgs& x,
           IntPropLevel ipl) {
    using namespace Int;
    if (same(x))
      throw ArgumentSame("Int::distinct");
    if (c.size() != x.size())
      throw ArgumentSizeMismatch("Int::distinct");
    GECODE_POST;
    ViewArray<OffsetView> cx(home,x.size());
    for (int i=0; i<c.size(); i++) {
      long long int cx_min = (static_cast<long long int>(c[i]) +
                              static_cast<long long int>(x[i].min()));
      long long int cx_max = (static_cast<long long int>(c[i]) +
                              static_cast<long long int>(x[i].max()));
      Limits::check(c[i],"Int::distinct");
      Limits::check(cx_min,"Int::distinct");
      Limits::check(cx_max,"Int::distinct");
      cx[i] = OffsetView(x[i],c[i]);
    }
    switch (vbd(ipl)) {
    case IPL_BND:
      GECODE_ES_FAIL(Distinct::Bnd<OffsetView>::post(home,cx));
      break;
    case IPL_DOM:
      GECODE_ES_FAIL(Distinct::Dom<OffsetView>::post(home,cx));
      break;
    default:
      GECODE_ES_FAIL(Distinct::Val<OffsetView>::post(home,cx));
    }
  }

  void
  distinct(Home home, const BoolVarArgs& b, const IntVarArgs& x,
           IntPropLevel ipl) {
    using namespace Int;
    if (same(x))
      throw ArgumentSame("Int::distinct");
    if (b.size() != x.size())
      throw ArgumentSizeMismatch("Int::distinct");
    GECODE_POST;

    int n = x.size();
    int min = Limits::max;
    int max = Limits::min;
    int m = 0;
    for (int i=0; i<n; i++)
      if (!b[i].zero()) {
        min = std::min(min,x[i].min());
        max = std::max(max,x[i].max());
        m++;
      }

    if (m < 2)
      return;

    int start;
    if (max < Limits::max-m)
      start = max+1;
    else if (min > Limits::min+m)
      start = min-(m+1);
    else
      throw OutOfLimits("Int::distinct");

    ViewArray<IntView> y(home,m);
    int j = 0;
    for (int i=0; i<n; i++)
      if (b[i].one()) {
        y[j] = x[i]; j++;
      } else if (b[i].none()) {
        y[j] = IntVar(home, Limits::min, Limits::max);
        GECODE_ES_FAIL((Bool::IteDom<IntView,ConstIntView,IntView>::post
                        (home, b[i], x[i], start+j, y[j])));
        j++;
      }
    assert(j == m);

    switch (vbd(ipl)) {
    case IPL_BND:
      GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home,y));
      break;
    case IPL_DOM:
      GECODE_ES_FAIL(Distinct::Dom<IntView>::post(home,y));
      break;
    default:
      GECODE_ES_FAIL(Distinct::Val<IntView>::post(home,y));
    }
  }

  void
  distinct(Home home, const IntVarArgs& x, int c,
           IntPropLevel ipl) {
    using namespace Int;
    if (same(x))
      throw ArgumentSame("Int::distinct");
    GECODE_POST;

    int n = x.size();
    int min = Limits::max;
    int max = Limits::min;
    int m = 0;
    for (int i=0; i<n; i++)
      if (!(x[i].assigned() && (x[i].val() == c))) {
        min = std::min(min,x[i].min());
        max = std::max(max,x[i].max());
        m++;
      }

    if (m < 2)
      return;

    int start;
    if (max < Limits::max-m)
      start = max+1;
    else if (min > Limits::min+m)
      start = min-(m+1);
    else
      throw OutOfLimits("Int::distinct");

    ViewArray<IntView> y(home,m);
    int j = 0;
    for (int i=0; i<n; i++)
      if (!x[i].in(c)) {
        y[j] = x[i]; j++;
      } else if (!(x[i].assigned() && (x[i].val() == c))) {
        y[j] = IntVar(home, Limits::min, Limits::max);
        GECODE_ES_FAIL(Distinct::EqIte::post
                       (home, x[i], y[j], c, start+j));
        j++;
      }
    assert(j == m);

    switch (vbd(ipl)) {
    case IPL_BND:
      GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home,y));
      break;
    case IPL_DOM:
      GECODE_ES_FAIL(Distinct::Dom<IntView>::post(home,y));
      break;
    default:
      GECODE_ES_FAIL(Distinct::Val<IntView>::post(home,y));
    }
  }

}

// STATISTICS: int-post
