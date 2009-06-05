/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
 *     Christian Schulte, 2007
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#include <climits>

namespace Gecode { namespace Support {

  /// Simple bitsets
  template<class A>
  class BitSet {
    /// Allocator
    A& a;
    /// Basetype for bits
    typedef unsigned int Base;
    /// Stored bits
    Base* data;
    /// Size of bitset (number of bits)
    int sz;
    /// Get number of Base elements for \a s bits
    static int base(int s);
  public:
    /// BitSet with space for \a s bits
    BitSet(A& a, int s);
    /// Copy BitSet \a bs
    BitSet(A& a, const BitSet& bs);
    /// Access value at bit \a i
    bool get(int i) const;
    /// Set bit \a i
    void set(int i);
    /// Return size of bitset (number of bits)
    int size(void) const;
  };


  template<class A>
  forceinline int
  BitSet<A>::base(int s) {
    return static_cast<int>(std::ceil(static_cast<double>(s)
                                      /(CHAR_BIT*sizeof(Base))));
  }

  template<class A>
  forceinline
  BitSet<A>::BitSet(A& a0, int s)
    : a(a0), sz(s) {
    data = a.alloc<Base>(base(sz));
    for (int i=base(sz); i--; ) data[i] = 0;
  }

  template<class A>
  forceinline
  BitSet<A>::BitSet(A& a0, const BitSet<A>& bs)
    : a(a0), data(a.alloc<Base>(base(bs.sz))), sz(bs.sz) {
    for (int i = base(sz); i--; ) 
      data[i] = bs.data[i];
  }

  template<class A>
  forceinline bool
  BitSet<A>::get(int i) const {
    assert(i < sz);
    int pos = i / (sizeof(Base)*CHAR_BIT);
    int bit = i % (sizeof(Base)*CHAR_BIT);
    return (data[pos] & ((Base)1 << bit)) != 0;
  }

  template<class A>
  forceinline void
  BitSet<A>::set(int i) {
    assert(i < sz);
    int pos = i / (sizeof(Base)*CHAR_BIT);
    int bit = i % (sizeof(Base)*CHAR_BIT);
    data[pos] |= 1 << bit;
  }

  template<class A>
  forceinline int
  BitSet<A>::size(void) const {
    return sz;
  }

}}

// STATISTICS: support-any

