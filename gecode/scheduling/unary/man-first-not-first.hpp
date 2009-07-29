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
  
  template<class ManTask>
  forceinline
  ManFirst<ManTask>::ManFirst(Space& home, TaskArray<ManTask>& t, ManTask& u)
    : TaskOnePropagator<ManTask>(home,t,u) {}

  template<class ManTask>
  forceinline
  ManFirst<ManTask>::ManFirst(Space& home, bool shared, 
                              ManFirst<ManTask>& p) 
    : TaskOnePropagator<ManTask>(home,shared,p) {}

  template<class ManTask>
  forceinline ExecStatus 
  ManFirst<ManTask>::post(Space& home, TaskArray<ManTask>& t, ManTask& u) {
    if (t.size() == 1)
      return manbefore(home, u, t[0]);
    if (t.size() > 1)
      (void) new (home) ManFirst<ManTask>(home,t,u);
    return ES_OK;
  }

  template<class ManTask>
  Actor* 
  ManFirst<ManTask>::copy(Space& home, bool share) {
    return new (home) ManFirst<ManTask>(home,share,*this);
  }

  template<class ManTask>
  ExecStatus 
  ManFirst<ManTask>::propagate(Space& home, const ModEventDelta&) {
    int n=t.size();
    for (int i=n; i--; )
      if (u.lct() <= t[i].est()) {
        t[i].cancel(home,*this); t[i]=t[--n];
      } else {
        GECODE_ME_CHECK(u.lct(home,t[i].lst()));
        GECODE_ME_CHECK(t[i].est(home,u.ect()));
      }
    t.size(n);
    return (t.size() == 0) ? ES_SUBSUMED(*this,home) : ES_FIX;
  }


  template<class ManTask>
  forceinline
  ManNotFirst<ManTask>::ManNotFirst(Space& home, 
                                    TaskArray<ManTask>& t, ManTask& u)
    : TaskOnePropagator<ManTask>(home,t,u) {}

  template<class ManTask>
  forceinline
  ManNotFirst<ManTask>::ManNotFirst(Space& home, bool shared, 
                              ManNotFirst<ManTask>& p) 
    : TaskOnePropagator<ManTask>(home,shared,p) {}

  template<class ManTask>
  forceinline ExecStatus 
  ManNotFirst<ManTask>::post(Space& home, TaskArray<ManTask>& t, ManTask& u) {
    if (t.size() == 1)
      return manbefore(home, t[0], u);
    if (t.size() > 1)
      (void) new (home) ManNotFirst<ManTask>(home,t,u);
    return ES_OK;
  }

  template<class ManTask>
  Actor* 
  ManNotFirst<ManTask>::copy(Space& home, bool share) {
    return new (home) ManNotFirst<ManTask>(home,share,*this);
  }

  template<class ManTask>
  ExecStatus 
  ManNotFirst<ManTask>::propagate(Space& home, const ModEventDelta&) {
    int ect = t[t.size()-1].ect();
    int lct = t[t.size()-1].lct();
    for (int i=t.size()-1; i--; ) {
      ect = std::min(ect,t[i].ect());
      lct = std::max(ect,t[i].lct());
    }

    GECODE_ME_CHECK(u.est(home,ect));
    return (lct <= u.est()) ? ES_SUBSUMED(*this,home) : ES_FIX;
  }

}}}

// STATISTICS: scheduling-prop
