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
        : engine(nullptr), stop(nullptr), _coverage(SpokeCoverage::Full), _parked(false) {}
    forceinline void
    Asset::init(Engine* e, Stop* so, SpokeCoverage c) {
        engine = e;
        stop = so;
        _coverage = c;
    }
    forceinline SpokeCoverage
    Asset::coverage(void) const {
        return _coverage;
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
    forceinline void
    Asset::park(void) {
        _parked = true;
    }
    forceinline void
    Asset::resume(void) {
        _parked = false;
    }
    forceinline bool
    Asset::parked(void) const {
        return _parked;
    }
    forceinline Asset::~Asset(void) {
        delete engine;
        delete stop;
    }

    template <bool best>
    forceinline
    PBS<best>::PBS(Engine** e, Stop** s, const SpokeCoverage* c, unsigned int n, const Statistics& stat0,
                   const Search::Options& opt)
        : stat(stat0), slice(opt.slice), assets(heap.alloc<Asset>(n)), n_assets(n), cur(0), end(EndReason::NotEnded) {
        ssi.cause = PortfolioStopCause::None;
        ssi.l = opt.slice;

        for (unsigned int i = 0U; i < n; i++) {
            assets[i].init(e[i], static_cast<PortfolioStop*>(s[i]), c[i]);
            static_cast<PortfolioStop*>(s[i])->share(&ssi);
        }
    }

    template <bool best>
    Space*
    PBS<best>::next(void) {
        // An asset has proven that no further solution exists
        if (end == EndReason::Exhausted)
            return nullptr;
        // Resume the assets that their own stop objects stopped
        for (unsigned int i = 0U; i < n_assets; i++)
            assets[i].resume();
        // Number of assets that used up their slice or are parked
        unsigned int n_sliced = 0;
        unsigned int n_parked = 0;
        while (n_assets > 0) {
            if (assets[cur].parked()) {
                if (++cur == n_assets)
                    cur = 0;
                continue;
            }
            ssi.cause = PortfolioStopCause::None;
            ssi.others = stat;
            for (unsigned int i = 0U; i < n_assets; i++)
                if (i != cur)
                    ssi.others += assets[i].statistics();
            if (Space* s = assets[cur].next()) {
                // Constrain other assets
                if (best) {
                    try {
                        for (unsigned int i = 0U; i < cur; i++)
                            assets[i].constrain(*s);
                        for (unsigned int i = cur + 1; i < n_assets; i++)
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
                if (ssi.cause == PortfolioStopCause::Slice) {
                    n_sliced++;
                } else if (ssi.cause == PortfolioStopCause::Portfolio) {
                    end = EndReason::Limit;
                    return nullptr;
                } else {
                    // The stop object of the asset stopped it, the others continue
                    assets[cur].park();
                    n_parked++;
                }
                cur++;
            } else if (pbs_asset_proof(asset_end, assets[cur].coverage())) {
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
            if (cur == n_assets)
                cur = 0;
            if ((n_assets > 0) && (n_parked == n_assets)) {
                // Only parked assets remain, they resume with the next call
                end = EndReason::Limit;
                return nullptr;
            }
            if (n_sliced + n_parked == n_assets) {
                n_sliced = 0;
                // Increment by one slice
                ssi.l += slice;
            }
        }
        // All assets ended without a proof
        end = EndReason::Incomplete;
        return nullptr;
    }

    template <bool best>
    EndReason
    PBS<best>::end_reason(void) const {
        return end;
    }

    template <bool best>
    Statistics
    PBS<best>::statistics(void) const {
        Statistics s(stat);
        for (unsigned int i = 0U; i < n_assets; i++)
            s += assets[i].statistics();
        return s;
    }

    template <bool best>
    void
    PBS<best>::constrain(const Space& b) {
        if (!best)
            throw NoBest("PBS::constrain");
        for (unsigned int i = 0U; i < n_assets; i++)
            assets[i].constrain(b);
    }

    template <bool best>
    PBS<best>::~PBS(void) {
        for (unsigned int i = 0U; i < n_assets; i++)
            assets[i].~Asset();
        // Note that n_assets might be different now!
        heap.rfree(assets);
    }

}}} // namespace Gecode::Search::Seq

// STATISTICS: search-seq
