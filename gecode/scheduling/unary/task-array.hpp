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

  /*
   * Task array
   */

  forceinline
  TaskArray::TaskArray(void) 
    : n(0), t(NULL) {}
  forceinline
  TaskArray::TaskArray(Space& home, const IntVarArgs& s, const IntArgs& p)
    : n(s.size()), t(home.alloc<Task>(n)) {
    assert((n > 0) && (n == p.size()));
    for (int i=n; i--; )
      t[i].init(s[i],p[i]);
  }
  forceinline
  TaskArray::TaskArray(const TaskArray& a)
    : n(a.n), t(a.t) {}
  forceinline const TaskArray& 
  TaskArray::operator =(const TaskArray& a) {
    n=a.n; t=a.t;
    return *this;
  }

  forceinline int 
  TaskArray::size(void) const {
    return n;
  }

  forceinline Task& 
  TaskArray::operator [](int i) {
    assert((i >= 0) && (i < n));
    return t[i];
  }
  forceinline const Task& 
  TaskArray::operator [](int i) const {
    assert((i >= 0) && (i < n));
    return t[i];
  }

  inline void 
  TaskArray::subscribe(Space& home, Propagator& p) {
    for (int i=n; i--; )
      t[i].subscribe(home,p);
  }

  inline void 
  TaskArray::cancel(Space& home, Propagator& p) {
    for (int i=n; i--; )
      t[i].cancel(home,p);
  }

  forceinline void 
  TaskArray::update(Space& home, bool share, TaskArray& a) {
    n=a.n; t=home.alloc<Task>(n);
    for (int i=n; i--; )
      t[i].update(home,share,a.t[i]);
  }


  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const TaskArray& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (t.size() > 0) {
      s << t[0];
      for (int i=1; i<t.size(); i++)
        s << ", " << t[i];
    }
    s << '}';
    return os << s.str();
  }


  /*
   * Task view array
   */
  template<class TaskView>
  forceinline
  TaskViewArray<TaskView>::TaskViewArray(TaskArray& t0)
    : t(t0) {}

  template<class TaskView>
  forceinline int 
  TaskViewArray<TaskView>::size(void) const {
    return t.size();
  }

  template<class TaskView>
  forceinline TaskView&
  TaskViewArray<TaskView>::operator [](int i) {
    assert((i >= 0) && (i < t.size()));
    return static_cast<TaskView&>(t[i]);
  }
  template<class TaskView>
  forceinline const TaskView&
  TaskViewArray<TaskView>::operator [](int i) const {
    assert((i >= 0) && (i < t.size()));
    return static_cast<const TaskView&>(t[i]);
  }

  template<class Char, class Traits, class TaskView>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, 
              const TaskViewArray<TaskView>& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (t.size() > 0) {
      s << t[0];
      for (int i=1; i<t.size(); i++)
        s << ", " << t[i];
    }
    s << '}';
    return os << s.str();
  }


}}}

// STATISTICS: scheduling-other
