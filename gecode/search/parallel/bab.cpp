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

  /// Parallel BAB engine
  class BAB : public Engine {
  protected:
    /// Parallel BAB search worker
    class Worker : public Engine::Worker {
    protected:
      /// Number of entries not yet constrained to be better
      int mark;
      /// Best solution found so far
      Space* best;
    public:
      /// Initialize for space \a s (of size \a sz) with engine \a e
      Worker(Space* s, size_t sz, BAB& e);
      /// Provide access to engine
      BAB& engine(void) const;
      /// Start execution of worker
      virtual void run(void);
      /// Accept better solution \a b
      void better(Space* b);
      /// Try to find some work
      void find(void);
      /// Destructor
      virtual ~Worker(void);
    };
    /// Array of worker references
    Worker** _worker;
    /// Best solution so far
    Space* best;
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
    BAB(Space* s, size_t sz, const Options& o);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Destructor
    virtual ~BAB(void);
    //@}
  };



  /*
   * Engine: basic access routines
   */
  forceinline BAB&
  BAB::Worker::engine(void) const {
    return static_cast<BAB&>(_engine);
  }
  forceinline BAB::Worker*
  BAB::worker(unsigned int i) const {
    return _worker[i];
  }


  /*
   * Engine: initialization
   */
  forceinline
  BAB::Worker::Worker(Space* s, size_t sz, BAB& e)
    : Engine::Worker(s,sz,e), mark(0), best(NULL) {}

  forceinline
  BAB::BAB(Space* s, size_t sz, const Options& o)
    : Engine(s,sz,o), best(NULL) {
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
  BAB::statistics(void) const {
    Statistics s;
    for (unsigned int i=0; i<workers(); i++)
      s += worker(i)->statistics();
    return s;
  }


  /*
   * Engine: search control
   */
  void
  BAB::Worker::better(Space* b) {
    m.acquire();
    delete best;
    best = b->clone(false);
    mark = path.entries();
    if (cur != NULL)
      cur->constrain(*best);
    m.release();
  }
  forceinline void 
  BAB::solution(Space* s) {
    m_search.acquire();
    if (best != NULL) {
      s->constrain(*best);
      if (s->status() == SS_FAILED) {
        delete s;
        m_search.release();
        return;
      } else {
        delete best;
        best = s->clone();
      }
    } else {
      best = s->clone();
    }
    // Announce better solutions
    for (unsigned int i=0; i<workers(); i++)
      worker(i)->better(best);
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
  BAB::Worker::find(void) {
    // Try to find new work (even if there is none)
    for (unsigned int i=0; i<engine().workers(); i++) {
      unsigned long int r_d;
      if (Space* s = engine().worker(i)->steal(r_d)) {
        // Reset this guy
        m.acquire();
        idle = false;
        d = 0;
        cur = s;
        mark = 0;
        if (best != NULL)
          cur->constrain(*best);
        Search::Worker::reset(cur,r_d);
        m.release();
        return;
      }
    }
  }

  void
  BAB::Worker::run(void) {
    // Okay, we are in business, start working
    while (true) {
      switch (engine().cmd()) {
      case BAB::C_WAIT:
        // Wait
        engine().wait();
        break;
      case BAB::C_TERMINATE:
        // Acknowledge termination request
        engine().acknowledge();
        // Wait until termination can proceed
        engine().wait_terminate();
        // Terminate thread
        engine().terminated();
        return;
      case BAB::C_WORK:
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
              /*
               * The invariant maintained by the engine is:
               *   For all nodes stored at a depth less than mark, there
               *   is no guarantee of betterness. For those above the mark,
               *   betterness is guaranteed.
               */
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
            cur = path.recompute(d,engine().opt().a_d,*this,best,mark);
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
  BAB::Worker::~Worker(void) {
    delete best;
  }

  BAB::~BAB(void) {
    terminate();
  }


  // Create parallel depth-first engine
  Search::Engine* bab(Space* s, size_t sz, const Options& o) {
    return new BAB(s,sz,o);
  }

}}}

// STATISTICS: search-any
