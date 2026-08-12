/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

namespace Gecode { namespace Word { namespace Logic {

  namespace NarySupport {
    forceinline bool changed(WordView x, WordValue lo, WordValue hi) {
      return (x.lo() != lo) || (x.hi() != hi);
    }
    template<class VY>
    forceinline bool changed(VY y, WordValue lo, WordValue hi) {
      return (y.lo() != lo) || (y.hi() != hi);
    }
  }

  template<NaryOperation op, class VY>
  forceinline
  Nary<op,VY>::Nary(Home home, ViewArray<WordView>& x0, VY y0,
                    WordValue c)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,VY,PC_WORD_BITS>(home,x0,y0), constant(c) {}

  template<NaryOperation op, class VY>
  forceinline
  Nary<op,VY>::Nary(Space& home, Nary<op,VY>& p)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,VY,PC_WORD_BITS>(home,p),
      constant(p.constant) {}

  template<NaryOperation op, class VY>
  forceinline ExecStatus
  Nary<op,VY>::narrow(Home home, ViewArray<WordView>& x, VY y,
                      WordValue constant) {
    const WordValue mask=y.mask();
    if (op == NO_AND) {
      for (int i=0; i<x.size(); i++)
        if (x[i].assigned() && (x[i].val() == 0)) {
          GECODE_ME_CHECK(y.eq(home,0));
          return ES_OK;
        }
    } else if (op == NO_OR) {
      for (int i=0; i<x.size(); i++)
        if (x[i].assigned() && (x[i].val() == mask)) {
          GECODE_ME_CHECK(y.eq(home,mask));
          return ES_OK;
        }
    }
    for (;;) {
      const WordValue ylo=y.lo(), yhi=y.hi();
      bool input_changed=false;

      if (op == NO_AND) {
        WordValue all_lo=constant, all_hi=constant;
        WordValue once=~constant&mask, twice=0;
        for (int i=0; i<x.size(); i++) {
          all_lo &= x[i].lo();
          all_hi &= x[i].hi();
          const WordValue not_one=~x[i].lo()&mask;
          twice |= once&not_one;
          once |= not_one;
        }
        GECODE_ME_CHECK(y.narrow(home,all_lo,all_hi));
        const WordValue exact_one=once&~twice&mask;
        const WordValue known_zero=~y.hi()&mask;
        for (int i=0; i<x.size(); i++) {
          const WordValue lo=x[i].lo(), hi=x[i].hi();
          GECODE_ME_CHECK(x[i].narrow(
            home,lo|y.lo(),hi&~(known_zero&exact_one&~lo)));
          input_changed |= NarySupport::changed(x[i],lo,hi);
        }
      } else if (op == NO_OR) {
        WordValue all_lo=constant, all_hi=constant;
        WordValue once=constant, twice=0;
        for (int i=0; i<x.size(); i++) {
          all_lo |= x[i].lo();
          all_hi |= x[i].hi();
          const WordValue may_one=x[i].hi();
          twice |= once&may_one;
          once |= may_one;
        }
        GECODE_ME_CHECK(y.narrow(home,all_lo,all_hi));
        const WordValue exact_one=once&~twice&mask;
        for (int i=0; i<x.size(); i++) {
          const WordValue lo=x[i].lo(), hi=x[i].hi();
          GECODE_ME_CHECK(x[i].narrow(
            home,lo|(y.lo()&exact_one&hi),hi&y.hi()));
          input_changed |= NarySupport::changed(x[i],lo,hi);
        }
      } else {
        WordValue parity=constant, unknown=0;
        WordValue once=0, twice=0;
        for (int i=0; i<x.size(); i++) {
          parity ^= x[i].lo();
          const WordValue u=x[i].unknown();
          unknown |= u;
          twice |= once&u;
          once |= u;
        }
        const WordValue known=~unknown&mask;
        GECODE_ME_CHECK(y.narrow(home,parity&known,
                                 (parity&known)|unknown));
        const WordValue exact_one=once&~twice&mask;
        const WordValue y_known=~y.unknown()&mask;
        const WordValue required=y.lo()^parity;
        for (int i=0; i<x.size(); i++) {
          const WordValue lo=x[i].lo(), hi=x[i].hi();
          const WordValue force=x[i].unknown()&exact_one&y_known;
          GECODE_ME_CHECK(x[i].narrow(
            home,lo|(force&required),hi&~(force&~required)));
          input_changed |= NarySupport::changed(x[i],lo,hi);
        }
      }

      if (!input_changed && !NarySupport::changed(y,ylo,yhi))
        break;
    }

    bool assigned=y.assigned();
    for (int i=0; i<x.size(); i++)
      assigned &= x[i].assigned();
    return assigned ? ES_OK : ES_FIX;
  }

  template<NaryOperation op, class VY>
  forceinline ExecStatus
  Nary<op,VY>::post(Home home, ViewArray<WordView>& x, VY y,
                    WordValue constant) {
    if (((op == NO_AND) && (constant == 0)) ||
        ((op == NO_OR) && (constant == y.mask()))) {
      GECODE_ME_CHECK(y.eq(home,constant));
      return ES_OK;
    }
    ExecStatus es=narrow(home,x,y,constant);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) Nary<op,VY>(home,x,y,constant);
    return ES_OK;
  }

  template<NaryOperation op, class VY>
  forceinline Actor*
  Nary<op,VY>::copy(Space& home) {
    return new (home) Nary<op,VY>(home,*this);
  }

  template<NaryOperation op, class VY>
  forceinline PropCost
  Nary<op,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  template<NaryOperation op, class VY>
  forceinline ExecStatus
  Nary<op,VY>::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x,y,constant);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
