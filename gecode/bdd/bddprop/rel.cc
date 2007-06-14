/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
 *
 *  Last modified:
 *     $Date: 2006-07-29 15:55:20 +0000 (Sat, 29 Jul 2006) $ by $Author: pekczynski $
 *     $Revision: 3478 $
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

// for set bounds propagators with bdds
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"
#include "gecode/set/int.hh"
#include "gecode/set/convex.hh"
#include "gecode/set/sequence.hh"
#include "gecode/set/distinct.hh"
#include "gecode/set/select.hh"

using namespace Gecode::Bdd;

namespace Gecode {

  void rel(Space* home, BddVar x, SetRelType r, BddVar y, SetConLevel scl) {
    switch (scl) {
    case SCL_BND_SBR:
      {
	std::cout << "rel set\n";
	BddView bx(x);
	BddView by(y);
	CrdBddView cx(bx);
	CrdBddView cy(by);
	SetBddView sx(cx);
	SetBddView sy(cy);
	rel_post<SetBddView, SetBddView>(home, sx, r, sy, scl); 
	break;
      }
    default:
      rel_con(home, x, r, y, scl);      
    }
  }

  void rel(Space* home, IntVar x, SetRelType r, BddVar y, SetConLevel scl) {
//     switch (scl) {
//     case SCL_BND_SBR:
//       {
// 	switch(r) {
// 	case SRT_SUB:
// 	  rel(home, y, SRT_SUP, x, scl);
// 	  break;
// 	case SRT_SUP:
// 	  rel(home, y, SRT_SUB, x, scl);
// 	  break;
// 	default:
// 	  rel(home, y, r, x, scl);
// 	}
// 	break;
//       }
//     default:
      rel_con(home, x, r, y, scl);
//     }
  }

  void rel(Space* home, BddVar x, BddSetRelType r, BddVar y, SetConLevel scl) {
    rel_con(home, x, r, y, scl);
  }

  void rel(Space* home, IntVar x, BddSetRelType r, BddVar y, SetConLevel scl) {
    rel_con(home, x, r, y, scl);
  }

  void rel(Space* home, BddVar x, BddSetOpType o, BddVar y, BddSetRelType r, 
	   BddVar z, SetConLevel scl) {
    rel_con_bdd(home, x, o, y, r, z, scl);
  }

  void rel(Space* home, BddVar x, BddSetOpType o, BddVar y, SetRelType r, 
	   BddVar z, SetConLevel scl) {
    rel_con_bdd(home, x, o, y, r, z, scl);
  }

  void rel(Space* home, BddVar x, SetOpType o, BddVar y, BddSetRelType r, 
	   BddVar z, SetConLevel scl) {
    rel_con_bdd(home, x, o, y, r, z, scl);
  }

  void rel(Space* home, BddVar x, SetOpType o, BddVar y, SetRelType r, 
	   BddVar z, SetConLevel scl) {
    if (scl == SCL_BND_SBR) {
      rel_con_sbr(home, x, o, y, r, z, scl);
    } else {
      rel_con_bdd(home, x, o, y, r, z, scl);
    }
    
  }

}

// STATISTICS: bdd-post
