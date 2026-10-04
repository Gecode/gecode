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

#include <gecode/int.hh>

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
        factory(Factory(valid(d),Transition())) {
      home.notice(*this,AP_DISPOSE);
    }

    Sequence(Home home, Domain d, Transition t, int max)
      : LocalObject(home),
        length(home,0,maximum(max)),
        materialized(home,0,max),
        x(nullptr), n(0), capacity(0),
        factory(Factory(valid(d),valid(t))) {
      home.notice(*this,AP_DISPOSE);
    }

    Sequence(Space& home, Sequence& s)
      : LocalObject(home,s), x(nullptr), n(s.n), capacity(s.n),
        factory(s.factory) {
      length.update(home,s.length);
      materialized.update(home,s.materialized);
      if (capacity > 0) {
        x = heap.alloc<IntVar>(capacity);
        try {
          for (int i=0; i<n; i++)
            x[i].update(home,s.x[i]);
        } catch (...) {
          heap.free<IntVar>(x,capacity);
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

  /// Materialize positions required by the minimum eventual length
  class OpenMaterialize : public Propagator {
  protected:
    OpenIntVarSequence sequence;
    IntView length;

    OpenMaterialize(Home home, OpenIntVarSequence sequence0)
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
      sequence.~OpenIntVarSequence();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence) {
      if (!sequence.length().assigned())
        (void) new (home) OpenMaterialize(home,sequence);
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
    Int::OpenMaterialize::post(home,*this);
  }

  OpenIntVarSequence::OpenIntVarSequence(Space& home, Domain d,
                                         Transition t, int max)
    : LocalHandle(new (home) Sequence(home,d,t,max)) {
    Int::OpenMaterialize::post(home,*this);
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
    while (s->n < l.min()) {
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
    if (home.failed())
      return IntVar();
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

namespace Gecode { namespace Int {

  class OpenSequencePropagator : public Propagator {
  protected:
    OpenIntVarSequence sequence;
    IntView length;
    IntView materialized;
    int posted;

    OpenSequencePropagator(Home home, OpenIntVarSequence sequence0)
      : Propagator(home), sequence(sequence0),
        length(sequence0.length()),
        materialized(sequence0.materialized()), posted(0) {
      length.subscribe(home,*this,PC_INT_BND);
      materialized.subscribe(home,*this,PC_INT_BND);
    }

    OpenSequencePropagator(Space& home, OpenSequencePropagator& p)
      : Propagator(home,p), posted(p.posted) {
      sequence.update(home,p.sequence);
      length.update(home,p.length);
      materialized.update(home,p.materialized);
    }

    bool
    closed(void) const {
      return length.max() == sequence.size();
    }

    void
    dispose_base(Space& home) {
      length.cancel(home,*this,PC_INT_BND);
      materialized.cancel(home,*this,PC_INT_BND);
      sequence.~OpenIntVarSequence();
      (void) Propagator::dispose(home);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,sequence.size()-posted);
    }

    virtual void
    reschedule(Space& home) {
      length.reschedule(home,*this,PC_INT_BND);
      materialized.reschedule(home,*this,PC_INT_BND);
    }
  };


  class OpenDistinct : public OpenSequencePropagator {
  protected:
    OpenDistinct(Home home, OpenIntVarSequence sequence)
      : OpenSequencePropagator(home,sequence) {}

    OpenDistinct(Space& home, OpenDistinct& p)
      : OpenSequencePropagator(home,p) {}

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,sequence.size());
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenDistinct(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = sequence.size();
      for (int i=posted; i<size; i++)
        if (!sequence[i].assigned())
          IntView(sequence[i]).subscribe(home,*this,PC_INT_VAL,false);
      posted = size;

      bool assigned;
      do {
        assigned = false;
        Region region;
        int* values = region.alloc<int>(size);
        int n = 0;
        for (int i=0; i<size; i++)
          if (sequence[i].assigned())
            values[n++] = sequence[i].val();
        if (n > 1) {
          std::sort(values,values+n);
          for (int i=1; i<n; i++)
            if (values[i-1] == values[i])
              return ES_FAILED;
        }
        for (int i=0; i<size; i++)
          if (!sequence[i].assigned()) {
            IntView x(sequence[i]);
            for (int j=0; j<n; j++)
              GECODE_ME_CHECK(x.nq(home,values[j]));
            assigned |= x.assigned();
          }
      } while (assigned);

      bool complete = closed();
      for (int i=0; complete && (i<size); i++)
        complete = sequence[i].assigned();
      return complete ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenSequencePropagator::reschedule(home);
      for (int i=0; i<posted; i++)
        if (!sequence[i].assigned())
          IntView(sequence[i]).reschedule(home,*this,PC_INT_VAL);
    }

    virtual size_t
    dispose(Space& home) {
      for (int i=0; i<posted; i++)
        if (!sequence[i].assigned())
          IntView(sequence[i]).cancel(home,*this,PC_INT_VAL);
      OpenSequencePropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence) {
      (void) new (home) OpenDistinct(home,sequence);
    }
  };


  class OpenRel : public OpenSequencePropagator {
  protected:
    IntRelType irt;
    IntPropLevel ipl;

    OpenRel(Home home, OpenIntVarSequence sequence,
            IntRelType irt0, IntPropLevel ipl0)
      : OpenSequencePropagator(home,sequence), irt(irt0), ipl(ipl0) {}

    OpenRel(Space& home, OpenRel& p)
      : OpenSequencePropagator(home,p), irt(p.irt), ipl(p.ipl) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenRel(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = sequence.size();
      const int first = std::max(1,posted);
      for (int i=first; i<size; i++)
        Gecode::rel(home,sequence[i-1],irt,sequence[i],ipl);
      posted = size;
      if (home.failed())
        return ES_FAILED;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      OpenSequencePropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence,
         IntRelType irt, IntPropLevel ipl) {
      (void) new (home) OpenRel(home,sequence,irt,ipl);
    }
  };


  class OpenSliding : public OpenSequencePropagator {
  protected:
    IntSet values;
    int width;
    int lower;
    int upper;
    IntPropLevel ipl;

    OpenSliding(Home home, OpenIntVarSequence sequence,
                const IntSet& values0, int width0,
                int lower0, int upper0, IntPropLevel ipl0)
      : OpenSequencePropagator(home,sequence), values(values0),
        width(width0), lower(lower0), upper(upper0), ipl(ipl0) {
      home.notice(*this,AP_DISPOSE);
    }

    OpenSliding(Space& home, OpenSliding& p)
      : OpenSequencePropagator(home,p), values(p.values),
        width(p.width), lower(p.lower), upper(p.upper), ipl(p.ipl) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSliding(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = sequence.size();
      for (int end=std::max(width,posted+1); end<=size; end++) {
        IntVarArgs window(width);
        for (int i=0; i<width; i++)
          window[i] = sequence[end-width+i];
        Gecode::sequence(home,window,values,width,lower,upper,ipl);
      }
      posted = size;
      if (home.failed())
        return ES_FAILED;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      values.~IntSet();
      OpenSequencePropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence, const IntSet& values,
         int width, int lower, int upper, IntPropLevel ipl) {
      (void) new (home)
        OpenSliding(home,sequence,values,width,lower,upper,ipl);
    }
  };


  class OpenSlidingSum : public OpenSequencePropagator {
  protected:
    int width;
    int lower;
    int upper;
    IntPropLevel ipl;

    OpenSlidingSum(Home home, OpenIntVarSequence sequence,
                   int width0, int lower0, int upper0, IntPropLevel ipl0)
      : OpenSequencePropagator(home,sequence),
        width(width0), lower(lower0), upper(upper0), ipl(ipl0) {}

    OpenSlidingSum(Space& home, OpenSlidingSum& p)
      : OpenSequencePropagator(home,p),
        width(p.width), lower(p.lower), upper(p.upper), ipl(p.ipl) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSlidingSum(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = sequence.size();
      for (int end=std::max(width,posted+1); end<=size; end++) {
        IntVarArgs window(width);
        for (int i=0; i<width; i++)
          window[i] = sequence[end-width+i];
        if (lower == upper) {
          Gecode::linear(home,window,IRT_EQ,lower,ipl);
        } else {
          Gecode::linear(home,window,IRT_GQ,lower,ipl);
          Gecode::linear(home,window,IRT_LQ,upper,ipl);
        }
      }
      posted = size;
      if (home.failed())
        return ES_FAILED;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      OpenSequencePropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence,
         int width, int lower, int upper, IntPropLevel ipl) {
      (void) new (home)
        OpenSlidingSum(home,sequence,width,lower,upper,ipl);
    }
  };


  template<bool min>
  class OpenMinMax : public OpenSequencePropagator {
  protected:
    IntView result;
    IntPropLevel ipl;

    OpenMinMax(Home home, OpenIntVarSequence sequence,
               IntView result0, IntPropLevel ipl0)
      : OpenSequencePropagator(home,sequence),
        result(result0), ipl(ipl0) {}

    OpenMinMax(Space& home, OpenMinMax& p)
      : OpenSequencePropagator(home,p), ipl(p.ipl) {
      result.update(home,p.result);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenMinMax(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = sequence.size();
      for (int i=posted; i<size; i++)
        if (min)
          Gecode::rel(home,sequence[i],IRT_GQ,IntVar(result.varimp()),ipl);
        else
          Gecode::rel(home,sequence[i],IRT_LQ,IntVar(result.varimp()),ipl);
      posted = size;
      if (home.failed())
        return ES_FAILED;
      if (!closed())
        return ES_FIX;
      if (size == 0)
        return ES_FAILED;
      IntVarArgs variables(size);
      for (int i=0; i<size; i++)
        variables[i] = sequence[i];
      if (min)
        Gecode::min(home,variables,IntVar(result.varimp()),ipl);
      else
        Gecode::max(home,variables,IntVar(result.varimp()),ipl);
      if (home.failed())
        return ES_FAILED;
      return home.ES_SUBSUMED(*this);
    }

    virtual size_t
    dispose(Space& home) {
      OpenSequencePropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence,
         IntView result, IntPropLevel ipl) {
      (void) new (home) OpenMinMax(home,sequence,result,ipl);
    }
  };


  class OpenPrecede : public OpenSequencePropagator {
  protected:
    int before;
    int after;
    BoolView seen;

    OpenPrecede(Home home, OpenIntVarSequence sequence,
                int before0, int after0)
      : OpenSequencePropagator(home,sequence),
        before(before0), after(after0) {}

    OpenPrecede(Space& home, OpenPrecede& p)
      : OpenSequencePropagator(home,p),
        before(p.before), after(p.after) {
      if (posted > 0)
        seen.update(home,p.seen);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenPrecede(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      if ((posted > 0) && seen.one())
        return home.ES_SUBSUMED(*this);
      const int size = sequence.size();
      for (int i=posted; i<size; i++) {
        BoolVar is_before(home,0,1);
        Gecode::rel(home,sequence[i],IRT_EQ,before,
                    Reify(is_before,RM_EQV));
        if (i == 0) {
          Gecode::rel(home,sequence[i],IRT_NQ,after);
          seen = BoolView(is_before);
        } else {
          BoolVar is_after(home,0,1);
          Gecode::rel(home,sequence[i],IRT_EQ,after,
                      Reify(is_after,RM_EQV));
          Gecode::rel(home,is_after,IRT_LQ,BoolVar(seen.varimp()));
          BoolVar next(home,0,1);
          Gecode::rel(home,BoolVar(seen.varimp()),BOT_OR,is_before,next);
          seen = BoolView(next);
        }
      }
      posted = size;
      if (home.failed())
        return ES_FAILED;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      OpenSequencePropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence, int before, int after) {
      (void) new (home) OpenPrecede(home,sequence,before,after);
    }
  };


  template<bool horizon_first>
  class OpenSequenceBrancher : public Brancher {
  protected:
    class Description : public Choice {
    public:
      enum Kind {
        VALUE,
        HORIZON
      };

      const Kind kind;
      const int position;
      const int value;

      Description(const Brancher& b, Kind kind0, int position0, int value0)
        : Choice(b,2), kind(kind0),
          position(position0), value(value0) {}

      virtual void
      archive(Archive& e) const {
        Choice::archive(e);
        e << static_cast<unsigned int>(kind) << position << value;
      }
    };

    OpenIntVarSequence sequence;

    OpenSequenceBrancher(Home home, OpenIntVarSequence sequence0)
      : Brancher(home), sequence(sequence0) {}

    OpenSequenceBrancher(Space& home, OpenSequenceBrancher& b)
      : Brancher(home,b) {
      sequence.update(home,b.sequence);
    }

  public:
    virtual bool
    status(const Space&) const {
      if (horizon_first &&
          (sequence.length().max() > sequence.size()))
        return true;
      for (int i=0; i<sequence.size(); i++)
        if (!sequence[i].assigned())
          return true;
      return sequence.length().max() > sequence.size();
    }

    virtual const Choice*
    choice(Space&) {
      if (horizon_first &&
          (sequence.length().max() > sequence.size()))
        return new Description(*this,Description::HORIZON,
                               sequence.size(),0);
      for (int i=0; i<sequence.size(); i++)
        if (!sequence[i].assigned())
          return new Description(*this,Description::VALUE,
                                 i,sequence[i].min());
      assert(sequence.length().max() > sequence.size());
      return new Description(*this,Description::HORIZON,
                             sequence.size(),0);
    }

    virtual const Choice*
    choice(const Space&, Archive& e) {
      unsigned int kind;
      int position, value;
      e >> kind >> position >> value;
      return new Description(*this,
                             static_cast<typename Description::Kind>(kind),
                             position,value);
    }

    virtual ExecStatus
    commit(Space& home, const Choice& choice0, unsigned int alternative) {
      const Description& choice =
        static_cast<const Description&>(choice0);
      if (choice.kind == Description::VALUE) {
        IntView x(sequence[choice.position]);
        ModEvent me = (alternative == 0)
          ? x.eq(home,choice.value)
          : x.nq(home,choice.value);
        return me_failed(me) ? ES_FAILED : ES_OK;
      }
      if (alternative == 0) {
        IntView length(sequence.length());
        return me_failed(length.eq(home,choice.position))
          ? ES_FAILED : ES_OK;
      }
      sequence.materialize(home,choice.position+1);
      return home.failed() ? ES_FAILED : ES_OK;
    }

    virtual void
    print(const Space&, const Choice& choice0, unsigned int alternative,
          std::ostream& o) const {
      const Description& choice =
        static_cast<const Description&>(choice0);
      if (choice.kind == Description::VALUE) {
        o << "sequence[" << choice.position << "] "
          << ((alternative == 0) ? "=" : "!=") << " " << choice.value;
      } else if (alternative == 0) {
        o << "sequence length = " << choice.position;
      } else {
        o << "sequence length > " << choice.position;
      }
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSequenceBrancher<horizon_first>(home,*this);
    }

    virtual size_t
    dispose(Space& home) {
      sequence.~OpenIntVarSequence();
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarSequence sequence) {
      (void) new (home) OpenSequenceBrancher<horizon_first>(home,sequence);
    }
  };

}}

namespace Gecode {

  void
  distinct(Home home, OpenIntVarSequence sequence, IntPropLevel) {
    GECODE_POST;
    Int::OpenDistinct::post(home,sequence);
  }

  void
  rel(Home home, OpenIntVarSequence sequence,
      IntRelType irt, IntPropLevel ipl) {
    switch (irt) {
    case IRT_EQ:
    case IRT_NQ:
    case IRT_LQ:
    case IRT_LE:
    case IRT_GQ:
    case IRT_GR:
      break;
    default:
      throw Int::UnknownRelation("Int::rel");
    }
    GECODE_POST;
    Int::OpenRel::post(home,sequence,irt,ipl);
  }

  void
  sequence(Home home, OpenIntVarSequence x, const IntSet& values,
           int width, int lower, int upper, IntPropLevel ipl) {
    Int::Limits::check(values.min(),"Int::sequence");
    Int::Limits::check(values.max(),"Int::sequence");
    Int::Limits::check(width,"Int::sequence");
    Int::Limits::check(lower,"Int::sequence");
    Int::Limits::check(upper,"Int::sequence");
    if (width < 1)
      throw Int::OutOfLimits("Int::sequence");
    GECODE_POST;
    lower = std::max(0,lower);
    upper = std::min(width,upper);
    if (upper < lower) {
      rel(home,x.length(),IRT_LE,width);
      return;
    }
    if ((lower == 0) && (upper == width))
      return;
    Int::OpenSliding::post(home,x,values,width,lower,upper,ipl);
  }

  void
  slidingsum(Home home, OpenIntVarSequence sequence,
             int width, int lower, int upper, IntPropLevel ipl) {
    Int::Limits::check(width,"Int::slidingsum");
    Int::Limits::check(lower,"Int::slidingsum");
    Int::Limits::check(upper,"Int::slidingsum");
    if (width < 1)
      throw Int::OutOfLimits("Int::slidingsum");
    GECODE_POST;
    if (upper < lower) {
      rel(home,sequence.length(),IRT_LE,width);
      return;
    }
    Int::OpenSlidingSum::post(home,sequence,width,lower,upper,ipl);
  }

  void
  min(Home home, OpenIntVarSequence sequence,
      IntVar result, IntPropLevel ipl) {
    GECODE_POST;
    Int::OpenMinMax<true>::post(home,sequence,Int::IntView(result),ipl);
  }

  void
  max(Home home, OpenIntVarSequence sequence,
      IntVar result, IntPropLevel ipl) {
    GECODE_POST;
    Int::OpenMinMax<false>::post(home,sequence,Int::IntView(result),ipl);
  }

  void
  precede(Home home, OpenIntVarSequence sequence,
          int before, int after, IntPropLevel) {
    Int::Limits::check(before,"Int::precede");
    Int::Limits::check(after,"Int::precede");
    GECODE_POST;
    Int::OpenPrecede::post(home,sequence,before,after);
  }

  void
  precede(Home home, OpenIntVarSequence sequence,
          const IntArgs& values, IntPropLevel ipl) {
    if (values.size() < 2)
      return;
    for (int i=values.size(); i--; )
      Int::Limits::check(values[i],"Int::precede");
    GECODE_POST;
    for (int i=values.size()-1; i--; )
      Int::OpenPrecede::post(home,sequence,values[i],values[i+1]);
    (void) ipl;
  }

  void
  branch(Home home, OpenIntVarSequence sequence) {
    branch(home,sequence,OIB_HORIZON_FIRST);
  }

  void
  branch(Home home, OpenIntVarSequence sequence, OpenIntBranch order) {
    GECODE_POST;
    switch (order) {
    case OIB_HORIZON_FIRST:
      Int::OpenSequenceBrancher<true>::post(home,sequence);
      break;
    case OIB_VALUE_FIRST:
      Int::OpenSequenceBrancher<false>::post(home,sequence);
      break;
    default:
      throw Int::UnknownBranching("Int::branch");
    }
  }

}

// STATISTICS: int-prop
