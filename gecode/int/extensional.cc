/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
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

#include "gecode/int/extensional.hh"

namespace Gecode {

  using namespace Int;

  void
  extensional(Space* home, const IntVarArgs& x, const Table& table, 
              ExtensionalAlgorithm ea, IntConLevel icl) {
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);

    // All variables in the correct domain
    for (int i = xv.size(); i--; ) {
      GECODE_ME_FAIL(home, xv[i].gq(home, table.tablei->min));
      GECODE_ME_FAIL(home, xv[i].lq(home, table.tablei->max));
    }

    switch (icl) {
    default:
      switch (ea) {
#ifdef GECODE_USE_ADVISORS
      case EA_INCREMENTAL:
        GECODE_ES_FAIL(home,(Extensional::Incremental<IntView>::post(home,xv,table)));
        break;
#endif
      default:
        GECODE_ES_FAIL(home,Extensional::Basic<IntView>::post(home,xv,table));
      break;
      }
    }
  }

  void
  extensional(Space* home, const IntArgs& c, const IntVarArgs& x, 
              const Table& table, ExtensionalAlgorithm ea, 
              IntConLevel icl) {
    if (c.size() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if (!table.tablei->finalized())
      table.tablei->finalize();
    if (table.tablei->arity != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if (home->failed()) return;
    ViewArray<OffsetView> cx(home,x.size());
    for (int i = c.size(); i--; )
      if ((c[i] < Limits::Int::int_min) || (c[i] > Limits::Int::int_max))
        throw NumericalOverflow("Int::extensional");
      else
        cx[i].init(x[i],c[i]);

    // All variables in the correct domain
    for (int i = cx.size(); i--; ) {
      GECODE_ME_FAIL(home, cx[i].gq(home, table.tablei->min));
      GECODE_ME_FAIL(home, cx[i].lq(home, table.tablei->max));
    }

    switch (icl) {
    default:
      GECODE_ES_FAIL(home,Extensional::Basic<OffsetView>::post(home,cx,table));
      break;
    }
  }

}

// STATISTICS: int-post

