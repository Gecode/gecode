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
 *     Christian Schulte, 2017
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
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, bool pos,
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
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, bool pos,
              Reify r,
              IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    GECODE_POST;

    if (pos) {
      // Enforce invariant that there is at least one tuple...
      if (t.tuples() == 0) {
        if (x.size() != 0) {
          if (r.mode() != RM_PMI)
            GECODE_ME_FAIL(BoolView(r.var()).zero(home));
        } else {
          if (r.mode() != RM_IMP)
            GECODE_ME_FAIL(BoolView(r.var()).one(home));
        }
        return;
      }
      
      ViewArray<IntView> xv(home,x);
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((Extensional::postrecompact<IntView,BoolView,RM_EQV>
                        (home,xv,t,r.var())));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Extensional::postrecompact<IntView,BoolView,RM_IMP>
                        (home,xv,t,r.var())));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Extensional::postrecompact<IntView,BoolView,RM_PMI>
                        (home,xv,t,r.var())));
        break;
      default: throw UnknownReifyMode("Int::extensional");
      }
    } else {
      // Enforce invariant that there is at least one tuple...
      if (t.tuples() == 0) {
        if (x.size() == 0) {
          if (r.mode() != RM_PMI)
            GECODE_ME_FAIL(BoolView(r.var()).zero(home));
        } else {
          if (r.mode() != RM_IMP)
            GECODE_ME_FAIL(BoolView(r.var()).one(home));
        }
        return;
      }
      
      ViewArray<IntView> xv(home,x);
      NegBoolView n(r.var());
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((Extensional::postrecompact<IntView,NegBoolView,RM_EQV>
                        (home,xv,t,n)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Extensional::postrecompact<IntView,NegBoolView,RM_PMI>
                        (home,xv,t,n)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Extensional::postrecompact<IntView,NegBoolView,RM_IMP>
                        (home,xv,t,n)));
        break;
      default: throw UnknownReifyMode("Int::extensional");
      }
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

  void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, bool pos,
              Reify r,
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
      // Enforce invariant that there is at least one tuple...
      if (t.tuples() == 0) {
        if (x.size() != 0) {
          if (r.mode() != RM_PMI)
            GECODE_ME_FAIL(BoolView(r.var()).zero(home));
        } else {
          if (r.mode() != RM_IMP)
            GECODE_ME_FAIL(BoolView(r.var()).one(home));
        }
        return;
      }
      
      ViewArray<BoolView> xv(home,x);
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((Extensional::postrecompact<BoolView,BoolView,RM_EQV>
                        (home,xv,t,r.var())));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Extensional::postrecompact<BoolView,BoolView,RM_IMP>
                        (home,xv,t,r.var())));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Extensional::postrecompact<BoolView,BoolView,RM_PMI>
                        (home,xv,t,r.var())));
        break;
      default: throw UnknownReifyMode("Int::extensional");
      }
    } else {
      // Enforce invariant that there is at least one tuple...
      if (t.tuples() == 0) {
        if (x.size() == 0) {
          if (r.mode() != RM_PMI)
            GECODE_ME_FAIL(BoolView(r.var()).zero(home));
        } else {
          if (r.mode() != RM_IMP)
            GECODE_ME_FAIL(BoolView(r.var()).one(home));
        }
        return;
      }
      
      ViewArray<BoolView> xv(home,x);
      NegBoolView n(r.var());
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((Extensional::postrecompact<BoolView,NegBoolView,RM_EQV>
                        (home,xv,t,n)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Extensional::postrecompact<BoolView,NegBoolView,RM_PMI>
                        (home,xv,t,n)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Extensional::postrecompact<BoolView,NegBoolView,RM_IMP>
                        (home,xv,t,n)));
        break;
      default: throw UnknownReifyMode("Int::extensional");
      }
    }
  }

}

// STATISTICS: int-post
