/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include <climits>
#include <algorithm>
#include <limits>

namespace Gecode { namespace Int { namespace Extensional {

  /**
   * \brief Traits class for variables
   *
   * Each variable must specialize this traits class and add a \code
   * typedef \endcode for the view \a View corresponding to this variable.
   */
  template<class Var>
  class VarTraits {};

  /**
   * \brief Traits class for variables
   *
   * This class specializes the VarTraits for integer variables.
   */
  template<>
  class VarTraits<IntVar> {
  public:
    /// The variable type of an IntView
    typedef Int::IntView View;
  };

  /**
   * \brief Traits class for variables
   *
   * This class specializes the VarTraits for Boolean variables.
   */
  template<>
  class VarTraits<BoolVar> {
  public:
    /// The variable type of an IntView
    typedef Int::BoolView View;
  };


  /*
   * States
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::State::init(void) {
    i_deg=o_deg=0;
  }


  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename LayeredGraph<View,Val,Degree,StateIdx>::State&
  LayeredGraph<View,Val,Degree,StateIdx>::i_state(int i, StateIdx is) {
    return layers[i].states[is];
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename LayeredGraph<View,Val,Degree,StateIdx>::State&
  LayeredGraph<View,Val,Degree,StateIdx>::i_state
  (int i, const typename LayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return i_state(i,e.i_state);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Val,Degree,StateIdx>::i_dec
  (int i, const typename LayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return --i_state(i,e).o_deg == 0;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename LayeredGraph<View,Val,Degree,StateIdx>::State&
  LayeredGraph<View,Val,Degree,StateIdx>::o_state(int i, StateIdx os) {
    return layers[i+1].states[os];
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline typename LayeredGraph<View,Val,Degree,StateIdx>::State&
  LayeredGraph<View,Val,Degree,StateIdx>::o_state
  (int i, const typename LayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return o_state(i,e.o_state);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Val,Degree,StateIdx>::o_dec
  (int i, const typename LayeredGraph<View,Val,Degree,StateIdx>::Edge& e) {
    return --o_state(i,e).i_deg == 0;
  }


  /*
   * Value iterator
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>::LayerValues::LayerValues(void) {}
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>::LayerValues
  ::LayerValues(const Layer& l)
    : s1(l.support), s2(l.support+l.size) {}
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::LayerValues::init(const Layer& l) {
    s1=l.support; s2=l.support+l.size;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Val,Degree,StateIdx>::LayerValues
  ::operator ()(void) const {
    return s1<s2;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::LayerValues::operator ++(void) {
    s1++;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Val,Degree,StateIdx>::LayerValues::val(void) const {
    return s1->val;
  }


  /*
   * Index advisors
   *
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>::Index::Index(Space& home, Propagator& p,
                                                       Council<Index>& c,
                                                       int i0)
    : Advisor(home,p,c), i(i0) {}

  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>::Index::Index(Space& home, Index& a)
    : Advisor(home,a), i(a.i) {}


  /*
   * Index ranges
   *
   */
  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::IndexRange(void)
    : _fst(INT_MAX), _lst(INT_MIN) {}
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::reset(void) {
    _fst=INT_MAX; _lst=INT_MIN;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::add(int i) {
    _fst=std::min(_fst,i); _lst=std::max(_lst,i);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::add
  (const typename LayeredGraph<View,Val,Degree,StateIdx>::IndexRange& ir) {
    _fst=std::min(_fst,ir._fst); _lst=std::max(_lst,ir._lst);
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::empty(void) const {
    return _fst>_lst;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::lshift(int n) {
    if (empty())
      return;
    if (n > _lst) {
      reset();
    } else {
      _fst = std::max(0,_fst-n);
      _lst -= n;
    }
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::fst(void) const {
    return _fst;
  }
  template<class View, class Val, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Val,Degree,StateIdx>::IndexRange::lst(void) const {
    return _lst;
  }



  /*
   * The layered graph
   *
   */

  template<class View, class Val, class Degree, class StateIdx>
  template<class Var>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>::LayeredGraph(Home home,
                                                       const VarArgArray<Var>& x,
                                                       const DFA& dfa)
    : Propagator(home), c(home), n(x.size()),
      max_states(static_cast<StateIdx>(dfa.n_states())) {
    assert(n > 0);
  }

  template<class View, class Val, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Val,Degree,StateIdx>::audit(void) {
#ifdef GECODE_AUDIT
    // Check states and edge information to be consistent
    unsigned int n_e = 0; // Number of edges
    unsigned int n_s = 0; // Number of states
    StateIdx m_s = 0; // Maximal number of states per layer
    for (int i=n; i--; ) {
      n_s += layers[i].n_states;
      m_s = std::max(m_s,layers[i].n_states);
      for (ValSize j=layers[i].size; j--; )
        n_e += layers[i].support[j].n_edges;
    }
    n_s += layers[n].n_states;
    m_s = std::max(m_s,layers[n].n_states);
    assert(n_e == n_edges);
    assert(n_s <= n_states);
    assert(m_s <= max_states);
#endif
  }

  template<class View, class Val, class Degree, class StateIdx>
  template<class Var>
  forceinline ExecStatus
  LayeredGraph<View,Val,Degree,StateIdx>::initialize(Space& home,
                                                     const VarArgArray<Var>& x,
                                                     const DFA& dfa) {

    // Allocate memory for layers
    layers = home.alloc<Layer>(n+1);

    const int n_dfa_states = dfa.n_states();

    {
      Region r;

      // Allocate temporary memory for edges
      Edge* edges = r.alloc<Edge>(dfa.max_degree());

      // Reachability sets and touched states
      unsigned char* i_reachable = r.alloc<unsigned char>(n_dfa_states);
      unsigned char* o_reachable = r.alloc<unsigned char>(n_dfa_states);
      StateIdx* i_reached = r.alloc<StateIdx>(n_dfa_states);
      StateIdx* o_reached = r.alloc<StateIdx>(n_dfa_states);
      for (int i=0; i<n_dfa_states; i++)
        i_reachable[i] = o_reachable[i] = 0;
      unsigned int n_i_reached = 1;
      unsigned int n_o_reached = 0;
      i_reachable[0] = 1;
      i_reached[0] = 0;

      // Forward pass: add transitions
      for (int i=0; i<n; i++) {
        layers[i].x = x[i];
        layers[i].support = home.alloc<Support>(layers[i].x.size());
        ValSize j=0;
        // Clear the next frontier
        for (unsigned int k=0; k<n_o_reached; k++)
          o_reachable[o_reached[k]] = 0;
        n_o_reached = 0;
        // Enter links leaving reachable states
        for (ViewValues<View> nx(layers[i].x); nx(); ++nx) {
          unsigned int n_support_edges=0;
          for (DFA::Transitions t(dfa,nx.val()); t(); ++t)
            if (i_reachable[t.i_state()] != 0) {
              edges[n_support_edges].i_state =
                static_cast<StateIdx>(t.i_state());
              edges[n_support_edges].o_state =
                static_cast<StateIdx>(t.o_state());
              n_support_edges++;
              if (o_reachable[t.o_state()] == 0) {
                o_reachable[t.o_state()] = 1;
                o_reached[n_o_reached++] =
                  static_cast<StateIdx>(t.o_state());
              }
            }
          assert(n_support_edges <= dfa.max_degree());
          // Found support for value
          if (n_support_edges > 0) {
            Support& s = layers[i].support[j];
            s.val = static_cast<Val>(nx.val());
            s.n_edges = static_cast<Degree>(n_support_edges);
            s.edges = Heap::copy(home.alloc<Edge>(n_support_edges),edges,
                                 n_support_edges);
            j++;
          }
        }
        if ((layers[i].size = j) == 0)
          return ES_FAILED;
        std::swap(i_reachable,o_reachable);
        std::swap(i_reached,o_reached);
        std::swap(n_i_reached,n_o_reached);
      }
    }

    Region r;

    // Maps from DFA state IDs to layer-local state indices
    StateIdx* i_map = r.alloc<StateIdx>(n_dfa_states);
    StateIdx* o_map = r.alloc<StateIdx>(n_dfa_states);
    unsigned char* i_mapped = r.alloc<unsigned char>(n_dfa_states);
    unsigned char* o_mapped = r.alloc<unsigned char>(n_dfa_states);
    StateIdx* i_touched = r.alloc<StateIdx>(n_dfa_states);
    StateIdx* o_touched = r.alloc<StateIdx>(n_dfa_states);
    for (int i=0; i<n_dfa_states; i++)
      i_mapped[i] = o_mapped[i] = 0;
    unsigned int n_i_mapped = 0;
    unsigned int n_o_mapped = 0;

    // Count and map reachable edges entering final states
    unsigned long long int final_degree = 0;
    for (ValSize j=0; j<layers[n-1].size; j++) {
      Support& s = layers[n-1].support[j];
      for (unsigned int d=0; d<static_cast<unsigned int>(s.n_edges); d++) {
        const int os = static_cast<int>(s.edges[d].o_state);
        if ((os >= dfa.final_fst()) && (os < dfa.final_lst())) {
          final_degree++;
          if (o_mapped[os] == 0) {
            o_mapped[os] = 1;
            o_map[os] = static_cast<StateIdx>(n_o_mapped);
            o_touched[n_o_mapped++] = static_cast<StateIdx>(os);
          }
        }
      }
    }
    if (final_degree == 0)
      return ES_FAILED;

    // Initialize the map for the terminal layer
    if (final_degree <= static_cast<unsigned long long int>
        (Gecode::Support::IntTypeTraits<Degree>::max)) {
      // Merge all terminal states
      for (unsigned int i=0; i<n_o_mapped; i++)
        o_map[o_touched[i]] = 0;
      layers[n].n_states = 1;
    } else {
      // Keep terminal states separate
      layers[n].n_states = static_cast<StateIdx>(n_o_mapped);
    }

    unsigned long long int total_states = layers[n].n_states;
    unsigned long long int total_edges = 0;
    unsigned int max_s = layers[n].n_states;

    // Backward pass: prune and translate endpoints to layer-local indices
    for (int i=n; i--; ) {
      for (unsigned int j=0; j<n_i_mapped; j++)
        i_mapped[i_touched[j]] = 0;
      n_i_mapped = 0;
      ValSize n_supports=0;
      for (ValSize j=0; j<layers[i].size; j++) {
        Support& s = layers[i].support[j];
        unsigned int n_support_edges=0;
        for (unsigned int d=0; d<static_cast<unsigned int>(s.n_edges); d++) {
          Edge e = s.edges[d];
          const int is = static_cast<int>(e.i_state);
          const int os = static_cast<int>(e.o_state);
          if (o_mapped[os] != 0) {
            e.o_state = o_map[os];
            if (i_mapped[is] == 0) {
              i_mapped[is] = 1;
              i_map[is] = static_cast<StateIdx>(n_i_mapped);
              i_touched[n_i_mapped++] = static_cast<StateIdx>(is);
            }
            e.i_state = i_map[is];
            s.edges[n_support_edges++] = e;
          }
        }
        s.n_edges = static_cast<Degree>(n_support_edges);
        if (n_support_edges > 0) {
          layers[i].support[n_supports++] = s;
          total_edges += n_support_edges;
        }
      }
      if ((layers[i].size = n_supports) == 0)
        return ES_FAILED;
      layers[i].n_states = static_cast<StateIdx>(n_i_mapped);
      total_states += n_i_mapped;
      max_s = std::max(max_s,n_i_mapped);
      LayerValues lv(layers[i]);
      GECODE_ME_CHECK(layers[i].x.narrow_v(home,lv,false));
      if (!layers[i].x.assigned())
        layers[i].x.subscribe(home, *new (home) Index(home,*this,c,i));

      std::swap(i_map,o_map);
      std::swap(i_mapped,o_mapped);
      std::swap(i_touched,o_touched);
      std::swap(n_i_mapped,n_o_mapped);
    }

    if ((total_states >
         static_cast<unsigned long long int>
         ((std::numeric_limits<unsigned int>::max)())) ||
        (total_edges >
         static_cast<unsigned long long int>
         ((std::numeric_limits<unsigned int>::max)())))
      throw OutOfLimits("Int::regular");
    n_states = static_cast<unsigned int>(total_states);
    n_edges = static_cast<unsigned int>(total_edges);
    max_states = static_cast<StateIdx>(max_s);

    // Allocate persistent states in terminal-first, descending-layer order
    State* states = home.alloc<State>(n_states);
    for (unsigned int i=0; i<n_states; i++)
      states[i].init();
    for (int i=n+1; i--; ) {
      layers[i].states = states;
      states += layers[i].n_states;
    }

    // Reconstruct state degrees from the surviving edges
    for (int i=0; i<n; i++)
      for (ValSize j=0; j<layers[i].size; j++) {
        Support& s = layers[i].support[j];
        for (Degree d=s.n_edges; d--; ) {
          i_state(i,s.edges[d]).o_deg++;
          o_state(i,s.edges[d]).i_deg++;
        }
      }

    // Restore artificial boundary degrees
    layers[0].states[0].i_deg = 1;
    for (StateIdx i=0; i<layers[n].n_states; i++)
      layers[n].states[i].o_deg = 1;

    // Schedule if subsumption is needed
    if (c.empty())
      View::schedule(home,*this,ME_INT_VAL);

    audit();
    return ES_OK;
  }

  template<class View, class Val, class Degree, class StateIdx>
  ExecStatus
  LayeredGraph<View,Val,Degree,StateIdx>::advise(Space& home,
                                                 Advisor& _a, const Delta& d) {
    // Check whether state information has already been created
    if (layers[0].states == nullptr) {
      State* states = home.alloc<State>(n_states);
      for (unsigned int i=0; i<n_states; i++)
        states[i].init();
      layers[n].states = states;
      states += layers[n].n_states;
      for (int i=n; i--; ) {
        layers[i].states = states;
        states += layers[i].n_states;
        for (ValSize j=layers[i].size; j--; ) {
          Support& s = layers[i].support[j];
          for (Degree deg=s.n_edges; deg--; ) {
            i_state(i,s.edges[deg]).o_deg++;
            o_state(i,s.edges[deg]).i_deg++;
          }
        }
      }
    }

    Index& a = static_cast<Index&>(_a);
    const int i = a.i;

    if (layers[i].size <= layers[i].x.size()) {
      // Propagator has already done everything
      if (View::modevent(d) == ME_INT_VAL) {
        a.dispose(home,c);
        return c.empty() ? ES_NOFIX : ES_FIX;
      } else {
        return ES_FIX;
      }
    }

    bool i_mod = false;
    bool o_mod = false;

    if (View::modevent(d) == ME_INT_VAL) {
      Val n = static_cast<Val>(layers[i].x.val());
      ValSize j=0;
      for (; layers[i].support[j].val < n; j++) {
        Support& s = layers[i].support[j];
        n_edges -= s.n_edges;
        // Supported value not any longer in view
        for (Degree deg=s.n_edges; deg--; ) {
          // Adapt states
          o_mod |= i_dec(i,s.edges[deg]);
          i_mod |= o_dec(i,s.edges[deg]);
        }
      }
      assert(layers[i].support[j].val == n);
      layers[i].support[0] = layers[i].support[j++];
      ValSize s=layers[i].size;
      layers[i].size = 1;
      for (; j<s; j++) {
        Support& ls = layers[i].support[j];
        n_edges -= ls.n_edges;
        for (Degree deg=ls.n_edges; deg--; ) {
          // Adapt states
          o_mod |= i_dec(i,ls.edges[deg]);
          i_mod |= o_dec(i,ls.edges[deg]);
        }
      }
    } else if (layers[i].x.any(d)) {
      ValSize j=0;
      ValSize k=0;
      ValSize s=layers[i].size;
      for (ViewRanges<View> rx(layers[i].x); rx() && (j<s);) {
        Support& ls = layers[i].support[j];
        if (ls.val < static_cast<Val>(rx.min())) {
          // Supported value not any longer in view
          n_edges -= ls.n_edges;
          for (Degree deg=ls.n_edges; deg--; ) {
            // Adapt states
            o_mod |= i_dec(i,ls.edges[deg]);
            i_mod |= o_dec(i,ls.edges[deg]);
          }
          ++j;
        } else if (ls.val > static_cast<Val>(rx.max())) {
          ++rx;
        } else {
          layers[i].support[k++]=ls;
          ++j;
        }
      }
      assert(k > 0);
      layers[i].size = k;
      // Remove remaining values
      for (; j<s; j++) {
        Support& ls=layers[i].support[j];
        n_edges -= ls.n_edges;
        for (Degree deg=ls.n_edges; deg--; ) {
          // Adapt states
          o_mod |= i_dec(i,ls.edges[deg]);
          i_mod |= o_dec(i,ls.edges[deg]);
        }
      }
    } else {
      Val min = static_cast<Val>(layers[i].x.min(d));
      ValSize j=0;
      // Skip values smaller than min (to keep)
      for (; layers[i].support[j].val < min; j++) {}
      Val max = static_cast<Val>(layers[i].x.max(d));
      ValSize k=j;
      ValSize s=layers[i].size;
      // Remove pruned values
      for (; (j<s) && (layers[i].support[j].val <= max); j++) {
        Support& ls=layers[i].support[j];
        n_edges -= ls.n_edges;
        for (Degree deg=ls.n_edges; deg--; ) {
          // Adapt states
          o_mod |= i_dec(i,ls.edges[deg]);
          i_mod |= o_dec(i,ls.edges[deg]);
        }
      }
      // Keep remaining values
      while (j<s)
        layers[i].support[k++]=layers[i].support[j++];
      layers[i].size=k;
      assert(k > 0);
    }

    audit();

    bool fix = true;
    if (o_mod && (i > 0)) {
      o_ch.add(i-1); fix = false;
     }
    if (i_mod && (i+1 < n)) {
      i_ch.add(i+1); fix = false;
    }
    if (fix) {
      if (View::modevent(d) == ME_INT_VAL) {
        a.dispose(home,c);
        return c.empty() ? ES_NOFIX : ES_FIX;
      }
      return ES_FIX;
    } else {
      return (View::modevent(d) == ME_INT_VAL)
        ? home.ES_NOFIX_DISPOSE(c,a) : ES_NOFIX;
    }
  }

  template<class View, class Val, class Degree, class StateIdx>
  forceinline size_t
  LayeredGraph<View,Val,Degree,StateIdx>::dispose(Space& home) {
    c.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Val, class Degree, class StateIdx>
  void
  LayeredGraph<View,Val,Degree,StateIdx>::reschedule(Space& home) {
    View::schedule(home,*this,c.empty() ? ME_INT_VAL : ME_INT_DOM);
  }

  template<class View, class Val, class Degree, class StateIdx>
  ExecStatus
  LayeredGraph<View,Val,Degree,StateIdx>::propagate(Space& home,
                                                    const ModEventDelta&) {
    // Forward pass
    for (int i=i_ch.fst(); i<=i_ch.lst(); i++) {
      bool i_mod = false;
      bool o_mod = false;
      ValSize j=0;
      ValSize k=0;
      ValSize ls=layers[i].size;
      do {
        Support& s=layers[i].support[j];
        n_edges -= s.n_edges;
        for (Degree d=s.n_edges; d--; )
          if (i_state(i,s.edges[d]).i_deg == 0) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            i_mod |= o_dec(i,s.edges[d]);
            // Remove edge
            s.edges[d] = s.edges[--s.n_edges];
          }
        n_edges += s.n_edges;
        // Check whether value is still supported
        if (s.n_edges == 0) {
          layers[i].size--;
          GECODE_ME_CHECK(layers[i].x.nq(home,s.val));
        } else {
          layers[i].support[k++]=s;
        }
      } while (++j<ls);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
      if (i_mod && (i+1 < n))
        i_ch.add(i+1);
    }

    // Backward pass
    for (int i=o_ch.lst(); i>=o_ch.fst(); i--) {
      bool o_mod = false;
      ValSize j=0;
      ValSize k=0;
      ValSize ls=layers[i].size;
      do {
        Support& s=layers[i].support[j];
        n_edges -= s.n_edges;
        for (Degree d=s.n_edges; d--; )
          if (o_state(i,s.edges[d]).o_deg == 0) {
            // Adapt states
            o_mod |= i_dec(i,s.edges[d]);
            (void)   o_dec(i,s.edges[d]);
            // Remove edge
            s.edges[d] = s.edges[--s.n_edges];
          }
        n_edges += s.n_edges;
        // Check whether value is still supported
        if (s.n_edges == 0) {
          layers[i].size--;
          GECODE_ME_CHECK(layers[i].x.nq(home,s.val));
        } else {
          layers[i].support[k++]=s;
        }
      } while (++j<ls);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
    }

    a_ch.add(i_ch); i_ch.reset();
    a_ch.add(o_ch); o_ch.reset();

    audit();

    // Check subsumption
    if (c.empty())
      return home.ES_SUBSUMED(*this);
    else
      return ES_FIX;
  }


  template<class View, class Val, class Degree, class StateIdx>
  template<class Var>
  ExecStatus
  LayeredGraph<View,Val,Degree,StateIdx>::post(Home home,
                                               const VarArgArray<Var>& x,
                                               const DFA& dfa) {
    if (x.size() == 0) {
      // Check whether the start state 0 is also a final state
      if ((dfa.final_fst() <= 0) && (dfa.final_lst() >= 0))
        return ES_OK;
      return ES_FAILED;
    }
    assert(x.size() > 0);
    for (int i=0; i<x.size(); i++) {
      DFA::Symbols s(dfa);
      typename VarTraits<Var>::View xi(x[i]);
      GECODE_ME_CHECK(xi.inter_v(home,s,false));
    }
    LayeredGraph<View,Val,Degree,StateIdx>* p =
      new (home) LayeredGraph<View,Val,Degree,StateIdx>(home,x,dfa);
    return p->initialize(home,x,dfa);
  }

  template<class View, class Val, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Val,Degree,StateIdx>
  ::LayeredGraph(Space& home, LayeredGraph<View,Val,Degree,StateIdx>& p)
    : Propagator(home,p),
      n(p.n), layers(home.alloc<Layer>(n+1)),
      max_states(p.max_states), n_states(p.n_states), n_edges(p.n_edges) {
    c.update(home,p.c);
    // Do not allocate states, postpone to advise!
    layers[n].n_states = p.layers[n].n_states;
    layers[n].states = nullptr;
    // Allocate memory for edges
    Edge* edges = home.alloc<Edge>(n_edges);
    // Copy layers
    for (int i=0; i<n; i++) {
      layers[i].x.update(home,p.layers[i].x);
      assert(layers[i].x.size() == p.layers[i].size);
      layers[i].size = p.layers[i].size;
      layers[i].support = home.alloc<Support>(layers[i].size);
      for (ValSize j=0; j<layers[i].size; j++) {
        layers[i].support[j].val = p.layers[i].support[j].val;
        layers[i].support[j].n_edges = p.layers[i].support[j].n_edges;
        assert(layers[i].support[j].n_edges > 0);
        layers[i].support[j].edges =
          Heap::copy(edges,p.layers[i].support[j].edges,
                     layers[i].support[j].n_edges);
        edges += layers[i].support[j].n_edges;
      }
      layers[i].n_states = p.layers[i].n_states;
      layers[i].states = nullptr;
    }
    audit();
  }

  template<class View, class Val, class Degree, class StateIdx>
  PropCost
  LayeredGraph<View,Val,Degree,StateIdx>::cost(const Space&,
                                               const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,n);
  }

  template<class View, class Val, class Degree, class StateIdx>
  Actor*
  LayeredGraph<View,Val,Degree,StateIdx>::copy(Space& home) {
    // Eliminate an assigned prefix
    {
      int k=0;
      while (layers[k].size == 1) {
        assert(layers[k].support[0].n_edges == 1);
        n_states -= layers[k].n_states;
        k++;
      }
      if (k > 0) {
        /*
         * The state information is always available: either the propagator
         * has been created (hence, also the state information has been
         * created), or the first variable become assigned and hence
         * an advisor must have been run (which then has created the state
         * information).
         */
        // Eliminate assigned layers
        n -= k; layers += k;
        // Eliminate edges
        n_edges -= static_cast<unsigned int>(k);
        // Update advisor indices
        for (Advisors<Index> as(c); as(); ++as)
          as.advisor().i -= k;
        // Update all change information
        a_ch.lshift(k);
      }
    }
    audit();

    // Compress states
    if (!a_ch.empty()) {
      int f = a_ch.fst();
      int l = a_ch.lst();
      assert((f >= 0) && (l <= n));
      Region r;
      // State map for in-states
      StateIdx* i_map = r.alloc<StateIdx>(max_states);
      // State map for out-states
      StateIdx* o_map = r.alloc<StateIdx>(max_states);
      // Number of in-states
      StateIdx i_n = 0;

      n_states -= layers[l].n_states;
      // Initialize map for in-states and compress
      for (StateIdx j=0; j<layers[l].n_states; j++)
        if ((layers[l].states[j].i_deg != 0) ||
            (layers[l].states[j].o_deg != 0)) {
          layers[l].states[i_n]=layers[l].states[j];
          i_map[j]=i_n++;
        }
      layers[l].n_states = i_n;
      n_states += layers[l].n_states;
      assert(i_n > 0);

      // Update in-states in edges for last layer, if any
      if (l < n)
        for (ValSize j=layers[l].size; j--; ) {
          Support& s = layers[l].support[j];
          for (Degree d=s.n_edges; d--; )
            s.edges[d].i_state = i_map[s.edges[d].i_state];
        }

      // Update all changed layers
      for (int i=l-1; i>=f; i--) {
        // In-states become out-states
        std::swap(o_map,i_map); i_n=0;
        // Initialize map for in-states and compress
        n_states -= layers[i].n_states;
        for (StateIdx j=0; j<layers[i].n_states; j++)
          if ((layers[i].states[j].o_deg != 0) ||
              (layers[i].states[j].i_deg != 0)) {
            layers[i].states[i_n]=layers[i].states[j];
            i_map[j]=i_n++;
          }
        layers[i].n_states = i_n;
        n_states += layers[i].n_states;
        assert(i_n > 0);

        // Update states in edges
        for (ValSize j=layers[i].size; j--; ) {
          Support& s = layers[i].support[j];
          for (Degree d=s.n_edges; d--; ) {
            s.edges[d].i_state = i_map[s.edges[d].i_state];
            s.edges[d].o_state = o_map[s.edges[d].o_state];
          }
        }
      }

      // Update out-states in edges for previous layer, if any
      if (f > 0)
        for (ValSize j=layers[f-1].size; j--; ) {
          Support& s = layers[f-1].support[j];
          for (Degree d=s.n_edges; d--; )
            s.edges[d].o_state = i_map[s.edges[d].o_state];
        }

      a_ch.reset();
    }
    audit();

    return new (home) LayeredGraph<View,Val,Degree,StateIdx>(home,*this);
  }

  /// Select small types for the layered graph propagator
  template<class Var>
  forceinline ExecStatus
  post_lgp(Home home, const VarArgArray<Var>& x, const DFA& dfa) {
    Gecode::Support::IntType t_state_idx =
      Gecode::Support::u_type(static_cast<unsigned int>(dfa.n_states()));
    Gecode::Support::IntType t_degree =
      Gecode::Support::u_type(dfa.max_degree());
    Gecode::Support::IntType t_val =
      std::max(Support::s_type(dfa.symbol_min()),
               Support::s_type(dfa.symbol_max()));
    switch (t_val) {
    case Gecode::Support::IT_CHAR:
      // fall through
    case Gecode::Support::IT_SHRT:
      switch (t_state_idx) {
      case Gecode::Support::IT_CHAR:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned char,unsigned char>
            ::post(home,x,dfa);
        case Gecode::Support::IT_SHRT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned short int,unsigned char>
            ::post(home,x,dfa);
        case Gecode::Support::IT_INT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned int,unsigned char>
            ::post(home,x,dfa);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_SHRT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned char,unsigned short int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_SHRT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned short int,unsigned short int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_INT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned int,unsigned short int>
            ::post(home,x,dfa);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_INT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned char,unsigned int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_SHRT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned short int,unsigned int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_INT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,short int,unsigned int,unsigned int>
            ::post(home,x,dfa);
        default: GECODE_NEVER;
        }
        break;
      default: GECODE_NEVER;
      }
      break;
    case Gecode::Support::IT_INT:
      switch (t_state_idx) {
      case Gecode::Support::IT_CHAR:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned char,unsigned char>
            ::post(home,x,dfa);
        case Gecode::Support::IT_SHRT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned short int,unsigned char>
            ::post(home,x,dfa);
        case Gecode::Support::IT_INT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned int,unsigned char>
            ::post(home,x,dfa);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_SHRT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned char,unsigned short int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_SHRT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned short int,unsigned short int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_INT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned int,unsigned short int>
            ::post(home,x,dfa);
        default: GECODE_NEVER;
        }
        break;
      case Gecode::Support::IT_INT:
        switch (t_degree) {
        case Gecode::Support::IT_CHAR:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned char,unsigned int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_SHRT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned short int,unsigned int>
            ::post(home,x,dfa);
        case Gecode::Support::IT_INT:
          return Extensional::LayeredGraph
            <typename VarTraits<Var>::View,int,unsigned int,unsigned int>
            ::post(home,x,dfa);
        default: GECODE_NEVER;
        }
        break;
      default: GECODE_NEVER;
      }

    default: GECODE_NEVER;
    }
    return ES_OK;
  }

}}}

// STATISTICS: int-prop
