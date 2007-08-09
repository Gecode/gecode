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

using namespace Gecode::CpltSet;

namespace Gecode {

  void
  disjointglb(Space* home, const CpltSetVarArgs& x, int index) {
    if (home->failed()) return;

    int n = x.size();
    ViewArray<CpltSetView> bv(home, n);
    for (int i = n; i--; )
      bv[i] = x[i];
    GECODE_ES_FAIL(home, DisjointGlb<CpltSetView>::post(home, bv, index));

  }

  void
  disjointsudoku(Space* home, CpltSetVar x, int order) {
    if (home->failed()) return;
    ViewArray<CpltSetView> bv(home, 1);
    bv[0] = x;
    GECODE_ES_FAIL(home, DisjointSudoku<CpltSetView>::post(home, bv[0], order));

  }

}

// STATISTICS: bdd-post
