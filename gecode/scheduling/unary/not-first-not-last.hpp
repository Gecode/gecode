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

#include <algorithm>

namespace Gecode { namespace Scheduling { namespace Unary {

  template<class TaskView>
  forceinline ExecStatus
  notlast(Space& home, TaskViewArray<TaskView>& t) {
    sort<TaskView,STO_LCT,true>(t);

    Region r(home);

    OmegaTree<TaskView> o(r,t);
    TaskViewIterator<TaskView,STO_LST,true> q(r,t);
    int* lct = r.alloc<int>(t.size());

    for (int i=t.size(); i--; )
      lct[i] = t[i].lct();

    for (int i=0; i<t.size(); i++) {
      int j = -1;
      while (q() && (t[i].lct() > t[q.task()].lst())) {
        if ((j >= 0) && (o.ect() > t[q.task()].lst()))
          lct[q.task()] = std::min(lct[q.task()],t[j].lst());
        j = q.task();
        o.insert(j); ++q;
      }
      if ((j >= 0) && (o.ect(i) > t[i].lst()))
        lct[i] = std::min(lct[i],t[j].lst());
    }

    for (int i=t.size(); i--; )
      GECODE_ME_CHECK(t[i].lct(home,lct[i]));
      
    return ES_OK;
  }

  template<class Task>
  ExecStatus
  notfirstnotlast(Space& home, TaskArray<Task>& t) {
    TaskViewArray<typename TaskTraits<Task>::TaskViewFwd> f(t);
    GECODE_ES_CHECK(notlast(home,f));
    TaskViewArray<typename TaskTraits<Task>::TaskViewBwd> b(t);
    return notlast(home,b);
  }
  
}}}

// STATISTICS: scheduling-prop
