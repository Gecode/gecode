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

#ifdef _MSC_VER

#include <intrin.h>

#if defined(_M_IX86)
#pragma intrinsic(_BitScanForward)
#define GECODE_SUPPORT_MSVC_32
#endif

#if defined(_M_X64) || defined(_M_IA64)
#pragma intrinsic(_BitScanForward64)
#define GECODE_SUPPORT_MSVC_64
#endif

#endif

namespace Gecode { namespace Support {

  class BitSetBase;

  /// Date item for bitsets
  class BitSetData {
    friend class BitSetBase;
  protected:
#ifdef GECODE_SUPPORT_MSVC_64
    /// Basetype for bits
    typedef unsigned __int64 Base;
#else
    /// Basetype for bits
    typedef unsigned long int Base;
#endif
    /// The bits
    Base bits;
    /// Bits per base
    static const unsigned int bpb = 
      static_cast<unsigned int>(CHAR_BIT * sizeof(Base));
  public:
    /// Initialize (no bit is set)
    void init(void);
    /// Get number of data elements for \a s bits
    static unsigned int data(unsigned int s);
    /// Test wether any bit with position greater or equal to \a i is set
    bool operator ()(unsigned int i=0U) const;
    /// Access value at bit \a i
    bool get(unsigned int i) const;
    /// Set bit \a i
    void set(unsigned int i);
    /// Clear bit \a i
    void clear(unsigned int i);
    /// Return next set bit with position greater or equal to \a i (there must be a bit)
    unsigned int next(unsigned int i=0U) const;
  };

  /// Basic bitset support
  class BitSetBase {
  protected:
    /// Bits per base
    static const unsigned int bpb = BitSetData::bpb;
    /// Size of bitset (number of bits)
    unsigned int sz;
    /// Stored bits
    BitSetData* data;
    /// Dispose memory for bit set
    template<class A>
    void dispose(A& a);
    /// Access value at bit \a i (no index check)
    bool _get(unsigned int i) const;
    /// Set bit \a i (no index check)
    void _set(unsigned int i);
  public:
    /// Default constructor (yields empty set)
    BitSetBase(void);
    /// Initialize for \a s bits and allocator \a a
    template<class A>
    BitSetBase(A& a, unsigned int s);
    /// Copy from bitset \a bs with allocator \a a
    template<class A>
    BitSetBase(A& a, const BitSetBase& bs);
    /// Initialize for \a s bits and allocator \a a (only after default constructor)
    template<class A>
    void init(A& a, unsigned int s);
    /// Return size of bitset (number of bits)
    unsigned int size(void) const;
    /// Access value at bit \a i
    bool get(unsigned int i) const;
    /// Set bit \a i
    void set(unsigned int i);
    /// Clear bit \a i
    void clear(unsigned int i);
    /// Return position greater or equal \a i of next set bit (\a i is allowed to be equal to size)
    unsigned int next(unsigned int i) const;
  };


  /*
   * Bitset data
   *
   */

  forceinline void
  BitSetData::init(void) {
    bits = static_cast<Base>(0);
  }
  forceinline unsigned int
  BitSetData::data(unsigned int s) {
    return (s+bpb-1) / bpb;
  }
  forceinline bool
  BitSetData::operator ()(unsigned int i) const {
    return (bits >> i) != static_cast<Base>(0U);
  }
  forceinline bool
  BitSetData::get(unsigned int i) const {
    return (bits & (static_cast<Base>(1U) << i)) != static_cast<Base>(0U);
  }
  forceinline void
  BitSetData::set(unsigned int i) {
    bits |= (static_cast<Base>(1U) << i);
  }
  forceinline void
  BitSetData::clear(unsigned int i) {
    bits &= ~(static_cast<Base>(1U) << i);
  }
  forceinline unsigned int
  BitSetData::next(unsigned int i) const {
    assert(bits != static_cast<Base>(0));
#if defined(GECODE_SUPPORT_MSVC_32)
    assert(bpb == 32);
    unsigned long int p;
    _BitScanForward(&p,bits >> i);
    return static_cast<unsigned int>(p)+i;
#elif defined(GECODE_SUPPORT_MSVC_64)
    assert(bpb == 64);
    unsigned long int p;
    _BitScanForward64(&p,bits >> i);
    return static_cast<unsigned int>(p)+i;
#else
    if ((bpb == 32) || (bpb == 64)) {
      Base b = bits >> i;
      if ((bpb == 64) && ((b & 0xFFFFFFFFUL) == 0UL)) {
        b >>= 32; i += 32U;
      }
      if ((b & 0xFFFFUL) == 0UL) {
        b >>= 16; i += 16U;
      }
      if ((b & 0xFFUL) == 0UL) {
        b >>= 8; i += 8U;
      }
      if ((b & 0xFUL) == 0UL) {
        b >>= 4; i += 4U;
      }
      if ((b & 0x3UL) == 0UL) {
        b >>= 2; i += 2U;
      }
      if ((b & 0x1UL) == 0UL) {
        i += 1;
      }
      return i;
    } else {
      while (!get(i)) i++;
      return i;
    }
#endif
  }



