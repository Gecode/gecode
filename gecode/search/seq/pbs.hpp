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

#include <climits>

namespace Gecode { namespace Search { namespace Seq {


  forceinline
  PortfolioStop::PortfolioStop(Stop* so0)
    : so(so0) {}
  forceinline void
  PortfolioStop::share(SharedStopInfo* ssi0) {
    ssi = ssi0;
  }


  forceinline void
  Slave::init(Engine* e, Stop* s) {
    slave = e; stop = s;
  }
  forceinline Space*
  Slave::next(void) {
    return slave->next();
  }
  forceinline Statistics
  Slave::statistics(void) const {
    return slave->statistics();
  }
  forceinline bool
  Slave::stopped(void) const {
    return slave->stopped();
  }
  forceinline void
  Slave::constrain(const Space& b) {
    slave->constrain(b);
  }
  forceinline
  Slave::~Slave(void) {
    delete slave;
    delete stop;
  }


  template<bool best>
  forceinline
  PBS<best>::PBS(Engine** e, Stop** s, unsigned int n,
                 const Statistics& stat0,
                 const Search::Options& opt)
    : stat(stat0), slice(opt.slice),
      slaves(heap.alloc<Slave>(n)), n_slaves(n), cur(0),
      slave_stop(false) {
    ssi.done = false;
    ssi.l = opt.slice;

    for (unsigned int i=0U; i<n; i++) {
      slaves[i].init(e[i],static_cast<PortfolioStop*>(s[i]));
      static_cast<PortfolioStop*>(s[i])->share(&ssi);
    }
  }

  template<bool best>
  Space*
  PBS<best>::next(void) {
    slave_stop = false;
    unsigned int n_exhausted = 0;
    while (n_slaves > 0) {
      if (Space* s = slaves[cur].next()) {
        // Constrain other slaves
        if (best) {
          for (unsigned int i=0U; i<cur; i++)
            slaves[i].constrain(*s);
          for (unsigned int i=cur+1; i<n_slaves; i++)
            slaves[i].constrain(*s);
        }
        return s;
      }
      if (slaves[cur].stopped()) {
        if (ssi.done) {
          cur++; n_exhausted++;
        } else {
          slave_stop = true;
          return NULL;
        }
      } else {
        // This slave is done, kill it after saving the statistics
        stat += slaves[cur].statistics();
        slaves[cur].~Slave();
        slaves[cur] = slaves[--n_slaves];
        if (n_slaves == 1)
          // Disable stoping by seeting a high limit
          ssi.l = ULONG_MAX;
      }
      if (n_exhausted == n_slaves) {
        n_exhausted = 0;
        // Increment by one slice
        ssi.l += slice;
      }
      if (cur == n_slaves)
        cur = 0;
    }
    return NULL;
  }

  template<bool best>
  bool
  PBS<best>::stopped(void) const {
    return slave_stop;
  }

  template<bool best>
  Statistics
  PBS<best>::statistics(void) const {
    Statistics s(stat);
    for (unsigned int i=0U; i<n_slaves; i++)
      s += slaves[i].statistics();
    return s;
  }

  template<bool best>
  void
  PBS<best>::constrain(const Space& b) {
    if (!best)
      throw NoBest("PBS::constrain");
    for (unsigned int i=0U; i<n_slaves; i++)
      slaves[i].constrain(b);
  }

  template<bool best>
  PBS<best>::~PBS(void) {
    for (unsigned int i=0U; i<n_slaves; i++)
      slaves[i].~Slave();
    // Note that n_slaves might be different now!
    heap.rfree(slaves);
  }

}}}

// STATISTICS: search-seq
