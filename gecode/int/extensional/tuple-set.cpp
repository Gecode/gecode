/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Lagerkvist, 2007
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
      for (int t=0; t<n_tuples; t++)
        tuple[t] = td + t*arity;
      TupleCompare tc(arity);
      Support::quicksort(tuple, n_tuples, tc);
      // Remove duplicates
      int j=1;
      for (int t=1; t<n_tuples; t++) {
        for (int a=0; a<arity; a++)
          if (tuple[t-1][a] != tuple[t][a])
            goto notsame;
        goto same;
      notsame: ;
        tuple[j++] = tuple[t];
      same: ;
      }
      assert(j <= n_tuples);
      n_tuples=j;
      // Initialize hash key
      key = static_cast<std::size_t>(n_tuples);
      cmb_hash(key, arity);
      // Copy into now possibly smaller area
      int* new_td = heap.alloc<int>(n_tuples*arity);
      for (int t=0; t<n_tuples; t++) {
        for (int a=0; a<arity; a++) {
          new_td[t*arity+a] = tuple[t][a];
          cmb_hash(key,tuple[t][a]);
        }
        tuple[t] = new_td + t*arity;
      }
      heap.rfree(td);
      td = new_td;
    }
    
    // Only now compute how many tuples are needed!
    n_words = BitSetData::data(static_cast<unsigned int>(n_tuples));

    // Compute range information
    {
      /*
       * Pass one: compute how many values and ranges are needed
       */
      // How many values
      unsigned int n_vals = 0U;
      // How many ranges
      unsigned int n_ranges = 0U;
      for (int a=0; a<arity; a++) {
        // Sort tuple according to position
        PosCompare pc(a);
        Support::quicksort(tuple, n_tuples, pc);
        // Scan values
        {
          int max=tuple[0][a];
          n_vals++; n_ranges++;
          for (int i=1; i<n_tuples; i++) {
            assert(tuple[i-1][a] <= tuple[i][a]);
            if (max+1 == tuple[i][a]) {
              n_vals++;
              max=tuple[i][a];
            } else if (max+1 < tuple[i][a]) {
              n_vals++; n_ranges++;
              max=tuple[i][a];
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
      for (unsigned int i=0; i<n_vals * n_words; i++)
        cs[i].init();
      for (int a=0; a<arity; a++) {
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

  TupleSet::TupleSet(int a, const Gecode::DFA& dfa) {
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
          assert(n_edges <= dfa.max_degree());
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
        finalize();
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
        finalize();
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
  
    finalize();
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
    if (raw().finalized())
      throw Int::AlreadyFinalized("TupleSet::add()");
    if (t.size() != raw().arity)
      throw Int::ArgumentSizeMismatch("TupleSet::add()");
    Tuple a = raw().add();
    for (int i=0; i<t.size(); i++)
      a[i]=t[i];
  }

}

// STATISTICS: int-prop

