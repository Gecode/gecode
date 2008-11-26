/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

namespace Gecode { namespace Int { namespace Dom {

  template <class View>
  forceinline
  ReRange<View>::ReRange(Space& home, View x, int min0, int max0, BoolView b)
    : ReUnaryPropagator<View,PC_INT_BND,BoolView>(home,x,b),
      min(min0), max(max0) {}

  template <class View>
  ExecStatus
  ReRange<View>::post(Space& home, View x, int min, int max, BoolView b) {
    if (min > max) {
      return ES_FAILED;
    } else if (min == max) {
      return Rel::ReEqDomInt<View,BoolView>::post(home,x,min,b);
    } else if ((max < x.min()) || (min > x.max())) {
      GECODE_ME_CHECK(b.zero(home));
    } else if ((min <= x.min()) && (x.max() <= max)) {
      GECODE_ME_CHECK(b.one(home));
    } else {
      (void) new (home) ReRange<View>(home,x,min,max,b);
    }
    return ES_OK;
  }


  template <class View>
  forceinline
  ReRange<View>::ReRange(Space& home, bool share, ReRange& p)
    : ReUnaryPropagator<View,PC_INT_BND,BoolView>(home,share,p),
      min(p.min), max(p.max) {}

  template <class View>
  Actor*
  ReRange<View>::copy(Space& home, bool share) {
    return new (home) ReRange<View>(home,share,*this);
  }

  template <class View>
  inline Support::Symbol
  ReRange<View>::ati(void) {
    return Reflection::mangle<View>("Gecode::Int::Dom::ReRange");
  }

  template <class View>
  Reflection::ActorSpec
  ReRange<View>::spec(const Space& home, Reflection::VarMap& m) const {
    Reflection::ActorSpec s =
      ReUnaryPropagator<View,PC_INT_BND,BoolView>::spec(home, m, ati());
    return s << min << max;
  }

  template <class View>
  void
  ReRange<View>::post(Space& home, Reflection::VarMap& vars,
                      const Reflection::ActorSpec& spec) {
    spec.checkArity(4);
    View x0(home, vars, spec[0]);
    BoolView b(home, vars, spec[1]);
    int min = spec[2]->toInt();
    int max = spec[3]->toInt();
    (void) new (home) ReRange<View>(home, x0, min, max, b);
  }

  template <class View>
  ExecStatus
  ReRange<View>::propagate(Space& home, const ModEventDelta&) {
    if (b.one()) {
      GECODE_ME_CHECK(x0.gq(home,min));
      GECODE_ME_CHECK(x0.lq(home,max));
      goto subsumed;
    }
    if (b.zero()) {
      Iter::Ranges::Singleton r(min,max);
      GECODE_ME_CHECK(x0.minus_r(home,r,false));
      goto subsumed;
    }
    if ((x0.max() <= max) && (x0.min() >= min)) {
      GECODE_ME_CHECK(b.one_none(home));  goto subsumed;
    }
    if ((x0.max() < min) || (x0.min() > max)) {
      GECODE_ME_CHECK(b.zero_none(home)); goto subsumed;
    }
    return ES_FIX;
  subsumed:
    x0.cancel(home,*this,PC_INT_BND);
    return ES_SUBSUMED(*this,sizeof(*this));
  }


}}}

// STATISTICS: int-prop

