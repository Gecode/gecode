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

namespace Gecode { namespace Scheduling { namespace Unary {

  forceinline ExecStatus
  purge(Space& home, Propagator& p, TaskArray<OptTask>& t) {
    int j=0, m=0;
    for (int i=0; i < t.size(); i++)
      if (t[i].excluded()) {
        t[i].cancel(home,p);
      } else {
        if (t[i].mandatory())
          m++;
        t[j++]=t[i];
      }
    t.size(j);
    if (t.size() < 2)
      return ES_SUBSUMED(p,home);
    if (t.size() == m) {
      TaskArray<Task> mt(home,m);
      for (int i=m; i--; )
        mt[i].init(t[i].start(),t[i].p());
      // All tasks are mandatory, rewrite
      GECODE_REWRITE(p,Mandatory::post(home,mt));
    }
    return ES_OK;
  }
  
}}}

// STATISTICS: scheduling-prop
