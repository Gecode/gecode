/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2012
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

#include <gecode/float/rel.hh>

#include <algorithm>

namespace Gecode {

  void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVal n) {
    using namespace Float;
    Limits::check(n,"Float::rel");
    if (home.failed()) return;
    FloatView x(x0);
    switch (frt) {
    case FRT_EQ: GECODE_ME_FAIL(x.eq(home,n)); break;
    case FRT_LQ: GECODE_ME_FAIL(x.lq(home,n)); break;
    case FRT_GQ: GECODE_ME_FAIL(x.gq(home,n)); break;
    default: throw UnknownRelation("Float::rel");
    }
  }

  void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVar x1) {
    using namespace Float;
    if (home.failed()) return;
    switch (frt) {
    case FRT_EQ:
      GECODE_ES_FAIL((Rel::Eq<FloatView,FloatView>::post(home,x0,x1)));
      break;
    case FRT_GQ:
      std::swap(x0,x1); // Fall through
    case FRT_LQ:
      GECODE_ES_FAIL((Rel::Lq<FloatView>::post(home,x0,x1))); break;
    default:
      throw UnknownRelation("Float::rel");
    }
  }

#define GECODE_WRITE_REIFIED_UNA_PROP(name,view0,mode,a,c,bvar) \
        if (b) GECODE_ES_FAIL((name<view0,Int::BoolView,mode>:: \
                               post(home,a,c,bvar))); \
        else   GECODE_ES_FAIL((name<view0,Int::NegBoolView,mode>:: \
                               post(home,a,c,Int::NegBoolView(bvar))));

  void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVar x1, Reify r, bool b) {
    using namespace Float;
    if (home.failed()) return;
    switch (frt) {
    case FRT_EQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReEq,FloatView,RM_EQV,x0,x1,r.var());
        break;
      case RM_IMP:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReEq,FloatView,RM_IMP,x0,x1,r.var());
        break;
      case RM_PMI:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReEq,FloatView,RM_PMI,x0,x1,r.var());
        break;
      default: throw Int::UnknownReifyMode("Float::rel");
      }
      break;
    case FRT_GQ:
      std::swap(x0,x1); // Fall through
    case FRT_LQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReLq,FloatView,RM_EQV,x0,x1,r.var());
        break;
      case RM_IMP:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReLq,FloatView,RM_EQV,x0,x1,r.var());
        break;
      case RM_PMI:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReLq,FloatView,RM_EQV,x0,x1,r.var());
        break;
      default: throw Int::UnknownReifyMode("Float::rel");
      }
      break;
    default:
      throw Int::UnknownRelation("Float::rel");
    }
  }

  void
  rel(Home home, FloatVar x, FloatRelType frt, FloatVal n, Reify r, bool b) {
    using namespace Float;
    Limits::check(n,"Float::rel");
    if (home.failed()) return;
    switch (frt) {
    case FRT_EQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_EQV,x,n,r.var());
        break;
      case RM_IMP:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_IMP,x,n,r.var());
        break;
      case RM_PMI:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_PMI,x,n,r.var());
        break;
      default: throw Int::UnknownReifyMode("Float::rel");
      }
    case FRT_LQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,FloatView,RM_EQV,x,n,r.var());
        break;
      case RM_IMP:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,FloatView,RM_IMP,x,n,r.var());
        break;
      case RM_PMI:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,FloatView,RM_PMI,x,n,r.var());
        break;
      default: throw Int::UnknownReifyMode("Float::rel");
      }
      break;
    case FRT_GQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReGqFloat,FloatView,RM_EQV,x,n,r.var());
        break;
      case RM_IMP:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReGqFloat,FloatView,RM_IMP,x,n,r.var());
        break;
      case RM_PMI:
        GECODE_WRITE_REIFIED_UNA_PROP(Rel::ReGqFloat,FloatView,RM_PMI,x,n,r.var());
        break;
      default: throw Int::UnknownReifyMode("Float::rel");
      }
      break;
    default:
      throw Int::UnknownRelation("Float::rel");
    }
  }

#undef GECODE_WRITE_REIFIED_UNA_PROP

  void
  rel(Home home, const FloatVarArgs& x, FloatRelType frt, FloatVal c) {
    using namespace Float;
    Limits::check(c,"Float::rel");
    if (home.failed()) return;
    switch (frt) {
    case FRT_EQ:
      for (int i=x.size(); i--; ) {
        FloatView xi(x[i]); GECODE_ME_FAIL(xi.eq(home,c));
      }
      break;
    case FRT_LQ:
      for (int i=x.size(); i--; ) {
        FloatView xi(x[i]); GECODE_ME_FAIL(xi.lq(home,c));
      }
      break;
    case FRT_GQ:
      for (int i=x.size(); i--; ) {
        FloatView xi(x[i]); GECODE_ME_FAIL(xi.gq(home,c));
      }
      break;
    default:
      throw UnknownRelation("Float::rel");
    }
  }

}

// STATISTICS: float-post
