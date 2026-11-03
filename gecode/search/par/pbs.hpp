/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <algorithm>

namespace Gecode { namespace Search { namespace Par {

  template<class Collect>
  forceinline
  Spoke<Collect>::Completion::Completion(void) {
    // A spoke has not been submitted yet.  This initial signal lets the
    // first submission use the same consume-before-reuse handshake as all
    // subsequent submissions.
    done.signal();
  }

  template<class Collect>
  forceinline void
  Spoke<Collect>::Completion::terminated(void) {
    done.signal();
  }

  template<class Collect>
  forceinline void
  Spoke<Collect>::Completion::wait(void) {
    done.wait();
  }


  forceinline
  CollectAll::CollectAll(void)
    : solutions(heap) {}
  forceinline bool
  CollectAll::add(Space* s, Spoke<CollectAll>*) {
    solutions.push(s);
    return true;
  }
  forceinline bool
  CollectAll::constrain(const Space& b) {
    (void) b;
    return false;
  }
  forceinline bool
  CollectAll::empty(void) const {
    return solutions.empty();
  }
  forceinline Space*
  CollectAll::get(Spoke<CollectAll>*&) {
    return solutions.pop();
  }
  forceinline
  CollectAll::~CollectAll(void) {
    while (!solutions.empty())
      delete solutions.pop();
  }


  forceinline
  CollectBest::CollectBest(void)
    : b(nullptr), reporter(nullptr) {}
  forceinline bool
  CollectBest::add(Space* s, Spoke<CollectBest>* r) {
    if ((b != nullptr) &&
        !Search::better(*s,*b,"PBS::CollectBest::add")) {
      delete s;
      return false;
    }
    delete b;
    b = s;
    reporter = r;
    return true;
  }
  forceinline bool
  CollectBest::constrain(const Space& s) {
    if ((b != nullptr) &&
        !Search::better(s,*b,"PBS::CollectBest::constrain"))
      return false;
    delete b;
    b = s.clone();
    reporter = nullptr;
    return true;
  }
  forceinline bool
  CollectBest::empty(void) const {
    return reporter == nullptr;
  }
  forceinline Space*
  CollectBest::get(Spoke<CollectBest>*& r) {
    assert(!empty());
    r = reporter;
    reporter = nullptr;
    return b->clone();
  }
  forceinline
  CollectBest::~CollectBest(void) {
    delete b;
  }


  forceinline
  PortfolioStop::PortfolioStop(Stop* so0, Stop* po0,
                               const WorkerControl& control)
    : so(so0), po(po0), worker_control(control), tostop(nullptr) {}

  forceinline void
  PortfolioStop::share(std::atomic<bool>* ts) {
    tostop = ts;
  }


  template<class Collect>
  forceinline
  Spoke<Collect>::Spoke(PBS<Collect>* p, Engine* e, Stop* so)
    : Support::Runnable(false), portfolio(p), engine(e), stop(so) {}
  template<class Collect>
  forceinline Statistics
  Spoke<Collect>::statistics(void) const {
    return engine->statistics();
  }
  template<class Collect>
  forceinline bool
  Spoke<Collect>::stopped(void) const {
    return engine->stopped();
  }
  template<class Collect>
  forceinline bool
  Spoke<Collect>::limited(void) const {
#ifdef GECODE_HAS_EXPERIMENTAL
    return engine->limited();
#else
    return false;
#endif
  }
  template<class Collect>
  forceinline Support::Terminator*
  Spoke<Collect>::terminator(void) const {
    return const_cast<Completion*>(&completion);
  }
  template<class Collect>
  forceinline void
  Spoke<Collect>::wait(void) {
    completion.wait();
  }
  template<class Collect>
  forceinline void
  Spoke<Collect>::wake(void) {
    static_cast<PortfolioStop*>(stop)->wake();
  }
  template<class Collect>
  forceinline void
  Spoke<Collect>::constrain(const Space& b) {
    engine->constrain(b);
  }
  template<class Collect>
  Spoke<Collect>::~Spoke(void) {
    delete engine;
    delete stop;
  }



  template<class Collect>
  forceinline
  PBS<Collect>::PBS(Engine** engines, Stop** stops, unsigned int n,
                    const Statistics& stat0)
    : stat(stat0), spokes(heap.alloc<Spoke<Collect>*>(n)),
      n_spokes(n), n_active(n),
      spoke_stop(false), incomplete(false), tostop(false), n_busy(0) {
    // Initialize spokes
    for (unsigned int i=0U; i<n_spokes; i++) {
      spokes[i] = new Spoke<Collect>(this,engines[i],stops[i]);
      static_cast<PortfolioStop*>(stops[i])->share(&tostop);
    }
  }


