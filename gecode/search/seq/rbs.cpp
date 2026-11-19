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

#include <gecode/search/seq/rbs.hh>

#include <algorithm>
#include <memory>

namespace Gecode { namespace Search { namespace Seq {

    namespace {

        /// Return the default decision of hub() for event \a ev after a spoke of coverage \a c
        MetaDecision
        default_decision(MetaEvent ev, SpokeCoverage c) {
            // A new spoke would repeat an incomplete search of a spoke that is not partial
            if ((ev == MetaEvent::Incomplete) && (c != SpokeCoverage::Partial))
                return MetaDecision::Finish;
            return MetaDecision::Restart;
        }

    } // namespace

    bool
    RestartStop::stop(const Statistics& s, const Options& o) {
        Support::Lock lock(m);
        // Stop if the fail limit for the engine says so
        if (s.fail > l) {
            e_stopped = true;
            e_fail = std::max(e_fail, s.fail);
            return true;
        }
        // Stop if the stop object for the meta engine says so
        if ((m_stop != nullptr) && m_stop->stop(m_stat + s, o)) {
            e_stopped = false;
            return true;
        }
        return false;
    }

    bool
    RestartStop::meta_stop(const Statistics& s, const Options& o) {
        Support::Lock lock(m);
        return (m_stop != nullptr) && m_stop->stop(m_stat + s, o);
    }

    NoGoods&
    RBS::hub_nogoods(void) {
        // The no-goods hold in the hub only for a spoke of full coverage
        if (coverage != SpokeCoverage::Full)
            return NoGoods::eng;
        NoGoods& ng = e->nogoods();
        // Reset number of no-goods found
        ng.ng(0);
        return ng;
    }

    bool
    RBS::hub_event(MetaEvent ev) {
        // Constrain the hub with a new solution, whatever hub() does
        if ((ev == MetaEvent::Solution) && best)
            hub->constrain(*last);
        Search::Statistics episode_stat = e->statistics();
        NoGoods& ng = hub_nogoods();
        HubContext hc = MetaContextAccess::hub(MetaEngine::Restart, ev, default_decision(ev, coverage), episode, 0U, 1U,
                                               last, episode_stat, ng);
        hub->hub(hc);
        stop->nogood(ng.ng());
        MetaDecision d = hc.decision();
        if (stop->status(hub) == SS_FAILED) {
            // A failed hub proves that no further solution exists
            stop->update(e->statistics());
            delete hub;
            hub = nullptr;
            e->reset(nullptr);
            end = EndReason::Exhausted;
            return false;
        }
        switch (d) {
        case MetaDecision::Finish:
            end = EndReason::Incomplete;
            return false;
        case MetaDecision::Continue:
            // After a limit, the episode continues with the next cutoff as budget
            if (ev == MetaEvent::Limit)
                stop->extend(++(*co));
            return true;
        case MetaDecision::Restart:
            break;
        default:
            GECODE_NEVER;
        }
        stop->restart();
        // A restart after a solution keeps the cutoff, not an extended limit
        if (ev == MetaEvent::Solution)
            stop->renew(e->statistics());
        else
            stop->limit(e->statistics(), ++(*co));
        // The old hub becomes the spoke, which is deleted if spoke() throws
        std::unique_ptr<Space> next_hub(hub->clone());
        std::unique_ptr<Space> spoke(hub);
        hub = next_hub.release();
        episode++;
        SpokeContext sc = MetaContextAccess::spoke(MetaEngine::Restart, ev, episode, 0U, 1U, last, episode_stat, ng);
        spoke->spoke(sc);
        spoke->random_split(sc);
        hub->reserve_brancher_ids(*spoke);
        coverage = sc.coverage();
        e->reset(spoke.release());
        return true;
    }

    Space*
    RBS::next(void) {
        // A proof or a decision to finish ends the search
        if ((end == EndReason::Exhausted) || (end == EndReason::Incomplete))
            return nullptr;
        if (solution) {
            solution = false;
            if (!hub_event(MetaEvent::Solution))
                return nullptr;
        }
        while (true) {
            Space* n = e->next();
            if (n != nullptr) {
                // The engine found a solution
                solution = true;
                delete last;
                last = n->clone();
                end = EndReason::NotEnded;
                return n;
            }
            MetaEvent ev;
            switch (e->end_reason()) {
            case EndReason::Limit:
                if (!stop->enginestopped()) {
                    // A stop from above
                    end = EndReason::Limit;
                    return nullptr;
                }
                ev = MetaEvent::Limit;
                break;
            case EndReason::Exhausted:
                if (coverage != SpokeCoverage::Partial) {
                    // The spoke covers the hub, so this is a proof
                    end = EndReason::Exhausted;
                    return nullptr;
                }
                ev = MetaEvent::Exhausted;
                break;
            case EndReason::Incomplete:
                ev = MetaEvent::Incomplete;
                break;
            default:
                GECODE_NEVER;
                return nullptr;
            }
            /*
             * The engine might not have checked its stop object, for example
             * when the spoke failed at the root or a parallel engine had no
             * work. Without this check, the restarts might never stop.
             */
            if ((ev != MetaEvent::Limit) && stop->meta_stop(e->statistics(), m_opt)) {
                end = EndReason::Limit;
                return nullptr;
            }
            if (!hub_event(ev))
                return nullptr;
        }
        GECODE_NEVER;
        return nullptr;
    }

    Search::Statistics
    RBS::statistics(void) const {
        return stop->metastatistics() + e->statistics();
    }

    void
    RBS::constrain(const Space& b) {
        if (!best)
            throw NoBest("RBS::constrain");
        if (hub == nullptr)
            return;
        if ((last != nullptr) && !Search::better(b, *last, "RBS::constrain"))
            return;
        delete last;
        last = b.clone();
        hub->constrain(b);
        e->constrain(b);
    }

    EndReason
    RBS::end_reason(void) const {
        /*
         * What might happen during parallel search is that the engine has
         * been stopped by the cutoff but the meta engine reports a limit,
         * because the stop object of the meta engine also fired. The next
         * invocation of next() then performs the restart, and no restart
         * will be missed.
         */
        return end;
    }

    RBS::~RBS(void) {
        delete e;
        delete hub;
        delete last;
        delete co;
        delete stop;
    }

}}} // namespace Gecode::Search::Seq

// STATISTICS: search-seq
