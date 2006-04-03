/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Gabor Szokoli, 2003
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

#include "int/rel.hh"

#include <algorithm>

namespace Gecode {

  using namespace Int;

  void
  rel(Space* home, IntVar x0, IntRelType r, int c, IntConLevel) {
    if (home->failed()) return;
    IntView x(x0);
    switch (r) {
    case IRT_EQ:
      GECODE_ME_FAIL(home,x.eq(home,c)); break;
    case IRT_NQ:
      GECODE_ME_FAIL(home,x.nq(home,c)); break;
    case IRT_LQ:
      GECODE_ME_FAIL(home,x.lq(home,c)); break;
    case IRT_LE:
      GECODE_ME_FAIL(home,x.le(home,c)); break;
    case IRT_GQ:
      GECODE_ME_FAIL(home,x.gq(home,c)); break;
    case IRT_GR:
      GECODE_ME_FAIL(home,x.gr(home,c)); break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space* home, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      if (icl == ICL_BND) {
	GECODE_ES_FAIL(home,(Rel::EqBnd<IntView,IntView>::post(home,x0,x1)));
      } else {
	GECODE_ES_FAIL(home,(Rel::EqDom<IntView,IntView>::post(home,x0,x1)));
      }
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,Rel::Nq<IntView>::post(home,x0,x1)); break;
    case IRT_GQ:
      std::swap(x0,x1); // Fall through
    case IRT_LQ:
      GECODE_ES_FAIL(home,Rel::Lq<IntView>::post(home,x0,x1)); break;
    case IRT_GR:
      std::swap(x0,x1); // Fall through
    case IRT_LE:
      GECODE_ES_FAIL(home,Rel::Le<IntView>::post(home,x0,x1)); break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }


  void
  rel(Space* home, IntVar x0, IntRelType r, IntVar x1, BoolVar b, 
      IntConLevel icl) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      if (icl == ICL_BND) {
	if (Rel::ReEqBnd<IntView,BoolView>::post(home,x0,x1,b) 
	    == ES_FAILED)
	  home->fail();
      } else {
	if (Rel::ReEqDom<IntView,BoolView>::post(home,x0,x1,b)
	    == ES_FAILED)
	  home->fail();
      }
      break;
    case IRT_NQ:
      {
	NegBoolView n(b);
	if (icl == ICL_BND) {
	  if (Rel::ReEqBnd<IntView,NegBoolView>::post(home,x0,x1,n)
	      == ES_FAILED)
	    home->fail();
	} else {
	  if (Rel::ReEqDom<IntView,NegBoolView>::post(home,x0,x1,n)
	      == ES_FAILED)
	    home->fail();
	}
      }
      break;
    case IRT_GQ:
      std::swap(x0,x1); // Fall through
    case IRT_LQ:
      if (Rel::ReLq<IntView,BoolView>::post(home,x0,x1,b) == ES_FAILED)
	home->fail();
      break;
    case IRT_LE:
      std::swap(x0,x1); // Fall through
    case IRT_GR:
      {
	NegBoolView n(b);
	if (Rel::ReLq<IntView,NegBoolView>::post(home,x0,x1,n) == ES_FAILED)
	  home->fail();
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  rel(Space* home, IntVar x, IntRelType r, int c, BoolVar b, 
      IntConLevel icl) {
    if (home->failed()) return;
    switch (r) {
    case IRT_EQ:
      if (icl == ICL_BND) {
	if (Rel::ReEqBndInt<IntView,BoolView>::post(home,x,c,b)
	    == ES_FAILED)
	  home->fail();
      } else {
	if (Rel::ReEqDomInt<IntView,BoolView>::post(home,x,c,b)
	    == ES_FAILED)
	  home->fail();
      }
      break;
    case IRT_NQ:
      {
	NegBoolView n(b);
	if (icl == ICL_BND) {
	  if (Rel::ReEqBndInt<IntView,NegBoolView>::post(home,x,c,n)
	      == ES_FAILED)
	    home->fail();
	} else {
	  if (Rel::ReEqDomInt<IntView,NegBoolView>::post(home,x,c,n)
	      == ES_FAILED)
	    home->fail();
	}
      }
      break;
    case IRT_LE:
      c--; // Fall through
    case IRT_LQ:
      if (Rel::ReLqInt<IntView,BoolView>::post(home,x,c,b) == ES_FAILED)
	home->fail();
      break;
    case IRT_GQ:
      c--; // Fall through
    case IRT_GR: 
      {
	NegBoolView n(b);
	if (Rel::ReLqInt<IntView,NegBoolView>::post(home,x,c,n) == ES_FAILED)
	  home->fail();
      }
      break;
    default:
      throw UnknownRelation("Int::rel");
    }
  }

  void
  eq(Space* home, IntVar x0, IntVar x1, IntConLevel icl) {
    if (home->failed()) return;
    if (icl == ICL_BND) {
      GECODE_ES_FAIL(home,(Rel::EqBnd<IntView,IntView>::post(home,x0,x1)));
    } else {
      GECODE_ES_FAIL(home,(Rel::EqDom<IntView,IntView>::post(home,x0,x1)));
    }
  }

  void
  eq(Space* home, IntVar x0, int c, IntConLevel) {
    if (home->failed()) return;
    IntView x(x0);
    GECODE_ME_FAIL(home,x.eq(home,c));
  }

  void
  eq(Space* home, IntVar x0, IntVar x1, BoolVar b, IntConLevel icl) {
    if (home->failed()) return;
    if (icl == ICL_BND) {
      if (Rel::ReEqBnd<IntView,BoolView>::post(home,x0,x1,b) == ES_FAILED)
	home->fail();
    } else {
      if (Rel::ReEqDom<IntView,BoolView>::post(home,x0,x1,b) == ES_FAILED)
	home->fail();
    }
  }

  void
  eq(Space* home, IntVar x, int c, BoolVar b, IntConLevel icl) {
    if (home->failed()) return;
    if (icl == ICL_BND) {
      if (Rel::ReEqBndInt<IntView,BoolView>::post(home,x,c,b) == ES_FAILED)
	home->fail();
    } else {
      if (Rel::ReEqDomInt<IntView,BoolView>::post(home,x,c,b) == ES_FAILED)
	home->fail();
    }
  }

  void
  eq(Space* home, const IntVarArgs& x, IntConLevel icl) {
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    if (icl == ICL_BND) {
      GECODE_ES_FAIL(home,Rel::NaryEqBnd<IntView>::post(home,xv));
    } else {
      GECODE_ES_FAIL(home,Rel::NaryEqDom<IntView>::post(home,xv));
    }
  }


  void
  lex(Space* home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel) {
    if (x.size() != y.size())
      throw ArgumentSizeMismatch("Int::lex");
    if (home->failed()) return;

    ViewArray<ViewTuple<IntView,2> > xy(home,x.size());
    switch (r) {
    case IRT_GR:
      for (int i = x.size(); i--; ) {
	xy[i][0]=y[i]; xy[i][1]=x[i];
      }
      goto le;
    case IRT_LE:
      for (int i = x.size(); i--; ) {
	xy[i][0]=x[i]; xy[i][1]=y[i];
      }
    le:
      if (Rel::Lex<IntView>::post(home,xy,true) == ES_FAILED)
	home->fail();
      break;
    case IRT_GQ:
      for (int i = x.size(); i--; ) {
	xy[i][0]=y[i]; xy[i][1]=x[i];
      }
      goto lq;
    case IRT_LQ:
      for (int i = x.size(); i--; ) {
	xy[i][0]=x[i]; xy[i][1]=y[i];
      }
    lq:
      if (Rel::Lex<IntView>::post(home,xy,false) == ES_FAILED)
	home->fail();
      break;
    default:
      throw OnlyInequalityRelations("Int::lex");
    }
  }

}

// STATISTICS: int-post

