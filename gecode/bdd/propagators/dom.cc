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

namespace Gecode {
  
  void
  dom(Space* home, CpltSetVar x, SetRelType r, int i, SetConLevel) {
    IntSet d(i,i);
    dom(home, x, r, d);
  }

  void
  dom(Space* home, CpltSetVar x, SetRelType r, int i, int j, SetConLevel) {
    IntSet d(i,j);
    dom(home, x, r, d);
  }

  void 
  dom(Space* home, CpltSetVar x, SetRelType r, const IntSet& s, SetConLevel) {
    if (home->failed()) return;
    Bdd::BddView bv(x);

    switch(r) {
    case SRT_EQ:
      {
	if (s.size() == 1) {
	  GECODE_ME_FAIL(home, bv.eq(home, s.min(), s.max()));
	} else {
	  IntSetRanges ir(s);
	  GECODE_ME_FAIL(home,bv.eqI(home, ir));
	}
      }
      break;
    case SRT_DISJ:
      {
      	if (s.size() == 1) {
      	  GECODE_ME_FAIL(home,bv.exclude(home, s.min(), s.max()));
      	} else {
      	  IntSetRanges rd(s);
      	  GECODE_ME_FAIL(home,bv.excludeI(home, rd));
      	}
      }
      break;
    case SRT_NQ:
      {
	if (s.size() == 1) {
	  GECODE_ME_FAIL(home, bv.nq(home, s.min(), s.max()));
	} else {
	  IntSetRanges ir(s);
	  GECODE_ME_FAIL(home, bv.nqI(home, ir));
	}
      }
      break;
    case SRT_SUB:
      {
 	if (s.size() == 1) {
 	  GECODE_ME_FAIL(home,bv.intersect(home, s.min(), s.max()));
 	} else {
	  IntSetRanges rd(s);
	  GECODE_ME_FAIL(home,bv.intersectI(home, rd));
 	}
      }
      break;
    case SRT_SUP:
      {
	if (s.size() == 1) {
	  GECODE_ME_FAIL(home,bv.include(home, s.min(), s.max()));
	} else {
	  IntSetRanges rd(s);
	  GECODE_ME_FAIL(home,bv.includeI(home, rd));
	}
      }
      break;
    case SRT_CMPL:
      {
	if (s.size() == 1) {
	  GECODE_ME_FAIL(home,bv.exclude(home, s.min(), s.max()));
	  GECODE_ME_FAIL(home,
			 bv.include(home,
				    Limits::Set::int_min,
				    s.min()-1) );
	  GECODE_ME_FAIL(home,
			 bv.include(home, s.max()+1,
				    Limits::Set::int_max) );
	} else {
	  IntSetRanges rd1(s);
	  Iter::Ranges::Compl<Limits::Set::int_min, 
	    Limits::Set::int_max, IntSetRanges > rdC1(rd1);
	  GECODE_ME_FAIL(home,bv.includeI(home, rdC1));
	  IntSetRanges rd2(s);
	  Iter::Ranges::Compl<Limits::Set::int_min, 
	    Limits::Set::int_max, IntSetRanges > rdC2(rd2);
	  GECODE_ME_FAIL(home,bv.intersectI(home, rdC2));
	}
      }
      break;
    default:
      {
	// could implement here lex ordering constraints s < 1 or something
	// thats nice
	throw Bdd::InvalidRelation("rel lex not implemented");
	return;
      }
    }
  }

}

// STATISTICS: bdd-post
