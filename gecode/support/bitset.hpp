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
    /// Size of bitset
    int size;
  public:
    /// BitSet with space for \a s bits
    BitSet(A& a, int s);
    /// Copy BitSet \a bs
    BitSet(A& a, const BitSet& bs);
    /// Initialize BitSet for \a s bits. The bits are  set to \a value.
    void init(int s, bool value=false);
    /// Access value at bit \a i
    bool get(int i) const;
    /// Set bit \a i
    void set(int i);
  };


  template<class A>
  forceinline
  BitSet<A>::BitSet(A& a0, int s)
    : a(a0) {
    size = static_cast<int>(std::ceil(static_cast<double>(s)
                                      /(CHAR_BIT*sizeof(Base))));
    data = a.alloc<Base>(size);
    for (int i = size; i--; ) data[i] = 0;
  }

  template<class A>
  forceinline
  BitSet<A>::BitSet(A& a0, const BitSet<A>& bs)
    : a(a0), data(a.alloc<Base>(bs.size)), size(bs.size) {
    for (int i = size; i--; ) 
      data[i] = bs.data[i];
  }

  template<class A>
  forceinline bool
  BitSet<A>::get(int i) const {
    int pos = i / (sizeof(Base)*CHAR_BIT);
    int bit = i % (sizeof(Base)*CHAR_BIT);
    assert(pos < size);
    return (data[pos] & ((Base)1 << bit)) != 0;
  }

  template<class A>
  forceinline void
  BitSet<A>::set(int i) {
    int pos = i / (sizeof(Base)*CHAR_BIT);
    int bit = i % (sizeof(Base)*CHAR_BIT);
    assert(pos < size);
    data[pos] |= 1 << bit;
  }

}}

// STATISTICS: support-any

