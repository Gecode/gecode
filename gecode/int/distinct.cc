/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Gabor Szokoli, 2003
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

#include "gecode/int/distinct.hh"

namespace Gecode {

  using namespace Int;

  void
  distinct(Space* home, const IntVarArgs& x, IntConLevel icl, PropVar) {
    if (x.same())
      throw ArgumentSame("Int::distinct");
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    switch (icl) {
    case ICL_BND:
      GECODE_ES_FAIL(home,Distinct::Bnd<IntView>::post(home,xv));
      break;
    case ICL_DOM:
      GECODE_ES_FAIL(home,Distinct::Dom<IntView>::post(home,xv));
      break;
    default:
      GECODE_ES_FAIL(home,Distinct::Val<IntView>::post(home,xv));
    }
  }

  void
  distinct(Space* home, const IntArgs& c, const IntVarArgs& x,
           IntConLevel icl, PropVar) {
    if (x.same())
      throw ArgumentSame("Int::distinct");
    if (c.size() != x.size())
      throw ArgumentSizeMismatch("Int::distinct");
    if (home->failed()) return;
    ViewArray<OffsetView> cx(home,x.size());
    for (int i = c.size(); i--; )
      if ((c[i] < Limits::Int::int_min) || (c[i] > Limits::Int::int_max))
        throw NumericalOverflow("Int::distinct");
      else
        cx[i].init(x[i],c[i]);
    switch (icl) {
    case ICL_BND:
      GECODE_ES_FAIL(home,Distinct::Bnd<OffsetView>::post(home,cx));
      break;
    case ICL_DOM:
      GECODE_ES_FAIL(home,Distinct::Dom<OffsetView>::post(home,cx));
      break;
    default:
      GECODE_ES_FAIL(home,Distinct::Val<OffsetView>::post(home,cx));
    }
  }

}

// STATISTICS: int-post

