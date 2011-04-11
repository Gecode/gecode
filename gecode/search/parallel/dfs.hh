/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#ifndef __GECODE_SEARCH_PARALLEL_DFS_HH__
#define __GECODE_SEARCH_PARALLEL_DFS_HH__

#include <gecode/search/parallel/engine.hh>

namespace Gecode { namespace Search { namespace Parallel {

  /// %Parallel depth-first search engine
  class DFS : public Engine {
  protected:
    /// %Parallel depth-first search worker
    class Worker : public Engine::Worker {
    public:
      /// Initialize for space \a s (of size \a sz) with engine \a e
      Worker(Space* s, size_t sz, DFS& e);
      /// Provide access to engine
      DFS& engine(void) const;
      /// Start execution of worker
      virtual void run(void);
      /// Try to find some work
      void find(void);
      /// Reset engine to restart at space \a s and return new root space
      Space* reset(Space* s);
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
    /// Reset engine to restart at space \a s and return new root space
    Space* reset(Space* s);
    //@}

    /// \name Engine interface
    //@{
    /// Initialize for space \a s (of size \a sz) with options \a o
    DFS(Space* s, size_t sz, const Options& o);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Destructor
    virtual ~DFS(void);
    //@}
  };


  /*
   * Basic access routines
   */
  forceinline DFS&
  DFS::Worker::engine(void) const {
    return static_cast<DFS&>(_engine);
  }
  forceinline DFS::Worker*
  DFS::worker(unsigned int i) const {
    return _worker[i];
  }


  /*
   * Engine: initialization
   */
  forceinline
  DFS::Worker::Worker(Space* s, size_t sz, DFS& e)
    : Engine::Worker(s,sz,e) {}
  forceinline
  DFS::DFS(Space* s, size_t sz, const Options& o)
    : Engine(o) {
    // Create workers
    _worker = static_cast<Worker**>
      (heap.ralloc(workers() * sizeof(Worker*)));
    // The first worker gets the entire search tree
    _worker[0] = new Worker(s,sz,*this);
    // All other workers start with no work
    for (unsigned int i=1; i<workers(); i++)
      _worker[i] = new Worker(NULL,sz,*this);
    // Block all workers
    block();
    // Create and start threads
    for (unsigned int i=0; i<workers(); i++)
      Support::Thread::run(_worker[i]);
  }

  /*
   * Statistics
   */
  forceinline Space*
  DFS::Worker::reset(Space* s) {
    delete cur;
    path.reset();
    d = 0;
    idle = false;
    if ((s == NULL) || (s->status(*this) == SS_FAILED)) {
      delete s;
      cur = NULL;
      Search::Worker::reset();
      return NULL;
    } else {
      cur = s;
      Search::Worker::reset(cur);
      return s->clone(false);
    }
  }
  forceinline Space*
  DFS::reset(Space* s) {
    // All workers are marked as busy again
    n_busy = workers();
    for (unsigned int i=1; i<workers(); i++)
      (void) worker(i)->reset(NULL);
    return worker(0)->reset(s);
  }

  /*
   * Engine: search control
   */
  forceinline void 
  DFS::solution(Space* s) {
    m_search.acquire();
    bool bs = signal();
    solutions.push(s);
    if (bs)
      e_search.signal();
    m_search.release();
  }
  



  /*
   * Worker: finding and stealing working
   */
  forceinline void
  DFS::Worker::find(void) {
    // Try to find new work (even if there is none)
    for (unsigned int i=0; i<engine().workers(); i++) {
      unsigned long int r_d = 0ul;
      if (Space* s = engine().worker(i)->steal(r_d)) {
        // Reset this guy
        m.acquire();
        idle = false;
        d = 0;
        cur = s;
        Search::Worker::reset(cur,r_d);
        m.release();
        return;
      }
    }
  }

}}}

#endif

// STATISTICS: search-parallel
