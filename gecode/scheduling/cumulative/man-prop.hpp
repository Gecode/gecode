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

namespace Gecode { namespace Scheduling { namespace Cumulative {
  
  template<class ManTask>
  forceinline
  ManProp<ManTask>::ManProp(Home home, int c0, TaskArray<ManTask>& t)
    : TaskProp<ManTask>(home,t), c(c0) {}

  template<class ManTask>
  forceinline
  ManProp<ManTask>::ManProp(Space& home, bool shared, 
                            ManProp<ManTask>& p) 
    : TaskProp<ManTask>(home,shared,p), c(p.c) {}

  template<class ManTask>
  forceinline ExecStatus 
  ManProp<ManTask>::post(Home home, int c, TaskArray<ManTask>& t) {
    if (t.size() > 1)
      (void) new (home) ManProp<ManTask>(home,c,t);
    return ES_OK;
  }

  template<class ManTask>
  Actor* 
  ManProp<ManTask>::copy(Space& home, bool share) {
    return new (home) ManProp<ManTask>(home,share,*this);
  }

  template<class ManTask>  
  forceinline size_t 
  ManProp<ManTask>::dispose(Space& home) {
    (void) TaskProp<ManTask>::dispose(home);
    return sizeof(*this);
  }

  template<class ManTask>
  ExecStatus 
  ManProp<ManTask>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(overload(home,c,t));
    for (int i=t.size(); i--; )
      if (!t[i].assigned())
        return ES_FIX;
    // Perform definitive overload check
    TaskViewArray<typename TaskTraits<ManTask>::TaskViewFwd> f(t);
    Region r(home);
    TaskViewIter<typename TaskTraits<ManTask>::TaskViewFwd,STO_EST,true> 
      est(r,f);
    TaskViewIter<typename TaskTraits<ManTask>::TaskViewFwd,STO_LCT,true> 
      lct(r,f);
    int t;
    double u=0;
    while (est()) {
      assert(lct());
      t = std::min(f[est.task()].est(),f[lct.task()].lct());
      while (est() && (f[est.task()].est() == t)) {
        u += f[est.task()].c(); ++est;
      }
      while (lct() && (f[lct.task()].lct() == t)) {
        u -= f[lct.task()].c(); ++lct;
      }
      if (u > static_cast<double>(c))
        return ES_FAILED;
    }
    return ES_SUBSUMED(*this,home);
  }

}}}

// STATISTICS: scheduling-prop
