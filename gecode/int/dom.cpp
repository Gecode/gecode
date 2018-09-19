/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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


#include <gecode/int/dom.hh>
#include <gecode/int/rel.hh>

namespace Gecode {

  void
  dom(Home home, IntVar x, int n, IntPropLevel) {
    using namespace Int;
    Limits::check(n,"Int::dom");
    GECODE_POST;
    IntView xv(x);
    GECODE_ME_FAIL(xv.eq(home,n));
  }

  void
  dom(Home home, const IntVarArgs& x, int n, IntPropLevel) {
    using namespace Int;
    Limits::check(n,"Int::dom");
    GECODE_POST;
    for (int i=0; i<x.size(); i++) {
      IntView xv(x[i]);
      GECODE_ME_FAIL(xv.eq(home,n));
    }
  }

  void
  dom(Home home, IntVar x, int min, int max, IntPropLevel) {
    using namespace Int;
    Limits::check(min,"Int::dom");
    Limits::check(max,"Int::dom");
    GECODE_POST;
    IntView xv(x);
    GECODE_ME_FAIL(xv.gq(home,min));
    GECODE_ME_FAIL(xv.lq(home,max));
  }

  void
  dom(Home home, const IntVarArgs& x, int min, int max, IntPropLevel) {
    using namespace Int;
    Limits::check(min,"Int::dom");
    Limits::check(max,"Int::dom");
    GECODE_POST;
    for (int i=0; i<x.size(); i++) {
      IntView xv(x[i]);
      GECODE_ME_FAIL(xv.gq(home,min));
      GECODE_ME_FAIL(xv.lq(home,max));
    }
  }

  void
  dom(Home home, IntVar x, const IntSet& is, IntPropLevel) {
    using namespace Int;
    Limits::check(is.min(),"Int::dom");
    Limits::check(is.max(),"Int::dom");
    GECODE_POST;
    IntView xv(x);
    IntSetRanges ris(is);
    GECODE_ME_FAIL(xv.inter_r(home,ris,false));
  }

  void
  dom(Home home, const IntVarArgs& x, const IntSet& is, IntPropLevel) {
    using namespace Int;
    Limits::check(is.min(),"Int::dom");
    Limits::check(is.max(),"Int::dom");
    GECODE_POST;
    for (int i=0; i<x.size(); i++) {
      IntSetRanges ris(is);
      IntView xv(x[i]);
      GECODE_ME_FAIL(xv.inter_r(home,ris,false));
    }
  }

  void
  dom(Home home, IntVar x, int n, Reify r, IntPropLevel) {
    using namespace Int;
    Limits::check(n,"Int::dom");
    GECODE_POST;
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Rel::ReEqDomInt<IntView,BoolView,RM_EQV>
                      ::post(home,x,n,r.var())));
      break;
    case RM_IMP:
      GECODE_ES_FAIL((Rel::ReEqDomInt<IntView,BoolView,RM_IMP>
                      ::post(home,x,n,r.var())));
      break;
    case RM_PMI:
      GECODE_ES_FAIL((Rel::ReEqDomInt<IntView,BoolView,RM_PMI>
                      ::post(home,x,n,r.var())));
      break;
    default: throw UnknownReifyMode("Int::dom");
    }
  }

  void
  dom(Home home, IntVar x, int min, int max, Reify r, IntPropLevel) {
    using namespace Int;
    Limits::check(min,"Int::dom");
    Limits::check(max,"Int::dom");
    GECODE_POST;
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Dom::ReRange<IntView,RM_EQV>
                      ::post(home,x,min,max,r.var())));
      break;
    case RM_IMP:
      GECODE_ES_FAIL((Dom::ReRange<IntView,RM_IMP>
                      ::post(home,x,min,max,r.var())));
      break;
    case RM_PMI:
      GECODE_ES_FAIL((Dom::ReRange<IntView,RM_PMI>
                      ::post(home,x,min,max,r.var())));
      break;
    default: throw UnknownReifyMode("Int::dom");
    }
  }


  void
  dom(Home home, IntVar x, const IntSet& is, Reify r, IntPropLevel) {
    using namespace Int;
    Limits::check(is.min(),"Int::dom");
    Limits::check(is.max(),"Int::dom");
    GECODE_POST;
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Dom::ReIntSet<IntView,RM_EQV>::post(home,x,is,r.var())));
      break;
    case RM_IMP:
      GECODE_ES_FAIL((Dom::ReIntSet<IntView,RM_IMP>::post(home,x,is,r.var())));
      break;
    case RM_PMI:
      GECODE_ES_FAIL((Dom::ReIntSet<IntView,RM_PMI>::post(home,x,is,r.var())));
      break;
    default: throw UnknownReifyMode("Int::dom");
    }
  }

  void
  dom(Home home, IntVar x, IntVar d, IntPropLevel) {
    using namespace Int;
    GECODE_POST;
    IntView xv(x), dv(d);
    if (xv != dv) {
      ViewRanges<IntView> r(dv);
      GECODE_ME_FAIL(xv.inter_r(home,r,false));
    }
  }

  void
  dom(Home home, BoolVar x, BoolVar d, IntPropLevel) {
    using namespace Int;
    GECODE_POST;
    if (d.one())
      GECODE_ME_FAIL(BoolView(x).one(home));
    else if (d.zero())
      GECODE_ME_FAIL(BoolView(x).zero(home));
  }

  void
  dom(Home home, const IntVarArgs& x, const IntVarArgs& d, IntPropLevel) {
    using namespace Int;
    if (x.size() != d.size())
      throw ArgumentSizeMismatch("Int::dom");
    for (int i=0; i<x.size(); i++) {
      GECODE_POST;
      IntView xv(x[i]), dv(d[i]);
      if (xv != dv) {
        ViewRanges<IntView> r(dv);
        GECODE_ME_FAIL(xv.inter_r(home,r,false));
      }
    }
  }

  void
  dom(Home home, const BoolVarArgs& x, const BoolVarArgs& d, IntPropLevel) {
    using namespace Int;
    if (x.size() != d.size())
      throw ArgumentSizeMismatch("Int::dom");
    for (int i=0; i<x.size(); i++) {
      GECODE_POST;
      if (d[i].one())
        GECODE_ME_FAIL(BoolView(x[i]).one(home));
      else if (d[i].zero())
        GECODE_ME_FAIL(BoolView(x[i]).zero(home));
    }
  }

}

// STATISTICS: int-post

