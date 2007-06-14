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

using namespace Gecode::Bdd;

namespace Gecode {

  void selectUnion(Space* home, const BddVarArgs& x, BddVar s, BddVar t, 
		   SetConLevel scl) {
    selectUnion_con(home, x, s, t, scl);
  }

//   void range(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
// 	     SetConLevel scl) {
//     ViewArray<IntView> iv(home, x.size());
//     ViewArray<SingletonBddView> sbv(home, iv.size());
//     for (int i = sbv.size(); i--; ) {
//       sbv[i].init(iv[i]);
//       GECODE_ME_FAIL(home, sbv[i].cardinality(home, 1));
//     }
//     BddView selview(s);
//     BddView unionview(t);
    
//     selectUnion_post(home, sbv, selview, unionview, scl);
//   }

  void findNonEmptySub(Space* home, const BddVarArgs& x, BddVar s, BddVar t, 
		       SetConLevel scl) {
    findNonEmptySub_con(home, x, s, t, scl);
  }

}

// STATISTICS: bdd-post
