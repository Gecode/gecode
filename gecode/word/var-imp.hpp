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
  class BoundedWordVarImp;

  /// Delta recording bits newly fixed to zero and one
  class WordDelta : public Delta {
    friend class WordVarImp;
  private:
    WordValue _zero;
    WordValue _one;
    WordValue _old_minimum;
    WordValue _old_maximum;
    WordValue _new_minimum;
    WordValue _new_maximum;
    WordDomainType _domain_type;
  public:
    WordDelta(void);
    WordDelta(WordValue zero, WordValue one,
              WordDomainType domain_type=WDT_CUBE,
              WordValue old_minimum=0, WordValue old_maximum=0,
              WordValue new_minimum=0, WordValue new_maximum=0);
    WordValue zero(void) const;
    WordValue one(void) const;
    WordDomainType domain_type(void) const;
    /// Return the old internal order-rank minimum
    WordValue old_minimum(void) const;
    /// Return the old internal order-rank maximum
    WordValue old_maximum(void) const;
    /// Return the new internal order-rank minimum
    WordValue new_minimum(void) const;
    /// Return the new internal order-rank maximum
    WordValue new_maximum(void) const;
  };
}}

#include <gecode/word/var-imp/delta.hpp>

namespace Gecode { namespace Word {
  /// Word variable implementation
  class WordVarImp : public WordVarImpBase {
    friend class BoundedWordVarImp;
  protected:
    unsigned int _width;
    WordDomainType _domain_type;
    WordValue _lo;
    WordValue _hi;
    WordVarImp(Space& home, WordVarImp& x);
    WordVarImp(Space& home, unsigned int width, WordValue lo, WordValue hi,
               WordDomainType domain_type);
  public:
    WordVarImp(Space& home, unsigned int width, WordValue lo, WordValue hi);

    unsigned int width(void) const;
    WordValue mask(void) const;
    WordValue lo(void) const;
    WordValue hi(void) const;
    WordDomainType domain_type(void) const;
    bool bounded(void) const;
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue minimum(void) const;
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue maximum(void) const;
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
    static WordDomainType domain_type(const Delta& d);
    static WordValue old_minimum(const Delta& d);
    static WordValue old_maximum(const Delta& d);
    static WordValue new_minimum(const Delta& d);
    static WordValue new_maximum(const Delta& d);

  private:
    GECODE_WORD_EXPORT WordVarImp* perform_copy(Space& home);
  public:
    WordVarImp* copy(Space& home);
  };

  /// Word implementation with one immutable ranked interval
  class BoundedWordVarImp : public WordVarImp {
    friend class WordVarImp;
  protected:
    WordValue _minimum;
    WordValue _maximum;
    BoundedWordVarImp(Space& home, BoundedWordVarImp& x);
  public:
    BoundedWordVarImp(Space& home, unsigned int width,
                      WordValue lo, WordValue hi,
                      WordDomainType domain_type,
                      WordValue minimum, WordValue maximum);

    /// Return the internal order-rank minimum
    WordValue minimum(void) const;
    /// Return the internal order-rank maximum
    WordValue maximum(void) const;
    WordValue rank(WordValue value) const;

    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum);
    ModEvent narrow_range(Space& home, WordValue minimum,
                          WordValue maximum);
  };
}}

#include <gecode/word/var-imp/word.hpp>

// STATISTICS: word-var

#endif
