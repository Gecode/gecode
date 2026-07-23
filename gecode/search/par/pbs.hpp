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
  Variant<Collect>::Completion::Completion(void) {
    // A variant has not been submitted yet.  This initial signal lets the
    // first submission use the same consume-before-reuse handshake as all
    // subsequent submissions.
    done.signal();
  }

  template<class Collect>
  forceinline void
  Variant<Collect>::Completion::terminated(void) {
    done.signal();
  }

  template<class Collect>
  forceinline void
  Variant<Collect>::Completion::wait(void) {
    done.wait();
  }


  forceinline
  CollectAll::CollectAll(void)
    : solutions(heap) {}
  forceinline bool
  CollectAll::add(Space* s, Variant<CollectAll>*) {
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
  CollectAll::get(Variant<CollectAll>*&) {
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
  CollectBest::add(Space* s, Variant<CollectBest>* r) {
    if (b != nullptr) {
      b->constrain(*s);
      if (b->status() == SS_FAILED) {
        delete b;
      } else {
        delete s;
        return false;
      }
    }
    b = s;
    reporter = r;
    return true;
  }
  forceinline bool
  CollectBest::constrain(const Space& s) {
    if (b != nullptr) {
      b->constrain(s);
      if (b->status() == SS_FAILED) {
        delete b;
      } else {
        return false;
      }
    }
    b = s.clone();
    reporter = nullptr;
    return true;
  }
  forceinline bool
  CollectBest::empty(void) const {
    return reporter == nullptr;
  }
  forceinline Space*
  CollectBest::get(Variant<CollectBest>*& r) {
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
  PortfolioStop::PortfolioStop(Stop* so0)
    : so(so0), tostop(nullptr) {}

  forceinline void
  PortfolioStop::share(std::atomic<bool>* ts) {
    tostop = ts;
  }


  template<class Collect>
  forceinline
  Variant<Collect>::Variant(PBS<Collect>* p, Engine* e, Stop* so)
    : Support::Runnable(false), portfolio(p), engine(e), stop(so) {}
  template<class Collect>
  forceinline Statistics
  Variant<Collect>::statistics(void) const {
    return engine->statistics();
  }
  template<class Collect>
  forceinline bool
  Variant<Collect>::stopped(void) const {
    return engine->stopped();
  }
  template<class Collect>
  forceinline Support::Terminator*
  Variant<Collect>::terminator(void) const {
    return const_cast<Completion*>(&completion);
  }
  template<class Collect>
  forceinline void
  Variant<Collect>::wait(void) {
    completion.wait();
  }
  template<class Collect>
  forceinline void
  Variant<Collect>::constrain(const Space& b) {
    engine->constrain(b);
  }
  template<class Collect>
  Variant<Collect>::~Variant(void) {
    delete engine;
    delete stop;
  }



  template<class Collect>
  forceinline
  PBS<Collect>::PBS(Engine** engines, Stop** stops, unsigned int n,
                    const Statistics& stat0)
    : stat(stat0), variants(heap.alloc<Variant<Collect>*>(n)),
      n_variants(n), n_active(n),
      variant_stop(false), tostop(false), n_busy(0) {
    // Initialize variants
    for (unsigned int i=0U; i<n_variants; i++) {
      variants[i] = new Variant<Collect>(this,engines[i],stops[i]);
      static_cast<PortfolioStop*>(stops[i])->share(&tostop);
    }
  }


  template<class Collect>
  forceinline bool
  PBS<Collect>::report(Variant<Collect>* variant, Space* s) {
    // If b is false the report should be repeated (solution was worse)
    bool b = true;
    m.acquire();
    if (s != nullptr) {
      b = solutions.add(s,variant);
      if (b)
        tostop.store(true, std::memory_order_release);
    } else if (variant->stopped()) {
      if (!tostop.load(std::memory_order_acquire))
        variant_stop.store(true, std::memory_order_release);
    } else {
      // Move variant to inactive, as it has exhausted its engine
      unsigned int i=0;
      while (variants[i] != variant)
        i++;
      assert(i < n_active);
      assert(n_active > 0);
      std::swap(variants[i],variants[--n_active]);
      tostop.store(true, std::memory_order_release);
    }
    if (b) {
      if (--n_busy == 0)
        idle.signal();
    }
    m.release();
    return b;
  }

  template<class Collect>
  void
  Variant<Collect>::run(void) {
    Space* s;
    do {
      s = engine->next();
    } while (!portfolio->report(this,s));
  }

  template<class Collect>
  Space*
  PBS<Collect>::next(void) {
    m.acquire();
    if (solutions.empty()) {
      // Clear all
      tostop.store(false, std::memory_order_release);
      variant_stop.store(false, std::memory_order_release);

      // Invariant: all variants are idle!
      assert(n_busy == 0);
      assert(!tostop.load(std::memory_order_acquire));

      if (n_active > 0) {
        // Run all active variants
        n_busy = n_active;
        for (unsigned int i=0U; i<n_active; i++) {
          // Consume the previous completion before reusing this variant.  The
          // initial signal handles the first submission.
          variants[i]->wait();
          Support::Thread::run(variants[i]);
        }
        m.release();
        // Wait for all variants to become idle
        idle.wait();
        m.acquire();
      }
    }

    // Invariant all variants are idle!
    assert(n_busy == 0);

    Space* s;

    // Process solutions
    if (solutions.empty()) {
      s = nullptr;
    } else {
      Variant<Collect>* r;
      s = solutions.get(r);
      if (Collect::best)
        for (unsigned int i=0U; i<n_active; i++)
          if (variants[i] != r)
            variants[i]->constrain(*s);
    }

    m.release();
    return s;
  }

  template<class Collect>
  bool
  PBS<Collect>::stopped(void) const {
    return variant_stop.load(std::memory_order_acquire);
  }

  template<class Collect>
  Statistics
  PBS<Collect>::statistics(void) const {
    assert(n_busy == 0);
    Statistics s(stat);
    for (unsigned int i=0U; i<n_variants; i++)
      s += variants[i]->statistics();
    return s;
  }

  template<class Collect>
  void
  PBS<Collect>::constrain(const Space& b) {
    assert(n_busy == 0);
    if (!Collect::best)
      throw NoBest("PBS::constrain");
    if (solutions.constrain(b)) {
      // The solution is better
      for (unsigned int i=0U; i<n_active; i++)
        variants[i]->constrain(b);
    }
  }

  template<class Collect>
  PBS<Collect>::~PBS(void) {
    // A report can make n_busy zero before Variant::run and Thread::Run::exec
    // have returned.  Wait for the completion handshake before deleting the
    // PBS-owned variants.
    for (unsigned int i=0U; i<n_variants; i++)
      variants[i]->wait();
    assert(n_busy == 0);
    for (unsigned int i=0U; i<n_variants; i++)
      delete variants[i];
    heap.free<Variant<Collect>*>(variants,n_variants);
  }

}}}

// STATISTICS: search-par
