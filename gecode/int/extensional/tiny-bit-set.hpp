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
     for (unsigned int i=0U; i < n; i++)
       bits[i].init(true);
     /// Clear unused suffix bits
     for (unsigned int i=n; i < sz; i++)
       bits[i].init(false);
   }

   template<unsigned int sz>
   template<unsigned int largersz>
   forceinline
   TinyBitSet<sz>::TinyBitSet(Space&, const TinyBitSet<largersz>& sbs) {
     GECODE_ASSUME(sz <= largersz);
     assert(!sbs.empty());
     for (unsigned int i = sz; i--; )
       bits[i] = sbs.bits[i];
     assert(!empty());
   }
      
   template<unsigned int sz>
   template<class IndexType>
   forceinline
   TinyBitSet<sz>::TinyBitSet(Space&, const BitSet<IndexType>& sbs) {
     assert(sz == sbs.width());
     assert(!sbs.empty());
     for (unsigned int i = sz; i--; )
       bits[i].init(false);
     for (unsigned int i = sbs.words(); i--; ) {
       bits[sbs.index[i]] = sbs.bits[i];
     }
     assert(!empty());
   }

   template<unsigned int sz>
   forceinline void
   TinyBitSet<sz>::clear_mask(BitSetData* mask) {
     for (unsigned int i=sz; i--; ) {
       mask[i].init(false);
       assert(mask[i].none());
     }
   }

   template<unsigned int sz>
   forceinline void
   TinyBitSet<sz>::add_to_mask(const BitSetData* b, BitSetData* mask) const {
     for (unsigned int i=sz; i--; )
       mask[i] = BitSetData::o(mask[i],b[i]);
   }

   template<unsigned int sz>
   template<bool sparse>
   forceinline void
   TinyBitSet<sz>::intersect_with_mask(const BitSetData* mask) {
     for (unsigned int i=sz; i--; )
       bits[i] = BitSetData::a(bits[i], mask[i]);
   }

   template<unsigned int sz>
   forceinline void
   TinyBitSet<sz>::intersect_with_masks(const BitSetData* a, const BitSetData* b) {
     for (unsigned int i=sz; i--; )
       bits[i] = BitSetData::a(bits[i], BitSetData::o(a[i],b[i]));
   }

   template<unsigned int sz>
   forceinline void
   TinyBitSet<sz>::nand_with_mask(const BitSetData* b) {
     for (unsigned int i=sz; i--; )
       bits[i] = BitSetData::a(bits[i],~(b[i]));
   }

   template<unsigned int sz>
   forceinline bool
   TinyBitSet<sz>::intersects(const BitSetData* b) {
     for (unsigned int i=sz; i--; )
       if (!BitSetData::a(bits[i],b[i]).none())
         return true;
     return false;
   }

   template<unsigned int sz>
   forceinline bool
   TinyBitSet<sz>::empty(void) const { // Linear complexity...
     for (unsigned int i=sz; i--; )
       if (!bits[i].none())
         return false;
     return true;
   }

   template<unsigned int sz>
   forceinline unsigned int
   TinyBitSet<sz>::width(void) const {
     assert(!empty());
     /// Find the index of the last non-zero word
     for (unsigned int i=sz; i--; )
       if (!bits[i].none())
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
