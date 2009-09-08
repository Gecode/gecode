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
#include <cmath>

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /*
   * Omega tree
   */

  forceinline void
  OmegaNode::init(const OmegaNode&, const OmegaNode&) {
    e = 0; env = -Int::Limits::infinity;
  }

  forceinline void
  OmegaNode::update(const OmegaNode& l, const OmegaNode& r) {
    e = l.e + r.e; env = std::max(l.env + r.e, r.env);
  }

  template<class TaskView>
  OmegaTree<TaskView>::OmegaTree(Region& r, int c0,
                                 const TaskViewArray<TaskView>& t)
    : TaskTree<TaskView,OmegaNode>(r,t), c(c0) {
    for (int i=tasks.size(); i--; ) {
      leaf(i).e = 0; leaf(i).env = -Int::Limits::infinity;
    }
    init();
  }

  template<class TaskView>
  forceinline void 
  OmegaTree<TaskView>::insert(int i) {
    leaf(i).e = tasks[i].e(); 
    leaf(i).env = c*tasks[i].est()+tasks[i].e();
    update(i);
  }

  template<class TaskView>
  forceinline void
  OmegaTree<TaskView>::remove(int i) {
    leaf(i).e = 0; leaf(i).env = -Int::Limits::infinity;
    update(i);
  }

  template<class TaskView>
  forceinline int 
  OmegaTree<TaskView>::env(void) const {
    return root().env;
  }
  

  /*
   * Extended Omega tree
   */

  forceinline void
  ExtOmegaNode::init(const ExtOmegaNode& l, const ExtOmegaNode& r) {
    OmegaNode::init(l,r);
    cenv = -Int::Limits::infinity;
  }

  forceinline void
  ExtOmegaNode::update(const ExtOmegaNode& l, const ExtOmegaNode& r) {
    OmegaNode::update(l,r);
    cenv = std::max(l.cenv + r.e, r.cenv);
  }

  template<class TaskView>
  ExtOmegaTree<TaskView>::ExtOmegaTree(Region& r, int c0,
                                       const TaskViewArray<TaskView>& t,
                                       int ci0)
    : TaskTree<TaskView,ExtOmegaNode>(r,t), c(c0), ci(ci0) {
    for (int i=tasks.size(); i--; ) {
      leaf(i).e = 0; leaf(i).env = leaf(i).cenv = -Int::Limits::infinity;
    }
    init();
  }

  template<class TaskView>
  forceinline int
  ExtOmegaTree<TaskView>::diff(int i) {
    // Enter task i
    leaf(i).e = tasks[i].e(); 
    leaf(i).env = c*tasks[i].est()+tasks[i].e();
    leaf(i).cenv = (c-ci)*tasks[i].est()+tasks[i].e();
    TaskTree<TaskView,ExtOmegaNode>::update(i);
    // Perform computation of node for task with minest
    int met = 0;
    {
      int e = 0;
      while (!n_leaf(met)) {
        if (node[n_right(met)].cenv + e > (c-ci)*tasks[i].lct()) {
          met = n_right(met);
        } else {
          e += node[n_right(met)].e; met = n_left(met);
        }
      }
    }
    // Now perform split from leaf met upwards
    int aenv, be;
    ExtOmegaNode eon;
    eon.e = 0;
    eon.env = eon.cenv = -Int::Limits::infinity;
    {
      int a = met;
      while (!n_root(a)) {
        int p = n_parent(a);
        if (a == n_left(p)) {
          // a belongs to alpha, n_right(p) not!
          node[p].update(node[n_left(p)],eon);
        } else {
          // both belong to alpha
          node[p].update(node[n_left(p)],node[n_right(p)]);
        }
        a = p;
      }
      aenv = node[0].env;
      // Fix again
      TaskTree<TaskView,ExtOmegaNode>::update(met,false);
    }
    {
      int a = met;
      while (!n_root(a)) {
        int p = n_parent(a);
        if (a == n_left(p)) {
          // n_right(p) belongs to beta, b does not!
          node[p].update(eon,node[n_right(p)]);
        } else {
          // both belong to alpha
          node[p].update(node[n_left(p)],node[n_right(p)]);
        }
        a = p;
      }
      be = node[0].e;
      // Fix again
      TaskTree<TaskView,ExtOmegaNode>::update(met,false);
    }
    int env = aenv + be;
    return
      static_cast<int>(ceil(static_cast<double>(env + (c-ci)*tasks[i].lct()) / c));
  }

  

  /*
   * Omega lambda tree
   */

  forceinline void
  OmegaLambdaNode::init(const OmegaLambdaNode& l, const OmegaLambdaNode& r) {
    OmegaNode::init(l,r);
    le = lenv = -Int::Limits::infinity; res = undef;
  }

  forceinline void
  OmegaLambdaNode::update(const OmegaLambdaNode& l, const OmegaLambdaNode& r) {
    OmegaNode::update(l,r);
    le = std::max(l.le + r.e, l.e + r.le);
    if ((r.lenv >= l.env + r.le) && (r.lenv >= l.lenv + r.e)) {
      lenv = r.lenv; res = r.res;
    } else if (l.env + r.le >= l.lenv + r.e) {
      assert(l.env + r.le > r.lenv);
      lenv = l.env + r.le; res = r.res;
    } else {
      assert((l.lenv + r.e > r.lenv) && (l.lenv + r.e > l.env + r.le));
      lenv = l.lenv + r.e; res = l.res;
    }
  }


  template<class TaskView>
  OmegaLambdaTree<TaskView>::OmegaLambdaTree(Region& r, int c0,
                                             const TaskViewArray<TaskView>& t)
    : TaskTree<TaskView,OmegaLambdaNode>(r,t), c(c0) {
    // Enter all tasks into tree (omega = all tasks, lambda = empty)
    for (int i=tasks.size(); i--; ) {
      leaf(i).e = tasks[i].e();
      leaf(i).le = -Int::Limits::infinity;
      leaf(i).env = c*tasks[i].est()+tasks[i].e();
      leaf(i).lenv = -Int::Limits::infinity;
      leaf(i).res = OmegaLambdaNode::undef;
    }
    init();
  }

  template<class TaskView>
  forceinline void 
  OmegaLambdaTree<TaskView>::shift(int i) {
    // i is in omega
    assert(leaf(i).e > 0);
    leaf(i).le = leaf(i).e;
    leaf(i).e = 0;
    leaf(i).lenv = leaf(i).env;
    leaf(i).env = -Int::Limits::infinity;
    leaf(i).res = i;
    update(i);
  }

  template<class TaskView>
  forceinline void
  OmegaLambdaTree<TaskView>::lremove(int i) {
    // i not in omega but in lambda
    assert((leaf(i).e == 0) && (leaf(i).le > 0));
    leaf(i).le = 0; 
    leaf(i).lenv = -Int::Limits::infinity;
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
  OmegaLambdaTree<TaskView>::env(void) const {
    return root().env;
  }
  
  template<class TaskView>
  forceinline int 
  OmegaLambdaTree<TaskView>::lenv(void) const {
    return root().lenv;
  }
  
}}}

// STATISTICS: scheduling-prop
