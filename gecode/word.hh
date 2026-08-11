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

#ifndef GECODE_WORD_HH
#define GECODE_WORD_HH

#include <cstdint>
#include <iostream>
#include <vector>
#include <gecode/int.hh>

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))
# ifdef GECODE_BUILD_WORD
#  define GECODE_WORD_EXPORT __declspec(dllexport)
# else
#  define GECODE_WORD_EXPORT __declspec(dllimport)
# endif
#else
# ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#  define GECODE_WORD_EXPORT __attribute__((visibility("default")))
# else
#  define GECODE_WORD_EXPORT
# endif
#endif

#ifndef GECODE_BUILD_WORD
# define GECODE_LIBRARY_NAME "Word"
# include <gecode/support/auto-link.hpp>
#endif

namespace Gecode {
  /** \brief Unsigned storage type for word values
   *  \ingroup TaskModelWordVars
   */
  typedef std::uint64_t WordValue;
}

#include <gecode/word/exception.hpp>
#include <gecode/word/var-imp.hpp>

namespace Gecode {
  namespace Word { class WordView; }

  /**
   * \defgroup TaskModelWord Word-vector constraints
   */
  /**
   * \defgroup TaskModelWordVars Word-vector variables and arrays
   * \ingroup TaskModelWord
   */
  /**
   * \brief Fixed-width word variable
   *
   * The domain is represented by a known-one lower mask and a may-be-one
   * upper mask. Widths from 1 through 64 are supported.
   * \ingroup TaskModelWordVars
   */
  class WordVar : public VarImpVar<Word::WordVarImp> {
    friend class WordVarArray;
    friend class WordVarArgs;
  private:
    using VarImpVar<Word::WordVarImp>::x;
    void _init(Space& home, unsigned int width, WordValue lo, WordValue hi);
  public:
    WordVar(void);
    WordVar(const WordVar& y);
    WordVar(const Word::WordView& y);
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width);
    GECODE_WORD_EXPORT WordVar(Space& home, unsigned int width,
                              WordValue lo, WordValue hi);
    unsigned int width(void) const;
    WordValue mask(void) const;
    WordValue lo(void) const;
    WordValue hi(void) const;
    WordValue unknown(void) const;
    unsigned int unknown_size(void) const;
    bool assigned(void) const;
    bool in(WordValue value) const;
    WordValue val(void) const;
    WordVar& operator=(const WordVar&) = default;
  };
}

#include <gecode/word/var/word.hpp>
#include <gecode/word/view.hpp>

namespace Gecode {
  forceinline WordVar::WordVar(const Word::WordView& y)
    : VarImpVar<Word::WordVarImp>(y.varimp()) {}
}

#include <gecode/word/array-traits.hpp>

namespace Gecode {
  /// Passing word variables
  class WordVarArgs : public VarArgArray<WordVar> {
  public:
    WordVarArgs(void);
    explicit WordVarArgs(int n);
    WordVarArgs(const WordVarArgs& a);
    WordVarArgs(const VarArray<WordVar>& a);
    WordVarArgs(const std::vector<WordVar>& a);
    WordVarArgs(std::initializer_list<WordVar> a);
    template<class InputIterator> WordVarArgs(InputIterator first, InputIterator last);
    GECODE_WORD_EXPORT WordVarArgs(Space& home, int n, unsigned int width,
                                  WordValue lo, WordValue hi);
    WordVarArgs& operator=(const WordVarArgs&) = default;
  };
  /// Word variable array
  class WordVarArray : public VarArray<WordVar> {
  public:
    WordVarArray(void);
    WordVarArray(Space& home, int n);
    WordVarArray(const WordVarArray& a);
    WordVarArray(Space& home, const WordVarArgs& a);
    GECODE_WORD_EXPORT WordVarArray(Space& home, int n, unsigned int width,
                                   WordValue lo, WordValue hi);
    WordVarArray& operator=(const WordVarArray&) = default;
  };

  /// Restrict \a x to the word cube described by \a lo and \a hi
  GECODE_WORD_EXPORT void dom(Home home, WordVar x,
                              WordValue lo, WordValue hi);
  /// Assign \a x to \a value
  GECODE_WORD_EXPORT void dom(Home home, WordVar x, WordValue value);

  /**
   * \defgroup TaskModelWordChannel Channel constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Channel bit \a bit of \a x to Boolean variable \a b
  GECODE_WORD_EXPORT void channel(Home home, WordVar x,
                                  unsigned int bit, BoolVar b);
  /// Channel bit \a bit of \a x to Boolean constant \a value
  GECODE_WORD_EXPORT void channel(Home home, WordVar x,
                                  unsigned int bit, int value);
  //@}

  /// Word relation type
  enum WordRelType {
    WRT_EQ, ///< Equality
    WRT_NQ  ///< Disequality
  };

  /// Word logical operation type
  enum WordOpType {
    WOT_AND,  ///< Bitwise conjunction
    WOT_OR,   ///< Bitwise disjunction
    WOT_XOR,  ///< Bitwise exclusive disjunction
    WOT_NAND, ///< Complement of bitwise conjunction
    WOT_NOR,  ///< Complement of bitwise disjunction
    WOT_XNOR  ///< Complement of bitwise exclusive disjunction
  };

  /**
   * \defgroup TaskModelWordRel Word relations
   * \ingroup TaskModelWord
   */
  //@{
  /// Post the relation \a wrt between \a x and \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y);
  /// Post the relation \a wrt between \a x and \a y, reified by \a r
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              WordVar y, Reify r);
  /// Post the relation \a wrt between \a x and an explicitly-sized constant
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              unsigned int width, WordValue value);
  /// Post the relation \a wrt to a constant, reified by \a r
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordRelType wrt,
                              unsigned int width, WordValue value, Reify r);
  //@}

  /**
   * \defgroup TaskModelWordLogic Word logical constraints
   * \ingroup TaskModelWord
   */
  //@{
  /// Post bitwise complement \a y = ~\a x
  GECODE_WORD_EXPORT void complement(Home home, WordVar x, WordVar y);
  /// Post complement from an explicitly-sized constant input
  GECODE_WORD_EXPORT void complement(Home home, unsigned int width,
                                     WordValue value, WordVar y);
  /// Post complement of \a x equal to an explicitly-sized constant
  GECODE_WORD_EXPORT void complement(Home home, WordVar x,
                                     unsigned int width, WordValue value);
  /// Post the binary logical operation \a z = \a x \a wot \a y
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordVar y, WordVar z);
  /// Post a binary operation with an explicitly-sized constant operand
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              unsigned int width, WordValue value, WordVar z);
  /// Post a binary operation equal to an explicitly-sized constant result
  GECODE_WORD_EXPORT void rel(Home home, WordVar x, WordOpType wot,
                              WordVar y, unsigned int width, WordValue value);
  /**
   * \brief Post the n-ary logical operation \a y = \a wot(\a x)
   *
   * Nand, nor, and xnor are the complement of the complete and, or, and
   * xor aggregate respectively. Empty aggregates use the corresponding
   * fixed-width identity.
   */
  GECODE_WORD_EXPORT void rel(Home home, WordOpType wot,
                              const WordVarArgs& x, WordVar y);
  //@}

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const WordVar& x);
}

#include <gecode/word/array.hpp>
#include <gecode/word/print.hpp>

#endif
