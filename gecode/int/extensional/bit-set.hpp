/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Christian Schulte, 2017
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

  forceinline
  CompressedSupport::CompressedSupport(void)
    : b(nullptr), e(nullptr) {}

  forceinline
  CompressedSupport::CompressedSupport(const TupleSet::CSupportWord* b0,
                                       const TupleSet::CSupportWord* e0)
    : b(b0), e(e0) {}

  forceinline const TupleSet::CSupportWord*
  CompressedSupport::begin(void) const {
    return b;
  }

  forceinline const TupleSet::CSupportWord*
  CompressedSupport::end(void) const {
    return e;
  }

  forceinline bool
  CompressedSupport::empty(void) const {
    return (b == nullptr) || (b >= e);
  }

  forceinline const TupleSet::BitSetData*
  find_support_word(const CompressedSupport& s, unsigned int widx) {
    const TupleSet::CSupportWord* b = s.begin();
    const TupleSet::CSupportWord* e = s.end();
    while (b < e) {
      const TupleSet::CSupportWord* m = b + ((e-b) >> 1);
      if (widx < m->widx) {
        e = m;
      } else if (widx > m->widx) {
        b = m+1;
      } else {
        return &m->bits;
      }
    }
    return nullptr;
  }

  template<class IndexType>
  forceinline unsigned int
  BitSet<IndexType>::limit(void) const {
    return static_cast<unsigned int>(_active_words);
  }
  
  template<class IndexType>
  forceinline bool
  BitSet<IndexType>::empty(void) const {
    return _active_words == 0U;
  }
  
  template<class IndexType>
  forceinline unsigned int
  BitSet<IndexType>::words(void) const {
    return static_cast<unsigned int>(_active_words);
  }

  template<class IndexType>
  forceinline unsigned int
  BitSet<IndexType>::size(void) const {
    return words();
  }
      
  template<class IndexType>
  forceinline unsigned int
  BitSet<IndexType>::width(void) const {
    assert(!empty());
    IndexType width = _word_index[0];
    for (IndexType i=1; i<_active_words; i++)
      width = std::max(width,_word_index[i]);
    assert(static_cast<unsigned int>(width+1U) >= words());
    return static_cast<unsigned int>(width+1U);
  }

  template<class IndexType>
  forceinline
  BitSet<IndexType>::BitSet(Space& home, unsigned int n,
                            bool track_positions)
    : _active_words(static_cast<IndexType>(n)),
      _word_capacity(static_cast<IndexType>(n)),
      _word_index(home.alloc<IndexType>(n)),
      _word_bits(home.alloc<BitSetData>(n)),
      _active_position(track_positions ? home.alloc<IndexType>(n) : nullptr) {
    // Set all bits in all words (including the last)
    for (IndexType i=0; i<_active_words; i++) {
      _word_bits[i].init(true);
      _word_index[i] = i;
      if (_active_position != nullptr)
        _active_position[i] = i+1;
    }
  }

  template<class IndexType>
  template<class OldIndexType>
  forceinline
  BitSet<IndexType>::BitSet(Space& home,
                            const BitSet<OldIndexType>& bs)
    : _active_words(static_cast<IndexType>(bs._active_words)),
      _word_capacity(static_cast<IndexType>(
        bs.empty() ? 0U : bs.width())),
      _word_index((_active_words > 0U) ?
                  home.alloc<IndexType>(_active_words) : nullptr),
      _word_bits((_active_words > 0U) ?
                 home.alloc<BitSetData>(_active_words) : nullptr),
      _active_position(((bs._active_position != nullptr) &&
                        (_word_capacity > 0U)) ?
                       home.alloc<IndexType>(_word_capacity) : nullptr) {
    for (IndexType i=0; i<_active_words; i++) {
      _word_bits[i] = bs._word_bits[i];
      _word_index[i] = static_cast<IndexType>(bs._word_index[i]);
    }
    if (_active_position != nullptr) {
      for (IndexType i=0; i<_word_capacity; i++)
        _active_position[i] = 0;
      for (IndexType i=0; i<_active_words; i++)
        _active_position[_word_index[i]] = i+1;
    }
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::flush(void) {
    _active_words = 0U;
    assert(empty());
  }
    
  template<class IndexType>
  forceinline
  BitSet<IndexType>::BitSet(Space&, const TinyBitSet<1U>&)
    : _active_words(0U), _word_capacity(0U), _word_index(nullptr), _word_bits(nullptr), _active_position(nullptr) {
    GECODE_NEVER;
  }
  template<class IndexType>
  forceinline
  BitSet<IndexType>::BitSet(Space&, const TinyBitSet<2U>&)
    : _active_words(0U), _word_capacity(0U), _word_index(nullptr), _word_bits(nullptr), _active_position(nullptr) {
    GECODE_NEVER;
  }
  template<class IndexType>
  forceinline
  BitSet<IndexType>::BitSet(Space&, const TinyBitSet<3U>&)
    : _active_words(0U), _word_capacity(0U), _word_index(nullptr), _word_bits(nullptr), _active_position(nullptr) {
    GECODE_NEVER;
  }
  template<class IndexType>
  forceinline
  BitSet<IndexType>::BitSet(Space&, const TinyBitSet<4U>&)
    : _active_words(0U), _word_capacity(0U), _word_index(nullptr), _word_bits(nullptr), _active_position(nullptr) {
    GECODE_NEVER;
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::replace_and_decrease(IndexType active_pos,
                                          BitSetData word) {
    assert(_active_words > 0U);
    BitSetData old_word = _word_bits[active_pos];
    if (word != old_word) {
      _word_bits[active_pos] = word;
      if (word.none()) {
        assert(_word_bits[active_pos].none());
        const IndexType removed_word_index = _word_index[active_pos];
        _active_words--;
        const IndexType moved_word_index = _word_index[_active_words];
        _word_bits[active_pos] = _word_bits[_active_words];
        _word_index[active_pos] = moved_word_index;
        if (_active_position != nullptr) {
          _active_position[removed_word_index] = 0;
          if (active_pos < _active_words)
            _active_position[moved_word_index] = active_pos+1;
        }
      }
    }
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::clear_mask(BitSetData* mask) const {
    assert(_active_words > 0U);
    for (IndexType i=0; i<_active_words; i++) {
      mask[i].init(false);
      assert(mask[i].none());
    }
  }
  
  template<class IndexType>
  forceinline void
  BitSet<IndexType>::add_to_mask(const BitSetData* support,
                                 BitSetData* mask) const {
    assert(_active_words > 0U);
    for (IndexType i=0; i<_active_words; i++)
      mask[i] = BitSetData::o(mask[i],support[_word_index[i]]);
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::add_to_mask(const CompressedSupport& support,
                                 BitSetData* mask) const {
    if ((_active_words == 0U) || support.empty())
      return;
    if (_active_position == nullptr) {
      for (IndexType i=0; i<_active_words; i++) {
        const BitSetData* support_word =
          find_support_word(support,_word_index[i]);
        if (support_word != nullptr)
          mask[i] = BitSetData::o(mask[i],*support_word);
      }
      return;
    }
    for (const TupleSet::CSupportWord* s=support.begin();
         s<support.end(); ++s) {
      if (s->widx >= static_cast<unsigned int>(_word_capacity))
        continue;
      const IndexType active_pos = _active_position[s->widx];
      if (active_pos == 0U)
        continue;
      mask[active_pos-1] = BitSetData::o(mask[active_pos-1],s->bits);
    }
  }

  template<class IndexType>
  template<bool sparse>
  forceinline void
  BitSet<IndexType>::intersect_with_mask(const BitSetData* mask) {
    assert(_active_words > 0U);
    if (sparse) {
      for (IndexType i = _active_words; i--; ) {
        assert(!_word_bits[i].none());
        BitSetData old_word = _word_bits[i];
        BitSetData new_word = BitSetData::a(old_word, mask[_word_index[i]]);
        replace_and_decrease(i,new_word);
        assert(i == _active_words || !_word_bits[i].none());
      }
    } else { // The same except different _word_indexing in mask
      for (IndexType i = _active_words; i--; ) {
        assert(!_word_bits[i].none());
        BitSetData old_word = _word_bits[i];
        BitSetData new_word = BitSetData::a(old_word, mask[i]);
        replace_and_decrease(i,new_word);
        assert(i == _active_words || !_word_bits[i].none());
      }
    }
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::intersect_with_mask(const CompressedSupport& support) {
    if (_active_words == 0U)
      return;
    for (IndexType i = _active_words; i--; ) {
      assert(!_word_bits[i].none());
      const BitSetData* support_word =
        find_support_word(support,_word_index[i]);
      BitSetData new_word;
      if (support_word == nullptr) {
        new_word.init(false);
      } else {
        new_word = BitSetData::a(_word_bits[i],*support_word);
      }
      replace_and_decrease(i,new_word);
      assert(i == _active_words || !_word_bits[i].none());
    }
  }
  
  template<class IndexType>
  forceinline void
  BitSet<IndexType>::intersect_with_masks(const BitSetData* a,
                                          const BitSetData* b) {
    assert(_active_words > 0U);
    for (IndexType i = _active_words; i--; ) {
      assert(!_word_bits[i].none());
      BitSetData old_word = _word_bits[i];
      IndexType offset = _word_index[i];
      BitSetData union_word = BitSetData::o(a[offset], b[offset]);
      BitSetData new_word = BitSetData::a(old_word,union_word);
      replace_and_decrease(i,new_word);
      assert(i == _active_words || !_word_bits[i].none());
    }
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::intersect_with_masks(const CompressedSupport& a,
                                          const CompressedSupport& b) {
    if (_active_words == 0U)
      return;
    for (IndexType i = _active_words; i--; ) {
      assert(!_word_bits[i].none());
      const BitSetData* first_support_word =
        find_support_word(a,_word_index[i]);
      const BitSetData* second_support_word =
        find_support_word(b,_word_index[i]);
      BitSetData union_word;
      if (first_support_word != nullptr) {
        union_word = *first_support_word;
      } else {
        union_word.init(false);
      }
      if (second_support_word != nullptr)
        union_word = BitSetData::o(union_word,*second_support_word);
      BitSetData new_word = BitSetData::a(_word_bits[i],union_word);
      replace_and_decrease(i,new_word);
      assert(i == _active_words || !_word_bits[i].none());
    }
  }
  
  template<class IndexType>
  forceinline void
  BitSet<IndexType>::nand_with_mask(const BitSetData* mask) {
    assert(_active_words > 0U);
    for (IndexType i = _active_words; i--; ) {
      assert(!_word_bits[i].none());
      BitSetData new_word =
        BitSetData::a(_word_bits[i],~(mask[_word_index[i]]));
      replace_and_decrease(i,new_word);
      assert(i == _active_words || !_word_bits[i].none());
    }
  }

  template<class IndexType>
  forceinline void
  BitSet<IndexType>::nand_with_mask(const CompressedSupport& support) {
    if ((_active_words == 0U) || support.empty())
      return;
    if (_active_position == nullptr) {
      for (IndexType i = _active_words; i--; ) {
        assert(!_word_bits[i].none());
        const BitSetData* support_word =
          find_support_word(support,_word_index[i]);
        if (support_word != nullptr) {
          BitSetData new_word = BitSetData::a(_word_bits[i],~(*support_word));
          replace_and_decrease(i,new_word);
          assert(i == _active_words || !_word_bits[i].none());
        }
      }
      return;
    }
    for (const TupleSet::CSupportWord* s=support.begin();
         s<support.end(); ++s) {
      if (s->widx >= static_cast<unsigned int>(_word_capacity))
        continue;
      const IndexType active_pos = _active_position[s->widx];
      if (active_pos == 0U)
        continue;
      const IndexType i = active_pos-1;
      BitSetData new_word = BitSetData::a(_word_bits[i],~(s->bits));
      replace_and_decrease(i,new_word);
    }
  }

  template<class IndexType>
  forceinline bool
  BitSet<IndexType>::intersects(const BitSetData* mask) const {
    for (IndexType i=0; i<_active_words; i++)
      if (!BitSetData::a(_word_bits[i],mask[_word_index[i]]).none())
        return true;
    return false;
  }

  template<class IndexType>
  forceinline bool
  BitSet<IndexType>::intersects(const CompressedSupport& support) const {
    if ((_active_words == 0U) || support.empty())
      return false;
    if (_active_position == nullptr) {
      for (IndexType i=0; i<_active_words; i++) {
        const BitSetData* support_word =
          find_support_word(support,_word_index[i]);
        if ((support_word != nullptr) &&
            !BitSetData::a(_word_bits[i],*support_word).none())
          return true;
      }
      return false;
    }
    for (const TupleSet::CSupportWord* s=support.begin();
         s<support.end(); ++s) {
      if (s->widx >= static_cast<unsigned int>(_word_capacity))
        continue;
      const IndexType active_pos = _active_position[s->widx];
      if ((active_pos != 0U) &&
          !BitSetData::a(_word_bits[active_pos-1],s->bits).none())
        return true;
    }
    return false;
  }
    
  template<class IndexType>
  forceinline unsigned long long int
  BitSet<IndexType>::ones(const BitSetData* mask) const {
    unsigned long long int count = 0U;
    for (IndexType i=0; i<_active_words; i++)
      count += static_cast<unsigned long long int>
        (BitSetData::a(_word_bits[i],mask[_word_index[i]]).ones());
    return count;
  }

  template<class IndexType>
  forceinline unsigned long long int
  BitSet<IndexType>::ones(const CompressedSupport& support) const {
    unsigned long long int count = 0U;
    if ((_active_words == 0U) || support.empty())
      return 0U;
    if (_active_position == nullptr) {
      for (IndexType i=0; i<_active_words; i++) {
        const BitSetData* support_word =
          find_support_word(support,_word_index[i]);
        if (support_word != nullptr)
          count += static_cast<unsigned long long int>
            (BitSetData::a(_word_bits[i],*support_word).ones());
      }
      return count;
    }
    for (const TupleSet::CSupportWord* s=support.begin();
         s<support.end(); ++s) {
      if (s->widx >= static_cast<unsigned int>(_word_capacity))
        continue;
      const IndexType active_pos = _active_position[s->widx];
      if (active_pos == 0U)
        continue;
      count += static_cast<unsigned long long int>
        (BitSetData::a(_word_bits[active_pos-1],s->bits).ones());
    }
    return count;
  }
    
  template<class IndexType>
  forceinline unsigned long long int
  BitSet<IndexType>::ones(void) const {
    unsigned long long int count = 0U;
    for (IndexType i=0; i<_active_words; i++)
      count += static_cast<unsigned long long int>(_word_bits[i].ones());
    return count;
  }
    
  template<class IndexType>
  forceinline unsigned long long int
  BitSet<IndexType>::bits(void) const {
    return (static_cast<unsigned long long int>(_active_words) *
            static_cast<unsigned long long int>(BitSetData::bpb));
  }

}}}

// STATISTICS: int-prop
