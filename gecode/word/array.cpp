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

#include <gecode/word.hh>

namespace Gecode {
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordValue lo, WordValue hi)
    : VarArgArray<WordVar>(n) {
    Word::check_domain(width,lo,hi,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,width,lo,hi);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type)
    : VarArgArray<WordVar>(n) {
    if ((width == 0U) || (width > 64U) ||
        ((domain_type != WDT_CUBE) &&
         (domain_type != WDT_UNSIGNED) &&
         (domain_type != WDT_SIGNED)))
      throw Word::OutOfLimits("WordVarArgs::WordVarArgs");
    const WordValue mask = Word::width_mask(width);
    WordValue lo = 0;
    WordValue hi = mask;
    WordValue minimum = (domain_type == WDT_SIGNED) ?
      Word::sign_bit(width) : 0;
    WordValue maximum = (domain_type == WDT_SIGNED) ?
      (Word::sign_bit(width)-1) : mask;
    if (domain_type != WDT_CUBE)
      Word::check_bounded_domain(width,lo,hi,domain_type,minimum,maximum,
                                 "WordVarArgs::WordVarArgs");
    for (int i = size(); i--; ) {
      if (domain_type == WDT_CUBE)
        a[i]._init(home,width,lo,hi);
      else
        a[i]._init(home,width,lo,hi,domain_type,minimum,maximum);
    }
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type,
                           WordValue minimum, WordValue maximum)
    : VarArgArray<WordVar>(n) {
    WordValue lo = 0;
    WordValue hi = Word::width_mask(width);
    Word::check_bounded_domain(width,lo,hi,domain_type,minimum,maximum,
                               "WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,width,lo,hi,domain_type,minimum,maximum);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordValue lo, WordValue hi,
                           WordDomainType domain_type,
                           WordValue minimum, WordValue maximum)
    : VarArgArray<WordVar>(n) {
    Word::check_bounded_domain(width,lo,hi,domain_type,minimum,maximum,
                               "WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,width,lo,hi,domain_type,minimum,maximum);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordValue lo, WordValue hi)
    : VarArray<WordVar>(home,n) {
    Word::check_domain(width,lo,hi,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,width,lo,hi);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type)
    : VarArray<WordVar>(home,n) {
    if ((width == 0U) || (width > 64U) ||
        ((domain_type != WDT_CUBE) &&
         (domain_type != WDT_UNSIGNED) &&
         (domain_type != WDT_SIGNED)))
      throw Word::OutOfLimits("WordVarArray::WordVarArray");
    const WordValue mask = Word::width_mask(width);
    WordValue lo = 0;
    WordValue hi = mask;
    WordValue minimum = (domain_type == WDT_SIGNED) ?
      Word::sign_bit(width) : 0;
    WordValue maximum = (domain_type == WDT_SIGNED) ?
      (Word::sign_bit(width)-1) : mask;
    if (domain_type != WDT_CUBE)
      Word::check_bounded_domain(width,lo,hi,domain_type,minimum,maximum,
                                 "WordVarArray::WordVarArray");
    for (int i = size(); i--; ) {
      if (domain_type == WDT_CUBE)
        x[i]._init(home,width,lo,hi);
      else
        x[i]._init(home,width,lo,hi,domain_type,minimum,maximum);
    }
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type,
                             WordValue minimum, WordValue maximum)
    : VarArray<WordVar>(home,n) {
    WordValue lo = 0;
    WordValue hi = Word::width_mask(width);
    Word::check_bounded_domain(width,lo,hi,domain_type,minimum,maximum,
                               "WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,width,lo,hi,domain_type,minimum,maximum);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordValue lo, WordValue hi,
                             WordDomainType domain_type,
                             WordValue minimum, WordValue maximum)
    : VarArray<WordVar>(home,n) {
    Word::check_bounded_domain(width,lo,hi,domain_type,minimum,maximum,
                               "WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,width,lo,hi,domain_type,minimum,maximum);
  }
}

// STATISTICS: word-other
