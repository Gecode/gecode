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

#include <gecode/set.hh>

#include <gecode/set/int.hh>
#include <gecode/set/rel.hh>

namespace Gecode {

  void
  rel(Home home, SetVar s, IntRelType rt, IntVar x) {
    GECODE_POST;
    switch (rt) {
    case IRT_EQ:
      {
        Gecode::Int::IntView xv(x);
        Set::SingletonView xsingle(xv);
        GECODE_ES_FAIL(
                       (Set::Rel::Eq<Set::SetView,Set::SingletonView>
                        ::post(home,s,xsingle)));

      }
      break;
    case IRT_NQ:
      {
        Gecode::Set::SetView sv(s);
        GECODE_ME_FAIL( sv.cardMin(home, 1));
        Gecode::Int::IntView xv(x);
        Set::SingletonView xsingle(xv);
        GECODE_ES_FAIL(
                       (Set::Rel::NoSubset<Set::SingletonView,Set::SetView>
                        ::post(home,xsingle,sv)));

      }
      break;
    case IRT_LQ:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_LQ, x);
        GECODE_ES_FAIL(Set::Int::MaxElement<Set::SetView>::post(home,s,tmp));
      }
      break;
    case IRT_LE:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_LE, x);
        GECODE_ES_FAIL(Set::Int::MaxElement<Set::SetView>::post(home,s,tmp));
      }
      break;
    case IRT_GQ:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_GQ, x);
        GECODE_ES_FAIL(Set::Int::MinElement<Set::SetView>::post(home,s,tmp));
      }
      break;
    case IRT_GR:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_GR, x);
        GECODE_ES_FAIL(Set::Int::MinElement<Set::SetView>::post(home,s,tmp));
      }
      break;
    default:
      throw Int::UnknownRelation("Set::rel");
    }

  }

}

namespace Gecode { namespace Set { namespace Int {

  /// Reify \a m to be the minimum of \a s
  void remin(Home home, SetVar s, IntVar m, Reify r) {
    IntVar c(home, 0, static_cast<int>(Set::Limits::card));
    cardinality(home, s, c);
    // Whether set is not empty
    BoolVar ne(home, 0, 1);
    rel(home, c, IRT_GR, 0, ne);
    if (r.mode() != RM_PMI)
      rel(home, r.var(), BOT_IMP, ne, 1);
    min(home, s, m, ne);
  }

  /// Reify \a m to be the maximum of \a s
  void remax(Home home, SetVar s, IntVar m, Reify r) {
    IntVar c(home, 0, static_cast<int>(Set::Limits::card));
    cardinality(home, s, c);
    // Whether set is not empty
    BoolVar ne(home, 0, 1);
    rel(home, c, IRT_GR, 0, ne);
    if (r.mode() != RM_PMI)
      rel(home, r.var(), BOT_IMP, ne, 1);
    max(home, s, m, ne);
  }

}}}

namespace Gecode {

