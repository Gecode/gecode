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

  OpenIntVarList::OpenIntVarList(void) {}

  OpenIntVarList::Factory
  OpenIntVarList::factory(Domain d) {
    if (!d)
      throw InvalidFunction("OpenIntVarList");
    return [d](Space& home, int i) { return IntVar(home,d(i)); };
  }

  OpenIntVarList::OpenIntVarList(Space& home, int max)
    : OpenIntVarList(home,
        IntSet(Int::Limits::min,Int::Limits::max),max) {}

  OpenIntVarList::OpenIntVarList(Space& home, const IntSet& d,
                                 int max)
    : OpenIntVarList(home,[d](int) { return d; },max) {}

  OpenIntVarList::OpenIntVarList(Space& home, const IntSet& d,
                                 Transition t, int max)
    : OpenIntVarList(home,[d](int) { return d; },t,max) {}

  OpenIntVarList::OpenIntVarList(Space& home, Domain d, int max)
    : OpenVarList<IntVar>(home,factory(d),max) {}

  OpenIntVarList::OpenIntVarList(Space& home, Domain d,
                                 Transition t, int max)
    : OpenVarList<IntVar>(home,factory(d),t,max) {}

  OpenBoolVarList::OpenBoolVarList(void) {}

  OpenBoolVarList::OpenBoolVarList(Space& home, int max)
    : OpenVarList<BoolVar>(home,
        [](Space& home, int) { return BoolVar(home,0,1); },max) {}

  OpenBoolVarList::OpenBoolVarList(Space& home, Transition t,
                                   int max)
    : OpenVarList<BoolVar>(home,
        [](Space& home, int) { return BoolVar(home,0,1); },t,max) {}

}

namespace Gecode { namespace Int {

  class OpenListPropagator : public Propagator {
  protected:
    OpenIntVarList list;
    IntView length;
    int posted;

    OpenListPropagator(Home home, OpenIntVarList list0)
      : Propagator(home), list(list0),
        length(list0.length()),
        posted(0) {
      length.subscribe(home,*this,PC_INT_BND);
      list.subscribe(home,*this);
    }

    OpenListPropagator(Space& home, OpenListPropagator& p)
      : Propagator(home,p), posted(p.posted) {
      list.update(home,p.list);
      length.update(home,p.length);
    }

    bool
    closed(void) const {
      return length.max() == list.prefix_size();
    }

    void
    dispose_base(Space& home) {
      length.cancel(home,*this,PC_INT_BND);
      list.cancel(home,*this);
      list.~OpenIntVarList();
      (void) Propagator::dispose(home);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,list.prefix_size()-posted);
    }

    virtual void
    reschedule(Space& home) {
      length.reschedule(home,*this,PC_INT_BND);
      list.reschedule(home,*this);
    }
  };


  class OpenDistinct : public OpenListPropagator {
  protected:
    OpenDistinct(Home home, OpenIntVarList list)
      : OpenListPropagator(home,list) {}

    OpenDistinct(Space& home, OpenDistinct& p)
      : OpenListPropagator(home,p) {}

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,list.prefix_size());
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenDistinct(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.prefix_size();
      for (int i=posted; i<size; i++)
        if (!list[i].assigned())
          IntView(list[i]).subscribe(home,*this,PC_INT_VAL,false);
      posted = size;

      bool assigned;
      do {
        assigned = false;
        Region region;
        int* values = region.alloc<int>(size);
        int n = 0;
        for (int i=0; i<size; i++)
          if (list[i].assigned())
            values[n++] = list[i].val();
        if (n > 1) {
          std::sort(values,values+n);
          for (int i=1; i<n; i++)
            if (values[i-1] == values[i])
              return ES_FAILED;
        }
        for (int i=0; i<size; i++)
          if (!list[i].assigned()) {
            IntView x(list[i]);
            for (int j=0; j<n; j++)
              GECODE_ME_CHECK(x.nq(home,values[j]));
            assigned |= x.assigned();
          }
      } while (assigned);

      bool complete = closed();
      for (int i=0; complete && (i<size); i++)
        complete = list[i].assigned();
      return complete ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator::reschedule(home);
      for (int i=0; i<posted; i++)
        if (!list[i].assigned())
          IntView(list[i]).reschedule(home,*this,PC_INT_VAL);
    }

