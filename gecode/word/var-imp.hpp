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

#ifndef GECODE_WORD_VAR_IMP_HPP
#define GECODE_WORD_VAR_IMP_HPP

namespace Gecode { namespace Word {
  class WordVarImp;

  /// Delta recording bits newly fixed to zero and one
  class WordDelta : public Delta {
    friend class WordVarImp;
  private:
    WordValue _zero;
    WordValue _one;
  public:
    WordDelta(void);
    WordDelta(WordValue zero, WordValue one);
    WordValue zero(void) const;
    WordValue one(void) const;
  };
}}

#include <gecode/word/var-imp/delta.hpp>

namespace Gecode { namespace Word {
  /// Word variable implementation
  class WordVarImp : public WordVarImpBase {
  protected:
    unsigned int _width;
    WordValue _lo;
    WordValue _hi;
    WordVarImp(Space& home, WordVarImp& x);
  public:
    WordVarImp(Space& home, unsigned int width, WordValue lo, WordValue hi);

    unsigned int width(void) const;
    WordValue mask(void) const;
    WordValue lo(void) const;
    WordValue hi(void) const;
    WordValue unknown(void) const;
    WordValue val(void) const;
    unsigned int unknown_size(void) const;
    bool assigned(void) const;
    bool in(WordValue n) const;

    ModEvent narrow(Space& home, WordValue lo, WordValue hi);
    ModEvent eq(Space& home, WordValue value);

    GECODE_WORD_EXPORT void subscribe(Space& home, Propagator& p,
                                      PropCond pc, bool schedule=true);
    GECODE_WORD_EXPORT void reschedule(Space& home, Propagator& p,
                                       PropCond pc);
    GECODE_WORD_EXPORT void subscribe(Space& home, Advisor& a, bool fail);

    static ModEventDelta med(ModEvent me);
    static WordValue zero(const Delta& d);
    static WordValue one(const Delta& d);

  private:
    GECODE_WORD_EXPORT WordVarImp* perform_copy(Space& home);
  public:
    WordVarImp* copy(Space& home);
  };
}}

#include <gecode/word/var-imp/word.hpp>

// STATISTICS: word-var

#endif
