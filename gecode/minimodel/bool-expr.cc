/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#include "gecode/minimodel.hh"

namespace Gecode { 

  namespace MiniModel {
    
    bool
    BoolExpr::Node::decrement(void) {
      if (--use == 0) {
	if ((l != NULL) && l->decrement())
	  delete l;
	if ((r != NULL) && r->decrement())
	  delete r;
	return true;
      }
      return false;
    }
    
    BoolVar
    BoolExpr::Node::post(Space* home) const {
      if (t == BT_VAR)
	return x;
      BoolVar b(home,0,1);
      post(home,b);
      return b;
    }

    int
    BoolExpr::Node::post(Space* home, NodeType t, 
			 BoolVarArgs& b, int i) const {
      if (this->t != t) {
	b[i] = post(home);
	return i+1;
      } else {
	return l->post(home,t,b,r->post(home,t,b,i));
      }
    }

    void
    BoolExpr::Node::post(Space* home, BoolVar b) const {
      assert(t != BT_VAR);
      switch (t) {
      case BT_NOT:
	bool_not(home, l->post(home), b);
	break;
      case BT_AND:
	if (same > 2) {
	  BoolVarArgs ba(same);
	  (void) post(home,BT_AND,ba,0);
	  bool_and(home, ba, b);
	} else {
	  bool_and(home, l->post(home), r->post(home), b);
	}
	break;
      case BT_OR:
	if (same > 2) {
	  BoolVarArgs ba(same);
	  (void) post(home,BT_OR,ba,0);
	  bool_or(home, ba, b);
	} else {
	  bool_or(home, l->post(home), r->post(home), b);
	}
	break;
      case BT_IMP:
	bool_imp(home, l->post(home), r->post(home), b);
	break;
      case BT_XOR:
	bool_xor(home, l->post(home), r->post(home), b);
	break;
      case BT_EQV:
	bool_eqv(home, l->post(home), r->post(home), b);
	break;
      default:
	GECODE_NEVER;
      case BT_RLIN:
	rl.post(home,b);
	break;
      }
    }

    void
    BoolExpr::Node::post(Space* home, bool b) const {
      if (b) {
	switch (t) {
	case BT_VAR:
	  rel(home, x, IRT_EQ, 1);
	  break;
	case BT_NOT:
	  l->post(home,false);
	  break;
	case BT_OR:
	  if (same > 2) {
	    BoolVarArgs ba(same);
	    (void) post(home,BT_OR,ba,0);
	    bool_or(home, ba, true);
	  } else {
	    bool_or(home, l->post(home), r->post(home), true);
	  }
	  break;
	case BT_AND:
	  l->post(home,true);
	  r->post(home,true);
	  break;
	case BT_EQV:
	  if ((l->t == BT_VAR) && (r->t != BT_VAR)) {
	    r->post(home,l->x);
	  } else if ((l->t != BT_VAR) && (r->t == BT_VAR)) {
	    l->post(home,r->x);
	  } else if ((l->t != BT_VAR) && (r->t != BT_VAR)) {
	    BoolVar b(home,0,1);
	    l->post(home,b);
	    r->post(home,b);
	  } else {
	    BoolVar b(home,1,1);
	    post(home,b);
	  }
	  break;
	case BT_RLIN:
	  rl.post(home,true,ICL_DEF);
	  break;
	default:
	  {
	    BoolVar b(home,1,1);
	    post(home,b);
	  }
	  break;
	}
      } else {
	switch (t) {
	case BT_VAR:
	  rel(home, x, IRT_EQ, 0);
	  break;
	case BT_NOT:
	  l->post(home,true);
	  break;
	case BT_OR:
	  l->post(home,false);
	  r->post(home,false);
	  break;
	case BT_AND:
	  if (same > 2) {
	    BoolVarArgs ba(same);
	    (void) post(home,BT_AND,ba,0);
	    bool_and(home, ba, false);
	  } else {
	    bool_and(home, l->post(home), r->post(home), false);
	  }
	  break;
	case BT_IMP:
	  l->post(home,true);
	  r->post(home,false);
	  break;
	case BT_XOR:
	  if ((l->t == BT_VAR) && (r->t != BT_VAR)) {
	    r->post(home,l->x);
	  } else if ((l->t != BT_VAR) && (r->t == BT_VAR)) {
	    l->post(home,r->x);
	  } else if ((l->t != BT_VAR) && (r->t != BT_VAR)) {
	    BoolVar b(home,0,1);
	    l->post(home,b);
	    r->post(home,b);
	  } else {
	    BoolVar b(home,0,0);
	    post(home,b);
	  }
	  break;
	case BT_RLIN:
	  rl.post(home,false,ICL_DEF);
	  break;
	default: 
	  {
	    BoolVar b(home,0,0);
	    post(home,b);
	  }
	  break;
	}
      }
    }

  }

}

// STATISTICS: minimodel-any
