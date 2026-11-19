/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2012
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

#ifndef GECODE_SEARCH_SEQ_RBS_HH
#define GECODE_SEARCH_SEQ_RBS_HH

#include <gecode/search.hh>

#include <climits>

namespace Gecode { namespace Search { namespace Seq {

    class RBS;

    /// %Stop-object for restart engine
    class GECODE_SEARCH_EXPORT RestartStop : public Stop {
        template <class, template <class> class>
        friend class ::Gecode::RBS;
        friend class ::Gecode::Search::Seq::RBS;

    private:
        /// Mutex protecting restart stop state
        mutable Support::Mutex m;
        /// The failure limit for the engine
        unsigned long long int l;
        /// The last cutoff, the failure budget of the current episode
        unsigned long long int c;
        /// The stop object for the meta engine
        Stop* m_stop;
        /// Whether the engine was stopped
        bool e_stopped;
        /// The largest failure count of a worker that the limit stopped
        unsigned long long int e_fail;
        /// Accumulated statistics for the meta engine
        Statistics m_stat;

    public:
        /// Stop the meta engine if indicated by the stop object \a s
        RestartStop(Stop* s);
        /// Return true if meta engine must be stopped
        virtual bool stop(const Statistics& s, const Options& o);
        /// Return true if the stop object for the meta engine says so
        bool meta_stop(const Statistics& s, const Options& o);
        /// Increment current restart count
        void restart(void);
        /// Add no-goods to meta statistics
        void nogood(unsigned long int n);
        /// Test hub status with meta statistics
        SpaceStatus status(Space* s);
        /// Add statistics \a s and start an episode with cutoff \a c
        void limit(const Statistics& s, unsigned long long int c);
        /// Add statistics \a s and start an episode with the last cutoff
        void renew(const Statistics& s);
        /**
         * \brief Give the engine, which continues its search, a budget of \a c fails
         *
         * The budget counts from the failures of the worker that the limit
         * stopped, as stop() compares the failures of each worker with the
         * limit. The limit saturates instead of wrapping around.
         */
        void extend(unsigned long long int c);
        /// Update statistics
        void update(const Search::Statistics& s);
        /// Return whether the engine has been stopped
        bool enginestopped(void) const;
        /// Return statistics for the meta engine
        Statistics metastatistics(void) const;
    };

    /// Engine for restart-based search
    class GECODE_SEARCH_EXPORT RBS : public Engine {
    protected:
        /// The actual engine
        Engine* e;
        /// The hub space to restart from
        Space* hub;
        /// The last solution space (possibly nullptr)
        Space* last;
        /// The cutoff object
        Cutoff* co;
        /// The stop control object
        RestartStop* stop;
        /// The options for the stop object of the meta engine
        Options m_opt;
        /// The current episode
        unsigned long int episode;
        /// The coverage of the spoke of the current episode
        SpokeCoverage coverage;
        /// Whether the last call to next() returned a solution that hub() has not seen
        bool solution;
        /// Whether the engine performs best solution search
        bool best;
        /// Why the last call to next() returned no solution
        EndReason end;
        /// Return the no-goods of the last spoke search, valid in the hub
        NoGoods& hub_nogoods(void);
        /// Call hub() for event \a ev and act on its decision, return whether search continues
        bool hub_event(MetaEvent ev);

    public:
        /**
         * \brief Constructor
         *
         * The value \a coverage is the coverage that the spoke hook declared
         * for the space explored by the first engine \a e0.
         */
        RBS(Space* s, RestartStop* stop0, Engine* e0, const Search::Statistics& stat, const Options& o, bool best,
            SpokeCoverage coverage);
        /// Return next solution (nullptr, if none exists or search has been stopped)
        virtual Space* next(void);
        /// Return statistics
        virtual Statistics statistics(void) const;
        /// Return why the last call to next() returned no solution
        virtual EndReason end_reason(void) const;
        /// Constrain future solutions to be better than \a b
        virtual void constrain(const Space& b);
        /// Destructor
        virtual ~RBS(void);
    };

}}} // namespace Gecode::Search::Seq

#include <gecode/search/seq/rbs.hpp>

#endif

// STATISTICS: search-seq
