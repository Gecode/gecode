/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

#ifndef GECODE_SEARCH_WORKER_CONTROL_HH
#define GECODE_SEARCH_WORKER_CONTROL_HH

#include <gecode/search.hh>

namespace Gecode { namespace Search {

  /// Private access to worker-control attachment state
  class GECODE_SEARCH_EXPORT WorkerControlAccess {
  public:
    /// Attach \a control to a leaf engine with fixed \a capacity
    static void attach(WorkerControl& control, unsigned int capacity);
    /// Detach \a control from its leaf engine
    static void detach(WorkerControl& control);
    /// Return the request generation
    static unsigned long long int generation(const WorkerControl& control);
    /// Return whether \a control is engaged
    static bool engaged(const WorkerControl& control);
    /// Return the current request
    static unsigned int requested(const WorkerControl& control);
    /// Return a consistent request and generation snapshot
    static void snapshot(const WorkerControl& control, unsigned int& requested,
                         unsigned long long int& generation);
    /// Wait for the event belonging to worker \a worker
    static void wait(WorkerControl& control, unsigned int worker);
    /// Signal the event belonging to worker \a worker
    static void signal(WorkerControl& control, unsigned int worker);
    /// Signal every worker event
    static void signal_all(WorkerControl& control);
    /// Publish an internal scheduler snapshot
    static void observe(WorkerControl& control,
                        unsigned long long int generation,
                        unsigned int leases, unsigned int parked,
                        unsigned int owners, unsigned int parked_owners);
    /// Record an internal scheduler handoff
    static void handoff(WorkerControl& control);
    /// Return the scheduler's observed request generation
    static unsigned long long int observed_generation(
      const WorkerControl& control);
    /// Return the observed execution lease count
    static unsigned int leases(const WorkerControl& control);
    /// Return the observed parked worker count
    static unsigned int parked(const WorkerControl& control);
    /// Return the observed logical owner count
    static unsigned int owners(const WorkerControl& control);
    /// Return the observed parked logical owner count
    static unsigned int parked_owners(const WorkerControl& control);
    /// Return the observed lease handoff count
    static unsigned long long int handoffs(const WorkerControl& control);
  };

}}

#endif
