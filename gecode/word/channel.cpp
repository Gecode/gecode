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
 *
 */

#include <gecode/word/channel.hh>

namespace Gecode { namespace Word { namespace Channel {

  Encoding::Encoding(Home home, WordView x, Int::IntView y)
      : MixBinaryPropagator<WordView, PC_WORD_DOM, Int::IntView,
                            Int::PC_INT_DOM>(home, x, y) {}

  Encoding::Encoding(Space& home, Encoding& p)
      : MixBinaryPropagator<WordView, PC_WORD_DOM, Int::IntView,
                            Int::PC_INT_DOM>(home, p) {}

  ExecStatus
  Encoding::narrow(Home home, WordView x, Int::IntView y) {
    assert(x.width() <= 8U);
    int supported[256];
    unsigned int count = 0;
    WordValue lo = x.mask(), hi = 0;
    WordValue minimum = x.mask(), maximum = 0;
    for (WordValue code = 0; code <= x.mask(); code++) {
      if (!x.in(code) || !y.in(static_cast<int>(code)))
        continue;
      supported[count++] = static_cast<int>(code);
      lo &= code;
      hi |= code;
      if (x.bounded()) {
        const WordValue r = rank(x.domain_type(), x.width(), code);
        minimum = std::min(minimum, r);
        maximum = std::max(maximum, r);
      }
    }
    if (count == 0)
      return ES_FAILED;

    Iter::Values::Array values(supported, count);
    GECODE_ME_CHECK(y.inter_v(home, values, false));
    GECODE_ME_CHECK(x.narrow(home, lo, hi));
    if (x.bounded())
      GECODE_ME_CHECK(x.narrow_rank_range(home, minimum, maximum));
    // Every surviving encoding remains in both Word hulls: this is a fixpoint.
    return ES_OK;
  }

  ExecStatus
  Encoding::post(Home home, WordView x, Int::IntView y) {
    GECODE_ES_CHECK(narrow(home, x, y));
    if (!x.assigned() || !y.assigned())
      (void)new (home) Encoding(home, x, y);
    return ES_OK;
  }

  Actor*
  Encoding::copy(Space& home) {
    return new (home) Encoding(home, *this);
  }

  ExecStatus
  Encoding::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home, x0, x1));
    return (x0.assigned() && x1.assigned()) ? home.ES_SUBSUMED(*this)
                                            : ES_FIX;
  }

}}}

namespace Gecode {

  void
  channel(Home home, WordVar x, unsigned int bit, BoolVar b) {
    if (bit >= x.width())
      throw Word::OutOfLimits("Word::channel");
    GECODE_POST;
    const WordValue bit_mask = WordValue(1) << bit;
    GECODE_ES_FAIL(Word::Channel::Bit::post(
      home,Word::WordView(x),bit_mask,Int::BoolView(b)));
  }

  void
  channel(Home home, WordVar x, unsigned int bit, int value) {
    if (bit >= x.width())
      throw Word::OutOfLimits("Word::channel");
    if ((value < 0) || (value > 1))
      throw Int::NotZeroOne("Word::channel");
    GECODE_POST;
    const WordValue bit_mask = WordValue(1) << bit;
    Word::WordView xv(x);
    if (value == 0)
      GECODE_ME_FAIL(xv.narrow(home,xv.lo(),xv.hi() & ~bit_mask));
    else
      GECODE_ME_FAIL(xv.narrow(home,xv.lo() | bit_mask,xv.hi()));
  }

  void
  channel(Home home, WordVar x, IntVar y,
          WordInterpretation interpretation) {
    if ((x.domain_type() != WDT_CUBE) &&
        (x.domain_type() !=
         Word::Channel::bounded_domain_type(interpretation)))
      throw Word::OutOfLimits("Word::channel");
    GECODE_POST;
    if (x.domain_type() == WDT_CUBE) {
      if (interpretation == WI_UNSIGNED)
        GECODE_ES_FAIL((Word::Channel::Numeric<
          Word::WordView,Word::PC_WORD_BITS,WI_UNSIGNED>::post(
            home,Word::WordView(x),Int::IntView(y))));
      else
        GECODE_ES_FAIL((Word::Channel::Numeric<
          Word::WordView,Word::PC_WORD_BITS,WI_SIGNED>::post(
            home,Word::WordView(x),Int::IntView(y))));
    } else if (interpretation == WI_UNSIGNED) {
      GECODE_ES_FAIL((Word::Channel::Numeric<
        Word::UnsignedWordView,Word::PC_WORD_BND,WI_UNSIGNED>::post(
          home,Word::UnsignedWordView(x),Int::IntView(y))));
    } else {
      GECODE_ES_FAIL((Word::Channel::Numeric<
        Word::SignedWordView,Word::PC_WORD_BND,WI_SIGNED>::post(
          home,Word::SignedWordView(x),Int::IntView(y))));
    }
  }

  void
  channel_encoding(Home home, WordVar x, IntVar y) {
    if (x.width() > 8U)
      throw Word::OutOfLimits("Word::channel_encoding");
    GECODE_POST;
    GECODE_ES_FAIL(Word::Channel::Encoding::post(home, Word::WordView(x),
                                                 Int::IntView(y)));
  }

  void
  reduce_and(Home home, WordVar x, BoolVar b) {
    GECODE_POST;
    GECODE_ES_FAIL((Word::Channel::Reduction<Word::Channel::ReductionType::AND>
                    ::post(home,Word::WordView(x),Int::BoolView(b))));
  }

  void
  reduce_or(Home home, WordVar x, BoolVar b) {
    GECODE_POST;
    GECODE_ES_FAIL((Word::Channel::Reduction<Word::Channel::ReductionType::OR>
                    ::post(home,Word::WordView(x),Int::BoolView(b))));
  }

  void
  reduce_xor(Home home, WordVar x, BoolVar b) {
    GECODE_POST;
    GECODE_ES_FAIL((Word::Channel::Reduction<Word::Channel::ReductionType::XOR>
                    ::post(home,Word::WordView(x),Int::BoolView(b))));
  }

}

// STATISTICS: word-post
