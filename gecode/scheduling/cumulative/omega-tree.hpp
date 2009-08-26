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

namespace Gecode { namespace Scheduling { namespace Cumulative {

  forceinline void
  OmegaNode::init(const OmegaNode&, const OmegaNode&, int) {
    e = 0; env = -Int::Limits::infinity;
  }

  forceinline void
  OmegaNode::update(const OmegaNode& l, const OmegaNode& r, int c) {
    e = l.e + r.e; env = std::max(l.env + r.e, r.env);
  }

  template<class TaskView>
  OmegaTree<TaskView>::OmegaTree(Region& r, int c0,
                                 const TaskViewArray<TaskView>& t)
    : TaskTree<TaskView,OmegaNode,int>(r,t), c(c0) {
    for (int i=tasks.size(); i--; ) {
      leaf(i).e = 0; leaf(i).env = -Int::Limits::infinity;
    }
    init(c);
  }

  template<class TaskView>
  forceinline void 
  OmegaTree<TaskView>::insert(int i) {
    leaf(i).e = tasks[i].e(); 
    leaf(i).env = c*tasks[i].est();
    update(i,c);
  }

  template<class TaskView>
  forceinline void
  OmegaTree<TaskView>::remove(int i) {
    leaf(i).e = 0; leaf(i).env = -Int::Limits::infinity;
    update(i,c);
  }

  template<class TaskView>
  forceinline int 
  OmegaTree<TaskView>::env(void) const {
    return root().env;
  }
  
}}}

// STATISTICS: scheduling-prop
