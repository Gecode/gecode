/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /// Event type for task
  enum EventType {
    ET_LRT = 0, ///< Latest required time of task
    ET_LCT = 1, ///< Latest completion time of task
    ET_EST = 2, ///< Earliest start time of task
    ET_ERT = 3, ///< Earliest required time of task
    ET_END = 4  ///< End marker
  };

  /// Event for task
  class Event {
  public:
    EventType e; ///< Type of event
    int t;       ///< Time of event
    int i;       ///< Number of task
    /// Initialize event
    void init(EventType e, int t, int i);
  };

  /// Sort events by time and type
  class EventByTimeType {
  public:
    /// Sort order
    bool operator ()(const Event& e1, const Event& e2) const;
  };


  forceinline void
  Event::init(EventType e0, int t0, int i0) {
    e=e0; t=t0; i=i0;
  }

  forceinline bool
  EventByTimeType::operator ()(const Event& e1, const Event& e2) const {
    if (e1.t == e2.t)
      return e1.e < e2.e;
    return e1.t < e2.t;
  }

  // Basic propagation
  template<class Task>
  ExecStatus
  basic(Space& home, int c, TaskArray<Task>& t) {
    Region r(home);

    Event* e = r.alloc<Event>(4*t.size()+1);

    // Initialize events
    {
      int n=0;
      for (int i=t.size(); i--; ) {
        e[n++].init(ET_EST,t[i].est(),i);
        e[n++].init(ET_LCT,t[i].lct(),i);
        // Check whether task has required part
        if (t[i].lst() < t[i].ect()) {
          e[n++].init(ET_ERT,t[i].lst(),i);
          e[n++].init(ET_LRT,t[i].ect(),i);
        }
      }
      
      // Check whether no task has a required part
      if (n == 2*t.size())
        return ES_OK;
      
      // Write end marker
      e[n++].init(ET_END,Int::Limits::infinity,-1);
      
      // Sort events
      EventByTimeType ebtt;
      Support::quicksort(e, n, ebtt);
    }

    // Set of current and required tasks
    Support::BitSet<Region> ct(r,t.size());
    Support::BitSet<Region> rt(r,t.size());

    // Process events, use c as the capacity that is still free
    while (true) {
      // Current time
      int t_c = e->t;
      // Process events for completion of required part
      for (; (e->t == t_c) && (e->e == ET_LRT); e++) {
        rt.clear(e->i); ct.set(e->i);
        c += t[e->i].c();
      }
      // Process events for completion of task
      for (; (e->t == t_c) && (e->e == ET_LCT); e++) {
        ct.clear(e->i);
      }
      // Process events for start of task
      for (; (e->t == t_c) && (e->e == ET_EST); e++) {
        ct.set(e->i);
      }
      // Process events for start of required part
      for (; (e->t == t_c) && (e->e == ET_ERT); e++) {
        ct.clear(e->i); rt.set(e->i);
        c -= t[e->i].c();        
        if (c < 0)
          return ES_FAILED;
      }

      if (e->e == ET_END)
        break;
      
      // Next time
      int t_n = e->t;
      assert(t_n != t_c);
      for (Iter::Values::BitSet<Support::BitSet<Region> > j(ct); j(); ++j) 
        if (t[j.val()].c() > c) {
          // Task cannot run from t_c to t_n
        }

    }

    return ES_OK;
  }

}}}

// STATISTICS: scheduling-prop