    virtual size_t
    dispose(Space& home) {
      for (int i=0; i<posted; i++)
        if (!list[i].assigned())
          IntView(list[i]).cancel(home,*this,PC_INT_VAL);
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list) {
      (void) new (home) OpenDistinct(home,list);
    }
  };


  class OpenRel : public OpenListPropagator {
  protected:
    IntRelType irt;
    IntPropLevel ipl;

    OpenRel(Home home, OpenIntVarList list,
            IntRelType irt0, IntPropLevel ipl0)
      : OpenListPropagator(home,list), irt(irt0), ipl(ipl0) {}

    OpenRel(Space& home, OpenRel& p)
      : OpenListPropagator(home,p), irt(p.irt), ipl(p.ipl) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenRel(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.prefix_size();
      const int first = std::max(1,posted);
      for (int i=first; i<size; i++)
        Gecode::rel(home(*this),list[i-1],irt,list[i],ipl);
      posted = size;
      if (home.failed())
        return ES_FAILED;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list,
         IntRelType irt, IntPropLevel ipl) {
      (void) new (home) OpenRel(home,list,irt,ipl);
    }
  };


  class OpenSliding : public OpenListPropagator {
  protected:
    IntSet values;
    int width;
    int lower;
    int upper;
    IntPropLevel ipl;

    OpenSliding(Home home, OpenIntVarList list,
                const IntSet& values0, int width0,
                int lower0, int upper0, IntPropLevel ipl0)
      : OpenListPropagator(home,list), values(values0),
        width(width0), lower(lower0), upper(upper0), ipl(ipl0) {
      home.notice(*this,AP_DISPOSE);
    }

    OpenSliding(Space& home, OpenSliding& p)
      : OpenListPropagator(home,p), values(p.values),
        width(p.width), lower(p.lower), upper(p.upper), ipl(p.ipl) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSliding(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.prefix_size();
      for (int end=std::max(width,posted+1); end<=size; end++) {
        IntVarArgs window(width);
        for (int i=0; i<width; i++)
          window[i] = list[end-width+i];
        Gecode::sequence(home(*this),window,values,width,lower,upper,ipl);
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
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list, const IntSet& values,
         int width, int lower, int upper, IntPropLevel ipl) {
      (void) new (home)
        OpenSliding(home,list,values,width,lower,upper,ipl);
    }
  };


  class OpenSlidingSum : public OpenListPropagator {
  protected:
    int width;
    int lower;
    int upper;
    IntPropLevel ipl;

    OpenSlidingSum(Home home, OpenIntVarList list,
                   int width0, int lower0, int upper0, IntPropLevel ipl0)
      : OpenListPropagator(home,list),
        width(width0), lower(lower0), upper(upper0), ipl(ipl0) {}

    OpenSlidingSum(Space& home, OpenSlidingSum& p)
      : OpenListPropagator(home,p),
        width(p.width), lower(p.lower), upper(p.upper), ipl(p.ipl) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSlidingSum(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.prefix_size();
      for (int end=std::max(width,posted+1); end<=size; end++) {
        IntVarArgs window(width);
        for (int i=0; i<width; i++)
          window[i] = list[end-width+i];
        if (lower == upper) {
          Gecode::linear(home(*this),window,IRT_EQ,lower,ipl);
        } else {
          Gecode::linear(home(*this),window,IRT_GQ,lower,ipl);
          Gecode::linear(home(*this),window,IRT_LQ,upper,ipl);
        }
      }
      posted = size;
      if (home.failed())
        return ES_FAILED;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list,
         int width, int lower, int upper, IntPropLevel ipl) {
      (void) new (home)
        OpenSlidingSum(home,list,width,lower,upper,ipl);
    }
  };


  template<bool min>
  class OpenMinMax : public OpenListPropagator {
  protected:
    IntView result;
    IntPropLevel ipl;

    OpenMinMax(Home home, OpenIntVarList list,
               IntView result0, IntPropLevel ipl0)
      : OpenListPropagator(home,list),
        result(result0), ipl(ipl0) {}

    OpenMinMax(Space& home, OpenMinMax& p)
      : OpenListPropagator(home,p), ipl(p.ipl) {
      result.update(home,p.result);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenMinMax(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.prefix_size();
      for (int i=posted; i<size; i++)
        if (min)
          Gecode::rel(home(*this),list[i],IRT_GQ,
                      IntVar(result.varimp()),ipl);
        else
          Gecode::rel(home(*this),list[i],IRT_LQ,
                      IntVar(result.varimp()),ipl);
      posted = size;
      if (home.failed())
        return ES_FAILED;
      if (!closed())
        return ES_FIX;
      if (size == 0)
        return ES_FAILED;
      IntVarArgs variables(size);
      for (int i=0; i<size; i++)
        variables[i] = list[i];
      if (min)
        Gecode::min(home(*this),variables,IntVar(result.varimp()),ipl);
      else
        Gecode::max(home(*this),variables,IntVar(result.varimp()),ipl);
      if (home.failed())
        return ES_FAILED;
      return home.ES_SUBSUMED(*this);
    }

    virtual size_t
    dispose(Space& home) {
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list,
         IntView result, IntPropLevel ipl) {
      (void) new (home) OpenMinMax(home,list,result,ipl);
    }
  };


  class OpenPrecede : public OpenListPropagator {
  protected:
    int before;
    int after;
    BoolView seen;

    OpenPrecede(Home home, OpenIntVarList list,
                int before0, int after0)
      : OpenListPropagator(home,list),
        before(before0), after(after0) {}

    OpenPrecede(Space& home, OpenPrecede& p)
      : OpenListPropagator(home,p),
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
      const int size = list.prefix_size();
      for (int i=posted; i<size; i++) {
        BoolVar is_before(home,0,1);
        Gecode::rel(home(*this),list[i],IRT_EQ,before,
                    Reify(is_before,RM_EQV));
        if (i == 0) {
          Gecode::rel(home(*this),list[i],IRT_NQ,after);
          seen = BoolView(is_before);
        } else {
          BoolVar is_after(home,0,1);
          Gecode::rel(home(*this),list[i],IRT_EQ,after,
                      Reify(is_after,RM_EQV));
          Gecode::rel(home(*this),is_after,IRT_LQ,BoolVar(seen.varimp()));
          BoolVar next(home,0,1);
          Gecode::rel(home(*this),BoolVar(seen.varimp()),BOT_OR,is_before,next);
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
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list, int before, int after) {
      (void) new (home) OpenPrecede(home,list,before,after);
    }
  };


  /// Standard view/value selectors over the materialized prefix
  template<class View, int n>
  class OpenListBrancher : public Brancher {
  protected:
    typedef typename View::VarType Var;
    OpenVarList<Var> list;
    ViewSel<View>* vs[n];
    ValSelCommitBase<View,int>* vsc;
    BrancherFilter<View> filter;
    SharedData<VarValPrint<Var,int>> printer;
    OpenVarBranch order;
    IntValBranch::Select values;
    mutable int start;

    OpenListBrancher(Home home, OpenVarList<Var> list0,
                     ViewSel<View>* vs0[n],
                     ValSelCommitBase<View,int>* vsc0,
                     OpenVarBranch order0, IntValBranch::Select values0,
                     BranchFilter<Var> bf, VarValPrint<Var,int> vvp)
      : Brancher(home), list(list0), vsc(vsc0),
        filter(bf ? bf : [](const Space&, Var, int) { return true; }),
        printer(vvp), order(order0), values(values0), start(0) {
      for (int i=0; i<n; i++)
        vs[i] = vs0[i];
      home.notice(*this,AP_DISPOSE,true);
    }

    OpenListBrancher(Space& home, OpenListBrancher& b)
      : Brancher(home,b), vsc(nullptr),
        filter(b.filter), printer(b.printer), order(b.order),
        values(b.values), start(b.start) {
      list.update(home,b.list);
      int copied = 0;
      try {
        if (b.vsc)
          vsc = b.vsc->copy(home);
        for (; copied<n; copied++)
          vs[copied] = b.vs[copied]->copy(home);
      } catch (...) {
        for (int i=0; i<copied; i++)
          vs[i]->dispose(home);
        if (vsc)
          vsc->dispose(home);
        throw;
      }
    }

    /// Select from a temporary array, keeping positions stable across growth
    int
    position(Space& home) {
      Region r;
      ViewArray<View> x(r,list.prefix_size());
      for (int i=0; i<x.size(); i++)
        x[i] = View(list[i]);
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
          (list.length().max() > list.prefix_size()))
        return true;
      for (int i=start; i<list.prefix_size(); i++)
        if (!list[i].assigned() && filter(home,View(list[i]),i)) {
          start = i;
          return true;
        }
      return list.length().max() > list.prefix_size();
    }

    virtual const Choice*
    choice(Space& home) {
      if ((order == OVB_HORIZON_FIRST) &&
          (list.length().max() > list.prefix_size()))
        return new PosValChoice<int>(*this,2,Pos(-1),list.prefix_size());
      for (int i=start; i<list.prefix_size(); i++)
        if (!list[i].assigned() && filter(home,View(list[i]),i)) {
          start = i;
          const int p = position(home);
          View x(list[p]);
          return vsc ? new PosValChoice<int>(*this,2,Pos(p),vsc->val(home,x,p))
                     : value_choice(*this,p,x);
        }
      return new PosValChoice<int>(*this,2,Pos(-1),list.prefix_size());
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
        View x(list[p]);
        const ModEvent me = vsc
          ? vsc->commit(home,a,x,p,static_cast<const PosValChoice<int>&>(c).val())
          : x.eq(home,value(c,a));
        return me_failed(me) ? ES_FAILED : ES_OK;
      }
      const int size = static_cast<const PosValChoice<int>&>(c).val();
      if (a == 0)
        return me_failed(IntView(list.length()).eq(home,size))
          ? ES_FAILED : ES_OK;
      list.materialize(home,size+1);
      return home.failed() ? ES_FAILED : ES_OK;
    }

    virtual NGL*
    ngl(Space& home, const Choice& c, unsigned int a) const {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p < 0)
        return (a == 0) ? new (home) Branch::EqNGL<IntView>
          (home,IntView(list.length()),
           static_cast<const PosValChoice<int>&>(c).val()) : nullptr;
      // No-good extraction can run in an ancestor with a shorter prefix.
      if (p >= list.prefix_size())
        return nullptr;
      View x(list[p]);
      return vsc
        ? vsc->ngl(home,a,x,static_cast<const PosValChoice<int>&>(c).val())
        : new (home) Branch::EqNGL<View>(home,x,value(c,a));
    }

    virtual void
    print(const Space& home, const Choice& c, unsigned int a,
          std::ostream& o) const {
      const int p = static_cast<const PosChoice&>(c).pos().pos;
      if (p < 0) {
        o << "list length " << ((a == 0) ? "=" : ">") << " "
          << static_cast<const PosValChoice<int>&>(c).val();
      } else {
        View x(list[p]);
        const int v = vsc ? static_cast<const PosValChoice<int>&>(c).val()
                          : value(c,a);
        if (printer())
          printer()(home,*this,a,list[p],p,v,o);
        else if (vsc)
          vsc->print(home,a,x,p,v,o);
        else
          o << "list[" << p << "] = " << v;
      }
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenListBrancher(home,*this);
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
      list.~OpenVarList();
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }

    template<class VarBranch>
    static void
    post(Home home, OpenVarList<Var> list, VarBranch vars[n],
         ValSelCommitBase<View,int>* vsc,
         OpenVarBranch order, IntValBranch::Select values,
         BranchFilter<Var> bf, VarValPrint<Var,int> vvp) {
      ViewSel<View>* vs[n];
      for (int i=0; i<n; i++)
        vs[i] = Branch::viewsel(home,vars[i]);
      (void) new (home)
        OpenListBrancher(home,list,vs,vsc,order,values,bf,vvp);
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
  post_open_brancher(Home home, OpenVarList<typename View::VarType> list,
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
    typename ArrayTraits<VarArgArray<Var>>::ArgsType x(list.prefix_size());
    for (int i=0; i<x.size(); i++)
      x[i] = list[i];
    for (int i=0; i<n; i++) {
      check_open_selector(selectors[i],list.length().max() > list.prefix_size());
      selectors[i].expand(home,x);
    }
    const IntValBranch::Select values =
      static_cast<IntValBranch::Select>(vals.select());
    ValSelCommitBase<View,int>* vsc = open_valselcommit(home,vals);
    switch (n) {
    case 1:
      OpenListBrancher<View,1>::post
        (home,list,selectors,vsc,order,values,bf,vvp);
      break;
    case 2:
      OpenListBrancher<View,2>::post
        (home,list,selectors,vsc,order,values,bf,vvp);
      break;
    case 3:
      OpenListBrancher<View,3>::post
        (home,list,selectors,vsc,order,values,bf,vvp);
      break;
    case 4:
      OpenListBrancher<View,4>::post
        (home,list,selectors,vsc,order,values,bf,vvp);
      break;
    }
  }

}}

