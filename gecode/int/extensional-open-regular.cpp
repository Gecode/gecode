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

namespace Gecode { namespace Int { namespace Extensional {

  template<class View, PropCond pc>
  class OpenRegular : public Propagator {
  protected:
    OpenVarList<typename View::VarType> list;
    IntView length;
    DFA dfa;
    unsigned char* start;
    int offset;
    int subscribed;

    OpenRegular(Home home,
                OpenVarList<typename View::VarType> list0, DFA dfa0)
      : Propagator(home), list(list0),
        length(list0.length()),
        dfa(dfa0),
        start(static_cast<Space&>(home).alloc<unsigned char>(dfa.n_states())),
        offset(0), subscribed(list0.prefix_size()) {
      home.notice(*this,AP_DISPOSE);
      for (int i=0; i<dfa.n_states(); i++)
        start[i] = 0;
      start[0] = 1;
      length.subscribe(home,*this,PC_INT_VAL);
      list.subscribe(home,*this);
      for (int i=0; i<subscribed; i++)
        if (!list[i].assigned())
          View(list[i]).subscribe(home,*this,pc);
    }

    OpenRegular(Space& home, OpenRegular& p)
      : Propagator(home,p), dfa(p.dfa),
        start(home.alloc<unsigned char>(p.dfa.n_states())),
        offset(p.offset), subscribed(p.subscribed) {
      list.update(home,p.list);
      length.update(home,p.length);
      for (int i=0; i<dfa.n_states(); i++)
        start[i] = p.start[i];
    }

    bool
    final(int state) const {
      if ((dfa.final_fst() == 0) && (dfa.final_lst() == 0))
        return state == 0;
      return (state >= dfa.final_fst()) && (state < dfa.final_lst());
    }

    bool
    viable(void) const {
      return (dfa.final_fst() < dfa.final_lst()) ||
             ((dfa.final_fst() == 0) && (dfa.final_lst() == 0));
    }

    bool
    tail(const unsigned char* reachable, bool closed) const {
      if (closed) {
        for (int i=0; i<dfa.n_states(); i++)
          if ((reachable[i] != 0) && final(i))
            return true;
      } else {
        if (!viable())
          return false;
        for (int i=0; i<dfa.n_states(); i++)
          if (reachable[i] != 0)
            return true;
      }
      return false;
    }

  private:
    /// Compute backward state support in region-owned storage
    const unsigned char*
    compute_backward_support(Region& r, bool closed) const {
      const int n = list.prefix_size()-offset;
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
      if (closed) {
        for (int s=0; s<k; s++)
          if (final(s))
            last[s] = 1;
      } else if (viable()) {
        for (int s=0; s<k; s++)
          last[s] = 1;
      }

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
      length.reschedule(home,*this,PC_INT_VAL);
      list.reschedule(home,*this);
      for (int i=offset; i<subscribed; i++)
        if (!list[i].assigned())
          View(list[i]).reschedule(home,*this,pc);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.prefix_size();
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
      const bool closed = length.max() == size;
      if (n == 0) {
        if (!tail(start,closed))
          return ES_FAILED;
        return closed ? home.ES_SUBSUMED(*this) : ES_FIX;
      }

      Region r;
      const unsigned char* backward = compute_backward_support(r,closed);

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
        unsigned char* swap = reachable;
        reachable = next;
        next = swap;
      }

      int consumed = 0;
      while ((consumed < n) && list[offset+consumed].assigned()) {
        const unsigned char* live =
          backward + static_cast<size_t>(consumed+1)*k;
        unsigned char* next = r.alloc<unsigned char>(k);
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
      length.cancel(home,*this,PC_INT_VAL);
      list.cancel(home,*this);
      // On space deletion, the list buffer can already be disposed.
      if (!home.failed())
        for (int i=offset; i<subscribed; i++)
          if (!list[i].assigned())
            View(list[i]).cancel(home,*this,pc);
      dfa.~DFA();
      list.~OpenVarList();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static ExecStatus
    post(Home home, OpenVarList<typename View::VarType> list, DFA dfa) {
      for (int i=0; i<list.prefix_size(); i++) {
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
