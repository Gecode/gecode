/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
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
 */

#include <gecode/word.hh>
#include <algorithm>
#include <limits>
#include <numeric>

namespace Gecode {

  class WordTupleSet::Data : public SharedHandle::Object {
  public:
    int arity;
    unsigned int* widths;
    /// First bit-support index of each column
    int* offset;
    /// Total width over all columns
    int bits;
    int n_tuples;
    bool finalized;
    /// Number of 64-row bitmap blocks
    int blocks;

    Data(const IntArgs& w)
      : arity(w.size()), widths(nullptr), offset(nullptr), bits(0),
        n_tuples(0), finalized(false), blocks(0),
        capacity(0), rows(nullptr), ones(nullptr), orders(nullptr) {
      if (arity > std::numeric_limits<int>::max()/128)
        throw Word::OutOfLimits("WordTupleSet");
      for (int i=0; i<arity; i++)
        if ((w[i] < 1) || (w[i] > 64))
          throw Word::OutOfLimits("WordTupleSet");
      widths=allocate<unsigned int>(arity);
      try {
        offset=allocate<int>(arity);
      } catch (...) {
        // The destructor does not run when the constructor throws.
        release(widths);
        throw;
      }
      for (int i=0; i<arity; i++) {
        widths[i]=static_cast<unsigned int>(w[i]);
        offset[i]=bits;
        bits+=w[i];
      }
    }
    virtual ~Data(void) {
      release(widths); release(offset);
      release(rows); release(ones); release(orders);
    }
    void add(const WordValArgs& row) {
      if (finalized)
        throw Word::AlreadyFinalized("WordTupleSet::add");
      if ((row.size() != arity) ||
          (n_tuples == std::numeric_limits<int>::max()-63))
        throw Word::OutOfLimits("WordTupleSet::add");
      for (int i=0; i<arity; i++)
        if ((widths[i] < 64) && (row[i] >> widths[i]))
          throw Word::OutOfLimits("WordTupleSet::add");
      if (n_tuples == capacity) grow();
      std::copy(row.begin(),row.end(),rows+cell(n_tuples));
      n_tuples++;
    }
    void finalize(void) {
      if (finalized) return;
      canonicalize_rows();
      blocks=(n_tuples+63)/64;
      // Take ownership only once both allocations succeed, so a retry
      // after a failure does not overwrite an owned block.
      WordValue* new_ones=allocate<WordValue>(static_cast<size_t>(bits)*blocks);
      int* new_orders;
      try {
        new_orders=allocate<int>(2*static_cast<size_t>(arity)*n_tuples);
      } catch (...) {
        release(new_ones);
        throw;
      }
      ones=new_ones; orders=new_orders;
      for (int i=0; i<arity; i++) {
        build_bit_supports(i);
        build_column_order(i);
      }
      finalized=true;
    }

    WordValue value(int t, int i) const { return rows[cell(t)+i]; }
    /// Rows with bit \a bit set; rows without it are the complement
    const WordValue* support(int bit) const {
      return ones+static_cast<size_t>(bit)*blocks;
    }
    /// Rows of column \a i ordered by \a rank_order, or nullptr if the
    /// order is not an integer order
    const int* order(int i, Word::IntegerRankOrder rank_order) const {
      switch (rank_order) {
      case Word::IntegerRankOrder::UNSIGNED:
        return orders+2*static_cast<size_t>(i)*n_tuples;
      case Word::IntegerRankOrder::SIGNED:
        return orders+(2*static_cast<size_t>(i)+1)*n_tuples;
      case Word::IntegerRankOrder::NONE:
      case Word::IntegerRankOrder::NON_INTEGER:
        return nullptr;
      }
      GECODE_NEVER;
      return nullptr;
    }
  private:
    /// Allocated row capacity
    int capacity;
    /// Row-major values, arity per row
    WordValue* rows;
    /// One bitmap of blocks per column bit
    WordValue* ones;
    /// Two row orders per column, of n_tuples each
    int* orders;

