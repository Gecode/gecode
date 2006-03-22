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
    using namespace Channel;
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::channel");
    if (x.same(y))
      throw ArgumentSame("Int::channel");
    if (home->failed()) return;
    if (x.size() == 0)
      return;

    ViewCardBnds<IntView>* xvc 
      = ViewCardBnds<IntView>::allocate(home,x.size());
    ViewCardBnds<IntView>* yvc 
      = ViewCardBnds<IntView>::allocate(home,x.size());
    for (int i=x.size(); i--; ) {
      xvc[i].view = x[i];
      yvc[i].view = y[i];
    }
    if (Dom<IntView>::post(home,x.size(),xvc,yvc) == ES_FAILED)
      home->fail();
  }

}

// STATISTICS: int-post
