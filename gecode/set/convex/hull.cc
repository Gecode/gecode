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

#include "gecode/set.hh"
#include "gecode/set/convex.hh"

namespace Gecode { namespace Set { namespace Convex {

  Actor*
  ConvexHull::copy(Space* home, bool share) {
    return new (home) ConvexHull(home,share,*this);
  }

  ExecStatus
  ConvexHull::propagate(Space* home) {
    //x1 is the convex hull of x0

    GECODE_ME_CHECK( x1.cardMin(home,x0.cardMin()) );
    GECODE_ME_CHECK( x0.cardMax(home,x1.cardMax()) );

    do {

      //intersect x1 with (x0.lubMin(),x0.lubMax())
      //This empties x1 if x0.ub is empty. twice.
      GECODE_ME_CHECK( x1.exclude(home,Limits::Set::int_min,
                                  x0.lubMin()-1) );
      GECODE_ME_CHECK( x1.exclude(home,x0.lubMax()+1,
                                  Limits::Set::int_max) );

      int minElement = std::min(x1.glbMin(),x0.glbMin());
      int maxElement = std::max(x1.glbMax(),x0.glbMax());

      if (minElement<maxElement) {
        GECODE_ME_CHECK( x1.include(home, minElement, maxElement));
      }

      unsigned int cardMin = x1.cardMin();

      LubRanges<SetView> ubRangeIt(x1);
      Iter::Ranges::Cache< LubRanges<SetView> > ubRangeItC(ubRangeIt);
      for (;ubRangeItC();++ubRangeItC){
        if (ubRangeItC.width() < cardMin
            || ubRangeItC.min() > minElement //No need to test for empty lb.
            || ubRangeItC.max() < maxElement
            ) {
          GECODE_ME_CHECK( x1.exclude(home,
                                      ubRangeItC.min(), ubRangeItC.max()) );
        }
      }

      LubRanges<SetView> ubRangeIt2(x1);
      GECODE_ME_CHECK(x0.intersectI(home,ubRangeIt2) );

      if (x1.lubMin()!=BndSet::MIN_OF_EMPTY) {
        if(x1.lubMin()==x1.glbMin()) {
              GECODE_ME_CHECK(x0.include(home,x1.lubMin()));
        }
        if(x1.lubMax()==x1.glbMax()) {
              GECODE_ME_CHECK(x0.include(home,x1.lubMax()));
        }
      }
    } while(x0.assigned()&&!x1.assigned());
    
    //If x0 is assigned, x1 should be too.
    assert(x1.assigned() || !x0.assigned());

    if (x1.assigned()) {
      return ES_SUBSUMED;
    }

    return ES_NOFIX;
  }

}}}

// STATISTICS: set-prop
