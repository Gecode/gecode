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
  forceinline int 
  TaskTree<TaskView>::inner(void) const {
    return tasks.size()-1;
  }
  template<class TaskView>
  forceinline int 
  TaskTree<TaskView>::nodes(void) const {
    return 2*tasks.size() - 1;
  }

  template<class TaskView>
  forceinline bool 
  TaskTree<TaskView>::root(int i) {
    return i == 0;
  }
  template<class TaskView>
  forceinline int 
  TaskTree<TaskView>::left(int i) {
    return 2*(i+1) - 1;
  }
  template<class TaskView>
  forceinline int
  TaskTree<TaskView>::right(int i) {
    return 2*(i+1);
  }
  template<class TaskView>
  forceinline int
  TaskTree<TaskView>::parent(int i) {
    return (i+1)/2 - 1;
  }

  template<class TaskView>
  forceinline
  TaskTree<TaskView>::TaskTree(Region& r, const TaskViewArray<TaskView>& t)
    : tasks(t), leave(r.alloc<int>(tasks.size())) {
    // Compute a sorting map to order by non decreasing est
    int* map = r.alloc<int>(t.size());
    sort<TaskView,STO_EST,true>(map, t);
    // Compute inverse of sorting map
    for (int i=t.size(); i--; )
      leave[map[i]] = i;
    r.free<int>(map,t.size());
    // Compute index of first leave in tree: the next larger power of two
    int fst = 1;
    while (fst < tasks.size())
      fst <<= 1;
    fst--;
    // Remap task indices to leave indices
    for (int i=tasks.size(); i--; )
      if (leave[i] + fst >= nodes())
        leave[i] += fst - tasks.size();
      else
        leave[i] += fst;
  }

}}}

// STATISTICS: scheduling-prop
