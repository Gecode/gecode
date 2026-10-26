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
#include <gecode/int/arithmetic.hh>
#include <gecode/int/distinct.hh>
#include <gecode/int/linear.hh>
#include <gecode/int/rel.hh>
#include <gecode/int/sequence.hh>

#include <vector>

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

  /// Follow list growth and remember which positions have been incorporated
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
      return length.max() == list.known_length();
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
      return PropCost::linear(PropCost::LO,list.known_length()-posted);
    }

    virtual void
    reschedule(Space& home) {
      length.reschedule(home,*this,PC_INT_BND);
      list.reschedule(home,*this);
    }
  };


  class OpenDistinct : public OpenListPropagator {
  protected:
    /// Variables whose assigned values have not yet been propagated
    std::vector<IntView> active;
    /// Values already excluded from active variables, retained for growth
    IntSet assigned;

    OpenDistinct(Home home, OpenIntVarList list)
      : OpenListPropagator(home,list) {
      home.notice(*this,AP_DISPOSE);
    }

    OpenDistinct(Space& home, OpenDistinct& p)
      : OpenListPropagator(home,p), assigned(p.assigned) {
      active.reserve(p.active.size());
      for (IntView x : p.active) {
        IntView y;
        y.update(home,x);
        active.push_back(y);
      }
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,
        static_cast<int>(active.size())+list.known_length()-posted);
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenDistinct(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      for (int i=posted; i<size; i++) {
        IntView x(list[i]);
        IntSetRanges old(assigned);
        GECODE_ME_CHECK(x.minus_r(home,old,false));
        active.push_back(x);
        if (!x.assigned())
          x.subscribe(home,*this,PC_INT_VAL,false);
      }
      posted = size;

      Region region;
      int* values = region.alloc<int>(active.size());
      while (true) {
        int n = 0;
        for (int i=static_cast<int>(active.size()); i--; )
          if (active[i].assigned()) {
            values[n++] = active[i].val();
            active[i] = active.back();
            active.pop_back();
          }
        if (n == 0)
          break;

        IntSet delta(values,n);
        if (delta.size() != static_cast<unsigned int>(n))
          return ES_FAILED;
        IntSetRanges old(assigned);
        IntSetRanges added(delta);
        Iter::Ranges::Union<IntSetRanges,IntSetRanges> both(old,added);
        assigned = IntSet(both);
        for (IntView x : active) {
          IntSetRanges removed(delta);
          GECODE_ME_CHECK(x.minus_r(home,removed,false));
        }
      }

      if (closed()) {
        ViewArray<IntView> x(home,static_cast<int>(active.size()));
        for (int i=0; i<x.size(); i++)
          x[i] = active[i];
        GECODE_REWRITE(*this,Distinct::Val<IntView>::post(home(*this),x));
      }
      return ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator::reschedule(home);
      for (IntView x : active)
        if (!x.assigned())
          x.reschedule(home,*this,PC_INT_VAL);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      for (IntView x : active)
        if (!x.assigned())
          x.cancel(home,*this,PC_INT_VAL);
      active.~vector();
      assigned.~IntSet();
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
      const int size = list.known_length();
      const int first = std::max(1,posted);
      for (int i=first; i<size; i++) {
        IntView x0(list[i-1]);
        IntView x1(list[i]);
        switch (irt) {
        case IRT_EQ:
          if ((vbd(ipl) == IPL_DOM) || (vbd(ipl) == IPL_DEF)) {
            GECODE_ES_CHECK((Rel::EqDom<IntView,IntView>
                            ::post(home(*this),x0,x1)));
          } else {
            GECODE_ES_CHECK((Rel::EqBnd<IntView,IntView>
                            ::post(home(*this),x0,x1)));
          }
          break;
        case IRT_NQ:
          GECODE_ES_CHECK((Rel::Nq<IntView,IntView>
                          ::post(home(*this),x0,x1)));
          break;
        case IRT_GQ:
          std::swap(x0,x1); // Fall through
        case IRT_LQ:
          GECODE_ES_CHECK((Rel::Lq<IntView,IntView>
                          ::post(home(*this),x0,x1)));
          break;
        case IRT_GR:
          std::swap(x0,x1); // Fall through
        case IRT_LE:
          GECODE_ES_CHECK((Rel::Le<IntView,IntView>
                          ::post(home(*this),x0,x1)));
          break;
        default:
          GECODE_NEVER;
        }
      }
      posted = size;
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


  /// Identify the windows affected by a position's domain change
  class OpenWindowAdvisor : public Advisor {
  public:
    int index;

    OpenWindowAdvisor(Space& home, Propagator& p,
                      Council<OpenWindowAdvisor>& council, int index0)
      : Advisor(home,p,council), index(index0) {}

    OpenWindowAdvisor(Space& home, OpenWindowAdvisor& a)
      : Advisor(home,a), index(a.index) {}

    void
    dispose(Space& home, Council<OpenWindowAdvisor>& council) {
      Advisor::dispose(home,council);
    }
  };

  /// Subscribe once per position and revisit only affected complete windows
  class OpenWindowPropagator : public OpenListPropagator {
  protected:
    int width;
    /// Inclusive range of window starts needing propagation
    int first;
    int last;
    Council<OpenWindowAdvisor> council;

    OpenWindowPropagator(Home home, OpenIntVarList list, int width0)
      : OpenListPropagator(home,list), width(width0),
        first(Int::Limits::max), last(-1), council(home) {}

    OpenWindowPropagator(Space& home, OpenWindowPropagator& p)
      : OpenListPropagator(home,p), width(p.width),
        first(p.first), last(p.last), council(home) {
      council.update(home,p.council);
    }

    void
    changed(int index) {
      first = std::min(first,std::max(0,index-width+1));
      last = std::max(last,std::min(index,list.known_length()-width));
    }

    void
    appended(int size) {
      if ((size > posted) && (size >= width)) {
        first = std::min(first,std::max(0,posted-width+1));
        last = size-width;
      }
      posted = size;
    }

    void
    subscribe(Space& home, int index) {
      IntView x(list[index]);
      x.subscribe(home,*new (home)
                  OpenWindowAdvisor(home,*this,council,index));
    }

    void
    dispose_windows(Space& home) {
      if (!home.failed())
        for (Advisors<OpenWindowAdvisor> a(council); a(); ++a) {
          IntView x(list[a.advisor().index]);
          if (!x.assigned())
            x.cancel(home,a.advisor());
        }
      council.dispose(home);
      OpenListPropagator::dispose_base(home);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,list.known_length());
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator::reschedule(home);
      first = 0;
      last = list.known_length()-width;
      IntView::schedule(home,*this,ME_INT_DOM);
    }
  };


  class OpenSliding : public OpenWindowPropagator {
  protected:
    IntSet values;
    int lower;
    int upper;

    OpenSliding(Home home, OpenIntVarList list,
                const IntSet& values0, int width0,
                int lower0, int upper0)
      : OpenWindowPropagator(home,list,width0), values(values0),
        lower(lower0), upper(upper0) {
      home.notice(*this,AP_DISPOSE);
    }

    OpenSliding(Space& home, OpenSliding& p)
      : OpenWindowPropagator(home,p), values(p.values),
        lower(p.lower), upper(p.upper) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSliding(home,*this);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta&) {
      OpenWindowAdvisor& a = static_cast<OpenWindowAdvisor&>(advisor);
      IntView x(list[a.index]);
      if (Sequence::undecided(x,values))
        return ES_FIX;
      changed(a.index);
      if (!x.assigned())
        x.cancel(home,a);
      return home.ES_NOFIX_DISPOSE(council,a);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      for (int i=posted; i<size; i++)
        if (Sequence::undecided(IntView(list[i]),values))
          subscribe(home,i);
      appended(size);

      if (first <= last) {
        const int begin = first;
        const int end = last;
        first = Int::Limits::max;
        last = -1;
        Region region;
        const int n = end-begin+width;
        Sequence::TakesStatus* status =
          region.alloc<Sequence::TakesStatus>(n);
        for (int i=0; i<n; i++)
          status[i] = Sequence::takes(IntView(list[begin+i]),values);
        int must = 0;
        int may = 0;
        for (int i=0; i<width; i++) {
          must += status[i] == Sequence::TS_YES;
          may += status[i] != Sequence::TS_NO;
        }
        for (int start=0; start<=end-begin; start++) {
          if ((must > upper) || (may < lower))
            return ES_FAILED;
          if ((must == upper) || (may == lower))
            for (int i=start; i<start+width; i++)
              if (status[i] == Sequence::TS_MAYBE) {
                IntView x(list[begin+i]);
                if (must == upper)
                  GECODE_ME_CHECK(Sequence::exclude(home,x,values));
                else
                  GECODE_ME_CHECK(Sequence::include(home,x,values));
              }
          if (start < end-begin) {
            must -= status[start] == Sequence::TS_YES;
            may -= status[start] != Sequence::TS_NO;
            must += status[start+width] == Sequence::TS_YES;
            may += status[start+width] != Sequence::TS_NO;
          }
        }
      }
      if (first <= last)
        return ES_NOFIX;
      return closed() && ((size < width) || council.empty()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      values.~IntSet();
      dispose_windows(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list, const IntSet& values,
         int width, int lower, int upper, IntPropLevel) {
      (void) new (home)
        OpenSliding(home,list,values,width,lower,upper);
    }
  };


  class OpenSlidingSum : public OpenWindowPropagator {
  protected:
    int lower;
    int upper;
    IntPropLevel ipl;

    OpenSlidingSum(Home home, OpenIntVarList list,
                   int width0, int lower0, int upper0, IntPropLevel ipl0)
      : OpenWindowPropagator(home,list,width0),
        lower(lower0), upper(upper0), ipl(ipl0) {
      // Choose the default from the full window, before linear posting
      // eliminates assigned terms. Delayed posting then has the same
      // propagation strength as posting before those assignments.
      if (vbd(ipl) == IPL_DEF)
        ipl = static_cast<IntPropLevel>
          (ipl | ((width <= 2) ? IPL_DOM : IPL_BND));
    }

    OpenSlidingSum(Space& home, OpenSlidingSum& p)
      : OpenWindowPropagator(home,p),
        lower(p.lower), upper(p.upper), ipl(p.ipl) {}

    ExecStatus
    equal_windows(Space& home, int size) {
      if ((posted < width) && (size >= width)) {
        if (width == 1) {
          GECODE_ME_CHECK(IntView(list[0]).eq(home,lower));
        } else if ((width == 2) && (vbd(ipl) == IPL_DOM)) {
          IntView x(list[0]);
          IntView y(list[1]);
          // Clip with wide residuals before constructing y-lower: its
          // range must fit integer limits. Binary domain filtering then
          // uses range intersections, even for very large domains.
          const long long int sum = lower;
          GECODE_ME_CHECK(x.gq(home,sum-y.max()));
          GECODE_ME_CHECK(x.lq(home,sum-y.min()));
          GECODE_ME_CHECK(y.gq(home,sum-x.max()));
          GECODE_ME_CHECK(y.lq(home,sum-x.min()));
          GECODE_ES_CHECK((Rel::EqDom<MinusView,OffsetView>::post
                          (home(*this),MinusView(x),OffsetView(y,-lower))));
        } else {
          Region region;
          Linear::Term<IntView>* terms =
            region.alloc<Linear::Term<IntView>>(width);
          for (int i=0; i<width; i++) {
            terms[i].a = 1;
            terms[i].x = IntView(list[i]);
          }
          Linear::post(home(*this),terms,width,IRT_EQ,lower,ipl);
          if (home.failed())
            return ES_FAILED;
        }
      }
      // Equal consecutive window sums imply periodic values. The first
      // sum and these equalities retain the selected propagation level without
      // copying every window into a separate linear propagator.
      for (int i=std::max(width,posted); i<size; i++) {
        IntView x(list[i]);
        IntView representative(list[i%width]);
        if (vbd(ipl) == IPL_DOM) {
          GECODE_ES_CHECK((Rel::EqDom<IntView,IntView>::post
                          (home(*this),x,representative)));
        } else {
          GECODE_ES_CHECK((Rel::EqBnd<IntView,IntView>::post
                          (home(*this),x,representative)));
        }
      }
      posted = size;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSlidingSum(home,*this);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta& delta) {
      if (IntView::modevent(delta) == ME_INT_DOM)
        return ES_FIX;
      OpenWindowAdvisor& a = static_cast<OpenWindowAdvisor&>(advisor);
      changed(a.index);
      if (list[a.index].assigned())
        return home.ES_NOFIX_DISPOSE(council,a);
      return ES_NOFIX;
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if (lower == upper)
        return equal_windows(home,size);
      for (int i=posted; i<size; i++)
        if (!list[i].assigned())
          subscribe(home,i);
      appended(size);

      if (first <= last) {
        const int begin = first;
        const int end = last;
        first = Int::Limits::max;
        last = -1;
        Region region;
        const int n = end-begin+width;
        int* minimum = region.alloc<int>(n);
        int* maximum = region.alloc<int>(n);
        for (int i=0; i<n; i++) {
          minimum[i] = list[begin+i].min();
          maximum[i] = list[begin+i].max();
        }
        // Both width and variable bounds are limited to Int::Limits, so
        // sums and the residual bounds below fit in long long int.
        long long int min_sum = 0;
        long long int max_sum = 0;
        for (int i=0; i<width; i++) {
          min_sum += minimum[i];
          max_sum += maximum[i];
        }
        for (int start=0; start<=end-begin; start++) {
          if ((min_sum > upper) || (max_sum < lower))
            return ES_FAILED;
          for (int i=start; i<start+width; i++) {
            IntView x(list[begin+i]);
            GECODE_ME_CHECK(x.gq(home,
              static_cast<long long int>(lower)-max_sum+maximum[i]));
            GECODE_ME_CHECK(x.lq(home,
              static_cast<long long int>(upper)-min_sum+minimum[i]));
          }
          if (start < end-begin) {
            min_sum -= minimum[start];
            max_sum -= maximum[start];
            min_sum += minimum[start+width];
            max_sum += maximum[start+width];
          }
        }
      }
      if (first <= last)
        return ES_NOFIX;
      return closed() && ((size < width) || council.empty()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      dispose_windows(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list,
         int width, int lower, int upper, IntPropLevel ipl) {
      (void) new (home)
        OpenSlidingSum(home,list,width,lower,upper,ipl);
    }
  };


  /// Track maxima; negated views implement min(x) = -max(-x)
  template<class View>
  class OpenMinMax : public OpenListPropagator {
  protected:
    class Index : public Advisor {
    public:
      int i;

      Index(Space& home, Propagator& p, Council<Index>& c, int i0)
        : Advisor(home,p,c), i(i0) {}

      Index(Space& home, Index& a)
        : Advisor(home,a), i(a.i) {}
    };

    View result;
    IntPropLevel ipl;
    Council<Index> c;
    /// Maximum lower bound seen so far; bounds only increase
    int lower;
    /// Result upper bound last applied to all incorporated positions
    int upper;
    /// The result occurs in the list and supplies its own equality witness
    bool witness;

    OpenMinMax(Home home, OpenIntVarList list,
               View result0, IntPropLevel ipl0)
      : OpenListPropagator(home,list),
        result(result0), ipl(ipl0), c(home),
        lower(Limits::min), upper(result0.max()), witness(false) {
      result.subscribe(home,*this,PC_INT_BND);
    }

    OpenMinMax(Space& home, OpenMinMax& p)
      : OpenListPropagator(home,p), ipl(p.ipl), c(home),
        lower(p.lower), upper(p.upper), witness(p.witness) {
      result.update(home,p.result);
      c.update(home,p.c);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenMinMax(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      const int size = list.known_length();
      return PropCost::linear(PropCost::LO,
        ((closed() && !witness) || (result.max() < upper)) ?
        size : size-posted);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta& d) {
      if (View::modevent(d) == ME_INT_DOM)
        return ES_FIX;
      Index& a = static_cast<Index&>(advisor);
      View x = View(IntView(list[a.i]));
      lower = std::max(lower,x.min());
      if (x.assigned())
        a.dispose(home,c);
      return ((lower > result.min()) ||
              (witness && closed() && c.empty())) ? ES_NOFIX : ES_FIX;
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if (result.max() < upper) {
        for (int i=0; i<posted; i++) {
          View x = View(IntView(list[i]));
          GECODE_ME_CHECK(x.lq(home,result.max()));
        }
        upper = result.max();
      }
      for (int i=posted; i<size; i++) {
        View x = View(IntView(list[i]));
        if (x == result) {
          witness = true;
          continue;
        }
        GECODE_ME_CHECK(x.lq(home,result.max()));
        lower = std::max(lower,x.min());
        if (!x.assigned())
          x.subscribe(home,*new (home) Index(home,*this,c,i));
      }
      posted = size;
      GECODE_ME_CHECK(result.gq(home,lower));
      // Future positions can supply the extremum. Enforce an equality witness
      // only once the list is closed, using the ordinary min/max propagator.
      if (!closed())
        return ES_FIX;
      if (size == 0)
        return ES_FAILED;
      if (witness)
        return (result.assigned() || c.empty()) ?
          home.ES_SUBSUMED(*this) : ES_FIX;
      ViewArray<View> variables(home,size);
      for (int i=0; i<size; i++)
        variables[i] = View(IntView(list[i]));
      View y = result;
      if (vbd(ipl) == IPL_DOM)
        GECODE_REWRITE(*this,(Arithmetic::NaryMaxDom<View>
                             ::post(home(*this),variables,y)));
      GECODE_REWRITE(*this,(Arithmetic::NaryMaxBnd<View>
                           ::post(home(*this),variables,y)));
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator::reschedule(home);
      result.reschedule(home,*this,PC_INT_BND);
    }

    virtual size_t
    dispose(Space& home) {
      if (!home.failed())
        for (Advisors<Index> as(c); as(); ++as) {
          View x = View(IntView(list[as.advisor().i]));
          x.cancel(home,as.advisor());
        }
      c.dispose(home);
      result.cancel(home,*this,PC_INT_BND);
      OpenListPropagator::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenIntVarList list,
         View result, IntPropLevel ipl) {
      (void) new (home) OpenMinMax(home,list,result,ipl);
    }
  };


  class OpenPrecede : public OpenListPropagator {
  protected:
    /// Keep the view as well as its position for independent disposal
    class Index : public Advisor {
    public:
      IntView x;
      int i;

      Index(Space& home, Propagator& p, Council<Index>& c,
            IntView x0, int i0)
        : Advisor(home,p,c), x(x0), i(i0) {}

      Index(Space& home, Index& a)
        : Advisor(home,a), i(a.i) {
        x.update(home,a.x);
      }
    };

    Council<Index> council;
    int before;
    int after;
    /// First and second positions that can still take before
    int alpha;
    int beta;
    /// First position assigned after, or infinity while none is known
    int gamma;

    OpenPrecede(Home home, OpenIntVarList list,
                int before0, int after0)
      : OpenListPropagator(home,list), council(home),
        before(before0), after(after0), alpha(0), beta(0),
        gamma(Limits::max) {}

    OpenPrecede(Space& home, OpenPrecede& p)
      : OpenListPropagator(home,p), council(home),
        before(p.before), after(p.after),
        alpha(p.alpha), beta(p.beta), gamma(p.gamma) {
      council.update(home,p.council);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,list.known_length()-alpha);
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) OpenPrecede(home,*this);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& a0, const Delta&) {
      Index& a = static_cast<Index&>(a0);
      const int i = a.i;
      bool changed = false;
      if (a.x.assigned()) {
        if ((a.x.val() == after) && (i < gamma)) {
          gamma = i;
          changed = true;
        }
        changed |= (i == alpha) || (i == beta);
        a.dispose(home,council);
      } else if ((i < alpha) || (i > gamma)) {
        a.x.cancel(home,a);
        a.dispose(home,council);
      } else {
        changed = ((i == alpha) || (i == beta)) && !a.x.in(before);
      }
      return changed ? ES_NOFIX : ES_FIX;
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if (before == after) {
        for (int i=posted; i<size; i++)
          GECODE_ME_CHECK(IntView(list[i]).nq(home,before));
        posted = size;
        return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }

      for (int i=posted; i<size; i++) {
        IntView x(list[i]);
        if (x.assigned()) {
          if (x.val() == after)
            gamma = std::min(gamma,i);
        } else if (i <= gamma) {
          x.subscribe(home,*new (home) Index(home,*this,council,x,i));
        }
      }
      posted = size;

      while ((alpha < size) && !IntView(list[alpha]).in(before)) {
        GECODE_ME_CHECK(IntView(list[alpha]).nq(home,after));
        alpha++;
      }
      if (alpha == size) {
        beta = size;
        return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }
      IntView first(list[alpha]);
      GECODE_ME_CHECK(first.nq(home,after));
      if (first.assigned())
        return home.ES_SUBSUMED(*this);

      // An exhausted cursor resumes at the previous end after growth.
      beta = std::max(beta,alpha+1);
      while ((beta < size) && !IntView(list[beta]).in(before))
        beta++;
      if (beta > gamma) {
        GECODE_ME_CHECK(first.eq(home,before));
        return home.ES_SUBSUMED(*this);
      }
      return ES_FIX;
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator::reschedule(home);
      IntView::schedule(home,*this,ME_INT_DOM);
    }

    virtual size_t
    dispose(Space& home) {
      for (Advisors<Index> as(council); as(); ++as)
        as.advisor().x.cancel(home,as.advisor());
      council.dispose(home);
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
    IntValBranch::Select values;
    bool select_first;
    mutable int start;

    OpenListBrancher(Home home, OpenVarList<Var> list0,
                     ViewSel<View>* vs0[n],
                     ValSelCommitBase<View,int>* vsc0,
                     IntValBranch::Select values0,
                     bool select_first0,
                     BranchFilter<Var> bf, VarValPrint<Var,int> vvp)
      : Brancher(home), list(list0), vsc(vsc0),
        filter(bf ? bf : [](const Space&, Var, int) { return true; }),
        printer(vvp), values(values0),
        select_first(select_first0), start(0) {
      for (int i=0; i<n; i++)
        vs[i] = vs0[i];
      home.notice(*this,AP_DISPOSE,true);
    }

    OpenListBrancher(Space& home, OpenListBrancher& b)
      : Brancher(home,b), vsc(nullptr),
        filter(b.filter), printer(b.printer),
        values(b.values), select_first(b.select_first), start(b.start) {
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
      if (select_first)
        return start;
      Region r;
      ViewArray<View> x(r,list.known_length());
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
      for (int i=start; i<list.known_length(); i++)
        if (!list[i].assigned() && filter(home,View(list[i]),i)) {
          start = i;
          return true;
        }
      return list.length().max() > list.known_length();
    }

    virtual const Choice*
    choice(Space& home) {
      for (int i=start; i<list.known_length(); i++)
        if (!list[i].assigned() && filter(home,View(list[i]),i)) {
          start = i;
          const int p = position(home);
          View x(list[p]);
          return vsc ? new PosValChoice<int>(*this,2,Pos(p),vsc->val(home,x,p))
                     : value_choice(*this,p,x);
        }
      // Position -1 marks a length choice: close here or require another item.
      return new PosValChoice<int>(*this,2,Pos(-1),list.known_length());
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
        // Recomputation can replay choices before propagation grows the prefix.
        if (p >= list.known_length()) {
          list.materialize(home,p+1);
          if (home.failed())
            return ES_FAILED;
        }
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
      if (p >= list.known_length())
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
        const int v = vsc ? static_cast<const PosValChoice<int>&>(c).val()
                          : value(c,a);
        // Tracing can print replayed choices before commit grows the prefix.
        if (p >= list.known_length()) {
          o << "list[" << p << "] alternative " << a << " (value " << v << ")";
          return;
        }
        View x(list[p]);
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
         IntValBranch::Select values,
         BranchFilter<Var> bf, VarValPrint<Var,int> vvp) {
      ViewSel<View>* vs[n];
      for (int i=0; i<n; i++)
        vs[i] = Branch::viewsel(home,vars[i]);
      (void) new (home)
        OpenListBrancher(home,list,vs,vsc,values,
                         vars[0].select() == VarBranch::SEL_NONE,bf,vvp);
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
                     BranchFilter<typename View::VarType> bf,
                     VarValPrint<typename View::VarType,int> vvp) {
    VarBranch selectors[4] = {vars.a,vars.b,vars.c,vars.d};
    int n = 1;
    while ((n < 4) && (selectors[n-1].select() != VarBranch::SEL_NONE) &&
           (selectors[n-1].select() != VarBranch::SEL_RND) &&
           (selectors[n].select() != VarBranch::SEL_NONE))
      n++;
    typedef typename View::VarType Var;
    typename ArrayTraits<VarArgArray<Var>>::ArgsType x(list.known_length());
    for (int i=0; i<x.size(); i++)
      x[i] = list[i];
    for (int i=0; i<n; i++) {
      check_open_selector(selectors[i],list.length().max() > list.known_length());
      selectors[i].expand(home,x);
    }
    const IntValBranch::Select values =
      static_cast<IntValBranch::Select>(vals.select());
    ValSelCommitBase<View,int>* vsc = open_valselcommit(home,vals);
    switch (n) {
    case 1:
      OpenListBrancher<View,1>::post
        (home,list,selectors,vsc,values,bf,vvp);
      break;
    case 2:
      OpenListBrancher<View,2>::post
        (home,list,selectors,vsc,values,bf,vvp);
      break;
    case 3:
      OpenListBrancher<View,3>::post
        (home,list,selectors,vsc,values,bf,vvp);
      break;
    case 4:
      OpenListBrancher<View,4>::post
        (home,list,selectors,vsc,values,bf,vvp);
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
    Int::OpenMinMax<Int::MinusView>::post(home,list,
      Int::MinusView(Int::IntView(result)),ipl);
  }

  void
  max(Home home, OpenIntVarList list,
      IntVar result, IntPropLevel ipl) {
    GECODE_POST;
    Int::OpenMinMax<Int::IntView>::post(home,list,Int::IntView(result),ipl);
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
  branch(Home home, OpenIntVarList list) {
    branch(home,list,INT_VAR_NONE(),INT_VAL_MIN());
  }

  void
  branch(Home home, OpenBoolVarList list) {
    branch(home,list,BOOL_VAR_NONE(),BOOL_VAL_MIN());
  }

  void
  branch(Home home, OpenIntVarList list,
         IntVarBranch vars, IntValBranch vals,
         IntBranchFilter bf, IntVarValPrint vvp) {
    branch(home,list,TieBreak<IntVarBranch>(vars),vals,bf,vvp);
  }

  void
  branch(Home home, OpenIntVarList list,
         TieBreak<IntVarBranch> vars, IntValBranch vals,
         IntBranchFilter bf, IntVarValPrint vvp) {
    GECODE_POST;
    Int::post_open_brancher<Int::IntView>(home,list,vars,vals,bf,vvp);
  }

  void
  branch(Home home, OpenBoolVarList list,
         BoolVarBranch vars, BoolValBranch vals,
         BoolBranchFilter bf, BoolVarValPrint vvp) {
    branch(home,list,TieBreak<BoolVarBranch>(vars),vals,bf,vvp);
  }

  void
  branch(Home home, OpenBoolVarList list,
         TieBreak<BoolVarBranch> vars, BoolValBranch vals,
         BoolBranchFilter bf, BoolVarValPrint vvp) {
    GECODE_POST;
    Int::post_open_brancher<Int::BoolView>(home,list,vars,vals,bf,vvp);
  }

}

// STATISTICS: int-prop
