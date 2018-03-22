/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
 *     Guido Tack, 2010
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

namespace Gecode { namespace Int { namespace Unary {

  template<class Task>
  ExecStatus
  timetabling(Space& home, Propagator& p, TaskArray<Task>& t) {
    Region r;

    bool assigned;
    if (Event* e = Event::events(r,t,assigned)) {
      // Whether resource is free
      bool free = true;
      // Set of current but not required tasks
      Support::BitSet<Region> tasks(r,static_cast<unsigned int>(t.size()));

      // Process events
      do {
        // Current time
        int time = e->time();

        // Process events for completion of required part
        for ( ; (e->type() == Event::LRT) && (e->time() == time); e++)
          if (t[e->idx()].mandatory()) {
            tasks.set(static_cast<unsigned int>(e->idx()));
            free = true;
          }

        // Process events for completion of task
        for ( ; (e->type() == Event::LCT) && (e->time() == time); e++)
          tasks.clear(static_cast<unsigned int>(e->idx()));

        // Process events for start of task
        for ( ; (e->type() == Event::EST) && (e->time() == time); e++)
          tasks.set(static_cast<unsigned int>(e->idx()));

        // Process events for zero-length task
        for ( ; (e->type() == Event::ZRO) && (e->time() == time); e++)
          if (!free)
            return ES_FAILED;

        // Norun start time
        int nrstime = time;
        // Process events for start of required part
        for ( ; (e->type() == Event::ERT) && (e->time() == time); e++)
          if (t[e->idx()].mandatory()) {
            tasks.clear(static_cast<unsigned int>(e->idx()));
            if (!free)
              return ES_FAILED;
            free = false;
            nrstime = time+1;
          } else if (t[e->idx()].optional() && !free) {
            GECODE_ME_CHECK(t[e->idx()].excluded(home));
          }

        if (!free)
          for (Iter::Values::BitSet<Support::BitSet<Region> > j(tasks);
               j(); ++j)
            // Task j cannot run from time to next time - 1
            if (t[j.val()].mandatory())
              GECODE_ME_CHECK(t[j.val()].norun(home, nrstime, e->time() - 1));

      } while (e->type() != Event::END);
    }

    if (assigned)
      return home.ES_SUBSUMED(p);

    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop
