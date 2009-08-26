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
  OmegaNode::init(const OmegaNode&, const OmegaNode&, 
                  const TaskNoInfo&) {
    p = 0; ect = -Int::Limits::infinity;
  }

  forceinline void
  OmegaNode::update(const OmegaNode& l, const OmegaNode& r, 
                    const TaskNoInfo&) {
    p = l.p + r.p; ect = std::max(l.ect + r.p, r.ect);
  }

  template<class TaskView>
  OmegaTree<TaskView>::OmegaTree(Region& r, const TaskViewArray<TaskView>& t)
    : TaskTree<TaskView,OmegaNode,TaskNoInfo>(r,t) {
    for (int i=tasks.size(); i--; ) {
      leaf(i).p = 0; leaf(i).ect = -Int::Limits::infinity;
    }
    TaskNoInfo n;
    init(n);
  }

  template<class TaskView>
  forceinline void 
  OmegaTree<TaskView>::insert(int i) {
    leaf(i).p = tasks[i].p(); leaf(i).ect = tasks[i].ect();
    TaskNoInfo n;
    update(i,n);
  }

  template<class TaskView>
  forceinline void
  OmegaTree<TaskView>::remove(int i) {
    leaf(i).p = 0; leaf(i).ect = -Int::Limits::infinity;
    TaskNoInfo n;
    update(i,n);
  }

  template<class TaskView>
  forceinline int 
  OmegaTree<TaskView>::ect(void) const {
    return root().ect;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaTree<TaskView>::ect(int i) const {
    // Check whether task i is in?
    OmegaTree<TaskView>& o = const_cast<OmegaTree<TaskView>&>(*this);
    if (o.leaf(i).p > 0) {
      o.remove(i);
      int ect = o.root().ect;
      o.insert(i);
      return ect;
    } else {
      return root().ect;
    }
  }
  
}}}

// STATISTICS: scheduling-prop