    template<class T> static T* allocate(size_t n) {
      return (n == 0) ? nullptr : heap.alloc<T>(n);
    }
    template<class T> static void release(T* p) {
      if (p != nullptr) heap.rfree(p);
    }
    size_t cell(int t) const { return static_cast<size_t>(t)*arity; }
    WordValue* row(int t) { return rows+cell(t); }
    void grow(void) {
      int n=(capacity < (std::numeric_limits<int>::max()-63)/2) ?
        std::max(2*capacity,16) : std::numeric_limits<int>::max()-63;
      if (arity > 0)
        rows=heap.realloc<WordValue>(rows,cell(capacity),cell(n));
      capacity=n;
    }
    /// Sort rows lexicographically and remove duplicates.
    void canonicalize_rows(void) {
      if (n_tuples == 0) return;
      int* sorted=heap.alloc<int>(n_tuples);
      std::iota(sorted,sorted+n_tuples,0);
      std::sort(sorted,sorted+n_tuples,[&](int a, int b) {
        return std::lexicographical_compare(row(a),row(a)+arity,
                                            row(b),row(b)+arity);
      });
      int distinct=static_cast<int>(
        std::unique(sorted,sorted+n_tuples,[&](int a, int b) {
          return std::equal(row(a),row(a)+arity,row(b));
        })-sorted);
      WordValue* canonical;
      try {
        canonical=allocate<WordValue>(cell(distinct));
      } catch (...) {
        heap.rfree(sorted);
        throw;
      }
      for (int t=0; t<distinct; t++)
        std::copy(row(sorted[t]),row(sorted[t])+arity,canonical+cell(t));
      heap.rfree(sorted);
      release(rows);
      rows=canonical; n_tuples=distinct; capacity=distinct;
    }
    void build_bit_supports(int i) {
      for (int t=0; t<n_tuples; t++)
        for (unsigned int b=0; b<widths[i]; b++)
          if (value(t,i) & (WordValue(1)<<b))
            ones[static_cast<size_t>(offset[i]+b)*blocks+t/64] |=
              WordValue(1)<<(t%64);
    }
    /// Unsigned row order, followed by its sign-boundary rotation.
    void build_column_order(int i) {
      int* unsigned_order=orders+2*static_cast<size_t>(i)*n_tuples;
      int* signed_order=unsigned_order+n_tuples;
      std::iota(unsigned_order,unsigned_order+n_tuples,0);
      // Rows are already lexicographically sorted on the first column.
      if (i != 0)
        std::sort(unsigned_order,unsigned_order+n_tuples,[&](int a, int b) {
          return value(a,i) < value(b,i);
        });
      WordValue sign=WordValue(1)<<(widths[i]-1);
      int* split=std::lower_bound(unsigned_order,unsigned_order+n_tuples,sign,
        [&](int t, WordValue v) { return value(t,i) < v; });
      int* rest=std::copy(split,unsigned_order+n_tuples,signed_order);
      std::copy(unsigned_order,split,rest);
    }
  };

  WordTupleSet::WordTupleSet(const IntArgs& widths)
    : SharedHandle(new Data(widths)) {}
  const WordTupleSet::Data& WordTupleSet::data(void) const {
    return *static_cast<const Data*>(object());
  }
  WordTupleSet& WordTupleSet::add(const WordValArgs& row) {
    static_cast<Data*>(object())->add(row);
    return *this;
  }
  void WordTupleSet::finalize(void) { static_cast<Data*>(object())->finalize(); }
  bool WordTupleSet::finalized(void) const { return data().finalized; }
  int WordTupleSet::arity(void) const { return data().arity; }
  int WordTupleSet::tuples(void) const { return data().n_tuples; }
  unsigned int WordTupleSet::width(int i) const { return data().widths[i]; }
  WordValue WordTupleSet::value(int t, int i) const { return data().value(t,i); }

  namespace Word { namespace Extensional {

    class Compact : public Propagator {
    protected:
      ViewArray<WordView> x;
      WordTupleSet table;
      WordValue* live;
      int* index;
      int limit; // Active prefix of index; it can only shrink.
      int* residue; // Cached bitmap-block indices, rechecked against live.
      int* lower; // Monotone endpoints in each column's rank ordering.
      int* upper;
      WordValue* previous_lo; // Masks after the last successful publication.
      WordValue* previous_hi;

