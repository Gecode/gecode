/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

  template<class Task>
  forceinline ExecStatus
  subsumed(Space& home, Propagator& p, int c, TaskArray<Task>& t) {
    Region r(home);

    if (Event* e = Event::events(r,t)) {
      // Process events, use ccur as the capacity that is still free
      while (e->type() != Event::END) {
        // Current time
        int time = e->time();
        // Process events for completion of required part
        for ( ; (e->time() == time) && (e->type() == Event::LRT); e++) 
          c += t[e->idx()].c();
        // Process events for zero-length task
        for ( ; (e->time() == time) && (e->type() == Event::ZRO); e++)
          if (c < t[e->idx()].c())
            return ES_FAILED;
        // Process events for start of required part
        for ( ; (e->time() == time) && (e->type() == Event::ERT); e++) {
          c -= t[e->idx()].c();
          if (c < 0)
            return ES_FAILED;
        }
      }
    } else {
      return ES_OK;
    }

    return home.ES_SUBSUMED(p);
  }
  
}}}

// STATISTICS: int-prop
