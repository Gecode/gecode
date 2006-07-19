/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#include "gecode/set/projectors.hh"

namespace Gecode {

  int
  Projector::arity(void) const {
    return std::max(glb.arity(), lub.arity());
  }

  SetExpr::proj_scope
  Projector::scope(void) const {
    return SetExpr::combineScopes(glb.scope(-1), lub.scope(1));
  }

  ExecStatus
  Projector::check(Space* home, ViewArray<Set::SetView>& x) {
    {
      // Check if glb violates current upper bound of x[i]
      SetExprRanges glbranges(x,glb,false);
      Iter::Ranges::Size<SetExprRanges> g(glbranges);
      Set::LubRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(g, xir))
	return ES_FAILED;
      while (g()) ++g;
      if (g.size() > x[i].cardMax()) {
	return ES_FAILED;
      }	
    }
    {
      // Check if lub violates current lower bound of x[i]
      SetExprRanges lubranges(x,lub,true);
      Iter::Ranges::Size<SetExprRanges> l(lubranges);
      Set::GlbRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(xir, l))
	return ES_FAILED;
      while (l()) ++l;
      if (l.size() < x[i].cardMin()) {
	return ES_FAILED;
      }
    }
    {
      // Check if monotone interpretation of lower bound is
      // contained in current lower bound of x[i]. If not, the constraint
      // is not subsumed.
      SetExprRanges glbranges(x,glb,true);
      Set::GlbRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(glbranges, xir)) {
	return ES_FIX;
      }
    }
    {
      // Check if current upper bound of x[i] if contained in
      // anti-monotone interpretation of upper bound. If not, the constraint
      // is not subsumed.
      SetExprRanges lubranges(x,lub,false);
      Set::LubRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(xir, lubranges)) {
	return ES_FIX;
      }
    }
    // Both bounds, interpreted monotonically (glb) and anti-monotonically
    // (lub) are contained in the respective bounds of x[i]. This means
    // that the bounds are entailed.
    return ES_SUBSUMED;
  }

}

// STATISTICS: set-prop
