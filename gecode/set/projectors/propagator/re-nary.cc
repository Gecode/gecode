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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set/projectors/propagator.hh"

namespace Gecode { namespace Set { namespace Projection {

  size_t
  ReNaryProjection::dispose(Space* home) {
    unforce(home);
    if (!home->failed()) {
      x.cancel(home,this,PC_SET_ANY);
      b.cancel(home,this,Gecode::Int::PC_INT_VAL);
    }
    ps.~ProjectorSet();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  ExecStatus
  ReNaryProjection::post(Space* home, ViewArray<SetView>& x,
                         Gecode::Int::BoolView b, ProjectorSet& ps) {
    if (ps.arity() != x.size() - 1)
      throw Set::InvalidProjector("");
    (void) new (home) ReNaryProjection(home,x,b,ps);
    return ES_OK;
  }

  Actor*
  ReNaryProjection::copy(Space* home, bool share) {
    return new (home) ReNaryProjection(home,share,*this);
  }

  ExecStatus
  ReNaryProjection::propagate(Space* home) {
    if (b.one())
      GECODE_REWRITE((NaryProjection<false>::post(home,x,ps)));
    if (b.zero())
      GECODE_REWRITE((NaryProjection<true>::post(home,x,ps)));

    switch (ps.check(home, x)) {
    case ES_SUBSUMED:
      b.one_none(home);
      return ES_SUBSUMED;
    case ES_FAILED:
      b.zero_none(home);
      return ES_SUBSUMED;
    default:
      return ES_FIX;
    }
  }

}}}

// STATISTICS: set-prop
