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

namespace Gecode { namespace Int { namespace Extensional {

  /*
   * Entry for sparse bit-set
   *
   */
  forceinline unsigned int
  SparseBitSet::Entry::data(unsigned int n) {
    assert(n != 0);
    return ((n - 1)) / 2 + 1;
  }
  
  /*
   * Data for sparse bit-set
   *
   */
  forceinline SparseBitSet::Data*
  SparseBitSet::Data::allocate(Space& home, unsigned int n) {
    return static_cast<Data*>
      (home.ralloc(sizeof(Data) + (n-1)*sizeof(Entry)));
  }
  
  forceinline void
  SparseBitSet::Data::init(unsigned int n) {
    assert(n > 0U);
    limit = n;
    // Set all bits in all words (including the last)
    for (unsigned int i = 0U; i <= ((limit-1)>>1); i++) {
      es[i].bits[0].init(true); es[i].bits[1].init(true);
      es[i].index[0] = 2*i; es[i].index[1] = 2*i+1;
    }
  }
  
  forceinline BitSetData
  SparseBitSet::Data::word(unsigned int i) const {
    assert(i < limit);
    return es[i>>1].bits[i&1U];
  }
  
  forceinline unsigned int
  SparseBitSet::Data::index(unsigned int i) const {
    assert(i < limit);
    return es[i>>1].index[i&1U];
  }

  forceinline unsigned int
  SparseBitSet::Data::width(void) const {
    assert(limit > 0);
    unsigned int width = index(0);
    for (unsigned int i = 1U; i < limit; i++)
      width = std::max(width,index(i));
    return width+1U;
  }
  
  forceinline void
  SparseBitSet::Data::replace_and_decrease(unsigned int i, BitSetData w) {
    assert(i < limit);
    assert(limit > 0U);
    BitSetData w_i = word(i);
    if (w != w_i) {
      es[i>>1].bits[i&1U] = w;
      if (w.none()) {
         assert(word(i).none());
         es[i>>1].bits[i&1U] = word(limit-1);
         es[i>>1].index[i&1U] = index(limit-1);
         limit--;
       }
     }
   }

   forceinline void
   SparseBitSet::Data::clear_mask(BitSetData* mask) {
     assert(limit > 0);
     for (unsigned int i = limit; i--; ) {
       mask[i].init(false);
       assert(mask[i].none());
     }
   }

   forceinline void
   SparseBitSet::Data::add_to_mask(const BitSetData* b, BitSetData* mask) const {
     assert(limit > 0);
     for (unsigned int i = limit; i--; )
       mask[i] = BitSetData::o(mask[i],b[index(i)]);
   }

   template<bool sparse>
   forceinline void
   SparseBitSet::Data::intersect_with_mask(const BitSetData* mask) {
     assert(limit > 0);
     if (sparse) {
       for (unsigned int i = limit; i--; ) {
         assert(!word(i).none());
         BitSetData w_i = word(i);
         BitSetData w_a = BitSetData::a(w_i, mask[index(i)]);
         replace_and_decrease(i,w_a);
         assert(i == limit || !word(i).none());
       }
     } else { // The same except different indexing in mask
       for (unsigned int i = limit; i--; ) {
         assert(!word(i).none());
         BitSetData w_i = word(i);
         BitSetData w_a = BitSetData::a(w_i, mask[i]);
         replace_and_decrease(i,w_a);
         assert(i == limit || !word(i).none());
       }
     }
   }

   forceinline void
   SparseBitSet::Data::intersect_with_masks(const BitSetData* a, const BitSetData* b) {
     assert(limit > 0);
     for (unsigned int i = limit; i--; ) {
       assert(!word(i).none());
       BitSetData w_i = word(i);
       unsigned int offset = index(i);
       BitSetData w_o = BitSetData::o(a[offset], b[offset]);
       BitSetData w_a = BitSetData::a(w_i,w_o);
       replace_and_decrease(i,w_a);
       assert(i == limit || !word(i).none());
     }
   }

   forceinline void
   SparseBitSet::Data::nand_with_mask(const BitSetData* b) {
     assert(limit > 0);
     for (unsigned int i = limit; i--; ) {
       assert(!word(i).none());
       BitSetData w = BitSetData::a(word(i),~(b[index(i)]));
       replace_and_decrease(i,w);
       assert(i == limit || !word(i).none());
     }
   }

   forceinline bool
   SparseBitSet::Data::intersects(const BitSetData* b) {
     assert(limit > 0);
     for (unsigned int i = limit; i--; )
       if (!BitSetData::a(word(i),b[index(i)]).none())
         return true;
     return false;
   }
      
  /*
   * Sparse bit-set
   *
   */
  forceinline
  SparseBitSet::SparseBitSet(Space& home, unsigned int n) {
    d = Data::allocate(home,Entry::data(n));
    d->init(n);
  }

  forceinline
  SparseBitSet::SparseBitSet(Space& home, const SparseBitSet& sbs) {
    assert(sbs.d->limit > 0);
    d = Data::allocate(home,((sbs.d->limit-1)>>1)+1U);
    d->limit = sbs.d->limit;
    for (unsigned int i=0U; i <= ((d->limit-1)>>1); i++)
      d->es[i] = sbs.d->es[i];
  }
      
  template<unsigned int size>
  forceinline
  SparseBitSet::SparseBitSet(Space&, const TinyBitSet<size>&) {
    GECODE_NEVER;
  }

  template<unsigned int size>
  forceinline
  SparseBitSet::SparseBitSet(Space&, const SmallBitSet<size>&) {
    GECODE_NEVER;
  }

  forceinline bool
  SparseBitSet::empty(void) const {
    return d->limit == 0U;
  }

  forceinline unsigned int
  SparseBitSet::width(void) const {
    assert(d->width() >= words());
    return d->width();
  }
  
  forceinline void
  SparseBitSet::clear_mask(BitSetData* mask) {
    d->clear_mask(mask);
  }
  
  forceinline void
  SparseBitSet::add_to_mask(const BitSetData* b, BitSetData* mask) const {
    d->add_to_mask(b,mask);
  }

  template<bool sparse>
  forceinline void
  SparseBitSet::intersect_with_mask(const BitSetData* mask) {
    d->intersect_with_mask<sparse>(mask);
  }

  forceinline void
  SparseBitSet::intersect_with_masks(const BitSetData* a, const BitSetData* b) {
    d->intersect_with_masks(a,b);
  }
  
  forceinline void
  SparseBitSet::nand_with_mask(const BitSetData* b) {
    d->nand_with_mask(b);
  }
  
  forceinline bool
  SparseBitSet::intersects(const BitSetData* b) {
    return d->intersects(b);
  }
  
  forceinline unsigned int
  SparseBitSet::words(void) const {
    return d->limit;
  }
  
  forceinline BitSetData
  SparseBitSet::word(unsigned int i) const {
    return d->word(i);
  }
  
  forceinline unsigned int
  SparseBitSet::index(unsigned int i) const {
    return d->index(i);
  }
       
}}}

// STATISTICS: int-prop
