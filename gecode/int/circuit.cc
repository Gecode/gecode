/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#include "gecode/int/circuit.hh"

namespace Gecode {

  void
  circuit(Space* home, const IntVarArgs& x, IntConLevel) {
    using namespace Int;
    if (x.same())
      throw ArgumentSame("Int::circuit");
    if (home->failed()) return;
    if (x.size() == 0)
      return;
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(home,Circuit::Simple<IntView>::post(home,xv));
  }

}

// STATISTICS: int-post
