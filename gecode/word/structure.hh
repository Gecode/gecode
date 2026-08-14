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

#ifndef GECODE_WORD_STRUCTURE_HH
#define GECODE_WORD_STRUCTURE_HH

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Structure {

  /// Fixed unary structural operation
  enum FixedOp {
    FO_EXTRACT,
    FO_REPEAT,
    FO_ZERO_EXTEND,
    FO_SIGN_EXTEND,
    FO_SHIFT_LEFT,
    FO_LOGICAL_SHIFT_RIGHT,
    FO_ARITHMETIC_SHIFT_RIGHT,
    FO_ROTATE_LEFT,
    FO_ROTATE_RIGHT
  };

  /// Masked equality propagator for fixed unary structural operations
  template<class View0, class View1>
  class Fixed : public MixBinaryPropagator<
    View0,PC_WORD_BITS,View1,PC_WORD_BITS> {
  protected:
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x0;
    using MixBinaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS>::x1;
    FixedOp op;
    unsigned int a;
    unsigned int b;
    Fixed(Home home, View0 x0, View1 x1, FixedOp op,
          unsigned int a, unsigned int b);
    Fixed(Space& home, Fixed& p);
  public:
    static ExecStatus narrow(Home home, View0 x0, View1 x1,
                             FixedOp op, unsigned int a, unsigned int b);
  protected:
    static bool done(View0 x0, View1 x1, FixedOp op,
                     unsigned int a, unsigned int b);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 x0, View1 x1, FixedOp op,
                           unsigned int a=0, unsigned int b=0);
  };

  /// Masked equality propagator for concatenation
  template<class View0, class View1, class View2>
  class Concat : public MixTernaryPropagator<
    View0,PC_WORD_BITS,View1,PC_WORD_BITS,View2,PC_WORD_BITS> {
  protected:
    using MixTernaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS,
      View2,PC_WORD_BITS>::x0;
    using MixTernaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS,
      View2,PC_WORD_BITS>::x1;
    using MixTernaryPropagator<
      View0,PC_WORD_BITS,View1,PC_WORD_BITS,
      View2,PC_WORD_BITS>::x2;
    Concat(Home home, View0 high, View1 low, View2 result);
    Concat(Space& home, Concat& p);
    static ExecStatus narrow(Home home, View0 high, View1 low, View2 result);
  public:
    virtual Actor* copy(Space& home);
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, View0 high, View1 low, View2 result);
  };

  /**
   * \brief Shift controlled by an unsigned word amount
   *
   * The actor computes sound cube hulls over the at most \c width distinct
   * in-range amounts and one overshift class. It is deliberately word-level:
   * no amount bits are channelled to Boolean variables. The propagation
   * property is cube-hull support over these bounded shift classes, not domain
   * consistency.
   */
  class VariableShift : public TernaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    FixedOp op;
    VariableShift(Home home, WordView x, WordView amount,
                  WordView result, FixedOp op);
    VariableShift(Space& home, VariableShift& p);
    static ExecStatus narrow(Home home, WordView x, WordView amount,
                             WordView result, FixedOp op);
    static bool done(WordView x, WordView amount, WordView result,
                     FixedOp op);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x, WordView amount,
                           WordView result, FixedOp op);
  };

}}}

#include <gecode/word/structure/fixed.hpp>
#include <gecode/word/structure/concat.hpp>
#include <gecode/word/structure/variable-shift.hpp>
#include <gecode/word/structure/bounded.hpp>

#endif

// STATISTICS: word-prop