  void
  rel(Home home, SetVar s, IntRelType rt, IntVar x, Reify r) {
    GECODE_POST;
    switch (rt) {
    case IRT_EQ:
      {
        Gecode::Int::IntView xv(x);
        Set::SingletonView xs(xv);
        switch (r.mode()) {
        case RM_EQV:
          GECODE_ES_FAIL((Set::Rel::ReEq<Set::SetView,Set::SingletonView,
                          Gecode::Int::BoolView,RM_EQV>
                          ::post(home,s,xs,r.var())));
          break;
        case RM_IMP:
          GECODE_ES_FAIL((Set::Rel::ReEq<Set::SetView,Set::SingletonView,
                          Gecode::Int::BoolView,RM_IMP>
                          ::post(home,s,xs,r.var())));
          break;
        case RM_PMI:
          GECODE_ES_FAIL((Set::Rel::ReEq<Set::SetView,Set::SingletonView,
                          Gecode::Int::BoolView,RM_PMI>
                          ::post(home,s,xs,r.var())));
          break;
        default:
          throw Gecode::Int::UnknownReifyMode("Set::rel");
        }
      }
      break;
    case IRT_NQ:
      {
        IntVar c(home, 0, static_cast<int>(Set::Limits::card));
        cardinality(home, s, c);
        // Whether set is not empty
        BoolVar ne(home, 0 , 1);
        rel(home, c, IRT_GR, 0, ne);
        // Whether {x} is a subset of s
        BoolVar ss(home, 0 , 1);
        rel(home, x, SRT_SUB, s, ss);
        BoolVar b;
        switch (r.mode()) {
        case RM_EQV:
          b=r.var();
          break;
        case RM_IMP:
          b=BoolVar(home, 0, 1);
          rel(home, r.var(), BOT_IMP, b, 1);
          break;
        case RM_PMI:
          b=BoolVar(home, 0, 1);
          rel(home, b, BOT_IMP, r.var(), 1);
          break;
        default:
          throw Gecode::Int::UnknownReifyMode("Set::rel");
        }
        BoolVarArgs p(1); p[0]=ne;
        BoolVarArgs n(1); n[0]=ss;
        clause(home, BOT_AND, p, n, b);
      }
      break;
    case IRT_LQ:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_LQ, x, r);
        Gecode::Set::Int::remax(home, s, tmp, r);
      }
      break;
    case IRT_LE:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_LE, x, r);
        Gecode::Set::Int::remax(home, s, tmp, r);
      }
      break;
    case IRT_GQ:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_GQ, x, r);
        Gecode::Set::Int::remin(home, s, tmp, r);
      }
      break;
    case IRT_GR:
      {
        IntVar tmp(home, Int::Limits::min, Int::Limits::max);
        rel(home, tmp, IRT_GR, x, r);
        Gecode::Set::Int::remin(home, s, tmp, r);
      }
      break;
    default:
      throw Int::UnknownRelation("Set::rel");
    }
  }

  void
  min(Home home, SetVar s, IntVar x) {
    GECODE_POST;
    GECODE_ES_FAIL(Set::Int::MinElement<Set::SetView>::post(home,s,x));
  }

  void
  notMin(Home home, SetVar s, IntVar x) {
    GECODE_POST;
    GECODE_ES_FAIL(Set::Int::NotMinElement<Set::SetView>::post(home,s,x));
  }

  void
  min(Home home, SetVar s, IntVar x, Reify r) {
    GECODE_POST;
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Set::Int::ReMinElement<Set::SetView,RM_EQV>
                     ::post(home,s,x,r.var())));
      break;
    case RM_IMP:
      GECODE_ES_FAIL((Set::Int::ReMinElement<Set::SetView,RM_IMP>
                     ::post(home,s,x,r.var())));
      break;
    case RM_PMI:
      GECODE_ES_FAIL((Set::Int::ReMinElement<Set::SetView,RM_PMI>
                     ::post(home,s,x,r.var())));
      break;
    default: throw Gecode::Int::UnknownReifyMode("Set::min");
    }
  }

  void
  max(Home home, SetVar s, IntVar x) {
    GECODE_POST;
    GECODE_ES_FAIL(Set::Int::MaxElement<Set::SetView>::post(home,s,x));
  }

  void
  notMax(Home home, SetVar s, IntVar x) {
    GECODE_POST;
    GECODE_ES_FAIL(Set::Int::NotMaxElement<Set::SetView>::post(home,s,x));
  }

  void
  max(Home home, SetVar s, IntVar x, Reify r) {
    GECODE_POST;
    switch (r.mode()) {
    case RM_EQV:
      GECODE_ES_FAIL((Set::Int::ReMaxElement<Set::SetView,RM_EQV>
                     ::post(home,s,x,r.var())));
      break;
    case RM_IMP:
      GECODE_ES_FAIL((Set::Int::ReMaxElement<Set::SetView,RM_IMP>
                     ::post(home,s,x,r.var())));
      break;
    case RM_PMI:
      GECODE_ES_FAIL((Set::Int::ReMaxElement<Set::SetView,RM_PMI>
                     ::post(home,s,x,r.var())));
      break;
    default: throw Gecode::Int::UnknownReifyMode("Set::max");
    }
  }

  void weights(Home home, IntSharedArray elements, IntSharedArray weights,
               SetVar x, IntVar y) {
    GECODE_POST;
    GECODE_ES_FAIL(Set::Int::Weights<Set::SetView>::post(home,elements,
                                                              weights,x,y));
  }

}

// STATISTICS: set-post
