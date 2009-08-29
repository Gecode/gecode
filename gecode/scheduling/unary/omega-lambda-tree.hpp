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

  forceinline void
  OmegaLambdaNode::init(const OmegaLambdaNode& l, const OmegaLambdaNode& r) {
    OmegaNode::init(l,r);
    lp = p; lect = ect; res = undef;
  }

  forceinline void
  OmegaLambdaNode::update(const OmegaLambdaNode& l, const OmegaLambdaNode& r) {
    OmegaNode::update(l,r);
    lp = std::max(l.lp + r.p, l.p + r.lp);
    if ((r.lect >= l.ect + r.lp) && (r.lect >= l.lect + r.p)) {
      lect = r.lect; res = r.res;
    } else if (l.ect + r.lp >= l.lect + r.p) {
      assert(l.ect + r.lp > r.lect);
      lect = l.ect + r.lp; res = r.res;
    } else {
      assert((l.lect + r.p > r.lect) && (l.lect + r.p > l.ect + r.lp));
      lect = l.lect + r.p; res = l.res;
    }
  }


  template<class TaskView>
  OmegaLambdaTree<TaskView>::OmegaLambdaTree(Region& r, 
                                             const TaskViewArray<TaskView>& t,
                                             bool inc)
    : TaskTree<TaskView,OmegaLambdaNode>(r,t) {
    if (inc) {
      // Enter all tasks into tree (omega = all tasks, lambda = empty)
      for (int i=tasks.size(); i--; ) {
        leaf(i).p = leaf(i).lp = tasks[i].p();
        leaf(i).ect = leaf(i).lect = tasks[i].ect();
        leaf(i).res = OmegaLambdaNode::undef;
      }
    } else {
      // Enter no tasks into tree (omega = empty, lambda = empty)
      for (int i=tasks.size(); i--; ) {
        leaf(i).p = leaf(i).lp = 0;
        leaf(i).ect = leaf(i).lect = -Int::Limits::infinity;
        leaf(i).res = OmegaLambdaNode::undef;
      }
     }
    init();
  }

  template<class TaskView>
  forceinline void 
  OmegaLambdaTree<TaskView>::shift(int i) {
    // That means that i is in omega
    assert(leaf(i).p > 0);
    leaf(i).p = 0;
    leaf(i).ect = -Int::Limits::infinity;
    leaf(i).res = i;
    update(i);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::oinsert(int i) {
    leaf(i).p = tasks[i].p(); 
    leaf(i).ect = tasks[i].ect();
    update(i);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::linsert(int i) {
    leaf(i).lp = tasks[i].p(); 
    leaf(i).lect = tasks[i].ect();
    leaf(i).res = i;
    update(i);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::lremove(int i) {
    leaf(i).lp = 0; 
    leaf(i).lect = -Int::Limits::infinity;
    leaf(i).res = OmegaLambdaNode::undef;
    update(i);
  }

  template<class TaskView>
  forceinline bool
  OmegaLambdaTree<TaskView>::lempty(void) const {
    return root().res < 0;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::responsible(void) const {
    return root().res;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::ect(void) const {
    return root().ect;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::lect(void) const {
    return root().lect;
  }
  
}}}

// STATISTICS: scheduling-prop
