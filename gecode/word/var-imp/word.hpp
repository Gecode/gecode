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

  forceinline WordValue low_through_highest(WordValue value) {
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return value;
  }

  forceinline bool cube_successor(WordValue lo, WordValue hi,
                                  WordValue bound, WordValue mask,
                                  WordValue& result) {
    const WordValue conflicts = ((bound & ~hi) | (~bound & lo)) & mask;
    if (conflicts == 0) {
      result = bound;
      return true;
    }
    const WordValue through_highest = low_through_highest(conflicts);
    const WordValue highest = through_highest ^ (through_highest >> 1);
    WordValue step;
    if ((highest & lo) != 0) {
      step = highest;
    } else {
      const WordValue above = ~(highest | (highest - 1)) & mask;
      const WordValue eligible = (hi & ~lo) & ~bound & above;
      if (eligible == 0)
        return false;
      step = eligible & (WordValue(0) - eligible);
    }
    const WordValue lower = step - 1;
    result = (bound & ~(step | lower)) | step | (lo & lower);
    return true;
  }

  forceinline bool cube_predecessor(WordValue lo, WordValue hi,
                                    WordValue bound, WordValue mask,
                                    WordValue& result) {
    WordValue complement;
    if (!cube_successor(mask ^ hi,mask ^ lo,mask ^ bound,mask,complement))
      return false;
    result = mask ^ complement;
    return true;
  }

  forceinline WordValue sign_bit(unsigned int width) {
    return WordValue(1) << (width-1);
  }

  forceinline WordValue rank(WordDomainType domain_type,
                             unsigned int width, WordValue value) {
    return value ^ ((domain_type == WDT_SIGNED) ? sign_bit(width) : 0);
  }

  forceinline void ordered_cube(WordDomainType domain_type,
                                unsigned int width,
                                WordValue lo, WordValue hi,
                                WordValue& ordered_lo,
                                WordValue& ordered_hi) {
    ordered_lo = lo;
    ordered_hi = hi;
    if (domain_type == WDT_SIGNED) {
      const WordValue flip = sign_bit(width) & ~(ordered_lo ^ ordered_hi);
      ordered_lo ^= flip;
      ordered_hi ^= flip;
    }
  }

  forceinline void encoded_cube(WordDomainType domain_type,
                                unsigned int width,
                                WordValue ordered_lo,
                                WordValue ordered_hi,
                                WordValue& lo, WordValue& hi) {
    lo = ordered_lo;
    hi = ordered_hi;
    if (domain_type == WDT_SIGNED) {
      const WordValue flip = sign_bit(width) & ~(lo ^ hi);
      lo ^= flip;
      hi ^= flip;
    }
  }

  forceinline bool cube_contains(WordValue lo, WordValue hi,
                                 WordValue value, WordValue mask) {
    return ((value & ~mask) == 0) && ((value & lo) == lo) &&
      ((value & ~hi) == 0);
  }

  forceinline bool synchronize_domain(unsigned int width,
                                      WordDomainType domain_type,
                                      WordValue& lo, WordValue& hi,
                                      WordValue& minimum,
                                      WordValue& maximum) {
    const WordValue m = width_mask(width);
    WordValue ordered_lo, ordered_hi;
    ordered_cube(domain_type,width,lo,hi,ordered_lo,ordered_hi);
    WordValue first, last;
    if (!cube_successor(ordered_lo,ordered_hi,minimum,m,first) ||
        !cube_predecessor(ordered_lo,ordered_hi,maximum,m,last) ||
        (first > last))
      return false;
    minimum = first;
    maximum = last;
    const WordValue difference = first ^ last;
    const WordValue varying = low_through_highest(difference);
    const WordValue fixed = m & ~varying;
    ordered_lo |= first & fixed;
    ordered_hi &= first | ~fixed;
    encoded_cube(domain_type,width,ordered_lo,ordered_hi,lo,hi);
    return true;
  }

  forceinline void check_bounded_domain(unsigned int width,
                                        WordValue& lo, WordValue& hi,
                                        WordDomainType domain_type,
                                        WordValue& minimum,
                                        WordValue& maximum,
                                        const char* location) {
    check_domain(width,lo,hi,location);
    if ((domain_type != WDT_UNSIGNED) && (domain_type != WDT_SIGNED))
      throw OutOfLimits(location);
    const WordValue m = width_mask(width);
    if (((minimum | maximum) & ~m) != 0)
      throw OutOfLimits(location);
    minimum = rank(domain_type,width,minimum);
    maximum = rank(domain_type,width,maximum);
    if ((minimum > maximum) ||
        !synchronize_domain(width,domain_type,lo,hi,minimum,maximum))
      throw VariableEmptyDomain(location);
  }

  forceinline WordVarImp::WordVarImp(Space& home, unsigned int width,
                                     WordValue lo, WordValue hi)
    : WordVarImpBase(home), _width(width), _domain_type(WDT_CUBE),
      _lo(lo), _hi(hi) {}
  forceinline WordVarImp::WordVarImp(Space& home, unsigned int width,
                                     WordValue lo, WordValue hi,
                                     WordDomainType domain_type)
    : WordVarImpBase(home), _width(width), _domain_type(domain_type),
      _lo(lo), _hi(hi) {}
  forceinline WordVarImp::WordVarImp(Space& home, WordVarImp& x)
    : WordVarImpBase(home,x), _width(x._width),
      _domain_type(x._domain_type), _lo(x._lo), _hi(x._hi) {}

  forceinline
  BoundedWordVarImp::BoundedWordVarImp(Space& home, unsigned int width,
                                       WordValue lo, WordValue hi,
                                       WordDomainType domain_type,
                                       WordValue minimum, WordValue maximum)
    : WordVarImp(home,width,lo,hi,domain_type),
      _minimum(minimum), _maximum(maximum) {
    if (!synchronize_domain(width,domain_type,_lo,_hi,
                            _minimum,_maximum))
      throw VariableEmptyDomain("BoundedWordVarImp::BoundedWordVarImp");
  }
  forceinline
  BoundedWordVarImp::BoundedWordVarImp(Space& home, BoundedWordVarImp& x)
    : WordVarImp(home,x), _minimum(x._minimum), _maximum(x._maximum) {}

  forceinline unsigned int WordVarImp::width(void) const { return _width; }
  forceinline WordValue WordVarImp::mask(void) const { return width_mask(_width); }
  forceinline WordValue WordVarImp::lo(void) const { return _lo; }
  forceinline WordValue WordVarImp::hi(void) const { return _hi; }
  forceinline WordDomainType WordVarImp::domain_type(void) const {
    return _domain_type;
  }
  forceinline bool WordVarImp::bounded(void) const {
    return _domain_type != WDT_CUBE;
  }
  forceinline WordValue WordVarImp::minimum(void) const {
    if (!bounded())
      throw BoundsOfCubeVar("WordVar::minimum");
    const WordValue r =
      static_cast<const BoundedWordVarImp*>(this)->minimum();
    return rank(_domain_type,_width,r);
  }
  forceinline WordValue WordVarImp::maximum(void) const {
    if (!bounded())
      throw BoundsOfCubeVar("WordVar::maximum");
    const WordValue r =
      static_cast<const BoundedWordVarImp*>(this)->maximum();
    return rank(_domain_type,_width,r);
  }
  forceinline WordValue BoundedWordVarImp::minimum(void) const {
    return _minimum;
  }
  forceinline WordValue BoundedWordVarImp::maximum(void) const {
    return _maximum;
  }
  forceinline WordValue BoundedWordVarImp::rank(WordValue value) const {
    return Word::rank(_domain_type,_width,value);
  }
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
    if (!cube_contains(_lo,_hi,n,mask()))
      return false;
    if (!bounded())
      return true;
    const BoundedWordVarImp* b =
      static_cast<const BoundedWordVarImp*>(this);
    const WordValue r = b->rank(n);
    return (r >= b->_minimum) && (r <= b->_maximum);
  }

  forceinline ModEvent
  WordVarImp::narrow(Space& home, WordValue lo, WordValue hi) {
    if (bounded()) {
      BoundedWordVarImp* b = static_cast<BoundedWordVarImp*>(this);
      return b->narrow_domain(home,lo,hi,b->_minimum,b->_maximum);
    }
    const WordValue m = mask();
    if (((lo | hi) & ~m) != 0 || (lo & ~hi) != 0)
      return fail(home);
    const WordValue new_lo = _lo | lo;
    const WordValue new_hi = _hi & hi;
    if ((new_lo & ~new_hi) != 0)
      return fail(home);
    if ((new_lo == _lo) && (new_hi == _hi))
      return ME_WORD_NONE;
    WordDelta d(_hi & ~new_hi,new_lo & ~_lo);
    _lo = new_lo;
    _hi = new_hi;
    return notify(home, assigned() ? ME_WORD_VAL : ME_WORD_BITS, d);
  }

  forceinline ModEvent
  BoundedWordVarImp::narrow_domain(Space& home,
                                   WordValue lo, WordValue hi,
                                   WordValue minimum, WordValue maximum) {
    const WordValue m = mask();
    if ((((lo | hi | minimum | maximum) & ~m) != 0) ||
        ((lo & ~hi) != 0) || (minimum > maximum))
      return fail(home);
    WordValue new_lo = _lo | lo;
    WordValue new_hi = _hi & hi;
    WordValue new_minimum = std::max(_minimum,minimum);
    WordValue new_maximum = std::min(_maximum,maximum);
    if (((new_lo & ~new_hi) != 0) || (new_minimum > new_maximum) ||
        !synchronize_domain(_width,_domain_type,new_lo,new_hi,
                            new_minimum,new_maximum))
      return fail(home);
    const bool bits = (new_lo != _lo) || (new_hi != _hi);
    const bool bounds = (new_minimum != _minimum) ||
      (new_maximum != _maximum);
    if (!bits && !bounds)
      return ME_WORD_NONE;
    WordDelta d(_hi & ~new_hi,new_lo & ~_lo,_domain_type,
                _minimum,_maximum,new_minimum,new_maximum);
    _lo = new_lo;
    _hi = new_hi;
    _minimum = new_minimum;
    _maximum = new_maximum;
    return notify(home,assigned() ? ME_WORD_VAL :
                  (bits ? (bounds ? ME_WORD_DOM : ME_WORD_BITS) :
                   ME_WORD_BND),d);
  }

  forceinline ModEvent
  BoundedWordVarImp::narrow_range(Space& home,
                                  WordValue minimum, WordValue maximum) {
    const WordValue m = mask();
    if ((((minimum | maximum) & ~m) != 0) || (minimum > maximum))
      return fail(home);
    const WordValue new_minimum = std::max(_minimum,minimum);
    const WordValue new_maximum = std::min(_maximum,maximum);
    if (new_minimum > new_maximum)
      return fail(home);
    if ((new_minimum == _minimum) && (new_maximum == _maximum))
      return ME_WORD_NONE;

    WordValue ordered_lo, ordered_hi;
    ordered_cube(_domain_type,_width,_lo,_hi,ordered_lo,ordered_hi);
    const WordValue difference = new_minimum ^ new_maximum;
    const WordValue fixed = m & ~low_through_highest(difference);
    const bool admitted =
      cube_contains(ordered_lo,ordered_hi,new_minimum,m) &&
      cube_contains(ordered_lo,ordered_hi,new_maximum,m);
    const bool no_prefix =
      ((new_minimum & fixed & ~ordered_lo) == 0) &&
      (((~new_minimum) & fixed & ordered_hi) == 0);
    if (!admitted || !no_prefix)
      return narrow_domain(home,_lo,_hi,new_minimum,new_maximum);

    WordDelta d(0,0,_domain_type,_minimum,_maximum,
                new_minimum,new_maximum);
    _minimum = new_minimum;
    _maximum = new_maximum;
    return notify(home,ME_WORD_BND,d);
  }

  forceinline ModEvent WordVarImp::eq(Space& home, WordValue value) {
    if (bounded()) {
      BoundedWordVarImp* b = static_cast<BoundedWordVarImp*>(this);
      const WordValue r = b->rank(value);
      return b->narrow_domain(home,value,value,r,r);
    }
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
    if (_domain_type == WDT_CUBE)
      return new (home) WordVarImp(home,*this);
    return new (home) BoundedWordVarImp(
      home,*static_cast<BoundedWordVarImp*>(this));
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
  forceinline WordDomainType WordVarImp::domain_type(const Delta& d) {
    return static_cast<const WordDelta&>(d).domain_type();
  }
  forceinline WordValue WordVarImp::old_minimum(const Delta& d) {
    return static_cast<const WordDelta&>(d).old_minimum();
  }
  forceinline WordValue WordVarImp::old_maximum(const Delta& d) {
    return static_cast<const WordDelta&>(d).old_maximum();
  }
  forceinline WordValue WordVarImp::new_minimum(const Delta& d) {
    return static_cast<const WordDelta&>(d).new_minimum();
  }
  forceinline WordValue WordVarImp::new_maximum(const Delta& d) {
    return static_cast<const WordDelta&>(d).new_maximum();
  }
}}

// STATISTICS: word-var
