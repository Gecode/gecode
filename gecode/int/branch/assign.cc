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

#include "gecode/int/branch.hh"

namespace Gecode { namespace Int { namespace Branch {

  size_t
  AssignDesc::size(void) const {
    return sizeof(*this);
  }


  bool
  Assign::status(const Space*) const {
    for (int i = pos; i<x.size(); i++)
      if (!x[i].assigned()) {
	pos = i;
	return true;
      }
    return false;
  }
  ExecStatus
  Assign::commit(Space* home, const BranchingDesc* _d, unsigned int) {
    const AssignDesc* d = static_cast<const AssignDesc*>(_d);
    return me_failed(x[d->pos()].eq(home,d->val())) ? ES_FAILED : ES_OK;
  }


  Actor*
  AssignMin::copy(Space* home, bool share) {
    return new (home) AssignMin(home,share,*this);
  }
  BranchingDesc*
  AssignMin::description(const Space*) const {
    assert(!x[pos].assigned());
    return new AssignDesc(this, pos, x[pos].min());
  }


  Actor*
  AssignMed::copy(Space* home, bool share) {
    return new (home) AssignMed(home,share,*this);
  }
  BranchingDesc*
  AssignMed::description(const Space*) const {
    assert(!x[pos].assigned());
    return new AssignDesc(this, pos, x[pos].med());
  }


  Actor*
  AssignMax::copy(Space* home, bool share) {
    return new (home) AssignMax(home,share,*this);
  }
  BranchingDesc*
  AssignMax::description(const Space*) const {
    assert(!x[pos].assigned());
    return new AssignDesc(this, pos, x[pos].max());
  }

}}}

// STATISTICS: int-branch

