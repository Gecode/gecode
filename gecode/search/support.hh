/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#ifndef GECODE_SEARCH_SUPPORT_HH
#define GECODE_SEARCH_SUPPORT_HH

#include <gecode/search.hh>

namespace Gecode { namespace Search {

    /// Whether incoming solution \a s should replace incumbent \a b
    forceinline bool
    better(const Space& s, const Space& b, const char* l) {
        switch (s.compare(b)) {
        case SpaceComparison::Better:
            return true;
        case SpaceComparison::Equivalent:
        case SpaceComparison::Worse:
            return false;
        case SpaceComparison::Incomparable:
            throw Incomparable(l);
        default:
            GECODE_NEVER;
        }
        return false;
    }

    /// Clone space \a s depending on options \a o
    forceinline Space* snapshot(Space* s, const Options& o);

    /**
     * \brief Whether a portfolio asset whose spoke has coverage \a c and that
     *        ended for reason \a r proves that the hub has no further solution
     *
     * Only an asset that exhausted a spoke of full or reduced coverage ends
     * the portfolio with a proof. An asset that exhausted a spoke of partial
     * coverage, such as a fragment for large neighbourhood search, has proven
     * nothing about the hub and is dropped.
     */
    forceinline bool
    pbs_asset_proof(EndReason r, SpokeCoverage c) {
        return (r == EndReason::Exhausted) && (c != SpokeCoverage::Partial);
    }

    /// Which stop object stopped an asset of a portfolio
    enum class PortfolioStopCause : unsigned char {
        None,      ///< No stop object has stopped the asset
        Slice,     ///< The asset used up its slice of a sequential portfolio
        Interrupt, ///< Another asset of a parallel portfolio interrupted the asset
        Asset,     ///< The stop object of the asset
        Portfolio, ///< The stop object of the portfolio
    };

    /// Virtualize a worker to an engine
    template <class Worker>
    class WorkerToEngine : public Engine {
    protected:
        /// The worker to wrap into an engine
        Worker w;
        /// Why the last call to next() returned no solution
        EndReason end;

    public:
        /// Initialization
        WorkerToEngine(Space* s, const Options& o);
        /// Return next solution (nullptr, if none exists or search has been stopped)
        Space* next(void) override;
        /// Return statistics
        Search::Statistics statistics(void) const override;
        /// Return why the last call to next() returned no solution
        EndReason end_reason(void) const override;
        /// Constrain future solutions to be better than \a b
        void constrain(const Space& b) override;
        /// Reset engine to restart at space \a s
        void reset(Space* s) override;
        /// Return no-goods
        NoGoods& nogoods(void) override;
    };

    forceinline Space*
    snapshot(Space* s, const Options& o) {
        return o.clone ? s->clone() : s;
    }

    template <class Worker>
    WorkerToEngine<Worker>::WorkerToEngine(Space* s, const Options& o)
        : Engine(o, 1U), w(s, o), end(EndReason::NotEnded) {}
    template <class Worker>
    Space*
    WorkerToEngine<Worker>::next(void) {
        Space* s = w.next();
        end = (s != nullptr) ? EndReason::NotEnded : w.end_reason();
        return s;
    }
    template <class Worker>
    Search::Statistics
    WorkerToEngine<Worker>::statistics(void) const {
        return w.statistics();
    }
    template <class Worker>
    EndReason
    WorkerToEngine<Worker>::end_reason(void) const {
        return end;
    }
    template <class Worker>
    void
    WorkerToEngine<Worker>::reset(Space* s) {
        w.reset(s);
        end = EndReason::NotEnded;
    }
    template <class Worker>
    void
    WorkerToEngine<Worker>::constrain(const Space& b) {
        w.constrain(b);
    }
    template <class Worker>
    NoGoods&
    WorkerToEngine<Worker>::nogoods(void) {
        return w.nogoods();
    }

}} // namespace Gecode::Search

#endif

// STATISTICS: search-other
