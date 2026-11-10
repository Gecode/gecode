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
 */

#ifndef GECODE_WORD_ARITHMETIC_GLOBAL_DOMAIN_HPP
#define GECODE_WORD_ARITHMETIC_GLOBAL_DOMAIN_HPP

#include <gecode/word/arithmetic/global-integer.hpp>

#include <type_traits>
#include <utility>

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {

  /*
   * An owned, growable array whose elements live in a block on Gecode::heap.
   * Copies are deep. Only the first size() elements are constructed, so T
   * needs a default constructor only for Array(size_t).
   */
  template<class T>
  class Array {
    static_assert(std::is_nothrow_move_constructible<T>::value,
                  "growth must not leave a partly moved array");
    T* elements;
    size_t n, capacity;
  public:
    Array(void) : elements(nullptr), n(0), capacity(0) {}
    /// An array of \a m value-initialised elements
    explicit Array(size_t m) : Array() {
      reserve(m);
      for (; n<m; n++) new (&elements[n]) T();
    }
    Array(const Array& o) : Array() {
      reserve(o.n);
      for (; n<o.n; n++) new (&elements[n]) T(o.elements[n]);
    }
    Array(Array&& o) noexcept
      : elements(o.elements), n(o.n), capacity(o.capacity) {
      o.elements=nullptr; o.n=0; o.capacity=0;
    }
    Array& operator=(const Array& o) { return *this=Array(o); }
    Array& operator=(Array&& o) noexcept {
      std::swap(elements,o.elements); std::swap(n,o.n);
      std::swap(capacity,o.capacity);
      return *this;
    }
    ~Array(void) {
      for (size_t i=0; i<n; i++) elements[i].~T();
      if (elements) heap.rfree(elements);
    }
    size_t size(void) const { return n; }
    T& operator[](size_t i) { assert(i<n); return elements[i]; }
    const T& operator[](size_t i) const { assert(i<n); return elements[i]; }
    T* begin(void) { return elements; }
    T* end(void) { return elements+n; }
    const T* begin(void) const { return elements; }
    const T* end(void) const { return elements+n; }
    /// Grow the storage to hold at least \a m elements
    void reserve(size_t m) {
      if (m<=capacity) return;
      T* block=static_cast<T*>(heap.ralloc(sizeof(T)*m));
      for (size_t i=0; i<n; i++) {
        new (&block[i]) T(std::move(elements[i])); elements[i].~T();
      }
      if (elements) heap.rfree(elements);
      elements=block; capacity=m;
    }
    // Takes x by value, so pushing one of this array's own elements is safe.
    void push_back(T x) {
      if (n==capacity) reserve(std::max<size_t>(4,2*capacity));
      new (&elements[n]) T(std::move(x)); n++;
    }
  };

  const Integer zero, one(1);
  // Convert an interval between unsigned and signed rank orders. Flipping
  // the sign bit splits it into at most two intervals. Cube endpoint queries
  // compute their hull without visiting any represented values.
  // Hull [first,last] of the values of an ordered cube in a union of ranges.
  struct CubeHull {
    WordCube ordered;
    WordValue mask;
    bool found=false;
    WordValue first=0,last=0;
    // Extend the hull by the cube values in [l,u].
    void include(WordValue l,WordValue u) {
      WordValue a,b;
      if (l>u || !cube_successor(ordered.lo,ordered.hi,l,mask,a) || a>u ||
          !cube_predecessor(ordered.lo,ordered.hi,u,mask,b) || b<a) return;
      first=found ? std::min(first,a) : a;
      last=found ? std::max(last,b) : b;
      found=true;
    }
  };
  inline bool rank_hull(unsigned int width,WordValue lo,WordValue hi,
                 WordValue lower,WordValue upper,WordValue source_sign,
                 WordValue target_sign,WordValue& first,WordValue& last) {
    CubeHull hull{order_cube(target_sign ? WDT_SIGNED : WDT_UNSIGNED,width,{lo,hi}),
                  width_mask(width)};
    if (source_sign==target_sign) hull.include(lower,upper);
    else {
      const WordValue bit=WordValue(1)<<(width-1);
      if (lower<bit) hull.include(lower^bit,std::min(upper,bit-1)^bit);
      if (upper>=bit) hull.include(std::max(lower,bit)^bit,upper^bit);
    }
    if (hull.found) { first=hull.first; last=hull.last; }
    return hull.found;
  }
  struct Domain {
    WordValue lo,hi,mask,sign;
    unsigned int width;
    WordDomainType kind;
    Integer lower,upper,residue,step;
    /// Read \a x as a signed or unsigned number, using its rank interval
    /// only when that interval is in an integer order
    Domain(WordView x,bool signed_value)
      : lo(x.lo()),hi(x.hi()),mask(x.mask()),
        sign(signed_value ? WordValue(1)<<(x.width()-1) : 0),
        width(x.width()),kind(signed_value ? WDT_SIGNED : WDT_UNSIGNED),
        step(1) {
      WordRankInterval source={0,mask};
      WordValue source_sign=0;
      switch (integer_rank_order(x.domain_type())) {
      case IntegerRankOrder::NONE: case IntegerRankOrder::NON_INTEGER:
        break;
      case IntegerRankOrder::UNSIGNED:
        source={x.rank_minimum(),x.rank_maximum()};
        break;
      case IntegerRankOrder::SIGNED:
        source={x.rank_minimum(),x.rank_maximum()};
        source_sign=WordValue(1)<<(width-1);
        break;
      }
      WordValue first=0,last=0;
      bool ok=rank_hull(width,lo,hi,source.minimum,source.maximum,
                        source_sign,sign,first,last);
      assert(ok); (void) ok;
      lower=Integer(first)-Integer(sign); upper=Integer(last)-Integer(sign);
    }
    ModEvent publish(Space& home,WordView x) const {
      switch (integer_rank_order(x.domain_type())) {
      case IntegerRankOrder::NONE: case IntegerRankOrder::NON_INTEGER:
        return x.narrow(home,lo,hi);
      case IntegerRankOrder::UNSIGNED:
        return publish_ranks(home,UnsignedWordView(x.varimp()),0);
      case IntegerRankOrder::SIGNED:
        return publish_ranks(home,SignedWordView(x.varimp()),
                             WordValue(1)<<(width-1));
      }
      GECODE_NEVER;
      return ME_WORD_FAILED;
    }
    /// Narrow the typed \a x, whose ranks have sign bit \a target_sign
    template<class View>
    ModEvent publish_ranks(Space& home,View x,WordValue target_sign) const {
      WordValue first=0,last=0;
      if (!rank_hull(width,lo,hi,lower_rank(),upper_rank(),sign,target_sign,
                     first,last))
        return ME_WORD_FAILED;
      return narrow_word_domain(home,x,{lo,hi},{first,last});
    }
    Integer number(WordValue encoded) const {
      return sign && (encoded&sign) ? Integer(encoded)-Integer::power_two(width) :
        Integer(encoded);
    }
    Integer base(void) const { return number(lo); }
    /// Return the rank of \a value, which must lie within this word's range
    WordValue rank(const Integer& value) const {
      return (value+Integer(sign)).word();
    }
    WordValue lower_rank(void) const { return rank(lower); }
    WordValue upper_rank(void) const { return rank(upper); }
    bool sync(void) {
      if ((lo & ~hi) || lower>upper) return false;
      WordValue first,last;
      WordCube ordered=order_cube(kind,width,{lo,hi});
      WordValue& ordered_lo=ordered.lo;
      WordValue& ordered_hi=ordered.hi;
      if (!cube_successor(ordered_lo,ordered_hi,lower_rank(),mask,first) ||
          first>upper_rank() ||
          !cube_predecessor(ordered_lo,ordered_hi,upper_rank(),mask,last) || last<first) return false;
      lower=Integer(first)-Integer(sign); upper=Integer(last)-Integer(sign);
      WordValue varying=first^last;
      varying|=varying>>1; varying|=varying>>2; varying|=varying>>4;
      varying|=varying>>8; varying|=varying>>16; varying|=varying>>32;
      ordered_lo|=first&~varying; ordered_hi&=last|varying;
      const WordCube encoded=encode_cube(kind,width,ordered);
      lo=encoded.lo; hi=encoded.hi;
      return true;
    }
    bool range(Integer l,Integer u) {
      l=std::max(l,Integer(lower)); u=std::min(u,Integer(upper));
      if (l>u) return false;
      lower=l; upper=u; return sync();
    }
    bool assigned(void) const { return lower==upper; }
    unsigned int prefix(void) const {
      WordValue unknown=lo^hi;
      unsigned int k=0;
      while (k<64 && !(unknown & (WordValue(1)<<k))) k++;
      return k;
    }
  };
  typedef Array<Domain> Domains;

  // Intersect a*x=b (mod m). The unary progression is kept for this pass;
  // only its interval/bit hull can be published to a WordVar.
  inline bool congruence(Domain& x,Integer a,Integer b,Integer m) {
    assert(m>zero);
    Integer g=gcd(a,m);
    if (!mod(b,g).zero()) return false;
    m=m/g;
    if (m==one) return true;
    Integer r=mod((b/g)*inverse(a/g,m),m);
    // Generalized CRT also joins different rows within this propagation pass.
    g=gcd(x.step,m);
    if (!mod(r-x.residue,g).zero()) return false;
    Integer reduced=m/g;
    Integer multiple=reduced==one ? zero :
      mod(((r-x.residue)/g)*inverse(x.step/g,reduced),reduced);
    r=x.residue+x.step*multiple; m=x.step*reduced; r=mod(r,m);
    x.residue=r; x.step=m;
    Integer first=r+ceil_div(Integer(x.lower)-r,m)*m;
    Integer last=r+floor_div(Integer(x.upper)-r,m)*m;
    if (first>last || !x.range(first,last)) return false;
    // The 2-primary part of any modulus fixes a low bit prefix.
    unsigned int k=0;
    while (k<64 && !m.bit(k)) k++;
    k=std::min(k,x.width);
    WordValue bits=k==64 ? ~WordValue(0) : (WordValue(1)<<k)-1;
    WordValue residue=mod(r,Integer::power_two(k)).word();
    x.lo|=residue; x.hi&=~bits|residue;
    return x.sync();
  }

}}}}

#endif

// STATISTICS: word-prop
