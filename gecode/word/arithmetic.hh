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

#ifndef GECODE_WORD_ARITHMETIC_HH
#define GECODE_WORD_ARITHMETIC_HH

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Arithmetic {

  /**
   * \brief Bit-consistent propagator for fixed-width modular addition
   *
   * Carries are represented by a temporary two-state chain during
   * propagation. No carry variables or carry state are stored in the space.
   */
  class Add : public TernaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    Add(Home home, WordView x0, WordView x1, WordView x2);
    Add(Space& home, Add& p);
    static ExecStatus narrow(Home home, WordView x0, WordView x1,
                             WordView x2);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2);
  };

  /// Bit-consistent addition with an exposed final carry
  class AddCarry : public MixNaryOnePropagator<
    WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL> {
  protected:
    using MixNaryOnePropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::x;
    using MixNaryOnePropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::y;
    AddCarry(Home home, ViewArray<WordView>& x, Int::BoolView carry);
    AddCarry(Space& home, AddCarry& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2, Int::BoolView carry);
  };

  /**
   * \brief Bit-consistent propagator for fixed-width modular negation
   *
   * Carries for two's-complement negation are represented by a temporary
   * two-state chain during propagation.
   */
  class Neg : public BinaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using BinaryPropagator<WordView,PC_WORD_BITS>::x0;
    using BinaryPropagator<WordView,PC_WORD_BITS>::x1;
    Neg(Home home, WordView x0, WordView x1);
    Neg(Space& home, Neg& p);
    static ExecStatus narrow(Home home, WordView x0, WordView x1);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1);
  };

  /**
   * \brief Bit-consistent propagator for fixed-width modular subtraction
   *
   * Borrows are represented by a temporary two-state chain during
   * propagation. No borrow variables or borrow state are stored in the space.
   */
  class Sub : public TernaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    Sub(Home home, WordView x0, WordView x1, WordView x2);
    Sub(Space& home, Sub& p);
    static ExecStatus narrow(Home home, WordView x0, WordView x1,
                             WordView x2);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2);
  };

  /// Bit-consistent subtraction with an exposed final borrow
  class SubBorrow : public MixNaryOnePropagator<
    WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL> {
  protected:
    using MixNaryOnePropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::x;
    using MixNaryOnePropagator<
      WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>::y;
    SubBorrow(Home home, ViewArray<WordView>& x, Int::BoolView borrow);
    SubBorrow(Space& home, SubBorrow& p);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2, Int::BoolView borrow);
  };

  /**
   * \brief Low-prefix propagator for fixed-width modular multiplication
   *
   * The propagator is exact for assigned operands and propagates fixed low
   * prefixes in both directions, including modular inversion of odd factors.
   * This is deliberately weaker than bit consistency for multiplication.
   */
  class Mult : public TernaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    Mult(Home home, WordView x0, WordView x1, WordView x2);
    Mult(Space& home, Mult& p);
    static ExecStatus narrow(Home home, WordView x0, WordView x1,
                             WordView x2);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2);
  };

  /** \brief Native unsigned division propagator
   *
   * Exact on assigned words, with sound unsigned range-hull propagation and
   * inverse pruning for assigned quotient or divisor. It does not claim
   * domain consistency for cube domains.
   */
  class Div : public TernaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    Div(Home home, WordView x0, WordView x1, WordView x2);
    Div(Space& home, Div& p);
    static ExecStatus narrow(Home home, WordView x0, WordView x1,
                             WordView x2);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2);
  };

  /** \brief Native unsigned remainder propagator
   *
   * Exact on assigned words, with sound unsigned range-hull propagation and
   * bidirectional low-bit propagation for power-of-two divisors. It does not
   * claim domain consistency for cube domains.
   */
  class Mod : public TernaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    Mod(Home home, WordView x0, WordView x1, WordView x2);
    Mod(Space& home, Mod& p);
    static ExecStatus narrow(Home home, WordView x0, WordView x1,
                             WordView x2);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, WordView x0, WordView x1,
                           WordView x2);
  };

}}}

#include <gecode/word/arithmetic/add.hpp>
#include <gecode/word/arithmetic/neg-sub.hpp>
#include <gecode/word/arithmetic/mult.hpp>
#include <gecode/word/arithmetic/divmod.hpp>

#endif

// STATISTICS: word-prop
