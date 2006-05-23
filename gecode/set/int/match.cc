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



#include "gecode/set/int.hh"

#include "gecode/iter.hh"

namespace Gecode { namespace Set { namespace Int {

  PropCost
  Match::cost(void) const {
    return PC_LINEAR_LO;
  }

  void
  Match::dispose(Space* home) {
    x0.cancel(this, PC_SET_ANY);
    xs.cancel(this, Gecode::Int::PC_INT_BND);
    Propagator::dispose(home);
  }

  Actor*
  Match::copy(Space* home, bool share) {
    return new (home) Match(home,share,*this);
  }

  ExecStatus
  Match::propagate(Space* home) {

    int xs_size = xs.size();

    bool loopFlag;

    do {
      loopFlag = false;
      
      // Order int vars in xs
      GECODE_ME_CHECK(xs[0].gq(home,x0.lubMin()));
      for (int i=xs_size-1; i--; ) {
	GECODE_ME_CHECK_B(loopFlag, xs[i+1].gq(home,xs[i].min() + 1));
      }
      
      GECODE_ME_CHECK_B(loopFlag, xs[xs_size-1].lq(home,x0.lubMax()));
      for (int i=xs_size-2; i--; ) {
	GECODE_ME_CHECK_B(loopFlag, xs[i].lq(home,xs[i+1].max() - 1));
      }

      // if y from xs is assigned, add to glb(x0)
      for (int i=xs_size; i--; ) {
        if (xs[i].assigned()) {
	  GECODE_ME_CHECK_B(loopFlag, x0.include(home,xs[i].val()));
	}
      }

      // intersect every y in xs with lub(x0)
      for (int i=xs_size; i--; ) {
        LubRanges<SetView> ub(x0);
        GECODE_ME_CHECK_B(loopFlag, xs[i].inter(home,ub));
      }

      // remove gaps between vars in xs from lub(x0)
      GECODE_ME_CHECK_B(loopFlag,
			x0.exclude(home,Limits::Set::int_min,xs[0].min()-1));
      GECODE_ME_CHECK_B(loopFlag,
			x0.exclude(home,xs[xs_size-1].max()+1,
				   Limits::Set::int_max));

      for (int i=xs_size-1; i--; ) {
        int start = xs[i].max() + 1;
        int end   = xs[i+1].min() - 1;
        if (start<=end) {
          GECODE_ME_CHECK_B(loopFlag, x0.exclude(home,start,end));
        }
      }

      // try to assign vars in xs from glb(x0)
      if (x0.glbSize()>0) {

        LubRanges<SetView> ub(x0);
        Iter::Ranges::ToValues<LubRanges<SetView> > ubv(ub);
        GlbRanges<SetView> lb(x0);
        Iter::Ranges::ToValues<GlbRanges<SetView> > lbv(lb);

        int i=0;
        for (; ubv() && lbv() && ubv.val()==lbv.val();
            ++ubv, ++lbv, i++) {
          GECODE_ME_CHECK_B(loopFlag, xs[i].eq(home,lbv.val()));
        }

        if (i<xs_size-1 && x0.lubMax()==x0.glbMax()) {
	  LubRanges<SetView> lbx0(x0);
	  GlbRanges<SetView> ubx0(x0);
	  Iter::Ranges::Inter<LubRanges<SetView>,GlbRanges<SetView> >
	    inter(lbx0, ubx0);
	  
	  int to = x0.glbMax();
	  int from = to;
	  while (inter()) {
	    from = inter.min();
	    ++inter;
	  }

	  int i=xs_size-1;
	  for (int j=to; j>=from;j--,i--) {
            GECODE_ME_CHECK_B(loopFlag, xs[i].eq(home,j));
	  }
        }
      }

    } while (loopFlag);

    for (int i=xs_size; i--; )
      if (!xs[i].assigned())	
	return ES_FIX;
    return ES_SUBSUMED;
  }

}}}

// STATISTICS: set-prop