  template<class Collect>
  forceinline bool
  PBS<Collect>::report(Spoke<Collect>* spoke, Space* s) {
    // If b is false the report should be repeated (solution was worse)
    bool b = true;
    bool wake = false;
    m.acquire();
    if (s != nullptr) {
      try {
        b = solutions.add(s,spoke);
        if (b) {
          tostop.store(true, std::memory_order_release);
          wake = true;
        }
      } catch (...) {
        delete s;
        if (failure == nullptr)
          failure = std::current_exception();
        tostop.store(true, std::memory_order_release);
        wake = true;
      }
    } else if (spoke->stopped() && !spoke->limited()) {
      if (!tostop.load(std::memory_order_acquire)) {
        spoke_stop.store(true, std::memory_order_release);
        tostop.store(true, std::memory_order_release);
        wake = true;
      }
    } else {
      // The spoke is done or stopped at a local limit (then search is incomplete)
      if (spoke->limited())
        incomplete = true;
      // Move spoke to inactive
      unsigned int i=0;
      while (spokes[i] != spoke)
        i++;
      assert(i < n_active);
      assert(n_active > 0);
      std::swap(spokes[i],spokes[--n_active]);
      tostop.store(true, std::memory_order_release);
      wake = true;
    }
    if (wake)
      for (unsigned int i=0U; i<n_active; i++)
        spokes[i]->wake();
    if (b) {
      if (--n_busy == 0)
        idle.signal();
    }
    m.release();
    return b;
  }

  template<class Collect>
  forceinline void
  PBS<Collect>::fail(void) {
    m.acquire();
    if (failure == nullptr)
      failure = std::current_exception();
    tostop.store(true, std::memory_order_release);
    for (unsigned int i=0U; i<n_active; i++)
      spokes[i]->wake();
    if (--n_busy == 0)
      idle.signal();
    m.release();
  }

  template<class Collect>
  void
  Spoke<Collect>::run(void) {
    try {
      Space* s;
      do {
        s = engine->next();
      } while (!portfolio->report(this,s));
    } catch (...) {
      portfolio->fail();
    }
  }

  template<class Collect>
  Space*
  PBS<Collect>::next(void) {
    m.acquire();
    if (failure != nullptr) {
      std::exception_ptr f = failure;
      m.release();
      std::rethrow_exception(f);
    }
    if (solutions.empty())
      spoke_stop.store(false, std::memory_order_release);
    while (solutions.empty() && (n_active > 0) && (failure == nullptr) &&
           !spoke_stop.load(std::memory_order_acquire)) {
      // Clear the internal stop used to interrupt sibling spokes
      tostop.store(false, std::memory_order_release);

      // Invariant: all spokes are idle!
      assert(n_busy == 0);
      assert(!tostop.load(std::memory_order_acquire));

      // Run all active spokes
      n_busy = n_active;
      for (unsigned int i=0U; i<n_active; i++) {
        // Consume the previous completion before reusing this spoke.  The
        // initial signal handles the first submission.
        spokes[i]->wait();
        Support::Thread::run(spokes[i]);
      }
      m.release();
      // Wait for all spokes to become idle
      idle.wait();
      m.acquire();
    }

    // Invariant all spokes are idle!
    assert(n_busy == 0);

    if (solutions.empty() && (n_active == 0) && incomplete)
      spoke_stop.store(true, std::memory_order_release);

    // Report a stored failure before broadcasting a solution, which could
    // call into the failed asset and replace the original exception.
    if (failure != nullptr) {
      std::exception_ptr f = failure;
      m.release();
      std::rethrow_exception(f);
    }

    Space* s;

    // Process solutions
    if (solutions.empty()) {
      s = nullptr;
    } else {
      Spoke<Collect>* r;
      s = solutions.get(r);
      if (Collect::best) {
        try {
          for (unsigned int i=0U; i<n_active; i++)
            if (spokes[i] != r)
              spokes[i]->constrain(*s);
        } catch (...) {
          delete s;
          failure = std::current_exception();
          std::exception_ptr f = failure;
          m.release();
          std::rethrow_exception(f);
        }
      }
    }

    m.release();
    return s;
  }

  template<class Collect>
  bool
  PBS<Collect>::stopped(void) const {
    return spoke_stop.load(std::memory_order_acquire);
  }

  template<class Collect>
  Statistics
  PBS<Collect>::statistics(void) const {
    assert(n_busy == 0);
    Statistics s(stat);
    for (unsigned int i=0U; i<n_spokes; i++)
      s += spokes[i]->statistics();
    return s;
  }

  template<class Collect>
  void
  PBS<Collect>::constrain(const Space& b) {
    assert(n_busy == 0);
    if (!Collect::best)
      throw NoBest("PBS::constrain");
    // Report a stored failure before comparing, so that the original
    // exception is not replaced by one from the comparison below.
    if (failure != nullptr)
      std::rethrow_exception(failure);
    try {
      if (solutions.constrain(b)) {
        // The solution is better
        for (unsigned int i=0U; i<n_active; i++)
          spokes[i]->constrain(b);
      }
    } catch (...) {
      failure = std::current_exception();
      std::rethrow_exception(failure);
    }
  }

  template<class Collect>
  PBS<Collect>::~PBS(void) {
    // A report can make n_busy zero before Spoke::run and Thread::Run::exec
    // have returned.  Wait for the completion handshake before deleting the
    // PBS-owned spokes.
    for (unsigned int i=0U; i<n_spokes; i++)
      spokes[i]->wait();
    assert(n_busy == 0);
    for (unsigned int i=0U; i<n_spokes; i++)
      delete spokes[i];
    heap.free<Spoke<Collect>*>(spokes,n_spokes);
  }

}}}

// STATISTICS: search-par
