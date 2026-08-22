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

namespace Gecode {

  class OpenIntVarSequence::Sequence : public LocalObject {
  public:
    class Factory {
    public:
      Domain domain;
      Transition transition;

      Factory(Domain domain0, Transition transition0)
        : domain(domain0), transition(transition0) {}
    };

    IntVar length;
    IntVar materialized;
    IntVar* x;
    int n;
    int capacity;
    SharedData<Factory> factory;

    static int
    maximum(int max) {
      Int::Limits::check(max,"OpenIntVarSequence");
      if (max < 0)
        throw Int::VariableEmptyDomain("OpenIntVarSequence");
      return max;
    }

    static Domain
    valid(Domain d) {
      if (!d)
        throw InvalidFunction("OpenIntVarSequence");
      return d;
    }

    static Transition
    valid(Transition t) {
      if (!t)
        throw InvalidFunction("OpenIntVarSequence");
      return t;
    }

    Sequence(Home home, Domain d, int max)
      : LocalObject(home),
        length(home,0,maximum(max)),
        materialized(home,0,max),
        x(nullptr), n(0), capacity(0),
        factory(Factory(valid(d),Transition())) {}

    Sequence(Home home, Domain d, Transition t, int max)
      : LocalObject(home),
        length(home,0,maximum(max)),
        materialized(home,0,max),
        x(nullptr), n(0), capacity(0),
        factory(Factory(valid(d),valid(t))) {}

    Sequence(Space& home, Sequence& s)
      : LocalObject(home,s), x(nullptr), n(s.n), capacity(s.n),
        factory(s.factory) {
      length.update(home,s.length);
      materialized.update(home,s.materialized);
      if (capacity > 0) {
        x = heap.alloc<IntVar>(capacity);
        for (int i=0; i<n; i++)
          x[i].update(home,s.x[i]);
      }
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) Sequence(home,*this);
    }

    virtual size_t
    dispose(Space&) {
      if (capacity > 0)
        heap.free<IntVar>(x,capacity);
      factory.~SharedData<Factory>();
      return sizeof(*this);
    }

    void
    reserve(void) {
      if (n == capacity) {
        int next = (capacity < 4) ? 4 : capacity + capacity / 2;
        x = heap.realloc<IntVar>(x,capacity,next);
        capacity = next;
      }
    }

    void
    append(IntVar y) {
      x[n++] = y;
    }
  };

}

namespace Gecode { namespace Int {

  class OpenSequence : public Propagator {
  protected:
    OpenIntVarSequence sequence;
    IntView length;

    OpenSequence(Home home, OpenIntVarSequence sequence0)
      : Propagator(home), sequence(sequence0), length(sequence0.length()) {
      length.subscribe(home,*this,PC_INT_BND);
    }

    OpenSequence(Space& home, OpenSequence& p)
      : Propagator(home,p) {
      sequence.update(home,p.sequence);
      length.update(home,p.length);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSequence(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      const int n = length.min()-sequence.size();
      return (n <= 1) ? PropCost::unary(PropCost::LO) :
                        PropCost::linear(PropCost::LO,n);
    }

    virtual void
    reschedule(Space& home) {
      length.reschedule(home,*this,PC_INT_BND);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      sequence.materialize(home,length.min());
      if (home.failed())
        return ES_FAILED;
      if (length.assigned())
        return home.ES_SUBSUMED(*this);
      return ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      length.cancel(home,*this,PC_INT_BND);
      sequence.~OpenIntVarSequence();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence) {
      if (!sequence.length().assigned())
        (void) new (home) OpenSequence(home,sequence);
    }
  };

}}

namespace Gecode {

  OpenIntVarSequence::OpenIntVarSequence(void) {}

  OpenIntVarSequence::OpenIntVarSequence(Space& home, int max)
    : OpenIntVarSequence(home,
        IntSet(Int::Limits::min,Int::Limits::max),max) {}

  OpenIntVarSequence::OpenIntVarSequence(Space& home, const IntSet& d,
                                         int max)
    : OpenIntVarSequence(home,[d](int) { return d; },max) {}

  OpenIntVarSequence::OpenIntVarSequence(Space& home, const IntSet& d,
                                         Transition t, int max)
    : OpenIntVarSequence(home,[d](int) { return d; },t,max) {}

