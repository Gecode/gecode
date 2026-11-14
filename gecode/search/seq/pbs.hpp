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
  Asset::Asset(void)
    : engine(nullptr), stop(nullptr) {}
  forceinline void
  Asset::init(Engine* e, Stop* so) {
    engine = e; stop = so;
  }
  forceinline Space*
  Asset::next(void) {
    return engine->next();
  }
  forceinline Statistics
  Asset::statistics(void) const {
    return engine->statistics();
  }
  forceinline EndReason
  Asset::end_reason(void) const {
    return engine->end_reason();
  }
  forceinline void
  Asset::constrain(const Space& b) {
    engine->constrain(b);
  }
  forceinline
  Asset::~Asset(void) {
    delete engine;
    delete stop;
  }


  template<bool best>
  forceinline
  PBS<best>::PBS(Engine** e, Stop** s, unsigned int n,
                 const Statistics& stat0,
                 const Search::Options& opt)
    : stat(stat0), slice(opt.slice),
      assets(heap.alloc<Asset>(n)), n_assets(n), cur(0),
      end(EndReason::NotEnded) {
    ssi.done = false;
    ssi.l = opt.slice;

    for (unsigned int i=0U; i<n; i++) {
      assets[i].init(e[i],static_cast<PortfolioStop*>(s[i]));
      static_cast<PortfolioStop*>(s[i])->share(&ssi);
    }
  }

  template<bool best>
  Space*
  PBS<best>::next(void) {
    // An asset has proven that no further solution exists
    if (end == EndReason::Exhausted)
      return nullptr;
    unsigned int n_exhausted = 0;
    while (n_assets > 0) {
      ssi.done = false;
      if (Space* s = assets[cur].next()) {
        // Constrain other assets
        if (best) {
          try {
            for (unsigned int i=0U; i<cur; i++)
              assets[i].constrain(*s);
            for (unsigned int i=cur+1; i<n_assets; i++)
              assets[i].constrain(*s);
          } catch (...) {
            delete s;
            throw;
          }
        }
        end = EndReason::NotEnded;
        return s;
      }
      EndReason asset_end = assets[cur].end_reason();
      if (asset_end == EndReason::Limit) {
        if (ssi.done) {
          cur++; n_exhausted++;
        } else {
          end = EndReason::Limit;
          return nullptr;
        }
      } else if (pbs_asset_proof(asset_end)) {
        end = EndReason::Exhausted;
        return nullptr;
      } else {
        // The asset ended without a proof, kill it after saving the statistics
        stat += assets[cur].statistics();
        assets[cur].~Asset();
        assets[cur] = assets[--n_assets];
        if (n_assets == 1)
          // Disable stopping by setting a high limit
          ssi.l = ULONG_MAX;
      }
      if (n_exhausted == n_assets) {
        n_exhausted = 0;
        // Increment by one slice
        ssi.l += slice;
      }
      if (cur == n_assets)
        cur = 0;
    }
    // All assets ended without a proof
    end = EndReason::Incomplete;
    return nullptr;
  }

  template<bool best>
  EndReason
  PBS<best>::end_reason(void) const {
    return end;
  }

  template<bool best>
  Statistics
  PBS<best>::statistics(void) const {
    Statistics s(stat);
    for (unsigned int i=0U; i<n_assets; i++)
      s += assets[i].statistics();
    return s;
  }

  template<bool best>
  void
  PBS<best>::constrain(const Space& b) {
    if (!best)
      throw NoBest("PBS::constrain");
    for (unsigned int i=0U; i<n_assets; i++)
      assets[i].constrain(b);
  }

  template<bool best>
  PBS<best>::~PBS(void) {
    for (unsigned int i=0U; i<n_assets; i++)
      assets[i].~Asset();
    // Note that n_assets might be different now!
    heap.rfree(assets);
  }

}}}

// STATISTICS: search-seq
