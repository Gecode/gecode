/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include <climits>
#include <algorithm>

namespace Gecode { namespace Int { namespace Extensional {

  /*
   * States
   */
  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::State::State(void) 
    : i_deg(0), o_deg(0) {}


  /*
   * Value iterator
   */
  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::LayerValues::LayerValues(void) {}
  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::LayerValues::LayerValues(const Layer& l)
    : s1(l.support), s2(l.support+l.size) {}
  template<class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::LayerValues::init(const Layer& l) {
    s1=l.support; s2=l.support+l.size;
  }
  template<class View, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Degree,StateIdx>::LayerValues::operator ()(void) const {
    return s1<s2;
  }
  template<class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::LayerValues::operator ++(void) {
    s1++;
  }
  template<class View, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Degree,StateIdx>::LayerValues::val(void) const {
    return s1->val;
  }


  /*
   * Index advisors
   *
   */
  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::Index::Index(Space& home, Propagator& p,
                                                   Council<Index>& c,
                                                   StateIdx i0)
    : Advisor(home,p,c), i(i0) {}

  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::Index::Index(Space& home, bool share,
                                                   Index& a)
    : Advisor(home,share,a), i(a.i) {}


  /*
   * Index ranges
   *
   */
  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::IndexRange::IndexRange(void)
    : _fst(INT_MAX), _lst(INT_MIN) {}
  template<class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::IndexRange::reset(void) {
    _fst=INT_MAX; _lst=INT_MIN;
  }
  template<class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::IndexRange::add(int i) {
    _fst=std::min(_fst,i); _lst=std::max(_lst,i);
  }
  template<class View, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Degree,StateIdx>::IndexRange::fst(void) const {
    return _fst;
  }
  template<class View, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Degree,StateIdx>::IndexRange::lst(void) const {
    return _lst;
  }



  /*
   * The layered graph
   *
   */

  template<class View, class Degree, class StateIdx>
  template<class Var>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::LayeredGraph(Space& home,
                                                   const VarArgArray<Var>& x, 
                                                   const DFA& dfa)
    : Propagator(home), c(home), n(x.size()), n_states(dfa.n_states()) {
    assert(n > 0);
  }

  template<class View, class Degree, class StateIdx>
  template<class Var>
  forceinline ExecStatus
  LayeredGraph<View,Degree,StateIdx>::initialize(Space& home,
                                                 const VarArgArray<Var>& x, 
                                                 const DFA& dfa) {
    // Allocate memory
    layers = home.alloc<Layer>(n+2)+1;
    states = home.alloc<State>((n+1)*n_states);

    // Mark initial state as being reachable
    states[0].i_deg = 1;

    // Mark final states as reachable as well
    for (int s = dfa.final_fst(); s < dfa.final_lst(); s++)
      states[n*n_states + s].o_deg = 1;

    // Temporary memory for edges
    Region r(home);
    Edge* edges = r.alloc<Edge>(dfa.max_degree());

    // Forward pass: add transitions
    for (int i=0; i<n; i++) {
      layers[i].x = x[i];
      layers[i].support = home.alloc<Support>(layers[i].x.size());
      unsigned int j=0;
      // Enter links leaving reachable states (indegree != 0)
      for (ViewValues<View> nx(layers[i].x); nx(); ++nx) {
        Degree n_edges=0;
        for (DFA::Transitions t(dfa,nx.val()); t(); ++t)
          if (states[i*n_states + t.i_state()].i_deg != 0) {
            StateIdx i_s = static_cast<StateIdx>(i*n_states + t.i_state());
            states[i_s].o_deg++;
            StateIdx o_s = static_cast<StateIdx>((i+1)*n_states +  t.o_state());
            states[o_s].i_deg++;
            edges[n_edges].i_state = i_s;
            edges[n_edges].o_state = o_s;
            n_edges++;
          }
        assert(n_edges <= dfa.max_degree());
        // Found support for value
        if (n_edges > 0) {
          layers[i].support[j].val = nx.val();
          layers[i].support[j].n_edges = n_edges;
          layers[i].support[j].edges = home.alloc<Edge>(n_edges);
          for (Degree d=n_edges; d--; )
            layers[i].support[j].edges[d] = edges[d];
          j++;
        }
      }
      if ((layers[i].size = j) == 0)
        return ES_FAILED;
    }

    // Backward pass: prune all transitions that do not lead to final state
    for (int i=n; i--; ) {
      unsigned int k=0;
      for (unsigned int j=0; j<layers[i].size; j++) {
        for (Degree d=layers[i].support[j].n_edges; d--; )
          if (states[layers[i].support[j].edges[d].o_state].o_deg == 0) {
            // Adapt states
            states[layers[i].support[j].edges[d].i_state].o_deg--;
            states[layers[i].support[j].edges[d].o_state].i_deg--;
            // Unreachable, prune edge
            layers[i].support[j].edges[d] = 
              layers[i].support[j].edges[--layers[i].support[j].n_edges];
          }
        // Value has support, copy the support information
        if (layers[i].support[j].n_edges > 0)
          layers[i].support[k++]=layers[i].support[j];
      }
      if ((layers[i].size = k) == 0)
        return ES_FAILED;
      LayerValues lv(layers[i]);
      GECODE_ME_CHECK(layers[i].x.narrow_v(home,lv,false));
      if (!layers[i].x.assigned())
        layers[i].x.subscribe(home, *new (home) Index(home,*this,c,i));
    }
    // Schedule if subsumption is needed
    if (c.empty())
      View::schedule(home,*this,ME_INT_VAL);
    return ES_OK;
  }

