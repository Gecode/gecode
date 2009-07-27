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
  forceinline
  OmegaLambdaTree<TaskView>::Node::Node(void) {
    // Initialization is performed when tree is initialized
  }


  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::update(int i) {
    assert(!root(i));
    do {
      i = parent(i);
      int l = left(i), r = right(i);
      node[i].p = node[l].p + node[r].p;
      node[i].lp = std::max(node[l].lp + node[r].p,
                            node[l].p + node[r].lp);
      node[i].ect = std::max(node[l].ect + node[r].p, node[r].ect);
      if ((node[r].lect >= node[l].ect + node[r].lp) &&
          (node[r].lect >= node[l].lect + node[r].p)) {
        node[i].lect = node[r].lect;
        node[i].r = node[r].r;
      } else if (node[l].ect + node[r].lp >= node[l].lect + node[r].p) {
        assert(node[l].ect + node[r].lp > node[r].lect);
        node[i].lect = node[l].ect + node[r].lp;
        node[i].r = node[r].r;
      } else {
        assert(node[l].lect + node[r].p > node[r].lect);
        assert(node[l].lect + node[r].p > node[l].ect + node[r].lp);
        node[i].lect = node[l].lect + node[r].p;
        node[i].r = node[l].r;
      }
    } while (!root(i));
  }


  template<class TaskView>
  forceinline
  OmegaLambdaTree<TaskView>::OmegaLambdaTree(Region& r, 
                                             const TaskViewArray<TaskView>& t,
                                             bool inc)
    : TaskTree<TaskView>(r,t), node(r.alloc<Node>(nodes())) {
    if (inc) {
      // Enter all tasks into tree (omega = all tasks, lambda = empty)
      for (int i=tasks.size(); i--; ) {
        node[leave[i]].p = node[leave[i]].lp = t[i].p();
        node[leave[i]].ect = node[leave[i]].lect = t[i].ect();
        node[leave[i]].r = undef;
      }
      for (int i=inner(); i--; ) {
        int l = left(i), r = right(i);
        node[i].p = node[i].lp = node[l].p + node[r].p;
        node[i].ect = node[i].lect = std::max(node[l].ect + node[r].p, 
                                              node[r].ect);
        node[leave[i]].r = undef;
      }
    } else {
      // Enter no tasks into tree (omega = empty, lambda = empty)
      for (int i=nodes(); i--; ) {
        int l = left(i), r = right(i);
        node[i].p = node[i].lp = 0;
        node[i].ect = node[i].lect = -infty;
        node[i].r = undef;
      }
     }
  }

  template<class TaskView>
  forceinline void 
  OmegaLambdaTree<TaskView>::shift(int i) {
    // That means that i is in omega
    assert(node[leave[i]].p > 0);
    node[leave[i]].p = 0;
    node[leave[i]].ect = -infty;
    node[leave[i]].r = i;
    update(leave[i]);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::oinsert(int i) {
    node[leave[i]].p = t[i].p(); 
    node[leave[i]].ect = t[i].ect();
    update(leave[i]);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::linsert(int i) {
    node[leave[i]].lp = t[i].p(); 
    node[leave[i]].lect = t[i].ect();
    node[leave[i]].r = i;
    update(leave[i]);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::lremove(int i) {
    node[leave[i]].lp = 0; 
    node[leave[i]].lect = -infty;
    node[leave[i]].r = undef;
    update(leave[i]);
  }

  template<class TaskView>
  forceinline bool
  OmegaLambdaTree<TaskView>::lempty(void) const {
    return node[0].r < 0;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::responsible(void) const {
    return node[0].r;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::ect(void) const {
    return node[0].ect;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::lect(void) const {
    return node[0].lect;
  }
  
}}}

// STATISTICS: scheduling-prop
