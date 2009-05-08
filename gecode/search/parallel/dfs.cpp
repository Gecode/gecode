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

#include <gecode/search/parallel/engine.hh>

namespace Gecode { namespace Search { namespace Parallel {

  /// Parallel DFS engine
  class DFS : public Engine {
  protected:
    /// Parallel depth-first search worker
    class Worker : public Engine::Worker {
    protected:
      /// Reset engine to restart at space \a s
      void reset(Space* s);
    public:
      /// Initialize for space \a s (of size \a sz) with engine \a e
      Worker(Space* s, size_t sz, DFS& e);
      /// Provide access to engine
      DFS& engine(void) const;
      /// Start execution of worker
      virtual void run(void);
      /// Try to find some work
      void find(void);
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
    : Engine(s,sz,o) {
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
    for (unsigned int i=0; i<workers(); i++) {
      Support::Thread t(_worker[i]);
    }
  }






  /*
   * Statistics
   */
  Statistics 
  DFS::statistics(void) const {
    Statistics s;
    for (unsigned int i=0; i<workers(); i++)
      s += worker(i)->statistics();
    return s;
  }


  void
  DFS::Worker::reset(Space* s) {
    delete cur;
    path.reset();
    d = 0;
    if (s->status(*this) == SS_FAILED) {
      cur = NULL;
      Search::Worker::reset();
    } else {
      cur = s->clone();
      Search::Worker::reset(cur);
    }
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
      unsigned long int r_d;
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

  void
  DFS::Worker::run(void) {
    // Okay, we are in business, start working
    while (true) {
      switch (engine().cmd()) {
      case DFS::C_WAIT:
        // Wait
        engine().wait();
        break;
      case DFS::C_TERMINATE:
        // Acknowledge termination request
        engine().acknowledge();
        // Wait until termination can proceed
        engine().wait_terminate();
        // Terminate thread
        engine().terminated();
        return;
      case DFS::C_WORK:
        // Perform exploration work
        {
          m.acquire();
          if (idle) {
            m.release();
            // Try to find new work
            find();
          } else if (cur != NULL) {
            start();
            if (stop(engine().opt().stop,path.size())) {
              // Report stop
              m.release();
              engine().stop();
            } else {
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
                  engine().solution(s);
                }
                break;
              case SS_BRANCH:
                {
                  Space* c;
                  if ((d == 0) || (d >= engine().opt().c_d)) {
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
          } else if (path.next(*this)) {
            cur = path.recompute(d,engine().opt().a_d,*this);
            Worker::current(cur);
            m.release();
          } else {
            idle = true;
            m.release();
            // Report that worker is idle
            engine().idle();
          }
        }
        break;
      default:
        GECODE_NEVER;
      }
    }
  }


  /*
   * Termination and deletion
   */
  DFS::~DFS(void) {
    terminate();
    heap.rfree(_worker);
  }


  // Create parallel depth-first engine
  Search::Engine* dfs(Space* s, size_t sz, const Options& o) {
    return new DFS(s,sz,o);
  }

}}}

// STATISTICS: search-any