      /// Rows of column \a i in its integer rank order, or nullptr if the
      /// column has no integer rank order
      const int* rows_by_rank(int i) const {
        return table.data().order(i,integer_rank_order(x[i].domain_type()));
      }
      bool alive(int t) const {
        return (live[t/64] & (WordValue(1)<<(t%64))) != 0;
      }
      void remove(int t) { live[t/64] &= ~(WordValue(1)<<(t%64)); }
      void compact(void) {
        for (int p=limit; p--;)
          if (!live[index[p]]) index[p]=index[--limit];
      }
      bool witness(int bit, bool one) {
        const WordValue* support=table.data().support(bit);
        int& r=residue[2*bit+one];
        if (live[r] & (one ? support[r] : ~support[r])) return true;
        for (int p=0; p<limit; p++) {
          int k=index[p];
          if (live[k] & (one ? support[k] : ~support[k])) {
            r=k; return true;
          }
        }
        return false;
      }
      void initialize_buffers(Space& space) {
        const int n=table.data().blocks;
        live=space.alloc<WordValue>(n);
        index=space.alloc<int>(n);
        residue=space.alloc<int>(table.data().bits*2);
        lower=space.alloc<int>(x.size());
        upper=space.alloc<int>(x.size());
        previous_lo=space.alloc<WordValue>(x.size());
        previous_hi=space.alloc<WordValue>(x.size());
        for (int k=0; k<n; k++) { live[k]=~WordValue(0); index[k]=k; }
        if (table.tuples()%64) live[n-1]=(WordValue(1)<<(table.tuples()%64))-1;
        std::fill(residue,residue+table.data().bits*2,0);
        for (int i=0; i<x.size(); i++) {
          lower[i]=0; upper[i]=table.tuples()-1;
          previous_lo[i]=0; previous_hi[i]=x[i].mask();
        }
      }
      void remove_aliased_rows(void) {
        for (int i=0; i<x.size(); i++) {
          // An aliased column must agree within each individual tuple.
          for (int j=0; j<i; j++)
            if (x[i] == x[j])
              for (int r=0; r<table.tuples(); r++)
                if (table.value(r,i) != table.value(r,j)) remove(r);
        }
      }
      Compact(Home home, ViewArray<WordView>& v, const WordTupleSet& t)
        : Propagator(home), x(v), table(t), limit(t.data().blocks) {
        initialize_buffers(home);
        remove_aliased_rows();
        x.subscribe(home,*this,PC_WORD_DOM);
        home.notice(*this,AP_DISPOSE);
      }
      Compact(Space& home, Compact& p)
        : Propagator(home,p), table(p.table), limit(p.limit) {
        x.update(home,p.x);
        int n=table.data().blocks;
        live=home.alloc<WordValue>(n);
        std::copy(p.live,p.live+n,live);
        index=home.alloc<int>(limit);
        std::copy(p.index,p.index+limit,index);
        int nr=table.data().bits*2;
        residue=home.alloc<int>(nr);
        std::copy(p.residue,p.residue+nr,residue);
        lower=home.alloc<int>(x.size()); upper=home.alloc<int>(x.size());
        previous_lo=home.alloc<WordValue>(x.size());
        previous_hi=home.alloc<WordValue>(x.size());
        std::copy(p.lower,p.lower+x.size(),lower);
        std::copy(p.upper,p.upper+x.size(),upper);
        std::copy(p.previous_lo,p.previous_lo+x.size(),previous_lo);
        std::copy(p.previous_hi,p.previous_hi+x.size(),previous_hi);
      }
    public:
      static void post(Home home, ViewArray<WordView>& x, const WordTupleSet& t) {
        (void) new (home) Compact(home,x,t);
      }
      virtual Actor* copy(Space& home) { return new (home) Compact(home,*this); }
      virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::HI,x.size());
      }
      virtual void reschedule(Space& home) { x.reschedule(home,*this,PC_WORD_DOM); }
      virtual size_t dispose(Space& home) {
        home.ignore(*this,AP_DISPOSE);
        x.cancel(home,*this,PC_WORD_DOM);
        table.~WordTupleSet();
        (void) Propagator::dispose(home);
        return sizeof(*this);
      }
      virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        do {
          if (!filter_rows()) return ES_FAILED;
          for (int i=0; i<x.size(); i++)
            GECODE_ES_CHECK(narrow_column(home,i));
        } while (has_unfiltered_column());
        bool assigned=true;
        for (int i=0; i<x.size(); i++)
          assigned=assigned && x[i].assigned();
        // Every remaining tuple survives its own hull, including aliases.
        return assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
    private:
      /// Intersect only changed bits and newly excluded interval endpoints.
      bool filter_rows(void) {
        const auto& d=table.data();
        // Only newly fixed bits need to intersect the reversible sparse table.
        for (int i=0; i<x.size(); i++) {
          WordValue changed=(x[i].lo()^previous_lo[i]) |
                            (x[i].hi()^previous_hi[i]);
          for (unsigned int b=0; b<x[i].width(); b++) {
            WordValue mask=WordValue(1)<<b;
            if (!(changed & mask)) continue;
            const WordValue* support=d.support(d.offset[i]+b);
            bool one=(x[i].lo() & mask) != 0;
            for (int p=limit; p--;) {
              int k=index[p];
              live[k] &= one ? support[k] : ~support[k];
              if (!live[k]) index[p]=index[--limit];
            }
          }
          if (const int* o=rows_by_rank(i)) {
            WordValue lo=x[i].rank_minimum(), hi=x[i].rank_maximum();
            while (lower[i]<=upper[i] &&
                   rank(x[i].domain_type(),x[i].width(),d.value(o[lower[i]],i))<lo)
              remove(o[lower[i]++]);
            while (lower[i]<=upper[i] &&
                   rank(x[i].domain_type(),x[i].width(),d.value(o[upper[i]],i))>hi)
              remove(o[upper[i]--]);
          }
        }
        compact();
        return limit != 0;
      }
      /// Publish one live-column hull, then record its successful mask tells.
      ExecStatus narrow_column(Space& home, int i) {
        const auto& d=table.data();
        WordValue lo=x[i].lo(), hi=x[i].hi(), unknown=x[i].unknown();
        for (unsigned int b=0; b<x[i].width(); b++) {
          WordValue mask=WordValue(1)<<b;
          if (!(unknown & mask)) continue;
          if (!witness(d.offset[i]+b,false)) lo |= mask;
          if (!witness(d.offset[i]+b,true)) hi &= ~mask;
        }
        GECODE_ME_CHECK(x[i].narrow(home,lo,hi));
        if (const int* o=rows_by_rank(i)) {
          while (!alive(o[lower[i]])) lower[i]++;
          while (!alive(o[upper[i]])) upper[i]--;
          // The live endpoint rows lie in the column, so one tell narrows
          // it to their ranks.
          const WordValue minimum=
            rank(x[i].domain_type(),x[i].width(),d.value(o[lower[i]],i));
          const WordValue maximum=
            rank(x[i].domain_type(),x[i].width(),d.value(o[upper[i]],i));
          GECODE_ME_CHECK(x[i].narrow_rank_range(home,minimum,maximum));
        }
        // Closing cube and interval keeps every live row of a column whose
        // rows are cut by its interval, as each lies in both. A value set
        // can drop the value of a live row, and a float interval cuts no
        // rows, so only the published masks count as filtered for such a
        // column, and its closed interval can exclude the endpoint rows.
        const bool may_drop_live_rows=x[i].has_value_set() ||
          (x[i].bounded() && (rows_by_rank(i) == nullptr));
        if (may_drop_live_rows) {
          previous_lo[i]=lo; previous_hi[i]=hi;
        } else {
          previous_lo[i]=x[i].lo(); previous_hi[i]=x[i].hi();
        }
        return ES_OK;
      }
      /// Test whether a column has bits or bounds that rows are not filtered by
      bool has_unfiltered_column(void) {
        const auto& d=table.data();
        for (int i=0; i<x.size(); i++) {
          if ((x[i].lo() != previous_lo[i]) || (x[i].hi() != previous_hi[i]))
            return true;
          if (!x[i].has_value_set())
            continue;
          if (const int* o=rows_by_rank(i)) {
            const WordDomainType kind=x[i].domain_type();
            const WordValue first=rank(kind,x[i].width(),d.value(o[lower[i]],i));
            const WordValue last=rank(kind,x[i].width(),d.value(o[upper[i]],i));
            if ((first < x[i].rank_minimum()) || (last > x[i].rank_maximum()))
              return true;
          }
        }
        return false;
      }
    };
  }}

  void extensional(Home home, const WordVarArgs& x, const WordTupleSet& t) {
    if (!t.finalized()) throw Word::NotYetFinalized("Word::extensional");
    if (x.size() != t.arity()) throw Word::OutOfLimits("Word::extensional");
    for (int i=0; i<x.size(); i++)
      if (x[i].width() != t.width(i))
        throw Word::WidthMismatch("Word::extensional");
    GECODE_POST;
    if (!t.tuples()) { home.fail(); return; }
    if (!x.size()) return;
    ViewArray<Word::WordView> views(home,x);
    Word::Extensional::Compact::post(home,views,t);
  }
}

// STATISTICS: word-prop
