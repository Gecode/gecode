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

  /// Basic bitset support
  class BitSetBase {
  protected:
    /// Basetype for bits
    typedef unsigned int Base;
    /// Bits per base
    static const int bpb = static_cast<int>(CHAR_BIT * sizeof(Base));
    /// Size of bitset (number of bits)
    int sz;
    /// Stored bits
    Base* data;
    /// Get number of Base elements for \a s bits
    static int base(int s);
    /// Dispose memory for bit set
    template<class A>
    void dispose(A& a);
  public:
    /// Default constructor (yields empty set)
    BitSetBase(void);
    /// Initialize for \a s bits and allocator \a a
    template<class A>
    BitSetBase(A& a, int s);
    /// Copy from bitset \a bs with allocator \a a
    template<class A>
    BitSetBase(A& a, const BitSetBase& bs);
    /// Initialize for \a s bits and allocator \a a (only after default constructor)
    template<class A>
    void init(A& a, int s);
    /// Return size of bitset (number of bits)
    int size(void) const;
    /// Access value at bit \a i
    bool get(int i) const;
    /// Set bit \a i
    void set(int i);
    /// Clear bit \a i
    void clear(int i);
  };

  forceinline int
  BitSetBase::base(int s) {
    return (s+bpb-1) / bpb;
  }

  template<class A>
  forceinline void
  BitSetBase::dispose(A& a) {
    a.template free<Base>(data,base(sz));
  }

  forceinline
  BitSetBase::BitSetBase(void)
    : sz(0), data(NULL) {}

  template<class A>
  forceinline
  BitSetBase::BitSetBase(A& a, int s)
    : sz(s), data(a.template alloc<Base>(base(sz))) {
    for (int i=base(sz); i--; ) 
      data[i] = 0;
  }

  template<class A>
  forceinline
  BitSetBase::BitSetBase(A& a, const BitSetBase& bs)
    : sz(bs.sz), data(a.template alloc<Base>(base(bs.sz))) {
    for (int i = base(sz); i--; ) 
      data[i] = bs.data[i];
  }

  template<class A>
  forceinline void
  BitSetBase::init(A& a, int s) {
    assert((sz == 0) && (data == NULL));
    sz=s; data=a.template alloc<Base>(base(sz));
    for (int i=base(sz); i--; )
      data[i] = 0;
  }

  forceinline int
  BitSetBase::size(void) const {
    return sz;
  }

  forceinline bool
  BitSetBase::get(int i) const {
    assert((i >= 0) && (i < sz));
    if (bpb == 8) {
    } else {
      int pos = i / bpb;
      int bit = i % bpb;
    return (data[pos] & (static_cast<Base>(1) << bit)) != 0;
  }

  forceinline void
  BitSetBase::set(int i) {
    assert((i >= 0) && (i < sz));
    int pos = i / bpb;
    int bit = i % bpb;
    data[pos] |= 1 << bit;
  }

  forceinline void
  BitSetBase::clear(int i) {
    assert(i < sz);
    int pos = i / bpb;
    int bit = i % bpb;
    data[pos] &= ~(1 << bit);
  }

}}

// STATISTICS: support-any

