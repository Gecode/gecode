/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
 *     Guido Tack, 2010
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

namespace Gecode { namespace Int { namespace Cumulative {

  /// Sort order for tasks by decreasing capacity
  template<class Task>
  class TaskByDecCap {
  public:
    /// Sort order
    bool operator ()(const Task& t1, const Task& t2) const;
  };

  template<class Task>
  forceinline bool
  TaskByDecCap<Task>::operator ()(const Task& t1, const Task& t2) const {
    return t1.c() > t2.c();
  }

  // Basic propagation (timetabling)
  template<class Task, class Cap>
  ExecStatus
  timetabling(Space& home, bool& subsumed, Cap c, TaskArray<Task>& t) {
    subsumed = false;
    int ccur = c.max();
    int cmax = ccur;
    int cmin = ccur;
    // Sort tasks by decreasing capacity
    TaskByDecCap<Task> tbdc;
    Support::quicksort(&t[0], t.size(), tbdc);

    Region r(home);

    bool assigned;
    Event* e = Event::events(r,t,assigned);

    if (e == NULL) {
      subsumed = assigned;
      return ES_FIX;
    }
      
    // Set of current but not required tasks
    Support::BitSet<Region> tasks(r,static_cast<unsigned int>(t.size()));

    // Process events, use ccur as the capacity that is still free
    while (e->type() != Event::END) {
      // Current time
      int time = e->time();

      // Process events for completion of required part
      for ( ; (e->time() == time) && (e->type() == Event::LRT); e++) 
        if (t[e->idx()].mandatory()) {
          tasks.set(static_cast<unsigned int>(e->idx())); ccur += t[e->idx()].c();
        }
      // Process events for completion of task
      for ( ; (e->time() == time) && (e->type() == Event::LCT); e++)
        tasks.clear(static_cast<unsigned int>(e->idx()));
      // Process events for start of task
      for ( ; (e->time() == time) && (e->type() == Event::EST); e++)
        tasks.set(static_cast<unsigned int>(e->idx()));
      // Process events for zero-length task
      for ( ; (e->time() == time) && (e->type() == Event::ZRO); e++) {
        ccur -= t[e->idx()].c();
        if (ccur < cmin) cmin=ccur;
        if (ccur < 0)
          return ES_FAILED;
        ccur += t[e->idx()].c();
      }

      // norun start time for 0-length tasks
      int zltime = time;
      // Process events for start of required part
      for ( ; (e->time() == time) && (e->type() == Event::ERT); e++) 
        if (t[e->idx()].mandatory()) {
          tasks.clear(static_cast<unsigned int>(e->idx())); 
          ccur -= t[e->idx()].c();
          if (ccur < cmin) cmin=ccur;
          zltime = time+1;
          if (ccur < 0)
            return ES_FAILED;
        } else if (t[e->idx()].optional() && (t[e->idx()].c() > ccur)) {
          GECODE_ME_CHECK(t[e->idx()].excluded(home));
        }
      
      // Exploit that tasks are sorted according to capacity
      for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks); 
           j() && (t[j.val()].c() > ccur); ++j) 
        // Task j cannot run from time to next time - 1
        if (t[j.val()].mandatory()) {
          if (t[j.val()].pmin() > 0) {
            GECODE_ME_CHECK(t[j.val()].norun(home, time, e->time() - 1));
          } else {
            GECODE_ME_CHECK(t[j.val()].norun(home, zltime, e->time() - 1));
          }
        }
    }

    GECODE_ME_CHECK(c.gq(home,cmax-cmin));

    subsumed = assigned;
    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop
