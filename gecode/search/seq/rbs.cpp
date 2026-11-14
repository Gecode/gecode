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
  RBS::master_nogoods(void) {
    // The no-goods of an incomplete slave search only hold in its restricted
    // search space, not in the master.
    if (!nogoods_valid)
      return NoGoods::eng;
    NoGoods& ng = e->nogoods();
    // Reset number of no-goods found
    ng.ng(0);
    return ng;
  }

  Space*
  RBS::next(void) {
    // A failed master proves that no further solution exists
    if (master == nullptr)
      return nullptr;
    m_stopped = false;
    if (restart) {
      restart = false;
      sslr++;
      NoGoods& ng = master_nogoods();
      MetaInfo mi(stop->restarts(),MetaInfo::RR_SOL,sslr,e->statistics().fail,last,ng);
      bool r = master->master(mi);
      stop->nogood(ng.ng());
      if (stop->status(master) == SS_FAILED) {
        stop->update(e->statistics());
        delete master;
        master = nullptr;
        e->reset(nullptr);
        return nullptr;
      } else if (r) {
        stop->update(e->statistics());
        Space* slave = master;
        master = master->clone();
        complete = slave->slave(mi);
        nogoods_valid = complete;
        e->reset(slave);
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
        return n;
      } else if ( (!complete && !e->stopped()) ||
                  (e->stopped() && stop->enginestopped()) ) {
        // The engine must perform a true restart
        // The number of the restart has been incremented in the stop object
        if (!complete && !e->stopped()) {
          /*
           * The engine might not have checked its stop object, for example
           * when the slave failed at the root or a parallel engine had no
           * work. Without this check, the restarts might never stop.
           */
          if (stop->meta_stop(e->statistics(),m_opt)) {
            m_stopped = true;
            return nullptr;
          }
          stop->restart();
        }
        sslr = 0;
        NoGoods& ng = master_nogoods();
        MetaInfo mi(stop->restarts(),e->stopped() ? MetaInfo::RR_LIM : MetaInfo::RR_CMPL,sslr,e->statistics().fail,last,ng);
        (void) master->master(mi);
        stop->nogood(ng.ng());
        unsigned long long int nl = ++(*co);
        stop->limit(e->statistics(),nl);
        if (stop->status(master) == SS_FAILED) {
          // The statistics of the engine have been added by limit()
          delete master;
          master = nullptr;
          e->reset(nullptr);
          return nullptr;
        }
        Space* slave = master;
        master = master->clone();
        complete = slave->slave(mi);
        nogoods_valid = complete;
        e->reset(slave);
      } else {
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
    if (master == nullptr)
      return;
    if (last != nullptr) {
      last->constrain(b);
      if (last->status() == SS_FAILED) {
        delete last;
      } else {
        return;
      }
    }
    last = b.clone();
    master->constrain(b);
    e->constrain(b);
  }

  bool
  RBS::stopped(void) const {
    /*
     * What might happen during parallel search is that the
     * engine has been stopped but the meta engine has not, so
     * the meta engine does not perform a restart. However the
     * invocation of next will do so and no restart will be
     * missed.
     *
     * The engine may still report a stop when the master failed during a
     * restart after a limit. A failed master is a proof, not a stop.
     */
    return (master != nullptr) && (m_stopped || e->stopped());
  }

  RBS::~RBS(void) {
    delete e;
    delete master;
    delete last;
    delete co;
    delete stop;
  }

}}}

// STATISTICS: search-seq
