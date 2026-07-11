/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2007
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

#include <sstream>

namespace Gecode {

  /*
   * Ranges
   *
   */
  forceinline unsigned int
  TupleSet::Range::width(void) const {
    return static_cast<unsigned int>(max - min + 1);
  }

  forceinline const TupleSet::BitSetData*
  TupleSet::Range::supports(unsigned int n_words, int n) const {
    assert((min <= n) && (n <= max));
    if (s == nullptr)
      return nullptr;
    const unsigned long offset =
      static_cast<unsigned long>(n_words) *
      static_cast<unsigned long>(n - min);
    return s + offset;
  }

  
  /*
   * Tuple set data
   *
   */
  forceinline
  TupleSet::Data::Data(int a) 
    : arity(a), n_words(0U), // To be initialized in finalize
      n_tuples(0), n_free(n_initial_free),
      min(Int::Limits::max), max(Int::Limits::min), key(0),
      td(heap.alloc<int>(n_initial_free * a)),
      vd(heap.alloc<ValueData>(a)),
      range(nullptr), range_base(nullptr), support(nullptr),
      state(TS_BUILDING),
      sparse_n_vals(0U), sparse_offsets(nullptr),
      sparse_tuples(nullptr), sparse_tv(nullptr),
      compressed_offsets(nullptr), compressed_words(nullptr),
      compressed_n_entries(0U) {
  }
  
  forceinline bool
  TupleSet::Data::finalized(void) const {
    return (state == TS_DENSE) ||
      (state == TS_SPARSE) ||
      (state == TS_DENSE_COMPRESSED);
  }

  forceinline bool
  TupleSet::Data::failed(void) const {
    return state == TS_FAILED;
  }

  forceinline bool
  TupleSet::Data::terminal(void) const {
    return state != TS_BUILDING;
  }

  forceinline TupleSet::Tuple
  TupleSet::Data::add(void) {
    if (n_free == 0)
      resize();
    assert(n_free > 0);
    n_free--;
    Tuple t = td + n_tuples*arity;
    n_tuples++;
    return t;
  }

  forceinline TupleSet::Tuple
  TupleSet::Data::get(int i) const {
    assert((i >= 0) && (i < n_tuples));
    return td + i*arity;
  }

  forceinline unsigned int
  TupleSet::ValueData::start(int k) const {
    if (n > 1U) {
      unsigned int l=0U, h=n-1U;
      while (true) {
        assert(l<=h);
        unsigned int m = l + ((h-l) >> 1);
        if (k < r[m].min)
          h=m-1U;
        else if (k > r[m].max)
          l=m+1U;
        else
          return m;
      }
      GECODE_NEVER;
    } else {
      return 0U;
    }
  }

  forceinline void
  TupleSet::Data::set(BitSetData* d, unsigned int i) {
    d[i / BitSetData::bpb].set(i % BitSetData::bpb);
  }

  forceinline bool
  TupleSet::Data::get(const BitSetData* d, unsigned int i) {
    return d[i / BitSetData::bpb].get(i % BitSetData::bpb);
  }

  forceinline unsigned int
  TupleSet::Data::tuple2idx(Tuple t) const {
    return static_cast<unsigned int>((t - td) / static_cast<unsigned int>(arity));
  }

  forceinline const TupleSet::Range*
  TupleSet::Data::fst(int i) const {
    return &vd[i].r[0];
  }
  forceinline const TupleSet::Range*
  TupleSet::Data::lst(int i) const {
    return &vd[i].r[vd[i].n-1U];
  }

  
  /*
   * Tuple set
   *
   */
  forceinline TupleSet&
  TupleSet::add(const IntArgs& t) {
    _add(t); return *this;
  }

  forceinline
  TupleSet::TupleSet(void) {}

  forceinline
  TupleSet::operator bool(void) const {
    return object() != nullptr;
  }

  forceinline void
  TupleSet::finalize(void) {
    Data* d = static_cast<Data*>(object());
    if (d == nullptr)
      throw Int::UninitializedTupleSet("TupleSet::finalize()");
    if (d->failed())
      throw Int::AlreadyFinalized("TupleSet::finalize()");
    if (!d->finalized())
      d->finalize();
  }

