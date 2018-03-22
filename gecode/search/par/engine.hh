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

#ifndef __GECODE_SEARCH_PAR_ENGINE_HH__
#define __GECODE_SEARCH_PAR_ENGINE_HH__

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>
#include <gecode/search/par/path.hh>

namespace Gecode { namespace Search { namespace Par {

  /// %Parallel depth-first search engine
  template<class Tracer>
  class Engine : public Search::Engine, public Support::Terminator {
  protected:
    /// %Parallel depth-first search worker
    class Worker : public Search::Worker, public Support::Runnable {
    public:
      /// Search tracer
      Tracer tracer;
    protected:
      /// Reference to engine
      Engine& _engine;
      /// Mutex for access to worker
      Support::Mutex m;
      /// Current path ins search tree
      Path<Tracer> path;
      /// Current space being explored
      Space* cur;
      /// Distance until next clone
      unsigned int d;
      /// Whether the worker is idle
      bool idle;
    public:
      /// Initialize for space \a s with engine \a e
      Worker(Space* s, Engine& e);
      /// Hand over some work (NULL if no work available)
      Space* steal(unsigned long int& d, Tracer& myt, Tracer& ot);
      /// Return statistics
      Statistics statistics(void);
      /// Provide access to engine
      Engine& engine(void) const;
      /// Return no-goods
      NoGoods& nogoods(void);
      /// Destructor
      virtual ~Worker(void);
      /// Terminator (engine)
      virtual Support::Terminator* terminator(void) const;
    };
    /// Search options
    Options _opt;
  public:
    /// Provide access to search options
    const Options& opt(void) const;
    /// Return number of workers
    unsigned int workers(void) const;

    /// \name Commands from engine to workers and wait management
    //@{
    /// Commands from engine to workers
    enum Cmd {
      C_WORK,     ///< Perform work
      C_WAIT,     ///< Run into wait lock
      C_RESET,    ///< Perform reset operation
      C_TERMINATE ///< Terminate
    };
  protected:
    /// The current command
    volatile Cmd _cmd;
    /// Mutex for forcing workers to wait
    Support::Mutex _m_wait;
  public:
    /// Return current command
    Cmd cmd(void) const;
    /// Block all workers
    void block(void);
    /// Release all workers
    void release(Cmd c);
    /// Ensure that worker waits
    void wait(void);
    //@}

    /// \name Termination control
    //@{
  protected:
    /// Mutex for access to termination information
    Support::Mutex _m_term;
    /// Number of workers that have not yet acknowledged termination
    volatile unsigned int _n_term_not_ack;
    /// Event for termination acknowledgment
    Support::Event _e_term_ack;
    /// Mutex for waiting for termination
    Support::Mutex _m_wait_terminate;
    /// Number of not yet terminated workers
    volatile unsigned int _n_not_terminated;
    /// Event for termination (all threads have terminated)
    Support::Event _e_terminate;
  public:
    /// For worker to acknowledge termination command
    void ack_terminate(void);
    /// For worker to register termination
    virtual void terminated(void);
    /// For worker to wait until termination is legal
    void wait_terminate(void);
    /// For engine to peform thread termination
    void terminate(void);
    //@}

    /// \name Reset control
    //@{
  protected:
    /// Mutex for access to reset information
    Support::Mutex _m_reset;
    /// Number of workers that have not yet acknowledged reset
    volatile unsigned int _n_reset_not_ack;
    /// Event for reset acknowledgment started
    Support::Event e_reset_ack_start;
    /// Event for reset acknowledgment stopped
    Support::Event e_reset_ack_stop;
    /// Mutex for waiting for reset
    Support::Mutex m_wait_reset;
  public:
    /// For worker to acknowledge start of reset cycle
    void ack_reset_start(void);
    /// For worker to acknowledge stop of reset cycle
    void ack_reset_stop(void);
    /// For worker to wait for all workers to reset
    void wait_reset(void);
    //@}

    /// \name Search control
    //@{
  protected:
    /// Mutex for search
    Support::Mutex m_search;
    /// Event for search (solution found, no more solutions, search stopped)
    Support::Event e_search;
    /// Queue of solutions
    Support::DynamicQueue<Space*,Heap> solutions;
    /// Number of busy workers
    volatile unsigned int n_busy;
    /// Whether a worker had been stopped
    volatile bool has_stopped;
    /// Whether search state changed such that signal is needed
    bool signal(void) const;
  public:
    /// Report that worker is idle
    void idle(void);
    /// Report that worker is busy
    void busy(void);
    /// Report that worker has been stopped
    void stop(void);
    //@}

    /// \name Engine interface
    //@{
    /// Initialize with options \a o
    Engine(const Options& o);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    //@}
  };

}}}

#include <gecode/search/par/engine.hpp>

#endif

// STATISTICS: search-par
