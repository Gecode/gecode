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

  void
  ProjectorSet::add(const Projector& p) {
    _ps.ensure(_count+1);    
    new (&_ps[_count]) Projector(p);
    for (int i=_count+1; i<_ps.size(); i++)
      new (&_ps[i]) Projector();
    _count++;

    _arity = std::max(_arity, p.arity());
  }

  SetExpr::proj_scope
  ProjectorSet::scope(void) const {
    SetExpr::proj_scope scope;
    for (int i=0; i<_count; i++) {
      scope = SetExpr::combineScopes(_ps[i].scope(), scope);
    }
    return scope;
  }

  ExecStatus
  ProjectorSet::check(Space* home, ViewArray<Set::SetView>& x) {
    ExecStatus es = ES_SUBSUMED;
    for (int i=0; i<_count; i++) {
      ExecStatus es_new = _ps[i].check(home, x);
      switch (es_new) {
      case ES_FAILED:
	return ES_FAILED;
      case ES_SUBSUMED:
	break;
      default:
	es = es_new;
	break;
      }
    }
    return es;
  }

}

// STATISTICS: set-prop
