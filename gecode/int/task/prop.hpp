/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Int {

  template<class Task, class PL>
  forceinline
  TaskProp<Task,PL>::TaskProp(Home home, TaskArray<Task>& t0)
    : Propagator(home), t(t0) {
    t.subscribe(home,*this,PL::pc);
  }

  template<class Task, class PL>
  forceinline
  TaskProp<Task,PL>::TaskProp(Space& home, TaskProp<Task,PL>& p)
    : Propagator(home,p) {
    t.update(home,p.t);
  }

  template<class Task, class PL>
  PropCost
  TaskProp<Task,PL>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,t.size());
  }

  template<class Task, class PL>
  void
  TaskProp<Task,PL>::reschedule(Space& home) {
    t.reschedule(home,*this,PL::pc);
  }

  template<class Task, class PL>
  forceinline size_t
  TaskProp<Task,PL>::dispose(Space& home) {
    t.cancel(home,*this,PL::pc);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}

// STATISTICS: int-prop
