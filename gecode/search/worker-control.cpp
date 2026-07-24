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

#include <gecode/search/worker-control.hh>

namespace Gecode { namespace Search {

  class WorkerControl::State : public Support::RefCount {
  public:
    enum Lifecycle {
      NEVER_BOUND,
      ATTACHED,
      DETACHED
    };

    class TestSupport {
    public:
      unsigned int capacity;
      std::atomic<unsigned long long int>
        epoch[WorkerControlAccess::GATE_COUNT];
      std::atomic<unsigned int>
        worker[WorkerControlAccess::GATE_COUNT];
      std::atomic<unsigned int>
        expected[WorkerControlAccess::GATE_COUNT];
      std::atomic<unsigned int>
        reached_count[WorkerControlAccess::GATE_COUNT];
      std::atomic<bool> enabled[WorkerControlAccess::GATE_COUNT];
      Support::Event reached[WorkerControlAccess::GATE_COUNT];
      Support::Event* release;
      std::atomic<unsigned long long int>* seen;
      std::atomic<bool>* waiting;
      std::atomic<unsigned int> admitted;
      std::atomic<unsigned int> generation_admitted;
      std::atomic<unsigned int> max_admitted;
      std::atomic<unsigned long long int> measured_generation;
      std::atomic<unsigned long long int> completed_generation;
      std::atomic<unsigned long long int>* action_generation;
      std::atomic<unsigned int>* logical;
      std::atomic<bool>* lease;
      std::atomic<bool>* parked;
      std::atomic<unsigned long long int>* incumbent_deliveries;

      TestSupport(unsigned int capacity0)
        : capacity(capacity0),
          release(new Support::Event
                  [WorkerControlAccess::GATE_COUNT * capacity]),
          seen(new std::atomic<unsigned long long int>
               [WorkerControlAccess::GATE_COUNT * capacity]),
          waiting(new std::atomic<bool>
                  [WorkerControlAccess::GATE_COUNT * capacity]),
          admitted(0U), generation_admitted(0U), max_admitted(0U),
          measured_generation(0U), completed_generation(0U),
          action_generation(
            new std::atomic<unsigned long long int>[capacity]),
          logical(new std::atomic<unsigned int>[capacity]),
          lease(new std::atomic<bool>[capacity]),
          parked(new std::atomic<bool>[capacity]),
          incumbent_deliveries(
            new std::atomic<unsigned long long int>[capacity]) {
        for (unsigned int i=0U; i<capacity; i++) {
          logical[i].store(0U,std::memory_order_relaxed);
          lease[i].store(false,std::memory_order_relaxed);
          parked[i].store(false,std::memory_order_relaxed);
          incumbent_deliveries[i].store(0U,std::memory_order_relaxed);
          action_generation[i].store(0U,std::memory_order_relaxed);
        }
        for (unsigned int g=0U; g<WorkerControlAccess::GATE_COUNT; g++) {
          epoch[g].store(0U,std::memory_order_relaxed);
          worker[g].store(WorkerControlAccess::ALL_WORKERS,
                          std::memory_order_relaxed);
          expected[g].store(0U,std::memory_order_relaxed);
          reached_count[g].store(0U,std::memory_order_relaxed);
          enabled[g].store(false,std::memory_order_relaxed);
          for (unsigned int i=0U; i<capacity; i++)
            {
              seen[g*capacity+i].store(0U,std::memory_order_relaxed);
              waiting[g*capacity+i].store(false,std::memory_order_relaxed);
            }
        }
      }

      ~TestSupport(void) {
        delete [] release;
        delete [] seen;
        delete [] waiting;
        delete [] logical;
        delete [] lease;
        delete [] parked;
        delete [] incumbent_deliveries;
        delete [] action_generation;
      }
    };

