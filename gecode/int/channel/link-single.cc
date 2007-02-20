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

#include "gecode/int/channel.hh"

namespace Gecode { namespace Int { namespace Channel {

  forceinline
  LinkSingle::LinkSingle(Space* home, BoolView x0, IntView x1)
    : MixBinaryPropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_VAL>
  (home,x0,x1) {}

  ExecStatus
  LinkSingle::post(Space* home, BoolView x0, IntView x1) {
    if (x1.assigned()) {
      switch (x1.val()) {
      case 0:
        GECODE_ME_CHECK(x0.zero(home)); break;
      case 1:
        GECODE_ME_CHECK(x0.one(home)); break;
      default:
        return ES_FAILED;
      }
    } else if (x0.zero()) {
      GECODE_ME_CHECK(x1.eq(home,0));
    } else if (x0.one()) {
      GECODE_ME_CHECK(x1.eq(home,1));
    } else {
      GECODE_ME_CHECK(x1.gq(home,0));
      GECODE_ME_CHECK(x1.lq(home,1));
      (void) new (home) LinkSingle(home,x0,x1);
    }
    return ES_OK;
  }


  forceinline
  LinkSingle::LinkSingle(Space* home, bool share, LinkSingle& p)
    : MixBinaryPropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_VAL>
  (home,share,p) {}

  Actor*
  LinkSingle::copy(Space* home, bool share) {
    return new (home) LinkSingle(home,share,*this);
  }

  PropCost
  LinkSingle::cost(void) const {
    return PC_UNARY_LO;
  }

  ExecStatus
  LinkSingle::propagate(Space* home) {
    if (x0.zero()) {
      GECODE_ME_CHECK(x1.eq(home,0));
    } else if (x0.one()) {
      GECODE_ME_CHECK(x1.eq(home,1));
    } else {
      assert(x0.none() && x1.assigned());
      if (x1.val() == 0) {
        GECODE_ME_CHECK(x0.zero_none(home));
      } else {
        assert(x1.val() == 1);
        GECODE_ME_CHECK(x0.one_none(home));
      }
    }
    return ES_SUBSUMED(this,sizeof(*this));
  }

}}}

// STATISTICS: int-prop

