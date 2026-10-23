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
 *
 */

#include <gecode/int/extensional.hh>

#include <limits>
#include <utility>

namespace Gecode { namespace Int { namespace Extensional {

  /// Shortest accepting suffix length from each state, or -1 if none exists
  static SharedArray<int>
  accepting_distances(const DFA& dfa) {
    const int k = dfa.n_states();
    SharedArray<int> distance(k);
    Region r;
    int* first = r.alloc<int>(k);
    int* source = r.alloc<int>(dfa.n_transitions());
    int* next = r.alloc<int>(dfa.n_transitions());
    int* queue = r.alloc<int>(k);
    for (int s=0; s<k; s++) {
      distance[s] = -1;
      first[s] = -1;
    }

    // Index incoming transitions for breadth-first traversal from final states.
    int e = 0;
    for (DFA::Transitions t(dfa); t(); ++t, e++) {
      source[e] = t.i_state();
      next[e] = first[t.o_state()];
      first[t.o_state()] = e;
    }
    int end = 0;
    for (int s=dfa.final_fst(); s<dfa.final_lst(); s++) {
      distance[s] = 0;
      queue[end++] = s;
    }
    // The default DFA accepts only the empty word.
    if ((dfa.final_fst() == 0) && (dfa.final_lst() == 0)) {
      distance[0] = 0;
      queue[end++] = 0;
    }
    for (int i=0; i<end; i++) {
      const int s = queue[i];
      for (int e=first[s]; e != -1; e=next[e]) {
        const int p = source[e];
        if (distance[p] == -1) {
          distance[p] = distance[s]+1;
          queue[end++] = p;
        }
      }
    }
    return distance;
  }

  /**
   * \brief Regular-language filtering over an open list's materialized prefix
   *
   * start contains the states reachable after the assigned positions before
   * offset. Forward and backward support are recomputed for the remaining
   * positions. The last layer must have an accepting continuation within the
   * remaining maximum length; closing the list allows only accepting states.
   * Assigned leading positions are then folded into start, preserving every
   * supported state when the automaton has nondeterministic transitions.
   */
  template<class View, PropCond pc>
  class OpenRegular : public Propagator {
  protected:
    OpenVarList<typename View::VarType> list;
    IntView length;
    DFA dfa;
    SharedArray<int> distance;
    unsigned char* start;
    int offset;
    int subscribed;

    OpenRegular(Home home,
                OpenVarList<typename View::VarType> list0, DFA dfa0)
      : Propagator(home), list(list0),
        length(list0.length()),
        dfa(dfa0), distance(accepting_distances(dfa0)),
        start(static_cast<Space&>(home).alloc<unsigned char>(dfa.n_states())),
        offset(0), subscribed(list0.known_length()) {
      home.notice(*this,AP_DISPOSE);
      for (int i=0; i<dfa.n_states(); i++)
        start[i] = 0;
      start[0] = 1;
      length.subscribe(home,*this,PC_INT_BND);
      list.subscribe(home,*this);
      for (int i=0; i<subscribed; i++)
        if (!list[i].assigned())
          View(list[i]).subscribe(home,*this,pc);
    }

    OpenRegular(Space& home, OpenRegular& p)
      : Propagator(home,p), dfa(p.dfa), distance(p.distance),
        start(home.alloc<unsigned char>(p.dfa.n_states())),
        offset(p.offset), subscribed(p.subscribed) {
      list.update(home,p.list);
      length.update(home,p.length);
      for (int i=0; i<dfa.n_states(); i++)
        start[i] = p.start[i];
    }

    bool
    has_accepting_suffix(const unsigned char* reachable, int remaining) const {
      for (int s=0; s<dfa.n_states(); s++)
        if ((reachable[s] != 0) &&
            (distance[s] >= 0) && (distance[s] <= remaining))
          return true;
      return false;
    }

  private:
    /// Mark states supporting the remaining domains through the last layer
    const unsigned char*
    compute_backward_support(Region& r, int remaining) const {
      const int n = list.known_length()-offset;
      const int k = dfa.n_states();
      const unsigned long long cells =
        static_cast<unsigned long long>(n+1) *
        static_cast<unsigned long long>(k);
      if (cells >
          static_cast<unsigned long long>
          ((std::numeric_limits<unsigned long>::max)()))
        throw OutOfLimits("Int::extensional");

      unsigned char* backward =
        r.alloc<unsigned char>(static_cast<unsigned long>(cells));
      for (size_t i=0; i<static_cast<size_t>(cells); i++)
        backward[i] = 0;

      unsigned char* last = backward + static_cast<size_t>(n)*k;
      for (int s=0; s<k; s++)
        if ((distance[s] >= 0) && (distance[s] <= remaining))
          last[s] = 1;

      for (int i=n; i--; ) {
        unsigned char* in = backward + static_cast<size_t>(i)*k;
        unsigned char* out = in+k;
        View x(list[offset+i]);
        for (ViewValues<View> v(x); v(); ++v)
          for (DFA::Transitions t(dfa,v.val()); t(); ++t)
            if (out[t.o_state()] != 0)
              in[t.i_state()] = 1;
      }
      return backward;
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenRegular(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,subscribed-offset);
    }

