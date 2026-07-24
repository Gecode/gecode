/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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
 *
 */

namespace Gecode { namespace Search { namespace Par {


  /*
   * Basic access routines
   */
  template<class Tracer>
  forceinline Engine<Tracer>&
  Engine<Tracer>::Worker::engine(void) const {
    return _engine;
  }
  template<class Tracer>
  forceinline const Options&
  Engine<Tracer>::opt(void) const {
    return _opt;
  }
  template<class Tracer>
  forceinline unsigned int
  Engine<Tracer>::workers(void) const {
    return static_cast<unsigned int>(opt().threads);
  }
  template<class Tracer>
  forceinline bool
  Engine<Tracer>::stopped(void) const {
    return has_stopped.load(std::memory_order_acquire);
  }



  /*
   * Engine: command and wait handling
   */
  template<class Tracer>
  forceinline typename Engine<Tracer>::Cmd
  Engine<Tracer>::cmd(void) const {
    return _cmd.load(std::memory_order_acquire);
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::block(void) {
    _cmd.store(C_WAIT, std::memory_order_release);
    Support::Thread::acquireGlobalMutex(&_m_wait);
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::release(Cmd c) {
    _cmd.store(c, std::memory_order_release);
    if (scheduler_enabled && ((c == C_WORK) || (c == C_RESET) ||
                              (c == C_TERMINATE)))
      WorkerControlAccess::signal_all(worker_control);
    Support::Thread::releaseGlobalMutex(&_m_wait);
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::wait(void) {
    _m_wait.acquire(); _m_wait.release();
  }


  /*
   * Engine: initialization
   */
  template<class Tracer>
  forceinline
  Engine<Tracer>::Worker::Worker(Space* s, Engine& e)
    : tracer(e.opt().tracer), _engine(e),
      path(s == nullptr ? 0 : e.opt().nogoods_limit), d(0),
      idle(false) {
    tracer.worker();
    if (s != nullptr) {
      if (s->status(*this) == SS_FAILED) {
        fail++;
        cur = nullptr;
        if (!engine().opt().clone)
          delete s;
      } else {
        cur = snapshot(s,engine().opt());
      }
    } else {
      cur = nullptr;
    }
  }

  template<class Tracer>
  forceinline
  Engine<Tracer>::Engine(const Options& o)
    : Search::Engine(o,static_cast<unsigned int>(o.threads)),
      _opt(o), scheduler_enabled(false),
      scheduler_worker(nullptr), scheduler_requested(workers()),
      scheduler_leases(0U), scheduler_cursor(0U), scheduler_generation(0U),
      _cmd(C_WAIT), solutions(heap) {
    // Initialize termination information
    _n_term_not_ack = workers();
    _n_not_terminated = workers();
    // Initialize search information
    n_busy = workers();
    has_stopped.store(false, std::memory_order_release);
    // Initialize reset information
    _n_reset_not_ack = workers();
  }

  template<class Tracer>
  unsigned int
  Engine<Tracer>::scheduler_select(SchedulerLogical logical,
                                    unsigned int exclude) const {
    for (unsigned int offset=0U; offset<workers(); offset++) {
      unsigned int i = (scheduler_cursor + offset) % workers();
      if ((i != exclude) && !scheduler_worker[i].lease &&
          scheduler_worker[i].parked &&
          (scheduler_worker[i].logical == logical))
        return i;
    }
    return workers();
  }

  template<class Tracer>
  bool
  Engine<Tracer>::scheduler_grow(void) {
    bool wake = false;
    while (scheduler_leases < scheduler_requested) {
      unsigned int i = scheduler_select(SL_OWNER,workers());
      if (i == workers())
        i = scheduler_select(SL_PENDING,workers());
      if (i == workers())
        i = scheduler_select(SL_IDLE,workers());
      if (i == workers()) {
        for (i=0U; i<workers(); i++)
          if (!scheduler_worker[i].lease)
            break;
      }
      if (i == workers())
        break;
      scheduler_worker[i].lease = true;
      scheduler_worker[i].parked = false;
      scheduler_leases++;
      scheduler_cursor = (i+1U) % workers();
      wake = true;
    }
    return wake;
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_observe(void) {
    unsigned int parked = 0U;
    unsigned int owners = 0U;
    unsigned int parked_owners = 0U;
    for (unsigned int i=0U; i<workers(); i++) {
      if (scheduler_worker[i].parked)
        parked++;
      if (scheduler_worker[i].logical == SL_OWNER) {
        owners++;
        if (scheduler_worker[i].parked)
          parked_owners++;
      }
    }
    WorkerControlAccess::observe(
      worker_control,
      scheduler_generation.load(std::memory_order_relaxed),
      scheduler_leases,parked,owners,parked_owners);
    for (unsigned int i=0U; i<workers(); i++)
      WorkerControlAccess::observe_worker(
        worker_control,i,
        static_cast<unsigned int>(scheduler_worker[i].logical),
        scheduler_worker[i].lease,scheduler_worker[i].parked);
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_enable(bool root_owner) {
    if (!WorkerControlAccess::engaged(worker_control))
      return;
    scheduler_worker = static_cast<SchedulerWorker*>
      (heap.ralloc(workers() * sizeof(SchedulerWorker)));
    unsigned long long int generation;
    WorkerControlAccess::snapshot(
      worker_control,scheduler_requested,generation);
    scheduler_leases = scheduler_requested;
    for (unsigned int i=0U; i<workers(); i++) {
      scheduler_worker[i].lease = i < scheduler_requested;
      scheduler_worker[i].parked = false;
      scheduler_worker[i].logical =
        ((i == 0U) && root_owner) ? SL_OWNER : SL_PENDING;
    }
    scheduler_generation.store(generation,std::memory_order_release);
    scheduler_observe();
    scheduler_enabled = true;
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_reset(bool root_owner) {
    if (!scheduler_enabled)
      return;
    unsigned int requested;
    unsigned long long int generation;
    WorkerControlAccess::snapshot(worker_control,requested,generation);
    scheduler_mutex.acquire();
    scheduler_requested = requested;
    scheduler_leases = scheduler_requested;
    scheduler_cursor = 0U;
    for (unsigned int i=0U; i<workers(); i++) {
      scheduler_worker[i].lease = i < scheduler_requested;
      scheduler_worker[i].parked = false;
      scheduler_worker[i].logical =
        ((i == 0U) && root_owner) ? SL_OWNER : SL_PENDING;
    }
    scheduler_generation.store(generation,std::memory_order_release);
    scheduler_observe();
    scheduler_mutex.release();
  }

  template<class Tracer>
  bool
  Engine<Tracer>::scheduler_admit(unsigned int worker) {
    if (!scheduler_enabled)
      return true;
    WorkerControlAccess::gate(worker_control,
                              WorkerControlAccess::GATE_ADMISSION,worker);
    while (cmd() == C_WORK) {
      unsigned long long int generation =
        WorkerControlAccess::generation(worker_control);
      if ((generation ==
           scheduler_generation.load(std::memory_order_acquire)) &&
          (WorkerControlAccess::requested(worker_control) == workers()))
        return true;

      bool wake;
      bool admitted;
      unsigned int requested;
      WorkerControlAccess::snapshot(worker_control,requested,generation);
      scheduler_mutex.acquire();
      if (generation >
          scheduler_generation.load(std::memory_order_relaxed)) {
        scheduler_requested = requested;
        scheduler_generation.store(generation,std::memory_order_release);
      }
      wake = scheduler_grow();
      if (scheduler_worker[worker].lease &&
          (scheduler_leases > scheduler_requested)) {
        scheduler_worker[worker].lease = false;
        scheduler_leases--;
      }
      admitted = scheduler_worker[worker].lease;
      scheduler_worker[worker].parked = !admitted;
      scheduler_observe();
      scheduler_mutex.release();

      if (wake)
        WorkerControlAccess::signal_all(worker_control);
      if (admitted)
        return true;
      WorkerControlAccess::gate(worker_control,
                                WorkerControlAccess::GATE_EVENT_WAIT,worker);
      WorkerControlAccess::wait(worker_control,worker);
    }
    return false;
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::scheduler_action_begin(unsigned int worker) {
    if (scheduler_enabled)
      WorkerControlAccess::action_begin(
        worker_control,worker,scheduler_generation);
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::scheduler_action_end(unsigned int worker) {
    if (scheduler_enabled)
      WorkerControlAccess::action_end(worker_control,worker);
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::scheduler_failed_scan(unsigned int worker) {
    if (scheduler_enabled)
      WorkerControlAccess::gate(worker_control,
                                WorkerControlAccess::GATE_FAILED_SCAN,worker);
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_owner(unsigned int worker) {
    if (!scheduler_enabled)
      return;
    scheduler_mutex.acquire();
    scheduler_worker[worker].logical = SL_OWNER;
    scheduler_observe();
    scheduler_mutex.release();
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_idle(unsigned int worker) {
    if (!scheduler_enabled)
      return;
    scheduler_mutex.acquire();
    scheduler_worker[worker].logical = SL_IDLE;
    scheduler_observe();
    scheduler_mutex.release();
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_solution(unsigned int worker) {
    if (scheduler_enabled)
      WorkerControlAccess::solution(worker_control,worker);
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::scheduler_incumbent(unsigned int worker) {
    if (scheduler_enabled)
      WorkerControlAccess::incumbent(worker_control,worker);
  }

  template<class Tracer>
  void
  Engine<Tracer>::scheduler_handoff(unsigned int worker,
                                    bool work_remains) {
    if (!scheduler_enabled)
      return;
    unsigned int target = workers();
    scheduler_mutex.acquire();
    if (scheduler_worker[worker].lease) {
      if (scheduler_leases > scheduler_requested) {
        scheduler_worker[worker].lease = false;
        scheduler_worker[worker].parked = true;
        scheduler_leases--;
      } else if (work_remains) {
        target = scheduler_select(SL_OWNER,worker);
        if (target == workers())
          target = scheduler_select(SL_PENDING,worker);
        if (target != workers()) {
          scheduler_worker[worker].lease = false;
          scheduler_worker[worker].parked = true;
          scheduler_worker[target].lease = true;
          scheduler_worker[target].parked = false;
          scheduler_cursor = (target+1U) % workers();
          WorkerControlAccess::handoff(worker_control,worker,target);
        }
      }
    }
    scheduler_observe();
    scheduler_mutex.release();
    if (target != workers())
      WorkerControlAccess::signal(worker_control,target);
  }

  /*
   * Statistics
   */
  template<class Tracer>
  forceinline Statistics
  Engine<Tracer>::Worker::statistics(void) {
    m.acquire();
    Statistics s = *this;
    m.release();
    return s;
  }


  /*
   * Engine: search control
   */
  template<class Tracer>
  forceinline bool
  Engine<Tracer>::signal(void) const {
    return solutions.empty() && (n_busy > 0) &&
      !has_stopped.load(std::memory_order_acquire);
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::idle(void) {
    m_search.acquire();
    bool bs = signal();
    n_busy--;
    if (bs && (n_busy == 0))
      e_search.signal();
    m_search.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::busy(void) {
    m_search.acquire();
    assert(n_busy > 0);
    n_busy++;
    m_search.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::stop(void) {
    m_search.acquire();
    bool bs = signal();
    has_stopped.store(true, std::memory_order_release);
    if (bs)
      e_search.signal();
    m_search.release();
  }

  template<class Tracer>
  forceinline bool
  Engine<Tracer>::work_remains(void) {
    m_search.acquire();
    bool remains = n_busy > 0;
    m_search.release();
    return remains;
  }


  /*
   * Engine: termination control
   */
  template<class Tracer>
  forceinline void
  Engine<Tracer>::terminated(void) {
    unsigned int n;
    _m_term.acquire();
    n = --_n_not_terminated;
    _m_term.release();
    // The signal must be outside of the look, otherwise a thread might be
    // terminated that still holds a mutex.
    if (n == 0)
      _e_terminate.signal();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::ack_terminate(void) {
    _m_term.acquire();
    if (--_n_term_not_ack == 0)
      _e_term_ack.signal();
    _m_term.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::wait_terminate(void) {
    _m_wait_terminate.acquire();
    _m_wait_terminate.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::terminate(void) {
    // Grab the wait mutex for termination
    _m_wait_terminate.acquire();
    // Release all threads
    release(C_TERMINATE);
    // Wait until all threads have acknowledged termination request
    _e_term_ack.wait();
    // Release waiting threads
    _m_wait_terminate.release();
    // Wait until all threads have in fact terminated
    _e_terminate.wait();
    // Now all threads are terminated!
  }

  /*
   * Engine: reset control
   */
  template<class Tracer>
  forceinline void
  Engine<Tracer>::ack_reset_start(void) {
    _m_reset.acquire();
    if (--_n_reset_not_ack == 0)
      e_reset_ack_start.signal();
    _m_reset.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::ack_reset_stop(void) {
    _m_reset.acquire();
    if (++_n_reset_not_ack == workers())
      e_reset_ack_stop.signal();
    _m_reset.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::wait_reset(void) {
    m_wait_reset.acquire();
    m_wait_reset.release();
  }



  /*
   * Worker: finding and stealing working
   */
  template<class Tracer>
  forceinline Space*
  Engine<Tracer>::Worker::steal(unsigned long int& d, 
                                Tracer& myt, Tracer& ot) {
    /*
     * Make a quick check whether the worker might have work
     *
     * If that is not true any longer, the worker will be asked
     * again eventually.
     */
    m.acquire();
    Space* s = path.steal() ? path.steal(*this,d,myt,ot) : nullptr;
    m.release();
    // Tell that there will be one more busy worker
    if (s != nullptr)
      engine().busy();
    return s;
  }

  /*
   * Return No-Goods
   */
  template<class Tracer>
  forceinline NoGoods&
  Engine<Tracer>::Worker::nogoods(void) {
    return path;
  }

  /*
   * Engine: search control
   */
  template<class Tracer>
  Space*
  Engine<Tracer>::next(void) {
    // Invariant: the worker holds the wait mutex
    m_search.acquire();
    if (!solutions.empty()) {
      // No search needs to be done, take leftover solution
      Space* s = solutions.pop();
      m_search.release();
      return s;
    }
    // We ignore stopped (it will be reported again if needed)
    has_stopped.store(false, std::memory_order_release);
    // No more solutions?
    if (n_busy == 0) {
      m_search.release();
      return nullptr;
    }
    m_search.release();
    // Okay, now search has to continue, make the guys work
    release(C_WORK);

    /*
     * Wait until a search related event has happened. It might be that
     * the event has already been signalled in the last run, but the
     * solution has been removed. So we have to try until there has
     * something new happened.
     */
    while (true) {
      e_search.wait();
      m_search.acquire();
      if (!solutions.empty()) {
        // Report solution
        Space* s = solutions.pop();
        m_search.release();
        // Make workers wait again
        block();
        return s;
      }
      // No more solutions or stopped?
      if ((n_busy == 0) || has_stopped.load(std::memory_order_acquire)) {
        m_search.release();
        // Make workers wait again
        block();
        return nullptr;
      }
      m_search.release();
    }
    GECODE_NEVER;
    return nullptr;
  }

  template<class Tracer>
  Support::Terminator* 
  Engine<Tracer>::Worker::terminator(void) const {
    return &_engine;
  }

  /*
   * Termination and deletion
   */
  template<class Tracer>
  Engine<Tracer>::Worker::~Worker(void) {
    delete cur;
    path.reset(0);
    tracer.done();
  }

  /*
   * Destructor
   */
  template<class Tracer>
  Engine<Tracer>::~Engine(void) {
    while (!solutions.empty())
      delete solutions.pop();
    if (scheduler_worker != nullptr)
      heap.rfree(scheduler_worker);
  }

}}}

// STATISTICS: search-par
