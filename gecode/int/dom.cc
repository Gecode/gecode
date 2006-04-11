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


#include "gecode/int/dom.hh"

namespace Gecode {

  using namespace Int;

  void
  dom(Space* home, IntVar x, int min, int max, IntConLevel) {
    if (home->failed()) return;
    IntView xv(x);
    GECODE_ME_FAIL(home,xv.gq(home,min));
    GECODE_ME_FAIL(home,xv.lq(home,max));
  }

  void
  dom(Space* home, IntVarArgs& x, int min, int max, IntConLevel) {
    if (home->failed()) return;
    for (int i=x.size(); i--; ) {
      IntView xv(x[i]);
      GECODE_ME_FAIL(home,xv.gq(home,min));
      GECODE_ME_FAIL(home,xv.lq(home,max));
    }
  }

  void
  dom(Space* home, IntVar x, const IntSet& is, IntConLevel) {
    if (home->failed()) return;
    IntView xv(x);
    IntSetRanges ris(is);
    GECODE_ME_FAIL(home,xv.inter(home,ris));
  }

  void
  dom(Space* home, IntVarArgs& x, const IntSet& is, IntConLevel) {
    if (home->failed()) return;
    for (int i = x.size(); i--; ) {
      IntSetRanges ris(is);
      IntView xv(x[i]);
      GECODE_ME_FAIL(home,xv.inter(home,ris));
    }
  }

  void
  dom(Space* home, IntVar x, int min, int max, BoolVar b, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Dom::ReRange<IntView>::post(home,x,min,max,b));
  }


  void
  dom(Space* home, IntVar x, const IntSet& is, BoolVar b, IntConLevel) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Dom::ReIntSet<IntView>::post(home,x,is,b));
  }

}

// STATISTICS: int-post

