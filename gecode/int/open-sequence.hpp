/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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

namespace Gecode {

  template<class Var>
  class OpenVarSequence<Var>::Sequence : public LocalObject {
  public:
    class Functions {
    public:
      Factory create;
      Transition transition;

      Functions(Factory create0, Transition transition0)
        : create(create0), transition(transition0) {}
    };

    IntVar length;
    IntVar materialized;
    Var* x;
    int n;
    int capacity;
    SharedData<Functions> functions;

    static int
    maximum(int max) {
      Int::Limits::check(max,"OpenVarSequence");
      if (max < 0)
        throw Int::VariableEmptyDomain("OpenVarSequence");
      return max;
    }

    static Factory
    valid(Factory create) {
      if (!create)
        throw InvalidFunction("OpenVarSequence");
      return create;
    }

    static Transition
    valid(Transition t) {
      if (!t)
        throw InvalidFunction("OpenVarSequence");
      return t;
    }

    Sequence(Home home, Factory create, int max)
      : LocalObject(home),
        length(home,0,maximum(max)),
        materialized(home,0,max),
        x(nullptr), n(0), capacity(0),
        functions(Functions(valid(create),Transition())) {
      home.notice(*this,AP_DISPOSE);
    }

    Sequence(Home home, Factory create, Transition t, int max)
      : LocalObject(home),
        length(home,0,maximum(max)),
        materialized(home,0,max),
        x(nullptr), n(0), capacity(0),
        functions(Functions(valid(create),valid(t))) {
      home.notice(*this,AP_DISPOSE);
    }

    Sequence(Space& home, Sequence& s)
      : LocalObject(home,s), x(nullptr), n(s.n), capacity(s.n),
        functions(s.functions) {
      length.update(home,s.length);
      materialized.update(home,s.materialized);
      if (capacity > 0) {
        x = heap.alloc<Var>(capacity);
        try {
          for (int i=0; i<n; i++)
            x[i].update(home,s.x[i]);
        } catch (...) {
          heap.free<Var>(x,capacity);
          throw;
        }
      }
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) Sequence(home,*this);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      if (capacity > 0)
        heap.free<Var>(x,capacity);
      functions.~SharedData<Functions>();
      return sizeof(*this);
    }

    void
    reserve(void) {
      if (n == capacity) {
        int next = (capacity < 4) ? 4 :
          capacity + std::min(capacity / 2,Int::Limits::max-capacity);
        x = heap.realloc<Var>(x,capacity,next);
        capacity = next;
      }
    }

    void
    append(Var y) {
      x[n++] = y;
    }
  };

}

namespace Gecode { namespace Int {

  /// Materialize positions required by the minimum eventual length
  template<class Var>
  class OpenMaterialize : public Propagator {
  protected:
    OpenVarSequence<Var> sequence;
    IntView length;

    OpenMaterialize(Home home, OpenVarSequence<Var> sequence0)
      : Propagator(home), sequence(sequence0), length(sequence0.length()) {
      length.subscribe(home,*this,PC_INT_BND);
    }

    OpenMaterialize(Space& home, OpenMaterialize& p)
      : Propagator(home,p) {
      sequence.update(home,p.sequence);
      length.update(home,p.length);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenMaterialize(home,*this);
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
      sequence.~OpenVarSequence();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenVarSequence<Var> sequence) {
      if (!sequence.length().assigned())
        (void) new (home) OpenMaterialize(home,sequence);
    }
  };

}}

namespace Gecode {

  template<class Var>
  OpenVarSequence<Var>::OpenVarSequence(void) {}

  template<class Var>
  OpenVarSequence<Var>::OpenVarSequence(Space& home, Factory create, int max)
    : LocalHandle(new (home) Sequence(home,create,max)) {
    Int::OpenMaterialize<Var>::post(home,*this);
  }

  template<class Var>
  OpenVarSequence<Var>::OpenVarSequence(Space& home, Factory create,
                                      Transition t, int max)
    : LocalHandle(new (home) Sequence(home,create,t,max)) {
    Int::OpenMaterialize<Var>::post(home,*this);
  }

