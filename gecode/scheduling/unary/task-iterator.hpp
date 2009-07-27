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

  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline
  TaskIterator<TaskView,sto,inc>
  ::TaskIterator(Region& r, 
                 const TaskViewArray<TaskView>& t)
    : map(r.alloc<int>(t.size())), i(t.size()-1) {
    sort<TaskView,sto,!inc>(map,t);
  }

  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline bool
  TaskIterator<TaskView,sto,inc>::operator ()(void) const {
    return i>=0;
  }
  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline int
  TaskIterator<TaskView,sto,inc>::left(void) const {
    return i+1;
  }
  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline void
  TaskIterator<TaskView,sto,inc>::operator ++(void) {
    i--;
  }

  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline int
  TaskIterator<TaskView,sto,inc>::task(void) const {
    return map[i];
  }


  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline
  MandatoryTaskIterator<TaskView,sto,inc>
  ::MandatoryTaskIterator(Region& r, 
                          const TaskViewArray<TaskView>& t)
    : TaskIterator<TaskView,sto,inc>(r,t), tasks(t), m(0) {
    for (int j=tasks.size(); j--; )
      if (tasks[i].mandatory())
        m++;
    while (TaskIterator<TaskView,sto,inc>::operator ()() &&
           !tasks[map[i]].mandatory())
      TaskIterator<TaskView,sto,inc>::operator ++();
  }

  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline bool
  MandatoryTaskIterator<TaskView,sto,inc>::operator ()(void) const {
    return m>0;
  }
  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline int
  MandatoryTaskIterator<TaskView,sto,inc>::left(void) const {
    return m;
  }
  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline void
  MandatoryTaskIterator<TaskView,sto,inc>::operator ++(void) {
    m--;
    while (TaskIterator<TaskView,sto,inc>::operator ()() &&
           !tasks[map[i]].mandatory())
      TaskIterator<TaskView,sto,inc>::operator ++();
  }

}}}

// STATISTICS: scheduling-prop
