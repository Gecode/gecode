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

#ifndef GECODE_WORD_COUNT_HH
#define GECODE_WORD_COUNT_HH

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Count {

  /**
   * \brief Population-count propagator
   *
   * Requires \code #include <gecode/word/count.hh> \endcode
   * \ingroup FuncWordProp
   */
  class Popcount : public MixBinaryPropagator<
    WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND> {
  protected:
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>::x0;
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>::x1;
    Popcount(Home home, WordView x, Int::IntView count);
    Popcount(Space& home, Popcount& p);
    static unsigned int ones(WordValue bits);
    static ExecStatus prune(Home home, WordView x, Int::IntView count);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x, Int::IntView count);
  };

  /// Direction for a zero-count propagator
  enum ZeroDirection { ZD_LEADING, ZD_TRAILING };

  /**
   * \brief Leading- or trailing-zero-count propagator
   *
   * Requires \code #include <gecode/word/count.hh> \endcode
   * \ingroup FuncWordProp
   */
  template<ZeroDirection direction>
  class ZeroCount : public MixBinaryPropagator<
    WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND> {
  protected:
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>::x0;
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::IntView,Int::PC_INT_BND>::x1;
    ZeroCount(Home home, WordView x, Int::IntView count);
    ZeroCount(Space& home, ZeroCount& p);
    static unsigned int zeros(WordValue bits, unsigned int width);
    static WordValue region(unsigned int width, unsigned int size);
    static bool single_bit(WordValue bits);
    static ExecStatus prune(Home home, WordView x, Int::IntView count);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x, Int::IntView count);
  };

}}}

#include <gecode/word/count/popcount.hpp>
#include <gecode/word/count/zero-count.hpp>

#endif

// STATISTICS: word-prop
