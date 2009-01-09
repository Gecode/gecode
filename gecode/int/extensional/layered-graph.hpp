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

  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::Edge::Edge(StateIdx i, StateIdx o, Edge* n)
    : i_state(i), o_state(o), next(n) {}
  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::Edge::operator delete(void* p, size_t s) {
    (void) p; (void) s;
  }
  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::Edge::operator delete(void* p, Space& home) {
    (void) p; (void) home;
  }
  template <class View, class Degree, class StateIdx>
  forceinline void*
  LayeredGraph<View,Degree,StateIdx>::Edge::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }



  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::LayerValues::LayerValues(void) {}
  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::LayerValues::LayerValues(const Layer& l)
    : s1(l.support), s2(l.support+l.size) {}
  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::LayerValues::init(const Layer& l) {
    s1=l.support; s2=l.support+l.size;
  }
  template <class View, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Degree,StateIdx>::LayerValues::operator ()(void) const {
    return s1<s2;
  }
  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::LayerValues::operator ++(void) {
    s1++;
  }
  template <class View, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Degree,StateIdx>::LayerValues::val(void) const {
    return s1->val;
  }


  /*
   * Index advisors
   *
   */
  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::Index::Index(Space& home, Propagator& p,
                                                   Council<Index>& c, 
                                                   StateIdx i0)
    : Advisor(home,p,c), i(i0) {}

  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::Index::Index(Space& home, bool share, 
                                                   Index& a)
    : Advisor(home,share,a), i(a.i) {}


  /*
   * Index ranges
   *
   */
  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::IndexRange::IndexRange(void) 
    : _fst(INT_MAX), _lst(INT_MIN) {}
  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::IndexRange::reset(void) {
    _fst=INT_MAX; _lst=INT_MIN;
  }
  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::IndexRange::add(int i) {
    _fst=std::min(_fst,i); _lst=std::max(_lst,i);
  }
  template <class View, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Degree,StateIdx>::IndexRange::fst(void) const {
    return _fst;
  }
  template <class View, class Degree, class StateIdx>
  forceinline int
  LayeredGraph<View,Degree,StateIdx>::IndexRange::lst(void) const {
    return _lst;
  }


  
  /*
   * The layered graph
   *
   */

  template <class View, class Degree, class StateIdx>
  forceinline bool
  LayeredGraph<View,Degree,StateIdx>::constructed(void) const {
    return layers != NULL;
  }

  template <class View, class Degree, class StateIdx>
  forceinline void
  LayeredGraph<View,Degree,StateIdx>::eliminate(void) {
    if (!constructed() || (layers[0].size > 1))
      return;
    assert(layers[0].size == 1);
    // Skip all layers corresponding to assigned views
    int i = 1;
    while (layers[i].size == 1)
      i++;
    // There is only a single edge
    Edge* e = layers[i-1].support[0].edges;
    assert((e->next == NULL) && (states[e->o_state].i_deg == 1));
    // New start state
    start = e->o_state % dfa.n_states();
    layers += i;
    x.drop_fst(i);
    for (Advisors<Index> as(c); as(); ++as)
      as.advisor().i -= i;
  }

  template <class View, class Degree, class StateIdx>
  forceinline ExecStatus
  LayeredGraph<View,Degree,StateIdx>::construct(Space& home) {
    int n = x.size();
    layers = home.alloc<Layer>(n+2)+1;
    
    unsigned int n_states = dfa.n_states();

    // Allocate memory
    states = home.alloc<State>((n+1)*n_states);

    // Initialize states (indegree and outdegree)
    for (int i = (n+1)*n_states; i--; )
      states[i].i_deg = states[i].o_deg = 0;

    // Mark initial state as being reachable
    states[start].i_deg = 1;

    // Mark final states as reachable as well
    for (int s = dfa.final_fst(); s < dfa.final_lst(); s++)
      states[n*n_states + s].o_deg = 1;

    // Forward pass: add transitions
    for (int i=0; i<n; i++) {
      layers[i].support = home.alloc<Support >(x[i].size());
      unsigned int j=0;
      // Enter links leaving reachable states (indegree != 0)
      for (ViewValues<View> nx(x[i]); nx(); ++nx) {
        Edge* e = NULL;
        for (DFA::Transitions t(dfa,nx.val()); t(); ++t)
          if (states[i*n_states + t.i_state()].i_deg != 0) {
            int i_s = i*n_states + t.i_state();
            states[i_s].o_deg++; 
            int o_s = (i+1)*n_states +  t.o_state();
            states[o_s].i_deg++;
            e = new (home) Edge(i_s, o_s, e);
          }
        // Found support for value
        if (e != NULL) {
          layers[i].support[j].val   = nx.val();
          layers[i].support[j].edges = e;
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
        Edge** p = &layers[i].support[j].edges;
        Edge*  e = *p;
        do {
          if (states[e->o_state].o_deg != 0) {
            // This state is still reachable, keep edge
            p = &e->next;
          } else {
            // Unreachable state, prune edge
            states[e->i_state].o_deg--; states[e->o_state].i_deg--;
            *p = e->next;
          }
          e = e->next;
        } while (e != NULL);
        // Write endmarker for edges
        *p = NULL;
        // Value has support, copy the support information
        if (layers[i].support[j].edges != NULL)
          layers[i].support[k++]=layers[i].support[j];
      }
      if ((layers[i].size = k) == 0)
        return ES_FAILED;
      LayerValues lv(layers[i]);
      GECODE_ME_CHECK(x[i].narrow_v(home,lv,false));
    }

    if (c.empty())
      return ES_SUBSUMED(*this,home);
    return ES_FIX;
  }

  template <class View, class Degree, class StateIdx>
  forceinline ExecStatus
  LayeredGraph<View,Degree,StateIdx>::prune(Space& home) {
    // Forward pass
    for (int i=i_ch.fst(); i<=i_ch.lst(); i++) {
      bool i_mod = false;
      bool o_mod = false;
      unsigned int j=0;
      unsigned int k=0;
      unsigned int s=layers[i].size;
      do {
        // Some edges might have lost support
        Edge** p = &layers[i].support[j].edges;
        Edge*  e = *p;
        do {
          if (states[e->i_state].i_deg == 0) {
            // Adapt states
            o_mod |= ((--states[e->i_state].o_deg) == 0);
            i_mod |= ((--states[e->o_state].i_deg) == 0);
            // Remove edge
            *p = e->next;
          } else {
            // Keep edge
            p = &e->next;
          }
          e = e->next;
        } while (e != NULL);
        // Write endmarker for edges
        *p=NULL;
        // Check whether value is still supported
        if (layers[i].support[j].edges == NULL) {
          layers[i].size--;
          GECODE_ME_CHECK(x[i].nq(home,layers[i].support[j++].val));
        } else {
          layers[i].support[k++]=layers[i].support[j++];
        }
      } while (j<s);
      assert(k > 0);
      // Update modification information
      if (o_mod && (i > 0))
        o_ch.add(i-1);
      if (i_mod && (i+1 < x.size()))
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
        Edge** p = &layers[i].support[j].edges;
        Edge*  e = *p;
        do {
          if (states[e->o_state].o_deg != 0) {
            // This state is still reachable, keep edge
            p = &e->next;
          } else {
            // Unreachable state, prune edge
            o_mod |= (--states[e->i_state].o_deg == 0);
            --states[e->o_state].i_deg;
            *p = e->next;
          }
          e = e->next;
        } while (e != NULL);
        // Write endmarker for edges
        *p = NULL;
        // Check whether value has still support
        if (layers[i].support[j].edges == NULL) {
          layers[i].size--;
          GECODE_ME_CHECK(x[i].nq(home,layers[i].support[j++].val));
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
    if (c.empty())
      return ES_SUBSUMED(*this,home);
    return ES_FIX;
  }

  template <class View, class Degree, class StateIdx>
  ExecStatus
  LayeredGraph<View,Degree,StateIdx>::advise(Space& home, 
                                             Advisor& _a, const Delta& d) {
    Index& a = static_cast<Index&>(_a);
    int i = a.i;
    bool i_mod = false;
    bool o_mod = false;
    bool is_fix = true;
    Layer& l = layers[i];

    if (!constructed())
      goto nofix;

    if (l.size == x[i].size())
      goto fix;

    if (View::modevent(d) == ME_INT_VAL) {
      int n = x[i].val();
      unsigned int j=0;
      for (; l.support[j].val < n; j++)
        // Supported value not any longer in view
        for (Edge* e=l.support[j].edges; e!=NULL; e=e->next) {
          // Adapt states
          o_mod |= ((--states[e->i_state].o_deg) == 0);
          i_mod |= ((--states[e->o_state].i_deg) == 0);
        }
      assert(l.support[j].val == n);
      l.support[0] = l.support[j++];
      unsigned int s=l.size;
      l.size = 1;
      for (; j<s; j++)
        for (Edge* e=l.support[j].edges; e!=NULL; e=e->next) {
          // Adapt states
          o_mod |= ((--states[e->i_state].o_deg) == 0);
          i_mod |= ((--states[e->o_state].i_deg) == 0);
        }
    } else if (x[i].any(d)) {
      unsigned int j=0;
      unsigned int k=0;
      unsigned int s=l.size;
      for (ViewRanges<View> rx(x[i]); rx() && (j<s);)
        if (l.support[j].val < rx.min()) {
          // Supported value not any longer in view
          for (Edge* e=l.support[j].edges; e!=NULL; e=e->next) {
            // Adapt states
            o_mod |= ((--states[e->i_state].o_deg) == 0);
            i_mod |= ((--states[e->o_state].i_deg) == 0);
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
        for (Edge* e=l.support[j].edges; e!=NULL; e=e->next) {
          // Adapt states
          o_mod |= ((--states[e->i_state].o_deg) == 0);
          i_mod |= ((--states[e->o_state].i_deg) == 0);
        }
    } else {
      int min = x[i].min(d);
      unsigned int j=0;
      // Skip values smaller than min (to keep)
      for (; l.support[j].val < min; j++) {}
      int max = x[i].max(d);
      unsigned int k=j;
      unsigned int s=l.size;
      // Remove pruned values
      for (; (j<s) && (l.support[j].val <= max); j++)
        for (Edge* e=l.support[j].edges; e!=NULL; e=e->next) {
          // Adapt states
          o_mod |= ((--states[e->i_state].o_deg) == 0);
          i_mod |= ((--states[e->o_state].i_deg) == 0);
        }
      // Keep remaining values
      while (j<s)
        l.support[k++]=l.support[j++];
      l.size =k;
      assert(k > 0);        
    }
    if (o_mod && (i > 0)) {
      o_ch.add(i-1); is_fix = false;
     }
    if (i_mod && (i+1 < x.size())) {
      i_ch.add(i+1); is_fix = false;
    }
    if (is_fix) {
    fix:
      if (View::modevent(d) == ME_INT_VAL) {
        a.dispose(home,c);
        return c.empty() ? ES_NOFIX : ES_FIX;
      }
      return ES_FIX;
    } else {
    nofix:
      return (View::modevent(d) == ME_INT_VAL) 
        ? ES_SUBSUMED_NOFIX(a,home,c) : ES_NOFIX;
    }
  }

  template <class View, class Degree, class StateIdx>
  ExecStatus
  LayeredGraph<View,Degree,StateIdx>::propagate(Space& home, 
                                                const ModEventDelta&) {
    ExecStatus es = constructed() ? prune(home) : construct(home);
    return es;
  }

  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>::LayeredGraph(Space& home, 
                                                   ViewArray<View>& x0, DFA& d)
    : Propagator(home), c(home), x(x0), dfa(d), start(0), layers(NULL) {
    assert(x.size() > 0);
    ModEvent me = ME_INT_BND;
    for (int i=x.size(); i--; )
      if (x[i].assigned())
        me = ME_INT_VAL;
      else
        x[i].subscribe(home, *new (home) Index(home,*this,c,i));
    View::schedule(home,*this,me);
    home.notice(*this,AP_DISPOSE);
  }

  template <class View, class Degree, class StateIdx>
  forceinline size_t
  LayeredGraph<View,Degree,StateIdx>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    c.dispose(home);
    dfa.~DFA();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class View, class Degree, class StateIdx>
  forceinline ExecStatus
  LayeredGraph<View,Degree,StateIdx>::post(Space& home, ViewArray<View>& x, 
                                           DFA& d) {
    if (x.size() == 0) {
      // Check whether the start state 0 is also a final state
      if ((d.final_fst() <= 0) && (d.final_lst() >= 0))
        return ES_OK;
      return ES_FAILED;
    }
    assert(x.size() > 0);
    for (int i=x.size(); i--; ) {
      DFA::Symbols s(d);
      GECODE_ME_CHECK(x[i].inter_v(home,s,false));
    }
    (void) new (home) LayeredGraph<View,Degree,StateIdx>(home,x,d);
    return ES_OK;
  }

  template <class View, class Degree, class StateIdx>
  forceinline
  LayeredGraph<View,Degree,StateIdx>
  ::LayeredGraph(Space& home, bool share, 
                 LayeredGraph<View,Degree,StateIdx>& p)
    : Propagator(home,share,p), layers(NULL) {
    assert(p.x.size() > 0);
    p.eliminate();
    c.update(home,share,p.c);
    x.update(home,share,p.x);
    dfa.update(home,share,p.dfa);
    start = p.start;
  }

  template <class View, class Degree, class StateIdx>
  PropCost
  LayeredGraph<View,Degree,StateIdx>::cost(const Space&, 
                                           const ModEventDelta&) const {
    return cost_hi(x.size(), PC_LINEAR_HI);
  }

  template <class View, class Degree, class StateIdx>
  Actor*
  LayeredGraph<View,Degree,StateIdx>::copy(Space& home, bool share) {
    return new (home) LayeredGraph<View,Degree,StateIdx>(home,share,*this);
  }

  /// Select small types for the layered graph propagator
  template <class View>
  forceinline ExecStatus
  post_lgp(Space& home, ViewArray<View>& x, DFA dfa) {
    Gecode::Support::IntType t_state_idx = 
      Gecode::Support::s_type(x.size()*dfa.n_states());
    Gecode::Support::IntType t_degree =
      Gecode::Support::u_type(dfa.max_degree());
    switch (t_state_idx) {
    case Gecode::Support::IT_CHAR: 
    case Gecode::Support::IT_SHRT:
      switch (t_degree) {
      case Gecode::Support::IT_CHAR:
        return Extensional::LayeredGraph<View,unsigned char,short int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_SHRT:
        return Extensional::LayeredGraph<View,unsigned short int,short int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_INT:
        return Extensional::LayeredGraph<View,unsigned int,short int>
          ::post(home,x,dfa);
      default: GECODE_NEVER;
      }
      break;
    case Gecode::Support::IT_INT:
      switch (t_degree) {
      case Gecode::Support::IT_CHAR:
        return Extensional::LayeredGraph<View,unsigned char,int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_SHRT:
        return Extensional::LayeredGraph<View,unsigned short int,int>
          ::post(home,x,dfa);
      case Gecode::Support::IT_INT:
        return Extensional::LayeredGraph<View,unsigned int,int>
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

