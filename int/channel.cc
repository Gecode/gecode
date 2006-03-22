/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2005-10-27 21:01:30 +0200 (Thu, 27 Oct 2005) $ by $Author: schulte $
 *     $Revision: 2420 $
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

#include "int/channel.hh"

namespace Gecode {

  using namespace Int;

  void
  channel(Space* home, const IntVarArgs& x, const IntVarArgs& y,
	  IntConLevel) {
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::channel");
    if (home->failed()) return;

    ViewArray<IntView> xv(home,x);
    ViewArray<IntView> yv(home,y);
    if (Channel::Dom<IntView>::post(home,xv,yv) == ES_FAILED)
      home->fail();
  }

}

// STATISTICS: int-post