  forceinline void
  TupleSet::finalize(ExtensionalPropKind epk) {
    Data* d = static_cast<Data*>(object());
    if (d == nullptr)
      throw Int::UninitializedTupleSet("TupleSet::finalize()");
    if (d->failed())
      throw Int::AlreadyFinalized("TupleSet::finalize()");
    if (!d->finalized())
      d->finalize(epk);
  }

  forceinline bool
  TupleSet::finalized(void) const {
    const Data* d = static_cast<Data*>(object());
    return (d != nullptr) && d->finalized();
  }

  forceinline bool
  TupleSet::failed(void) const {
    const Data* d = static_cast<Data*>(object());
    return (d != nullptr) && d->failed();
  }

  forceinline TupleSet::Data&
  TupleSet::data(void) const {
    Data* d = static_cast<Data*>(object());
    if (d == nullptr)
      throw Int::UninitializedTupleSet("TupleSet");
    if (!d->finalized())
      throw Int::NotYetFinalized("TupleSet");
    return *d;
  }
  forceinline TupleSet::Data&
  TupleSet::raw(void) const {
    Data* d = static_cast<Data*>(object());
    if (d == nullptr)
      throw Int::UninitializedTupleSet("TupleSet");
    return *d;
  }

  forceinline bool
  TupleSet::operator !=(const TupleSet& t) const {
    return !(*this == t);
  }
  forceinline int
  TupleSet::arity(void) const {
    return raw().arity;
  }
  forceinline int
  TupleSet::tuples(void) const {
    return raw().n_tuples;
  }
  forceinline unsigned int
  TupleSet::words(void) const {
    return data().n_words;
  }
  forceinline int
  TupleSet::min(void) const {
    return data().min;
  }
  forceinline int
  TupleSet::max(void) const {
    return data().max;
  }
  forceinline TupleSet::Tuple
  TupleSet::operator [](int i) const {
    return data().get(i);
  }
  forceinline const TupleSet::Range*
  TupleSet::fst(int i) const {
    return data().fst(i);
  }
  forceinline const TupleSet::Range*
  TupleSet::lst(int i) const {
    return data().lst(i);
  }

  forceinline bool
  TupleSet::operator ==(const TupleSet& t) const {
    if (tuples() != t.tuples())
      return false;
    if (arity() != t.arity())
      return false;
    if (min() != t.min())
      return false;
    if (max() != t.max())
      return false;
    return equal(t);
  }

  forceinline std::size_t
  TupleSet::hash(void) const {
    return data().key;
  }

  forceinline ExtensionalPropKind
  TupleSet::representation(void) const {
    switch (data().state) {
    case Data::TS_DENSE:
      return EPK_DENSE;
    case Data::TS_SPARSE:
      return EPK_SPARSE;
    case Data::TS_DENSE_COMPRESSED:
      return EPK_DENSE_COMPRESSED;
    case Data::TS_FAILED:
    case Data::TS_BUILDING:
    default:
      GECODE_NEVER;
      return EPK_DENSE;
    }
  }

  forceinline unsigned int
  TupleSet::sparse_values(void) const {
    return data().sparse_n_vals;
  }

  forceinline const unsigned int*
  TupleSet::sparse_tuple_value_ids(void) const {
    return data().sparse_tv;
  }

  forceinline const unsigned int*
  TupleSet::sparse_support_offsets(void) const {
    return data().sparse_offsets;
  }

  forceinline bool
  TupleSet::support_id(int p, int n, unsigned int& gid) const {
    const Data& d = data();
    if ((p < 0) || (p >= d.arity))
      return false;
    const ValueData& v = d.vd[p];
    if (v.base == nullptr)
      return false;
    unsigned int l = 0U, h = v.n;
    while (l < h) {
      const unsigned int m = l + ((h-l) >> 1);
      if (n < v.r[m].min)
        h = m;
      else if (n > v.r[m].max)
        l = m+1U;
      else {
        gid = v.base[m] + static_cast<unsigned int>(n - v.r[m].min);
        return true;
      }
    }
    return false;
  }

