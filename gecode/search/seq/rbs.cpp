/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

namespace Gecode { namespace Search { namespace Seq {

  bool
  RestartStop::stop(const Statistics& s, const Options& o) {
    Support::Lock lock(m);
    // Stop if the fail limit for the engine says so
    if (s.fail > l) {
      if (!e_stopped)
        m_stat.restart++;
      e_stopped = true;
      return true;
    }
    // Stop if the stop object for the meta engine says so
    if ((m_stop != nullptr) && m_stop->stop(m_stat+s,o)) {
      e_stopped = false;
      return true;
    }
    return false;
  }

  bool
  RestartStop::meta_stop(const Statistics& s, const Options& o) {
    Support::Lock lock(m);
    return (m_stop != nullptr) && m_stop->stop(m_stat+s,o);
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
    Search::Statistics episode_stat = e->statistics();
    NoGoods& ng = hub_nogoods();
    HubContext hc = MetaContextAccess::hub(MetaEngine::Restart,ev,episode,
                                           0U,1U,last,episode_stat,ng);
    hub->hub(hc);
    stop->nogood(ng.ng());
    // Only an episode that found a solution can continue
    bool cont = (ev == MetaEvent::Solution) &&
      (hc.decision() == MetaDecision::Continue);
    // Any other episode ends with a restart that takes the next cutoff
    if (ev != MetaEvent::Solution)
      stop->limit(e->statistics(),++(*co));
    if (stop->status(hub) == SS_FAILED) {
      // A failed hub proves that no further solution exists
      if (ev == MetaEvent::Solution)
        stop->update(e->statistics());
      delete hub;
      hub = nullptr;
      e->reset(nullptr);
      end = EndReason::Exhausted;
      return false;
    }
    if (cont)
      return true;
    if (ev == MetaEvent::Solution) {
      // A restart after a solution keeps the cutoff
      stop->update(e->statistics());
      stop->restart();
    }
    Space* spoke = hub;
    hub = hub->clone();
    episode++;
    SpokeContext sc = MetaContextAccess::spoke(MetaEngine::Restart,ev,episode,
                                               0U,1U,last,episode_stat,ng);
    spoke->spoke(sc);
    coverage = sc.coverage();
    e->reset(spoke);
    return true;
  }

  Space*
  RBS::next(void) {
    // A failed hub proves that no further solution exists
    if (hub == nullptr) {
      end = EndReason::Exhausted;
      return nullptr;
    }
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
        // The stop object has counted the restart
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
        if (coverage != SpokeCoverage::Partial) {
          // A new spoke would repeat the same incomplete search
          end = EndReason::Incomplete;
          return nullptr;
        }
        ev = MetaEvent::Incomplete;
        break;
      default:
        GECODE_NEVER;
        return nullptr;
      }
      if (ev != MetaEvent::Limit) {
        /*
         * The engine might not have checked its stop object, for example
         * when the spoke failed at the root or a parallel engine had no
         * work. Without this check, the restarts might never stop.
         */
        if (stop->meta_stop(e->statistics(),m_opt)) {
          end = EndReason::Limit;
          return nullptr;
        }
        stop->restart();
      }
      if (!hub_event(ev))
        return nullptr;
    }
    GECODE_NEVER;
    return nullptr;
  }

  Search::Statistics
  RBS::statistics(void) const {
    return stop->metastatistics()+e->statistics();
  }

  void
  RBS::constrain(const Space& b) {
    if (!best)
      throw NoBest("RBS::constrain");
    if (hub == nullptr)
      return;
    if ((last != nullptr) && !Search::better(b,*last,"RBS::constrain"))
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

}}}

// STATISTICS: search-seq
