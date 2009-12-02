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
#include <cmath>

namespace Gecode { namespace Int { namespace Extensional {

  /// Simple bitsets
  class BitSet : public Support::BitSetBase {
  public:
    /// Default (empty) initialization of BitSet
    BitSet(void);
    /// BitSet with space for \a s bits
    BitSet(Space& home, unsigned int s);
    /// Copy BitSet \a bs
    BitSet(Space& home, const BitSet& bs);
    /// Initialize BitSet for \a s bits
    void init(Space& home, unsigned int s);
    /// Access value at bit \a i
    bool get(unsigned int i) const;
    /// Set value at bit \a i
    void set(unsigned int i);
    /// Clear value at bit \a i
    void clear(unsigned int i);
  };

  forceinline
  BitSet::BitSet(void) {}
  forceinline
  BitSet::BitSet(Space& home, unsigned int s)
    : Support::BitSetBase(home,s) {}
  forceinline
  BitSet::BitSet(Space& home, const BitSet& bs)
    : Support::BitSetBase(home,bs) {}

  forceinline void
  BitSet::init(Space& home, unsigned int s) {
    Support::BitSetBase::init(home,static_cast<int>(s));
  }

  forceinline bool
  BitSet::get(unsigned int i) const {
    return Support::BitSetBase::get(static_cast<int>(i));
  }
  forceinline void
  BitSet::set(unsigned int i) {
    Support::BitSetBase::set(static_cast<int>(i));
  }
  forceinline void
  BitSet::clear(unsigned int i) {
    Support::BitSetBase::clear(static_cast<int>(i));
  }

}}}

// STATISTICS: int-other

