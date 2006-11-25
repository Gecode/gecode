/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#include "gecode/int/link.hh"

namespace Gecode { namespace Int { namespace Link {

  forceinline
  Link::Link(Space* home, IntView x0, BoolView x1)
    : MixBinaryPropagator<IntView,PC_INT_VAL,BoolView,PC_INT_VAL>
  (home,x0,x1) {}

  forceinline ExecStatus
  Link::post(Space* home, IntView x0, BoolView x1) {
    if (x0.assigned()) {
      if (x0.val() == 0) {
        GECODE_ME_CHECK(x1.zero(home));
      } else if (x0.val() == 1) {
        GECODE_ME_CHECK(x1.one(home));
      } else {
        return ES_FAILED;
      }
    } else if (x1.zero()) {
      GECODE_ME_CHECK(x0.eq(home,0));
    } else if (x1.one()) {
      GECODE_ME_CHECK(x0.eq(home,1));
    } else {
      GECODE_ME_CHECK(x0.gq(home,0));
      GECODE_ME_CHECK(x0.lq(home,1));
      (void) new (home) Link(home,x0,x1);
    }
    return ES_OK;
  }


  forceinline
  Link::Link(Space* home, bool share, Link& p)
    : MixBinaryPropagator<IntView,PC_INT_VAL,BoolView,PC_INT_VAL>
  (home,share,p) {}

  Actor*
  Link::copy(Space* home, bool share) {
    return new (home) Link(home,share,*this);
  }

  PropCost
  Link::cost(void) const {
    return PC_UNARY_LO;
  }

  ExecStatus
  Link::propagate(Space* home) {
    if (x1.zero()) {
      GECODE_ME_CHECK(x0.eq(home,0));
    } else if (x1.one()) {
      GECODE_ME_CHECK(x0.eq(home,1));
    } else {
      assert(x0.assigned());
      if (x0.val() == 0) {
        GECODE_ME_CHECK(x1.zero(home));
      } else {
        assert(x0.val() == 1);
        GECODE_ME_CHECK(x1.one(home));
      }
    }
    return ES_SUBSUMED(this,sizeof(*this));
  }

}}}

namespace Gecode {

  void
  link(Space* home, IntVar x0, BoolVar x1, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Int::Link::Link::post(home,x0,x1));
  }

}

// STATISTICS: int-post

