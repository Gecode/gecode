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

namespace Gecode { namespace Word { namespace Arithmetic {

  namespace SignedDivModSupport {
    forceinline bool negative(WordValue value, WordValue sign) {
      return (value & sign) != 0;
    }
    forceinline WordValue negate(WordValue value, WordValue mask) {
      return (~value+1) & mask;
    }
    forceinline WordValue magnitude(WordValue value, WordValue sign,
                                    WordValue mask) {
      return negative(value,sign) ? negate(value,mask) : value;
    }
    template<SignedDivModOperation op>
    forceinline WordValue evaluate(WordValue a, WordValue b,
                                   WordValue sign, WordValue mask) {
      if (b == 0)
        return (op == SDO_DIV) ? (negative(a,sign) ? 1 : mask) : a;
      const bool an=negative(a,sign), bn=negative(b,sign);
      const WordValue am=magnitude(a,sign,mask);
      const WordValue bm=magnitude(b,sign,mask);
      if (op == SDO_DIV) {
        const WordValue q=am/bm;
        return (an != bn) ? negate(q,mask) : q;
      }
      WordValue r=am%bm;
      if (an)
        r=negate(r,mask);
      if ((op == SDO_MOD) && (r != 0) && (an != bn))
        r=(r+b) & mask;
      return r;
    }
    forceinline bool changed(WordView x, WordValue lo, WordValue hi) {
      return (x.lo() != lo) || (x.hi() != hi);
    }
    forceinline ExecStatus equal(Home home, WordView x, WordView y) {
      const WordValue lo=x.lo()|y.lo(), hi=x.hi()&y.hi();
      GECODE_ME_CHECK(x.narrow(home,lo,hi));
      GECODE_ME_CHECK(y.narrow(home,lo,hi));
      return (x.assigned() && y.assigned()) ? ES_OK : ES_FIX;
    }
  }

  template<SignedDivModOperation op>
  forceinline
  SignedDivMod<op>::SignedDivMod(Home home, WordView a, WordView b,
                                 WordView r)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,a,b,r) {}

  template<SignedDivModOperation op>
  forceinline
  SignedDivMod<op>::SignedDivMod(Space& home, SignedDivMod<op>& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  template<SignedDivModOperation op>
  forceinline ExecStatus
  SignedDivMod<op>::narrow(Home home, WordView a, WordView b, WordView r) {
    const WordValue mask=a.mask();
    const WordValue sign=WordValue(1) << (a.width()-1);
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue rlo=r.lo(), rhi=r.hi();

      if (b.assigned()) {
        if (b.val() == 0) {
          if (op == SDO_DIV) {
            if ((a.lo()&sign) != 0) {
              GECODE_ME_CHECK(r.eq(home,1));
              return ES_OK;
            }
            if ((a.hi()&sign) == 0) {
              GECODE_ME_CHECK(r.eq(home,mask));
              return ES_OK;
            }
            GECODE_ME_CHECK(r.narrow(home,1,mask));
            if (mask == 1)
              return ES_OK;
            if (r.assigned()) {
              if (r.val() == 1)
                GECODE_ME_CHECK(a.narrow(home,a.lo()|sign,a.hi()));
              else if (r.val() == mask)
                GECODE_ME_CHECK(a.narrow(home,a.lo(),a.hi()&~sign));
              else
                return ES_FAILED;
            }
          }
          else {
            if (a == r)
              return ES_OK;
            return SignedDivModSupport::equal(home,a,r);
          }
        }
        if (b.val() == 1) {
          if (op == SDO_DIV) {
            if (a == r)
              return ES_OK;
            return SignedDivModSupport::equal(home,a,r);
          }
          GECODE_ME_CHECK(r.eq(home,0));
          return ES_OK;
        }
        if ((b.val() == mask) && (op != SDO_DIV)) {
          GECODE_ME_CHECK(r.eq(home,0));
          return ES_OK;
        }
      }

      if (a.assigned() && b.assigned()) {
        GECODE_ME_CHECK(r.eq(home,SignedDivModSupport::evaluate<op>(
          a.val(),b.val(),sign,mask)));
        return ES_OK;
      }

      if (a.assigned() && (a.val() == 0) &&
          ((op != SDO_DIV) || (b.lo() != 0))) {
        GECODE_ME_CHECK(r.eq(home,0));
        return ES_OK;
      }

      if (op == SDO_DIV) {
        const bool an=(a.lo()&sign) != 0, ap=(a.hi()&sign) == 0;
        const bool bn=(b.lo()&sign) != 0, bp=(b.hi()&sign) == 0;
        if ((b.lo() != 0) && ((an && bp) || (ap && bn)) &&
            (r.lo() != 0))
          GECODE_ME_CHECK(r.narrow(home,r.lo()|sign,r.hi()));
      } else {
        WordView s = (op == SDO_REM) ? a : b;
        const bool nonzero_source=(op == SDO_REM) || (b.lo() != 0);
        if (nonzero_source && ((s.hi()&sign) == 0))
          GECODE_ME_CHECK(r.narrow(home,r.lo(),r.hi()&~sign));
        else if (((s.lo()&sign) != 0) && (r.lo() != 0))
          GECODE_ME_CHECK(r.narrow(home,r.lo()|sign,r.hi()));
      }

      if (!SignedDivModSupport::changed(a,alo,ahi) &&
          !SignedDivModSupport::changed(b,blo,bhi) &&
          !SignedDivModSupport::changed(r,rlo,rhi))
        break;
    }
    return ES_FIX;
  }

  template<SignedDivModOperation op>
  forceinline ExecStatus
  SignedDivMod<op>::post(Home home, WordView a, WordView b, WordView r) {
    if (b.assigned() && (b.val() == a.mask()) && (op == SDO_DIV))
      return Neg::post(home,a,r);
    ExecStatus es=narrow(home,a,b,r);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) SignedDivMod<op>(home,a,b,r);
    return ES_OK;
  }

  template<SignedDivModOperation op>
  forceinline Actor*
  SignedDivMod<op>::copy(Space& home) {
    return new (home) SignedDivMod<op>(home,*this);
  }

  template<SignedDivModOperation op>
  forceinline PropCost
  SignedDivMod<op>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  template<SignedDivModOperation op>
  forceinline ExecStatus
  SignedDivMod<op>::propagate(Space& home, const ModEventDelta&) {
    if (x1.assigned() && (x1.val() == x0.mask()) && (op == SDO_DIV)) {
      GECODE_REWRITE(*this,(Neg::post(home(*this),x0,x2)));
    }
    ExecStatus es=narrow(home,x0,x1,x2);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
