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

namespace Gecode { namespace Int { namespace Cumulative {

  template<class ManTask, class Cap, class PL>
  forceinline
  ManProp<ManTask,Cap,PL>::ManProp(Home home, Cap c0, TaskArray<ManTask>& t)
    : TaskProp<ManTask,PL>(home,t), c(c0) {
    c.subscribe(home,*this,PC_INT_BND);
  }

  template<class ManTask, class Cap, class PL>
  forceinline
  ManProp<ManTask,Cap,PL>::ManProp(Space& home, ManProp<ManTask,Cap,PL>& p)
    : TaskProp<ManTask,PL>(home,p) {
    c.update(home,p.c);
  }

  template<class ManTask, class Cap, class PL>
  ExecStatus
  ManProp<ManTask,Cap,PL>::post(Home home, Cap c, TaskArray<ManTask>& t) {
    // Capacity must be nonnegative
    GECODE_ME_CHECK(c.gq(home, 0));
    // Check that tasks do not overload resource
    for (int i=0; i<t.size(); i++)
      if (t[i].c() > c.max())
        return ES_FAILED;
    if (t.size() == 1)
      GECODE_ME_CHECK(c.gq(home, t[0].c()));
    if (t.size() > 1) {
      if (c.assigned() && c.val()==1) {
        TaskArray<typename TaskTraits<ManTask>::UnaryTask> mt(home,t.size());
        for (int i=0; i<t.size(); i++)
          mt[i]=t[i];
        return Unary::ManProp<typename TaskTraits<ManTask>::UnaryTask,PL>
          ::post(home,mt);
      } else {
        (void) new (home) ManProp<ManTask,Cap,PL>(home,c,t);
      }
    }
    return ES_OK;
  }

  template<class ManTask, class Cap, class PL>
  Actor*
  ManProp<ManTask,Cap,PL>::copy(Space& home) {
    return new (home) ManProp<ManTask,Cap,PL>(home,*this);
  }

  template<class ManTask, class Cap, class PL>
  forceinline size_t
  ManProp<ManTask,Cap,PL>::dispose(Space& home) {
    (void) TaskProp<ManTask,PL>::dispose(home);
    c.cancel(home,*this,PC_INT_BND);
    return sizeof(*this);
  }

  template<class ManTask, class Cap, class PL>
  ExecStatus
  ManProp<ManTask,Cap,PL>::propagate(Space& home, const ModEventDelta& med) {
    // Only bounds changes?
    if (IntView::me(med) != ME_INT_DOM)
      GECODE_ES_CHECK(overload(home,c.max(),t));

    if (PL::advanced)
      GECODE_ES_CHECK(edgefinding(home,c.max(),t));

    if (PL::basic)
      GECODE_ES_CHECK(timetabling(home,*this,c,t));

    if (Cap::varderived() && c.assigned() && (c.val() == 1)) {
      // Check that tasks do not overload resource
      for (int i=0; i<t.size(); i++)
        if (t[i].c() > 1)
          return ES_FAILED;
      // Rewrite to unary resource constraint
      TaskArray<typename TaskTraits<ManTask>::UnaryTask> ut(home,t.size());
      for (int i=0; i<t.size(); i++)
        ut[i]=t[i];
      GECODE_REWRITE(*this,
        (Unary::ManProp<typename TaskTraits<ManTask>::UnaryTask,PL>
          ::post(home(*this),ut)));
    }

    if (!PL::basic && c.assigned())
      GECODE_ES_CHECK(subsumed(home,*this,c.val(),t));

    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop
