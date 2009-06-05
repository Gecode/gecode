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

namespace Gecode { namespace Int { namespace Extensional {

  /// Simple bitsets
  class BitSet {
    /// Basetype for bits
    typedef unsigned int Base;
    /// Stored bits
    Base* data;
    /// Size of bitset
    unsigned int size;
  public:
    /// Default (empty) initialization of BitSet
    BitSet(void);
    /// BitSet with space for \a s bits. The bits are  set to \a value.
    BitSet(Space& home, unsigned int s, bool value = false);
    /// Copy BitSet \a bs
    BitSet(Space& home, const BitSet& bs);
    /// Initialize BitSet for \a s bits. The bits are  set to \a value.
    void init(Space& home, unsigned int s, bool value=false);
    /// Access value at bit \a i
    bool get(unsigned int i) const;
    /// Set value at bit \a i to \a value
    void set(unsigned int i, bool value=true);
  };

  forceinline void
  BitSet::init(Space& home, unsigned int s, bool value) {
    size = static_cast<unsigned int>(std::ceil(static_cast<double>(s)
                                               /(CHAR_BIT*sizeof(Base))));
    data = home.alloc<Base>(size);
    Base ival = value ? ~0 : 0;
    for (unsigned int i = size; i--; ) data[i] = ival;
  }

  forceinline
  BitSet::BitSet(void) : data(NULL), size(0) {}

  forceinline
  BitSet::BitSet(Space& home, unsigned int s, bool value)
    : data(NULL), size(0) {
    init(home, s, value);
  }
  forceinline
  BitSet::BitSet(Space& home, const BitSet& bs)
    : data(home.alloc<Base>(bs.size)), size(bs.size) {
    for (unsigned int i = size; i--; ) data[i] = bs.data[i];
  }
  forceinline bool
  BitSet::get(unsigned int i) const {
    unsigned int pos = i / (sizeof(Base)*CHAR_BIT);
    unsigned int bit = i % (sizeof(Base)*CHAR_BIT);
    assert(pos < size);
    return (data[pos] & ((Base)1 << bit)) != 0;
  }
  forceinline void
  BitSet::set(unsigned int i, bool value) {
    unsigned int pos = i / (sizeof(Base)*CHAR_BIT);
    unsigned int bit = i % (sizeof(Base)*CHAR_BIT);
    assert(pos < size);
    if (value)
      data[pos] |= 1 << bit;
    else
      data[pos] &= ~(1 << bit);
  }

}}}

// STATISTICS: int-other

