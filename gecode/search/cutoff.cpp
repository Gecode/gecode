/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2013
 *     Guido Tack, 2013
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <algorithm>
#include <gecode/search.hh>

namespace Gecode { namespace Search {

  unsigned long int
  CutoffConstant::operator ()(void) const {
    return c;
  }
  unsigned long int
  CutoffConstant::operator ++(void) {
    return c;
  }


  unsigned long int
  CutoffLinear::operator ()(void) const {
    return n;
  }
  unsigned long int
  CutoffLinear::operator ++(void) {
    n += scale;
    return n;
  }


  unsigned long int
  CutoffLuby::start[CutoffLuby::n_start] = {
    1,1,2,1,1,2,4,1,1,2,1,1,2,4,8,1,1,2,1,1,2,4,1,1,2,1,1,2,4,8,16,
    1,1,2,1,1,2,4,1,1,2,1,1,2,4,8,1,1,2,1,1,2,4,1,1,2,1,1,2,4,8,16,32
  };
  unsigned long int
  CutoffLuby::operator ()(void) const {
    return scale*luby(i);
  }
  unsigned long int
  CutoffLuby::operator ++(void) {
    return scale*luby(i++);
  }


  unsigned long int
  CutoffGeometric::operator ()(void) const {
    return static_cast<unsigned long int>(scale * n);
  }
  unsigned long int
  CutoffGeometric::operator ++(void) {
    n *= base;
    return static_cast<unsigned long int>(scale * n);
  }


  unsigned long int
  CutoffRandom::operator ++(void) {
    cur = min+step*rnd(n);
    return cur;
  }
  unsigned long int
  CutoffRandom::operator ()(void) const {
    return cur;
  }


  unsigned long int
  CutoffAppend::operator ()(void) const {
    if (n > 0) {
      return (*c1)();
    } else {
      return (*c2)();
    }
  }
  unsigned long int
  CutoffAppend::operator ++(void) {
    if (n > 0) {
      n--;
      return ++(*c1);
    } else {
      return ++(*c2);
    }
  }


  unsigned long int
  CutoffMerge::operator ()(void) const {
    return (*c1)();
  }
  unsigned long int
  CutoffMerge::operator ++(void) {
    (void) ++(*c1);
    std::swap(c1,c2);
    return (*c1)();
  }


  unsigned long int
  CutoffRepeat::operator ()(void) const {
    return cutoff;
  }
  unsigned long int
  CutoffRepeat::operator ++(void) {
    i++;
    if (i == n) {
      cutoff = (*c)();
      i = 0;
    }
    return cutoff;
  }


  Cutoff*
  Cutoff::constant(unsigned long int scale) {
    return new CutoffConstant(scale);
  }
  Cutoff*
  Cutoff::linear(unsigned long int scale) {
    return new CutoffLinear(scale);
  }
  Cutoff*
  Cutoff::luby(unsigned long int scale) {
    return new CutoffLuby(scale);
  }
  Cutoff*
  Cutoff::geometric(unsigned long int base, double scale) {
    return new CutoffGeometric(base,scale);
  }
  Cutoff*
  Cutoff::rnd(unsigned int seed,
              unsigned long int min,
              unsigned long int max,
              unsigned long int n) {
    return new CutoffRandom(seed,min,max,n);
  }
  Cutoff*
  Cutoff::append(Cutoff* c1, unsigned long int n, Cutoff* c2) {
    return new CutoffAppend(c1,n,c2);
  }
  Cutoff*
  Cutoff::merge(Cutoff* c1, Cutoff* c2) {
    return new CutoffMerge(c1,c2);
  }
  Cutoff*
  Cutoff::repeat(Cutoff* c, unsigned long int n) {
  return new CutoffRepeat(c,n);
  }

}}

// STATISTICS: search-other
