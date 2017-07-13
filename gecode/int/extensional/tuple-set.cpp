/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
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

#include <gecode/int.hh>
#include <algorithm>

namespace Gecode { namespace Int { namespace Extensional {

  /// Import tuple type
  typedef ::Gecode::TupleSet::Tuple Tuple;

  /// Tuple comparison
  class TupleCompare {
  private:
    /// The arity of the tuples to compare
    int arity;
  public:
    /// Initialize with arity \a a
    TupleCompare(int a);
    /// Comparison of tuples \a a and \a b
    bool operator ()(const Tuple& a, const Tuple& b);
  };

  /// Tuple comparison by position
  class PosCompare {
  private:
    /// The position of the tuples to compare
    int p;
  public:
    /// Initialize with position \a p
    PosCompare(int p);
    /// Comparison of tuples \a a and \a b
    bool operator ()(const Tuple& a, const Tuple& b);
  };


  forceinline
  TupleCompare::TupleCompare(int a) : arity(a) {}

  forceinline bool
  TupleCompare::operator ()(const Tuple& a, const Tuple& b) {
    for (int i = 0; i < arity; i++)
      if (a[i] < b[i])
        return true;
      else if (a[i] > b[i])
        return false;
    return false;
  }


  forceinline
  PosCompare::PosCompare(int p0) : p(p0) {}

  forceinline bool
  PosCompare::operator ()(const Tuple& a, const Tuple& b) {
    return a[p] < b[p];
  }


}}}

namespace Gecode {

  /*
   * Tuple set data
   *
   */
  void
  TupleSet::Data::finalize(void) {
    using namespace Int::Extensional;
    assert(!finalized());
    // Mark as finalized
    n_free = -1;

    // Initialization
    if (n_tuples == 0) {
      delete td; td=nullptr;
      return;
    }

    // Compact and copy data
    Region r;
    // Set up tuple pointers
    Tuple* tuple = r.alloc<Tuple>(n_tuples);
    {
      for (int t=n_tuples; t--; )
        tuple[t] = td + t*arity;
      TupleCompare tc(arity);
      Support::quicksort(tuple, n_tuples, tc);
      // Remove duplicates
      int j=1;
      for (int t=1; t<n_tuples; t++) {
        for (int a=arity; a--; )
          if (tuple[t-1][a] != tuple[t][a])
            goto notsame;
        goto same;
      notsame: ;
        tuple[j++] = tuple[t];
      same: ;
      }
      assert(j <= n_tuples);
      n_tuples=j;
      // Copy into now possibly smaller area
      int* new_td = heap.alloc<int>(n_tuples*arity);
      for (int t=n_tuples; t--; ) {
        for (int a=arity; a--; )
          new_td[t*arity+a] = tuple[t][a];
        tuple[t] = new_td + t*arity;
      }
      heap.rfree(td);
      td = new_td;
    }
    
    // Only now compute how many tuples are needed!
    n_words = BitSetData::data(n_tuples);

    // Compute range information
    {
      /*
       * Pass one: compute how many values and ranges are needed
       */
      // How many values
      unsigned int n_vals = 0U;
      // How many ranges
      unsigned int n_ranges = 0U;
      for (int a=arity; a--; ) {
        // Sort tuple according to position
        PosCompare pc(a);
        Support::quicksort(tuple, n_tuples, pc);
        // Scan values
        {
          int min=tuple[0][a];
          int max=tuple[0][a];
          n_vals++; n_ranges++;
          for (int i=1; i<n_tuples; i++) {
            assert(tuple[i-1][a] <= tuple[i][a]);
            if (max+1 == tuple[i][a]) {
              n_vals++;
              max=tuple[i][a];
            } else if (max+1 < tuple[i][a]) {
              n_vals++; n_ranges++;
              min=max=tuple[i][a];
            } else {
              assert(max == tuple[i][a]);
            }
          }
        }
      }
      /*
       * Pass 2: allocate memory and fill data structures
       */
      // Allocate memory for ranges
      Range* cr = range = heap.alloc<Range>(n_ranges);
      // Allocate and initialize memory for supports
      BitSetData* cs = support = heap.alloc<BitSetData>(n_words * n_vals);
      for (unsigned int i=n_vals * n_words; i--; )
        cs[i].init();
      for (int a=arity; a--; ) {
        // Set range pointer
        vd[a].r = cr;
        // Sort tuple according to position
        PosCompare pc(a);
        Support::quicksort(tuple, n_tuples, pc);
        // Update min and max
        min = std::min(min,tuple[0][a]);
        max = std::max(max,tuple[n_tuples-1][a]);
        // Compress into non-overlapping ranges
        {
          unsigned int j=0U;
          vd[a].r[0].max=vd[a].r[0].min=tuple[0][a];
          for (int i=1; i<n_tuples; i++) {
            assert(tuple[i-1][a] <= tuple[i][a]);
            if (vd[a].r[j].max+1 == tuple[i][a]) {
              vd[a].r[j].max=tuple[i][a];
            } else if (vd[a].r[j].max+1 < tuple[i][a]) {
              j++; vd[a].r[j].min=vd[a].r[j].max=tuple[i][a];
            } else {
              assert(vd[a].r[j].max == tuple[i][a]);
            }
          }
          vd[a].n = j+1U;
          cr += j+1U;
        }
        // Set support pointer and set bits
        for (unsigned int i=0U; i<vd[a].n; i++) {
          vd[a].r[i].s = cs;
          cs += n_words * vd[a].r[i].width();
        }
        {
          int j=0;
          for (int i=0; i<n_tuples; i++) {
            while (tuple[i][a] > vd[a].r[j].max)
              j++;
            set(const_cast<BitSetData*>
                (vd[a].r[j].supports(n_words,tuple[i][a])),
                tuple2idx(tuple[i]));
          }
        }
      }
      assert(cs == support + n_words * n_vals);
      assert(cr == range + n_ranges);
    }
    if ((min < Int::Limits::min) || (max > Int::Limits::max))
      throw Int::OutOfLimits("TupleSet::finalize()");
    assert(finalized());
  }

