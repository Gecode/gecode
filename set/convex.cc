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

#include "set/convex.hh"

using namespace Gecode::Set;

namespace Gecode {
  void
  convex(Space* home, SetVar x) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Convex::Convex::post(home, x));
  }

  void
  convexHull(Space* home, SetVar x, SetVar y) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Convex::ConvexHull::post(home, x,y));
  }

}

// STATISTICS: set-post
