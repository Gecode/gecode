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
    // The zero count is 64 for zero; do not shift by the word width.
    return value ?
      (~WordValue(0) >> Support::count_leading_zeros_64(value)) : 0;
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

  forceinline WordCube
  flip_cube_order(WordDomainType domain_type, unsigned int width,
                  WordCube cube) {
    if (domain_type == WDT_SIGNED) {
      const WordValue flip = sign_bit(width) & ~(cube.lo ^ cube.hi);
      cube.lo ^= flip;
      cube.hi ^= flip;
    }
    return cube;
  }

  forceinline WordCube
  order_cube(WordDomainType domain_type, unsigned int width, WordCube cube) {
    return flip_cube_order(domain_type,width,cube);
  }

  forceinline WordCube
  encode_cube(WordDomainType domain_type, unsigned int width, WordCube cube) {
    return flip_cube_order(domain_type,width,cube);
  }

  forceinline bool cube_contains(WordValue lo, WordValue hi,
                                 WordValue value, WordValue mask) {
    return ((value & ~mask) == 0) && ((value & lo) == lo) &&
      ((value & ~hi) == 0);
  }

  forceinline WordIntervalResult
  find_admitted_interval(WordCube cube, WordRankInterval interval,
                         WordValue mask) {
    WordValue first, last;
    if (!cube_successor(cube.lo,cube.hi,interval.minimum,mask,first))
      return {false,{0,0}};
    if (!cube_predecessor(cube.lo,cube.hi,interval.maximum,mask,last))
      return {false,{0,0}};
    if (first > last)
      return {false,{0,0}};
    return {true,{first,last}};
  }

  forceinline WordCube
  tighten_cube_prefix(WordCube cube, WordRankInterval interval,
                       WordValue mask) {
    const WordValue difference = interval.minimum ^ interval.maximum;
    const WordValue fixed = mask & ~low_through_highest(difference);
    cube.lo |= interval.minimum & fixed;
    cube.hi &= interval.minimum | ~fixed;
    return cube;
  }

  /// Close a cube/interval intersection without publishing any variable change
  forceinline WordDomainResult
  compute_domain_closure(unsigned int width, WordDomainType domain_type,
                          WordDomain domain) {
    const WordValue mask = width_mask(width);
    const WordCube ordered = order_cube(domain_type,width,domain.cube);
    const WordIntervalResult admitted =
      find_admitted_interval(ordered,domain.interval,mask);
    if (!admitted.is_admitted)
      return {false,{{0,0},{0,0}}};
    const WordCube tightened =
      tighten_cube_prefix(ordered,admitted.interval,mask);
    return {true,{encode_cube(domain_type,width,tightened),admitted.interval}};
  }

  /// Commit closure into local domain storage only after successful calculation
  forceinline bool synchronize_domain(unsigned int width,
                                      WordDomainType domain_type,
                                      WordValue& lo, WordValue& hi,
                                      WordValue& minimum,
                                      WordValue& maximum) {
    const WordDomainResult result =
      compute_domain_closure(width,domain_type,{{lo,hi},{minimum,maximum}});
    if (!result.is_consistent)
      return false;
    lo = result.domain.cube.lo;
    hi = result.domain.cube.hi;
    minimum = result.domain.interval.minimum;
    maximum = result.domain.interval.maximum;
    return true;
  }

  forceinline WordDomain
  intersect_word_domains(WordDomain current, WordDomain requested) {
    return {{current.cube.lo | requested.cube.lo,
             current.cube.hi & requested.cube.hi},
            {std::max(current.interval.minimum,requested.interval.minimum),
             std::min(current.interval.maximum,requested.interval.maximum)}};
  }

  forceinline ModEvent
  classify_domain_change(WordDomain current, WordDomain next) {
    const bool has_new_bits = (next.cube.lo != current.cube.lo) ||
      (next.cube.hi != current.cube.hi);
    const bool has_new_bounds =
      (next.interval.minimum != current.interval.minimum) ||
      (next.interval.maximum != current.interval.maximum);
    if (!has_new_bits && !has_new_bounds)
      return ME_WORD_NONE;
    if (next.cube.lo == next.cube.hi)
      return ME_WORD_VAL;
    if (has_new_bits) {
      if (has_new_bounds)
        return ME_WORD_DOM;
      return ME_WORD_BITS;
    }
    return ME_WORD_BND;
  }

  forceinline bool
  can_publish_bounds_only(unsigned int width, WordDomainType domain_type,
                           WordCube cube, WordRankInterval interval) {
    const WordValue mask = width_mask(width);
    const WordCube ordered = order_cube(domain_type,width,cube);
    const WordValue difference = interval.minimum ^ interval.maximum;
    const WordValue fixed = mask & ~low_through_highest(difference);
    const bool are_endpoints_admitted =
      cube_contains(ordered.lo,ordered.hi,interval.minimum,mask) &&
      cube_contains(ordered.lo,ordered.hi,interval.maximum,mask);
    const bool is_prefix_unchanged =
      ((interval.minimum & fixed & ~ordered.lo) == 0) &&
      (((~interval.minimum) & fixed & ordered.hi) == 0);
    return are_endpoints_admitted && is_prefix_unchanged;
  }

  forceinline PreparedWordDomain
  prepare_bounded_domain(unsigned int width, WordValue lo, WordValue hi,
                         WordDomainType domain_type,
                         WordValue minimum, WordValue maximum,
                         const char* location) {
    check_domain(width,lo,hi,location);
    const bool is_bounded_kind =
      (domain_type == WDT_UNSIGNED) || (domain_type == WDT_SIGNED);
    if (!is_bounded_kind)
      throw OutOfLimits(location);
    const WordValue mask = width_mask(width);
    if (((minimum | maximum) & ~mask) != 0)
      throw OutOfLimits(location);
    const WordValue rank_minimum = rank(domain_type,width,minimum);
    const WordValue rank_maximum = rank(domain_type,width,maximum);
    if (rank_minimum > rank_maximum)
      throw VariableEmptyDomain(location);
    const WordDomainResult result = compute_domain_closure(
      width,domain_type,{{lo,hi},{rank_minimum,rank_maximum}});
    if (!result.is_consistent)
      throw VariableEmptyDomain(location);
    return {width,domain_type,result.domain.cube.lo,result.domain.cube.hi,
            result.domain.interval.minimum,result.domain.interval.maximum};
  }

  forceinline PreparedWordDomain
  prepare_full_domain(unsigned int width, WordDomainType domain_type,
                      const char* location) {
    const bool is_valid_width = (width >= 1U) && (width <= 64U);
    const bool is_valid_kind = (domain_type == WDT_CUBE) ||
      (domain_type == WDT_UNSIGNED) || (domain_type == WDT_SIGNED);
    if (!is_valid_width || !is_valid_kind)
      throw OutOfLimits(location);
    const WordValue mask = width_mask(width);
    if (domain_type == WDT_CUBE)
      return {width,domain_type,0,mask,0,mask};
    const WordValue minimum = (domain_type == WDT_SIGNED) ?
      sign_bit(width) : 0;
    const WordValue maximum = (domain_type == WDT_SIGNED) ?
      (sign_bit(width)-1) : mask;
    return prepare_bounded_domain(width,0,mask,domain_type,
                                  minimum,maximum,location);
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
  BoundedWordVarImp::BoundedWordVarImp(Space& home,
                                      const PreparedWordDomain& domain)
    : WordVarImp(home,domain.width,domain.lo,domain.hi,domain.domain_type),
      _minimum(domain.rank_minimum), _maximum(domain.rank_maximum) {}
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
    const bool has_excess_bits = ((lo | hi) & ~mask()) != 0;
    const bool is_empty_cube = (lo & ~hi) != 0;
    if (has_excess_bits || is_empty_cube)
      return fail(home);
    const WordValue new_lo = _lo | lo;
    const WordValue new_hi = _hi & hi;
    if ((new_lo & ~new_hi) != 0)
      return fail(home);
    const bool is_unchanged = (new_lo == _lo) && (new_hi == _hi);
    if (is_unchanged)
      return ME_WORD_NONE;
    WordDelta d(_hi & ~new_hi,new_lo & ~_lo);
    _lo = new_lo;
    _hi = new_hi;
    return notify(home, assigned() ? ME_WORD_VAL : ME_WORD_BITS, d);
  }

  /// Intersect and classify a canonical domain before committing and notifying
  forceinline ModEvent
  BoundedWordVarImp::narrow_domain(Space& home,
                                   WordValue lo, WordValue hi,
                                   WordValue minimum, WordValue maximum) {
    const bool has_excess_bits =
      ((lo | hi | minimum | maximum) & ~mask()) != 0;
    const bool is_empty = ((lo & ~hi) != 0) || (minimum > maximum);
    if (has_excess_bits || is_empty)
      return fail(home);
    const WordDomain current = {{_lo,_hi},{_minimum,_maximum}};
    const WordDomain requested = {{lo,hi},{minimum,maximum}};
    const WordDomain intersection = intersect_word_domains(current,requested);
    // An unchanged intersection already has the current domain's closure.
    if (classify_domain_change(current,intersection) == ME_WORD_NONE)
      return ME_WORD_NONE;
    const bool is_intersection_empty =
      ((intersection.cube.lo & ~intersection.cube.hi) != 0) ||
      (intersection.interval.minimum > intersection.interval.maximum);
    if (is_intersection_empty)
      return fail(home);
    const WordDomainResult result =
      compute_domain_closure(_width,_domain_type,intersection);
    if (!result.is_consistent)
      return fail(home);
    const WordDomain next = result.domain;
    const ModEvent event = classify_domain_change(current,next);
    if (event == ME_WORD_NONE)
      return ME_WORD_NONE;
    WordDelta delta(_hi & ~next.cube.hi,next.cube.lo & ~_lo,_domain_type,
                    _minimum,_maximum,next.interval.minimum,
                    next.interval.maximum);
    _lo = next.cube.lo;
    _hi = next.cube.hi;
    _minimum = next.interval.minimum;
    _maximum = next.interval.maximum;
    return notify(home,event,delta);
  }

  /// Publish admitted endpoint changes directly when the cube stays unchanged
  forceinline ModEvent
  BoundedWordVarImp::narrow_range(Space& home,
                                  WordValue minimum, WordValue maximum) {
    const bool has_excess_bits = ((minimum | maximum) & ~mask()) != 0;
    const bool is_empty_interval = minimum > maximum;
    if (has_excess_bits || is_empty_interval)
      return fail(home);
    const WordValue new_minimum = std::max(_minimum,minimum);
    const WordValue new_maximum = std::min(_maximum,maximum);
    if (new_minimum > new_maximum)
      return fail(home);
    const bool is_unchanged =
      (new_minimum == _minimum) && (new_maximum == _maximum);
    if (is_unchanged)
      return ME_WORD_NONE;

    if (!can_publish_bounds_only(_width,_domain_type,{_lo,_hi},
                                 {new_minimum,new_maximum}))
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
