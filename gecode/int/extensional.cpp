/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Lagerkvist, 2007
 *     Christian Schulte, 2004
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

#include <gecode/int/extensional.hh>

namespace Gecode {

  void
  extensional(Home home, const IntVarArgs& x, DFA dfa,
              IntPropLevel) {
    using namespace Int;
    if (same(x))
      throw ArgumentSame("Int::extensional");
    GECODE_POST;
    GECODE_ES_FAIL(Extensional::post_lgp(home,x,dfa));
  }

  void
  extensional(Home home, const BoolVarArgs& x, DFA dfa,
              IntPropLevel) {
    using namespace Int;
    if (same(x))
      throw ArgumentSame("Int::extensional");
    GECODE_POST;
    GECODE_ES_FAIL(Extensional::post_lgp(home,x,dfa));
  }

  void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t,
              bool pos,
              IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    GECODE_POST;

    if (pos) {
      if (t.tuples() == 0) {
        if (x.size() != 0)
          home.fail();
        return;
      }
      
      // Construct view array
      ViewArray<IntView> xv(home,x);
      GECODE_ES_FAIL((Extensional::postposcompact<IntView>(home,xv,t)));
    } else {
      if (t.tuples() == 0)
        return;
      
      // Construct view array
      ViewArray<IntView> xv(home,x);
      GECODE_ES_FAIL((Extensional::postnegcompact<IntView>(home,xv,t)));
      
    }
  }

  void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, bool pos,
              IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if ((t.min() < 0) || (t.max() > 1))
      throw NotZeroOne("Int::extensional");
    GECODE_POST;

    if (pos) {
      if (t.tuples() == 0) {
        if (x.size() != 0)
          home.fail();
        return;
      }
      
      // Construct view array
      ViewArray<BoolView> xv(home,x);
      GECODE_ES_FAIL((Extensional::postposcompact<BoolView>(home,xv,t)));
    } else {
      if (t.tuples() == 0)
        return;
      
      // Construct view array
      ViewArray<BoolView> xv(home,x);
      GECODE_ES_FAIL((Extensional::postnegcompact<BoolView>(home,xv,t)));
      
    }
  }

}

// STATISTICS: int-post
