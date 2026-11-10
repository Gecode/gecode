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
    assert(x.width() <= max_value_set_width);
    int supported[256];
    unsigned int count = 0;
    WordValueSet common = {{0, 0, 0, 0}};
    for (WordValue code = 0; code <= x.mask(); code++) {
      if (!x.in(code) || !y.in(static_cast<int>(code)))
        continue;
      supported[count++] = static_cast<int>(code);
      common = with_value(common, code);
    }
    if (count == 0)
      return ES_FAILED;

    Iter::Values::Array values(supported, count);
    GECODE_ME_CHECK(y.inter_v(home, values, false));
    GECODE_ME_CHECK(x.narrow_values(home, common));
    // x keeps exactly the common encodings with a value set, and otherwise
    // their hull, which contains every encoding left in y: a fixpoint.
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

  namespace {

    /** \brief Return the hull, in the order of \a target_type, of the
     * encodings in the rank interval of bounded \a source
     *
     * In every bounded order, the ranks below the sign bit name encodings
     * of one sign half and the ranks from it up those of the other. Each
     * piece of the interval split there is thus one encoding range within a
     * sign half, where rank_interval() is exact in every order.
     */
    WordRankInterval
    rank_hull(WordDomainType target_type, WordView source) {
      const unsigned int width = source.width();
      const WordValue s = sign_bit(width);
      const WordValue minimum = source.rank_minimum();
      const WordValue maximum = source.rank_maximum();
      const WordRankInterval pieces[] = {
        {minimum, std::min(maximum, s - 1)},
        {std::max(minimum, s), maximum}};
      WordRankInterval hull = {width_mask(width), 0};
      for (const WordRankInterval& piece : pieces) {
        if (piece.minimum > piece.maximum)
          continue;
        const WordEncodingRange encodings =
          encoding_range(source.domain_type(), width, piece);
        const WordRankInterval ranks =
          rank_interval(target_type, width, encodings.first, encodings.last);
        hull = {std::min(hull.minimum, ranks.minimum),
                std::max(hull.maximum, ranks.maximum)};
      }
      return hull;
    }

    /** \brief Narrow \a target to the cube of \a source and the hull of the
     * encodings in its rank interval
     */
    ModEvent
    narrow_to_source(Space& home, WordView target, WordView source) {
      const WordCube cube = {target.lo() | source.lo(),
                             target.hi() & source.hi()};
      const WordRankInterval proposed = source.bounded()
        ? rank_hull(target.domain_type(), source)
        : WordRankInterval{0, target.mask()};
      return narrow_word_domain(home, target, cube, proposed);
    }

  }

  Reinterpret::Reinterpret(Home home, WordView x, WordView y)
      : BinaryPropagator<WordView, PC_WORD_DOM>(home, x, y) {}

  Reinterpret::Reinterpret(Space& home, Reinterpret& p)
      : BinaryPropagator<WordView, PC_WORD_DOM>(home, p) {}

  ExecStatus
  Reinterpret::keep_common_values(Space& home, WordView x, WordView y) {
    const WordValueSet common = intersect_value_sets(x.values(), y.values());
    GECODE_ME_CHECK(x.narrow_values(home, common));
    GECODE_ME_CHECK(y.narrow_values(home, common));
    // A word with a value set keeps exactly the common values, and one
    // without keeps their hulls, which admit them all: a fixpoint.
    return ES_FIX;
  }

  ExecStatus
  Reinterpret::exchange_hulls(Space& home, WordView x, WordView y) {
    // Closure can move x beyond the hull of y. The exchange is cheap, so
    // repeat it to the common fixpoint, as Int::Rel::EqBnd does. Narrowing
    // y leaves it fixed for the current x, so an exchange that leaves x
    // unchanged ends there.
    for (;;) {
      GECODE_ME_CHECK(narrow_to_source(home, y, x));
      const ModEvent me = narrow_to_source(home, x, y);
      GECODE_ME_CHECK(me);
      if (!me_modified(me))
        return ES_FIX;
    }
  }

  ExecStatus
  Reinterpret::narrow(Space& home, WordView x, WordView y) {
    if (x.has_value_set() || y.has_value_set())
      return keep_common_values(home, x, y);
    return exchange_hulls(home, x, y);
  }

  ExecStatus
  Reinterpret::post(Home home, WordView x, WordView y) {
    GECODE_ES_CHECK(narrow(home, x, y));
    if (!x.assigned() || !y.assigned())
      (void)new (home) Reinterpret(home, x, y);
    return ES_OK;
  }

  Actor*
  Reinterpret::copy(Space& home) {
    return new (home) Reinterpret(home, *this);
  }

  ExecStatus
  Reinterpret::propagate(Space& home, const ModEventDelta&) {
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
    const Word::IntegerRankOrder order=
      Word::integer_rank_order(x.domain_type());
    switch (order) {
    case Word::IntegerRankOrder::NONE:
      break;
    case Word::IntegerRankOrder::UNSIGNED:
    case Word::IntegerRankOrder::SIGNED:
      if (x.domain_type() !=
          Word::Channel::bounded_domain_type(interpretation))
        throw Word::OutOfLimits("Word::channel");
      break;
    case Word::IntegerRankOrder::NON_INTEGER:
      throw Word::OutOfLimits("Word::channel");
    }
    GECODE_POST;
    switch (order) {
    case Word::IntegerRankOrder::NONE:
      if (interpretation == WI_UNSIGNED)
        GECODE_ES_FAIL((Word::Channel::Numeric<
          Word::WordView,Word::PC_WORD_BITS,WI_UNSIGNED>::post(
            home,Word::WordView(x),Int::IntView(y))));
      else
        GECODE_ES_FAIL((Word::Channel::Numeric<
          Word::WordView,Word::PC_WORD_BITS,WI_SIGNED>::post(
            home,Word::WordView(x),Int::IntView(y))));
      break;
    case Word::IntegerRankOrder::UNSIGNED:
      GECODE_ES_FAIL((Word::Channel::Numeric<
        Word::UnsignedWordView,Word::PC_WORD_BND,WI_UNSIGNED>::post(
          home,Word::UnsignedWordView(x),Int::IntView(y))));
      break;
    case Word::IntegerRankOrder::SIGNED:
      GECODE_ES_FAIL((Word::Channel::Numeric<
        Word::SignedWordView,Word::PC_WORD_BND,WI_SIGNED>::post(
          home,Word::SignedWordView(x),Int::IntView(y))));
      break;
    case Word::IntegerRankOrder::NON_INTEGER:
      GECODE_NEVER;
      break;
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

  WordVar
  reinterpret(Home home, WordVar x, WordDomainType domain_type) {
    const WordValueSetOption value_set =
      x.has_value_set() ? WVS_EXACT : WVS_NONE;
    WordVar y(home, x.width(), domain_type, value_set);
    if (home.failed())
      return y;
    PostInfo pi(home);
    if (Word::Channel::Reinterpret::post(home, Word::WordView(x),
                                         Word::WordView(y)) < ES_OK)
      home.fail();
    return y;
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
