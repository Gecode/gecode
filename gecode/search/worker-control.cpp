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

    std::atomic<unsigned int> requested;
    std::atomic<unsigned int> capacity;
    std::atomic<unsigned long long int> generation;
    Support::Mutex mutex;
    Lifecycle lifecycle;

    State(unsigned int workers)
      : RefCount(1U), requested(workers), capacity(0U), generation(0U),
        lifecycle(NEVER_BOUND) {}
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
    }
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
    state->capacity.store(capacity,std::memory_order_release);
    state->lifecycle = WorkerControl::State::ATTACHED;
  }

  void
  WorkerControlAccess::detach(WorkerControl& control) {
    if (control.state == nullptr)
      return;
    WorkerControl::State* state = control.state;
    Support::Lock lock(state->mutex);
    if (state->lifecycle == WorkerControl::State::ATTACHED)
      state->lifecycle = WorkerControl::State::DETACHED;
  }

  unsigned long long int
  WorkerControlAccess::generation(const WorkerControl& control) {
    return (control.state == nullptr) ? 0U :
      control.state->generation.load(std::memory_order_acquire);
  }

}}
