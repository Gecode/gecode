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

#include <gecode/int/branch.hh>

namespace Gecode {

  OpenIntVarSequence::OpenIntVarSequence(void) {}

  OpenIntVarSequence::Factory
  OpenIntVarSequence::factory(Domain d) {
    if (!d)
      throw InvalidFunction("OpenIntVarSequence");
    return [d](Space& home, int i) { return IntVar(home,d(i)); };
  }

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
    : OpenVarSequence<IntVar>(home,factory(d),max) {}

  OpenIntVarSequence::OpenIntVarSequence(Space& home, Domain d,
                                         Transition t, int max)
    : OpenVarSequence<IntVar>(home,factory(d),t,max) {}

  OpenBoolVarSequence::OpenBoolVarSequence(void) {}

  OpenBoolVarSequence::OpenBoolVarSequence(Space& home, int max)
    : OpenVarSequence<BoolVar>(home,
        [](Space& home, int) { return BoolVar(home,0,1); },max) {}

  OpenBoolVarSequence::OpenBoolVarSequence(Space& home, Transition t,
                                           int max)
    : OpenVarSequence<BoolVar>(home,
        [](Space& home, int) { return BoolVar(home,0,1); },t,max) {}

}

namespace Gecode { namespace Int {

  class OpenSequencePropagator : public Propagator {
  protected:
    OpenIntVarSequence sequence;
    IntView length;
    int posted;

    OpenSequencePropagator(Home home, OpenIntVarSequence sequence0)
      : Propagator(home), sequence(sequence0),
        length(sequence0.length()),
        posted(0) {
      length.subscribe(home,*this,PC_INT_BND);
      sequence.subscribe(home,*this);
    }

    OpenSequencePropagator(Space& home, OpenSequencePropagator& p)
      : Propagator(home,p), posted(p.posted) {
      sequence.update(home,p.sequence);
      length.update(home,p.length);
    }

    bool
    closed(void) const {
      return length.max() == sequence.size();
    }

    void
    dispose_base(Space& home) {
      length.cancel(home,*this,PC_INT_BND);
      sequence.cancel(home,*this);
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
      sequence.reschedule(home,*this);
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


  /// Standard view/value selectors over the materialized prefix
  template<class View, int n>
  class OpenSequenceBrancher : public Brancher {
  protected:
    typedef typename View::VarType Var;
    OpenVarSequence<Var> sequence;
    ViewSel<View>* vs[n];
    ValSelCommitBase<View,int>* vsc;
    BrancherFilter<View> filter;
    SharedData<VarValPrint<Var,int>> printer;
    OpenVarBranch order;
    IntValBranch::Select values;
    mutable int start;

    OpenSequenceBrancher(Home home, OpenVarSequence<Var> sequence0,
                         ViewSel<View>* vs0[n],
                         ValSelCommitBase<View,int>* vsc0,
                         OpenVarBranch order0, IntValBranch::Select values0,
                         BranchFilter<Var> bf, VarValPrint<Var,int> vvp)
      : Brancher(home), sequence(sequence0), vsc(vsc0),
        filter(bf ? bf : [](const Space&, Var, int) { return true; }),
        printer(vvp), order(order0), values(values0), start(0) {
      for (int i=0; i<n; i++)
        vs[i] = vs0[i];
      home.notice(*this,AP_DISPOSE,true);
    }

    OpenSequenceBrancher(Space& home, OpenSequenceBrancher& b)
      : Brancher(home,b), vsc(b.vsc ? b.vsc->copy(home) : nullptr),
        filter(b.filter), printer(b.printer), order(b.order),
        values(b.values), start(b.start) {
      sequence.update(home,b.sequence);
      for (int i=0; i<n; i++)
        vs[i] = b.vs[i]->copy(home);
    }

    /// Select from a temporary array, keeping positions stable across growth
    int
    position(Space& home) {
      Region r;
      ViewArray<View> x(r,sequence.size());
      for (int i=0; i<x.size(); i++)
        x[i] = View(sequence[i]);
      if (n == 1)
        return vs[0]->select(home,x,start,filter);
      int* ties = r.alloc<int>(x.size()-start);
      int n_ties;
      vs[0]->ties(home,x,start,ties,n_ties,filter);
      for (int i=1; (i<n-1) && (n_ties>1); i++)
        vs[i]->brk(home,x,ties,n_ties);
      return (n_ties > 1) ? vs[n-1]->select(home,x,ties,n_ties) : ties[0];
    }

    static const Choice*
    value_choice(const Brancher& b, int i, IntView x) {
      return new Branch::PosValuesChoice(b,Pos(i),x);
    }

    static const Choice*
    value_choice(const Brancher&, int, BoolView) {
      GECODE_NEVER;
      return nullptr;
    }

    int
    value(const Choice& c, unsigned int a) const {
      const Branch::PosValuesChoice& pvc =
        static_cast<const Branch::PosValuesChoice&>(c);
      return pvc.val(values == IntValBranch::SEL_VALUES_MIN
                     ? a : pvc.alternatives()-1-a);
    }

  public:
    virtual bool
    status(const Space& home) const {
      if ((order == OVB_HORIZON_FIRST) &&
          (sequence.length().max() > sequence.size()))
        return true;
      for (int i=start; i<sequence.size(); i++)
        if (!sequence[i].assigned() && filter(home,View(sequence[i]),i)) {
          start = i;
          return true;
        }
      return sequence.length().max() > sequence.size();
    }

    virtual const Choice*
    choice(Space& home) {
      if ((order == OVB_HORIZON_FIRST) &&
          (sequence.length().max() > sequence.size()))
        return new PosValChoice<int>(*this,2,Pos(-1),sequence.size());
      for (int i=start; i<sequence.size(); i++)
        if (!sequence[i].assigned() && filter(home,View(sequence[i]),i)) {
          start = i;
          const int p = position(home);
          View x(sequence[p]);
          return vsc ? new PosValChoice<int>(*this,2,Pos(p),vsc->val(home,x,p))
                     : value_choice(*this,p,x);
        }
      return new PosValChoice<int>(*this,2,Pos(-1),sequence.size());
    }

    virtual const Choice*
    choice(const Space&, Archive& e) {
      int p;
      e >> p;
      if ((p < 0) || vsc) {
        int v;
        e >> v;
        return new PosValChoice<int>(*this,2,Pos(p),v);
      }
      unsigned int a;
      e >> a;
      return new Branch::PosValuesChoice(*this,a,Pos(p),e);
    }

    virtual ExecStatus
    commit(Space& home, const Choice& c, unsigned int a) {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p >= 0) {
        View x(sequence[p]);
        const ModEvent me = vsc
          ? vsc->commit(home,a,x,p,static_cast<const PosValChoice<int>&>(c).val())
          : x.eq(home,value(c,a));
        return me_failed(me) ? ES_FAILED : ES_OK;
      }
      const int size = static_cast<const PosValChoice<int>&>(c).val();
      if (a == 0)
        return me_failed(IntView(sequence.length()).eq(home,size))
          ? ES_FAILED : ES_OK;
      sequence.materialize(home,size+1);
      return home.failed() ? ES_FAILED : ES_OK;
    }

    virtual NGL*
    ngl(Space& home, const Choice& c, unsigned int a) const {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p < 0)
        return (a == 0) ? new (home) Branch::EqNGL<IntView>
          (home,IntView(sequence.length()),
           static_cast<const PosValChoice<int>&>(c).val()) : nullptr;
      View x(sequence[p]);
      return vsc
        ? vsc->ngl(home,a,x,static_cast<const PosValChoice<int>&>(c).val())
        : new (home) Branch::EqNGL<View>(home,x,value(c,a));
    }

