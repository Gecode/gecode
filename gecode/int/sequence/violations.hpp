/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Last modified:
 *     $Date$
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
#include <cmath>

namespace Gecode { namespace Int { namespace Sequence {

  /// Simple bitsets
  class Violations {
    /// Basetype for bits
    typedef unsigned int Base;
    /// Size of bitset (number of bits)
    int sz;
    /// Stored bits
    Base* data;
    /// The (possibly) first set bit (set is empty if fst == sz)
    mutable int fst;
    /// Get number of Base elements for \a n bits
    static int base(int n);
    /// Access value at bit \a i
    bool get(int i) const;
    /// Set bit \a i
    void set(int i);
    /// Clear bit \a i
    void clear(int i);
  public:
    /// Default constructor
    Violations(void);
    /// Initialize violation set for \a n violations
    void init(Space& home, int n);
    /// Update violation set during cloning
    void update(Space& home, bool shared, Violations& v);
    /// Return whether set is empty
    bool empty(void) const;
    /// Add \a i to violation set
    void add(int i);
    /// Get first element from violation set and remove it
    int get(void);
  };


  forceinline int
  Violations::base(int n) {
    return static_cast<int>(std::ceil(static_cast<double>(n)
                                      /(CHAR_BIT*sizeof(Base))));
  }

  forceinline
  Violations::Violations(void)
    : sz(0), data(NULL), fst(0) {}

  forceinline void
  Violations::init(Space& home, int n) {
    sz = n;
    data = home.alloc<Base>(base(sz));
    for (int i=base(sz); i--; ) 
      data[i] = 0;
    fst = sz;
  }

  forceinline bool
  Violations::get(int i) const {
    assert(i < sz);
    int pos = i / static_cast<int>(sizeof(Base)*CHAR_BIT);
    int bit = i % static_cast<int>(sizeof(Base)*CHAR_BIT);
    return (data[pos] & ((Base)1 << bit)) != 0;
  }

  forceinline void
  Violations::set(int i) {
    assert(i < sz);
    int pos = i / static_cast<int>(sizeof(Base)*CHAR_BIT);
    int bit = i % static_cast<int>(sizeof(Base)*CHAR_BIT);
    data[pos] |= 1 << bit;
  }

  forceinline void
  Violations::clear(int i) {
    assert(i < sz);
    int pos = i / static_cast<int>(sizeof(Base)*CHAR_BIT);
    int bit = i % static_cast<int>(sizeof(Base)*CHAR_BIT);
    data[pos] &= ~(1 << bit);
  }

  forceinline bool
  Violations::empty(void) const {
    while (fst < sz)
      if (get(fst))
        return false;
      else
        fst++;
    return true;
  }
  
  forceinline void
  Violations::update(Space& home, bool share, Violations& v) {
    assert(v.empty());
    sz = v.sz;
    data = home.alloc<Base>(base(sz));
    for (int i=base(sz); i--; ) 
      data[i] = 0;
    fst = sz;
  }

  forceinline void
  Violations::add(int i) {
    set(i); if (i < fst) fst = i;
  }
  
  forceinline int
  Violations::get(void) {
    assert(!empty());
    while (!get(fst)) 
      fst++;
    clear(fst); fst++;
    return fst-1;
  }
  
}}}

// STATISTICS: int-prop
