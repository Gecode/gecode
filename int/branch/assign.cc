/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "int/branch.hh"

namespace Gecode { namespace Int { namespace Branch {

  unsigned int
  Assign::branch(void) {
    for (int i = pos; i<x.size(); i++)
      if (!x[i].assigned()) {
	pos = i;
	return 1;
      }
    return 0;
  }


  Actor*
  AssignMin::copy(Space* home, bool share) {
    return new (home) AssignMin(home,share,*this);
  }
  BranchingDesc*
  AssignMin::description(void) {
    return new PosValDesc<int>(this, pos, x[pos].min());
  }
  ExecStatus
  AssignMin::commit(Space* home, unsigned int, BranchingDesc* _d) {
    PosValDesc<int>* d = static_cast<PosValDesc<int>*>(_d);
    int p, v;
    if (d == NULL) {
      p = pos; v = x[pos].min();
    } else {
      p = d->pos(); v = d->val();
    }
    return me_failed(x[p].eq(home,v)) ? ES_FAILED : ES_OK;
  }


  Actor*
  AssignMed::copy(Space* home, bool share) {
    return new (home) AssignMed(home,share,*this);
  }
  BranchingDesc*
  AssignMed::description(void) {
    return new PosValDesc<int>(this, pos, x[pos].med());
  }
  ExecStatus
  AssignMed::commit(Space* home, unsigned int, BranchingDesc* _d) {
    PosValDesc<int>* d = static_cast<PosValDesc<int>*>(_d);
    int p, v;
    if (d == NULL) {
      p = pos; v = x[pos].med();
    } else {
      p = d->pos(); v = d->val();
    }
    return me_failed(x[p].eq(home,v)) ? ES_FAILED : ES_OK;
  }


  Actor*
  AssignMax::copy(Space* home, bool share) {
    return new (home) AssignMax(home,share,*this);
  }
  BranchingDesc*
  AssignMax::description(void) {
    return new PosValDesc<int>(this, pos, x[pos].max());
  }
  ExecStatus
  AssignMax::commit(Space* home, unsigned int, BranchingDesc* _d) {
    PosValDesc<int>* d = static_cast<PosValDesc<int>*>(_d);
    int p, v;
    if (d == NULL) {
      p = pos; v = x[pos].max();
    } else {
      p = d->pos(); v = d->val();
    }
    return me_failed(x[p].eq(home,v)) ? ES_FAILED : ES_OK;
  }

}}}

// STATISTICS: int-branch

