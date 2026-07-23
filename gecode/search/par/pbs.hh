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

namespace Gecode { namespace Search { namespace Par {

  /// Stop object used for controlling variants in a portfolio
  class GECODE_SEARCH_EXPORT PortfolioStop : public Stop {
  private:
    /// The stop object for the variants
    Stop* so;
    /// Whether search must be stopped
    std::atomic<bool>* tostop;
  public:
    /// Initialize
    PortfolioStop(Stop* so);
    /// Set pointer to shared \a tostop variable
    void share(std::atomic<bool>* ts);
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

  /// Runnable variant in a portfolio
  template<class Collect>
  class GECODE_SEARCH_EXPORT Variant : public Support::Runnable {
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
    Variant(PBS<Collect>* p, Engine* e, Stop* so);
    /// Return statistics of variant
    Statistics statistics(void) const;
    /// Check whether variant has been stopped
    bool stopped(void) const;
    /// Return the completion terminator
    virtual Support::Terminator* terminator(void) const;
    /// Wait for the variant to complete its current run
    void wait(void);
    /// Constrain with better solution \a b
    void constrain(const Space& b);
    /// Perform one run
    virtual void run(void);
    /// Delete variant
    virtual ~Variant(void);
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
    bool add(Space* s, Variant<CollectAll>* r);
    /// Dummy function
    bool constrain(const Space& b);
    /// Check whether there is any solution left
    bool empty(void) const;
    /// Return solution reported by \a r
    Space* get(Variant<CollectAll>*& r);
    /// Destructor
    ~CollectAll(void);
  };

  /// Collect best solutions
  class CollectBest {
  protected:
    /// Currently best solution
    Space* b;
    /// Who has reported the best solution (nullptr if solution has already been reported)
    Variant<CollectBest>* reporter;
  public:
    /// Whether it collects best solutions
    static const bool best = true;
    /// Initialize
    CollectBest(void);
    /// Add a solution \a s by \a r and return whether is was better
    bool add(Space* s, Variant<CollectBest>* r);
    /// Check whether \a b better and update accordingly
    bool constrain(const Space& b);
    /// Check whether there is any solution left
    bool empty(void) const;
    /// Return solution reported by \a r (only if a better one was found)
    Space* get(Variant<CollectBest>*& r);
    /// Destructor
    ~CollectBest(void);
  };

  /// Parallel portfolio engine implementation
  template<class Collect>
  class GECODE_SEARCH_EXPORT PBS : public Engine {
    friend class Variant<Collect>;
  protected:
    /// Portfolio statistics
    Statistics stat;
    /// Variant engines
    Variant<Collect>** variants;
    /// Number of variant engines
    unsigned int n_variants;
    /// Number of active variant engines
    unsigned int n_active;
    /// Whether a variant has been stopped
    std::atomic<bool> variant_stop;
    /// Shared stop flag
    std::atomic<bool> tostop;
    /// Collect solutions in this
    Collect solutions;
    /// Mutex for synchronization
    Support::Mutex m;
    /// Number of busy variants
    unsigned int n_busy;
    /// Signal that number of busy variants becomes zero
    Support::Event idle;
    /// Process report from variant, return false if solution was ignored
    bool report(Variant<Collect>* variant, Space* s);
    /**
     * The key invariant of the engine is as follows:
     *  - n_busy is always zero outside the next() function.
     *  - that entails, that locking is only needed inside next().
     *  - the variants 0..n_active-1 still might not have exhausted their
     *    search space.
     *  - the variants n_active..n_variants-1 have exhausted their search space.
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
