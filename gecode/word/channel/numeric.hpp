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

namespace Gecode { namespace Word { namespace Channel {

  forceinline
  Numeric::Numeric(Home home, WordView x, Int::IntView y,
                   WordDomainType interpretation0)
    : MixBinaryPropagator<
        WordView,PC_WORD_DOM,Int::IntView,Int::PC_INT_BND>(home,x,y),
      interpretation(interpretation0) {}

  forceinline
  Numeric::Numeric(Space& home, Numeric& p)
    : MixBinaryPropagator<
        WordView,PC_WORD_DOM,Int::IntView,Int::PC_INT_BND>(home,p),
      interpretation(p.interpretation) {}

  forceinline WordValue
  Numeric::to_rank(int value, unsigned int width,
                   WordDomainType interpretation) {
    if (interpretation == WDT_UNSIGNED)
      return static_cast<WordValue>(value);
    const WordValue mask = width_mask(width);
    const WordValue encoded = (value < 0) ?
      mask - static_cast<WordValue>(-static_cast<long long>(value)) + 1U :
      static_cast<WordValue>(value);
    return encoded ^ sign_bit(width);
  }

  forceinline int
  Numeric::from_rank(WordValue value, unsigned int width,
                     WordDomainType interpretation) {
    if (interpretation == WDT_UNSIGNED)
      return static_cast<int>(value);
    const WordValue sign = sign_bit(width);
    if (value >= sign)
      return static_cast<int>(value-sign);
    return -static_cast<int>(sign-value);
  }

  forceinline ExecStatus
  Numeric::narrow(Home home, WordView x, Int::IntView y,
                  WordDomainType interpretation) {
    const unsigned int width = x.width();
    long long int representable_min;
    long long int representable_max;
    if (interpretation == WDT_UNSIGNED) {
      representable_min = 0;
      representable_max = (width >= 31U) ? Int::Limits::max :
        static_cast<long long int>(width_mask(width));
    } else if (width >= 32U) {
      representable_min = Int::Limits::min;
      representable_max = Int::Limits::max;
    } else {
      const long long int sign = static_cast<long long int>(sign_bit(width));
      representable_min = -sign;
      representable_max = sign-1;
    }
    GECODE_ME_CHECK(y.gq(home,representable_min));
    GECODE_ME_CHECK(y.lq(home,representable_max));

    WordValue lo = x.lo();
    WordValue hi = x.hi();
    WordValue minimum = to_rank(y.min(),width,interpretation);
    WordValue maximum = to_rank(y.max(),width,interpretation);
    if (x.bounded()) {
      minimum = std::max(minimum,x.rank_minimum());
      maximum = std::min(maximum,x.rank_maximum());
    }
    if ((minimum > maximum) ||
        !synchronize_domain(width,interpretation,lo,hi,minimum,maximum))
      return ES_FAILED;

    if (x.bounded()) {
      GECODE_ME_CHECK(static_cast<BoundedWordVarImp*>(x.varimp())->
                      narrow_domain(home,lo,hi,minimum,maximum));
    } else {
      GECODE_ME_CHECK(x.narrow(home,lo,hi));
    }
    GECODE_ME_CHECK(y.gq(home,from_rank(minimum,width,interpretation)));
    GECODE_ME_CHECK(y.lq(home,from_rank(maximum,width,interpretation)));
    return ES_OK;
  }

  forceinline ExecStatus
  Numeric::post(Home home, WordView x, Int::IntView y,
                WordDomainType interpretation) {
    GECODE_ES_CHECK(narrow(home,x,y,interpretation));
    if (x.assigned()) {
      GECODE_ME_CHECK(y.eq(home,from_rank(
        rank(interpretation,x.width(),x.val()),x.width(),interpretation)));
    } else if (y.assigned()) {
      const WordValue value = rank(interpretation,x.width(),
                                   to_rank(y.val(),x.width(),interpretation));
      GECODE_ME_CHECK(x.eq(home,value));
    }
    if (!x.assigned() || !y.assigned())
      (void) new (home) Numeric(home,x,y,interpretation);
    return ES_OK;
  }

  forceinline Actor*
  Numeric::copy(Space& home) {
    return new (home) Numeric(home,*this);
  }

  forceinline ExecStatus
  Numeric::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,interpretation));
    if (x0.assigned()) {
      const WordValue r = rank(interpretation,x0.width(),x0.val());
      GECODE_ME_CHECK(x1.eq(home,from_rank(r,x0.width(),interpretation)));
    } else if (x1.assigned()) {
      const WordValue r = to_rank(x1.val(),x0.width(),interpretation);
      GECODE_ME_CHECK(x0.eq(home,rank(interpretation,x0.width(),r)));
    }
    return (x0.assigned() && x1.assigned()) ?
      home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
