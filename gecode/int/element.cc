/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
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

#include "gecode/int/element.hh"

namespace Gecode {

  using namespace Int;

  void
  element(Space* home, const IntArgs& c, IntVar x0, IntVar x1,
	  IntConLevel) {
    if (home->failed()) return;
    Element::IntSharedArray cs(c.size());
    for (int i = c.size(); i--; )
      if ((c[i] < Limits::Int::int_min) || (c[i] > Limits::Int::int_max))
	throw NumericalOverflow("Int::element");
      else
	cs[i] = c[i];
    if (Element::Int<IntView,IntView>::post(home,cs,x0,x1) == ES_FAILED)
      home->fail();
  }

  void
  element(Space* home, const IntVarArgs& c, IntVar x0, IntVar x1,
	  IntConLevel icl) {
    if (home->failed()) return;
    Element::IdxView<IntView>* iv = Element::IdxView<IntView>::init(home,c);
    if (icl == ICL_BND) {
      if (Element::ViewBnd<IntView,IntView>::post(home,iv,c.size(),x0,x1) 
	  == ES_FAILED)
	home->fail();
    } else {
      if (Element::ViewDom<IntView,IntView>::post(home,iv,c.size(),x0,x1) 
	  == ES_FAILED)
	home->fail();
    }
  }

}


// STATISTICS: int-post

