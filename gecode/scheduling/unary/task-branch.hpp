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

#include <gecode/int/linear.hh>

namespace Gecode { namespace Scheduling { namespace Unary {

  template<class Task>
  forceinline
  TaskBranch<Task>::TaskBranch(void) : bif(BIF_NONE) {}

  template<class Task>
  forceinline bool 
  TaskBranch<Task>::first(void) const {
    return (bif & BIF_FIRST) != BIF_NONE;
  }
  
  template<class Task>
  forceinline void 
  TaskBranch<Task>::first(bool b) {
    if (b) {
      bif |= BIF_FIRST;
    } else {
      bif &= ~BIF_FIRST;
    }
  }

  template<class Task>
  forceinline bool 
  TaskBranch<Task>::notfirst(void) const {
    return (bif & BIF_NOTFIRST) != BIF_NONE;
  }
  
  template<class Task>
  forceinline void 
  TaskBranch<Task>::notfirst(bool b) {
    if (b) {
      bif |= BIF_NOTFIRST;
    } else {
      bif &= ~BIF_NOTFIRST;
    }
  }

  template<class Task>
  forceinline bool 
  TaskBranch<Task>::available(void) const {
    return bif == BIF_NONE;
  }
  
  template<class Task>
  forceinline void 
  TaskBranch<Task>::update(Space& home, bool share, TaskBranch<Task>& b) {
    Task::update(home,share,b);
    bif = b.bif;
  }

  template<>
  forceinline ExecStatus
  TaskBranch<ManFixTask>::before(Space& home, TaskBranch<ManFixTask>& t) {
    Int::IntView x(st());
    Int::IntView y(t.st());
    return Int::Linear::LqBin<int,Int::IntView,Int::MinusView>
      ::post(home,x,y,-p());
  }

}}}

// STATISTICS: scheduling-branch
