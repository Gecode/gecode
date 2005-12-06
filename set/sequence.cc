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

#include "set/sequence.hh"

using namespace Gecode::Set;

namespace Gecode {

  void
  sequence(Space* home, const SetVarArgs& xa) {
    if (home->failed()) return;
    if (xa.size()==0)
      throw ArgumentEmpty("Set::seq");
    ViewArray<SetView> x(home,xa);
    GECODE_ES_FAIL(home,Sequence::Seq::post(home, x));
  }

  void
  sequentialUnion(Space* home, const SetVarArgs& xa, SetVar y) {
    if (home->failed()) return;
    ViewArray<SetView> x(home,xa);
    GECODE_ES_FAIL(home,Sequence::SeqU::post(home, x,y));
  }

}

// STATISTICS: set-post