  OpenIntVarSequence::OpenIntVarSequence(Space& home, Domain d, int max)
    : LocalHandle(new (home) Sequence(home,d,max)) {
    Int::OpenSequence::post(home,*this);
  }

  OpenIntVarSequence::OpenIntVarSequence(Space& home, Domain d,
                                         Transition t, int max)
    : LocalHandle(new (home) Sequence(home,d,t,max)) {
    Int::OpenSequence::post(home,*this);
  }

  void
  OpenIntVarSequence::update(Space& home, OpenIntVarSequence& s) {
    LocalHandle::update(home,s);
  }

  int
  OpenIntVarSequence::size(void) const {
    return static_cast<Sequence*>(object())->n;
  }

  IntVar
  OpenIntVarSequence::operator [](int i) const {
    Sequence* s = static_cast<Sequence*>(object());
    assert((i >= 0) && (i < s->n));
    return s->x[i];
  }

  IntVar
  OpenIntVarSequence::length(void) const {
    return static_cast<Sequence*>(object())->length;
  }

  IntVar
  OpenIntVarSequence::materialized(void) const {
    return static_cast<Sequence*>(object())->materialized;
  }

  void
  OpenIntVarSequence::append(Space& home, IntVar y) {
    if (home.failed())
      return;
    Sequence* s = static_cast<Sequence*>(object());
    if (s->n == Int::Limits::max) {
      home.fail();
      return;
    }
    for (int i=0; i<s->n; i++)
      if (!y.assigned() && !s->x[i].assigned() &&
          (y.varimp() == s->x[i].varimp()))
        throw Int::ArgumentSame("OpenIntVarSequence::append");
    s->reserve();
    const int next = s->n + 1;
    Int::IntView l(s->length);
    Int::IntView m(s->materialized);
    if (me_failed(l.gq(home,next)) || me_failed(m.gq(home,next))) {
      home.fail();
      return;
    }
    s->append(y);
    const Transition& transition = s->factory().transition;
    if (transition) {
      GECODE_VALID_FUNCTION(transition);
      transition(home,*this,next-1);
    }
  }

  void
  OpenIntVarSequence::materialize(Space& home, int n) {
    if (home.failed())
      return;
    Int::Limits::nonnegative(n,"OpenIntVarSequence::materialize");
    Sequence* s = static_cast<Sequence*>(object());
    Int::IntView l(s->length);
    if (me_failed(l.gq(home,n))) {
      home.fail();
      return;
    }
    while (s->n < n) {
      const int i = s->n;
      GECODE_VALID_FUNCTION(s->factory().domain);
      IntVar y(home,s->factory().domain(i));
      append(home,y);
      if (home.failed())
        return;
    }
  }

  IntVar
  OpenIntVarSequence::get(Space& home, int i) {
    Int::Limits::nonnegative(i,"OpenIntVarSequence::get");
    if (home.failed())
      return IntVar();
    Sequence* s = static_cast<Sequence*>(object());
    if (i >= s->length.max())
      throw Int::OutOfLimits("OpenIntVarSequence::get");
    materialize(home,i+1);
    return static_cast<Sequence*>(object())->x[i];
  }

  void
  OpenIntVarSequence::close(Space& home) {
    if (home.failed())
      return;
    materialize(home,length().min());
    if (home.failed())
      return;
    Sequence* s = static_cast<Sequence*>(object());
    if (me_failed(Int::IntView(s->length).eq(home,s->n)))
      home.fail();
  }

}

namespace Gecode { namespace Int { namespace Extensional {

  class OpenRegular : public Propagator {
  protected:
    OpenIntVarSequence sequence;
    IntView length;
    IntView materialized;
    DFA dfa;
    unsigned char* start;
    int offset;
    int subscribed;

