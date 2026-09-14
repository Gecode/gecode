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
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    Word::check_domain(width,lo,hi,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,width,lo,hi);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type,
                           WordValue minimum, WordValue maximum)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,0,Word::width_mask(width),domain_type,minimum,maximum,
      "WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordValue lo, WordValue hi,
                           WordDomainType domain_type,
                           WordValue minimum, WordValue maximum)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,lo,hi,domain_type,minimum,maximum,
      "WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordValue lo, WordValue hi)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    Word::check_domain(width,lo,hi,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,width,lo,hi);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type,
                             WordValue minimum, WordValue maximum)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,0,Word::width_mask(width),domain_type,minimum,maximum,
      "WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordValue lo, WordValue hi,
                             WordDomainType domain_type,
                             WordValue minimum, WordValue maximum)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,lo,hi,domain_type,minimum,maximum,
      "WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain);
  }
}

// STATISTICS: word-other
