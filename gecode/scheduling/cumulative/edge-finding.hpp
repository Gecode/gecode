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

  template<class TaskView>
  forceinline ExecStatus
  edgefinding(Space& home, int c, TaskViewArray<TaskView>& t) {
    sort<TaskView,STO_LCT,true>(t);

    Region r(home);

    // Detection
    int* prec = r.alloc<int>(t.size());
    for (int i=t.size(); i--; )
      prec[i] = -Int::Limits::infinity;

    OmegaLambdaTree<TaskView> ol(r,c,t);

    for (int j=0; j<t.size(); j++) {
      while (!ol.lempty() && (ol.env() > c*t[j].lct())) {
        int i = ol.responsible();
        prec[i] = t[j].lct();
        ol.lremove(i);
      }
      ol.shift(j);
    }
    for (int i=t.size(); i--; )
      prec[i] = std::max(prec[i],t[i].ect());

    // Propagation (very very naive so far)
    int* update = r.alloc<int>(t.size()*t.size());

    for (int i=0; i<t.size(); i++) {
      ExtOmegaTree<TaskView> eo(r,c,t,t[i].c());
      int u = -Int::Limits::infinity;
      for (int j=0; j<t.size(); j++) {
        u = std::max(u,eo.diff(j));
        update[i*t.size()+j]=u;
      }
    }
    
    /*
    for (int i=0; i<t.size(); i++)
      for (int j=0; j<t.size(); j++)
        if (t[j].lct() <= prec[i]) {
          std::cout << "t[" << i << "].est()=" << t[i].est()
                    << " >= " << update[i*t.size()+j] << std::endl;
          GECODE_ME_CHECK(t[i].est(home,));
        }
    */
  
    return ES_OK;
  }

  template<class Task>
  ExecStatus
  edgefinding(Space& home, int c, TaskArray<Task>& t) {
    TaskViewArray<typename TaskTraits<Task>::TaskViewFwd> f(t);
    GECODE_ES_CHECK(edgefinding(home,c,f));
    TaskViewArray<typename TaskTraits<Task>::TaskViewBwd> b(t);
    return edgefinding(home,c,b);
  }
    
}}}

// STATISTICS: scheduling-prop
