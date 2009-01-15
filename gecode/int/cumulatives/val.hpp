/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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

#include <vector>
#include <list>
#include <algorithm>

/* 
 * This is the propagation algorithm of the cumulatives constraint as 
 * presented in: 
 *   Nicolas Beldiceanu and Mats Carlsson, A New Multi-resource cumulatives 
 *   Constraint with Negative Heights. CP 2002, pages 63-79, Springer-Verlag.
 */

namespace Gecode { namespace Int { namespace Cumulatives {
      
  template <class ViewM, class ViewD, class ViewH, class View>
  forceinline
  Val<ViewM,ViewD,ViewH,View>::Val(Space& home, 
                                   const ViewArray<ViewM>& _machine,
                                   const ViewArray<View>& _start,
                                   const ViewArray<ViewD>& _duration,
                                   const ViewArray<View>& _end,
                                   const ViewArray<ViewH>& _height,
                                   const SharedArray<int>& _limit,
                                   bool _at_most) :
    Propagator(home),
    machine(_machine), start(_start), duration(_duration),
    end(_end), height(_height), limit(_limit), at_most(_at_most) {
    home.notice(*this,AP_DISPOSE);    

    machine.subscribe(home,*this,PC_INT_DOM);
    start.subscribe(home,*this,PC_INT_BND);
    duration.subscribe(home,*this,PC_INT_BND);
    end.subscribe(home,*this,PC_INT_BND);
    height.subscribe(home,*this,PC_INT_BND);
  }

  template <class ViewM, class ViewD, class ViewH, class View>
  ExecStatus
  Val<ViewM,ViewD,ViewH,View>
  ::post(Space& home, const ViewArray<ViewM>& machine,
         const ViewArray<View>& start, const ViewArray<ViewD>& duration,
         const ViewArray<View>& end, const ViewArray<ViewH>& height,
         const SharedArray<int>& limit, bool at_most) {
    (void) new (home) Val(home, machine, start, duration,
                          end, height, limit, at_most);

    return ES_OK;
  }

  template <class ViewM, class ViewD, class ViewH, class View>
  forceinline
  Val<ViewM,ViewD,ViewH,View>::Val(Space& home, bool share,
                                   Val<ViewM,ViewD,ViewH,View>& p)
    : Propagator(home,share,p), at_most(p.at_most) {
    machine.update(home,share,p.machine);
    start.update(home, share, p.start);
    duration.update(home, share, p.duration);
    end.update(home, share, p.end);
    height.update(home, share, p.height);
    limit.update(home, share, p.limit);
  }

  template <class ViewM, class ViewD, class ViewH, class View>
  size_t
  Val<ViewM,ViewD,ViewH,View>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    if (!home.failed()) {
      machine.cancel(home,*this,PC_INT_DOM);
      start.cancel(home,*this,PC_INT_BND);
      duration.cancel(home,*this,PC_INT_BND);
      end.cancel(home,*this,PC_INT_BND);
      height.cancel(home,*this,PC_INT_BND);
    }
    limit.~SharedArray();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class ViewM, class ViewD, class ViewH, class View>
  PropCost
  Val<ViewM,ViewD,ViewH,View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LOW, start.size());
  }

  template <class ViewM, class ViewD, class ViewH, class View>
  Actor*
  Val<ViewM,ViewD,ViewH,View>::copy(Space& home, bool share) {
    return new (home) Val<ViewM,ViewD,ViewH,View>(home,share,*this);
  }

  /// Types of events for the sweep-line
  typedef enum {EVENT_CHCK, EVENT_PROF, EVENT_PRUN} ev_t;
  /// An event collects the information for one evnet for the sweep-line
  class Event
  {
  public:
    /// The type of the event
    ev_t e;
    /// The task this event refers to
    int task;
    /// The date of this event
    int date;
    /// The quantity changed by this event (if any)
    int inc;
    /** If the type is EVENT_PROF and it is the first of the pair,
     * this value is true. Added to handle contribution-values
     * correctly for both at_most and at_least.
     */
    bool first_prof;

    /// Simple constructor
    Event(ev_t _e, int _task, int _date, int _inc = 0, bool _first_prof = false)
      : e(_e), task(_task), date(_date), inc(_inc), first_prof(_first_prof)
    {}

