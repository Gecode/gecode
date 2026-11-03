/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <climits>

namespace Gecode { namespace Search { namespace Seq {


  forceinline
  PortfolioStop::PortfolioStop(Stop* so0, Stop* po0)
    : so(so0), po(po0) {}
  forceinline void
  PortfolioStop::share(SharedStopInfo* ssi0) {
    ssi = ssi0;
  }


  forceinline
  Spoke::Spoke(void)
    : engine(nullptr), stop(nullptr) {}
  forceinline void
  Spoke::init(Engine* e, Stop* s) {
    engine = e; stop = s;
  }
  forceinline Space*
  Spoke::next(void) {
    return engine->next();
  }
  forceinline Statistics
  Spoke::statistics(void) const {
    return engine->statistics();
  }
  forceinline bool
  Spoke::stopped(void) const {
    return engine->stopped();
  }
  forceinline bool
  Spoke::limited(void) const {
#ifdef GECODE_HAS_EXPERIMENTAL
    return engine->limited();
#else
    return false;
#endif
  }
  forceinline void
  Spoke::constrain(const Space& b) {
    engine->constrain(b);
  }
  forceinline
  Spoke::~Spoke(void) {
    delete engine;
    delete stop;
  }


  template<bool best>
  forceinline
  PBS<best>::PBS(Engine** e, Stop** s, unsigned int n,
                 const Statistics& stat0,
                 const Search::Options& opt)
    : stat(stat0), slice(opt.slice),
      spokes(heap.alloc<Spoke>(n)), n_spokes(n), cur(0),
      spoke_stop(false), incomplete(false) {
    ssi.done = false;
    ssi.l = opt.slice;

    for (unsigned int i=0U; i<n; i++) {
      spokes[i].init(e[i],static_cast<PortfolioStop*>(s[i]));
      static_cast<PortfolioStop*>(s[i])->share(&ssi);
    }
  }

  template<bool best>
  Space*
  PBS<best>::next(void) {
    spoke_stop = false;
    unsigned int n_exhausted = 0;
    while (n_spokes > 0) {
      ssi.done = false;
      if (Space* s = spokes[cur].next()) {
        // Constrain other spokes
        if (best) {
          try {
            for (unsigned int i=0U; i<cur; i++)
              spokes[i].constrain(*s);
            for (unsigned int i=cur+1; i<n_spokes; i++)
              spokes[i].constrain(*s);
          } catch (...) {
            delete s;
            throw;
          }
        }
        return s;
      }
      if (spokes[cur].stopped() && !spokes[cur].limited()) {
        if (ssi.done) {
          cur++; n_exhausted++;
        } else {
          spoke_stop = true;
          return nullptr;
        }
      } else {
        // The spoke is done or stopped at a local limit (then search is incomplete)
        if (spokes[cur].limited())
          incomplete = true;
        // Kill the spoke after saving the statistics
        stat += spokes[cur].statistics();
        spokes[cur].~Spoke();
        spokes[cur] = spokes[--n_spokes];
        if (n_spokes == 1)
          // Disable stopping by setting a high limit
          ssi.l = ULONG_MAX;
      }
      if (n_exhausted == n_spokes) {
        n_exhausted = 0;
        // Increment by one slice
        ssi.l += slice;
      }
      if (cur == n_spokes)
        cur = 0;
    }
    spoke_stop = incomplete;
    return nullptr;
  }

  template<bool best>
  bool
  PBS<best>::stopped(void) const {
    return spoke_stop;
  }

  template<bool best>
  Statistics
  PBS<best>::statistics(void) const {
    Statistics s(stat);
    for (unsigned int i=0U; i<n_spokes; i++)
      s += spokes[i].statistics();
    return s;
  }

  template<bool best>
  void
  PBS<best>::constrain(const Space& b) {
    if (!best)
      throw NoBest("PBS::constrain");
    for (unsigned int i=0U; i<n_spokes; i++)
      spokes[i].constrain(b);
  }

  template<bool best>
  PBS<best>::~PBS(void) {
    for (unsigned int i=0U; i<n_spokes; i++)
      spokes[i].~Spoke();
    // Note that n_spokes might be different now!
    heap.rfree(spokes);
  }

}}}

// STATISTICS: search-seq
