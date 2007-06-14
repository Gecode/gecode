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



  void
  exactly(Space* home, BddVar x, IntSet& is, int c,  SetConLevel scl) {
    if (home->failed()) return;

    ViewArray<BddView> bv(home, 1);
    bv[0] = x;
    // std::cerr << "exactly on " << x << "\n";
    // SUBSUMPTION CHECK
    BddVarGlbRanges glb(x);
    if (glb()) {
      IntSetRanges ir(is);
      Gecode::Iter::Ranges::Inter<BddVarGlbRanges, IntSetRanges> inter(glb, ir);
      if (inter()) {
	// int v = inter.min();
	int s = inter.width();
	++inter;
	if (!inter() && s == 1) {
	  // std::cerr << "EXACTLY SUBSUMED!\n";
	  // std::cerr << "IntSet = " << is << "\n";
	  // std::cerr << "val = " << v << " and width = " << s << "\n";	
	  return;
	} else {
	  home->fail();
	  return;
	}
      }
      // std::cerr << "DONE\n";
    }

    BddVarUnknownRanges delta(x);
    IntSetRanges irange(is);
    Gecode::Iter::Ranges::Inter<BddVarUnknownRanges, IntSetRanges> 
      interdel(delta, irange);
    if (!interdel()) {
      // std::cerr << "EMPTY INTERSECT\n";
      home->fail();
      return;
    } 

    int mi = interdel.min();
    int ma = interdel.max();
    int s = interdel.width();
    ++interdel;
    if (!interdel() && s == 1) {
      // std::cerr << "EXACTLY INCLUDE!\n";
      // std::cerr << "EXACTLY SUBSUMED!\n";
      // std::cerr << "IntSet = " << is << "\n";
      // std::cerr << "val = " << mi << " and width = " << s << "\n";	
      GECODE_ME_FAIL(home, bv[0].include(home, mi));
      // std::cerr << "OKDONE\n";
      return;
    }
    // std::cerr << "DONE\n";

    Iter::Ranges::SingletonAppend<
      Gecode::Iter::Ranges::Inter<BddVarUnknownRanges, IntSetRanges>
      > si(mi,ma,interdel);

    BMI* mgr = bv[0].manager();
    unsigned int xtab = bv[0].table_width();
    unsigned int xoff = bv[0].offset();
    int xmin = bv[0].mgr_min();
    
    GecodeBdd d = cardConst(mgr, xtab, xoff, xmin, c, c, si);
    // std::cerr << "exactly: tell...";
    // GECODE_ME_FAIL(home, bv[0].tell_formula(home, d));
    GECODE_ES_FAIL(home, UnaryBddProp<BddView>::post(home, bv[0], d, scl));
    // std::cerr << "TELL-OK\n";

  }

  void
  atmost(Space* home, BddVar x, IntSet& is, int c,  SetConLevel scl) {
    if (home->failed()) return;

    // SUBSUMPTION CHECK
//     BddVarLubRanges lub(x);
//     IntSetRanges ir(is);
//     if (!Gecode::Iter::Ranges::subset(ir, lub)) {
//       std::cout << "SUBSUMED\n";
//     }

    ViewArray<BddView> bv(home, 1);
    bv[0] = x;

    BMI* mgr = bv[0].manager();
    unsigned int xtab = bv[0].table_width();
    unsigned int xoff = bv[0].offset();
    int xmin = bv[0].mgr_min();
    IntSetRanges ir(is);
    GecodeBdd d = cardConst(mgr, xtab, xoff, xmin, 0, c, ir);

    //GECODE_ME_FAIL(home, bv[0].tell_formula(home, d));

    GECODE_ES_FAIL(home, UnaryBddProp<BddView>::post(home, bv[0], d, scl));

  }

  void 
  atmost(Space* home, BddVar x, BddVar y, int c,  SetConLevel scl) {
    // std::cout << "call atmost\n";
    atmost_con(home, x, y, c, SRT_EQ, -1, scl);
  }

  void 
  atmostLex(Space* home, BddVar x, BddVar y, int c, 
	    BddSetRelType lex, SetConLevel scl) {
    atmost_con(home, x, y, c, lex, -1, scl);
  }

  void 
  atmostLexCard(Space* home, BddVar x, BddVar y, int c, 
		BddSetRelType lex, int d, SetConLevel scl) {
    atmost_con(home, x, y, c, lex, d, scl);
  }

  void 
  atmostCard(Space* home, BddVar x, BddVar y, int c, int d, SetConLevel scl) {
    atmost_con(home, x, y, c, SRT_EQ, d, scl);
  }

  void 
  atmost(Space* home, BddVar x, BddVar y, BddVar z, int c, SetConLevel scl) {
    atmost_con(home, x, y, z, c, SRT_EQ, -1, scl);
  }

  void 
  atmostOne(Space* home, const BddVarArgs& x, int c, SetConLevel scl) {
    atmostOne_con(home, x, c, scl);
  }


}

// STATISTICS: bdd-post
