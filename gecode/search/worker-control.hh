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

namespace Gecode { namespace Search {

  /// Private access to worker-control attachment state
  class GECODE_SEARCH_EXPORT WorkerControlAccess {
  public:
    /// Test boundary in adjustable parallel scheduling
    enum Gate {
      GATE_ADMISSION,
      GATE_EVENT_WAIT,
      GATE_FAILED_SCAN,
      GATE_ACTION_BEGIN,
      GATE_COUNT
    };
    /// Select every worker for a test boundary gate
    static const unsigned int ALL_WORKERS = static_cast<unsigned int>(-1);
    /// Attach \a control to a leaf engine with fixed \a capacity
    static void attach(WorkerControl& control, unsigned int capacity);
    /// Detach \a control from its leaf engine
    static void detach(WorkerControl& control);
    /// Return the request generation
    static unsigned long long int generation(const WorkerControl& control);
    /// Return whether \a control is engaged
    static bool engaged(const WorkerControl& control);
    /// Return whether \a control is currently attached to a leaf engine
    static bool attached(const WorkerControl& control);
    /// Return whether \a x and \a y share one control identity
    static bool same_identity(const WorkerControl& x,
                              const WorkerControl& y);
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
    /// Record the state of worker \a worker
    static void observe_worker(WorkerControl& control, unsigned int worker,
                               unsigned int logical, bool lease, bool parked);
    /// Record an internal scheduler handoff
    static void handoff(WorkerControl& control, unsigned int from,
                        unsigned int to);
    /// Record which worker produced a solution
    static void solution(WorkerControl& control, unsigned int worker);
    /// Record incumbent delivery to worker \a worker
    static void incumbent(WorkerControl& control, unsigned int worker);
    /// Pause a worker at an installed internal test boundary
    static void gate(WorkerControl& control, Gate gate, unsigned int worker);
    /// Install an internal test boundary gate
    static void gate_install(WorkerControl& control, Gate gate,
                             unsigned int worker, unsigned int expected);
    /// Wait until the installed gate has reached its expected count
    static void gate_wait(WorkerControl& control, Gate gate);
    /// Release one worker from an installed gate
    static void gate_release(WorkerControl& control, Gate gate,
                             unsigned int worker);
    /// Release all workers and disable an installed gate
    static void gate_release_all(WorkerControl& control, Gate gate);
    /// Return whether worker \a worker is stopped at gate \a gate
    static bool gate_waiting(const WorkerControl& control, Gate gate,
                             unsigned int worker);
    /// Begin one admitted parallel-search action
    static void action_begin(
      WorkerControl& control, unsigned int worker,
      const std::atomic<unsigned long long int>& generation);
    /// End one admitted parallel-search action
    static void action_end(WorkerControl& control, unsigned int worker);
    /// Reset the admitted-action high-water mark
    static void reset_max_admitted(WorkerControl& control);
    /// Reset the high-water mark for actions admitted in \a generation
    static void reset_max_admitted(WorkerControl& control,
                                   unsigned long long int generation);
    /// Return the current admitted-action count
    static unsigned int admitted(const WorkerControl& control);
    /// Return the admitted-action high-water mark
    static unsigned int max_admitted(const WorkerControl& control);
    /// Return the newest generation with a completed admitted action
    static unsigned long long int completed_generation(
      const WorkerControl& control);
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
    /// Return the source worker of the last handoff
    static unsigned int last_handoff_from(const WorkerControl& control);
    /// Return the target worker of the last handoff
    static unsigned int last_handoff_to(const WorkerControl& control);
    /// Return the worker that most recently produced a solution
    static unsigned int last_solution_worker(const WorkerControl& control);
    /// Return the handoff source observed when the last solution was produced
    static unsigned int solution_handoff_from(const WorkerControl& control);
    /// Return the handoff target observed when the last solution was produced
    static unsigned int solution_handoff_to(const WorkerControl& control);
    /// Return whether worker \a worker is a logical owner
    static bool owner(const WorkerControl& control, unsigned int worker);
    /// Return whether worker \a worker holds an execution lease
    static bool leased(const WorkerControl& control, unsigned int worker);
    /// Return whether worker \a worker is parked
    static bool worker_parked(const WorkerControl& control,
                              unsigned int worker);
    /// Return incumbent deliveries observed for worker \a worker
    static unsigned long long int incumbent_deliveries(
      const WorkerControl& control, unsigned int worker);
  };

}}

#endif
