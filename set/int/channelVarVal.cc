/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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



#include "set/int.hh"

#include "iter.hh"

#include "set/rel.hh"

namespace Gecode { namespace Set { namespace Int {

  PropCost
  ChannelVarVal::cost(void) const {
    return PC_QUADRATIC_LO;
  }

  ChannelVarVal::~ChannelVarVal(void) {
    xs.cancel(this, Gecode::Int::PC_INT_DOM);
    ys.cancel(this, PC_SET_ANY);
  }

  Actor*
  ChannelVarVal::copy(Space* home, bool share) {
    return new (home) ChannelVarVal(home,share,*this);
  }

  ExecStatus
  ChannelVarVal::propagate(Space* home) {
    int assigned = 0;
    for (int v=xs.size(); v--;) {
      if (xs[v].assigned()) {
	assigned += 1;
        for (int i=ys.size(); i--;) {
          if (i==xs[v].val()) {
            GECODE_ME_CHECK(ys[i].include(home, v));
          }
          else {
            GECODE_ME_CHECK(ys[i].exclude(home, v));
          }
        }
      } else {

        for (int i=ys.size(); i--;) {
          if (ys[i].notContains(v)) {
            GECODE_ME_CHECK(xs[v].nq(home, i));
          }
          if (ys[i].contains(v)) {
            GECODE_ME_CHECK(xs[v].eq(home, i));
          }
        }

        Gecode::Int::ViewRanges<Gecode::Int::IntView> xsv(xs[v]);
        int min = 0;
        for (; xsv(); ++xsv) {
          for (int i=min; i<xsv.min(); i++) {
            GECODE_ME_CHECK(ys[i].exclude(home, v));
          }
          min = xsv.max() + 1;
        }

      }
    }

    return (assigned==xs.size()) ? ES_SUBSUMED : ES_NOFIX;
  }


}}}

// STATISTICS: set-prop
