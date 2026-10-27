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

#ifndef GECODE_INT_SEQUENCE_OPEN_HPP
#define GECODE_INT_SEQUENCE_OPEN_HPP

#include <gecode/int/open-propagator.hpp>
#include <gecode/int/sequence.hh>
#include <gecode/int/linear.hh>
#include <gecode/int/rel.hh>

#include <algorithm>

namespace Gecode { namespace Int { namespace Sequence {

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
  template<class View>
  class OpenWindowPropagator : public OpenListPropagator<View> {
  protected:
    using OpenListPropagator<View>::list;
    using OpenListPropagator<View>::processed;

    int width;
    /// Inclusive range of window starts needing propagation
    int first;
    int last;
    Council<OpenWindowAdvisor> council;

    OpenWindowPropagator(Home home, OpenViewList<View> list, int width0)
      : OpenListPropagator<View>(home,list), width(width0),
        first(Int::Limits::max), last(-1), council(home) {}

    OpenWindowPropagator(Space& home, OpenWindowPropagator& p)
      : OpenListPropagator<View>(home,p), width(p.width),
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
      if ((size > processed) && (size >= width)) {
        first = std::min(first,std::max(0,processed-width+1));
        last = size-width;
      }
      processed = size;
    }

    void
    subscribe(Space& home, int index) {
      View x(list[index]);
      x.subscribe(home,*new (home)
                  OpenWindowAdvisor(home,*this,council,index));
    }

    void
    dispose_windows(Space& home) {
      if (!home.failed())
        for (Advisors<OpenWindowAdvisor> a(council); a(); ++a) {
          View x(list[a.advisor().index]);
          if (!x.assigned())
            x.cancel(home,a.advisor());
        }
      council.dispose(home);
      OpenListPropagator<View>::dispose_base(home);
    }

  public:
    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,list.known_length());
    }

