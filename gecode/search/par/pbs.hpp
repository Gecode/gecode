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
  Asset<Collect>::Completion::Completion(void) {
    // An asset has not been submitted yet.  This initial signal lets the
    // first submission use the same consume-before-reuse handshake as all
    // subsequent submissions.
    done.signal();
  }

  template<class Collect>
  forceinline void
  Asset<Collect>::Completion::terminated(void) {
    done.signal();
  }

  template<class Collect>
  forceinline void
  Asset<Collect>::Completion::wait(void) {
    done.wait();
  }


  forceinline
  CollectAll::CollectAll(void)
    : solutions(heap) {}
  forceinline bool
  CollectAll::add(Space* s, Asset<CollectAll>*) {
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
  CollectAll::get(Asset<CollectAll>*&) {
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
  CollectBest::add(Space* s, Asset<CollectBest>* r) {
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
  CollectBest::get(Asset<CollectBest>*& r) {
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
  PortfolioStatistics::PortfolioStatistics(const Statistics& s, unsigned int n0)
    : n(n0 + 1), stat(heap.alloc<Statistics>(n0 + 1)) {
    stat[0] = s;
  }

  forceinline
  PortfolioStatistics::~PortfolioStatistics(void) {
    heap.free<Statistics>(stat, n);
  }

  forceinline
  PortfolioStop::PortfolioStop(Stop* so0, Stop* po0,
                               const WorkerControl& control)
    : so(so0), po(po0), worker_control(control), tostop(nullptr), ps(nullptr), asset(0U),
      _cause(PortfolioStopCause::None) {}

  forceinline void
  PortfolioStop::reset(void) {
    _cause.store(PortfolioStopCause::None, std::memory_order_relaxed);
  }

  forceinline PortfolioStopCause
  PortfolioStop::cause(void) const {
    return _cause.load(std::memory_order_relaxed);
  }

  forceinline void
  PortfolioStop::share(std::atomic<bool>* ts, PortfolioStatistics* ps0, unsigned int i) {
    tostop = ts;
    ps = ps0;
    asset = i;
  }


  template<class Collect>
  forceinline
  Asset<Collect>::Asset(PBS<Collect>* p, Engine* e, Stop* so)
    : Support::Runnable(false), portfolio(p), engine(e), stop(so), _parked(false) {}
  template<class Collect>
  forceinline Statistics
  Asset<Collect>::statistics(void) const {
    return engine->statistics();
  }
  template<class Collect>
  forceinline EndReason
  Asset<Collect>::end_reason(void) const {
    return engine->end_reason();
  }
  template<class Collect>
  forceinline Support::Terminator*
  Asset<Collect>::terminator(void) const {
    return const_cast<Completion*>(&completion);
  }
  template<class Collect>
  forceinline void
  Asset<Collect>::wait(void) {
    completion.wait();
  }
  template<class Collect>
  forceinline void
  Asset<Collect>::wake(void) {
    static_cast<PortfolioStop*>(stop)->wake();
  }
  template<class Collect>
  forceinline void
  Asset<Collect>::constrain(const Space& b) {
    engine->constrain(b);
  }
  template<class Collect>
  forceinline PortfolioStopCause
  Asset<Collect>::cause(void) const {
    return static_cast<const PortfolioStop*>(stop)->cause();
  }
  template <class Collect>
  forceinline void
  Asset<Collect>::parked(bool p) {
    _parked = p;
  }
  template <class Collect>
  forceinline bool
  Asset<Collect>::parked(void) const {
    return _parked;
  }
  template <class Collect>
  Asset<Collect>::~Asset(void) {
    delete engine;
    delete stop;
  }



  template<class Collect>
  forceinline
  PBS<Collect>::PBS(Engine** engines, Stop** stops, unsigned int n,
                    const Statistics& stat0)
    : stat(stat0), portfolio_stat(stat0,n),
      assets(heap.alloc<Asset<Collect>*>(n)),
      n_assets(n), n_active(n),
        portfolio_stop(false), exhausted(false), end(EndReason::NotEnded), tostop(false), n_busy(0) {
    // Initialize assets
    for (unsigned int i=0U; i<n_assets; i++) {
      assets[i] = new Asset<Collect>(this,engines[i],stops[i]);
      static_cast<PortfolioStop*>(stops[i])->share(&tostop,&portfolio_stat,i);
    }
  }


  template<class Collect>
  forceinline bool
  PBS<Collect>::report(Asset<Collect>* asset, Space* s) {
    // If b is false the report should be repeated (solution was worse)
    bool b = true;
    bool wake = false;
    m.acquire();
    if (s != nullptr) {
      try {
        b = solutions.add(s,asset);
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
    } else if (asset->end_reason() == EndReason::Limit) {
      switch (asset->cause()) {
      case PortfolioStopCause::Interrupt:
        // The asset runs again with the next round
        break;
      case PortfolioStopCause::Portfolio:
        portfolio_stop.store(true, std::memory_order_release);
        tostop.store(true, std::memory_order_release);
        wake = true;
        break;
      default:
        // The stop object of the asset stopped it, the others continue
        asset->parked(true);
        break;
      }
    } else if (pbs_asset_proof(asset->end_reason())) {
      // Stop the other assets, no further solution exists
      exhausted = true;
      tostop.store(true, std::memory_order_release);
      wake = true;
    } else {
      // The asset ended without a proof, move it to inactive
      unsigned int i=0;
      while (assets[i] != asset)
        i++;
      assert(i < n_active);
      assert(n_active > 0);
      std::swap(assets[i],assets[--n_active]);
      tostop.store(true, std::memory_order_release);
      wake = true;
    }
    if (wake)
      for (unsigned int i=0U; i<n_active; i++)
        assets[i]->wake();
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
      assets[i]->wake();
    if (--n_busy == 0)
      idle.signal();
    m.release();
  }

  template<class Collect>
  void
  Asset<Collect>::run(void) {
    try {
      Space* s;
      do {
        static_cast<PortfolioStop*>(stop)->reset();
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
    if (solutions.empty()) {
      portfolio_stop.store(false, std::memory_order_release);
      // Resume the assets that their own stop objects stopped
      for (unsigned int i=0U; i<n_active; i++)
        assets[i]->parked(false);
    }
    while (solutions.empty() && (failure == nullptr) &&
           !exhausted && !portfolio_stop.load(std::memory_order_acquire)) {
      unsigned int n_run = 0;
      for (unsigned int i=0U; i<n_active; i++)
        if (!assets[i]->parked())
          n_run++;
      if (n_run == 0)
        break;

      // Clear the internal stop used to interrupt sibling assets
      tostop.store(false, std::memory_order_release);

      // Invariant: all assets are idle!
      assert(n_busy == 0);
      assert(!tostop.load(std::memory_order_acquire));

      // Run all active assets that are not parked
      n_busy = n_run;
      for (unsigned int i=0U; i<n_active; i++)
        if (!assets[i]->parked()) {
          // Consume the previous completion before reusing this asset.  The
          // initial signal handles the first submission.
          assets[i]->wait();
          Support::Thread::run(assets[i]);
        }
      m.release();
      // Wait for all assets to become idle
      idle.wait();
      m.acquire();
    }

    // Invariant all assets are idle!
    assert(n_busy == 0);

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
      if (exhausted)
        end = EndReason::Exhausted;
      else if (portfolio_stop.load(std::memory_order_acquire) || (n_active > 0))
        // Stopped by the portfolio stop object, or only parked assets remain
        end = EndReason::Limit;
      else
        // All assets ended without a proof
        end = EndReason::Incomplete;
    } else {
      end = EndReason::NotEnded;
      Asset<Collect>* r;
      s = solutions.get(r);
      if (Collect::best) {
        try {
          for (unsigned int i=0U; i<n_active; i++)
            if (assets[i] != r)
              assets[i]->constrain(*s);
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
  EndReason
  PBS<Collect>::end_reason(void) const {
    return end;
  }

  template<class Collect>
  Statistics
  PBS<Collect>::statistics(void) const {
    assert(n_busy == 0);
    Statistics s(stat);
    for (unsigned int i=0U; i<n_assets; i++)
      s += assets[i]->statistics();
    return s;
  }

  template<class Collect>
  void
  PBS<Collect>::constrain(const Space& b) {
    assert(n_busy == 0);
    if (!Collect::best)
      throw NoBest("PBS::constrain");
    try {
      if (solutions.constrain(b)) {
        // The solution is better
        for (unsigned int i=0U; i<n_active; i++)
          assets[i]->constrain(b);
      }
    } catch (...) {
      failure = std::current_exception();
      std::rethrow_exception(failure);
    }
  }

  template<class Collect>
  PBS<Collect>::~PBS(void) {
    // A report can make n_busy zero before Asset::run and Thread::Run::exec
    // have returned.  Wait for the completion handshake before deleting the
    // PBS-owned assets.
    for (unsigned int i=0U; i<n_assets; i++)
      assets[i]->wait();
    assert(n_busy == 0);
    for (unsigned int i=0U; i<n_assets; i++)
      delete assets[i];
    heap.free<Asset<Collect>*>(assets,n_assets);
  }

}}}

// STATISTICS: search-par
