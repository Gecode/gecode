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

namespace Gecode { namespace Search {

  forceinline
  Cutoff::Cutoff(void) {}
  forceinline
  Cutoff::~Cutoff(void) {}


  forceinline
  CutoffConstant::CutoffConstant(unsigned long int c0)
    : c(c0) {}


  forceinline
  CutoffLinear::CutoffLinear(unsigned long int s)
    : scale(s), n(0) {}


  forceinline
  CutoffLuby::CutoffLuby(unsigned long int scale0)
    : i(1U), scale(scale0) {}
  forceinline unsigned long int
  CutoffLuby::log(unsigned long int i) {
    if (i == 1U)
      return 0U;
    unsigned long int exp = 0U;
    while ( (i >> (++exp)) > 1U ) {}
    return exp;
  }
  forceinline unsigned long int
  CutoffLuby::luby(unsigned long int i) {
    while (true) {
      if (i <= n_start)
        return start[i-1];
      unsigned long int l = log(i);
      if (i == (1U<<(l+1))-1)
        return 1UL<<l;
      i=i-(1U<<l)+1;
    }
    GECODE_NEVER;
    return 0;
  }


  forceinline
  CutoffGeometric::CutoffGeometric(unsigned long int scale0, double base0)
    : n(1.0), scale(static_cast<double>(scale0)), base(base0) {}


  forceinline
  CutoffRandom::CutoffRandom(unsigned int seed,
                             unsigned long int min0,
                             unsigned long int max0,
                             unsigned long int n0)
      : rnd(seed), min(min0), n(n0 == 0 ? (max0-min+1U) : n0),
        step(std::max(1UL,
                      static_cast<unsigned long int>((max0-min0+1U)/n))) {
    cur = ++(*this);
  }


  forceinline
  CutoffAppend::CutoffAppend(Cutoff* d1, unsigned long int n0, Cutoff* d2)
    : c1(d1), c2(d2), n(n0) {}
  forceinline
  CutoffAppend::~CutoffAppend(void) {
    delete c1; delete c2;
  }


  forceinline
  CutoffMerge::CutoffMerge(Cutoff* d1, Cutoff* d2)
    : c1(d1), c2(d2) {}
  forceinline
  CutoffMerge::~CutoffMerge(void) {
    delete c1; delete c2;
  }


  forceinline
  CutoffRepeat::CutoffRepeat(Cutoff* c1, unsigned long int n0)
    : c(c1), i(0), n(n0) {
    cutoff = (*c)();
  }
  forceinline
  CutoffRepeat::~CutoffRepeat(void) {
    delete c;
  }

}}

// STATISTICS: search-other
