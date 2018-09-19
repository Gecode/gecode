/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <algorithm>

namespace Gecode { namespace Search { namespace Par {


  forceinline
  CollectAll::CollectAll(void)
    : solutions(heap) {}
  forceinline bool
  CollectAll::add(Space* s, Slave<CollectAll>*) {
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
  CollectAll::get(Slave<CollectAll>*&) {
    return solutions.pop();
  }
  forceinline
  CollectAll::~CollectAll(void) {
    while (!solutions.empty())
      delete solutions.pop();
  }


  forceinline
  CollectBest::CollectBest(void)
    : b(NULL), reporter(NULL) {}
  forceinline bool
  CollectBest::add(Space* s, Slave<CollectBest>* r) {
    if (b != NULL) {
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
    if (b != NULL) {
      b->constrain(s);
      if (b->status() == SS_FAILED) {
        delete b;
      } else {
        return false;
      }
    }
    b = s.clone();
    reporter = NULL;
    return true;
  }
  forceinline bool
  CollectBest::empty(void) const {
    return reporter == NULL;
  }
  forceinline Space*
  CollectBest::get(Slave<CollectBest>*& r) {
    assert(!empty());
    r = reporter;
    reporter = NULL;
    return b->clone();
  }
  forceinline
  CollectBest::~CollectBest(void) {
    delete b;
  }


  forceinline
  PortfolioStop::PortfolioStop(Stop* so0)
    : so(so0), tostop(NULL) {}

  forceinline void
  PortfolioStop::share(volatile bool* ts) {
    tostop = ts;
  }


  template<class Collect>
  forceinline
  Slave<Collect>::Slave(PBS<Collect>* m, Engine* s, Stop* so)
    : Support::Runnable(false), master(m), slave(s), stop(so) {}
  template<class Collect>
  forceinline Statistics
  Slave<Collect>::statistics(void) const {
    return slave->statistics();
  }
  template<class Collect>
  forceinline bool
  Slave<Collect>::stopped(void) const {
    return slave->stopped();
  }
  template<class Collect>
  forceinline void
  Slave<Collect>::constrain(const Space& b) {
    slave->constrain(b);
  }
  template<class Collect>
  Slave<Collect>::~Slave(void) {
    delete slave;
    delete stop;
  }



  template<class Collect>
  forceinline
  PBS<Collect>::PBS(Engine** engines, Stop** stops, unsigned int n,
                    const Statistics& stat0)
    : stat(stat0), slaves(heap.alloc<Slave<Collect>*>(n)),
      n_slaves(n), n_active(n),
      slave_stop(false), tostop(false), n_busy(0) {
    // Initialize slaves
    for (unsigned int i=0U; i<n_slaves; i++) {
      slaves[i] = new Slave<Collect>(this,engines[i],stops[i]);
      static_cast<PortfolioStop*>(stops[i])->share(&tostop);
    }
  }


  template<class Collect>
  forceinline bool
  PBS<Collect>::report(Slave<Collect>* slave, Space* s) {
    // If b is false the report should be repeated (solution was worse)
    bool b = true;
    m.acquire();
    if (s != NULL) {
      b = solutions.add(s,slave);
      if (b)
        tostop = true;
    } else if (slave->stopped()) {
      if (!tostop)
        slave_stop = true;
    } else {
      // Move slave to inactive, as it has exhausted its engine
      unsigned int i=0;
      while (slaves[i] != slave)
        i++;
      assert(i < n_active);
      assert(n_active > 0);
      std::swap(slaves[i],slaves[--n_active]);
      tostop = true;
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
  Slave<Collect>::run(void) {
    Space* s;
    do {
      s = slave->next();
    } while (!master->report(this,s));
  }

  template<class Collect>
  Space*
  PBS<Collect>::next(void) {
    m.acquire();
    if (solutions.empty()) {
      // Clear all
      tostop = false;
      slave_stop = false;

      // Invariant: all slaves are idle!
      assert(n_busy == 0);
      assert(!tostop);

      if (n_active > 0) {
        // Run all active slaves
        n_busy = n_active;
        for (unsigned int i=0U; i<n_active; i++)
          Support::Thread::run(slaves[i]);
        m.release();
        // Wait for all slaves to become idle
        idle.wait();
        m.acquire();
      }
    }

    // Invariant all slaves are idle!
    assert(n_busy == 0);

    Space* s;

    // Process solutions
    if (solutions.empty()) {
      s = NULL;
    } else {
      Slave<Collect>* r;
      s = solutions.get(r);
      if (Collect::best)
        for (unsigned int i=0U; i<n_active; i++)
          if (slaves[i] != r)
            slaves[i]->constrain(*s);
    }

    m.release();
    return s;
  }

  template<class Collect>
  bool
  PBS<Collect>::stopped(void) const {
    return slave_stop;
  }

  template<class Collect>
  Statistics
  PBS<Collect>::statistics(void) const {
    assert(n_busy == 0);
    Statistics s(stat);
    for (unsigned int i=0U; i<n_slaves; i++)
      s += slaves[i]->statistics();
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
        slaves[i]->constrain(b);
    }
  }

  template<class Collect>
  PBS<Collect>::~PBS(void) {
    assert(n_busy == 0);
    heap.free<Slave<Collect>*>(slaves,n_slaves);
  }

}}}

// STATISTICS: search-par
