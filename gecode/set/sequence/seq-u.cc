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

  ExecStatus
  SeqU::propagateSeqUnion(Space* home,
                          bool& modified, ViewArray<SetView>& x,
                          SetView& y) {
    GECODE_AUTOARRAY(GlbRanges<SetView>, XLBs,x.size());
    for (int i=x.size(); i--; ){
      GlbRanges<SetView> lb(x[i]);
      XLBs[i]=lb;
    }
    Iter::Ranges::NaryAppend<GlbRanges<SetView> > u(XLBs,x.size());
    GECODE_ME_CHECK_MODIFIED(modified, y.includeI(home,u));

    GLBndSet before(home);
    for (int i=0; i<x.size(); i++) {
      LubRanges<SetView> xiub(x[i]);
      before.includeI(home, xiub);
      BndSetRanges beforeR(before);
      GlbRanges<SetView> ylb(y);
      Iter::Ranges::Diff<GlbRanges<SetView>, BndSetRanges> diff(ylb, beforeR);
      if (diff()) {
        GECODE_ME_CHECK_MODIFIED(modified, x[i].exclude(home, diff.min(),
                                                        Limits::Set::int_max));
      }
    }
    before.dispose(home);

    GLBndSet after(home);
    for (int i=x.size(); i--; ) {
      LubRanges<SetView> xiub(x[i]);
      after.includeI(home, xiub);
      BndSetRanges afterR(after);
      GlbRanges<SetView> ylb(y);
      Iter::Ranges::Diff<GlbRanges<SetView>, BndSetRanges> diff(ylb, afterR);
      if (diff()) {
        int max = diff.max();
        for (; diff(); ++diff)
          max = diff.max();
        GECODE_ME_CHECK_MODIFIED(modified, x[i].exclude(home,
                                                        Limits::Set::int_min,
                                                        max));
      }
    }    
    after.dispose(home);
    
    return ES_FIX;
  }

  //Enforces sequentiality and ensures y contains union of Xi lower bounds.
  ExecStatus
  SeqU::propagate(Space* home) {
    ModEvent me0 = SetView::pme(this);
    bool ubevent = Rel::testSetEventUB(me0);
    bool anybevent = Rel::testSetEventAnyB(me0);
    bool cardevent = Rel::testSetEventCard(me0);

    bool modified = false;
    bool assigned=false;
    bool oldModified = false;

    do {
      oldModified = modified;
      modified = false;

      if (oldModified || modified || anybevent || cardevent)
        GECODE_ME_CHECK(propagateSeq(home,modified,assigned,x));
      if (oldModified || modified || anybevent)
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
    return ES_SUBSUMED(this,home);
  }

}}}

// STATISTICS: set-prop
