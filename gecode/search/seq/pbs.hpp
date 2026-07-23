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
  PortfolioStop::PortfolioStop(Stop* so0)
    : so(so0) {}
  forceinline void
  PortfolioStop::share(SharedStopInfo* ssi0) {
    ssi = ssi0;
  }


  forceinline
  Variant::Variant(void)
    : engine(nullptr), stop(nullptr) {}
  forceinline void
  Variant::init(Engine* e, Stop* s) {
    engine = e; stop = s;
  }
  forceinline Space*
  Variant::next(void) {
    return engine->next();
  }
  forceinline Statistics
  Variant::statistics(void) const {
    return engine->statistics();
  }
  forceinline bool
  Variant::stopped(void) const {
    return engine->stopped();
  }
  forceinline void
  Variant::constrain(const Space& b) {
    engine->constrain(b);
  }
  forceinline
  Variant::~Variant(void) {
    delete engine;
    delete stop;
  }


  template<bool best>
  forceinline
  PBS<best>::PBS(Engine** e, Stop** s, unsigned int n,
                 const Statistics& stat0,
                 const Search::Options& opt)
    : stat(stat0), slice(opt.slice),
      variants(heap.alloc<Variant>(n)), n_variants(n), cur(0),
      variant_stop(false) {
    ssi.done = false;
    ssi.l = opt.slice;

    for (unsigned int i=0U; i<n; i++) {
      variants[i].init(e[i],static_cast<PortfolioStop*>(s[i]));
      static_cast<PortfolioStop*>(s[i])->share(&ssi);
    }
  }

  template<bool best>
  Space*
  PBS<best>::next(void) {
    variant_stop = false;
    unsigned int n_exhausted = 0;
    while (n_variants > 0) {
      if (Space* s = variants[cur].next()) {
        // Constrain other variants
        if (best) {
          for (unsigned int i=0U; i<cur; i++)
            variants[i].constrain(*s);
          for (unsigned int i=cur+1; i<n_variants; i++)
            variants[i].constrain(*s);
        }
        return s;
      }
      if (variants[cur].stopped()) {
        if (ssi.done) {
          cur++; n_exhausted++;
        } else {
          variant_stop = true;
          return nullptr;
        }
      } else {
        // This variant is done, kill it after saving the statistics
        stat += variants[cur].statistics();
        variants[cur].~Variant();
        variants[cur] = variants[--n_variants];
        if (n_variants == 1)
          // Disable stopping by setting a high limit
          ssi.l = ULONG_MAX;
      }
      if (n_exhausted == n_variants) {
        n_exhausted = 0;
        // Increment by one slice
        ssi.l += slice;
      }
      if (cur == n_variants)
        cur = 0;
    }
    return nullptr;
  }

  template<bool best>
  bool
  PBS<best>::stopped(void) const {
    return variant_stop;
  }

  template<bool best>
  Statistics
  PBS<best>::statistics(void) const {
    Statistics s(stat);
    for (unsigned int i=0U; i<n_variants; i++)
      s += variants[i].statistics();
    return s;
  }

  template<bool best>
  void
  PBS<best>::constrain(const Space& b) {
    if (!best)
      throw NoBest("PBS::constrain");
    for (unsigned int i=0U; i<n_variants; i++)
      variants[i].constrain(b);
  }

  template<bool best>
  PBS<best>::~PBS(void) {
    for (unsigned int i=0U; i<n_variants; i++)
      variants[i].~Variant();
    // Note that n_variants might be different now!
    heap.rfree(variants);
  }

}}}

// STATISTICS: search-seq
