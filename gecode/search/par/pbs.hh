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

#ifndef GECODE_SEARCH_PAR_PBS_HH
#define GECODE_SEARCH_PAR_PBS_HH

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <atomic>
#include <exception>

namespace Gecode { namespace Search { namespace Par {

    /**
     * \brief Statistics of the assets of a parallel portfolio
     *
     * Each asset records the statistics that its engine passes to its stop
     * object. For an asset with a parallel engine, these are the statistics
     * of the worker that checks the stop object.
     */
    class GECODE_SEARCH_EXPORT PortfolioStatistics {
    private:
        /// Mutex for access
        Support::Mutex m;
        /// Number of entries: propagating the model and one for each asset
        unsigned int n;
        /// Statistics of propagating the model followed by those of each asset
        Statistics* stat;

    public:
        /// Initialize with statistics \a s of propagating the model for \a n assets
        PortfolioStatistics(const Statistics& s, unsigned int n);
        /// Record statistics \a s of asset \a i and return those of the portfolio
        Statistics update(unsigned int i, const Statistics& s);
        /// Delete
        ~PortfolioStatistics();
    };

    /**
     * \brief Stop object used for controlling assets in a portfolio
     *
     * The stop object of the asset is checked with the statistics of the
     * asset, the stop object of the portfolio with the statistics of the
     * whole portfolio.
     */
    class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
    private:
        /// The stop object for the assets
        Stop* so;
        /// The stop object for the portfolio (possibly nullptr)
        Stop* po;
        /// Worker control for waking a paused asset
        WorkerControl worker_control;
        /// Whether search must be stopped
        std::atomic<bool>* tostop;
        /// Statistics of the portfolio
        PortfolioStatistics* ps;
        /// Index of the asset
        unsigned int asset;
        /// Which stop object stopped the asset
        std::atomic<PortfolioStopCause> _cause;

    public:
        /// Initialize with asset stop \a so, portfolio stop \a po, and \a control
        PortfolioStop(Stop* so, Stop* po, const WorkerControl& control);
        /// Share stop flag \a ts and statistics \a ps of the portfolio with asset \a i
        void share(std::atomic<bool>* ts, PortfolioStatistics* ps, unsigned int i);
        /// Return true if portfolio engine must be stopped
        bool stop(const Statistics& s, const Options& o) override;
        /// Record the final statistics \a s of the asset with the portfolio
        void record(const Statistics& s);
        /// Wake workers so that a paused asset can observe the portfolio stop
        void wake();
        /// Forget which stop object stopped the asset
        void reset();
        /// Return which stop object stopped the asset
        PortfolioStopCause cause() const;
        /// Signal whether search must be stopped
        void stop(bool s);
        /// Whether search must be stopped
        bool stop() const;
    };

    // Forward declaration
    template <class Collect>
    class PBS;

    /// Runnable asset in a portfolio
    template <class Collect>
    class GECODE_SEARCH_EXPORT Asset : public Support::Runnable {
    protected:
        /// Completion event for the current run
        class Completion : public Support::Terminator {
        protected:
            /// The completion event
            Support::Event done;

        public:
            /// Initialize as completed
            Completion();
            /// Signal completion
            void terminated() override;
            /// Wait for completion and consume the signal
            void wait();
        } completion;
        /// The portfolio engine
        PBS<Collect>* portfolio;
        /// The wrapped search engine
        Engine* engine;
        /// Stop object
        Stop* stop;
        /// Coverage of the spoke of the asset
        SpokeCoverage _coverage;
        /// Whether the stop object of the asset stopped it during the current call to next()
        bool _parked;

    public:
        /// Initialize with portfolio \a p, engine \a e, its stop object \a so, and the coverage \a c of its spoke
        Asset(PBS<Collect>* p, Engine* e, Stop* so, SpokeCoverage c);
        /// Return the coverage of the spoke of the asset
        SpokeCoverage coverage() const;
        /// Return statistics of asset
        Statistics statistics() const;
        /// Return why the last call to next() of the asset returned no solution
        EndReason end_reason() const;
        /// Return the completion terminator
        Support::Terminator* terminator() const override;
        /// Wait for the asset to complete its current run
        void wait();
        /// Restore the completion of a run that could not be started
        void restore();
        /// Record the final statistics of the asset with the portfolio
        void record();
        /// Wake a paused asset so that it can observe the portfolio stop
        void wake();
        /// Constrain with better solution \a b
        void constrain(const Space& b);
        /// Return which stop object stopped the asset
        PortfolioStopCause cause() const;
        /// Set whether the asset is parked
        void parked(bool p);
        /// Whether the asset is parked
        bool parked() const;
        /// Perform one run
        void run() override;
        /// Delete asset
        ~Asset() override;
    };

