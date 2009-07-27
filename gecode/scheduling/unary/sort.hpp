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

#include <gecode/scheduling/unary.hh>

namespace Gecode { namespace Scheduling { namespace Unary {

  /// Sort by non-decreasing earliest start times
  template<class TaskView>
  class IncEst {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-increasing earliest start times
  template<class TaskView>
  class DecEst {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-decreasing earliest completion times
  template<class TaskView>
  class IncEct {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-increasing earliest completion times
  template<class TaskView>
  class DecEct {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-decreasing latest start times
  template<class TaskView>
  class IncLst {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-increasing latest start times
  template<class TaskView>
  class DecLst {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-decreasing latest completion times
  template<class TaskView>
  class IncLct {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sort by non-increasing latest completion times
  template<class TaskView>
  class DecLct {
  public:
    /// Sort order
    bool operator ()(const TaskView& t1, const TaskView& t2) const;
  };

  /// Sorting maps rather than tasks
  template<class TaskView, template<class> class STO>
  class SortMap {
  private:
    /// The tasks
    const TaskViewArray<TaskView>& tasks;
    /// The sorting order for tasks
    STO<TaskView> sto;
  public:
    /// Initialize
    SortMap(const TaskViewArray<TaskView>& t);
    /// Sort order
    bool operator ()(int& i, int& j) const;
  };

  template<class TaskView>
  forceinline bool
  IncEst<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.est() < t2.est();
  }

  template<class TaskView>
  forceinline bool
  DecEst<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.est() > t2.est();
  }

  template<class TaskView>
  forceinline bool
  IncEct<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.ect() < t2.ect();
  }

  template<class TaskView>
  forceinline bool
  DecEct<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.ect() > t2.ect();
  }

  template<class TaskView>
  forceinline bool
  IncLst<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.lst() < t2.lst();
  }

  template<class TaskView>
  forceinline bool
  DecLst<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.lst() > t2.lst();
  }

  template<class TaskView>
  forceinline bool
  IncLct<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.lct() < t2.lct();
  }

  template<class TaskView>
  forceinline bool
  DecLct<TaskView>::operator ()(const TaskView& t1, const TaskView& t2) const {
    return t1.lct() > t2.lct();
  }


  template<class TaskView, template<class> class STO>
  forceinline
  SortMap<TaskView,STO>::SortMap(const TaskViewArray<TaskView>& t) 
    : tasks(t) {}
  template<class TaskView, template<class> class STO>
  forceinline bool 
  SortMap<TaskView,STO>::operator ()(int& i, int& j) const {
    return sto(tasks[i],tasks[j]);
  }

  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline void 
  sort(TaskViewArray<TaskView>& t) {
    switch (sto) {
    case STO_EST:
      if (inc) {
        IncEst<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      } else {
        DecEst<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      }
      break;
    case STO_ECT:
      if (inc) {
        IncEct<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      } else {
        DecEct<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      }
      break;
    case STO_LST:
      if (inc) {
        IncLst<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      } else {
        DecLst<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      }
      break;
    case STO_LCT:
      if (inc) {
        IncLct<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      } else {
        DecLct<TaskView> o; Support::quicksort(&t[0], t.size(), o);
      }
      break;
    default:
      GECODE_NEVER;
    }
  }

  template<class TaskView, SortTaskOrder sto, bool inc>
  forceinline void
  sort(int* map, const TaskViewArray<TaskView>& t) {
    for (int i=t.size(); i--; )
      map[i]=i;
    switch (sto) {
    case STO_EST:
      if (inc) {
        SortMap<TaskView,IncEst> o(t); Support::quicksort(map, t.size(), o);
      } else {
        SortMap<TaskView,DecEst> o(t); Support::quicksort(map, t.size(), o);
      }
      break;
    case STO_ECT:
      if (inc) {
        SortMap<TaskView,IncEct> o(t); Support::quicksort(map, t.size(), o);
      } else {
        SortMap<TaskView,DecEct> o(t); Support::quicksort(map, t.size(), o);
      }
      break;
    case STO_LST:
      if (inc) {
        SortMap<TaskView,IncLst> o(t); Support::quicksort(map, t.size(), o);
      } else {
        SortMap<TaskView,DecLst> o(t); Support::quicksort(map, t.size(), o);
      }
      break;
    case STO_LCT:
      if (inc) {
        SortMap<TaskView,IncLct> o(t); Support::quicksort(map, t.size(), o);
      } else {
        SortMap<TaskView,DecLct> o(t); Support::quicksort(map, t.size(), o);
      }
      break;
    default:
      GECODE_NEVER;
    }
  }

}}}

// STATISTICS: scheduling-other
