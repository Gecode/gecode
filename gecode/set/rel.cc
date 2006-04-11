/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004, 2005
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

#include "gecode/set.hh"
#include "gecode/iter.hh"
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"
#include "gecode/set/int.hh"

namespace Gecode {
  using namespace Set;
  using namespace Set::Rel;
  using namespace Set::RelOp;
  
  template <class View0, class View1>
  void
  rel_post(Space* home, View0 x0, SetRelType r, View1 x1) {
    if (home->failed()) return;
    switch(r) {
    case SRT_EQ:
      {
	GECODE_ES_FAIL(home,
		       (Eq<View0,View1>::post(home,x0,x1)));
      }
      break;
    case SRT_NQ:
      {
	GECODE_ES_FAIL(home,
		       (Distinct<View0,View1>::post(home,x0,x1)));
      }
      break;
    case SRT_SUB:
      {
	GECODE_ES_FAIL(home,
		       (SubSet<View0,View1>::post(home, x0,x1)));
      }
      break;
    case SRT_SUP:
      {
	GECODE_ES_FAIL(home,
		       (SubSet<View1,View0>::post(home, x1,x0)));
      }
      break;
    case SRT_DISJ:
      {
	EmptyView emptyset;
	GECODE_ES_FAIL(home,(SuperOfInter<View0,View1,EmptyView>
			     ::post(home, x0, x1, emptyset)));
      }
      break;
    case SRT_CMPL:
      {
	ComplementView<View0> cx0(x0);
	GECODE_ES_FAIL(home,
		       (Eq<ComplementView<View0>, View1>
			::post(home, cx0, x1)));
      }
      break;
    }
  }

  template <class View0, class View1>
  void
  rel_re(Space* home, View0 x, SetRelType r, View1 y, BoolVar b) {
    if (home->failed()) return;
    switch(r) {
    case SRT_EQ:
      {
	GECODE_ES_FAIL(home,
		       (ReEq<View0,View1>::post(home, x,y,b)));
      }
      break;
    case SRT_NQ:
      {
	BoolVar notb(home, 0, 1);
	bool_not(home, b, notb);
	GECODE_ES_FAIL(home,
		       (ReEq<View0,View1>::post(home,
							   x,y,notb)));
      }
      break;
    case SRT_SUB:
      {
	GECODE_ES_FAIL(home,
		       (ReSubset<View0,View1>::post(home, x,y,b)));
      }
      break;
    case SRT_SUP:
      {
	GECODE_ES_FAIL(home,
		       (ReSubset<View1,View0>::post(home, y,x,b)));
      }
      break;
    case SRT_DISJ:
      {
	// x||y <=> b is equivalent to
	// ( y <= complement(x) and x<=complement(y) ) <=> b

	// set up BoolVars for the conjunction
	BoolVar b1(home, 0, 1);
	BoolVar b2(home, 0, 1);
	bool_and(home, b1, b2, b);	

	ComplementView<View0> xc(x);
	ComplementView<View1> yc(y);

	GECODE_ES_FAIL(home,
		       (ReSubset<View0,ComplementView<View1> >
			::post(home, x, yc, b1)));
	GECODE_ES_FAIL(home,
		       (ReSubset<View1,ComplementView<View0> >
			::post(home, y, xc, b2)));
      }
      break;
    case SRT_CMPL:
      {
	ComplementView<View0> cx(x);
	GECODE_ES_FAIL(home,
		       (ReEq<ComplementView<View0>,
			View1>::post(home, cx, y, b)));
      }
      break;
    }
  }

  void
  rel(Space* home, SetVar x, SetRelType r, SetVar y) {
    rel_post<SetView,SetView>(home,x,r,y);
  }

  void
  rel(Space* home, SetVar s, SetRelType r, IntVar x) {
    Gecode::Int::IntView xv(x);
    SingletonView xsingle(xv);
    rel_post<SetView,SingletonView>(home,s,r,xv);
  }

  void
  rel(Space* home, IntVar x, SetRelType r, SetVar s) {
    switch(r) {
    case SRT_SUB:
      rel(home, s, SRT_SUP, x);
      break;
    case SRT_SUP:
      rel(home, s, SRT_SUB, x);
      break;
    default:
      rel(home, s, r, x);
    }
  }

  void
  rel(Space* home, SetVar x, SetRelType r, SetVar y, BoolVar b) {
    rel_re<SetView,SetView>(home,x,r,y,b);
  }

  void
  rel(Space* home, SetVar s, SetRelType r, IntVar x, BoolVar b) {
    Gecode::Int::IntView xv(x);
    SingletonView xsingle(xv);
    rel_re<SetView,SingletonView>(home,s,r,xsingle,b);
  }

  void
  rel(Space* home, IntVar x, SetRelType r, SetVar s, BoolVar b) {
    switch(r) {
    case SRT_SUB:
      rel(home, s, SRT_SUP, x, b);
      break;
    case SRT_SUP:
      rel(home, s, SRT_SUB, x, b);
      break;
    default:
      rel(home, s, r, x, b);
    }
  }

}

// STATISTICS: set-post
