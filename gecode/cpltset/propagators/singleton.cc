/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset.hh"
#include "gecode/cpltset/propagators.hh"

using namespace Gecode::Bdd;

namespace Gecode {

  void singleton(Space* home, IntVar x, CpltSetVar s, SetConLevel scl) {
    Int::IntView iv(x);
    BddView bv(s);
    GECODE_ES_FAIL(home, (Bdd::Singleton<Int::IntView, BddView>
			  ::post(home, iv, bv)));
  }


}

// STATISTICS: bdd-post
