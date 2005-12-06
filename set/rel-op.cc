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

#include "set.hh"
#include "iter.hh"
#include "set/rel.hh"
#include "set/rel-op.hh"

namespace Gecode {
  using namespace Gecode::Set;
  using namespace Gecode::Set::Rel;
  using namespace Gecode::Set::RelOp;

  void
  rel(Space* home, SetVar x, SetOpType op, SetVar y, SetRelType r, SetVar z) {
    rel_op_post<SetView,SetView,SetView>(home, x, op, y, r, z);
  }

  void
  rel(Space* home, SetOpType op, const SetVarArgs& x, SetVar y) {
    if (home->failed()) return;
    ViewArray<SetView> xa(home,x);
    switch(op) {
    case SOT_UNION:
      GECODE_ES_FAIL(home,(RelOp::UnionN<SetView,SetView>::post(home, xa, y)));
      break;
    case SOT_DUNION:
      GECODE_ES_FAIL(home,
		     (RelOp::PartitionN<SetView,SetView>::post(home, xa, y)));
      break;
    case SOT_INTER:
      {
	GECODE_ES_FAIL(home,
		       (RelOp::IntersectionN<SetView,SetView>
			::post(home, xa, y)));
      }
      break;
    case SOT_MINUS:
      throw InvalidRelation("rel minus");
      break;
    }
  }

  void
  rel(Space* home, SetOpType op, const IntVarArgs& x, SetVar y) {
    if (home->failed()) return;
    ViewArray<SingletonView> xa(home,x.size());
    for (int i=x.size(); i--;) {
      Int::IntView iv(x[i]);
      SingletonView sv(iv);
      xa[i] = sv;
    }
      
    switch(op) {
    case SOT_UNION:
      GECODE_ES_FAIL(home,(RelOp::UnionN<SingletonView,SetView>
			   ::post(home, xa, y)));
      break;
    case SOT_DUNION:
      GECODE_ES_FAIL(home,(RelOp::PartitionN<SingletonView,SetView>
			   ::post(home, xa, y)));
      break;
    case SOT_INTER:
      GECODE_ES_FAIL(home,
		     (RelOp::IntersectionN<SingletonView,SetView>
		      ::post(home, xa, y)));
      break;
    case SOT_MINUS:
      throw InvalidRelation("rel minus");
      break;
    }
  }
}

// STATISTICS: set-post
