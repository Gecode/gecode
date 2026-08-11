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

namespace Gecode { namespace Word {
  forceinline WordValue width_mask(unsigned int width) {
    return (width >= 64U) ? ~WordValue(0)
                          : ((WordValue(1) << width) - WordValue(1));
  }

  forceinline void check_domain(unsigned int width, WordValue lo,
                                WordValue hi, const char* location) {
    if ((width == 0U) || (width > 64U))
      throw OutOfLimits(location);
    const WordValue m = width_mask(width);
    if (((lo | hi) & ~m) != 0)
      throw OutOfLimits(location);
    if ((lo & ~hi) != 0)
      throw VariableEmptyDomain(location);
  }

  forceinline WordVarImp::WordVarImp(Space& home, unsigned int width,
                                     WordValue lo, WordValue hi)
    : WordVarImpBase(home), _width(width), _lo(lo), _hi(hi) {}
  forceinline WordVarImp::WordVarImp(Space& home, WordVarImp& x)
    : WordVarImpBase(home,x), _width(x._width), _lo(x._lo), _hi(x._hi) {}

  forceinline unsigned int WordVarImp::width(void) const { return _width; }
  forceinline WordValue WordVarImp::mask(void) const { return width_mask(_width); }
  forceinline WordValue WordVarImp::lo(void) const { return _lo; }
  forceinline WordValue WordVarImp::hi(void) const { return _hi; }
  forceinline WordValue WordVarImp::unknown(void) const { return _hi & ~_lo; }
  forceinline WordValue WordVarImp::val(void) const { return _lo; }
  forceinline unsigned int WordVarImp::unknown_size(void) const {
    WordValue u = unknown();
    unsigned int n = 0;
    while (u != 0) { u &= u-1; n++; }
    return n;
  }
  forceinline bool WordVarImp::assigned(void) const { return _lo == _hi; }
  forceinline bool WordVarImp::in(WordValue n) const {
    return ((n & ~mask()) == 0) && ((n & _lo) == _lo) && ((n & ~_hi) == 0);
  }

  forceinline ModEvent
  WordVarImp::narrow(Space& home, WordValue lo, WordValue hi) {
    const WordValue m = mask();
    if (((lo | hi) & ~m) != 0 || (lo & ~hi) != 0)
      return fail(home);
    const WordValue new_lo = _lo | lo;
    const WordValue new_hi = _hi & hi;
    if ((new_lo & ~new_hi) != 0)
      return fail(home);
    if ((new_lo == _lo) && (new_hi == _hi))
      return ME_WORD_NONE;
    WordDelta d(_hi & ~new_hi, new_lo & ~_lo);
    _lo = new_lo;
    _hi = new_hi;
    return notify(home, assigned() ? ME_WORD_VAL : ME_WORD_BITS, d);
  }

  forceinline ModEvent WordVarImp::eq(Space& home, WordValue value) {
    if (!in(value))
      return fail(home);
    if (assigned())
      return ME_WORD_NONE;
    WordDelta d(_hi & ~value, value & ~_lo);
    _lo = _hi = value;
    return notify(home,ME_WORD_VAL,d);
  }

  forceinline WordVarImp* WordVarImp::copy(Space& home) {
    return copied() ? static_cast<WordVarImp*>(forward()) : perform_copy(home);
  }
  forceinline WordVarImp* WordVarImp::perform_copy(Space& home) {
    return new (home) WordVarImp(home,*this);
  }
  forceinline ModEventDelta WordVarImp::med(ModEvent me) {
    return WordVarImpBase::med(me);
  }
  forceinline WordValue WordVarImp::zero(const Delta& d) {
    return static_cast<const WordDelta&>(d).zero();
  }
  forceinline WordValue WordVarImp::one(const Delta& d) {
    return static_cast<const WordDelta&>(d).one();
  }
}}

// STATISTICS: word-var