  /*
   * Basic bit sets
   *
   */

  forceinline bool
  BitSetBase::_get(unsigned int i) const {
    return data[i / bpb].get(i % bpb);
  }
  forceinline void
  BitSetBase::_set(unsigned int i) {
    data[i / bpb].set(i % bpb);
  }

  template<class A>
  forceinline void
  BitSetBase::dispose(A& a) {
    a.template free<BitSetData>(data,BitSetData::data(sz+1));
  }

  forceinline
  BitSetBase::BitSetBase(void)
    : sz(0), data(NULL) {}

  template<class A>
  forceinline
  BitSetBase::BitSetBase(A& a, unsigned int s)
    : sz(s), 
      data(a.template alloc<BitSetData>(BitSetData::data(sz+1))) {
    for (unsigned int i = BitSetData::data(sz+1); i--; ) 
      data[i].init();
    // Set a bit at position sz as sentinel (for efficient next)
    _set(sz);
  }

  template<class A>
  forceinline
  BitSetBase::BitSetBase(A& a, const BitSetBase& bs)
    : sz(bs.sz), 
      data(a.template alloc<BitSetData>(BitSetData::data(sz+1))) {
    for (unsigned int i = BitSetData::data(sz+1); i--; ) 
      data[i] = bs.data[i];
    // Set a bit at position sz as sentinel (for efficient next)
    _set(sz);
  }

  template<class A>
  forceinline void
  BitSetBase::init(A& a, unsigned int s) {
    assert((sz == 0) && (data == NULL));
    sz=s; data=a.template alloc<BitSetData>(BitSetData::data(sz+1));
    for (unsigned int i=BitSetData::data(sz+1); i--; )
      data[i].init();
    // Set a bit at position sz as sentinel (for efficient next)
    _set(sz);
  }

  forceinline unsigned int
  BitSetBase::size(void) const {
    return sz;
  }

  forceinline bool
  BitSetBase::get(unsigned int i) const {
    assert(i < sz);
    return _get(i);
  }
  forceinline void
  BitSetBase::set(unsigned int i) {
    assert(i < sz);
    _set(i);
  }
  forceinline void
  BitSetBase::clear(unsigned int i) {
    assert(i < sz);
    data[i / bpb].clear(i % bpb);
  }

  forceinline unsigned int
  BitSetBase::next(unsigned int i) const {
    assert(i <= sz);
    unsigned int pos = i / bpb;
    unsigned int bit = i % bpb;
    if (data[pos](bit))
      return pos * bpb + data[pos].next(bit);
    // The sentinel bit guarantees that this loop always terminates
    do { 
      pos++; 
    } while (!data[pos]());
    return pos * bpb + data[pos].next();
  }

}}

#ifdef GECODE_SUPPORT_MSVC_32
#undef GECODE_SUPPORT_MSVC_32
#endif
#ifdef GECODE_SUPPORT_MSVC_64
#undef GECODE_SUPPORT_MSVC_64
#endif

// STATISTICS: support-any

