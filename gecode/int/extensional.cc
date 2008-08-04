/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
 *     Christian Schulte, 2004
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

  void
  extensional(Space* home, const IntVarArgs& x, DFA dfa, 
              IntConLevel, PropKind) {
    using namespace Int;
    if (x.same(home))
      throw ArgumentSame("Int::extensional");
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(home,(Extensional::LayeredGraph<IntView>
                         ::post(home,xv,dfa)));
  }

  void
  extensional(Space* home, const BoolVarArgs& x, DFA dfa, 
              IntConLevel, PropKind) {
    using namespace Int;
    if (x.same(home))
      throw ArgumentSame("Int::extensional");
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(home,(Extensional::LayeredGraph<BoolView>
                         ::post(home,xv,dfa)));
  }

  void
  extensional(Space* home, const IntVarArgs& x, const TupleSet& t, 
              IntConLevel, PropKind pk) {
    using namespace Int;
    if (!t.finalized())
      const_cast<TupleSet&>(t).finalize();
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if (home->failed()) return;

    // Construct view array
    ViewArray<IntView> xv(home,x);

    // All variables in the correct domain
    for (int i = xv.size(); i--; ) {
      GECODE_ME_FAIL(home, xv[i].gq(home, t.min()));
      GECODE_ME_FAIL(home, xv[i].lq(home, t.max()));
    }

    switch (pk) {
    case PK_SPEED:
      GECODE_ES_FAIL(home,(Extensional::Incremental<IntView>::post(home,xv,t)));
      break;
    default:
      GECODE_ES_FAIL(home,(Extensional::Basic<IntView>::post(home,xv,t)));
      break;
    }
  }

  void
  extensional(Space* home, const BoolVarArgs& x, const TupleSet& t, 
              IntConLevel, PropKind pk) {
    using namespace Int;
    if (!t.finalized())
      const_cast<TupleSet&>(t).finalize();
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if (home->failed()) return;

    // Construct view array
    ViewArray<BoolView> xv(home,x);

    // All variables in the correct domain
    for (int i = xv.size(); i--; ) {
      GECODE_ME_FAIL(home, xv[i].gq(home, t.min()));
      GECODE_ME_FAIL(home, xv[i].lq(home, t.max()));
    }

    switch (pk) {
    case PK_SPEED:
      GECODE_ES_FAIL(home,(Extensional::Incremental<BoolView>::post(home,xv,t)));
      break;
    default:
      GECODE_ES_FAIL(home,(Extensional::Basic<BoolView>::post(home,xv,t)));
      break;
    }
  }

  GECODE_REGISTER1(Int::Extensional::LayeredGraph<Int::IntView>);
  GECODE_REGISTER1(Int::Extensional::LayeredGraph<Int::BoolView>);

  GECODE_REGISTER1(Int::Extensional::Basic<Int::IntView>);
  GECODE_REGISTER1(Int::Extensional::Incremental<Int::IntView>);

  GECODE_REGISTER1(Int::Extensional::Basic<Int::BoolView>);
  GECODE_REGISTER1(Int::Extensional::Incremental<Int::BoolView>);

}

// STATISTICS: int-post