    virtual void
    reschedule(Space& home) {
      OpenListPropagator<View>::reschedule(home);
      first = 0;
      last = list.known_length()-width;
      View::schedule(home,*this,ME_INT_DOM);
    }
  };


  /// Membership counts over complete windows of an open list
  template<class View>
  class Open : public OpenWindowPropagator<View> {
  protected:
    using OpenWindowPropagator<View>::list;
    using OpenWindowPropagator<View>::processed;
    using OpenWindowPropagator<View>::closed;
    using OpenWindowPropagator<View>::width;
    using OpenWindowPropagator<View>::first;
    using OpenWindowPropagator<View>::last;
    using OpenWindowPropagator<View>::council;
    using OpenWindowPropagator<View>::appended;
    using OpenWindowPropagator<View>::changed;
    using OpenWindowPropagator<View>::subscribe;
    using OpenWindowPropagator<View>::dispose_windows;

    IntSet values;
    int lower;
    int upper;

    Open(Home home, OpenViewList<View> list,
         const IntSet& values0, int width0,
         int lower0, int upper0)
      : OpenWindowPropagator<View>(home,list,width0), values(values0),
        lower(lower0), upper(upper0) {
      home.notice(*this,AP_DISPOSE);
    }

    Open(Space& home, Open& p)
      : OpenWindowPropagator<View>(home,p), values(p.values),
        lower(p.lower), upper(p.upper) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) Open(home,*this);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta&) {
      OpenWindowAdvisor& a = static_cast<OpenWindowAdvisor&>(advisor);
      View x(list[a.index]);
      if (undecided(x,values))
        return ES_FIX;
      changed(a.index);
      if (!x.assigned())
        x.cancel(home,a);
      return home.ES_NOFIX_DISPOSE(council,a);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      for (int i=processed; i<size; i++)
        if (undecided(list[i],values))
          subscribe(home,i);
      appended(size);

      if (first <= last) {
        const int begin = first;
        const int end = last;
        first = Int::Limits::max;
        last = -1;
        Region region;
        const int n = end-begin+width;
        TakesStatus* status = region.alloc<TakesStatus>(n);
        for (int i=0; i<n; i++)
          status[i] = takes(list[begin+i],values);
        int must = 0;
        int may = 0;
        for (int i=0; i<width; i++) {
          must += status[i] == TS_YES;
          may += status[i] != TS_NO;
        }
        // Track the furthest end of each kind of forcing window so overlapping
        // windows apply each membership update once.
        int include_end = 0;
        int exclude_end = 0;
        for (int i=0; i<n; i++) {
          if (i <= end-begin) {
            if ((must > upper) || (may < lower))
              return ES_FAILED;
            if (must == upper)
              exclude_end = i+width;
            if (may == lower)
              include_end = i+width;
            if (i < end-begin) {
              must -= status[i] == TS_YES;
              may -= status[i] != TS_NO;
              must += status[i+width] == TS_YES;
              may += status[i+width] != TS_NO;
            }
          }
          if (status[i] == TS_MAYBE) {
            if ((i < include_end) && (i < exclude_end))
              return ES_FAILED;
            View x(list[begin+i]);
            if (i < exclude_end)
              GECODE_ME_CHECK(exclude(home,x,values));
            else if (i < include_end)
              GECODE_ME_CHECK(include(home,x,values));
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
    post(Home home, OpenViewList<View> list, const IntSet& values,
         int width, int lower, int upper, IntPropLevel) {
      (void) new (home)
        Open(home,list,values,width,lower,upper);
    }
  };


  /// Equal sums via the first complete window and periodic equalities
  template<class View>
  class OpenSlidingSumEq : public OpenListPropagator<View> {
  protected:
    using OpenListPropagator<View>::list;
    using OpenListPropagator<View>::processed;
    using OpenListPropagator<View>::closed;

    int width;
    int sum;
    IntPropLevel ipl;

    OpenSlidingSumEq(Home home, OpenViewList<View> list,
                     int width0, int sum0, IntPropLevel ipl0)
      : OpenListPropagator<View>(home,list),
        width(width0), sum(sum0), ipl(ipl0) {
      // Choose the default from the full window, before linear posting
      // eliminates assigned terms. Delayed posting then has the same
      // propagation strength as posting before those assignments.
      if (vbd(ipl) == IPL_DEF)
        ipl = static_cast<IntPropLevel>
          (ipl | ((width <= 2) ? IPL_DOM : IPL_BND));
    }

    OpenSlidingSumEq(Space& home, OpenSlidingSumEq& p)
      : OpenListPropagator<View>(home,p),
        width(p.width), sum(p.sum), ipl(p.ipl) {}

    static void
    linear_term(Linear::Term<IntView>& term, IntView x) {
      term.a = 1;
      term.x = x;
    }

    static void
    linear_term(Linear::Term<IntView>& term, MinusView x) {
      term.a = -1;
      term.x = x.base();
    }

    static ExecStatus
    binary_sum(Home home, IntView x, IntView y, int sum) {
      return Rel::EqDom<MinusView,OffsetView>::post
        (home,MinusView(x),OffsetView(y,-sum));
    }

    static ExecStatus
    binary_sum(Home home, MinusView x, MinusView y, int sum) {
      return binary_sum(home,x.base(),y.base(),-sum);
    }

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSlidingSumEq(home,*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      const int size = list.known_length();
      if ((processed < width) && (size >= width)) {
        if (width == 1) {
          GECODE_ME_CHECK(list[0].eq(home,sum));
        } else if ((width == 2) && (vbd(ipl) == IPL_DOM)) {
          View x(list[0]);
          View y(list[1]);
          // Clip with wide residuals before constructing y-sum: its
          // range must fit integer limits. Binary domain filtering then
          // uses range intersections, even for very large domains.
          const long long int target = sum;
          GECODE_ME_CHECK(x.gq(home,target-y.max()));
          GECODE_ME_CHECK(x.lq(home,target-y.min()));
          GECODE_ME_CHECK(y.gq(home,target-x.max()));
          GECODE_ME_CHECK(y.lq(home,target-x.min()));
          GECODE_ES_CHECK(binary_sum(home(*this),x,y,sum));
        } else {
          Region region;
          Linear::Term<IntView>* terms =
            region.alloc<Linear::Term<IntView>>(width);
          for (int i=0; i<width; i++)
            linear_term(terms[i],list[i]);
          Linear::post(home(*this),terms,width,IRT_EQ,sum,ipl);
          if (home.failed())
            return ES_FAILED;
        }
      }
      // Equal consecutive window sums imply periodic values. One window sum
      // and these equalities provide the selected propagation level.
      for (int i=std::max(width,processed); i<size; i++) {
        View x(list[i]);
        View representative(list[i%width]);
        if (vbd(ipl) == IPL_DOM) {
          GECODE_ES_CHECK((Rel::EqDom<View,View>::post
                          (home(*this),x,representative)));
        } else {
          GECODE_ES_CHECK((Rel::EqBnd<View,View>::post
                          (home(*this),x,representative)));
        }
      }
      processed = size;
      return closed() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    virtual size_t
    dispose(Space& home) {
      OpenListPropagator<View>::dispose_base(home);
      return sizeof(*this);
    }

    static void
    post(Home home, OpenViewList<View> list,
         int width, int sum, IntPropLevel ipl) {
      (void) new (home) OpenSlidingSumEq(home,list,width,sum,ipl);
    }
  };


  /// Bounds filtering for ranged sums over complete windows
  template<class View>
  class OpenSlidingSumBnd : public OpenWindowPropagator<View> {
  protected:
    using OpenWindowPropagator<View>::list;
    using OpenWindowPropagator<View>::processed;
    using OpenWindowPropagator<View>::closed;
    using OpenWindowPropagator<View>::width;
    using OpenWindowPropagator<View>::first;
    using OpenWindowPropagator<View>::last;
    using OpenWindowPropagator<View>::council;
    using OpenWindowPropagator<View>::changed;
    using OpenWindowPropagator<View>::appended;
    using OpenWindowPropagator<View>::subscribe;
    using OpenWindowPropagator<View>::dispose_windows;

    int lower;
    int upper;

    OpenSlidingSumBnd(Home home, OpenViewList<View> list,
                      int width0, int lower0, int upper0)
      : OpenWindowPropagator<View>(home,list,width0),
        lower(lower0), upper(upper0) {}

    OpenSlidingSumBnd(Space& home, OpenSlidingSumBnd& p)
      : OpenWindowPropagator<View>(home,p),
        lower(p.lower), upper(p.upper) {}

  public:
    virtual Actor*
    copy(Space& home) {
      return new (home) OpenSlidingSumBnd(home,*this);
    }

    virtual ExecStatus
    advise(Space& home, Advisor& advisor, const Delta& delta) {
      if (View::modevent(delta) == ME_INT_DOM)
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
      for (int i=processed; i<size; i++)
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
        struct Residual {
          long long int lower;
          long long int upper;
        };
        const int windows = end-begin+1;
        Residual* residual = region.alloc<Residual>(windows);
        for (int start=0; start<windows; start++) {
          if ((min_sum > upper) || (max_sum < lower))
            return ES_FAILED;
          residual[start].lower = static_cast<long long int>(lower)-max_sum;
          residual[start].upper = static_cast<long long int>(upper)-min_sum;
          if (start+1 < windows) {
            min_sum -= minimum[start];
            max_sum -= maximum[start];
            min_sum += minimum[start+width];
            max_sum += maximum[start+width];
          }
        }
        // Monotone queues track the strongest residuals among windows
        // covering each position. Every window enters and leaves once.
        int* lower_queue = region.alloc<int>(windows);
        int* upper_queue = region.alloc<int>(windows);
        int lower_head = 0;
        int lower_tail = 0;
        int upper_head = 0;
        int upper_tail = 0;
        for (int i=0; i<n; i++) {
          if ((lower_head < lower_tail) &&
              (lower_queue[lower_head] <= i-width))
            lower_head++;
          if ((upper_head < upper_tail) &&
              (upper_queue[upper_head] <= i-width))
            upper_head++;
          if (i < windows) {
            while ((lower_head < lower_tail) &&
                   (residual[lower_queue[lower_tail-1]].lower <=
                    residual[i].lower))
              lower_tail--;
            lower_queue[lower_tail++] = i;
            while ((upper_head < upper_tail) &&
                   (residual[upper_queue[upper_tail-1]].upper >=
                    residual[i].upper))
              upper_tail--;
            upper_queue[upper_tail++] = i;
          }
          View x(list[begin+i]);
          GECODE_ME_CHECK(x.gq(home,
            residual[lower_queue[lower_head]].lower+maximum[i]));
          GECODE_ME_CHECK(x.lq(home,
            residual[upper_queue[upper_head]].upper+minimum[i]));
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
    post(Home home, OpenViewList<View> list,
         int width, int lower, int upper) {
      (void) new (home)
        OpenSlidingSumBnd(home,list,width,lower,upper);
    }
  };


}}}

#endif

// STATISTICS: int-prop
