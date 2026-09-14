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

#ifndef GECODE_WORD_LOGIC_HH
#define GECODE_WORD_LOGIC_HH

#include <gecode/word.hh>

namespace Gecode { namespace Word { namespace Logic {

  /** \addtogroup FuncWordProp
   * @{
   */

  /// Primitive operation for a native binary word logic actor.
  enum BinaryOperation {
    BO_OR,
    BO_XOR
  };

  /** \brief Bit-consistent native binary word logic actor
   *
   * The three views are pairwise distinct. AND and aliases use the generic
   * truth-table projection path. Filtering publishes x, y, then z and repeats
   * when bounded-domain closure changes their computed masks.
   */
  template<BinaryOperation op>
  class Binary : public TernaryPropagator<WordView,PC_WORD_BITS> {
  public:
    Actor* copy(Space& home) override;
    PropCost cost(const Space& home, const ModEventDelta& med) const override;
    ExecStatus propagate(Space& home, const ModEventDelta& med) override;
    static ExecStatus post(Home home, WordView x, WordView y, WordView z);
  protected:
    using TernaryPropagator<WordView,PC_WORD_BITS>::x0;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x1;
    using TernaryPropagator<WordView,PC_WORD_BITS>::x2;
    Binary(Home home, WordView x, WordView y, WordView z);
    Binary(Space& home, Binary& p);
    static ExecStatus narrow(Home home, WordView x, WordView y, WordView z);
  };

  /** \brief Enforces a Boolean truth table independently at each word bit
   *
   * One to four pairwise distinct views index tuples, with view zero in the
   * least significant tuple bit. For each of the 2^n entries, allowed[t] is
   * the mask of word bit positions admitting tuple t. Posting copies those
   * entries; the caller retains ownership of the supplied array.
   */
  class Table : public NaryPropagator<WordView,PC_WORD_BITS> {
  public:
    Actor* copy(Space& home) override;
    size_t dispose(Space& home) override;
    PropCost cost(const Space& home, const ModEventDelta& med) const override;
    ExecStatus propagate(Space& home, const ModEventDelta& med) override;
    static ExecStatus post(Home home, ViewArray<WordView>& x,
                           const WordValue* allowed);
  protected:
    using NaryPropagator<WordView,PC_WORD_BITS>::x;
    Table(Home home, ViewArray<WordView>& x, const WordValue* allowed);
    Table(Space& home, Table& p);
    static ExecStatus narrow(Home home, ViewArray<WordView>& x,
                             const WordValue* allowed);
  private:
    WordValue allowed_[16];
  };

  /** \brief Projects aliased roles before creating a truth-table actor
   *
   * The original array has one to four views and allowed has 2^n entries,
   * each a bit-position admission mask. Both arrays are borrowed for posting;
   * distinct views retain first-occurrence order in Home-owned storage.
   */
  void post_table(Home home, const WordView* original, int n,
                  const WordValue* allowed);

  /// Primitive operation for a native n-ary word logic actor.
  enum NaryOperation {
    NO_AND,
    NO_OR,
    NO_XOR
  };

  /** \brief Bit-consistent native n-ary word logic actor
   *
   * Posting folds assigned inputs into constant, collapses AND/OR duplicates
   * and cancels XOR duplicates by parity. Remaining views are distinct and
   * are folded into the constant when assigned later. XOR's result is distinct
   * from all inputs; AND/OR retain a possible result alias and close its masks
   * locally.
   */
  template<NaryOperation op, class VY>
  class Nary : public MixNaryOnePropagator<
    WordView,PC_WORD_BITS,VY,PC_WORD_BITS> {
  public:
    Actor* copy(Space& home) override;
    size_t dispose(Space& home) override;
    PropCost cost(const Space& home, const ModEventDelta& med) const override;
    ExecStatus propagate(Space& home, const ModEventDelta& med) override;
    static ExecStatus post(Home home, ViewArray<WordView>& x, VY y,
                           WordValue constant);
  protected:
    using MixNaryOnePropagator<
      WordView,PC_WORD_BITS,VY,PC_WORD_BITS>::x;
    using MixNaryOnePropagator<
      WordView,PC_WORD_BITS,VY,PC_WORD_BITS>::y;
    Nary(Home home, ViewArray<WordView>& x, VY y, WordValue constant);
    Nary(Space& home, Nary& p);
    /// Returns ES_OK when no retained actor is needed, ES_FIX while active.
    static ExecStatus narrow(Home home, ViewArray<WordView>& x, VY y,
                             WordValue constant);
  private:
    WordValue constant;
  };

  /** @} */

}}}

#include <gecode/word/logic/table.hpp>
#include <gecode/word/logic/binary.hpp>
#include <gecode/word/logic/nary.hpp>

#endif

// STATISTICS: word-prop
