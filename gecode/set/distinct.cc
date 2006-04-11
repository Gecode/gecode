/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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

#include "gecode/set/distinct.hh"

using namespace Gecode::Set;

namespace Gecode {

  void
  atmostOne(Space* home, const SetVarArgs& xa, unsigned int c) {
    if (home->failed()) return;
    if (xa.size() < 2)
      return;
    ViewArray<SetView> x(home,xa);
    GECODE_ES_FAIL(home,Distinct::AtmostOne::post(home, x, c));
  }

  void
  distinct(Space* home, const SetVarArgs& xa, unsigned int c) {
    if (home->failed()) return;
    if (xa.size() < 2)
      return;
    ViewArray<SetView> x(home,xa);
    GECODE_ES_FAIL(home,Distinct::Distinct::post(home, x, c));
  }

}

// STATISTICS: set-post