  template<class View, class Degree, class StateIdx>
  ExecStatus
  LayeredGraph<View,Degree,StateIdx>::advise(Space& home,
                                             Advisor& _a, const Delta& d) {
    // Check whether state information has already been created
    if (states == NULL) {
      states = home.alloc<State>((n+1)*n_states);
      for (int i=n; i--; )
        for (unsigned int j=layers[i].size; j--; )
          for (Degree d=layers[i].support[j].n_edges; d--; ) {
            states[layers[i].support[j].edges[d].i_state].o_deg++;
            states[layers[i].support[j].edges[d].o_state].i_deg++;
          }
    }
    
    Index& a = static_cast<Index&>(_a);
    Layer& l = layers[a.i];

    if (l.size <= l.x.size())
      // Propagator has already done everything
      if (View::modevent(d) == ME_INT_VAL) {
        a.dispose(home,c);
        return c.empty() ? ES_NOFIX : ES_FIX;
      } else {
        return ES_FIX;
      }

    bool i_mod = false;
    bool o_mod = false;

    if (View::modevent(d) == ME_INT_VAL) {
      int n = l.x.val();
      unsigned int j=0;
      for (; l.support[j].val < n; j++)
        // Supported value not any longer in view
        for (Degree d=l.support[j].n_edges; d--; ) {
          // Adapt states
          o_mod |= ((--states[l.support[j].edges[d].i_state].o_deg) == 0);
          i_mod |= ((--states[l.support[j].edges[d].o_state].i_deg) == 0);
        }
      assert(l.support[j].val == n);
      l.support[0] = l.support[j++];
      unsigned int s=l.size;
      l.size = 1;
      for (; j<s; j++)
        for (Degree d=l.support[j].n_edges; d--; ) {
          // Adapt states
          o_mod |= ((--states[l.support[j].edges[d].i_state].o_deg) == 0);
          i_mod |= ((--states[l.support[j].edges[d].o_state].i_deg) == 0);
        }
    } else if (l.x.any(d)) {
      unsigned int j=0;
      unsigned int k=0;
      unsigned int s=l.size;
      for (ViewRanges<View> rx(l.x); rx() && (j<s);)
        if (l.support[j].val < rx.min()) {
          // Supported value not any longer in view
          for (Degree d=l.support[j].n_edges; d--; ) {
            // Adapt states
            o_mod |= ((--states[l.support[j].edges[d].i_state].o_deg) == 0);
            i_mod |= ((--states[l.support[j].edges[d].o_state].i_deg) == 0);
          }
          ++j;
        } else if (l.support[j].val > rx.max()) {
          ++rx;
        } else {
          l.support[k++]=l.support[j++];
        }
      assert(k > 0);
      l.size = k;
      // Remove remaining values
      for (; j<s; j++)
        for (Degree d=l.support[j].n_edges; d--; ) {
          // Adapt states
          o_mod |= ((--states[l.support[j].edges[d].i_state].o_deg) == 0);
          i_mod |= ((--states[l.support[j].edges[d].o_state].i_deg) == 0);
        }
    } else {
      int min = l.x.min(d);
      unsigned int j=0;
      // Skip values smaller than min (to keep)
      for (; l.support[j].val < min; j++) {}
      int max = l.x.max(d);
      unsigned int k=j;
      unsigned int s=l.size;
      // Remove pruned values
      for (; (j<s) && (l.support[j].val <= max); j++)
        for (Degree d=l.support[j].n_edges; d--; ) {
          // Adapt states
          o_mod |= ((--states[l.support[j].edges[d].i_state].o_deg) == 0);
          i_mod |= ((--states[l.support[j].edges[d].o_state].i_deg) == 0);
        }
      // Keep remaining values
      while (j<s)
        l.support[k++]=l.support[j++];
      l.size =k;
      assert(k > 0);
    }

    bool fix = true;
    if (o_mod && (a.i > 0)) {
      o_ch.add(a.i-1); fix = false;
     }
    if (i_mod && (a.i+1 < n)) {
      i_ch.add(a.i+1); fix = false;
    }
    if (fix) {
      if (View::modevent(d) == ME_INT_VAL) {
        a.dispose(home,c);
        return c.empty() ? ES_NOFIX : ES_FIX;
      }
      return ES_FIX;
    } else {
      return (View::modevent(d) == ME_INT_VAL)
        ? ES_SUBSUMED_NOFIX(a,home,c) : ES_NOFIX;
    }
  }

