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
  
  template<class OptTask>
  forceinline
  OptProp<OptTask>::OptProp(Space& home, TaskArray<OptTask>& t)
    : TaskProp<OptTask>(home,t) {}

  template<class OptTask>
  forceinline
  OptProp<OptTask>::OptProp(Space& home, bool shared, OptProp<OptTask>& p) 
    : TaskProp<OptTask>(home,shared,p) {}

  template<class OptTask>
  forceinline ExecStatus 
  OptProp<OptTask>::post(Space& home, TaskArray<OptTask>& t) {
    int m=0, o=0;
    for (int i=t.size(); i--; ) {
      if (t[i].mandatory())
        m++;
      else if (t[i].optional())
        o++;
    }
    if (m == t.size()) {
      TaskArray<typename TaskTraits<OptTask>::ManTask> mt(home,m);
      for (int i=m; i--; )
        mt[i].init(t[i].st(),t[i].p(),t[i].c());
      return ManProp<typename TaskTraits<OptTask>::ManTask>::post(home,mt);
    }
    if (o+m > 1)
      (void) new (home) OptProp<OptTask>(home,t);
    return ES_OK;
  }

  template<class OptTask>
  Actor* 
  OptProp<OptTask>::copy(Space& home, bool share) {
    return new (home) OptProp<OptTask>(home,share,*this);
  }

  template<class OptTask>
  ExecStatus 
  OptProp<OptTask>::propagate(Space& home, const ModEventDelta& med) {
    return ES_NOFIX;
  }

}}}

// STATISTICS: scheduling-prop