    virtual void
    print(const Space& home, const Choice& c, unsigned int a,
          std::ostream& o) const {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p < 0) {
        o << "sequence length " << ((a == 0) ? "=" : ">") << " "
          << static_cast<const PosValChoice<int>&>(c).val();
      } else {
        View x(sequence[p]);
        const int v = vsc ? static_cast<const PosValChoice<int>&>(c).val()
                          : value(c,a);
        if (printer())
          printer()(home,*this,a,sequence[p],p,v,o);
        else if (vsc)
          vsc->print(home,a,x,p,v,o);
        else
          o << "sequence[" << p << "] = " << v;
      }
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSequenceBrancher(home,*this);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE,true);
      for (int i=0; i<n; i++)
        vs[i]->dispose(home);
      if (vsc)
        vsc->dispose(home);
      filter.dispose(home);
      printer.~SharedData<VarValPrint<Var,int>>();
      sequence.~OpenVarSequence();
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }

    template<class VarBranch>
    static void
    post(Home home, OpenVarSequence<Var> sequence, VarBranch vars[n],
         ValSelCommitBase<View,int>* vsc,
         OpenVarBranch order, IntValBranch::Select values,
         BranchFilter<Var> bf, VarValPrint<Var,int> vvp) {
      ViewSel<View>* vs[n];
      for (int i=0; i<n; i++)
        vs[i] = Branch::viewsel(home,vars[i]);
      (void) new (home)
        OpenSequenceBrancher(home,sequence,vs,vsc,order,values,bf,vvp);
    }
  };

  /// Reject fixed-position statistics while new variables can still appear
  void
  check_open_selector(IntVarBranch vars, bool open) {
    switch (vars.select()) {
    case IntVarBranch::SEL_ACTION_MIN: case IntVarBranch::SEL_ACTION_MAX:
    case IntVarBranch::SEL_ACTION_SIZE_MIN: case IntVarBranch::SEL_ACTION_SIZE_MAX:
    case IntVarBranch::SEL_CHB_MIN: case IntVarBranch::SEL_CHB_MAX:
    case IntVarBranch::SEL_CHB_SIZE_MIN: case IntVarBranch::SEL_CHB_SIZE_MAX:
      if (open)
        throw UnknownBranching("Int::branch");
      break;
    default: break;
    }
  }

  void
  check_open_selector(BoolVarBranch vars, bool open) {
    switch (vars.select()) {
    case BoolVarBranch::SEL_ACTION_MIN: case BoolVarBranch::SEL_ACTION_MAX:
    case BoolVarBranch::SEL_CHB_MIN: case BoolVarBranch::SEL_CHB_MAX:
      if (open)
        throw UnknownBranching("Int::branch");
      break;
    default: break;
    }
  }

  ValSelCommitBase<IntView,int>*
  open_valselcommit(Home home, IntValBranch vals) {
    return ((vals.select() == IntValBranch::SEL_VALUES_MIN) ||
            (vals.select() == IntValBranch::SEL_VALUES_MAX))
      ? nullptr : Branch::valselcommit(home,vals);
  }

  ValSelCommitBase<BoolView,int>*
  open_valselcommit(Home home, BoolValBranch vals) {
    return Branch::valselcommit(home,vals);
  }

  template<class View, class VarBranch, class ValBranch>
  void
  post_open_brancher(Home home, OpenVarSequence<typename View::VarType> sequence,
                     TieBreak<VarBranch> vars, ValBranch vals,
                     OpenVarBranch order,
                     BranchFilter<typename View::VarType> bf,
                     VarValPrint<typename View::VarType,int> vvp) {
    if ((order != OVB_HORIZON_FIRST) && (order != OVB_VALUE_FIRST))
      throw UnknownBranching("Int::branch");
    VarBranch selectors[4] = {vars.a,vars.b,vars.c,vars.d};
    int n = 1;
    while ((n < 4) && (selectors[n-1].select() != VarBranch::SEL_NONE) &&
           (selectors[n-1].select() != VarBranch::SEL_RND) &&
           (selectors[n].select() != VarBranch::SEL_NONE))
      n++;
    typedef typename View::VarType Var;
    typename ArrayTraits<VarArgArray<Var>>::ArgsType x(sequence.size());
    for (int i=0; i<x.size(); i++)
      x[i] = sequence[i];
    for (int i=0; i<n; i++) {
      check_open_selector(selectors[i],sequence.length().max() > sequence.size());
      selectors[i].expand(home,x);
    }
    const IntValBranch::Select values =
      static_cast<IntValBranch::Select>(vals.select());
    ValSelCommitBase<View,int>* vsc = open_valselcommit(home,vals);
    switch (n) {
    case 1:
      OpenSequenceBrancher<View,1>::post
        (home,sequence,selectors,vsc,order,values,bf,vvp);
      break;
    case 2:
      OpenSequenceBrancher<View,2>::post
        (home,sequence,selectors,vsc,order,values,bf,vvp);
      break;
    case 3:
      OpenSequenceBrancher<View,3>::post
        (home,sequence,selectors,vsc,order,values,bf,vvp);
      break;
    case 4:
      OpenSequenceBrancher<View,4>::post
        (home,sequence,selectors,vsc,order,values,bf,vvp);
      break;
    }
  }

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
  branch(Home home, OpenIntVarSequence sequence, OpenVarBranch order) {
    branch(home,sequence,INT_VAR_NONE(),INT_VAL_MIN(),order);
  }

  void
  branch(Home home, OpenBoolVarSequence sequence, OpenVarBranch order) {
    branch(home,sequence,BOOL_VAR_NONE(),BOOL_VAL_MIN(),order);
  }

  void
  branch(Home home, OpenIntVarSequence sequence,
         IntVarBranch vars, IntValBranch vals, OpenVarBranch order,
         IntBranchFilter bf, IntVarValPrint vvp) {
    branch(home,sequence,TieBreak<IntVarBranch>(vars),vals,order,bf,vvp);
  }

  void
  branch(Home home, OpenIntVarSequence sequence,
         TieBreak<IntVarBranch> vars, IntValBranch vals, OpenVarBranch order,
         IntBranchFilter bf, IntVarValPrint vvp) {
    GECODE_POST;
    Int::post_open_brancher<Int::IntView>(home,sequence,vars,vals,order,bf,vvp);
  }

  void
  branch(Home home, OpenBoolVarSequence sequence,
         BoolVarBranch vars, BoolValBranch vals, OpenVarBranch order,
         BoolBranchFilter bf, BoolVarValPrint vvp) {
    branch(home,sequence,TieBreak<BoolVarBranch>(vars),vals,order,bf,vvp);
  }

  void
  branch(Home home, OpenBoolVarSequence sequence,
         TieBreak<BoolVarBranch> vars, BoolValBranch vals, OpenVarBranch order,
         BoolBranchFilter bf, BoolVarValPrint vvp) {
    GECODE_POST;
    Int::post_open_brancher<Int::BoolView>(home,sequence,vars,vals,order,bf,vvp);
  }

}

// STATISTICS: int-prop