    std::atomic<unsigned int> requested;
    std::atomic<unsigned int> capacity;
    std::atomic<unsigned long long int> generation;
    std::atomic<unsigned long long int> observed_generation;
    std::atomic<unsigned long long int> handoffs;
    std::atomic<unsigned int> leases;
    std::atomic<unsigned int> parked;
    std::atomic<unsigned int> owners;
    std::atomic<unsigned int> parked_owners;
    std::atomic<unsigned int> last_handoff_from;
    std::atomic<unsigned int> last_handoff_to;
    std::atomic<unsigned int> last_solution_worker;
    std::atomic<unsigned int> solution_handoff_from;
    std::atomic<unsigned int> solution_handoff_to;
    std::atomic<TestSupport*> test_support;
    Support::Mutex mutex;
    Lifecycle lifecycle;
    Support::Event* events;

    State(unsigned int workers)
      : RefCount(1U), requested(workers), capacity(0U), generation(0U),
        observed_generation(0U), handoffs(0U), leases(0U), parked(0U),
        owners(0U), parked_owners(0U),
        last_handoff_from(WorkerControlAccess::ALL_WORKERS),
        last_handoff_to(WorkerControlAccess::ALL_WORKERS),
        last_solution_worker(WorkerControlAccess::ALL_WORKERS),
        solution_handoff_from(WorkerControlAccess::ALL_WORKERS),
        solution_handoff_to(WorkerControlAccess::ALL_WORKERS),
        test_support(nullptr),
        lifecycle(NEVER_BOUND), events(nullptr) {}
    ~State(void) {
      delete [] events;
      delete test_support.load(std::memory_order_relaxed);
    }
  };

  WorkerControl::WorkerControl(void) noexcept
    : state(nullptr) {}

  WorkerControl::WorkerControl(unsigned int requested)
    : state(nullptr) {
    if (requested == 0U)
      throw InvalidWorkerRequest("WorkerControl::WorkerControl");
    state = new State(requested);
  }

  WorkerControl::WorkerControl(const WorkerControl& control)
    : state(control.state) {
    if (state != nullptr)
      state->inc();
  }

  WorkerControl&
  WorkerControl::operator =(const WorkerControl& control) {
    if (control.state != nullptr)
      control.state->inc();
    if ((state != nullptr) && state->dec())
      delete state;
    state = control.state;
    return *this;
  }

  WorkerControl::~WorkerControl(void) {
    if ((state != nullptr) && state->dec())
      delete state;
  }

  WorkerControl::operator bool(void) const noexcept {
    return state != nullptr;
  }

  unsigned int
  WorkerControl::requested(void) const noexcept {
    return (state == nullptr) ? 0U :
      state->requested.load(std::memory_order_acquire);
  }

  void
  WorkerControl::request(unsigned int workers) {
    if (state == nullptr)
      throw UninitializedWorkerControl("WorkerControl::request");
    if (workers == 0U)
      throw InvalidWorkerRequest("WorkerControl::request");

    bool changed = false;
    {
      Support::Lock lock(state->mutex);
      unsigned int capacity =
        state->capacity.load(std::memory_order_relaxed);
      if ((capacity != 0U) && (workers > capacity))
        throw InvalidWorkerRequest("WorkerControl::request");
      unsigned int old =
        state->requested.load(std::memory_order_relaxed);
      if (old != workers) {
        state->requested.store(workers,std::memory_order_release);
        (void) state->generation.fetch_add(1U,std::memory_order_release);
        changed = true;
      }
    }
    if (changed)
      WorkerControlAccess::signal_all(*this);
  }

  unsigned int
  WorkerControl::capacity(void) const noexcept {
    return (state == nullptr) ? 0U :
      state->capacity.load(std::memory_order_acquire);
  }

