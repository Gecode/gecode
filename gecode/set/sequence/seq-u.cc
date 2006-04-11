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

#include "gecode/set.hh"
#include "gecode/set/sequence.hh"

namespace Gecode { namespace Set { namespace Sequence {

  /*
   * "Sequenced union" propagator
   *
   */

  Actor*
  SeqU::copy(Space* home, bool share) {
    return new (home) SeqU(home,share,*this);
  }

  //Enforces sequentiality and ensures y contains union of Xi lower bounds.
  ExecStatus
  SeqU::propagate(Space* home) {
    ModEvent me0 = SetView::pme(this);
    bool ubevent = Rel::testSetEventUB(me0);
    bool lbevent = Rel::testSetEventLB(me0);
    bool anybevent = Rel::testSetEventAnyB(me0);
    bool cardevent = Rel::testSetEventCard(me0);

    bool modified = false;
    bool assigned=false;
    bool oldModified = false;

    do {
      oldModified = modified;
      modified = false;

      if (oldModified || modified || lbevent)
        GECODE_ME_CHECK(propagateSeq(home,modified,assigned,x));
      if (oldModified || modified || lbevent)
        GECODE_ME_CHECK(propagateSeqUnion(home,modified,x,y));
      if (oldModified || modified || ubevent)
        GECODE_ME_CHECK(RelOp::unionNXiUB(home,modified,x,y,unionOfDets));
      if (oldModified || modified || ubevent)
        GECODE_ME_CHECK(RelOp::partitionNYUB(home,modified,x,y,unionOfDets));
      if (oldModified || modified || anybevent)
        GECODE_ME_CHECK(RelOp::partitionNXiLB(home,modified,x,y,unionOfDets));
      if (oldModified || modified || cardevent || ubevent)
        GECODE_ME_CHECK(RelOp::partitionNCard(home,modified,x,y,unionOfDets));

    } while (modified);

    for (int i=x.size(); i--;)
      if (!x[i].assigned())
	return ES_FIX;
    return ES_SUBSUMED;
  }

}}}

// STATISTICS: set-prop