    virtual void
    reschedule(Space& home) {
      length.reschedule(home,*this,PC_INT_BND);
      list.reschedule(home,*this);
      for (int i=offset; i<subscribed; i++)
        if (!list[i].assigned())
          View(list[i]).reschedule(home,*this,pc);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      assert(size <= length.max());

      for (int i=subscribed; i<size; i++) {
        View x(list[i]);
        DFA::Symbols symbols(dfa);
        GECODE_ME_CHECK(x.inter_v(home,symbols,false));
        if (!x.assigned())
          x.subscribe(home,*this,pc,false);
      }
      subscribed = size;

      const int n = size-offset;
      const int k = dfa.n_states();
      const int remaining = length.max()-size;
      const bool closed = remaining == 0;
      if (n == 0) {
        if (!has_accepting_suffix(start,remaining))
          return ES_FAILED;
        return closed ? home.ES_SUBSUMED(*this) : ES_FIX;
      }

      Region r;
      const unsigned char* backward = compute_backward_support(r,remaining);

      bool any = false;
      for (int s=0; s<k; s++)
        any |= (start[s] != 0) && (backward[s] != 0);
      if (!any)
        return ES_FAILED;

      unsigned char* reachable = r.alloc<unsigned char>(k);
      unsigned char* next = r.alloc<unsigned char>(k);
      for (int s=0; s<k; s++)
        reachable[s] = start[s];
      for (int i=0; i<n; i++) {
        View x(list[offset+i]);
        Region values;
        int* supported = values.alloc<int>(dfa.n_symbols());
        int n_supported = 0;
        const unsigned char* out = backward + static_cast<size_t>(i+1)*k;
        for (ViewValues<View> v(x); v(); ++v) {
          bool support = false;
          for (DFA::Transitions t(dfa,v.val()); t() && !support; ++t)
            support = (reachable[t.i_state()] != 0) &&
                      (out[t.o_state()] != 0);
          if (support)
            supported[n_supported++] = v.val();
        }
        if (n_supported == 0)
          return ES_FAILED;
        Iter::Values::Array v(supported,n_supported);
        GECODE_ME_CHECK(x.narrow_v(home,v,false));

        for (int s=0; s<k; s++)
          next[s] = 0;
        for (ViewValues<View> value(x); value(); ++value)
          for (DFA::Transitions t(dfa,value.val()); t(); ++t)
            if ((reachable[t.i_state()] != 0) &&
                (out[t.o_state()] != 0))
              next[t.o_state()] = 1;
        std::swap(reachable,next);
      }

      // Pruning an item that aliases length can reduce the suffix budget.
      // Recompute support before consuming the prefix.
      if (length.max()-size < remaining)
        return ES_NOFIX;

      int consumed = 0;
      while ((consumed < n) && list[offset+consumed].assigned()) {
        const unsigned char* live =
          backward + static_cast<size_t>(consumed+1)*k;
        for (int s=0; s<k; s++)
          next[s] = 0;
        const int value = list[offset+consumed].val();
        for (DFA::Transitions t(dfa,value); t(); ++t)
          if ((start[t.i_state()] != 0) &&
              (live[t.o_state()] != 0))
            next[t.o_state()] = 1;
        for (int s=0; s<k; s++)
          start[s] = next[s];
        consumed++;
      }
      offset += consumed;

      if (closed && (offset == size))
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      length.cancel(home,*this,PC_INT_BND);
      list.cancel(home,*this);
      // On space deletion, the list buffer can already be disposed.
      if (!home.failed())
        for (int i=offset; i<subscribed; i++)
          if (!list[i].assigned())
            View(list[i]).cancel(home,*this,pc);
      dfa.~DFA();
      distance.~SharedArray();
      list.~OpenVarList();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static ExecStatus
    post(Home home, OpenVarList<typename View::VarType> list, DFA dfa) {
      for (int i=0; i<list.known_length(); i++) {
        DFA::Symbols symbols(dfa);
        View x(list[i]);
        GECODE_ME_CHECK(x.inter_v(home,symbols,false));
      }
      (void) new (home) OpenRegular(home,list,dfa);
      return ES_OK;
    }
  };

}}}

namespace Gecode {

  void
  extensional(Home home, OpenIntVarList list, DFA dfa,
              IntPropLevel) {
    GECODE_POST;
    GECODE_ES_FAIL((Int::Extensional::OpenRegular<Int::IntView,Int::PC_INT_DOM>
                    ::post(home,list,dfa)));
  }

  void
  extensional(Home home, OpenBoolVarList list, DFA dfa,
              IntPropLevel) {
    GECODE_POST;
    GECODE_ES_FAIL((Int::Extensional::OpenRegular<Int::BoolView,Int::PC_BOOL_VAL>
                    ::post(home,list,dfa)));
  }

}

// STATISTICS: int-prop