  forceinline
  OpenIntVarSequence::OpenIntVarSequence(const OpenVarSequence<IntVar>& x)
    : OpenVarSequence<IntVar>(x) {}

  forceinline
  OpenBoolVarSequence::OpenBoolVarSequence(const OpenVarSequence<BoolVar>& x)
    : OpenVarSequence<BoolVar>(x) {}

  template<class Var>
  void
  OpenVarSequence<Var>::update(Space& home, OpenVarSequence<Var>& s) {
    LocalHandle::update(home,s);
  }

  template<class Var>
  int
  OpenVarSequence<Var>::size(void) const {
    return static_cast<Sequence*>(object())->n;
  }

  template<class Var>
  Var
  OpenVarSequence<Var>::operator [](int i) const {
    Sequence* s = static_cast<Sequence*>(object());
    assert((i >= 0) && (i < s->n));
    return s->x[i];
  }

  template<class Var>
  IntVar
  OpenVarSequence<Var>::length(void) const {
    return static_cast<Sequence*>(object())->length;
  }

  template<class Var>
  void
  OpenVarSequence<Var>::append(Space& home, Var y) {
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
        throw Int::ArgumentSame("OpenVarSequence::append");
    s->reserve();
    const int next = s->n + 1;
    Int::IntView l(s->length);
    Int::IntView m(s->materialized);
    if (me_failed(l.gq(home,next)) || me_failed(m.gq(home,next))) {
      home.fail();
      return;
    }
    s->append(y);
    const Transition& transition = s->functions().transition;
    if (transition) {
      GECODE_VALID_FUNCTION(transition);
      transition(home,*this,next-1);
    }
  }

  template<class Var>
  void
  OpenVarSequence<Var>::materialize(Space& home, int n) {
    if (home.failed())
      return;
    Int::Limits::nonnegative(n,"OpenVarSequence::materialize");
    Sequence* s = static_cast<Sequence*>(object());
    Int::IntView l(s->length);
    if (me_failed(l.gq(home,n))) {
      home.fail();
      return;
    }
    while (s->n < l.min()) {
      const int i = s->n;
      GECODE_VALID_FUNCTION(s->functions().create);
      Var y = s->functions().create(home,i);
      append(home,y);
      if (home.failed())
        return;
    }
  }

  template<class Var>
  Var
  OpenVarSequence<Var>::get(Space& home, int i) {
    Int::Limits::nonnegative(i,"OpenVarSequence::get");
    if (home.failed())
      return Var();
    Sequence* s = static_cast<Sequence*>(object());
    if (i >= s->length.max())
      throw Int::OutOfLimits("OpenVarSequence::get");
    materialize(home,i+1);
    if (home.failed())
      return Var();
    return static_cast<Sequence*>(object())->x[i];
  }

  template<class Var>
  void
  OpenVarSequence<Var>::close(Space& home) {
    if (home.failed())
      return;
    materialize(home,length().min());
    if (home.failed())
      return;
    Sequence* s = static_cast<Sequence*>(object());
    if (me_failed(Int::IntView(s->length).eq(home,s->n)))
      home.fail();
  }

  template<class Var>
  void
  OpenVarSequence<Var>::subscribe(Space& home, Propagator& p) {
    Sequence* s = static_cast<Sequence*>(object());
    Int::IntView(s->materialized).subscribe(home,p,Int::PC_INT_BND);
  }

  template<class Var>
  void
  OpenVarSequence<Var>::cancel(Space& home, Propagator& p) {
    Sequence* s = static_cast<Sequence*>(object());
    Int::IntView(s->materialized).cancel(home,p,Int::PC_INT_BND);
  }

  template<class Var>
  void
  OpenVarSequence<Var>::reschedule(Space& home, Propagator& p) {
    Sequence* s = static_cast<Sequence*>(object());
    Int::IntView(s->materialized).reschedule(home,p,Int::PC_INT_BND);
  }

}

// STATISTICS: int-var