  template<class View, class Degree, class StateIdx>
  ExecStatus
  LayeredGraph<View,Degree,StateIdx>::propagate(Space& home,
                                                const ModEventDelta&) {
    // Forward pass
    for (int i=i_ch.fst(); i<=i_ch.lst(); i++) {
      bool i_mod = false;
      bool o_mod = false;
      unsigned int j=0;
      unsigned int k=0;
      unsigned int s=layers[i].size;
      do {
        for (Degree d=layers[i].support[j].n_edges; d--; )
          if (states[layers[i].support[j].edges[d].i_state].i_deg == 0) {
            // Adapt states
            o_mod |= ((--states[layers[i].support[j].edges[d].i_state].o_deg)
                      == 0);
            i_mod |= ((--states[layers[i].support[j].edges[d].o_state].i_deg)
                      == 0);
            // Remove edge
            layers[i].support[j].edges[d] = 
              layers[i].support[j].edges[--layers[i].support[j].n_edges];
          }
        // Check whether value is still supported
        if (layers[i].support[j].n_edges == 0) {
          layers[i].size--;
          GECODE_ME_CHECK(layers[i].x.nq(home,layers[i].support[j++].val));
        } else {
          layers[i].support[k++]=layers[i].support[j++];
        }
      } while (j<s);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
      if (i_mod && (i+1 < n))
        i_ch.add(i+1);
    }
    i_ch.reset();

    // Backward pass
    for (int i=o_ch.lst(); i>=o_ch.fst(); i--) {
      bool o_mod = false;
      unsigned int j=0;
      unsigned int k=0;
      unsigned int s=layers[i].size;
      do {
        for (Degree d=layers[i].support[j].n_edges; d--; )
          if (states[layers[i].support[j].edges[d].o_state].o_deg == 0) {
            // Adapt states
            o_mod |= ((--states[layers[i].support[j].edges[d].i_state].o_deg) 
                      == 0);
            --states[layers[i].support[j].edges[d].o_state].i_deg;
            // Remove edge
            layers[i].support[j].edges[d] = 
              layers[i].support[j].edges[--layers[i].support[j].n_edges];
          }
        // Check whether value is still supported
        if (layers[i].support[j].n_edges == 0) {
          layers[i].size--;
          GECODE_ME_CHECK(layers[i].x.nq(home,layers[i].support[j++].val));
        } else {
          layers[i].support[k++]=layers[i].support[j++];
        }
      } while (j<s);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
    }
    o_ch.reset();

    // Check subsumption
    if (c.empty()) {
      c.dispose(home);
      return ES_SUBSUMED(*this,sizeof(*this));
    }
    return ES_FIX;
  }


