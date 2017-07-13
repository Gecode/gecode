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
    * Tiny bit-set
    *
    */
   template<unsigned int size>
   forceinline
   TinyBitSet<size>::TinyBitSet(Space&, unsigned int n) {
     assert(n <= size);
     /// Set the active bits
     for (unsigned int i=0U; i < n; i++)
       bits[i].init(true);
     /// Clear unused suffix bits
     for (unsigned int i=n; i < size; i++)
       bits[i].init(false);
   }

   template<unsigned int size>
   template<unsigned int largersize>
   forceinline
   TinyBitSet<size>::TinyBitSet(Space&, const TinyBitSet<largersize>& sbs) {
     assert(size <= largersize);
     assert(!sbs.empty());
     for (unsigned int i = size; i--; )
       bits[i] = sbs.bits[i];
     assert(!empty());
   }
      
   template<unsigned int size>
   forceinline
   TinyBitSet<size>::TinyBitSet(Space&, const SparseBitSet& sbs) {
     assert(size == sbs.width());
     assert(!sbs.empty());
     for (unsigned int i = size; i--; )
       bits[i].init(false);
     for (unsigned int i = sbs.words(); i--; ) {
       bits[sbs.index(i)] = sbs.word(i);
     }
     assert(!empty());
   }

   template<unsigned int size>
   template<unsigned int smallsize>
   forceinline
   TinyBitSet<size>::TinyBitSet(Space&, const SmallBitSet<smallsize>& bs) {
     assert(size == bs.width());
     assert(!bs.empty());
     for (unsigned int i=size; i--; )
       bits[i].init(false);
     for (unsigned int i = bs.words(); i--; ) {
       bits[bs.index(i)] = bs.word(i);
     }
     assert(!empty());
   }

   template<unsigned int size>
   forceinline void
   TinyBitSet<size>::clear_mask(BitSetData* mask) {
     for (unsigned int i=size; i--; ) {
       mask[i].init(false);
       assert(mask[i].none());
     }
   }

   template<unsigned int size>
   forceinline void
   TinyBitSet<size>::add_to_mask(const BitSetData* b, BitSetData* mask) const {
     for (unsigned int i=size; i--; )
       mask[i] = BitSetData::o(mask[i],b[i]);
   }

   template<unsigned int size>
   template<bool sparse>
   forceinline void
   TinyBitSet<size>::intersect_with_mask(const BitSetData* mask) {
     for (unsigned int i=size; i--; )
       bits[i] = BitSetData::a(bits[i], mask[i]);
   }

   template<unsigned int size>
   forceinline void
   TinyBitSet<size>::intersect_with_masks(const BitSetData* a, const BitSetData* b) {
     for (unsigned int i=size; i--; )
       bits[i] = BitSetData::a(bits[i], BitSetData::o(a[i],b[i]));
   }

   template<unsigned int size>
   forceinline void
   TinyBitSet<size>::nand_with_mask(const BitSetData* b) {
     for (unsigned int i=size; i--; )
       bits[i] = BitSetData::a(bits[i],~(b[i]));
   }

   template<unsigned int size>
   forceinline bool
   TinyBitSet<size>::intersects(const BitSetData* b) {
     for (unsigned int i=size; i--; )
       if (!BitSetData::a(bits[i],b[i]).none())
         return true;
     return false;
   }

   template<unsigned int size>
   forceinline bool
   TinyBitSet<size>::empty(void) const { // Linear complexity...
     for (unsigned int i=size; i--; )
       if (!bits[i].none())
         return false;
     return true;
   }

   template<unsigned int size>
   forceinline unsigned int
   TinyBitSet<size>::width(void) const {
     assert(!empty());
     /// Find the index of the last non-zero word
     for (unsigned int i=size; i--; )
       if (!bits[i].none())
         return i+1U;
     GECODE_NEVER;
     return 0U;
   }
      
   template<unsigned int size>
   forceinline unsigned int
   TinyBitSet<size>::words(void) const {
     return width();
   }

}}}

// STATISTICS: int-prop
