/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#ifndef GECODE_SEARCH_PAR_PBS_HH
#define GECODE_SEARCH_PAR_PBS_HH

#include <gecode/search.hh>
#include <atomic>
#include <exception>

namespace Gecode { namespace Search { namespace Par {

  /// Stop object used for controlling spokes in a portfolio
  class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
  private:
    /// The stop object for the spokes
    Stop* so;
    /// The stop object for the portfolio (possibly nullptr)
    Stop* po;
    /// Worker control for waking a paused asset
    WorkerControl worker_control;
    /// Whether search must be stopped
    std::atomic<bool>* tostop;
  public:
    /// Initialize with spoke stop \a so, portfolio stop \a po, and \a control
    PortfolioStop(Stop* so, Stop* po, const WorkerControl& control);
    /// Set pointer to shared \a tostop variable
    void share(std::atomic<bool>* ts);
    /// Return true if portfolio engine must be stopped
    virtual bool stop(const Statistics& s, const Options& o);
    /// Wake workers so that a paused asset can observe the portfolio stop
    void wake(void);
    /// Signal whether search must be stopped
    void stop(bool s);
    /// Whether search must be stopped
    bool stop(void) const;
  };

  // Forward declaration
  template<class Collect>
  class PBS;

  /// Runnable spoke in a portfolio
  template<class Collect>
  class GECODE_SEARCH_EXPORT Spoke : public Support::Runnable {
  protected:
    /// Completion event for the current run
    class Completion : public Support::Terminator {
    protected:
      /// The completion event
      Support::Event done;
    public:
      /// Initialize as completed
      Completion(void);
      /// Signal completion
      virtual void terminated(void);
      /// Wait for completion and consume the signal
      void wait(void);
    } completion;
    /// The portfolio engine
    PBS<Collect>* portfolio;
    /// The wrapped search engine
    Engine* engine;
    /// Stop object
    Stop* stop;
  public:
    /// Initialize with portfolio \a p, engine \a e, and its stop object \a so
    Spoke(PBS<Collect>* p, Engine* e, Stop* so);
    /// Return statistics of spoke
    Statistics statistics(void) const;
    /// Check whether spoke has been stopped
    bool stopped(void) const;
    /// Check whether spoke stopped at a local limit
    bool limited(void) const;
    /// Return the completion terminator
    virtual Support::Terminator* terminator(void) const;
    /// Wait for the spoke to complete its current run
    void wait(void);
    /// Wake a paused spoke so that it can observe the portfolio stop
    void wake(void);
    /// Constrain with better solution \a b
    void constrain(const Space& b);
    /// Perform one run
    virtual void run(void);
    /// Delete spoke
    virtual ~Spoke(void);
  };

  /// Collect all solutions
  class CollectAll {
  protected:
    /// Queue of solutions
    Support::DynamicQueue<Space*,Heap> solutions;
  public:
    /// Whether it collects best solutions
    static const bool best = false;
    /// Initialize
    CollectAll(void);
    /// Add a solution \a a reported by \a r and always return true
    bool add(Space* s, Spoke<CollectAll>* r);
    /// Dummy function
    bool constrain(const Space& b);
    /// Check whether there is any solution left
    bool empty(void) const;
    /// Return solution reported by \a r
    Space* get(Spoke<CollectAll>*& r);
    /// Destructor
    ~CollectAll(void);
  };

  /// Collect best solutions
  class CollectBest {
  protected:
    /// Currently best solution
    Space* b;
    /// Who has reported the best solution (nullptr if solution has already been reported)
    Spoke<CollectBest>* reporter;
  public:
    /// Whether it collects best solutions
    static const bool best = true;
    /// Initialize
    CollectBest(void);
    /// Add a solution \a s by \a r and return whether is was better
    bool add(Space* s, Spoke<CollectBest>* r);
    /// Check whether \a b better and update accordingly
    bool constrain(const Space& b);
    /// Check whether there is any solution left
    bool empty(void) const;
    /// Return solution reported by \a r (only if a better one was found)
    Space* get(Spoke<CollectBest>*& r);
    /// Destructor
    ~CollectBest(void);
  };

  /// Parallel portfolio engine implementation
  template<class Collect>
  class GECODE_SEARCH_EXPORT PBS : public Engine {
    friend class Spoke<Collect>;
  protected:
    /// Portfolio statistics
    Statistics stat;
    /// Spoke engines
    Spoke<Collect>** spokes;
    /// Number of spoke engines
    unsigned int n_spokes;
    /// Number of active spoke engines
    unsigned int n_active;
    /// Whether a spoke has been stopped
    std::atomic<bool> spoke_stop;
    /// Whether a spoke stopped at a local limit
    bool incomplete;
    /// Shared stop flag
    std::atomic<bool> tostop;
    /// Collect solutions in this
    Collect solutions;
    /// Mutex for synchronization
    Support::Mutex m;
    /// Number of busy spokes
    unsigned int n_busy;
    /// Signal that number of busy spokes becomes zero
    Support::Event idle;
    /// Failure raised while running or updating a portfolio asset
    std::exception_ptr failure;
    /// Process report from spoke, return false if solution was ignored
    bool report(Spoke<Collect>* spoke, Space* s);
    /// Process failure from spoke
    void fail(void);
    /**
     * The key invariant of the engine is as follows:
     *  - n_busy is always zero outside the next() function.
     *  - that entails, that locking is only needed inside next().
     *  - the spokes 0..n_active-1 still might not have exhausted their
     *    search space.
     *  - the spokes n_active..n_spokes-1 have exhausted their search space.
     */
  public:
    /// Initialize
    PBS(Engine** s, Stop** so, unsigned int n, const Statistics& stat);
    /// Return next solution (nullptr, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Constrain future solutions to be better than \a b
    virtual void constrain(const Space& b);
    /// Destructor
    virtual ~PBS(void);
  };

}}}

#include <gecode/search/par/pbs.hpp>

#endif

// STATISTICS: search-par
