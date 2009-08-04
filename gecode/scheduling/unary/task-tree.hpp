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

  template<class TaskView, class Node>
  forceinline int 
  TaskTree<TaskView,Node>::inner(void) const {
    return tasks.size()-1;
  }
  template<class TaskView, class Node>
  forceinline int 
  TaskTree<TaskView,Node>::nodes(void) const {
    return 2*tasks.size() - 1;
  }

  template<class TaskView, class Node>
  forceinline bool 
  TaskTree<TaskView,Node>::root(int i) {
    return i == 0;
  }
  template<class TaskView, class Node>
  forceinline int 
  TaskTree<TaskView,Node>::left(int i) {
    return 2*(i+1) - 1;
  }
  template<class TaskView, class Node>
  forceinline int
  TaskTree<TaskView,Node>::right(int i) {
    return 2*(i+1);
  }
  template<class TaskView, class Node>
  forceinline int
  TaskTree<TaskView,Node>::parent(int i) {
    return (i+1)/2 - 1;
  }

  template<class TaskView, class Node>
  forceinline void
  TaskTree<TaskView,Node>::init(void) {
    for (int i=inner(); i--; )
      node[i].update(node[left(i)],node[right(i)]);
  }

  template<class TaskView, class Node>
  forceinline void
  TaskTree<TaskView,Node>::update(int i) {
    i = leaf[i];
    assert(!root(i));
    do {
      i = parent(i);
      node[i].update(node[left(i)],node[right(i)]);
    } while (!root(i));
  }

  template<class TaskView, class Node>
  forceinline
  TaskTree<TaskView,Node>::TaskTree(Region& r, 
                                    const TaskViewArray<TaskView>& t)
    : tasks(t), 
      node(r.alloc<Node>(nodes())),
      leaf(r.alloc<int>(tasks.size())) {
    // Compute a sorting map to order by non decreasing est
    int* map = r.alloc<int>(t.size());
    sort<TaskView,STO_EST,true>(map, t);
    // Compute inverse of sorting map
    for (int i=t.size(); i--; )
      leaf[map[i]] = i;
    r.free<int>(map,t.size());
    // Compute index of first leaf in tree: the next larger power of two
    int fst = 1;
    while (fst < tasks.size())
      fst <<= 1;
    fst--;
    // Remap task indices to leaf indices
    for (int i=tasks.size(); i--; )
      if (leaf[i] + fst >= nodes())
        leaf[i] += fst - tasks.size();
      else
        leaf[i] += fst;
  }

}}}

// STATISTICS: scheduling-prop
