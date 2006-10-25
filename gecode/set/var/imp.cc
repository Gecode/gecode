/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set.hh"

#include "gecode/set/var/imp-body.icc"

namespace Gecode { namespace Set {

  /*
   * "Standard" tell operations
   *
   */
  ModEvent
  SetVarImp::cardMin_full(Space* home,unsigned int newMin) {
    ModEvent me = ME_SET_CARD;
    if (_cardMin == _cardMax)
      me = checkLubCardAssigned(home,ME_SET_CARD);
    if (!me_failed(me))
      notify(home, me);
    return me;
  }

  ModEvent
  SetVarImp::cardMax_full(Space* home,unsigned int newMax) {
    ModEvent me = ME_SET_CARD;
    if (_cardMin == _cardMax)
      me = checkGlbCardAssigned(home,ME_SET_CARD);
    if (!me_failed(me))
      notify(home, me);
    return me;
  }

  ModEvent
  SetVarImp::processLubChange(Space* home) {
    if (!boundsConsistent()) {
      return ME_SET_FAILED;
    }
    ModEvent me;
    if (_cardMax > lub.size()) {
      _cardMax = lub.size();
      if (cardMin() > cardMax())
        return ME_SET_FAILED;
      if (assigned()) {
        lub.linkTo(home, glb);
        me = ME_SET_VAL;
      } else {
        me = checkLubCardAssigned(home, ME_SET_CLUB);
      }
    } else {
      me = checkLubCardAssigned(home, ME_SET_LUB);
    }
    if (!me_failed(me))
      notify(home, me);
    return me;
  }

  ModEvent
  SetVarImp::processGlbChange(Space* home) {
    if (!boundsConsistent())
      return ME_SET_FAILED;
    ModEvent me;
    if (_cardMin < glb.size()) {
      _cardMin = glb.size();
      if (cardMin() > cardMax())
        return ME_SET_FAILED;
      if (assigned()) {
        lub.linkTo(home, glb);
        me = ME_SET_VAL;
      } else {
        me = checkGlbCardAssigned(home, ME_SET_CGLB);
      }
    } else {
      me = checkGlbCardAssigned(home, ME_SET_GLB);
    }
    if (me!=ME_SET_FAILED)
      notify(home, me);
    return me;
  }

  /*
   * Copying variables
   *
   */

  forceinline
  SetVarImp::SetVarImp(Space* home, bool share, SetVarImp& x)
    : SetVarImpBase(home,share,x),
      _cardMin(x._cardMin), _cardMax(x._cardMax) {
    lub.update(home, x.lub);
    if (x.assigned()) {
      glb.linkTo(home,lub);
    } else {
      glb.update(home,x.glb);
    }
  }


  SetVarImp*
  SetVarImp::perform_copy(Space* home, bool share) {
    return new (home) SetVarImp(home,share,*this);
  }

}}

// STATISTICS: set-var

