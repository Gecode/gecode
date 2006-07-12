/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#include "gecode/int/channel.hh"

namespace Gecode {

  using namespace Int;

  void
  channel(Space* home, const IntVarArgs& x, const IntVarArgs& y,
	  IntConLevel icl) {
    using namespace Channel;
    int n = x.size();
    if (n != y.size())
      throw ArgumentSizeMismatch("Int::channel");
    if (x.same(y))
      throw ArgumentSame("Int::channel");
    if (home->failed()) return;
    if (n == 0)
      return;

    if (icl == ICL_VAL) {
      ValInfo<IntView>* vi
	= ValInfo<IntView>::allocate(home,2*n);
      for (int i=n; i--; ) {
	vi[i  ].init(x[i],n); 
	vi[i+n].init(y[i],n);
      }
      if (Val<IntView>::post(home,n,vi) == ES_FAILED)
	home->fail();
    } else {
      DomInfo<IntView>* di
	= DomInfo<IntView>::allocate(home,2*n);
      for (int i=n; i--; ) {
	di[i  ].init(x[i],n); 
	di[i+n].init(y[i],n);
      }
      if (Dom<IntView>::post(home,n,di) == ES_FAILED)
	home->fail();
    }
  }

}

// STATISTICS: int-post
