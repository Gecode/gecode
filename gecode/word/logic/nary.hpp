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
    forceinline bool aliases(WordView x, WordView y) {
      return x == y;
    }
    template<class VY>
    forceinline bool aliases(WordView, VY) {
      return false;
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

    // XOR normally reaches closure in one pass. Avoid scratch allocation and
    // publish only views for which that pass has computed a tighter mask.
    if (op == NO_XOR) {
      for (;;) {
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
        const WordValue result=parity&known;
        const WordValue ylo=y.lo()|result;
        const WordValue yhi=y.hi()&(result|unknown);
        if ((ylo&~yhi) != 0)
          return ES_FAILED;
        bool changed=NarySupport::changed(y,ylo,yhi);
        if (changed)
          GECODE_ME_CHECK(y.narrow(home,ylo,yhi));

        const WordValue exact_one=once&~twice&mask;
        const WordValue y_known=~y.unknown()&mask;
        const WordValue required=y.lo()^parity;
        for (int i=0; i<x.size(); i++) {
          const WordValue old_lo=x[i].lo(), old_hi=x[i].hi();
          const WordValue force=x[i].unknown()&exact_one&y_known;
          const WordValue lo=old_lo|(force&required);
          const WordValue hi=old_hi&~(force&~required);
          if ((lo&~hi) != 0)
            return ES_FAILED;
          if ((lo != old_lo) || (hi != old_hi)) {
            changed=true;
            GECODE_ME_CHECK(x[i].narrow(home,lo,hi));
          }
        }
        if (!changed)
          break;
      }
      bool assigned=y.assigned();
      for (int i=0; i<x.size(); i++)
        assigned &= x[i].assigned();
      return assigned ? ES_OK : ES_FIX;
    }

    const int stack_size=32;
    WordValue stack[2*stack_size];
    Region region;
    WordValue* xlo = x.size() <= stack_size ? stack :
      region.alloc<WordValue>(2*x.size());
    WordValue* xhi=xlo+x.size();
    bool bounded=y.bounded();
    for (int i=0; i<x.size(); i++)
      bounded |= x[i].bounded();

    for (;;) {
      for (int i=0; i<x.size(); i++) {
        xlo[i]=x[i].lo();
        xhi[i]=x[i].hi();
      }
      WordValue ylo=y.lo(), yhi=y.hi();

      for (;;) {
        bool changed=false;

        if (op == NO_AND) {
          WordValue all_lo=constant, all_hi=constant;
          WordValue once=~constant&mask, twice=0;
          for (int i=0; i<x.size(); i++) {
            all_lo &= xlo[i];
            all_hi &= xhi[i];
            const WordValue not_one=~xlo[i]&mask;
            twice |= once&not_one;
            once |= not_one;
          }
          const WordValue next_ylo=ylo|all_lo;
          const WordValue next_yhi=yhi&all_hi;
          if ((next_ylo&~next_yhi) != 0)
            return ES_FAILED;
          changed |= (next_ylo != ylo) || (next_yhi != yhi);
          ylo=next_ylo; yhi=next_yhi;
          const WordValue exact_one=once&~twice&mask;
          const WordValue known_zero=~yhi&mask;
          for (int i=0; i<x.size(); i++) {
            const WordValue lo=xlo[i]|ylo;
            const WordValue hi=xhi[i]&~(known_zero&exact_one&~xlo[i]);
            if ((lo&~hi) != 0)
              return ES_FAILED;
            changed |= (lo != xlo[i]) || (hi != xhi[i]);
            xlo[i]=lo; xhi[i]=hi;
          }
        } else {
          WordValue all_lo=constant, all_hi=constant;
          WordValue once=constant, twice=0;
          for (int i=0; i<x.size(); i++) {
            all_lo |= xlo[i];
            all_hi |= xhi[i];
            const WordValue may_one=xhi[i];
            twice |= once&may_one;
            once |= may_one;
          }
          const WordValue next_ylo=ylo|all_lo;
          const WordValue next_yhi=yhi&all_hi;
          if ((next_ylo&~next_yhi) != 0)
            return ES_FAILED;
          changed |= (next_ylo != ylo) || (next_yhi != yhi);
          ylo=next_ylo; yhi=next_yhi;
          const WordValue exact_one=once&~twice&mask;
          for (int i=0; i<x.size(); i++) {
            const WordValue lo=xlo[i]|(ylo&exact_one&xhi[i]);
            const WordValue hi=xhi[i]&yhi;
            if ((lo&~hi) != 0)
              return ES_FAILED;
            changed |= (lo != xlo[i]) || (hi != xhi[i]);
            xlo[i]=lo; xhi[i]=hi;
          }
        }

        for (int i=0; i<x.size(); i++)
          if (NarySupport::aliases(x[i],y)) {
            const WordValue lo=xlo[i]|ylo;
            const WordValue hi=xhi[i]&yhi;
            if ((lo&~hi) != 0)
              return ES_FAILED;
            changed |= (lo != xlo[i]) || (hi != xhi[i]) ||
              (lo != ylo) || (hi != yhi);
            xlo[i]=ylo=lo;
            xhi[i]=yhi=hi;
          }

        if (!changed)
          break;
      }

      if (NarySupport::changed(y,ylo,yhi))
        GECODE_ME_CHECK(y.narrow(home,ylo,yhi));
      for (int i=0; i<x.size(); i++)
        if (NarySupport::changed(x[i],xlo[i],xhi[i]))
          GECODE_ME_CHECK(x[i].narrow(home,xlo[i],xhi[i]));

      bool synchronized=(ylo == y.lo()) && (yhi == y.hi());
      for (int i=0; i<x.size(); i++)
        synchronized &= (xlo[i] == x[i].lo()) && (xhi[i] == x[i].hi());
      if (!bounded || synchronized) {
        bool assigned=y.assigned();
        for (int i=0; i<x.size(); i++)
          assigned &= x[i].assigned();
        return assigned ? ES_OK : ES_FIX;
      }
    }
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
