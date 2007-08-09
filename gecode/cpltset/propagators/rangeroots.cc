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

  void range(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
             SetConLevel scl) {
    range_con(home, x, s, t, scl);
  }

  void roots(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
             const CpltSetVarArgs& allvars, 
             SetConLevel scl) {
    roots_con(home, x, s, t, allvars, scl);
  }

  // constraints using the range constraint
  void alldifferent(Space* home, const IntVarArgs& x, CpltSetVar s, 
                    CpltSetVar t, const CpltSetVarArgs& allvars, SetConLevel scl) {
    nvalue_con(home, x, s, t, x.size(), allvars, scl);
  }

  void nvalue(Space* home, const IntVarArgs& x, CpltSetVar s, 
                    CpltSetVar t, int n, const CpltSetVarArgs& allvars, SetConLevel scl) {
    nvalue_con(home, x, s, t, n, allvars, scl);
  }

  void uses(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
            const IntVarArgs& y, CpltSetVar u, CpltSetVar v,
            SetConLevel scl) {
    uses_con(home, x, s, t, y, u, v, scl);
  }


}

// STATISTICS: bdd-post
