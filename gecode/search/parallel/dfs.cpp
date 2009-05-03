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

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>
#include <gecode/search/parallel/path.hh>

#include <gecode/support/thread.hh>

namespace Gecode { namespace Search { namespace Parallel {

  class DfsWorker;

  /// Parallel DFS engine
  class DfsEngine : public Engine {
  public:
    /// Commands from engine to workers
    enum Cmd {
      C_WORK,     ///< Perform work
      C_WAIT,     ///< Run into wait lock
      C_TERMINATE ///< Terminate
    };
    /// Search options
    const Options opt;
    /// Array of worker references
    DfsWorker** worker;
    /// Array of threads
    Support::Thread* thread;
    /// Mutex for forcing workers to wait
    Support::Mutex m_wait;
    /// Mutex for search
    Support::Mutex m_search;
    /// Command for workers
    Cmd cmd;
    /// Event for search (solution found, no more solutions)
    Support::Event e_search;
    /// Queue of solutions
    Support::DynamicQueue<Space*,Heap> solutions;
    /// Number of busy workers
    unsigned int n_busy;
    /// Number of not yet terminated workers
    unsigned int n_not_terminated;
    /// Event for termination (all threads have terminated)
    Support::Event e_terminate;
    /// Whether a worker had been stopped
    bool stop;
    /// Initialize for space \a s (of size \a sz) with options \a o
    DfsEngine(Space* s, size_t sz, const Options& o);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Destructor
    virtual ~DfsEngine(void);
  };


  /// Parallel depth-first search worker
  class GECODE_SEARCH_EXPORT DfsWorker 
    : public Worker, public Support::Runnable {
  private:
    /// Reference to engine
    DfsEngine& engine;
    /// Mutex for access to worker
    Support::Mutex m;
    /// Current path ins search tree
    Path path;
    /// Current space being explored
    Space* cur;
    /// Distance until next clone
    unsigned int d;
  protected:
    /// Reset engine to restart at space \a s
    void reset(Space* s);
  public:
    /// Initialize for space \a s (of size \a sz) with engine \a e
    DfsWorker(Space* s, size_t sz, DfsEngine& e);
    /// Start execution of worker
    virtual void run(void);
    /// %Search for next solution
    void next(void);
    /// Return statistics
    Statistics statistics(void) const;
    /// Destructor
    ~DfsWorker(void);
  };

  /*
   * Engine
   */
  DfsEngine::DfsEngine(Space* s, size_t sz, const Options& o)
    : opt(o), solutions(heap) {
    unsigned int n = opt.threads;
    assert(n > 1);
    // Create workers
    worker = static_cast<DfsWorker**>
      (heap.ralloc(n * sizeof(DfsWorker*)));
    worker[0] = new DfsWorker(s,sz,*this);
    for (unsigned int i=1; i<n; i++)
      worker[i] = new DfsWorker(NULL,sz,*this);
    // Acquire wait mutex such that no worker can actually start running
    m_wait.acquire();
    // Start threads
    thread = static_cast<Support::Thread*>
      (heap.ralloc(n * sizeof(Support::Thread)));
    for (unsigned int i=1; i<n; i++)
      (void) new (&thread[i]) Support::Thread(worker[i]);
    // Initialize
    n_busy = n_not_terminated = n;
    cmd = C_WORK;
    stop = false;
  }

  Space* 
  DfsEngine::next(void) {
    // Invariant: the worker holds the wait mutex
    m_search.acquire();
    if (!solutions.empty()) {
      // No search to be done, take leftover solution
      Space* s = solutions.pop();
      m_search.release();
      return s;
    }
    // No more solutions or stopped?
    if ((n_busy == NULL) || stop) {
      m_search.release();
      return NULL;
    }
    // Okay, no search has to continue, make the guys work
    cmd = C_WORK;
    m_wait.release();
    m_search.release();
    e_search.wait();
    m_search.acquire();
    // Make guys wait again
    cmd = C_WAIT;
    m_wait.acquire();
    if (!solutions.empty()) {
      // No search to be done, take leftover solution
      Space* s = solutions.pop();
      m_search.release();
      return s;
    }
    // No more solutions or stopped!
    assert((n_busy == NULL) || stop);
    m_search.release();
    return NULL;
  }
  Statistics 
  DfsEngine::statistics(void) const {
    Statistics s;
    return s;
  }
  bool 
  DfsEngine::stopped(void) const {
    return stop;
  }
  DfsEngine::~DfsEngine(void) {
    // Invariant: the engine has the wait mutex
    m_search.acquire();
    // Terminate threads
    cmd = C_TERMINATE;
    m_wait.release();
    m_search.release();
    e_terminate.wait();
    // Now all threads are terminated!
    heap.rfree(worker);
    heap.rfree(thread);
  }



  /*
   * Worker
   */
  DfsWorker::DfsWorker(Space* s, size_t sz, DfsEngine& e)
    : Worker(sz), engine(e), d(0) {
    if (s != NULL) {
      cur = (s->status(*this) == SS_FAILED) ? NULL : snapshot(s,engine.opt);
      if (cur == NULL)
        fail++;
    } else {
      cur = NULL;
    }
    current(s);
    current(NULL);
    current(cur);
  }

  void
  DfsWorker::reset(Space* s) {
    delete cur;
    path.reset();
    d = 0;
    if (s->status(*this) == SS_FAILED) {
      cur = NULL;
      Worker::reset();
    } else {
      cur = s->clone();
      Worker::reset(cur);
    }
  }

  void
  DfsWorker::next(void) {
    start();
    while (true) {
      while (cur) {
        if (stop(engine.opt.stop,path.size()))
          return;
        node++;
        switch (cur->status(*this)) {
        case SS_FAILED:
          fail++;
          delete cur;
          cur = NULL;
          Worker::current(NULL);
          break;
        case SS_SOLVED:
          {
            // Deletes all pending branchings
            (void) cur->description();
            Space* s = cur;
            cur = NULL;
            Worker::current(NULL);
            return;
          }
        case SS_BRANCH:
          {
            Space* c;
            if ((d == 0) || (d >= engine.opt.c_d)) {
              c = cur->clone();
              d = 1;
            } else {
              c = NULL;
              d++;
            }
            const BranchingDesc* desc = path.push(*this,cur,c);
            Worker::push(c,desc);
            cur->commit(*desc,0);
            break;
          }
        default:
          GECODE_NEVER;
        }
      }
      do {
        if (!path.next(*this))
          return;
        cur = path.recompute(d,engine.opt.a_d,*this);
      } while (cur == NULL);
      Worker::current(cur);
    }
  }

  void
  DfsWorker::run(void) {
    // Immediately start waiting
    engine.m_wait.acquire();
    engine.m_wait.release();
    // Okay, we are in business, start working
    next();
  }

  Statistics
  DfsWorker::statistics(void) const {
    Statistics s = *this;
    s.memory += path.size();
    return s;
  }

  DfsWorker::~DfsWorker(void) {
    delete cur;
    path.reset();
  }


  // Create parallel depth-first engine
  Engine* dfs(Space* s, size_t sz, const Options& o) {
    return ::Gecode::Search::Sequential::dfs(s,sz,o);
  }

}}}

// STATISTICS: search-any