    // Default constructor for region-allocated memory
    Event(void) {}

    /// Order events based on date.
    bool operator <(const Event& ev) const {
      if (date == ev.date) {
        if (e == EVENT_PROF && ev.e != EVENT_PROF) return true;
        if (e == EVENT_CHCK && ev.e == EVENT_PRUN) return true;
        return false;
      }
      return date < ev.date;
    }
  };

  template <class ViewM, class ViewD, class ViewH, class View>
  ExecStatus
  Val<ViewM,ViewD,ViewH,View>::prune(Space& home, int low, int up, int r,
                                     int ntask, int sheight,
                                     int* contribution,
                                     int* prune_tasks, int& prune_tasks_size) {

    if (ntask > 0 &&
        (at_most ? sheight > limit[r]
         : sheight < limit[r])) {
      return ES_FAILED;
    }

    int pti = 0;
    while (pti != prune_tasks_size) {
      int t = prune_tasks[pti];

      // Algorithm 2.
      // Prune the machine, start, and end for required
      // tasks for machine r that have heights possibly greater than 0.
      if (ntask != 0 &&
          (at_most ? height[t].min() < 0
           : height[t].max() > 0) &&
          (at_most ? sheight-contribution[t] > limit[r]
           : sheight-contribution[t] < limit[r])) {
        if (me_failed(machine[t].eq(home, r))||
            me_failed(start[t].gq(home, up-duration[t].max()+1))||
            me_failed(start[t].lq(home, low))||
            me_failed(end[t].lq(home,low+duration[t].max()))||
            me_failed(end[t].gq(home, up+1))||
            me_failed(duration[t].gq(home,std::min(up-start[t].max()+1,
                                                   end[t].min()-low)))
            ) {
          return ES_FAILED;
        }
      }

      // Algorithm 3.
      // Remove values that prevent us from reaching the limit
      if (at_most ? height[t].min() > std::min(0, limit[r])
          : height[t].max() < std::max(0, limit[r])) {
        if (at_most ? sheight-contribution[t]+height[t].min() > limit[r]
            : sheight-contribution[t]+height[t].max() < limit[r]) {
          if (end[t].min() > low  &&
              start[t].max() <= up &&
              duration[t].min() > 0) {
            if (me_failed(machine[t].nq(home, r))) {
              return ES_FAILED;
            }
          } else if (machine[t].assigned()) {
            int dtmin = duration[t].min();
            if (dtmin > 0) {
              Iter::Ranges::Singleton
                a(low-dtmin+1, up), b(low+1, up+dtmin);
              if (me_failed(start[t].minus_r(home,a,false)) ||
                  me_failed(end[t].minus_r(home, b,false)))
                return ES_FAILED;
            }
            if (me_failed(duration[t].lq(home,
                                         std::max(std::max(low-start[t].min(),
                                                           end[t].max()-up-1),
                                                  0)))) {
              return ES_FAILED;
            }
          }
        }
      }

      // Algorithm 4.
      // Remove bad negative values from for-sure heights.
      /* \note "It is not sufficient to only test for assignment of machine[t]
       *         since Algorithm 3 can remove the value." Check this against
       *         the conditions for Alg3.
       */
      if (machine[t].assigned() &&
          machine[t].val() == r &&
          end[t].min() > low    &&
          start[t].max() <= up  &&
          duration[t].min() > 0 ) {
        if (me_failed(at_most
                      ? height[t].lq(home, limit[r]-sheight+contribution[t])
                      : height[t].gq(home, limit[r]-sheight+contribution[t]))) {
          return ES_FAILED;
        }
      }

      // Remove tasks that are no longer relevant.
      if (!machine[t].in(r) || (end[t].max() <= up+1)) {
        prune_tasks[pti] = prune_tasks[--prune_tasks_size];
      } else {
        ++pti;
      }
    }

    return ES_OK;
  }
      
  namespace {
    template <class C>
    class Less {
    public:
      bool operator ()(const C& lhs, const C& rhs) {
        return lhs < rhs;
      }
    };
  }

