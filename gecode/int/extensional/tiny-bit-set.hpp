/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Christian Schulte, 2017
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

namespace Gecode { namespace Int { namespace Extensional {

  /*
   * Tiny bit-set
   *
   */
  template<unsigned int sz>
  forceinline
  TinyBitSet<sz>::TinyBitSet(Space&, unsigned int n) {
    assert(n <= sz);
    /// Set the active bits
      for (unsigned int i=0U; i<n; i++)
        _bits[i].init(true);
      /// Clear unused suffix bits
        for (unsigned int i=n; i<sz; i++)
          _bits[i].init(false);
  }

  template<unsigned int sz>
  template<unsigned int largersz>
  forceinline
  TinyBitSet<sz>::TinyBitSet(Space&, const TinyBitSet<largersz>& sbs) {
    GECODE_ASSUME(sz <= largersz);
    assert(!sbs.empty());
    for (unsigned int i=0U; i<sz; i++)
      _bits[i] = sbs._bits[i];
    assert(!empty());
  }
      
  template<unsigned int sz>
  template<class IndexType>
  forceinline
  TinyBitSet<sz>::TinyBitSet(Space&, const BitSet<IndexType>& sbs) {
    assert(sz == sbs.width());
    assert(!sbs.empty());
    for (unsigned int i=0U; i<sz; i++)
      _bits[i].init(false);
    for (unsigned int i=0U; i<sbs.words(); i++)
      _bits[sbs._index[i]] = sbs._bits[i];
    assert(!empty());
  }

  template<unsigned int sz>
  forceinline void
  TinyBitSet<sz>::clear_mask(BitSetData* mask) {
    for (unsigned int i=0U; i<sz; i++) {
      mask[i].init(false);
      assert(mask[i].none());
    }
  }

  template<unsigned int sz>
  forceinline void
  TinyBitSet<sz>::add_to_mask(const BitSetData* b, BitSetData* mask) const {
    for (unsigned int i=0U; i<sz; i++)
      mask[i] = BitSetData::o(mask[i],b[i]);
  }

  template<unsigned int sz>
  template<bool sparse>
  forceinline void
  TinyBitSet<sz>::intersect_with_mask(const BitSetData* mask) {
    for (unsigned int i=0U; i<sz; i++)
      _bits[i] = BitSetData::a(_bits[i], mask[i]);
  }

  template<unsigned int sz>
  forceinline void
  TinyBitSet<sz>::intersect_with_masks(const BitSetData* a,
                                       const BitSetData* b) {
    for (unsigned int i=0U; i<sz; i++)
      _bits[i] = BitSetData::a(_bits[i], BitSetData::o(a[i],b[i]));
  }

  template<unsigned int sz>
  forceinline void
  TinyBitSet<sz>::nand_with_mask(const BitSetData* b) {
    for (unsigned int i=0U; i<sz; i++)
      _bits[i] = BitSetData::a(_bits[i],~(b[i]));
  }

  template<unsigned int sz>
  forceinline void
  TinyBitSet<sz>::flush(void) {
    for (unsigned int i=0U; i<sz; i++)
      _bits[i].init(false);
    assert(empty());
  }
  
  template<unsigned int sz>
  forceinline bool
  TinyBitSet<sz>::intersects(const BitSetData* b) {
    for (unsigned int i=0U; i<sz; i++)
      if (!BitSetData::a(_bits[i],b[i]).none())
        return true;
    return false;
  }

  template<unsigned int sz>
  forceinline unsigned long long int
  TinyBitSet<sz>::ones(const BitSetData* b) const {
    unsigned long long int o = 0U;
    for (unsigned int i=0U; i<sz; i++)
      o += static_cast<unsigned long long int>
        (BitSetData::a(_bits[i],b[i]).ones());
    return o;
  }
    
  template<unsigned int sz>
  forceinline unsigned long long int
  TinyBitSet<sz>::ones(void) const {
    unsigned long long int o = 0U;
    for (unsigned int i=0U; i<sz; i++)
      o += static_cast<unsigned long long int>(_bits[i].ones());
    return o;
  }
    
  template<unsigned int sz>
  forceinline unsigned long long int
  TinyBitSet<sz>::bits(void) const {
    return (static_cast<unsigned long long int>(sz) *
            static_cast<unsigned long long int>(BitSetData::bpb));
  }
    
  template<unsigned int sz>
  forceinline bool
  TinyBitSet<sz>::empty(void) const { // Linear complexity...
    for (unsigned int i=0U; i<sz; i++)
      if (!_bits[i].none())
        return false;
    return true;
  }

  template<unsigned int sz>
  forceinline unsigned int
  TinyBitSet<sz>::width(void) const {
    assert(!empty());
    /// Find the index of the last non-zero word
      for (unsigned int i=sz; i--; )
        if (!_bits[i].none())
          return i+1U;
      GECODE_NEVER;
      return 0U;
  }
      
  template<unsigned int sz>
  forceinline unsigned int
  TinyBitSet<sz>::words(void) const {
    return width();
  }

  template<unsigned int sz>
  forceinline unsigned int
  TinyBitSet<sz>::size(void) const {
    return sz;
  }

}}}

// STATISTICS: int-prop
