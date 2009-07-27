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

  // Overload checking for mandatory tasks
  forceinline ExecStatus
  overloaded(Space& home, TaskArray<Task>& t) {
    TaskViewArray<TaskFwd> f(t);
    sort<TaskFwd,STO_LCT,true>(f);

    Region r(home);
    OmegaTree<TaskFwd> o(r,f);

    for (int i=0; i<f.size(); i++) {
      o.insert(i);
      if (o.ect() > f[i].lct())
        return ES_FAILED;
    }
    return ES_OK;
  }
  
  // Overload checking for mandatory optional tasks
  forceinline ExecStatus
  overloaded(Space& home, TaskArray<OptTask>& t) {
    std::cout << "\toverloaded(" << t << std::endl;
    TaskViewArray<OptTaskFwd> f(t);
    sort<OptTaskFwd,STO_LCT,true>(f);

    Region r(home);
    OmegaLambdaTree<OptTaskFwd> ol(r,f,false);

    std::cout << "\t\t";
    for (int i=0; i<f.size(); i++) {
      std::cout << "f[" << i << "] = " << f[i] << ": "; 
      if (f[i].optional()) {
        std::cout << "optional ";
        ol.linsert(i);
      } else if (f[i].mandatory()) {
        std::cout << "mandatory ";
        ol.oinsert(i);
        if (ol.ect() > f[i].lct()) {
          std::cout << "FAILED " << std::endl;
          return ES_FAILED;
        }
      }
      std::cout << std::endl;
      std::cout << "\t\t";
      while (!ol.lempty() && (ol.lect() > f[i].lct())) {
        int j = ol.responsible();
        std::cout << "exclude f[" << j << "] = " << j << std::endl;
        GECODE_ME_CHECK(f[j].excluded(home));
        ol.lremove(j);
      }
    }
    return ES_OK;
  }
  
}}}

// STATISTICS: scheduling-prop
