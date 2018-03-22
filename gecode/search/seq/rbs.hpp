/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2012
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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
    : l(0), m_stop(s), e_stopped(false) {}

  forceinline void
  RestartStop::limit(const Search::Statistics& s, unsigned long int l0) {
    l = l0;
    m_stat += s;
    e_stopped = false;
  }

  forceinline void
  RestartStop::update(const Search::Statistics& s) {
    m_stat += s;
  }

  forceinline bool
  RestartStop::enginestopped(void) const {
    return e_stopped;
  }

  forceinline Statistics
  RestartStop::metastatistics(void) const {
    return m_stat;
  }


  forceinline
  RBS::RBS(Space* s, RestartStop* stop0,
           Engine* e0, const Search::Statistics& stat, const Options& opt,
           bool best0)
    : e(e0), master(s), last(NULL), co(opt.cutoff), stop(stop0),
      sslr(0),
      complete(true), restart(false), best(best0) {
    stop->limit(stat,(*co)());
  }

}}}

// STATISTICS: search-seq
