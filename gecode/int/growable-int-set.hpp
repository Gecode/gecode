/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

namespace Gecode { namespace Int {

  forceinline
  GrowableIntSet::GrowableIntSet(void)
    : _ranges(nullptr), _count(0), _capacity(0) {}

  forceinline unsigned int
  GrowableIntSet::max_capacity(void) {
    return static_cast<unsigned int>(std::min(
      static_cast<size_t>(std::numeric_limits<unsigned int>::max()),
      static_cast<size_t>(std::numeric_limits<ptrdiff_t>::max()) /
        sizeof(Range)));
  }

  inline void
  GrowableIntSet::reserve(Space& home, unsigned int required) {
    if (required <= _capacity)
      return;
    const unsigned int limit = max_capacity();
    if (required > limit)
      throw MemoryExhausted();
    const unsigned int extra = _capacity / 2U + _capacity % 2U;
    const unsigned int grown = (_capacity == 0) ? 2U :
      ((_capacity > limit-extra) ? limit : _capacity+extra);
    const unsigned int capacity = std::max(required,grown);
    Range* ranges = home.alloc<Range>(capacity);
    for (unsigned int i=0; i<_count; i++)
      ranges[i] = _ranges[i];
    if (_ranges != nullptr)
      home.free<Range>(_ranges,_capacity);
    _ranges = ranges;
    _capacity = capacity;
  }

  forceinline unsigned int
  GrowableIntSet::first_max_ge(int value) const {
    unsigned int first = 0;
    unsigned int after = _count;
    while (first < after) {
      const unsigned int middle = first + (after-first)/2U;
      if (_ranges[middle].max < value)
        first = middle+1U;
      else
        after = middle;
    }
    return first;
  }

  forceinline bool
  GrowableIntSet::in(int value) const {
    const unsigned int i = first_max_ge(value);
    return (i < _count) && (_ranges[i].min <= value);
  }

  inline void
  GrowableIntSet::add(Space& home, int min, int max) {
    const unsigned int first = first_max_ge(min-1);
    unsigned int after = first;
    while ((after < _count) && (_ranges[after].min <= max+1))
      ++after;
    if (first < after) {
      _ranges[first].min = std::min(min,_ranges[first].min);
      _ranges[first].max = std::max(max,_ranges[after-1U].max);
      if (after > first+1U) {
        unsigned int out = first+1U;
        for (unsigned int i=after; i<_count; i++)
          _ranges[out++] = _ranges[i];
        _count = out;
      }
      return;
    }
    if (_count == max_capacity())
      throw MemoryExhausted();
    reserve(home,_count+1U);
    for (unsigned int i=_count; i>first; --i)
      _ranges[i] = _ranges[i-1U];
    _ranges[first].min = min;
    _ranges[first].max = max;
    ++_count;
  }

  template<class I>
  inline void
  GrowableIntSet::add_r(Space& home, I added) {
    if (!added())
      return;
    I remaining(added);
    ++remaining;
    if (!remaining()) {
      add(home,added.min(),added.max());
      return;
    }
    const unsigned int limit = max_capacity()-_count;
    unsigned int n = 1;
    for (; remaining(); ++remaining) {
      if (n >= limit)
        throw MemoryExhausted();
      ++n;
    }
    const unsigned int first = first_max_ge(added.min()-1);
    reserve(home,_count+n);
    // The hole protects unread old ranges while the union is written forward.
    for (unsigned int i=_count; i>first; --i)
      _ranges[i+n-1U] = _ranges[i-1U];
    unsigned int read = first+n;
    const unsigned int end = _count+n;
    unsigned int out = first;
    while (added() || (read < end)) {
      Range next;
      if (added() && ((read == end) ||
                      (added.min() < _ranges[read].min))) {
        next.min = added.min();
        next.max = added.max();
        ++added;
      } else {
        next = _ranges[read++];
      }
      if ((out > first) && (next.min <= _ranges[out-1U].max+1)) {
        _ranges[out-1U].max = std::max(_ranges[out-1U].max,next.max);
      } else {
        _ranges[out++] = next;
      }
    }
    _count = out;
  }

  inline void
  GrowableIntSet::update(Space& home, const GrowableIntSet& source) {
    const unsigned int count = source._count;
    Range* ranges = (count == 0) ? nullptr : home.alloc<Range>(count);
    for (unsigned int i=0; i<count; i++)
      ranges[i] = source._ranges[i];
    if (_ranges != nullptr)
      home.free<Range>(_ranges,_capacity);
    _ranges = ranges;
    _count = count;
    _capacity = count;
  }

  forceinline void
  GrowableIntSet::dispose(Space& home) {
    if (_ranges != nullptr)
      home.free<Range>(_ranges,_capacity);
    _ranges = nullptr;
    _count = 0;
    _capacity = 0;
  }

  forceinline
  GrowableIntSetRanges::GrowableIntSetRanges(const GrowableIntSet& set)
    : current(set._ranges),
      end((set._count == 0) ? set._ranges : set._ranges+set._count) {}

  forceinline bool
  GrowableIntSetRanges::operator ()(void) const {
    return current != end;
  }

  forceinline void
  GrowableIntSetRanges::operator ++(void) {
    ++current;
  }

  forceinline int
  GrowableIntSetRanges::min(void) const {
    return current->min;
  }

  forceinline int
  GrowableIntSetRanges::max(void) const {
    return current->max;
  }

  forceinline unsigned int
  GrowableIntSetRanges::width(void) const {
    return static_cast<unsigned int>(current->max) -
      static_cast<unsigned int>(current->min) + 1U;
  }

}}

// STATISTICS: int-other
