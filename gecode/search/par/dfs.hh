/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifndef __GECODE_SEARCH_PAR_DFS_HH__
#define __GECODE_SEARCH_PAR_DFS_HH__

#include <gecode/search/par/engine.hh>

namespace Gecode { namespace Search { namespace Par {

  /// %Parallel depth-first search engine
  template<class Tracer>
  class DFS : public Engine<Tracer> {
  protected:
    using Engine<Tracer>::idle;
    using Engine<Tracer>::busy;
    using Engine<Tracer>::stop;
    using Engine<Tracer>::block;
    using Engine<Tracer>::e_search;
    using Engine<Tracer>::e_reset_ack_start;
    using Engine<Tracer>::e_reset_ack_stop;
    using Engine<Tracer>::n_busy;
    using Engine<Tracer>::m_search;
    using Engine<Tracer>::m_wait_reset;
    using Engine<Tracer>::opt;
    using Engine<Tracer>::release;
    using Engine<Tracer>::signal;
    using Engine<Tracer>::solutions;
    using Engine<Tracer>::terminate;
    using Engine<Tracer>::workers;
    using Engine<Tracer>::C_WAIT;
    using Engine<Tracer>::C_RESET;
    using Engine<Tracer>::C_TERMINATE;
    using Engine<Tracer>::C_WORK;
    /// %Parallel depth-first search worker
    class Worker : public Engine<Tracer>::Worker {
    public:
      using Engine<Tracer>::Worker::_engine;
      using Engine<Tracer>::Worker::m;
      using Engine<Tracer>::Worker::path;
      using Engine<Tracer>::Worker::cur;
      using Engine<Tracer>::Worker::d;
      using Engine<Tracer>::Worker::idle;
      using Engine<Tracer>::Worker::node;
      using Engine<Tracer>::Worker::fail;
      using Engine<Tracer>::Worker::start;
      using Engine<Tracer>::Worker::tracer;
      using Engine<Tracer>::Worker::stop;
      /// Initialize for space \a s with engine \a e
      Worker(Space* s, DFS& e);
      /// Provide access to engine
      DFS& engine(void) const;
      /// Start execution of worker
      virtual void run(void);
      /// Try to find some work
      void find(void);
      /// Reset worker to restart at space \a s
      void reset(Space* s, unsigned int ngdl);
    };
    /// Array of worker references
    Worker** _worker;
  public:
    /// Provide access to worker \a i
    Worker* worker(unsigned int i) const;

    /// \name Search control
    //@{
    /// Report solution \a s
    void solution(Space* s);
    //@}

    /// \name Engine interface
    //@{
    /// Initialize for space \a s with options \a o
    DFS(Space* s, const Options& o);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Reset engine to restart at space \a s
    virtual void reset(Space* s);
    /// Return no-goods
    virtual NoGoods& nogoods(void);
    /// Destructor
    virtual ~DFS(void);
    //@}
  };

}}}

#include <gecode/search/par/dfs.hpp>

#endif

// STATISTICS: search-par
