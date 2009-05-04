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
    volatile Cmd cmd;
    /// Event for search (solution found, no more solutions)
    Support::Event e_search;
    /// Queue of solutions
    Support::DynamicQueue<Space*,Heap> solutions;
    /// Number of busy workers
    volatile unsigned int n_busy;
    /// Number of not yet terminated workers
    volatile unsigned int n_not_terminated;
    /// Event for termination (all threads have terminated)
    Support::Event e_terminate;
    /// Whether a worker had been stopped
    volatile bool stop;
    /// Whether search state changed such that signal is needed
    bool signal(void) const;

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
    /// Whether worker is currently idle
    bool idle;
  protected:
    /// Reset engine to restart at space \a s
    void reset(Space* s);
  public:
    /// Initialize for space \a s (of size \a sz) with engine \a e
    DfsWorker(Space* s, size_t sz, DfsEngine& e);
    unsigned int number(void) const;
    /// Start execution of worker
    virtual void run(void);
    /// Hand over some work
    Space* steal(void);
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
    // Create and start threads
    thread = static_cast<Support::Thread*>
      (heap.ralloc(n * sizeof(Support::Thread)));
    for (unsigned int i=0; i<n; i++)
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
    if ((n_busy == 0) || stop) {
      m_search.release();
      return NULL;
    }
    // Okay, no search has to continue, make the guys work
    cmd = C_WORK;
    m_wait.release();
    m_search.release();
    /*
     * Wait until a search related event has happened. It might be that
     * the event has already been signalled in the last run, but the
     * solution has been removed. So we have to try until there has
     * something new happened.
     */
    while (true) {
      std::cout << "WAITING ON SIGNAL" << std::endl;
      e_search.wait();
      std::cout << "AFTER SIGNAL" << std::endl;
      m_search.acquire();
      if (!solutions.empty()) {
        std::cout << "FOUND SOLUTION" << std::endl;
        // Report solution
        Space* s = solutions.pop();
        m_search.release();
        // Make guys wait again
        cmd = C_WAIT;
        m_wait.acquire();
        return s;
      }
      // No more solutions or stopped?
      if ((n_busy == NULL) || stop) {
        std::cout << "FOUND OTHER SEARCH EVENT" << std::endl;
        m_search.release();
        // Make guys wait again
        cmd = C_WAIT;
        m_wait.acquire();
        return NULL;
      }
      m_search.release();
    }
    GECODE_NEVER;
    return NULL;
  }

  bool
  DfsEngine::signal(void) const {
    return solutions.empty() && (n_busy > 0) && !stop;
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
    cmd = C_TERMINATE;
    m_wait.release();
    // Terminate threads
    e_terminate.wait();
    // Now all threads are terminated!
    heap.rfree(worker);
    heap.rfree(thread);
  }



  /*
   * Worker
   */
  DfsWorker::DfsWorker(Space* s, size_t sz, DfsEngine& e)
    : Worker(sz), engine(e), d(0), idle(false) {
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

  unsigned int
  DfsWorker::number(void) const {
    unsigned int i = 0;
    while (engine.worker[i] != this)
      i++;
    return i;
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

  Space*
  DfsWorker::steal(void) {
    m.acquire();
    Space* s = path.steal();
    m.release();
    // Tell that there will be one more busy worker
    if (s != NULL) {
      engine.m_search.acquire();
      assert(engine.n_busy > 0);
      engine.n_busy++;
      engine.m_search.release();
    }
    return s;
  }
  void
  DfsWorker::run(void) {
    // Immediately start waiting
    engine.m_wait.acquire();
    engine.m_wait.release();
    // Okay, we are in business, start working
    while (true) {
      std::cout << "RUN (" << number() << ")" << std::endl;
      switch (engine.cmd) {
      case DfsEngine::C_WAIT:
        // Grab wait lock
        std::cout << "WAIT (" << number() << ")" << std::endl;
        engine.m_wait.acquire();
        engine.m_wait.release();
        std::cout << "CONTINUE (" << number() << ")" << std::endl;
        break;
      case DfsEngine::C_TERMINATE:
        // Grab search lock
        std::cout << "TERMINATE (" << number() << ")" << std::endl;
        engine.m_search.acquire();
        engine.n_not_terminated--;
        // Check whether this thread is the last to terminate and signal
        if (engine.n_not_terminated == 0)
          engine.e_terminate.signal();
        engine.m_search.release();
        std::cout << "LEAVING (" << number() << ")" << std::endl;
        return;
      case DfsEngine::C_WORK:
        {
          m.acquire();
          if (idle) {
            std::cout << "IDLE (" << number() << ")" << std::endl;
            m.release();
            // Try to find new work (even if there is none)
            // This must happen outside the worker lock: otherwise deadlock
            for (unsigned int i=0; i<engine.opt.threads; i++)
              if (engine.worker[i] != this) {
                std::cout << "STEAL (" << i << " -> " 
                          << number() << ")" << std::endl;
                if (Space* s = engine.worker[i]->steal()) {
                  std::cout << "STOLEN (" << number() << ")" << std::endl;
                  // Reset this guy
                  m.acquire();
                  idle = false;
                  cur = s;
                  m.release();
                  break;
                }
              }
            std::cout << "BEFORE SLEEP (" << number() << ")" << std::endl;
            Support::Thread::sleep(10);
            std::cout << "AFTER IDLE (" << number() << ")" << std::endl;
          } else if (cur != NULL) {
            start();
            if (stop(engine.opt.stop,path.size())) {
              // Report stop
              m.release();
              engine.m_search.acquire();
              bool signal = engine.signal();
              engine.stop = true;
              if (signal)
                engine.e_search.signal();
              engine.m_search.release();
            } else {
              std::cout << "EXPLORE (" << number() << ")" << std::endl;
              node++;
              switch (cur->status(*this)) {
              case SS_FAILED:
                fail++;
                delete cur;
                cur = NULL;
                Worker::current(NULL);
                m.release();
                break;
              case SS_SOLVED:
                {
                  // Deletes all pending branchings
                  (void) cur->description();
                  Space* s = cur->clone(false);
                  delete cur;
                  cur = NULL;
                  Worker::current(NULL);
                  m.release();
                  // Report solution
                  engine.m_search.acquire();
                  bool signal = engine.signal();
                  engine.solutions.push(s);
                  if (signal)
                    engine.e_search.signal();
                  engine.m_search.release();
                }
                break;
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
                  m.release();
                }
                break;
              default:
                GECODE_NEVER;
              }
            }
          } else {
            std::cout << "RECOMPUTE (" << number() << ")" << std::endl;
            if (!path.next(*this)) {
              idle = true;
              m.release();
              // Report that worker is idle
              engine.m_search.acquire();
              bool signal = engine.signal();
              engine.n_busy--;
              if (signal && (engine.n_busy == 0)) {
                std::cout << "DETECTED DONE (" << number() << ")" << std::endl;
                engine.e_search.signal();
              }
              engine.m_search.release();
              m.acquire();
            } else {
              cur = path.recompute(d,engine.opt.a_d,*this);
            }
            Worker::current(cur);
            m.release();
          }
        }
      break;
      }
    }
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
    //    return new DfsEngine(s,sz,o);
    return ::Gecode::Search::Sequential::dfs(s,sz,o);
  }

}}}

// STATISTICS: search-any
