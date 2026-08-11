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

  /**
   * \brief Word-level propagator for a Boolean truth table at every bit
   *
   * The table has one bit for each tuple, with variable zero represented
   * by the least significant tuple bit. Views passed to this actor are
   * pairwise distinct.
   */
  class Table : public NaryPropagator<WordView,PC_WORD_BITS> {
  protected:
    using NaryPropagator<WordView,PC_WORD_BITS>::x;
    WordValue allowed[8];
    Table(Home home, ViewArray<WordView>& x, const WordValue* allowed);
    Table(Space& home, Table& p);
    static ExecStatus narrow(Home home, ViewArray<WordView>& x,
                             const WordValue* allowed);
  public:
    virtual Actor* copy(Space& home);
    virtual PropCost cost(const Space& home,
                          const ModEventDelta& med) const;
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static ExecStatus post(Home home, ViewArray<WordView>& x,
                           const WordValue* allowed);
  };

}}}

#include <gecode/word/logic/table.hpp>

#endif

// STATISTICS: word-prop