  template<class View, class Degree, class StateIdx>
  forceinline size_t
  LayeredGraph<View,Degree,StateIdx>::dispose(Space& home) {
    c.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Degree, class StateIdx>
  template<class Var>
  ExecStatus
  LayeredGraph<View,Degree,StateIdx>::post(Space& home, 
                                           const VarArgArray<Var>& x,
                                           const DFA& dfa) {
    if (x.size() == 0) {
      // Check whether the start state 0 is also a final state
      if ((dfa.final_fst() <= 0) && (dfa.final_lst() >= 0))
        return ES_OK;
      return ES_FAILED;
    }
    assert(x.size() > 0);
    for (int i=x.size(); i--; ) {
      DFA::Symbols s(dfa);
      VarViewTraits<Var>::View xi(x[i]);
      GECODE_ME_CHECK(xi.inter_v(home,s,false));
    }
    LayeredGraph<View,Degree,StateIdx>* p =
      new (home) LayeredGraph<View,Degree,StateIdx>(home,x,dfa);
    return p->initialize(home,x,dfa);
  }

  template<class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>
  ::LayeredGraph(Space& home, bool share,
                 LayeredGraph<View,Degree,StateIdx>& p)
    : Propagator(home,share,p), n(p.n), n_states(p.n_states),
      layers(home.alloc<Layer>(n+2)+1), states(NULL) {
    c.update(home,share,p.c);
    // The states are not copied but reconstructed when needed (advise)
    // Copy layers
    for (int i=n; i--; ) {
      layers[i].x.update(home,share,p.layers[i].x);
      assert(layers[i].x.size() == p.layers[i].size);
      layers[i].size = p.layers[i].size;
      layers[i].support = home.alloc<Support>(layers[i].size);
      for (unsigned int j=layers[i].size; j--; ) {
        layers[i].support[j].val = p.layers[i].support[j].val;
        layers[i].support[j].n_edges = p.layers[i].support[j].n_edges;
        assert(layers[i].support[j].n_edges > 0);
        layers[i].support[j].edges = 
          home.alloc<Edge>(layers[i].support[j].n_edges);
        for (Degree d=layers[i].support[j].n_edges; d--; )
          layers[i].support[j].edges[d] = p.layers[i].support[j].edges[d];
      }
    }
  }

  template<class View, class Degree, class StateIdx>
  PropCost
  LayeredGraph<View,Degree,StateIdx>::cost(const Space&,
                                           const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,n);
  }

  template<class View, class Degree, class StateIdx>
  Actor*
  LayeredGraph<View,Degree,StateIdx>::copy(Space& home, bool share) {
    // Eliminate an assigned prefix
    if (layers[0].size == 1) {
      // Skip all layers corresponding to assigned views
      StateIdx k = 1;
      while (layers[k].size == 1)
        k++;
      // There is only a single edge
      assert((layers[k-1].support[0].n_edges == 1) &&
             (states[layers[k-1].support[0].edges[0].o_state].i_deg == 1));
      // Eliminate assigned layers
      n -= k; layers += k;
      // Update advisor indices
      for (Advisors<Index> as(c); as(); ++as)
        as.advisor().i -= k;
      // Update states
      if (states != NULL)
        states += k*n_states;
      for (int i=n; i--; )
        for (unsigned int j=layers[i].size; j--; )
          for (Degree d=layers[i].support[j].n_edges; d--; ) {
            layers[i].support[j].edges[d].i_state -= k*n_states;
            layers[i].support[j].edges[d].o_state -= k*n_states;
          }
    }
    return new (home) LayeredGraph<View,Degree,StateIdx>(home,share,*this);
  }

  /// Select small types for the layered graph propagator
  template<class Var>
  forceinline ExecStatus
  post_lgp(Space& home, const VarArgArray<Var>& x, const DFA& dfa) {
    Gecode::Support::IntType t_state_idx =
      Gecode::Support::s_type((x.size()+2)*dfa.n_states());
    Gecode::Support::IntType t_degree =
      Gecode::Support::u_type(dfa.max_degree());
    switch (t_state_idx) {
    case Gecode::Support::IT_CHAR:
      switch (t_degree) {
      case Gecode::Support::IT_CHAR:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned char,signed char>
          ::post(home,x,dfa);
      case Gecode::Support::IT_SHRT:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned short int,signed char>
          ::post(home,x,dfa);
      case Gecode::Support::IT_INT:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned int,signed char>
          ::post(home,x,dfa);
      default: GECODE_NEVER;
      }
      break;
    case Gecode::Support::IT_SHRT:
      switch (t_degree) {
      case Gecode::Support::IT_CHAR:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned char,short int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_SHRT:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned short int,short int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_INT:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned int,short int>
          ::post(home,x,dfa);
      default: GECODE_NEVER;
      }
      break;
    case Gecode::Support::IT_INT:
      switch (t_degree) {
      case Gecode::Support::IT_CHAR:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned char,int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_SHRT:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned short int,int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_INT:
        return Extensional::LayeredGraph
          <VarViewTraits<Var>::View,unsigned int,int>
          ::post(home,x,dfa);
      default: GECODE_NEVER;
      }
      break;
    default: GECODE_NEVER;
    }
    return ES_OK;
  }

}}}

// STATISTICS: int-prop

