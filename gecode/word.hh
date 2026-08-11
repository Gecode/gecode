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
#include <gecode/kernel.hh>

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

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const WordVar& x);
}

#include <gecode/word/array.hpp>
#include <gecode/word/print.hpp>

#endif