  void
  WorkerControlAccess::attach(WorkerControl& control,
                              unsigned int capacity) {
    if (control.state == nullptr)
      return;
    WorkerControl::State* state = control.state;
    Support::Lock lock(state->mutex);
    if (state->lifecycle != WorkerControl::State::NEVER_BOUND)
      throw WorkerControlInUse("WorkerControlAccess::attach");
    if ((capacity == 0U) ||
        (state->requested.load(std::memory_order_relaxed) > capacity))
      throw InvalidWorkerRequest("WorkerControlAccess::attach");
    state->events = new Support::Event[capacity];
    state->capacity.store(capacity,std::memory_order_release);
    state->lifecycle = WorkerControl::State::ATTACHED;
  }

  void
  WorkerControlAccess::detach(WorkerControl& control) {
    if (control.state == nullptr)
      return;
    WorkerControl::State* state = control.state;
    {
      Support::Lock lock(state->mutex);
      if (state->lifecycle == WorkerControl::State::ATTACHED)
        state->lifecycle = WorkerControl::State::DETACHED;
    }
    signal_all(control);
  }

  unsigned long long int
  WorkerControlAccess::generation(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->generation.load(std::memory_order_acquire);
  }

  bool
  WorkerControlAccess::engaged(const WorkerControl& control) {
    return control.state != nullptr;
  }

  unsigned int
  WorkerControlAccess::requested(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->requested.load(std::memory_order_acquire);
  }

  void
  WorkerControlAccess::snapshot(const WorkerControl& control,
                                unsigned int& requested,
                                unsigned long long int& generation) {
    WorkerControl::State* state = control.state;
    if (state == nullptr) {
      requested = 0U;
      generation = 0U;
      return;
    }
    Support::Lock lock(state->mutex);
    requested = state->requested.load(std::memory_order_relaxed);
    generation = state->generation.load(std::memory_order_relaxed);
  }

  void
  WorkerControlAccess::wait(WorkerControl& control, unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    Support::Event* event = nullptr;
    {
      Support::Lock lock(state->mutex);
      unsigned int capacity =
        state->capacity.load(std::memory_order_relaxed);
      assert((state->events == nullptr) || (worker < capacity));
      if ((state->events != nullptr) && (worker < capacity))
        event = &state->events[worker];
    }
    if (event != nullptr)
      event->wait();
  }

  void
  WorkerControlAccess::signal(WorkerControl& control, unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    Support::Event* event = nullptr;
    {
      Support::Lock lock(state->mutex);
      unsigned int capacity =
        state->capacity.load(std::memory_order_relaxed);
      assert((state->events == nullptr) || (worker < capacity));
      if ((state->events != nullptr) && (worker < capacity))
        event = &state->events[worker];
    }
    if (event != nullptr)
      event->signal();
  }

  void
  WorkerControlAccess::signal_all(WorkerControl& control) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    Support::Event* events;
    unsigned int capacity;
    {
      Support::Lock lock(state->mutex);
      events = state->events;
      capacity = state->capacity.load(std::memory_order_relaxed);
    }
    if (events == nullptr)
      return;
    for (unsigned int i=0U; i<capacity; i++)
      events[i].signal();
  }

