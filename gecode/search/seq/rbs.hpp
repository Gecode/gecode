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


namespace Gecode { namespace Search { namespace Seq {

  forceinline
  RestartStop::RestartStop(Stop* s)
    : l(0U), c(0U), m_stop(s), e_stopped(false), e_fail(0U) {}

  forceinline void
  RestartStop::restart(void) {
    Support::Lock lock(m);
    m_stat.restart++;
  }

  forceinline void
  RestartStop::nogood(unsigned long int n) {
    Support::Lock lock(m);
    m_stat.nogood += n;
  }

  forceinline SpaceStatus
  RestartStop::status(Space* s) {
    Support::Lock lock(m);
    return s->status(m_stat);
  }

  forceinline void
  RestartStop::limit(const Search::Statistics& s, unsigned long long int c0) {
    Support::Lock lock(m);
    l = c = c0;
    m_stat += s;
    e_stopped = false;
    e_fail = 0U;
  }

  forceinline void
  RestartStop::renew(const Search::Statistics& s) {
    Support::Lock lock(m);
    l = c;
    m_stat += s;
    e_stopped = false;
    e_fail = 0U;
  }

  forceinline void
  RestartStop::extend(unsigned long long int c0) {
    Support::Lock lock(m);
    c = c0;
    l = (e_fail > ULLONG_MAX - c0) ? ULLONG_MAX : e_fail + c0;
    e_stopped = false;
    e_fail = 0U;
  }

  forceinline void
  RestartStop::update(const Search::Statistics& s) {
    Support::Lock lock(m);
    m_stat += s;
  }

  forceinline bool
  RestartStop::enginestopped(void) const {
    Support::Lock lock(m);
    return e_stopped;
  }

  forceinline Statistics
  RestartStop::metastatistics(void) const {
    Support::Lock lock(m);
    return m_stat;
  }


  forceinline
  RBS::RBS(Space* s, RestartStop* stop0,
           Engine* e0, const Search::Statistics& stat, const Options& opt,
           bool best0, SpokeCoverage coverage0)
    : e(e0), hub(s), last(nullptr), co(opt.cutoff), stop(stop0),
      m_opt(opt.expand()), episode(0UL), coverage(coverage0),
      solution(false),
      best(best0), end(EndReason::NotEnded) {
    stop->limit(stat,(*co)());
  }

}}}

// STATISTICS: search-seq
