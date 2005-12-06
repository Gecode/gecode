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

#include "set.hh"
#include "set/sequence.hh"

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
    /*
CHECK THIS
    bool ubevent = GECODE_VME2ME(VTI_SET, varmodevent()) &
      (ME_SET_LUB | ME_SET_BB | ME_SET_CLUB | ME_SET_CBB | ME_SET_VAL);
    bool lbevent = GECODE_VME2ME(VTI_SET, varmodevent()) &
      (ME_SET_GLB | ME_SET_BB | ME_SET_CGLB | ME_SET_CBB | ME_SET_VAL);
    bool anybevent = GECODE_VME2ME(VTI_SET, varmodevent()) &
      (ME_SET_LUB | ME_SET_GLB | ME_SET_BB |
       ME_SET_CLUB | ME_SET_CGLB | ME_SET_CBB | ME_SET_VAL);
    bool cardevent = GECODE_VME2ME(VTI_SET, varmodevent()) &
      (ME_SET_CARD | ME_SET_CLUB | ME_SET_CGLB | ME_SET_CBB);
    */

    bool ubevent = true;
    bool lbevent = true;
    bool anybevent = true;
    bool cardevent = true;


    bool modified=false;
    bool oldModified;
    do {
      oldModified = modified;
      modified = false;
      if (oldModified || lbevent)
        GECODE_ME_CHECK(propagateSeq(home,modified,x));
      if (oldModified || lbevent)
        GECODE_ME_CHECK(propagateSeqUnion(home,modified,x,y));
      if (oldModified || modified || ubevent)
        GECODE_ME_CHECK(RelOp::unionNXiUB(home,modified,x,y,unionOfDets));
      if (oldModified || modified || ubevent)
        GECODE_ME_CHECK(RelOp::partitionNYUB(home,modified,x,y,unionOfDets));
      if (oldModified || modified || anybevent)
        GECODE_ME_CHECK(RelOp::partitionNXiLB(home,modified,x,y,unionOfDets));
      if (oldModified || modified || cardevent)
        GECODE_ME_CHECK(RelOp::partitionNCard(home,modified,x,y,unionOfDets));
    } while (modified);

    //TODO: We might be able to detect VAL events and disperse this
    //functionality:
    //removing ground sets from x, accumulating the value:
    int xsize = x.size();
    int j=0;
    for (int i=0;i<xsize;i++){
      x[j] = x[i];
      if (x[j].assigned()) {
        GlbRanges<SetView> det(x[j]);
        unionOfDets.includeI(home,det);
        BndSetRanges dets(unionOfDets);
        GECODE_ME_CHECK( y.includeI(home,dets) );
      } else {
        j++;
      }
    }
    x.size(j);

    // When we run out of variables, make a final check and disolve:
    if (x.size()==0) {
      BndSetRanges all1(unionOfDets);
      GECODE_ME_CHECK( y.intersectI(home,all1) );
      BndSetRanges all2(unionOfDets);
      GECODE_ME_CHECK( y.includeI(home,all2) );
      unionOfDets.dispose(home);
      return ES_SUBSUMED;
    }

    return ES_FIX;
  }

}}}

// STATISTICS: set-prop
