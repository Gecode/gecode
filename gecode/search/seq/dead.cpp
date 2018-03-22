/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <gecode/search/seq/dead.hh>

namespace Gecode { namespace Search { namespace Seq {

  /// A dead engine (failed root)
  template<class Tracer>
  class GECODE_SEARCH_EXPORT Dead : public Engine {
  protected:
    /// Search tracer
    Tracer tracer;
    /// Statistics
    Statistics stat;
  public:
    /// Initialize
    Dead(const Options& o, const Statistics& stat0);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Delete
    virtual ~Dead(void);
  };

  template<class Tracer>
  Dead<Tracer>::Dead(const Options& o, const Statistics& stat0)
    : tracer(o.tracer), stat(stat0) {
    tracer.engine(SearchTracer::EngineType::AOE, 1U);
    tracer.worker();
  }

  template<class Tracer>
  Space*
  Dead<Tracer>::next(void) {
    return NULL;
  }

  template<class Tracer>
  bool
  Dead<Tracer>::stopped(void) const {
    return false;
  }

  template<class Tracer>
  Statistics
  Dead<Tracer>::statistics(void) const {
    return stat;
  }

  template<class Tracer>
  Dead<Tracer>::~Dead(void) {
    tracer.done();
  }

  Engine*
  dead(const Options& o, const Statistics& stat) {
    if (o.tracer)
      return new Dead<TraceRecorder>(o,stat);
    else
      return new Dead<NoTraceRecorder>(o,stat);
  }


}}}

// STATISTICS: search-seq
