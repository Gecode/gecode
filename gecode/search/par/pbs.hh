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

#ifndef __GECODE_SEARCH_PAR_PBS_HH__
#define __GECODE_SEARCH_PAR_PBS_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Par {

  /// Stop object used for controling slaves in a portfolio
  class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
  private:
    /// The stop object for the slaves
    Stop* so;
    /// Whether search must be stopped
    volatile bool* tostop;
  public:
    /// Initialize
    PortfolioStop(Stop* so);
    /// Set pointer to shared \a tostop variable
    void share(volatile bool* ts);
    /// Return true if portfolio engine must be stopped
    virtual bool stop(const Statistics& s, const Options& o);
    /// Signal whether search must be stopped
    void stop(bool s);
    /// Whether search must be stopped
    bool stop(void) const;
  };

  // Forward declaration
  template<class Collect>
  class PBS;

  /// Runnable slave of a portfolio master
  template<class Collect>
  class GECODE_SEARCH_EXPORT Slave : public Support::Runnable {
  protected:
    /// The master engine
    PBS<Collect>* master;
    /// The slave engine
    Engine* slave;
    /// Stop object
    Stop* stop;
  public:
    /// Initialize with master \a m, slave \a s, and its stop object \a so
    Slave(PBS<Collect>* m, Engine* s, Stop* so);
    /// Return statistics of slave
    Statistics statistics(void) const;
    /// Check whether slave has been stopped
    bool stopped(void) const;
    /// Constrain with better solution \a b
    void constrain(const Space& b);
    /// Perform one run
    virtual void run(void);
    /// Delete slave
    virtual ~Slave(void);
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
    bool add(Space* s, Slave<CollectAll>* r);
    /// Dummy function
    bool constrain(const Space& b);
    /// Check whether there is any solution left
    bool empty(void) const;
    /// Return solution reported by \a r
    Space* get(Slave<CollectAll>*& r);
    /// Destructor
    ~CollectAll(void);
  };

  /// Collect best solutions
  class CollectBest {
  protected:
    /// Currently best solution
    Space* b;
    /// Who has reported the best solution (NULL if solution has already been reported)
    Slave<CollectBest>* reporter;
  public:
    /// Whether it collects best solutions
    static const bool best = true;
    /// Initialize
    CollectBest(void);
    /// Add a solution \a s by \a r and return whether is was better
    bool add(Space* s, Slave<CollectBest>* r);
    /// Check whether \a b better and update accordingly
    bool constrain(const Space& b);
    /// Check whether there is any solution left
    bool empty(void) const;
    /// Return solution reported by \a r (only if a better one was found)
    Space* get(Slave<CollectBest>*& r);
    /// Destructor
    ~CollectBest(void);
  };

  /// Parallel portfolio engine implementation
  template<class Collect>
  class GECODE_SEARCH_EXPORT PBS : public Engine {
    friend class Slave<Collect>;
  protected:
    /// Master statistics
    Statistics stat;
    /// Slave engines
    Slave<Collect>** slaves;
    /// Number of slave engines
    unsigned int n_slaves;
    /// Number of active slave engines
    unsigned int n_active;
    /// Whether a slave has been stopped
    bool slave_stop;
    /// Shared stop flag
    volatile bool tostop;
    /// Collect solutions in this
    Collect solutions;
    /// Mutex for synchronization
    Support::Mutex m;
    /// Number of busy slaves
    unsigned int n_busy;
    /// Signal that number of busy slaves becomes zero
    Support::Event idle;
    /// Process report from slave, return false if solution was ignored
    bool report(Slave<Collect>* slave, Space* s);
    /**
     * The key invariant of the engine is as follows:
     *  - n_busy is always zero outside the next() function.
     *  - that entails, that locking is only needed insided next().
     *  - the slaves 0..n_active-1 still might not have exausted their
     *    search space.
     *  - the slaves n_active..n_slaves-1 have exhausted their search space.
     */
  public:
    /// Initialize
    PBS(Engine** s, Stop** so, unsigned int n, const Statistics& stat);
    /// Return next solution (NULL, if none exists or search has been stopped)
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
