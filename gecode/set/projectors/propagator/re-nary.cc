/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#include "gecode/set/projectors/propagator.hh"

namespace Gecode { namespace Set { namespace Projection {

  size_t
  ReNaryProjection::dispose(Space& home) {
    unforce(home);
    if (!home.failed()) {
      x.cancel(home,*this,PC_SET_ANY);
      b.cancel(home,*this,Gecode::Int::PC_INT_VAL);
    }
    ps.~ProjectorSet();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  ExecStatus
  ReNaryProjection::post(Space& home, ViewArray<SetView>& x,
                         Gecode::Int::BoolView b, ProjectorSet& ps) {
    if (ps.arity() != x.size() - 1)
      throw Set::InvalidProjector("");
    (void) new (home) ReNaryProjection(home,x,b,ps);
    return ES_OK;
  }

  Actor*
  ReNaryProjection::copy(Space& home, bool share) {
    return new (home) ReNaryProjection(home,share,*this);
  }

  Support::Symbol
  ReNaryProjection::ati(void) {
    return Support::Symbol("set.projectors.ReNaryProjection");
  }

  Reflection::ActorSpec
  ReNaryProjection::spec(const Space&, Reflection::VarMap&) const {
    throw Reflection::ReflectionException("Not implemented");
  }

  ExecStatus
  ReNaryProjection::propagate(Space& home, ModEventDelta) {
    ProjectorSet newps(ps);
    if (b.one())
      GECODE_REWRITE(*this,(NaryProjection<false>::post(home,x,newps)));
    if (b.zero())
      GECODE_REWRITE(*this,(NaryProjection<true>::post(home,x,newps)));

    switch (ps.check(home, x)) {
    case __ES_SUBSUMED:
      GECODE_ME_CHECK(b.one_none(home));
      return ES_SUBSUMED(*this,home);
    case ES_FAILED:
      GECODE_ME_CHECK(b.zero_none(home));
      return ES_SUBSUMED(*this,home);
    default:
      return ES_FIX;
    }
  }

}}}

// STATISTICS: set-prop
