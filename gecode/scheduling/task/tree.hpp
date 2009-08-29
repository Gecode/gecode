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

namespace Gecode { namespace Scheduling {

  template<class TaskView, class Node>
  forceinline int 
  TaskTree<TaskView,Node>::_inner(void) const {
    return tasks.size()-1;
  }
  template<class TaskView, class Node>
  forceinline int 
  TaskTree<TaskView,Node>::_nodes(void) const {
    return 2*tasks.size() - 1;
  }

  template<class TaskView, class Node>
  forceinline bool 
  TaskTree<TaskView,Node>::_root(int i) {
    return i == 0;
  }
  template<class TaskView, class Node>
  forceinline int 
  TaskTree<TaskView,Node>::_left(int i) {
    return 2*(i+1) - 1;
  }
  template<class TaskView, class Node>
  forceinline int
  TaskTree<TaskView,Node>::_right(int i) {
    return 2*(i+1);
  }
  template<class TaskView, class Node>
  forceinline int
  TaskTree<TaskView,Node>::_parent(int i) {
    return (i+1)/2 - 1;
  }

  template<class TaskView, class Node>
  forceinline Node&
  TaskTree<TaskView,Node>::leaf(int i) {
    return _node[_leaf[i]];
  }

  template<class TaskView, class Node>
  forceinline const Node&
  TaskTree<TaskView,Node>::root(void) const {
    return _node[0];
  }

  template<class TaskView, class Node>
  forceinline void
  TaskTree<TaskView,Node>::init(void) {
    for (int i=_inner(); i--; )
      _node[i].init(_node[_left(i)],_node[_right(i)]);
  }

  template<class TaskView, class Node>
  forceinline void
  TaskTree<TaskView,Node>::update(int i) {
    i = _leaf[i];
    assert(!_root(i));
    do {
      i = _parent(i);
      _node[i].update(_node[_left(i)],_node[_right(i)]);
    } while (!_root(i));
  }

  template<class TaskView, class Node>
  forceinline
  TaskTree<TaskView,Node>::TaskTree(Region& r, 
                                    const TaskViewArray<TaskView>& t)
    : tasks(t), 
      _node(r.alloc<Node>(_nodes())),
      _leaf(r.alloc<int>(tasks.size())) {
    // Compute a sorting map to order by non decreasing est
    int* map = r.alloc<int>(tasks.size());
    sort<TaskView,STO_EST,true>(map, tasks);
    // Compute inverse of sorting map
    for (int i=tasks.size(); i--; )
      _leaf[map[i]] = i;
    r.free<int>(map,tasks.size());
    // Compute index of first leaf in tree: the next larger power of two
    int fst = 1;
    while (fst < tasks.size())
      fst <<= 1;
    fst--;
    // Remap task indices to leaf indices
    for (int i=tasks.size(); i--; )
      if (_leaf[i] + fst >= _nodes())
        _leaf[i] += fst - tasks.size();
      else
        _leaf[i] += fst;
  }

}}

// STATISTICS: scheduling-prop
