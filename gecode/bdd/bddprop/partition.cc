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

  void partition(Space* home, const BddVarArgs& x, SetConLevel scl) {
    partition_con(home, x, false, SRT_EQ, false, -1, scl);
  }

  void partition(Space* home, const BddVarArgs& x, const BddVar& y, 
		 SetConLevel scl) {
    partition_con(home, x, y, false, SRT_EQ, false, -1, scl);
  }

  void partition(Space* home, const IntVarArgs& x, const BddVar& y, 
		 SetConLevel scl) {
    partition_con(home, x, y, false, SRT_EQ, false, -1, scl);
  }

  void partitionLex(Space* home, const BddVarArgs& x, BddSetRelType lex, 
		    SetConLevel scl) {
    partition_con(home, x, true, lex, false, -1, scl);
  }

  void partitionLexCard(Space* home, const BddVarArgs& x, BddSetRelType lex, 
			int c, SetConLevel scl) {
    partition_con(home, x, true, lex, true, c, scl);
  }

  void partitionCard(Space* home, const BddVarArgs& x, int c, 
		     SetConLevel scl) {
    partition_con(home, x, false, SRT_EQ, true, c, scl);
  }
}

// STATISTICS: bdd-post
