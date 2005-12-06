/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
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

#include "int/sortedness.hh"
namespace Gecode {

  using namespace Int;
  void sortedness(Space* home, 
		  const IntVarArgs& x, 
		  const IntVarArgs& y, 
		  IntConLevel) {
    int n = x.size();
    if (n != y.size()) {

      throw ArgumentSizeMismatch("Int::sortedness");
    }
    if (home->failed()) {
      return;
    }

    ViewArray<IntView> y0(home, y);
    ViewArray<IntView> x_check_shared(home, x);

    ViewArray<ViewTuple<IntView,1> > x0(home, n);

    for (int i = n; i--; ) {
      x0[i][0] = x[i];
    }

    if (x_check_shared.shared() ||
	y0.shared()) {
      GECODE_ES_FAIL(home, 
		     (Sortedness::
		      Sortedness<IntView, ViewTuple<IntView,1>, false, true>::
		      post(home, x0, y0)));

    } else {
      GECODE_ES_FAIL(home, 
		     (Sortedness::
		      Sortedness<IntView, ViewTuple<IntView,1>, false, false >::
		      post(home, x0, y0)));
    }
  }

  void sortedness(Space* home, 
		  const IntVarArgs& x, 
		  const IntVarArgs& y, 
		  const IntVarArgs& z, 
		  IntConLevel) {
    int n = x.size();
    if ((n != y.size()) || (n != z.size())) {
      throw ArgumentSizeMismatch("Int::sortedness");
    }
    if (home->failed()) {
      return;
    }

    ViewArray<IntView> x_check_shared(home, x);
    ViewArray<IntView> z_check_shared(home, z);
    // permutation variables have to be all different
    if (z_check_shared.shared() && z.size() > 1) {
      home->fail();
    }

    ViewArray<IntView> y0(home, y);
    ViewArray<ViewTuple<IntView, 2> > xz0(home, n);

    // assert that permutation variables encode a permutation
    ViewArray<IntView> pz0(home, n);
    for (int i = n; i--; ) {
      xz0[i][0] = x[i];
      xz0[i][1] = z[i];
      pz0[i]    = z[i];
      // Constrain z_i to a valid index
      GECODE_ME_FAIL(home,xz0[i][1].gq(home,0));
      GECODE_ME_FAIL(home,xz0[i][1].lq(home,n - 1));
    }
    // assert permutation
    GECODE_ES_FAIL(home, Distinct::Bnd<IntView>::post(home, pz0));

    if (x_check_shared.shared() ||
	y0.shared()) {
      GECODE_ES_FAIL(home, 
		     (Sortedness::
		      Sortedness<IntView, ViewTuple<IntView,2>, true, true >::
		      post(home, xz0, y0)));
    } else {
      GECODE_ES_FAIL(home, 
		     (Sortedness::
		      Sortedness<IntView, ViewTuple<IntView,2>, true, false >::
		      post(home, xz0, y0)));
    }
  }
}

// STATISTICS: int-post
