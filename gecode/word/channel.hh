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

#ifndef GECODE_WORD_CHANNEL_HH
#define GECODE_WORD_CHANNEL_HH

#include <gecode/word.hh>

/**
 * \namespace Gecode::Word::Channel
 * \brief %Channel propagators
 */

namespace Gecode { namespace Word { namespace Channel {

  /**
   * \brief Channel a word bit to a Boolean variable
   *
   * Requires \code #include <gecode/word/channel.hh> \endcode
   * \ingroup FuncWordProp
   */
  class Bit : public MixBinaryPropagator<
    WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL> {
  protected:
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::x0;
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::x1;
    /// Mask for the channelled bit
    WordValue bit_mask;
    /// Constructor for cloning \a p
    Bit(Space& home, Bit& p);
    /// Constructor for creation
    Bit(Home home, WordView x, WordValue bit_mask, Int::BoolView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator
    static ExecStatus post(Home home, WordView x, WordValue bit_mask,
                           Int::BoolView b);
  };

}}}

#include <gecode/word/channel/bit.hpp>

#endif

// STATISTICS: word-prop