  template <class ViewM, class ViewD, class ViewH, class View>
  ExecStatus
  Val<ViewM,ViewD,ViewH,View>::propagate(Space& home, const ModEventDelta&) {
    // Check for subsumption
    bool subsumed = true;
    for (int t = start.size(); t--; )
      if (!(duration[t].assigned() && end[t].assigned()   &&
            machine[t].assigned()  && start[t].assigned() &&
            height[t].assigned())) {
        subsumed = false;
        break;
      }
    // Propagate information for machine r
    Region region(home);
    Event *events = region.alloc<Event>(start.size()*8);
    int  events_size;
    int *prune_tasks = region.alloc<int>(start.size());
    int  prune_tasks_size;
    int *contribution = region.alloc<int>(start.size());
    for (int r = limit.size(); r--; ) {
      events_size = 0;
#define GECODE_PUSH_EVENTS(E) assert(events_size < start.size()*8);     \
        events[events_size++] = E
      
      // Find events for sweep-line
      for (int t = start.size(); t--; ) {
        if (machine[t].assigned() &&
            machine[t].val() == r &&
            start[t].max() < end[t].min()) {
          if (at_most
              ? height[t].min() > std::min(0, limit[r])
              : height[t].max() < std::max(0, limit[r])) {
            GECODE_PUSH_EVENTS(Event(EVENT_CHCK, t, start[t].max(), 1));
            GECODE_PUSH_EVENTS(Event(EVENT_CHCK, t, end[t].min(), -1));
          }
          if (at_most
              ? height[t].min() > 0
              : height[t].max() < 0) {
            GECODE_PUSH_EVENTS(Event(EVENT_PROF, t, start[t].max(),
                                   at_most ? height[t].min()
                                   : height[t].max(), true));
            GECODE_PUSH_EVENTS(Event(EVENT_PROF, t, end[t].min(),
                                   at_most ? -height[t].min()
                                   : -height[t].max()));
          }
        }
        
        if (machine[t].in(r)) {
          if (at_most
              ? height[t].min() < 0
              : height[t].max() > 0) {
            GECODE_PUSH_EVENTS(Event(EVENT_PROF, t, start[t].min(),
                                   at_most ? height[t].min()
                                   : height[t].max(), true));
            GECODE_PUSH_EVENTS(Event(EVENT_PROF, t, end[t].max(),
                                   at_most ? -height[t].min()
                                   : -height[t].max()));
          }
          if (!(machine[t].assigned() &&
                 height[t].assigned() &&
                  start[t].assigned() &&
                    end[t].assigned())) {
            GECODE_PUSH_EVENTS(Event(EVENT_PRUN, t, start[t].min()));
          }
        }
      }
#undef GECODE_PUSH_EVENTS

      // If there are no events, continue with next machine
      if (events_size == 0) {
        continue;
      }

      // Sort the events according to date
      Less<Event> less;
      Support::insertion(&events[0], events_size, less);

      // Sweep line along d, starting at 0
      int d        = 0;
      int ntask    = 0;
      int sheight  = 0;
      int ei = 0;
      
      prune_tasks_size = 0;
      for (int i = start.size(); i--; ) contribution[i] = 0;
      
      d = events[ei].date;
      while (ei < events_size) {
        if (events[ei].e != EVENT_PRUN) {
          if (d != events[ei].date) {
            GECODE_ES_CHECK(prune(home, d, events[ei].date-1, r,
                                  ntask, sheight,
                                  contribution, prune_tasks, prune_tasks_size));
            d = events[ei].date;
          }
          if (events[ei].e == EVENT_CHCK) {
            ntask += events[ei].inc;
          } else /* if (events[ei].e == EVENT_PROF) */ {
            sheight += events[ei].inc;
            if(events[ei].first_prof)
              contribution[events[ei].task] = at_most
                ? std::max(contribution[events[ei].task], events[ei].inc)
                : std::min(contribution[events[ei].task], events[ei].inc);
          }
        } else /* if (events[ei].e == EVENT_PRUN) */ {
          assert(prune_tasks_size<start.size());
          prune_tasks[prune_tasks_size++] = events[ei].task;
        }
        ei++;
      }
      
      GECODE_ES_CHECK(prune(home, d, d, r,
                            ntask, sheight,
                            contribution, prune_tasks, prune_tasks_size));
    }
    return subsumed ? ES_SUBSUMED(*this,home): ES_NOFIX;
  }
      
}}}

// STATISTICS: int-prop