  void
  WorkerControlAccess::observe(WorkerControl& control,
                               unsigned long long int generation,
                               unsigned int leases, unsigned int parked,
                               unsigned int owners,
                               unsigned int parked_owners) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    state->leases.store(leases,std::memory_order_release);
    state->parked.store(parked,std::memory_order_release);
    state->owners.store(owners,std::memory_order_release);
    state->parked_owners.store(parked_owners,std::memory_order_release);
    state->observed_generation.store(generation,std::memory_order_release);
  }

  void
  WorkerControlAccess::observe_worker(WorkerControl& control,
                                     unsigned int worker,
                                     unsigned int logical,
                                     bool lease, bool parked) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    if (support == nullptr)
      return;
    assert(worker < support->capacity);
    support->logical[worker].store(logical,std::memory_order_release);
    support->lease[worker].store(lease,std::memory_order_release);
    support->parked[worker].store(parked,std::memory_order_release);
  }

  void
  WorkerControlAccess::handoff(WorkerControl& control, unsigned int from,
                               unsigned int to) {
    if (control.state == nullptr)
      return;
    control.state->last_handoff_from.store(from,std::memory_order_release);
    control.state->last_handoff_to.store(to,std::memory_order_release);
    (void) control.state->handoffs.fetch_add(1U,std::memory_order_release);
  }

  void
  WorkerControlAccess::solution(WorkerControl& control, unsigned int worker) {
    if (control.state == nullptr)
      return;
    control.state->solution_handoff_from.store(
      control.state->last_handoff_from.load(std::memory_order_acquire),
      std::memory_order_release);
    control.state->solution_handoff_to.store(
      control.state->last_handoff_to.load(std::memory_order_acquire),
      std::memory_order_release);
    control.state->last_solution_worker.store(worker,
                                               std::memory_order_release);
  }

  void
  WorkerControlAccess::incumbent(WorkerControl& control,
                                 unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    if (support == nullptr)
      return;
    assert(worker < support->capacity);
    (void) support->incumbent_deliveries[worker].fetch_add(
      1U,std::memory_order_release);
  }

  void
  WorkerControlAccess::gate(WorkerControl& control, Gate gate,
                            unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    if (support == nullptr)
      return;
    unsigned int g = static_cast<unsigned int>(gate);
    if (!support->enabled[g].load(std::memory_order_acquire))
      return;
    unsigned int selected =
      support->worker[g].load(std::memory_order_relaxed);
    if ((selected != ALL_WORKERS) && (selected != worker))
      return;
    unsigned int capacity = support->capacity;
    if (worker >= capacity)
      return;
    unsigned long long int epoch =
      support->epoch[g].load(std::memory_order_acquire);
    std::atomic<unsigned long long int>& seen =
      support->seen[g*capacity+worker];
    unsigned long long int old = seen.load(std::memory_order_relaxed);
    while ((old != epoch) &&
           !seen.compare_exchange_weak(old,epoch,
                                      std::memory_order_acq_rel,
                                      std::memory_order_relaxed)) {}
    if (old == epoch)
      return;
    support->waiting[g*capacity+worker].store(true,
                                              std::memory_order_release);
    unsigned int reached =
      support->reached_count[g].fetch_add(1U,
                                          std::memory_order_acq_rel) + 1U;
    if (reached == support->expected[g].load(std::memory_order_acquire))
      support->reached[g].signal();
    support->release[g*capacity+worker].wait();
    support->waiting[g*capacity+worker].store(false,
                                              std::memory_order_release);
  }

  void
  WorkerControlAccess::gate_install(WorkerControl& control, Gate gate,
                                    unsigned int worker,
                                    unsigned int expected) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support;
    {
      Support::Lock lock(state->mutex);
      support = state->test_support.load(std::memory_order_relaxed);
      if (support == nullptr) {
        unsigned int capacity =
          state->capacity.load(std::memory_order_relaxed);
        support = new WorkerControl::State::TestSupport(capacity);
        state->test_support.store(support,std::memory_order_release);
      }
    }
    unsigned int capacity = support->capacity;
    assert((worker == ALL_WORKERS) || (worker < capacity));
    assert((expected > 0U) && (expected <= capacity));
    unsigned int g = static_cast<unsigned int>(gate);
    support->enabled[g].store(false,std::memory_order_release);
    support->worker[g].store(worker,std::memory_order_relaxed);
    support->expected[g].store(expected,std::memory_order_relaxed);
    support->reached_count[g].store(0U,std::memory_order_relaxed);
    (void) support->epoch[g].fetch_add(1U,std::memory_order_release);
    support->enabled[g].store(true,std::memory_order_release);
  }

  void
  WorkerControlAccess::gate_wait(WorkerControl& control, Gate gate) {
    if (control.state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      control.state->test_support.load(std::memory_order_acquire);
    assert(support != nullptr);
    support->reached[static_cast<unsigned int>(gate)].wait();
  }

  void
  WorkerControlAccess::gate_release(WorkerControl& control, Gate gate,
                                    unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    assert(support != nullptr);
    unsigned int capacity = support->capacity;
    assert(worker < capacity);
    unsigned int slot =
      static_cast<unsigned int>(gate)*capacity+worker;
    if (support->waiting[slot].load(std::memory_order_acquire))
      support->release[slot].signal();
  }

  void
  WorkerControlAccess::gate_release_all(WorkerControl& control, Gate gate) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    if (support == nullptr)
      return;
    unsigned int g = static_cast<unsigned int>(gate);
    support->enabled[g].store(false,std::memory_order_release);
    unsigned int capacity = support->capacity;
    unsigned long long int epoch =
      support->epoch[g].load(std::memory_order_acquire);
    for (unsigned int i=0U; i<capacity; i++)
      if (support->seen[g*capacity+i].load(std::memory_order_acquire) ==
          epoch &&
          support->waiting[g*capacity+i].load(std::memory_order_acquire))
        support->release[g*capacity+i].signal();
  }

  bool
  WorkerControlAccess::gate_waiting(const WorkerControl& control, Gate gate,
                                    unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return false;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    if ((support == nullptr) || (worker >= support->capacity))
      return false;
    unsigned int g = static_cast<unsigned int>(gate);
    return support->waiting[g*support->capacity+worker].load(
      std::memory_order_acquire);
  }

  void
  WorkerControlAccess::action_begin(
    WorkerControl& control, unsigned int worker,
    const std::atomic<unsigned long long int>& generation) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    if (support == nullptr)
      return;
    unsigned long long int action_generation =
      generation.load(std::memory_order_acquire);
    support->action_generation[worker].store(action_generation,
                                              std::memory_order_release);
    unsigned int current =
      support->admitted.fetch_add(1U,std::memory_order_acq_rel) + 1U;
    {
      Support::Lock lock(state->mutex);
      unsigned long long int measured =
        support->measured_generation.load(std::memory_order_relaxed);
      if (measured != 0U) {
        if (action_generation != measured)
          return;
        current = support->generation_admitted.fetch_add(
          1U,std::memory_order_relaxed) + 1U;
      }
      unsigned int maximum =
        support->max_admitted.load(std::memory_order_relaxed);
      if (maximum < current)
        support->max_admitted.store(current,std::memory_order_release);
    }
  }

  void
  WorkerControlAccess::action_end(WorkerControl& control,
                                  unsigned int worker) {
    if (control.state == nullptr)
      return;
    WorkerControl::State::TestSupport* support =
      control.state->test_support.load(std::memory_order_acquire);
    if (support != nullptr) {
      (void) support->admitted.fetch_sub(1U,std::memory_order_release);
      unsigned long long int generation =
        support->action_generation[worker].load(std::memory_order_acquire);
      {
        Support::Lock lock(control.state->mutex);
        if (support->measured_generation.load(std::memory_order_relaxed) ==
            generation)
          (void) support->generation_admitted.fetch_sub(
            1U,std::memory_order_relaxed);
      }
      unsigned long long int old =
        support->completed_generation.load(std::memory_order_relaxed);
      while ((old < generation) &&
             !support->completed_generation.compare_exchange_weak(
               old,generation,std::memory_order_release,
               std::memory_order_relaxed)) {}
    }
  }

  void
  WorkerControlAccess::reset_max_admitted(WorkerControl& control) {
    if (control.state != nullptr) {
      Support::Lock lock(control.state->mutex);
      WorkerControl::State::TestSupport* support =
        control.state->test_support.load(std::memory_order_acquire);
      assert(support != nullptr);
      unsigned int current =
        support->admitted.load(std::memory_order_acquire);
      support->measured_generation.store(0U,std::memory_order_release);
      support->max_admitted.store(current,std::memory_order_release);
    }
  }

  void
  WorkerControlAccess::reset_max_admitted(
    WorkerControl& control, unsigned long long int generation) {
    if (control.state != nullptr) {
      Support::Lock lock(control.state->mutex);
      WorkerControl::State::TestSupport* support =
        control.state->test_support.load(std::memory_order_acquire);
      assert(support != nullptr);
      support->generation_admitted.store(0U,std::memory_order_release);
      support->max_admitted.store(0U,std::memory_order_release);
      support->measured_generation.store(generation,
                                          std::memory_order_release);
    }
  }

  unsigned int
  WorkerControlAccess::admitted(const WorkerControl& control) {
    if (control.state == nullptr)
      return 0U;
    WorkerControl::State::TestSupport* support =
      control.state->test_support.load(std::memory_order_acquire);
    return (support == nullptr) ? 0U :
      support->admitted.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::max_admitted(const WorkerControl& control) {
    if (control.state == nullptr)
      return 0U;
    WorkerControl::State::TestSupport* support =
      control.state->test_support.load(std::memory_order_acquire);
    return (support == nullptr) ? 0U :
      support->max_admitted.load(std::memory_order_acquire);
  }

  unsigned long long int
  WorkerControlAccess::completed_generation(const WorkerControl& control) {
    if (control.state == nullptr)
      return 0U;
    WorkerControl::State::TestSupport* support =
      control.state->test_support.load(std::memory_order_acquire);
    return (support == nullptr) ? 0U :
      support->completed_generation.load(std::memory_order_acquire);
  }

  unsigned long long int
  WorkerControlAccess::observed_generation(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->observed_generation.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::leases(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->leases.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::parked(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->parked.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::owners(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->owners.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::parked_owners(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->parked_owners.load(std::memory_order_acquire);
  }

  unsigned long long int
  WorkerControlAccess::handoffs(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->handoffs.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::last_handoff_from(const WorkerControl& control) {
    return (control.state == nullptr) ? ALL_WORKERS :
      control.state->last_handoff_from.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::last_handoff_to(const WorkerControl& control) {
    return (control.state == nullptr) ? ALL_WORKERS :
      control.state->last_handoff_to.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::last_solution_worker(const WorkerControl& control) {
    return (control.state == nullptr) ? ALL_WORKERS :
      control.state->last_solution_worker.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::solution_handoff_from(const WorkerControl& control) {
    return (control.state == nullptr) ? ALL_WORKERS :
      control.state->solution_handoff_from.load(std::memory_order_acquire);
  }

  unsigned int
  WorkerControlAccess::solution_handoff_to(const WorkerControl& control) {
    return (control.state == nullptr) ? ALL_WORKERS :
      control.state->solution_handoff_to.load(std::memory_order_acquire);
  }

  bool
  WorkerControlAccess::owner(const WorkerControl& control,
                             unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return false;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    assert((support != nullptr) && (worker < support->capacity));
    return support->logical[worker].load(std::memory_order_acquire) == 0U;
  }

  bool
  WorkerControlAccess::leased(const WorkerControl& control,
                              unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return false;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    assert((support != nullptr) && (worker < support->capacity));
    return support->lease[worker].load(std::memory_order_acquire);
  }

  bool
  WorkerControlAccess::worker_parked(const WorkerControl& control,
                                     unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return false;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    assert((support != nullptr) && (worker < support->capacity));
    return support->parked[worker].load(std::memory_order_acquire);
  }

  unsigned long long int
  WorkerControlAccess::incumbent_deliveries(
    const WorkerControl& control, unsigned int worker) {
    WorkerControl::State* state = control.state;
    if (state == nullptr)
      return 0U;
    WorkerControl::State::TestSupport* support =
      state->test_support.load(std::memory_order_acquire);
    assert((support != nullptr) && (worker < support->capacity));
    return support->incumbent_deliveries[worker].load(
      std::memory_order_acquire);
  }

}}