    OpenRegular(Home home, OpenIntVarSequence sequence0, DFA dfa0)
      : Propagator(home), sequence(sequence0),
        length(sequence0.length()),
        materialized(sequence0.materialized()), dfa(dfa0),
        start(static_cast<Space&>(home).alloc<unsigned char>(dfa.n_states())),
        offset(0), subscribed(sequence0.size()) {
      for (int i=0; i<dfa.n_states(); i++)
        start[i] = 0;
      start[0] = 1;
      length.subscribe(home,*this,PC_INT_VAL);
      materialized.subscribe(home,*this,PC_INT_BND);
      for (int i=0; i<subscribed; i++)
        if (!sequence[i].assigned())
          IntView(sequence[i]).subscribe(home,*this,PC_INT_DOM);
    }

    OpenRegular(Space& home, OpenRegular& p)
      : Propagator(home,p), dfa(p.dfa),
        start(home.alloc<unsigned char>(p.dfa.n_states())),
        offset(p.offset), subscribed(p.subscribed) {
      sequence.update(home,p.sequence);
      length.update(home,p.length);
      materialized.update(home,p.materialized);
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
      materialized.reschedule(home,*this,PC_INT_BND);
      for (int i=offset; i<subscribed; i++)
        if (!sequence[i].assigned())
          IntView(sequence[i]).reschedule(home,*this,PC_INT_DOM);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = sequence.size();
      assert(materialized.min() == size);
      assert(size <= length.max());

      for (int i=subscribed; i<size; i++) {
        IntView x(sequence[i]);
        DFA::Symbols symbols(dfa);
        GECODE_ME_CHECK(x.inter_v(home,symbols,false));
        if (!x.assigned())
          x.subscribe(home,*this,PC_INT_DOM,false);
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

      const unsigned long long cells =
        static_cast<unsigned long long>(n+1) *
        static_cast<unsigned long long>(k);
      if (cells >
          static_cast<unsigned long long>
          ((std::numeric_limits<size_t>::max)()))
        throw OutOfLimits("Int::extensional");

      Region r;
      unsigned char* backward =
        r.alloc<unsigned char>(static_cast<size_t>(cells));
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
        IntView x(sequence[offset+i]);
        for (ViewValues<IntView> v(x); v(); ++v)
          for (DFA::Transitions t(dfa,v.val()); t(); ++t)
            if (out[t.o_state()] != 0)
              in[t.i_state()] = 1;
      }

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
        IntView x(sequence[offset+i]);
        Region values;
        int* supported = values.alloc<int>(dfa.n_symbols());
        int n_supported = 0;
        unsigned char* out = backward + static_cast<size_t>(i+1)*k;
        for (ViewValues<IntView> v(x); v(); ++v) {
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
        for (ViewValues<IntView> value(x); value(); ++value)
          for (DFA::Transitions t(dfa,value.val()); t(); ++t)
            if ((reachable[t.i_state()] != 0) &&
                (out[t.o_state()] != 0))
              next[t.o_state()] = 1;
        unsigned char* swap = reachable;
        reachable = next;
        next = swap;
      }

      int consumed = 0;
      while ((consumed < n) && sequence[offset+consumed].assigned()) {
        unsigned char* live =
          backward + static_cast<size_t>(consumed+1)*k;
        unsigned char* next = r.alloc<unsigned char>(k);
        for (int s=0; s<k; s++)
          next[s] = 0;
        const int value = sequence[offset+consumed].val();
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
      length.cancel(home,*this,PC_INT_VAL);
      materialized.cancel(home,*this,PC_INT_BND);
      for (int i=offset; i<subscribed; i++)
        if (!sequence[i].assigned())
          IntView(sequence[i]).cancel(home,*this,PC_INT_DOM);
      dfa.~DFA();
      sequence.~OpenIntVarSequence();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static ExecStatus
    post(Home home, OpenIntVarSequence sequence, DFA dfa) {
      for (int i=0; i<sequence.size(); i++) {
        DFA::Symbols symbols(dfa);
        IntView x(sequence[i]);
        GECODE_ME_CHECK(x.inter_v(home,symbols,false));
      }
      (void) new (home) OpenRegular(home,sequence,dfa);
      return ES_OK;
    }
  };

}}}

namespace Gecode {

  void
  extensional(Home home, OpenIntVarSequence sequence, DFA dfa,
              IntPropLevel) {
    GECODE_POST;
    GECODE_ES_FAIL(Int::Extensional::OpenRegular::post(home,sequence,dfa));
  }

}

// STATISTICS: int-prop
