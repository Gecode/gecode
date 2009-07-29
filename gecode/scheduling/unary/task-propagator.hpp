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

  template<class Task>  
  forceinline
  TaskPropagator<Task>::TaskPropagator(Space& home, TaskArray<Task>& t0)
    : Propagator(home), t(t0) {
    t.subscribe(home,*this);
  }

  template<class Task>  
  forceinline
  TaskPropagator<Task>::TaskPropagator(Space& home, 
                                       bool shared, TaskPropagator<Task>& p) 
    : Propagator(home,shared,p) {
    t.update(home,shared,p.t);
  }

  template<class Task>  
  PropCost 
  TaskPropagator<Task>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,t.size());
  }

  template<class Task>  
  size_t 
  TaskPropagator<Task>::dispose(Space& home) {
    t.cancel(home,*this);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  template<class Task>  
  forceinline
  TaskOnePropagator<Task>::TaskOnePropagator(Space& home, TaskArray<Task>& t,
                                             Task& u0)
    : TaskPropagator<Task>(home,t), u(u0) {
    u.subscribe(home,*this);
  }

  template<class Task>  
  forceinline
  TaskOnePropagator<Task>::TaskOnePropagator(Space& home, bool shared, 
                                             TaskOnePropagator<Task>& p) 
    : TaskPropagator<Task>(home,shared,p) {
    u.update(home,shared,p.u);
  }

  template<class Task>  
  PropCost 
  TaskOnePropagator<Task>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,t.size()+1);
  }

  template<class Task>  
  size_t 
  TaskOnePropagator<Task>::dispose(Space& home) {
    u.cancel(home,*this);
    (void) TaskPropagator<Task>::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: scheduling-prop
