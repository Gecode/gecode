/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
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

#include <gecode/int.hh>
#include <algorithm>
#include <limits>

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
    for (int i=0; i<arity; i++)
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
  TupleSet::Data::clear_support(void) {
    heap.rfree(range);
    heap.rfree(range_base);
    heap.rfree(support);
    heap.rfree(sparse_offsets);
    heap.rfree(sparse_tuples);
    heap.rfree(sparse_tv);
    heap.rfree(compressed_offsets);
    heap.rfree(compressed_words);
    range = nullptr;
    range_base = nullptr;
    support = nullptr;
    sparse_n_vals = 0U;
    sparse_offsets = nullptr;
    sparse_tuples = nullptr;
    sparse_tv = nullptr;
    compressed_offsets = nullptr;
    compressed_words = nullptr;
    compressed_n_entries = 0U;
    for (int a=0; a<arity; a++) {
      vd[a].n = 0U;
      vd[a].r = nullptr;
      vd[a].base = nullptr;
    }
  }

  TupleSet::Data::State
  TupleSet::Data::empty_state(ExtensionalPropKind epk) const {
    switch (epk) {
    case EPK_AUTO:
    case EPK_DENSE:
      return TS_DENSE;
    case EPK_SPARSE:
      return TS_SPARSE;
    case EPK_DENSE_COMPRESSED:
      return TS_DENSE_COMPRESSED;
    default:
      GECODE_NEVER;
    }
    return TS_FAILED;
  }

  TupleSet::Data::State
  TupleSet::Data::select_state(
    ExtensionalPropKind epk, bool dense_possible,
    bool sparse_possible, bool compressed_possible,
    bool support_bits_sparse, unsigned long long dense_bytes) const {
    const unsigned long long dense_small_threshold =
      2ULL * 1024ULL * 1024ULL;
    switch (epk) {
    case EPK_AUTO:
      if (dense_possible &&
          ((dense_bytes <= dense_small_threshold) || !support_bits_sparse))
        return TS_DENSE;
      if (compressed_possible)
        return TS_DENSE_COMPRESSED;
      if (dense_possible)
        return TS_DENSE;
      break;
    case EPK_DENSE:
      if (dense_possible)
        return TS_DENSE;
      break;
    case EPK_SPARSE:
      if (sparse_possible)
        return TS_SPARSE;
      break;
    case EPK_DENSE_COMPRESSED:
      if (compressed_possible)
        return TS_DENSE_COMPRESSED;
      break;
    default:
      GECODE_NEVER;
    }
    throw Int::OutOfLimits("TupleSet::finalize()");
  }

  void
  TupleSet::Data::finalize(void) {
    finalize(EPK_DENSE);
  }

  void
  TupleSet::Data::finalize(ExtensionalPropKind epk) {
    using namespace Int::Extensional;
    assert(!terminal());

    state = TS_FAILED;

    class FinalizeGuard {
    protected:
      Data& d;
      bool committed;
    public:
      FinalizeGuard(Data& d0) : d(d0), committed(false) {}
      ~FinalizeGuard(void) {
        if (!committed) {
          d.clear_support();
          d.n_words = 0U;
          d.min = Int::Limits::max;
          d.max = Int::Limits::min;
          d.key = 0U;
          d.state = TS_FAILED;
        }
      }
      void commit(State selected) {
        assert((selected != TS_BUILDING) && (selected != TS_FAILED));
        d.state = selected;
        committed = true;
      }
    } guard(*this);

    // Initialization
    if (n_tuples == 0) {
      const State selected = empty_state(epk);
      heap.rfree(td);
      td=nullptr;
      guard.commit(selected);
      return;
    }

    // Compact and copy data
    Region region;
    // Set up tuple pointers
    Tuple* tuples = region.alloc<Tuple>(n_tuples);
    {
      for (int t=0; t<n_tuples; t++)
        tuples[t] = td + t*arity;
      TupleCompare tc(arity);
      Support::quicksort(tuples, n_tuples, tc);
      // Remove duplicates
      int j=1;
      for (int t=1; t<n_tuples; t++) {
        for (int a=0; a<arity; a++)
          if (tuples[t-1][a] != tuples[t][a])
            goto notsame;
        goto same;
      notsame: ;
        tuples[j++] = tuples[t];
      same: ;
      }
      assert(j <= n_tuples);
      n_tuples=j;
      // Initialize hash key
      key = static_cast<std::size_t>(n_tuples);
      cmb_hash(key, arity);
      // Copy into now possibly smaller area
      const unsigned long long n_tcells64 =
        static_cast<unsigned long long>(n_tuples) *
        static_cast<unsigned long long>(arity);
      if (n_tcells64 > static_cast<unsigned long long>
          (std::numeric_limits<unsigned long>::max()))
        throw Int::OutOfLimits("TupleSet::finalize()");
      const unsigned long n_tcells = static_cast<unsigned long>(n_tcells64);
      for (int t=0; t<n_tuples; t++)
        for (int a=0; a<arity; a++)
          if (!Int::Limits::valid(tuples[t][a]))
            throw Int::OutOfLimits("TupleSet::finalize()");
      int* new_td = heap.alloc<int>(n_tcells);
      for (int t=0; t<n_tuples; t++) {
        for (int a=0; a<arity; a++) {
          new_td[static_cast<unsigned long>(t) *
                 static_cast<unsigned long>(arity) +
                 static_cast<unsigned long>(a)] = tuples[t][a];
          cmb_hash(key,tuples[t][a]);
        }
        tuples[t] = new_td + static_cast<unsigned long>(t) *
          static_cast<unsigned long>(arity);
      }
      heap.rfree(td);
      td = new_td;
    }
    
    // Only now compute how many words are needed!
    n_words = BitSetData::data(static_cast<unsigned int>(n_tuples));

    State selected = TS_BUILDING;
    // Compute range information
    {
      /*
       * Pass one: compute how many values and ranges are needed
       */
      // How many values
      unsigned long long n_vals64 = 0ULL;
      // How many ranges
      unsigned long long n_ranges64 = 0ULL;
      for (int a=0; a<arity; a++) {
        // Sort tuple according to position
        PosCompare pc(a);
        Support::quicksort(tuples, n_tuples, pc);
        // Scan values
        {
          int last_value=tuples[0][a];
          n_vals64++; n_ranges64++;
          for (int i=1; i<n_tuples; i++) {
            assert(tuples[i-1][a] <= tuples[i][a]);
            if (last_value+1 == tuples[i][a]) {
              n_vals64++;
              last_value=tuples[i][a];
            } else if (last_value+1 < tuples[i][a]) {
              n_vals64++; n_ranges64++;
              last_value=tuples[i][a];
            } else {
              assert(last_value == tuples[i][a]);
            }
          }
        }
      }
      const unsigned long long max_u64 =
        std::numeric_limits<unsigned long long>::max();
      const bool support_entries_overflow =
        (n_words != 0U) &&
        (n_vals64 > max_u64 / static_cast<unsigned long long>(n_words));
      const unsigned long long n_support_entries64 =
        support_entries_overflow ? max_u64 :
        static_cast<unsigned long long>(n_words) * n_vals64;
      const unsigned long long n_tcells64 =
        static_cast<unsigned long long>(n_tuples) *
        static_cast<unsigned long long>(arity);
      const unsigned long long sparse_support_cells_per_entry =
        static_cast<unsigned long long>(BitSetData::bpb / 4U);
      const bool support_bits_sparse =
        support_entries_overflow ||
        (n_support_entries64 > max_u64 / sparse_support_cells_per_entry) ||
        (n_tcells64 <=
         n_support_entries64 * sparse_support_cells_per_entry);
      const bool dense_possible = !support_entries_overflow &&
        (n_support_entries64 <= static_cast<unsigned long long>
         (std::numeric_limits<unsigned int>::max()));
      unsigned int n_offsets = 0U;
      const bool support_offsets_possible =
        support_offsets_size(n_vals64,n_offsets);
      const bool sparse_possible =
        (n_tcells64 <=
         static_cast<unsigned long long>(std::numeric_limits<unsigned int>::max())) &&
        support_offsets_possible;
      const bool compressed_possible = sparse_possible;
      const unsigned long long dense_bytes =
        (support_entries_overflow ||
         (n_support_entries64 > max_u64 / sizeof(BitSetData))) ? max_u64 :
        n_support_entries64 * static_cast<unsigned long long>(sizeof(BitSetData));

      // AUTO keeps small or genuinely dense payloads dense. Larger sparse
      // support matrices use the shared compressed representation; the sparse
      // representation is selected only when explicitly requested.
      selected = select_state(epk,dense_possible,sparse_possible,
                              compressed_possible,support_bits_sparse,
                              dense_bytes);
      const bool build_dense = (selected == TS_DENSE);
      const bool build_sparse = (selected == TS_SPARSE);
      const bool build_compressed = (selected == TS_DENSE_COMPRESSED);
      if ((n_vals64 > static_cast<unsigned long long>
           (std::numeric_limits<unsigned int>::max())) ||
          (n_ranges64 > static_cast<unsigned long long>
           (std::numeric_limits<unsigned int>::max())))
        throw Int::OutOfLimits("TupleSet::finalize()");
      const unsigned int n_vals = static_cast<unsigned int>(n_vals64);
      const unsigned int n_ranges = static_cast<unsigned int>(n_ranges64);
      /*
       * Pass 2: allocate memory and fill data structures
       */
      // Allocate memory for ranges
      Range* range_cursor = range = heap.alloc<Range>(n_ranges);
      // Allocate and initialize memory for support data.
      support = nullptr;
      sparse_n_vals = 0U;
      sparse_offsets = nullptr;
      sparse_tuples = nullptr;
      sparse_tv = nullptr;
      compressed_offsets = nullptr;
      compressed_words = nullptr;
      compressed_n_entries = 0U;
      BitSetData* support_cursor = nullptr;
      unsigned int n_support_entries_u32 = 0U;
      if (build_dense) {
        assert((n_words == 0U) ||
               (n_vals <= std::numeric_limits<unsigned int>::max() / n_words));
        n_support_entries_u32 = n_words * n_vals;
        support_cursor = support = heap.alloc<BitSetData>(n_support_entries_u32);
        for (unsigned int i=0; i<n_support_entries_u32; i++)
          support_cursor[i].init();
      }
      for (int a=0; a<arity; a++) {
        // Set range pointer
        vd[a].r = range_cursor;
        vd[a].base = nullptr;
        // Sort tuple according to position
        PosCompare pc(a);
        Support::quicksort(tuples, n_tuples, pc);
        // Update min and max
        min = std::min(min,tuples[0][a]);
        max = std::max(max,tuples[n_tuples-1][a]);
        // Compress into non-overlapping ranges
        {
          unsigned int j=0U;
          vd[a].r[0].max=vd[a].r[0].min=tuples[0][a];
          for (int i=1; i<n_tuples; i++) {
            assert(tuples[i-1][a] <= tuples[i][a]);
            if (vd[a].r[j].max+1 == tuples[i][a]) {
              vd[a].r[j].max=tuples[i][a];
            } else if (vd[a].r[j].max+1 < tuples[i][a]) {
              j++; vd[a].r[j].min=vd[a].r[j].max=tuples[i][a];
            } else {
              assert(vd[a].r[j].max == tuples[i][a]);
            }
          }
          vd[a].n = j+1U;
          range_cursor += j+1U;
        }
        // Set support pointer and set bits
        for (unsigned int i=0U; i<vd[a].n; i++) {
          vd[a].r[i].s = support_cursor;
          if (build_dense)
            support_cursor += n_words * vd[a].r[i].width();
        }
        if (build_dense) {
          int j=0;
          for (int i=0; i<n_tuples; i++) {
            while (tuples[i][a] > vd[a].r[j].max)
              j++;
            set(const_cast<BitSetData*>
                (vd[a].r[j].supports(n_words,tuples[i][a])),
                tuple2idx(tuples[i]));
          }
        }
      }
      if (build_sparse || build_compressed) {
        assert(support_offsets_possible);
        assert(n_tcells64 <=
               static_cast<unsigned long long>
               (std::numeric_limits<unsigned int>::max()));
        const unsigned int n_tcells = static_cast<unsigned int>(n_tcells64);
        range_base = (n_ranges > 0U) ?
          heap.alloc<unsigned int>(n_ranges) : nullptr;
        unsigned int* base_cursor = range_base;
        unsigned int first_support_id = 0U;
        for (int a=0; a<arity; a++) {
          vd[a].base = base_cursor;
          for (unsigned int i=0U; i<vd[a].n; i++) {
            vd[a].base[i] = first_support_id;
            first_support_id += vd[a].r[i].width();
          }
          base_cursor += vd[a].n;
        }
        assert(first_support_id == n_vals);
        assert((n_ranges == 0U) ||
               (base_cursor == range_base + n_ranges));

        unsigned int* support_offsets;
        if (build_sparse) {
          sparse_n_vals = n_vals;
          sparse_offsets = heap.alloc<unsigned int>(n_offsets);
          sparse_tv = (n_tcells > 0U) ?
            heap.alloc<unsigned int>(n_tcells) : nullptr;
          support_offsets = sparse_offsets;
        } else {
          support_offsets = region.alloc<unsigned int>(n_offsets);
        }
        for (unsigned int i=0U; i<n_offsets; i++)
          support_offsets[i] = 0U;
        for (unsigned int tid=0U;
             tid<static_cast<unsigned int>(n_tuples); tid++) {
          const Tuple t = td + tid*arity;
          for (int a=0; a<arity; a++) {
            const unsigned int range_index = vd[a].start(t[a]);
            const unsigned int support_id =
              vd[a].base[range_index] +
              static_cast<unsigned int>
              (t[a] - vd[a].r[range_index].min);
            if (build_sparse)
              sparse_tv[tid*arity+a] = support_id;
            support_offsets[support_id+1U]++;
          }
        }

        for (unsigned int i=1U; i<n_offsets; i++)
          support_offsets[i] += support_offsets[i-1U];

        if (build_sparse) {
          sparse_tuples = (n_tcells > 0U) ?
            heap.alloc<unsigned int>(n_tcells) : nullptr;
          unsigned int* next_support = (n_vals > 0U) ?
            region.alloc<unsigned int>(n_vals) : nullptr;
          for (unsigned int i=0U; i<n_vals; i++)
            next_support[i] = sparse_offsets[i];
          for (unsigned int tid=0U;
               tid<static_cast<unsigned int>(n_tuples); tid++) {
            for (int a=0; a<arity; a++) {
              const unsigned int support_id = sparse_tv[tid*arity+a];
              sparse_tuples[next_support[support_id]++] = tid;
            }
          }
        } else {
          unsigned int* tuples_by_support = (n_tcells > 0U) ?
            region.alloc<unsigned int>(n_tcells) : nullptr;
          unsigned int* next_support = (n_vals > 0U) ?
            region.alloc<unsigned int>(n_vals) : nullptr;
          for (unsigned int i=0U; i<n_vals; i++)
            next_support[i] = support_offsets[i];
          // Tuple ids are visited in increasing order, so each support list
          // is already sorted by tuple id (and hence by word index).
          for (unsigned int tid=0U;
               tid<static_cast<unsigned int>(n_tuples); tid++) {
            const Tuple t = td + tid*arity;
            for (int a=0; a<arity; a++) {
              const unsigned int range_index = vd[a].start(t[a]);
              const unsigned int support_id =
                vd[a].base[range_index] +
                static_cast<unsigned int>
                (t[a] - vd[a].r[range_index].min);
              tuples_by_support[next_support[support_id]++] = tid;
            }
          }

          compressed_offsets = heap.alloc<unsigned int>(n_offsets);
          compressed_offsets[0] = 0U;
          unsigned long long entries64 = 0ULL;
          for (unsigned int gid=0U; gid<n_vals; gid++) {
            unsigned int count = 0U;
            unsigned int last_widx = std::numeric_limits<unsigned int>::max();
            for (unsigned int p=support_offsets[gid];
                 p<support_offsets[gid+1U]; p++) {
              const unsigned int tid = tuples_by_support[p];
              const unsigned int widx = tid / BitSetData::bpb;
              if (widx != last_widx) {
                count++;
                last_widx = widx;
              }
            }
            entries64 += static_cast<unsigned long long>(count);
            if (entries64 >
                static_cast<unsigned long long>(std::numeric_limits<unsigned int>::max()))
              throw Int::OutOfLimits("TupleSet::finalize()");
            compressed_offsets[gid+1U] = static_cast<unsigned int>(entries64);
          }
          compressed_n_entries = static_cast<unsigned int>(entries64);
          compressed_words = (compressed_n_entries > 0U) ?
            heap.alloc<CSupportWord>(compressed_n_entries) : nullptr;

          unsigned int out = 0U;
          for (unsigned int gid=0U; gid<n_vals; gid++) {
            unsigned int current_widx = std::numeric_limits<unsigned int>::max();
            BitSetData bits;
            bits.init(false);
            const unsigned int begin = support_offsets[gid];
            const unsigned int end = support_offsets[gid+1U];
            for (unsigned int p=begin; p<end; p++) {
              const unsigned int tid = tuples_by_support[p];
              const unsigned int widx = tid / BitSetData::bpb;
              if (widx != current_widx) {
                if (current_widx != std::numeric_limits<unsigned int>::max()) {
                  compressed_words[out].widx = current_widx;
                  compressed_words[out].bits = bits;
                  out++;
                }
                current_widx = widx;
                bits.init(false);
              }
              bits.set(tid % BitSetData::bpb);
            }
            if (current_widx != std::numeric_limits<unsigned int>::max()) {
              compressed_words[out].widx = current_widx;
              compressed_words[out].bits = bits;
              out++;
            }
            assert(out == compressed_offsets[gid+1U]);
          }
          assert(out == compressed_n_entries);
        }
      }
      if (build_dense)
        assert(support_cursor == support + n_support_entries_u32);
      assert(range_cursor == range + n_ranges);
    }
    if ((min < Int::Limits::min) || (max > Int::Limits::max))
      throw Int::OutOfLimits("TupleSet::finalize()");
    guard.commit(selected);
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
    clear_support();
    heap.rfree(td);
    heap.rfree(vd);
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

  TupleSet::TupleSet(int a, const Gecode::DFA& dfa)
    : TupleSet(a,dfa,EPK_DENSE) {}

  TupleSet::TupleSet(int a, const Gecode::DFA& dfa,
                     ExtensionalPropKind epk) {
    /// Edges in layered graph
    struct Edge {
      int i_state; ///< Number of in-state
      int o_state; ///< Number of out-state
    };
    /// State in layered graph
    struct State {
      int i_deg; ///< In-degree (number of incoming arcs)
      int o_deg; ///< Out-degree (number of outgoing arcs)
      int n_tuples; ///< Number of tuples
      int* tuples; ///< The tuples
    };
    /// Support for a value
    struct Support {
      int val; ///< Supported value
      int n_edges; ///< Number of supporting edges
      Edge* edges; ///< Supporting edges
    };
    /// Layer in layered graph
    struct Layer {
      State* states; ///< States
      Support* supports; ///< Supported values
      int n_supports; ///< Number of supported values
    };
    // Initialize
    object(new Data(a));

    Region r;
    // Number of states
    int max_states = dfa.n_states();
    // Allocate memory for all layers and states
    Layer* layers = r.alloc<Layer>(a+1);
    State* states = r.alloc<State>(max_states*(a+1));

    for (int i=0; i<max_states*(a+1); i++) {
      states[i].i_deg = 0; states[i].o_deg = 0;
      states[i].n_tuples = 0;
      states[i].tuples = nullptr;
    }
    for (int i=0; i<a+1; i++) {
      layers[i].states = states + i*max_states;
      layers[i].n_supports = 0;
    }

    // Mark initial state as being reachable
    layers[0].states[0].i_deg = 1;
    layers[0].states[0].n_tuples = 1;
    layers[0].states[0].tuples = r.alloc<int>(1);
    assert(layers[0].states[0].tuples != nullptr);
  
    // Allocate temporary memory for edges and supports
    Edge* edges = r.alloc<Edge>(dfa.max_degree());
    Support* supports = r.alloc<Support>(dfa.n_symbols());
  
    // Forward pass: accumulate
    for (int i=0; i<a; i++) {
      int n_supports=0;
      for (DFA::Symbols s(dfa); s(); ++s) {
        int n_edges=0;
        for (DFA::Transitions t(dfa,s.val()); t(); ++t) {
          if (layers[i].states[t.i_state()].i_deg != 0) {
            // Create edge
            edges[n_edges].i_state = t.i_state();
            edges[n_edges].o_state = t.o_state();
            n_edges++;
            // Adjust degrees
            layers[i].states[t.i_state()].o_deg++;
            layers[i+1].states[t.o_state()].i_deg++;
            // Adjust number of tuples
            layers[i+1].states[t.o_state()].n_tuples
              += layers[i].states[t.i_state()].n_tuples;
          }
          assert(static_cast<unsigned int>(n_edges) <= dfa.max_degree());
        }
        // Found a support for the value
        if (n_edges > 0) {
          Support& support = supports[n_supports++];
          support.val = s.val();
          support.n_edges = n_edges;
          support.edges = Heap::copy(r.alloc<Edge>(n_edges),edges,n_edges);
        }
      }
      // Create supports
      if (n_supports > 0) {
        layers[i].supports =
          Heap::copy(r.alloc<Support>(n_supports),supports,n_supports);
        layers[i].n_supports = n_supports;
      } else {
        finalize(epk);
        return;
      }
    }

    // Mark final states as being reachable
    for (int s=dfa.final_fst(); s<dfa.final_lst(); s++) {
      if (layers[a].states[s].i_deg != 0U)
        layers[a].states[s].o_deg = 1U;
    }

    // Backward pass: validate
    for (int i=a; i--; ) {
      for (int j = layers[i].n_supports; j--; ) {
        Support& s = layers[i].supports[j];
        for (int k = s.n_edges; k--; ) {
          int i_state = s.edges[k].i_state;
          int o_state = s.edges[k].o_state;
          // State is unreachable
          if (layers[i+1].states[o_state].o_deg == 0) {
            // Adjust degree
            --layers[i+1].states[o_state].i_deg;
            --layers[i].states[i_state].o_deg;
            // Remove edge
            assert(s.n_edges > 0);
            s.edges[k] = s.edges[--s.n_edges];
          }
        }
        // Lost support
        if (s.n_edges == 0)
          layers[i].supports[j] = layers[i].supports[--layers[i].n_supports];
      }
      if (layers[i].n_supports == 0U) {
        finalize(epk);
        return;
      }
    }

    // Generate tuples
    for (int i=0; i<a; i++) {
      for (int j = layers[i].n_supports; j--; ) {
        Support& s = layers[i].supports[j];
        for (int k = s.n_edges; k--; ) {
          int i_state = s.edges[k].i_state;
          int o_state = s.edges[k].o_state;
          // Allocate memory for tuples if not done
          if (layers[i+1].states[o_state].tuples == nullptr) {
            int n_tuples = layers[i+1].states[o_state].n_tuples;
            layers[i+1].states[o_state].tuples = r.alloc<int>((i+1)*n_tuples);
            layers[i+1].states[o_state].n_tuples = 0;
          }
          int n = layers[i+1].states[o_state].n_tuples;
          // Write tuples
          for (int t=0; t < layers[i].states[i_state].n_tuples; t++) {
            // Copy the first i number of digits from the previous layer
            Heap::copy(&layers[i+1].states[o_state].tuples[n*(i+1)+t*(i+1)],
                       &layers[i].states[i_state].tuples[t*i], i);
            // Write the last digit
            layers[i+1].states[o_state].tuples[n*(i+1)+t*(i+1)+i] = s.val;
          }
          layers[i+1].states[o_state].n_tuples
            += layers[i].states[i_state].n_tuples;
        }
      }
    }

    // Add tuples to tuple set
    for (int s = dfa.final_fst(); s < dfa.final_lst(); s++) {
      for (int i=0; i<layers[a].states[s].n_tuples; i++) {
        int* tuple = &layers[a].states[s].tuples[i*a];
        add(IntArgs(a,tuple));
      }
    }
  
    finalize(epk);
  } 

  bool
  TupleSet::equal(const TupleSet& t) const {
    assert(tuples() == t.tuples());
    assert(arity() == t.arity());
    assert(min() == t.min());
    assert(max() == t.max());
    for (int i=0; i<tuples(); i++)
      for (int j=0; j<arity(); j++)
        if ((*this)[i][j] != t[i][j])
          return false;
    return true;
  }

  void
  TupleSet::_add(const IntArgs& t) {
    if (!*this)
      throw Int::UninitializedTupleSet("TupleSet::add()");
    if (raw().terminal())
      throw Int::AlreadyFinalized("TupleSet::add()");
    if (t.size() != raw().arity)
      throw Int::ArgumentSizeMismatch("TupleSet::add()");
    Tuple a = raw().add();
    for (int i=0; i<t.size(); i++)
      a[i]=t[i];
  }

}

// STATISTICS: int-prop