    /// Collect all solutions
    class CollectAll {
    protected:
        /// Queue of solutions
        Support::DynamicQueue<Space*, Heap> solutions;

    public:
        /// Whether it collects best solutions
        static const bool best = false;
        /// Initialize
        CollectAll();
        /// Add a solution \a a reported by \a r and always return true
        bool add(Space* s, Asset<CollectAll>* r);
        /// Dummy function
        bool constrain(const Space& b);
        /// Check whether there is any solution left
        bool empty() const;
        /// Return solution reported by \a r
        Space* get(Asset<CollectAll>*& r);
        /// Destructor
        ~CollectAll();
    };

    /// Collect best solutions
    class CollectBest {
    protected:
        /// Currently best solution
        Space* b;
        /// Who has reported the best solution (nullptr if solution has already been reported)
        Asset<CollectBest>* reporter;

    public:
        /// Whether it collects best solutions
        static const bool best = true;
        /// Initialize
        CollectBest();
        /// Add a solution \a s by \a r and return whether is was better
        bool add(Space* s, Asset<CollectBest>* r);
        /// Check whether \a b better and update accordingly
        bool constrain(const Space& b);
        /// Check whether there is any solution left
        bool empty() const;
        /// Return solution reported by \a r (only if a better one was found)
        Space* get(Asset<CollectBest>*& r);
        /// Destructor
        ~CollectBest();
    };

    /// Parallel portfolio engine implementation
    template <class Collect>
    class GECODE_SEARCH_EXPORT PBS : public Engine {
        friend class Asset<Collect>;

    protected:
        /// Statistics of propagating the model
        Statistics stat;
        /// Statistics of the portfolio for its stop object
        PortfolioStatistics portfolio_stat;
        /// Asset engines
        Asset<Collect>** assets;
        /// Number of asset engines
        unsigned int n_assets;
        /// Number of active asset engines
        unsigned int n_active;
        /// Whether the stop object of the portfolio has stopped an asset
        std::atomic<bool> portfolio_stop;
        /// Whether an asset has proven that no further solution exists
        bool exhausted;
        /// Why the last call to next() returned no solution
        EndReason end;
        /// Shared stop flag
        std::atomic<bool> tostop;
        /// Collect solutions in this
        Collect solutions;
        /// Mutex for synchronization
        Support::Mutex m;
        /// Number of busy assets
        unsigned int n_busy;
        /// Signal that number of busy assets becomes zero
        Support::Event idle;
        /// Failure raised while running or updating a portfolio asset
        std::exception_ptr failure;
        /// Process report from asset, return false if solution was ignored
        bool report(Asset<Collect>* asset, Space* s);
        /// Process failure from asset
        void fail();
        /**
         * \brief Handle that asset \a i could not be started
         *
         * Of the \a n assets that the current round did not start, the first
         * is \a i. Must be called with the mutex held, from the handler of the
         * exception, and returns with the mutex released once the started
         * assets are idle.
         */
        void launch_failed(unsigned int i, unsigned int n);
        /**
         * The key invariant of the engine is as follows:
         *  - n_busy is always zero outside the next() function.
         *  - that entails, that locking is only needed inside next().
         *  - the assets 0..n_active-1 still might not have exhausted their
         *    search space, the parked ones among them are not run until the
         *    next call to next().
         *  - the assets n_active..n_assets-1 have ended without a proof.
         */
    public:
        /// Initialize with \a n assets, their stop objects, and the coverage of their spokes
        PBS(Engine** s, Stop** so, const SpokeCoverage* c, unsigned int n, const Statistics& stat);
        /// Return next solution (nullptr, if none exists or search has been stopped)
        Space* next() override;
        /// Return statistics
        Statistics statistics() const override;
        /// Return why the last call to next() returned no solution
        EndReason end_reason() const override;
        /// Constrain future solutions to be better than \a b
        void constrain(const Space& b) override;
        /// Destructor
        ~PBS() override;
    };

}}} // namespace Gecode::Search::Par

#include <gecode/search/par/pbs.hpp>

#endif

// STATISTICS: search-par