  forceinline bool
  TupleSet::sparse_support(int p, int n,
                           const unsigned int*& b,
                           const unsigned int*& e,
                           unsigned int& gid) const {
    const Data& d = data();
    if ((d.sparse_offsets == nullptr) ||
        (d.sparse_tuples == nullptr) ||
        (d.sparse_n_vals == 0U))
      return false;
    if (!support_id(p,n,gid))
      return false;
    b = d.sparse_tuples + d.sparse_offsets[gid];
    e = d.sparse_tuples + d.sparse_offsets[gid+1U];
    return true;
  }

  forceinline bool
  TupleSet::dense_compressed_support(int p, int n,
                                     const CSupportWord*& b,
                                     const CSupportWord*& e) const {
    const Data& d = data();
    if ((d.compressed_offsets == nullptr) ||
        (d.compressed_words == nullptr))
      return false;
    unsigned int support_id0 = 0U;
    if (!support_id(p,n,support_id0))
      return false;
    b = d.compressed_words + d.compressed_offsets[support_id0];
    e = d.compressed_words + d.compressed_offsets[support_id0+1U];
    return true;
  }

  namespace Int { namespace Extensional {

    forceinline bool
    support_offsets_size(unsigned long long n_vals,
                         unsigned int& n_offsets) {
      if (n_vals >= static_cast<unsigned long long>
          (std::numeric_limits<unsigned int>::max()))
        return false;
      n_offsets = static_cast<unsigned int>(n_vals) + 1U;
      return true;
    }

    forceinline unsigned int
    TupleSetAccess::sparse_values(const TupleSet& ts) {
      return ts.sparse_values();
    }

    forceinline const unsigned int*
    TupleSetAccess::sparse_tuple_value_ids(const TupleSet& ts) {
      return ts.sparse_tuple_value_ids();
    }

    forceinline const unsigned int*
    TupleSetAccess::sparse_support_offsets(const TupleSet& ts) {
      return ts.sparse_support_offsets();
    }

    forceinline bool
    TupleSetAccess::support_id(const TupleSet& ts, int p, int n,
                               unsigned int& gid) {
      return ts.support_id(p,n,gid);
    }

    forceinline bool
    TupleSetAccess::sparse_support(const TupleSet& ts, int p, int n,
                                   const unsigned int*& b,
                                   const unsigned int*& e,
                                   unsigned int& gid) {
      return ts.sparse_support(p,n,b,e,gid);
    }

    forceinline bool
    TupleSetAccess::dense_compressed_support(const TupleSet& ts, int p, int n,
                                             const TupleSet::CSupportWord*& b,
                                             const TupleSet::CSupportWord*& e) {
      return ts.dense_compressed_support(p,n,b,e);
    }

  }}


  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const TupleSet& ts) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << "Number of tuples: " << ts.tuples() 
      << " (number of words: " << ts.words() << " with " 
      << Support::BitSetData::bpb << " bits)" << std::endl;
    for (int a=0; a < ts.arity(); a++) {
      unsigned int size = 0U;
      for (const TupleSet::Range* c=ts.fst(a); c<=ts.lst(a); c++)
        size += c->width();
      s << "\t[" << a << "] size: " << size
        << ", width: " 
        << static_cast<unsigned int>(ts.lst(a)->max - ts.fst(a)->min + 1)
        << ", ranges: "
        << (ts.lst(a) - ts.fst(a) + 1U)
        << std::endl;
    }
    return os << s.str();
  }


  /*
   * Range iterator
   *
   */
  forceinline
  TupleSet::Ranges::Ranges(const TupleSet& ts, int i) {
    c = &(ts.data().vd[i].r[0]);
    l = c + ts.data().vd[i].n;
  }

  forceinline bool
  TupleSet::Ranges::operator ()(void) const {
    return c<l;
  }
  forceinline void
  TupleSet::Ranges::operator ++(void) {
    c++;
  }

  forceinline int
  TupleSet::Ranges::min(void) const {
    return c->min;
  }
  forceinline int
  TupleSet::Ranges::max(void) const {
    return c->max;
  }
  forceinline unsigned int
  TupleSet::Ranges::width(void) const {
    return c->width();
  }

}

// STATISTICS: int-prop
