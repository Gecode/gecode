/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Scheduling { namespace Unary {

  template<class TaskView>
  forceinline ExecStatus
  detectable(Space& home, TaskViewArray<TaskView>& t) {
    sort<TaskView,STO_ECT,true>(t);

    Region r(home);

    OmegaTree<TaskView> o(r,t);
    TaskIterator<TaskView,STO_LST,true> q(r,t);
    int* est = r.alloc<int>(t.size());

    for (int i=0; i<t.size(); i++) {
      while (q() && (t[i].ect() > t[q.task()].lst())) {
        o.insert(q.task()); ++q;
      }
      est[i] = o.ect(i);
    }

    for (int i=t.size(); i--; )
      GECODE_ME_CHECK(t[i].est(home,est[i]));
      
    return ES_OK;
  }
  
  template<class Task>
  forceinline ExecStatus
  detectable(Space& home, TaskArray<Task>& t) {
    TaskViewArray<TaskTraits<Task>::TaskTypeFwd> f(t);
    GECODE_ES_CHECK(detectable(home,f));
    TaskViewArray<TaskTraits<Task>::TaskTypeBwd> b(t);
    return detectable(home,b);
  }

}}}

// STATISTICS: scheduling-prop
