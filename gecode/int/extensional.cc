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
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

