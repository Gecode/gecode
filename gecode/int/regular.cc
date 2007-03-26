/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#include "gecode/int/regular.hh"

namespace Gecode {

  using namespace Int;

  void
  regular(Space* home, const IntVarArgs& x, DFA& dfa, IntConLevel) {
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(home,(Regular::Dom<IntView,true>::post(home,xv,dfa)));
  }

  void
  regular(Space* home, const BoolVarArgs& x, DFA& dfa, IntConLevel) {
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(home,(Regular::Dom<BoolView,true>::post(home,xv,dfa)));
  }

}



// STATISTICS: int-post

