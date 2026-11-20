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

#include <gecode/search/par/pbs.hh>
#include <gecode/search/worker-control.hh>

namespace Gecode { namespace Search { namespace Par {

    Statistics
    PortfolioStatistics::update(unsigned int i, const Statistics& s) {
        Support::Lock lock(m);
        stat[i + 1] = s;
        Statistics t;
        for (unsigned int j = 0U; j < n; j++)
            t += stat[j];
        return t;
    }

    bool
    PortfolioStop::stop(const Statistics& s, const Options& o) {
        if (tostop->load(std::memory_order_acquire)) {
            _cause.store(PortfolioStopCause::Interrupt, std::memory_order_relaxed);
            return true;
        }
        if ((po != nullptr) && po->stop(ps->update(asset, s), o)) {
            _cause.store(PortfolioStopCause::Portfolio, std::memory_order_relaxed);
            return true;
        }
        if ((so != nullptr) && so->stop(s, o)) {
            _cause.store(PortfolioStopCause::Asset, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    void
    PortfolioStop::wake(void) {
        WorkerControlAccess::signal_all(worker_control);
    }

}}} // namespace Gecode::Search::Par

// STATISTICS: search-par
