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

#ifndef GECODE_SEARCH_SEQ_PBS_HH
#define GECODE_SEARCH_SEQ_PBS_HH

#include <gecode/search.hh>
#include <gecode/search/support.hh>

namespace Gecode { namespace Search { namespace Seq {

    /**
     * \brief Shared stop information
     *
     * A worker of a parallel asset can check its stop object after next()
     * of the asset has returned, while the portfolio already prepares the
     * next asset.
     */
    class SharedStopInfo {
    public:
        /// Which stop object stopped the running asset, its workers can run in parallel
        std::atomic<PortfolioStopCause> cause;
        /// The current failure limit, incremented for each slice
        std::atomic<unsigned long long int> l;
        /// Mutex protecting others
        Support::Mutex m;
        /// Statistics of the portfolio except for the running asset
        Statistics others;
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
        /// Pointer to shared stop information
        SharedStopInfo* ssi;

    public:
        /// Initialize with asset stop \a so and portfolio stop \a po
        PortfolioStop(Stop* so, Stop* po = nullptr);
        /// Initialize shared stop information
        void share(SharedStopInfo* ssi);
        /// Whether the portfolio has a stop object
        bool portfolio_stop() const;
        /// Return true if portfolio engine must be stopped
        bool stop(const Statistics& s, const Options& o) override;
    };

    /// Asset engine in a sequential portfolio
    class Asset {
    protected:
        /// The wrapped search engine
        Engine* engine;
        /// Stop object
        Stop* stop;
        /// Coverage of the spoke of the asset
        SpokeCoverage _coverage;
        /// Whether the stop object of the asset stopped it during the current call to next()
        bool _parked;

    public:
        /// Default constructor
        Asset();
        /// Copy constructor
        Asset(const Asset& s) = default;
        /// Assignment operator
        Asset& operator=(const Asset& s) = default;
        /// Initialize with engine \a e, its stop object \a so, and the coverage \a c of its spoke
        void init(Engine* e, Stop* so, SpokeCoverage c);
        /// Return the coverage of the spoke of the asset
        SpokeCoverage coverage() const;
        /// Return next solution
        Space* next();
        /// Return statistics of asset
        Statistics statistics() const;
        /// Return why the last call to next() of the asset returned no solution
        EndReason end_reason() const;
        /// Whether the portfolio has a stop object
        bool portfolio_stop() const;
        /// Constrain with better solution \a b
        void constrain(const Space& b);
        /// Park the asset after its stop object stopped it
        void park();
        /// Resume a parked asset
        void resume();
        /// Whether the asset is parked
        bool parked() const;
        /// Delete asset
        ~Asset();
    };

    /// Sequential portfolio engine implementation
    template <bool best>
    class GECODE_SEARCH_EXPORT PBS : public Engine {
    protected:
        /// Statistics of propagating the model and of removed assets
        Statistics stat;
        /// Stop information shared by the assets
        SharedStopInfo ssi;
        /// Size of a slice
        unsigned int slice;
        /// Assets
        Asset* assets;
        /// Number of asset engines
        unsigned int n_assets;
        /// Current asset to run
        unsigned int cur;
        /// Why the last call to next() returned no solution
        EndReason end;

    public:
        /// Initialize with \a n assets, their stop objects, and the coverage of their spokes
        PBS(Engine** assets, Stop** stops, const SpokeCoverage* coverage, unsigned int n, const Statistics& stat,
            const Search::Options& opt);
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

}}} // namespace Gecode::Search::Seq

#include <gecode/search/seq/pbs.hpp>

#endif

// STATISTICS: search-seq