  void
  TupleSet::Data::resize(void) {
    assert(n_free == 0);
    int n = static_cast<int>(1+n_tuples*1.5);
    td = heap.realloc<int>(td, n_tuples * arity, n * arity);
    n_free = n - n_tuples;
  }

  TupleSet::Data::~Data(void) {
    heap.rfree(td);
    heap.rfree(vd);
    heap.rfree(range);
    heap.rfree(support);
  }


  /*
   * Tuple set
   *
   */
  TupleSet::TupleSet(int a)
    : SharedHandle(new Data(a)) {}
  void
  TupleSet::init(int a) {
    object(new Data(a));
  }
  TupleSet::TupleSet(const TupleSet& ts)
    : SharedHandle(ts) {}
  TupleSet&
  TupleSet::operator =(const TupleSet& ts) {
    (void) SharedHandle::operator =(ts);
    return *this;
  }

  void
  TupleSet::_add(const IntArgs& t) {
    if (!*this)
      throw Int::UninitializedTupleSet("TupleSet::add()");
    if (raw().finalized())
      throw Int::AlreadyFinalized("TupleSet::add()");
    if (t.size() != raw().arity)
      throw Int::ArgumentSizeMismatch("TupleSet::add()");
    Tuple a = raw().add();
    for (int i=t.size(); i--; )
      a[i]=t[i];
  }

  TupleSet&
  TupleSet::add(int n, ...) {
    if (!*this)
      throw Int::UninitializedTupleSet("TupleSet::add()");
    if (raw().finalized())
      throw Int::AlreadyFinalized("TupleSet::add()");
    Tuple t = raw().add();
    va_list args;
    va_start(args, n);
    t[0]=n;
    for (int i=1; i<raw().arity; i++)
      t[i] = va_arg(args,int);
    va_end(args);
    return *this;
  }

}

// STATISTICS: int-prop

