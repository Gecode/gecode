/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *     Guido Tack, 2010
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

namespace Gecode { namespace Int { namespace Cumulative {

  template<class OptTask, class Cap, class PL>
  forceinline
  OptProp<OptTask,Cap,PL>::OptProp(Home home, Cap c0, TaskArray<OptTask>& t)
    : TaskProp<OptTask,PL>(home,t), c(c0) {
    c.subscribe(home,*this,PC_INT_BND);
  }

  template<class OptTask, class Cap, class PL>
  forceinline
  OptProp<OptTask,Cap,PL>::OptProp(Space& home, OptProp<OptTask,Cap,PL>& p)
    : TaskProp<OptTask,PL>(home,p) {
    c.update(home,p.c);
  }

  template<class OptTask, class Cap, class PL>
  ExecStatus
  OptProp<OptTask,Cap,PL>::post(Home home, Cap c, TaskArray<OptTask>& t) {
    // Capacity must be nonnegative
    GECODE_ME_CHECK(c.gq(home, 0));
    // Check for overload by single task and remove excluded tasks
    int n=t.size(), m=0;
    for (int i=n; i--; ) {
      if (t[i].c() > c.max())
        GECODE_ME_CHECK(t[i].excluded(home));
      if (t[i].excluded())
        t[i]=t[--n];
      else if (t[i].mandatory())
        m++;
    }
    t.size(n);
    if (t.size() < 2) {
      if (t.size() == 1) {
        if (t[0].mandatory()) {
          GECODE_ME_CHECK(c.gq(home, t[0].c()));
          return ES_OK;
        } else if (c.min() >= t[0].c()) {
          return ES_OK;
        }
      } else {
        return ES_OK;
      }
    }
    if (c.assigned() && (c.val() == 1)) {
      TaskArray<typename TaskTraits<OptTask>::UnaryTask> mt(home,t.size());
      for (int i=0; i<t.size(); i++)
        mt[i]=t[i];
      return Unary::OptProp<typename TaskTraits<OptTask>::UnaryTask,PL>
        ::post(home,mt);
    }
    if (m == t.size()) {
      TaskArray<typename TaskTraits<OptTask>::ManTask> mt(home,m);
      for (int i=0; i<m; i++)
        mt[i].init(t[i]);
      return ManProp<typename TaskTraits<OptTask>::ManTask,Cap,PL>
        ::post(home,c,mt);
    }
    (void) new (home) OptProp<OptTask,Cap,PL>(home,c,t);
    return ES_OK;
  }

  template<class OptTask, class Cap, class PL>
  Actor*
  OptProp<OptTask,Cap,PL>::copy(Space& home) {
    return new (home) OptProp<OptTask,Cap,PL>(home,*this);
  }

  template<class OptTask, class Cap, class PL>
  forceinline size_t
  OptProp<OptTask,Cap,PL>::dispose(Space& home) {
    (void) TaskProp<OptTask,PL>::dispose(home);
    c.cancel(home,*this,PC_INT_BND);
    return sizeof(*this);
  }

  template<class OptTask, class Cap, class PL>
  ExecStatus
  OptProp<OptTask,Cap,PL>::propagate(Space& home, const ModEventDelta& med) {
    // Did one of the Boolean views change?
    if (BoolView::me(med) == ME_BOOL_VAL)
      GECODE_ES_CHECK((purge<OptTask,PL>(home,*this,t,c)));

    // Only bounds changes?
    if (IntView::me(med) != ME_INT_DOM)
      GECODE_ES_CHECK(overload(home,c.max(),t));

    if (PL::basic)
      GECODE_ES_CHECK(timetabling(home,*this,c,t));

    if (PL::advanced) {
      // Partition into mandatory and optional activities
      int n = t.size();
      int i=0, j=n-1;
      while (true) {
        while ((i < n) && t[i].mandatory()) i++;
        while ((j >= 0) && !t[j].mandatory()) j--;
        if (i >= j) break;
        std::swap(t[i],t[j]);
      }

      if (i > 1) {
        // Truncate array to only contain mandatory tasks
        t.size(i);
        GECODE_ES_CHECK(edgefinding(home,c.max(),t));
        // Restore to also include optional tasks
        t.size(n);
      }
    }

    if (Cap::varderived() && c.assigned() && c.val()==1) {
      // Check that tasks do not overload resource
      for (int i=0; i<t.size(); i++)
        if (t[i].c() > 1)
          GECODE_ME_CHECK(t[i].excluded(home));
      // Rewrite to unary resource constraint
      TaskArray<typename TaskTraits<OptTask>::UnaryTask> ut(home,t.size());
      for (int i=0; i<t.size(); i++)
        ut[i]=t[i];
      GECODE_REWRITE(*this,
                     (Unary::OptProp<typename TaskTraits<OptTask>::UnaryTask,PL>
                      ::post(home(*this),ut)));
    }

    if (!PL::basic && c.assigned())
      GECODE_ES_CHECK(subsumed(home,*this,c.val(),t));

    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop
