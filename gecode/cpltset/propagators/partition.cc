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

  void partition(Space* home, const CpltSetVarArgs& x, SetConLevel scl) {
    partition_con(home, x, false, SRT_EQ, false, -1, scl);
  }

  void partition(Space* home, const CpltSetVarArgs& x, const CpltSetVar& y, 
		 SetConLevel scl) {
    partition_con(home, x, y, false, SRT_EQ, false, -1, scl);
  }

  void partition(Space* home, const IntVarArgs& x, const CpltSetVar& y, 
		 SetConLevel scl) {
    partition_con(home, x, y, false, SRT_EQ, false, -1, scl);
  }

  void partitionLex(Space* home, const CpltSetVarArgs& x, BddSetRelType lex, 
		    SetConLevel scl) {
    partition_con(home, x, true, lex, false, -1, scl);
  }

  void partitionLexCard(Space* home, const CpltSetVarArgs& x, BddSetRelType lex, 
			int c, SetConLevel scl) {
    partition_con(home, x, true, lex, true, c, scl);
  }

  void partitionCard(Space* home, const CpltSetVarArgs& x, int c, 
		     SetConLevel scl) {
    partition_con(home, x, false, SRT_EQ, true, c, scl);
  }
}

// STATISTICS: bdd-post