namespace Gecode {

  void
  distinct(Home home, OpenIntVarList list, IntPropLevel) {
    GECODE_POST;
    Int::OpenDistinct::post(home,list);
  }

  void
  rel(Home home, OpenIntVarList list,
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
    Int::OpenRel::post(home,list,irt,ipl);
  }

  void
  sequence(Home home, OpenIntVarList x, const IntSet& values,
           int width, int lower, int upper, IntPropLevel ipl) {
    Int::Limits::check(values.min(),"Int::list");
    Int::Limits::check(values.max(),"Int::list");
    Int::Limits::check(width,"Int::list");
    Int::Limits::check(lower,"Int::list");
    Int::Limits::check(upper,"Int::list");
    if (width < 1)
      throw Int::OutOfLimits("Int::list");
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
  slidingsum(Home home, OpenIntVarList list,
             int width, int lower, int upper, IntPropLevel ipl) {
    Int::Limits::check(width,"Int::slidingsum");
    Int::Limits::check(lower,"Int::slidingsum");
    Int::Limits::check(upper,"Int::slidingsum");
    if (width < 1)
      throw Int::OutOfLimits("Int::slidingsum");
    GECODE_POST;
    if (upper < lower) {
      rel(home,list.length(),IRT_LE,width);
      return;
    }
    Int::OpenSlidingSum::post(home,list,width,lower,upper,ipl);
  }

  void
  min(Home home, OpenIntVarList list,
      IntVar result, IntPropLevel ipl) {
    GECODE_POST;
    Int::OpenMinMax<true>::post(home,list,Int::IntView(result),ipl);
  }

  void
  max(Home home, OpenIntVarList list,
      IntVar result, IntPropLevel ipl) {
    GECODE_POST;
    Int::OpenMinMax<false>::post(home,list,Int::IntView(result),ipl);
  }

  void
  precede(Home home, OpenIntVarList list,
          int before, int after, IntPropLevel) {
    Int::Limits::check(before,"Int::precede");
    Int::Limits::check(after,"Int::precede");
    GECODE_POST;
    Int::OpenPrecede::post(home,list,before,after);
  }

  void
  precede(Home home, OpenIntVarList list,
          const IntArgs& values, IntPropLevel ipl) {
    if (values.size() < 2)
      return;
    for (int i=values.size(); i--; )
      Int::Limits::check(values[i],"Int::precede");
    GECODE_POST;
    for (int i=values.size()-1; i--; )
      Int::OpenPrecede::post(home,list,values[i],values[i+1]);
    (void) ipl;
  }

  void
  branch(Home home, OpenIntVarList list, OpenVarBranch order) {
    branch(home,list,INT_VAR_NONE(),INT_VAL_MIN(),order);
  }

  void
  branch(Home home, OpenBoolVarList list, OpenVarBranch order) {
    branch(home,list,BOOL_VAR_NONE(),BOOL_VAL_MIN(),order);
  }

  void
  branch(Home home, OpenIntVarList list,
         IntVarBranch vars, IntValBranch vals, OpenVarBranch order,
         IntBranchFilter bf, IntVarValPrint vvp) {
    branch(home,list,TieBreak<IntVarBranch>(vars),vals,order,bf,vvp);
  }

  void
  branch(Home home, OpenIntVarList list,
         TieBreak<IntVarBranch> vars, IntValBranch vals, OpenVarBranch order,
         IntBranchFilter bf, IntVarValPrint vvp) {
    GECODE_POST;
    Int::post_open_brancher<Int::IntView>(home,list,vars,vals,order,bf,vvp);
  }

  void
  branch(Home home, OpenBoolVarList list,
         BoolVarBranch vars, BoolValBranch vals, OpenVarBranch order,
         BoolBranchFilter bf, BoolVarValPrint vvp) {
    branch(home,list,TieBreak<BoolVarBranch>(vars),vals,order,bf,vvp);
  }

  void
  branch(Home home, OpenBoolVarList list,
         TieBreak<BoolVarBranch> vars, BoolValBranch vals, OpenVarBranch order,
         BoolBranchFilter bf, BoolVarValPrint vvp) {
    GECODE_POST;
    Int::post_open_brancher<Int::BoolView>(home,list,vars,vals,order,bf,vvp);
  }

}

// STATISTICS: int-prop
