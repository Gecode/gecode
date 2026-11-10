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

namespace Gecode {
  forceinline WordBits::WordBits(void) : _mask(0), _value(0) {}
  forceinline WordValue WordBits::mask(void) const { return _mask; }
  forceinline WordValue WordBits::value(void) const { return _value; }
  forceinline WordBits known_bits(WordValue mask, WordValue value) {
    if ((value & ~mask) != 0)
      throw Word::OutOfLimits("known_bits");
    WordBits known;
    known._mask = mask;
    known._value = value;
    return known;
  }

  namespace Word {
    /** \brief Return the cube of the words of \a width with \a known bits
     *
     * Throws OutOfLimits at \a location if \a width is not between 1 and
     * 64 or \a known fixes bits outside \a width.
     */
    forceinline WordCube known_cube(unsigned int width, WordBits known,
                                    const char* location) {
      if ((width == 0U) || (width > 64U))
        throw OutOfLimits(location);
      const WordValue mask = width_mask(width);
      if ((known.mask() & ~mask) != 0)
        throw OutOfLimits(location);
      return {known.value(),known.value() | (~known.mask() & mask)};
    }
  }

  forceinline void WordVar::_init(Space& home, unsigned int width,
                                  WordValue lo, WordValue hi,
                                  WordValueSetOption value_set) {
    if (value_set == WVS_EXACT) {
      _init(home,{width,WDT_CUBE,lo,hi,0,Word::width_mask(width)},value_set);
    } else {
      x = new (home) Word::WordVarImp(home,width,lo,hi);
    }
  }
  forceinline void WordVar::_init(Space& home,
                                  const Word::PreparedWordDomain& domain,
                                  WordValueSetOption value_set) {
    if (value_set == WVS_EXACT) {
      x = new (home) Word::ValueSetWordVarImp(home,domain);
    } else if (domain.domain_type == WDT_CUBE) {
      x = new (home) Word::WordVarImp(home,domain.width,domain.lo,domain.hi);
    } else {
      x = new (home) Word::BoundedWordVarImp(home,domain);
    }
  }
  forceinline WordVar::WordVar(void) {}
  forceinline WordVar::WordVar(const WordVar& y)
    : VarImpVar<Word::WordVarImp>(y.varimp()) {}
  forceinline unsigned int WordVar::width(void) const { return x->width(); }
  forceinline WordValue WordVar::mask(void) const { return x->mask(); }
  forceinline WordValue WordVar::lo(void) const { return x->lo(); }
  forceinline WordValue WordVar::hi(void) const { return x->hi(); }
  forceinline WordDomainType WordVar::domain_type(void) const {
    return x->domain_type();
  }
  forceinline bool WordVar::bounded(void) const { return x->bounded(); }
  forceinline bool WordVar::has_value_set(void) const {
    return x->has_value_set();
  }
  forceinline WordValue WordVar::minimum(void) const { return x->minimum(); }
  forceinline WordValue WordVar::maximum(void) const { return x->maximum(); }
  forceinline WordValue WordVar::unknown(void) const { return x->unknown(); }
  forceinline unsigned int WordVar::unknown_size(void) const { return x->unknown_size(); }
  forceinline bool WordVar::assigned(void) const { return x->assigned(); }
  forceinline bool WordVar::in(WordValue n) const { return x->in(n); }
  forceinline WordValue WordVar::val(void) const {
    if (!x->assigned()) throw Word::ValOfUnassignedVar("WordVar::val");
    return x->val();
  }
}

// STATISTICS: word-var
