/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-04-11 15:58:37 +0200 (Tue, 11 Apr 2006) $ by $Author: tack $
 *     $Revision: 3188 $
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

#include "gecode/set/projectors/propagator.hh"

using namespace Gecode::Set;

namespace Gecode {

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
		 ProjectorSet& ps, bool negated) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 2);
    x[0] = xa; x[1] = ya;
    if (negated) {
      GECODE_ES_FAIL(home,
		     (Projection::NaryProjection<true>::post(home, 
							     x, ps)));
    } else {
      GECODE_ES_FAIL(home,
		     (Projection::NaryProjection<false>::post(home, 
							      x, ps)));
    }
    
  }

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
		 const BoolVar& bv, ProjectorSet& ps) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 2);
    x[0] = xa; x[1] = ya;
    Gecode::Int::BoolView b(bv);
    GECODE_ES_FAIL(home,
		   (Projection::ReNaryProjection::post(home, 
						       x, b, ps)));
  }

  void projector(Space* home,
		 const SetVar& xa, const SetVar& ya, const SetVar& za,
		 ProjectorSet& ps, bool negated) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 3);
    x[0] = xa; x[1] = ya; x[2] = za;
    if (negated) {
      GECODE_ES_FAIL(home,
		     (Projection::NaryProjection<true>::post(home, 
							     x, ps)));
    } else {
      GECODE_ES_FAIL(home,
		     (Projection::NaryProjection<false>::post(home, 
							      x, ps)));
    }
    
  }

  void projector(Space* home,
		 const SetVar& xa, const SetVar& ya, const SetVar& za,
		 const BoolVar& bv, ProjectorSet& ps) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 3);
    x[0] = xa; x[1] = ya; x[2] = za;
    Gecode::Int::BoolView b(bv);
    GECODE_ES_FAIL(home,
		   (Projection::ReNaryProjection::post(home, 
							  x, b, ps)));
  }

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
		 const IntVar& i, Projector& p) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 2);
    x[0] = xa; x[1] = ya;
    Gecode::Int::IntView iv(i);
    GECODE_ES_FAIL(home,
		   (Projection::CardProjection::post(home,x,i,p)));
  }

  void projector(Space* home, const SetVar& xa, const SetVar& ya,
		 const SetVar& za, const IntVar& i, Projector& p) {
    if (home->failed()) return;
    ViewArray<SetView> x(home, 3);
    x[0] = xa; x[1] = ya; x[2] = za;
    Gecode::Int::IntView iv(i);
    GECODE_ES_FAIL(home,
		   (Projection::CardProjection::post(home,x,i,p)));
  }
  

}

// STATISTICS: set-post
