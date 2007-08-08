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

  void
  cardinality(Space* home, CpltSetVar x, unsigned int l, unsigned int u, 
	      SetConLevel scl) {
    // std::cerr << "calling card: " << l << "/" << u << " on " << x << "\n";
    if (home->failed()) return;

    ViewArray<BddView> bv(home, 1);
    bv[0] = x;
    BMI* mgr = x.manager();
    unsigned int off = bv[0].offset();
    unsigned int range = bv[0].table_width();

    // std::cout << "post cardinality for x=" << x << "\n";

    GecodeBdd c = cardcheck(mgr, range, off, static_cast<int> (l), static_cast<int> (u));

    switch (scl) {
    case SCL_LEX:
      {
	ViewArray<LexBddView> lexv(home, 1);
	lexv[0].init(bv[0]);
	GECODE_ES_FAIL(home, NaryBddProp<LexBddView>::post(home, lexv, c, scl));
	break;
      }
    case SCL_BND_BDD:  
      {
	ViewArray<BndBddView> bndv(home, 1);
	bndv[0].init(bv[0]);
	GECODE_ES_FAIL(home, NaryBddProp<BndBddView>::post(home, bndv, c, scl));
	break;
      }
    case SCL_BND_SBR:  
      {
	CrdBddView cv(x);
	SetBddView sv(cv);
	GECODE_ME_FAIL(home, sv.cardinality(home, l, u));
	break;
      }
    case SCL_CRD:
      {
// 	ViewArray<CrdBddView> crdv(home, 1);
// 	crdv[0].init(bv[0]);
// 	GECODE_ES_FAIL(home, NaryBddProp<CrdBddView>::post(home, crdv, c, scl));
// 	break;
	CrdBddView cv(x);
	// std::cout << "init card: " << l << "," <<u << " " << cv.offset() << "\n";
	GECODE_ME_FAIL(home, cv.cardinality(home, l, u));
	break;
      }
    case SCL_SPL:
      {
	SplitBddView sv(x);
	GECODE_ME_FAIL(home, sv.cardinality(home, l, u));
	break;
      }
    default:
      {

	BddView v(x);
	GECODE_ME_FAIL(home, v.cardinality(home, l, u));

// 	ViewArray<BddView> bv(home, 1);
// 	bv[0] = x;
// 	GECODE_ES_FAIL(home, NaryBddProp<BddView>::post(home, bv, c, scl));
	break;
      }
    }
  }

  void
  cardinality(Space* home, CpltSetVar x, unsigned int c, 
	      SetConLevel scl) {
    cardinality(home, x, c, c, scl);
  }

}

// STATISTICS: bdd-post
