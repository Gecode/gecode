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
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    if ((width == 0U) || (width > 64U))
      throw Word::OutOfLimits("WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved =
      Word::checked_value_set(width,WDT_CUBE,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,width,0,Word::width_mask(width),resolved);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordBits known,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::WordCube cube = Word::known_cube(width,known,"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved =
      Word::checked_value_set(width,WDT_CUBE,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,width,cube.lo,cube.hi,resolved);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           const WordValArgs& values,
                           WordDomainType domain_type,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArgs::WordVarArgs");
    const WordValArgs fitted = Word::prepare_values(width,values,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain,resolved,fitted);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain,resolved);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, WordDomainType format,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_float_domain(format,"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain,resolved);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, WordDomainType format,
                           double minimum, double maximum,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_float_interval(format,minimum,maximum,"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain,resolved);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type,
                           WordValue minimum, WordValue maximum,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,0,Word::width_mask(width),domain_type,
      Word::fit_to_width(width,minimum,"WordVarArgs::WordVarArgs"),
      Word::fit_to_width(width,maximum,"WordVarArgs::WordVarArgs"),"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain,resolved);
  }
  WordVarArgs::WordVarArgs(Space& home, int n, unsigned int width,
                           WordDomainType domain_type,
                           WordValue minimum, WordValue maximum,
                           WordBits known,
                           WordValueSetOption value_set)
    : VarArgArray<WordVar>(
        Word::require_array_count(n,"WordVarArgs::WordVarArgs")) {
    const Word::WordCube cube = Word::known_cube(width,known,"WordVarArgs::WordVarArgs");
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,cube.lo,cube.hi,domain_type,
      Word::fit_to_width(width,minimum,"WordVarArgs::WordVarArgs"),
      Word::fit_to_width(width,maximum,"WordVarArgs::WordVarArgs"),"WordVarArgs::WordVarArgs");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArgs::WordVarArgs");
    for (int i = size(); i--; )
      a[i]._init(home,domain,resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    if ((width == 0U) || (width > 64U))
      throw Word::OutOfLimits("WordVarArray::WordVarArray");
    const WordValueSetOption resolved =
      Word::checked_value_set(width,WDT_CUBE,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,width,0,Word::width_mask(width),resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordBits known,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::WordCube cube = Word::known_cube(width,known,"WordVarArray::WordVarArray");
    const WordValueSetOption resolved =
      Word::checked_value_set(width,WDT_CUBE,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,width,cube.lo,cube.hi,resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             const WordValArgs& values,
                             WordDomainType domain_type,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVarArray::WordVarArray");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArray::WordVarArray");
    const WordValArgs fitted = Word::prepare_values(width,values,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain,resolved,fitted);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_full_domain(width,domain_type,"WordVarArray::WordVarArray");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain,resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, WordDomainType format,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_float_domain(format,"WordVarArray::WordVarArray");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain,resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, WordDomainType format,
                             double minimum, double maximum,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain =
      Word::prepare_float_interval(format,minimum,maximum,"WordVarArray::WordVarArray");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain,resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type,
                             WordValue minimum, WordValue maximum,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,0,Word::width_mask(width),domain_type,
      Word::fit_to_width(width,minimum,"WordVarArray::WordVarArray"),
      Word::fit_to_width(width,maximum,"WordVarArray::WordVarArray"),"WordVarArray::WordVarArray");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain,resolved);
  }
  WordVarArray::WordVarArray(Space& home, int n, unsigned int width,
                             WordDomainType domain_type,
                             WordValue minimum, WordValue maximum,
                             WordBits known,
                             WordValueSetOption value_set)
    : VarArray<WordVar>(home,
        Word::require_array_count(n,"WordVarArray::WordVarArray")) {
    const Word::WordCube cube = Word::known_cube(width,known,"WordVarArray::WordVarArray");
    const Word::PreparedWordDomain domain = Word::prepare_bounded_domain(
      width,cube.lo,cube.hi,domain_type,
      Word::fit_to_width(width,minimum,"WordVarArray::WordVarArray"),
      Word::fit_to_width(width,maximum,"WordVarArray::WordVarArray"),"WordVarArray::WordVarArray");
    const WordValueSetOption resolved = Word::checked_value_set(
      domain.width,domain.domain_type,value_set,"WordVarArray::WordVarArray");
    for (int i = size(); i--; )
      x[i]._init(home,domain,resolved);
  }
}

// STATISTICS: word-other
