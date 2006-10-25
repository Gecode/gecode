/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */



#include "gecode/set/int.hh"

#include "gecode/iter.hh"

#include "gecode/set/rel.hh"

namespace Gecode { namespace Set { namespace Int {

  Actor*
  MinElement::copy(Space* home, bool share) {
    return new (home) MinElement(home,share,*this);
  }

  ExecStatus
  MinElement::propagate(Space* home) {
    //x1 is an element of x0.ub
    //x1 =< smallest element of x0.lb
    //x1 =< x0.cardinialityMin-est largest element of x0.ub
    //(these 2 take care of determined x0)
    //No element in x0 is smaller than x1
    //if x1 is determined, it is part of the ub.

    //Consequently:
    //The domain of x1 is a subset of x0.ub up to the first element in x0.lb.
    //x0 lacks everything smaller than smallest possible x1.

    LubRanges<SetView> ub(x0);
    GECODE_ME_CHECK(x1.inter(home,ub));
    GECODE_ME_CHECK(x1.lq(home,x0.glbMin()));
    //if cardMin>lbSize?
    assert(x0.cardMin()>=1);
    GECODE_ME_CHECK(x1.lq(home,x0.lubMaxN(x0.cardMin()-1)));
    GECODE_ME_CHECK( x0.exclude(home,
                                Limits::Set::int_min, x1.min()-1) );

    if (x1.assigned()) {
      GECODE_ME_CHECK(x0.include(home,x1.val()));
      GECODE_ME_CHECK(x0.exclude(home,
                                 Limits::Set::int_min, x1.val()-1));
      return ES_SUBSUMED;
    }

    return ES_FIX;
  }

  ExecStatus MaxElement::post(Space* home, SetView x0,
                              Gecode::Int::IntView x1) {
    GECODE_ME_CHECK(x0.cardMin(home,1));
    (void) new (home) MaxElement(home,x0,x1);
    return ES_OK;
  }

  Actor*
  MaxElement::copy(Space* home, bool share) {
    return new (home) MaxElement(home,share,*this);
  }

  ExecStatus
  MaxElement::propagate(Space* home) {
    LubRanges<SetView> ub(x0);
    GECODE_ME_CHECK(x1.inter(home,ub));
    GECODE_ME_CHECK(x1.gq(home,x0.glbMax()));
    assert(x0.cardMin()>=1);
    GECODE_ME_CHECK(x1.gq(home,x0.lubMinN(x0.cardMin()-1)));
    GECODE_ME_CHECK(x0.exclude(home,
                               x1.max()+1,Limits::Set::int_max) );

    if (x1.assigned()) {
      GECODE_ME_CHECK(x0.include(home,x1.val()));
      GECODE_ME_CHECK( x0.exclude(home,
                                  x1.val()+1,Limits::Set::int_max) );
      return ES_SUBSUMED;
    }

    return ES_FIX;
  }

}}}

// STATISTICS: set-prop
