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
  class Bit : public Propagator {
  protected:
    /// Word view
    WordView x;
    /// Boolean view
    Int::BoolView b;
    /// Mask for the channelled bit
    WordValue bit_mask;
    /// Advisor for changes to the word view
    Council<ViewAdvisor<WordView> > c;
    /// Constructor for cloning \a p
    Bit(Space& home, Bit& p);
    /// Constructor for creation
    Bit(Home home, WordView x, WordValue bit_mask, Int::BoolView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Cost function
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    /// Schedule propagator after it has been enabled
    virtual void reschedule(Space& home);
    /// Give advice for a word-view change
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Post propagator
    static ExecStatus post(Home home, WordView x, WordValue bit_mask,
                           Int::BoolView b);
  };

  /// Word-to-Boolean reduction operation
  enum ReductionType {
    RT_AND, ///< Conjunction of all significant bits
    RT_OR,  ///< Disjunction of all significant bits
    RT_XOR  ///< Exclusive-or of all significant bits
  };

  /**
   * \brief Reduce all significant bits of a word to a Boolean variable
   *
   * Requires \code #include <gecode/word/channel.hh> \endcode
   * \ingroup FuncWordProp
   */
  template<ReductionType rt>
  class Reduction : public MixBinaryPropagator<
    WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL> {
  protected:
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::x0;
    using MixBinaryPropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::x1;
    /// Constructor for cloning \a p
    Reduction(Space& home, Reduction& p);
    /// Constructor for creation
    Reduction(Home home, WordView x, Int::BoolView b);
    /// Test whether \a bits contains exactly one bit
    static bool single_bit(WordValue bits);
    /// Return the parity of \a bits
    static bool parity(WordValue bits);
    /// Propagate the reduction, returning ES_OK when it is decided
    static ExecStatus prune(Home home, WordView x, Int::BoolView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator
    static ExecStatus post(Home home, WordView x, Int::BoolView b);
  };

}}}

#include <gecode/word/channel/bit.hpp>
#include <gecode/word/channel/reduction.hpp>

#endif

// STATISTICS: word-prop
