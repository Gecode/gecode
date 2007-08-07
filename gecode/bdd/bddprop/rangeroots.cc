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

#include "gecode/bdd.hh"
#include "gecode/bdd/bddprop.hh"

using namespace Gecode::Bdd;

namespace Gecode {

  void range(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
	     SetConLevel scl) {
    range_con(home, x, s, t, scl);
  }

  void roots(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
	     const BddVarArgs& allvars, 
	     SetConLevel scl) {
    roots_con(home, x, s, t, allvars, scl);
  }

  // constraints using the range constraint
  void alldifferent(Space* home, const IntVarArgs& x, BddVar s, 
		    BddVar t, const BddVarArgs& allvars, SetConLevel scl) {
    nvalue_con(home, x, s, t, x.size(), allvars, scl);
  }

  void nvalue(Space* home, const IntVarArgs& x, BddVar s, 
		    BddVar t, int n, const BddVarArgs& allvars, SetConLevel scl) {
    nvalue_con(home, x, s, t, n, allvars, scl);
  }

  void uses(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
	    const IntVarArgs& y, BddVar u, BddVar v,
	    SetConLevel scl) {
    uses_con(home, x, s, t, y, u, v, scl);
  }


}

// STATISTICS: bdd-post
