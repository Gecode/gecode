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

#include <gecode/set.hh>
#include <gecode/int.hh>

namespace Gecode { namespace Set { namespace Int {

  template <class View>
  forceinline
  Card<View>::Card(Space& home, View y0, Gecode::Int::IntView y1)
    : IntSetPropagator<View,PC_SET_CARD,
                       Gecode::Int::PC_INT_BND> (home, y0, y1) {}

  template <class View>
  forceinline ExecStatus
  Card<View>::post(Space& home, View x0, Gecode::Int::IntView x1) {
    GECODE_ME_CHECK(x1.gq(home,0));
    GECODE_ME_CHECK(x0.cardMax(home, Gecode::Int::Limits::max));
    (void) new (home) Card(home,x0,x1);
    return ES_OK;
  }

  template <class View>
  forceinline
  Card<View>::Card(Space& home, bool share, Card& p)
    : IntSetPropagator<View,PC_SET_CARD,
                       Gecode::Int::PC_INT_BND> (home, share, p) {}

  template <class View>
  Actor*
  Card<View>::copy(Space& home, bool share) {
   return new (home) Card(home,share,*this);
  }

  template <class View>
  Support::Symbol
  Card<View>::ati(void) {
   return Reflection::mangle<View>("Gecode::Set::Int::Card");
  }

  template <class View>
  Reflection::ActorSpec
  Card<View>::spec(const Space& home, Reflection::VarMap& m) const {
   return IntSetPropagator<View,PC_SET_CARD,Gecode::Int::PC_INT_BND>
     ::spec(home, m, ati());
  }

  template <class View>
  ExecStatus
  Card<View>::propagate(Space& home, const ModEventDelta&) {
   int x1min, x1max;
   do {
     x1min = x1.min();
     x1max = x1.max();
     GECODE_ME_CHECK(x0.cardMin(home,x1min));
     GECODE_ME_CHECK(x0.cardMax(home,x1max));
     GECODE_ME_CHECK(x1.gq(home,(int)x0.cardMin()));
     GECODE_ME_CHECK(x1.lq(home,(int)x0.cardMax()));
   } while (x1.min() > x1min || x1.max() < x1max);
   if (x1.assigned())
     return ES_SUBSUMED(*this,home);
   return ES_FIX;
  }

  template <class View>
  void
  Card<View>::post(Space& home, Reflection::VarMap& vars,
                   const Reflection::ActorSpec& spec) {
    spec.checkArity(2);
    View x0(home, vars, spec[0]);
    Gecode::Int::IntView x1(home, vars, spec[1]);
    (void) new (home) Card<View>(home,x0,x1);
  }

}}}

// STATISTICS: set-prop
