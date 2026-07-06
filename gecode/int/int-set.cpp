/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Alexander Shepil <alexander.shepil@sap.com>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2003
 *     Alexander Shepil, 2024
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

#include <gecode/int.hh>

namespace Gecode {

#ifdef GECODE_HAS_FAULT_INJECTION
  std::atomic<int> IntSet::IntSetObject::fault_live_objects{0};

  void*
  IntSet::IntSetObject::operator new(size_t s) {
    void* p = ::operator new(s);
    fault_live_objects.fetch_add(1, std::memory_order_relaxed);
    return p;
  }

  void
  IntSet::IntSetObject::operator delete(void* p) {
    fault_live_objects.fetch_sub(1, std::memory_order_relaxed);
    ::operator delete(p);
  }

  void
  IntSet::fault_reset_allocations(void) {
    IntSetObject::fault_live_objects.store(0, std::memory_order_relaxed);
  }

  int
  IntSet::fault_live_allocations(void) {
    return IntSetObject::fault_live_objects.load(std::memory_order_relaxed);
  }
#endif

  IntSet::IntSetObject*
  IntSet::IntSetObject::allocate(int n) {
    IntSetObject* o = new IntSetObject;
    o->size = 0U;
    o->n = 0;
    o->r = nullptr;
    try {
#ifdef GECODE_HAS_FAULT_INJECTION
      Support::FailPoint::check(Support::FailPoint::Phase::IntSet);
#endif
      o->r = heap.alloc<Range>(n);
    } catch (...) {
      delete o;
      throw;
    }
    o->n = n;
    return o;
  }

  bool
  IntSet::IntSetObject::in(int n) const {
    int l = 0;
    int r = this->n - 1;

    while (l <= r) {
      int m = l + (r - l) / 2;
      if ((this->r[m].min <= n) && (n <= this->r[m].max)) {
        return true;
      } else if (l == r) {
        return false;
      } else if (n < this->r[m].min) {
        r=m-1;
      } else {
        l=m+1;
      }
    }
    return false;
  }

  bool
  IntSet::IntSetObject::equal(const IntSetObject& iso) const {
    assert((size == iso.size) || (n == iso.n));
    for (int i=0; i<n; i++)
      if ((r[i].min != iso.r[i].min) || (r[i].max != iso.r[i].max))
        return false;
    return true;
  }

  IntSet::IntSetObject::~IntSetObject(void) {
    if (r != nullptr)
      heap.free<Range>(r,n);
  }

  /// Sort ranges according to increasing minimum
  class IntSet::MinInc {
  public:
    bool operator ()(const Range &x, const Range &y);
  };

  forceinline bool
  IntSet::MinInc::operator ()(const Range &x, const Range &y) {
    return x.min < y.min;
  }

  void
  IntSet::normalize(Range* r, int n) {
    if (n > 0) {
      // Sort ranges
      {
        MinInc lt_mi;
        Support::quicksort<Range>(r, n, lt_mi);
      }
      // Conjoin continuous ranges
      {
        int min = r[0].min;
        int max = r[0].max;
        int i = 1;
        int j = 0;
        while (i < n) {
          if (max+1 < r[i].min) {
            r[j].min = min; r[j].max = max; j++;
            min = r[i].min; max = r[i].max; i++;
          } else {
            max = std::max(max,r[i].max); i++;
          }
        }
        r[j].min = min; r[j].max = max;
        n=j+1;
      }
      IntSetObject* o = IntSetObject::allocate(n);
      unsigned int s = 0;
      for (int i=0; i<n; i++) {
        s += static_cast<unsigned int>(r[i].max) -
          static_cast<unsigned int>(r[i].min) + 1U;
        o->r[i]=r[i];
      }
      o->size = s;
      object(o);
    }
  }

  void
  IntSet::init(const int r[], int n) {
    assert(n > 0);
    Region reg;
    Range* dr = reg.alloc<Range>(n);
    for (int i=0; i<n; i++) {
      dr[i].min=r[i]; dr[i].max=r[i];
    }
    normalize(&dr[0],n);
  }

  void
  IntSet::init(const int r[][2], int n) {
    assert(n > 0);
    Region reg;
    Range* dr = reg.alloc<Range>(n);
    int j = 0;
    for (int i=0; i<n; i++)
      if (r[i][0] <= r[i][1]) {
        dr[j].min=r[i][0]; dr[j].max=r[i][1]; j++;
      }
    normalize(&dr[0],j);
  }

  IntSet::IntSet(std::initializer_list<int> r) {
    int n = static_cast<int>(r.size());
    assert(n > 0);
    Region reg;
    Range* dr = reg.alloc<Range>(n);
    int j=0;
    for (int k : r) {
      dr[j].min=dr[j].max=k; j++;
    }
    normalize(&dr[0],j);
  }

  IntSet::IntSet(std::initializer_list<std::pair<int,int>> r) {
    int n = static_cast<int>(r.size());
    assert(n > 0);
    Region reg;
    Range* dr = reg.alloc<Range>(n);
    int j=0;
    for (const std::pair<int,int>& k : r) 
      if (k.first <= k.second) {
        dr[j].min=k.first; dr[j].max=k.second; j++;
      }
    normalize(&dr[0],j);
  }


  void
  IntSet::init(int n, int m) {
    if (n <= m) {
      IntSetObject* o = IntSetObject::allocate(1);
      o->r[0].min = n; o->r[0].max = m;
      o->size = static_cast<unsigned int>(m) - static_cast<unsigned int>(n) + 1U;
      object(o);
    }
  }

  const IntSet IntSet::empty;

}

// STATISTICS: int-var
