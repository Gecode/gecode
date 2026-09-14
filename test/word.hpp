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

namespace Test { namespace Word {

  forceinline
  Domain::Domain(unsigned int width, Gecode::WordValue lo,
                 Gecode::WordValue hi)
    : _width(width), _lo(lo), _hi(hi) {
    Gecode::Word::check_domain(width,lo,hi,"Test::Word::Domain");
  }
  forceinline unsigned int Domain::width(void) const { return _width; }
  forceinline Gecode::WordValue Domain::mask(void) const {
    return Gecode::Word::width_mask(_width);
  }
  forceinline Gecode::WordValue Domain::lo(void) const { return _lo; }
  forceinline Gecode::WordValue Domain::hi(void) const { return _hi; }
  forceinline Gecode::WordValue Domain::unknown(void) const {
    return _hi & ~_lo;
  }
  forceinline bool Domain::in(Gecode::WordValue value) const {
    return ((value & ~mask()) == 0) && ((value & _lo) == _lo) &&
      ((value & ~_hi) == 0);
  }

  forceinline Assignment::Assignment(int n0, const Domain& d0)
    : n(n0), d(d0) {}
  forceinline int Assignment::size(void) const { return n; }
  forceinline const Domain& Assignment::domain(void) const { return d; }
  forceinline Assignment::~Assignment(void) {}

  forceinline bool CpltAssignment::has_more(void) const {
    return more;
  }
  forceinline Gecode::WordValue CpltAssignment::operator[](int i) const {
    assert((i >= 0) && (i < n));
    return vals[i];
  }
  forceinline CpltAssignment::~CpltAssignment(void) { delete [] vals; }

  forceinline bool RandomAssignment::has_more(void) const {
    return remaining > 0;
  }
  forceinline Gecode::WordValue RandomAssignment::operator[](int i) const {
    assert((i >= 0) && (i < n));
    return vals[i];
  }
  forceinline RandomAssignment::~RandomAssignment(void) { delete [] vals; }

  forceinline bool PartialDomains::operator()(void) const {
    return state < end;
  }
  forceinline void PartialDomains::operator++(void) { state++; }

  forceinline bool PartialAssignment::has_more(void) const {
    return state < end;
  }
  forceinline void PartialAssignment::next(void) { state++; }
  forceinline int PartialAssignment::size(void) const { return n; }

  forceinline bool Test::eqv(void) const {
    return reified && ((rms & (1 << Gecode::RM_EQV)) != 0);
  }
  forceinline bool Test::imp(void) const {
    return reified && ((rms & (1 << Gecode::RM_IMP)) != 0);
  }
  forceinline bool Test::pmi(void) const {
    return reified && ((rms & (1 << Gecode::RM_PMI)) != 0);
  }

}}

// STATISTICS: test-word
