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

  namespace DivMod {
    forceinline WordValue min_nonzero(WordView x) {
      if (x.lo() != 0)
        return x.lo();
      return x.hi() & (~x.hi()+1);
    }
    forceinline void range_hull(unsigned int width, WordValue minimum,
                                WordValue maximum, WordValue& lo,
                                WordValue& hi) {
      assert(minimum <= maximum);
      WordValue varying = minimum ^ maximum;
      if (varying == 0) {
        lo = hi = minimum;
        return;
      }
      unsigned int bits = 0;
      while (varying != 0) {
        varying >>= 1;
        bits++;
      }
      varying = width_mask(bits);
      lo = minimum & ~varying;
      hi = (lo | varying) & width_mask(width);
    }
    forceinline ModEvent narrow_range(Space& home, WordView x,
                                      WordValue minimum, WordValue maximum) {
      WordValue lo, hi;
      range_hull(x.width(),minimum,maximum,lo,hi);
      return x.narrow(home,lo,hi);
    }
    forceinline void union_cube(WordValue lo0, WordValue hi0,
                                WordValue lo1, WordValue hi1,
                                WordValue& lo, WordValue& hi) {
      lo = lo0 & lo1;
      hi = hi0 | hi1;
    }
    forceinline bool changed(WordView x, WordValue lo, WordValue hi) {
      return (x.lo() != lo) || (x.hi() != hi);
    }
  }

  forceinline Div::Div(Home home, WordView a, WordView b, WordView q)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,a,b,q) {}
  forceinline Div::Div(Space& home, Div& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline ExecStatus
  Div::narrow(Home home, WordView a, WordView b, WordView q) {
    const WordValue mask = a.mask();
    if (a == b) {
      if (a == q) {
        GECODE_ME_CHECK(q.eq(home,1));
        return ES_OK;
      }
      GECODE_ME_CHECK(q.narrow(home,1,mask));
      if (a.assigned()) {
        GECODE_ME_CHECK(q.eq(home,(a.val() == 0) ? mask : 1));
        return ES_OK;
      }
      if (q.assigned()) {
        if ((mask != 1) && (q.val() == mask)) {
          GECODE_ME_CHECK(a.eq(home,0));
          return ES_OK;
        }
        if (q.val() != 1)
          return ES_FAILED;
      }
      return (mask == 1) ? ES_OK : ES_FIX;
    }
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue qlo=q.lo(), qhi=q.hi();
      if (b.hi() == 0) {
        GECODE_ME_CHECK(q.eq(home,mask));
        return ES_OK;
      } else {
        const WordValue divisor_min = DivMod::min_nonzero(b);
        WordValue lo, hi;
        DivMod::range_hull(a.width(),a.lo()/b.hi(),
                          a.hi()/divisor_min,lo,hi);
        if (b.lo() == 0)
          DivMod::union_cube(lo,hi,mask,mask,lo,hi);
        GECODE_ME_CHECK(q.narrow(home,lo,hi));
      }
      if (b.assigned() && (b.val() != 0)) {
        const WordValue divisor=b.val(), maximum_quotient=mask/divisor;
        if (q.lo() > maximum_quotient)
          return ES_FAILED;
        GECODE_ME_CHECK(DivMod::narrow_range(home,q,0,maximum_quotient));
        const WordValue quotient_min=q.lo();
        const WordValue quotient_max=
          (q.hi()<maximum_quotient) ? q.hi() : maximum_quotient;
        const WordValue dividend_min=quotient_min*divisor;
        const WordValue room=mask-quotient_max*divisor;
        const WordValue dividend_max=quotient_max*divisor+
          ((room<divisor-1) ? room : divisor-1);
        GECODE_ME_CHECK(DivMod::narrow_range(
          home,a,dividend_min,dividend_max));
      }
      if (a.assigned() && q.assigned()) {
        const WordValue dividend=a.val(), quotient=q.val();
        if (quotient == mask) {
          if (dividend == mask)
            GECODE_ME_CHECK(DivMod::narrow_range(home,b,0,1));
          else
            GECODE_ME_CHECK(b.eq(home,0));
        } else if (quotient == 0) {
          if (dividend == mask)
            return ES_FAILED;
          GECODE_ME_CHECK(DivMod::narrow_range(home,b,dividend+1,mask));
        } else {
          const WordValue lower=dividend/(quotient+1)+1;
          const WordValue upper=dividend/quotient;
          if (lower > upper)
            return ES_FAILED;
          GECODE_ME_CHECK(DivMod::narrow_range(home,b,lower,upper));
        }
      }
      if (!DivMod::changed(a,alo,ahi) && !DivMod::changed(b,blo,bhi) &&
          !DivMod::changed(q,qlo,qhi))
        break;
    }
    if (a.assigned() && b.assigned() && q.assigned()) {
      const WordValue expected=(b.val()==0) ? mask : a.val()/b.val();
      return (q.val()==expected) ? ES_OK : ES_FAILED;
    }
    return ES_FIX;
  }
  forceinline ExecStatus Div::post(Home home, WordView a, WordView b,
                                   WordView q) {
    ExecStatus es=narrow(home,a,b,q);
    if (es == ES_FAILED) return ES_FAILED;
    if (es == ES_FIX) (void) new (home) Div(home,a,b,q);
    return ES_OK;
  }
  forceinline Actor* Div::copy(Space& home) {
    return new (home) Div(home,*this);
  }
  forceinline PropCost Div::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x0.width());
  }
  forceinline ExecStatus Div::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x0,x1,x2);
    if (es == ES_FAILED) return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

  forceinline Mod::Mod(Home home, WordView a, WordView b, WordView r)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,a,b,r) {}
  forceinline Mod::Mod(Space& home, Mod& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline ExecStatus
  Mod::narrow(Home home, WordView a, WordView b, WordView r) {
    const WordValue mask=a.mask();
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue rlo=r.lo(), rhi=r.hi();
      if (b.hi() == 0) {
        if (a == r)
          return ES_OK;
        const WordValue lo=a.lo()|r.lo(), hi=a.hi()&r.hi();
        GECODE_ME_CHECK(a.narrow(home,lo,hi));
        GECODE_ME_CHECK(r.narrow(home,lo,hi));
      } else {
        WordValue nonzero_hi=b.hi()-1;
        if (nonzero_hi > a.hi()) nonzero_hi=a.hi();
        WordValue lo, hi;
        DivMod::range_hull(a.width(),0,nonzero_hi,lo,hi);
        if (b.lo() == 0)
          DivMod::union_cube(lo,hi,a.lo(),a.hi(),lo,hi);
        GECODE_ME_CHECK(r.narrow(home,lo,hi));
      }
      if (b.assigned() && (b.val() != 0)) {
        const WordValue divisor=b.val();
        GECODE_ME_CHECK(DivMod::narrow_range(home,r,0,divisor-1));
        if ((divisor & (divisor-1)) == 0) {
          const WordValue low=divisor-1;
          GECODE_ME_CHECK(r.narrow(home,a.lo()&low,a.hi()&low));
          GECODE_ME_CHECK(a.narrow(home,a.lo()|(r.lo()&low),
                                   (a.hi()&~low)|(r.hi()&low)));
        }
      }
      if ((b.lo() != 0) && r.assigned()) {
        if (r.val() == mask) return ES_FAILED;
        GECODE_ME_CHECK(DivMod::narrow_range(home,b,r.val()+1,mask));
      }
      if (a.assigned() && b.assigned()) {
        const WordValue expected=(b.val()==0) ? a.val() : a.val()%b.val();
        GECODE_ME_CHECK(r.eq(home,expected));
      }
      if (!DivMod::changed(a,alo,ahi) && !DivMod::changed(b,blo,bhi) &&
          !DivMod::changed(r,rlo,rhi))
        break;
    }
    if (a.assigned() && b.assigned() && r.assigned()) {
      const WordValue expected=(b.val()==0) ? a.val() : a.val()%b.val();
      return (r.val()==expected) ? ES_OK : ES_FAILED;
    }
    return ES_FIX;
  }
  forceinline ExecStatus Mod::post(Home home, WordView a, WordView b,
                                   WordView r) {
    if (a == b) {
      GECODE_ME_CHECK(r.eq(home,0));
      return ES_OK;
    }
    ExecStatus es=narrow(home,a,b,r);
    if (es == ES_FAILED) return ES_FAILED;
    if (es == ES_FIX) (void) new (home) Mod(home,a,b,r);
    return ES_OK;
  }
  forceinline Actor* Mod::copy(Space& home) {
    return new (home) Mod(home,*this);
  }
  forceinline PropCost Mod::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,x0.width());
  }
  forceinline ExecStatus Mod::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x0,x1,x2);
    if (es == ES_FAILED) return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

  forceinline
  DivModBoth::DivModBoth(Home home, WordView dividend0, WordView divisor0,
                         WordView quotient0, WordView remainder0)
    : Propagator(home), dividend(dividend0), divisor(divisor0),
      quotient(quotient0), remainder(remainder0) {
    dividend.subscribe(home,*this,PC_WORD_BITS);
    divisor.subscribe(home,*this,PC_WORD_BITS);
    quotient.subscribe(home,*this,PC_WORD_BITS);
    remainder.subscribe(home,*this,PC_WORD_BITS);
  }
  forceinline DivModBoth::DivModBoth(Space& home, DivModBoth& p)
    : Propagator(home,p) {
    dividend.update(home,p.dividend);
    divisor.update(home,p.divisor);
    quotient.update(home,p.quotient);
    remainder.update(home,p.remainder);
  }

  forceinline ExecStatus
  DivModBoth::narrow(Home home, WordView a, WordView b, WordView q,
                     WordView r) {
    const WordValue mask=a.mask();
    for (;;) {
      const WordValue alo=a.lo(), ahi=a.hi(), blo=b.lo(), bhi=b.hi();
      const WordValue qlo=q.lo(), qhi=q.hi(), rlo=r.lo(), rhi=r.hi();

      if (b.hi() == 0) {
        GECODE_ME_CHECK(q.eq(home,mask));
        if (!(a == r)) {
          const WordValue lo=a.lo()|r.lo(), hi=a.hi()&r.hi();
          GECODE_ME_CHECK(a.narrow(home,lo,hi));
          GECODE_ME_CHECK(r.narrow(home,lo,hi));
        }
      } else {
        const WordValue divisor_min=DivMod::min_nonzero(b);
        WordValue lo, hi;
        DivMod::range_hull(a.width(),a.lo()/b.hi(),
                          a.hi()/divisor_min,lo,hi);
        if (b.lo() == 0)
          DivMod::union_cube(lo,hi,mask,mask,lo,hi);
        GECODE_ME_CHECK(q.narrow(home,lo,hi));

        WordValue nonzero_hi=b.hi()-1;
        if (nonzero_hi > a.hi()) nonzero_hi=a.hi();
        DivMod::range_hull(a.width(),0,nonzero_hi,lo,hi);
        if (b.lo() == 0)
          DivMod::union_cube(lo,hi,a.lo(),a.hi(),lo,hi);
        GECODE_ME_CHECK(r.narrow(home,lo,hi));
      }

      if (b.assigned() && (b.val() != 0)) {
        const WordValue d=b.val(), maximum_quotient=mask/d;
        if (q.lo() > maximum_quotient)
          return ES_FAILED;
        GECODE_ME_CHECK(DivMod::narrow_range(home,q,0,maximum_quotient));
        const WordValue quotient_min=q.lo();
        const WordValue quotient_max=
          (q.hi()<maximum_quotient) ? q.hi() : maximum_quotient;
        const WordValue dividend_min=quotient_min*d;
        const WordValue room=mask-quotient_max*d;
        const WordValue dividend_max=quotient_max*d+
          ((room<d-1) ? room : d-1);
        GECODE_ME_CHECK(DivMod::narrow_range(
          home,a,dividend_min,dividend_max));
        GECODE_ME_CHECK(DivMod::narrow_range(home,r,0,d-1));
        if ((d & (d-1)) == 0) {
          const WordValue low=d-1;
          GECODE_ME_CHECK(r.narrow(home,a.lo()&low,a.hi()&low));
          GECODE_ME_CHECK(a.narrow(home,a.lo()|(r.lo()&low),
                                   (a.hi()&~low)|(r.hi()&low)));
        }
        if (q.assigned() && r.assigned()) {
          const WordValue qv=q.val(), rv=r.val();
          if ((rv >= d) || (qv > mask/d))
            return ES_FAILED;
          const WordValue product=qv*d;
          if (product > mask-rv)
            return ES_FAILED;
          GECODE_ME_CHECK(a.eq(home,product+rv));
        }
      }

      if (a.assigned() && q.assigned()) {
        const WordValue av=a.val(), qv=q.val();
        if (qv == mask) {
          if (av == mask)
            GECODE_ME_CHECK(DivMod::narrow_range(home,b,0,1));
          else
            GECODE_ME_CHECK(b.eq(home,0));
        } else if (qv == 0) {
          if (av == mask)
            return ES_FAILED;
          GECODE_ME_CHECK(DivMod::narrow_range(home,b,av+1,mask));
        } else {
          const WordValue lower=av/(qv+1)+1;
          const WordValue upper=av/qv;
          if (lower > upper)
            return ES_FAILED;
          GECODE_ME_CHECK(DivMod::narrow_range(home,b,lower,upper));
        }
      }
      if ((b.lo() != 0) && r.assigned()) {
        if (r.val() == mask)
          return ES_FAILED;
        GECODE_ME_CHECK(DivMod::narrow_range(home,b,r.val()+1,mask));
      }
      if (a.assigned() && b.assigned()) {
        const WordValue expected_q=(b.val()==0) ? mask : a.val()/b.val();
        const WordValue expected_r=(b.val()==0) ? a.val() : a.val()%b.val();
        GECODE_ME_CHECK(q.eq(home,expected_q));
        GECODE_ME_CHECK(r.eq(home,expected_r));
      }

      if (!DivMod::changed(a,alo,ahi) && !DivMod::changed(b,blo,bhi) &&
          !DivMod::changed(q,qlo,qhi) && !DivMod::changed(r,rlo,rhi))
        break;
    }
    if (a.assigned() && b.assigned() && q.assigned() && r.assigned()) {
      const WordValue expected_q=(b.val()==0) ? mask : a.val()/b.val();
      const WordValue expected_r=(b.val()==0) ? a.val() : a.val()%b.val();
      return ((q.val()==expected_q) && (r.val()==expected_r)) ?
        ES_OK : ES_FAILED;
    }
    if ((b.hi() == 0) && (a == r) && q.assigned() && (q.val() == mask))
      return ES_OK;
    return ES_FIX;
  }

  forceinline ExecStatus
  DivModBoth::post(Home home, WordView a, WordView b, WordView q, WordView r) {
    if (a == b) {
      GECODE_ME_CHECK(r.eq(home,0));
      return Div::post(home,a,b,q);
    }
    ExecStatus es=narrow(home,a,b,q,r);
    if (es == ES_FAILED) return ES_FAILED;
    if (es == ES_FIX) (void) new (home) DivModBoth(home,a,b,q,r);
    return ES_OK;
  }
  forceinline Actor* DivModBoth::copy(Space& home) {
    return new (home) DivModBoth(home,*this);
  }
  forceinline PropCost
  DivModBoth::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,dividend.width());
  }
  forceinline void DivModBoth::reschedule(Space& home) {
    dividend.reschedule(home,*this,PC_WORD_BITS);
    divisor.reschedule(home,*this,PC_WORD_BITS);
    quotient.reschedule(home,*this,PC_WORD_BITS);
    remainder.reschedule(home,*this,PC_WORD_BITS);
  }
  forceinline size_t DivModBoth::dispose(Space& home) {
    dividend.cancel(home,*this,PC_WORD_BITS);
    divisor.cancel(home,*this,PC_WORD_BITS);
    quotient.cancel(home,*this,PC_WORD_BITS);
    remainder.cancel(home,*this,PC_WORD_BITS);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
  forceinline ExecStatus
  DivModBoth::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,dividend,divisor,quotient,remainder);
    if (es == ES_FAILED) return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

}}}

// STATISTICS: word-prop
