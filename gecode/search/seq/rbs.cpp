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
    // The no-goods of an incomplete spoke search only hold in its restricted
    // search space, not in the hub.
    if (!nogoods_valid)
      return NoGoods::eng;
    NoGoods& ng = e->nogoods();
    // Reset number of no-goods found
    ng.ng(0);
    return ng;
  }

  Space*
  RBS::next(void) {
    // A failed hub proves that no further solution exists
    if (hub == nullptr) {
      end = EndReason::Exhausted;
      return nullptr;
    }
    if (restart) {
      restart = false;
      sslr++;
      NoGoods& ng = hub_nogoods();
      MetaInfo mi(stop->restarts(),MetaInfo::RR_SOL,sslr,e->statistics().fail,last,ng);
      bool r = hub->hub(mi);
      stop->nogood(ng.ng());
      if (stop->status(hub) == SS_FAILED) {
        stop->update(e->statistics());
        delete hub;
        hub = nullptr;
        e->reset(nullptr);
        end = EndReason::Exhausted;
        return nullptr;
      } else if (r) {
        stop->update(e->statistics());
        Space* spoke = hub;
        hub = hub->clone();
        complete = spoke->spoke(mi);
        nogoods_valid = complete;
        e->reset(spoke);
        sslr = 0;
        stop->restart();
      }
    }
    while (true) {
      Space* n = e->next();
      if (n != nullptr) {
        // The engine found a solution
        restart = true;
        delete last;
        last = n->clone();
        end = EndReason::NotEnded;
        return n;
      }
      EndReason inner = e->end_reason();
      bool limit = inner == EndReason::Limit;
      if ((!complete && !limit) || (limit && stop->enginestopped())) {
        // The engine must perform a true restart
        // The number of the restart has been incremented in the stop object
        if (!complete && !limit) {
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
        sslr = 0;
        NoGoods& ng = hub_nogoods();
        MetaInfo mi(stop->restarts(),limit ? MetaInfo::RR_LIM : MetaInfo::RR_CMPL,sslr,e->statistics().fail,last,ng);
        (void) hub->hub(mi);
        stop->nogood(ng.ng());
        unsigned long long int nl = ++(*co);
        stop->limit(e->statistics(),nl);
        if (stop->status(hub) == SS_FAILED) {
          // The statistics of the engine have been added by limit()
          delete hub;
          hub = nullptr;
          e->reset(nullptr);
          end = EndReason::Exhausted;
          return nullptr;
        }
        Space* spoke = hub;
        hub = hub->clone();
        complete = spoke->spoke(mi);
        nogoods_valid = complete;
        e->reset(spoke);
      } else {
        // A stop from above, or a complete spoke search has ended
        end = inner;
        return nullptr;
      }
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
