/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
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


#ifndef GECODE_SEARCH_PAR_FRONTIER_HH
#define GECODE_SEARCH_PAR_FRONTIER_HH

#include <gecode/search.hh>
#include <gecode/search/heuristic.hh>
#include <gecode/search/seq/choice-expansion.hh>
#include <exception>

#if defined(GECODE_HAS_EXPERIMENTAL) && defined(GECODE_HAS_THREADS)
namespace Gecode { namespace Search { namespace Par {

  /**
   * \brief Parallel engine for heuristic search
   *
   * Workers lease a node, expand it outside the lock, and submit its
   * children. Subclasses decide how open nodes are stored and selected.
   * All members are protected by the mutex.
   */
  class Frontier : public Search::Engine {
  protected:
    /// Node leased by a worker, or partially expanded node
    class Task {
    public:
      /// The node with its remaining alternatives
      Seq::ChoiceExpansion expansion;
      /// Depth of the node
      unsigned long int depth;
      /// Incumbent revision the node is constrained with
      unsigned long long int revision;
      /// Whether the node has been counted
      bool counted;
      /// Initialize
      Task(void) : depth(0), revision(0), counted(false) {}
    };
    /// Thread running a worker
    class Runner : public Support::Runnable, public Support::Terminator {
      /// The engine
      Frontier& engine;
      /// Worker identifier
      unsigned int id;
    public:
      /// Events for waking up and for termination
      Support::Event wake, done;
      /// Initialize
      Runner(Frontier& e, unsigned int i);
      /// Run worker
      void run(void) override;
      /// Return terminator
      Support::Terminator* terminator(void) const override;
      /// Signal termination
      void terminated(void) override;
    };
    /// Search options
    Options opt;
    /// Guidance
    HeuristicGuidance guidance;
    /// Best solution so far
    Incumbent best;
    /// Solution to be returned by next()
    std::unique_ptr<Space> solution;
    /// Number of created entries
    unsigned long long int order;
    /// Partially expanded nodes
    Support::DynamicQueue<Task*,Heap> pending;
    /// Number of partially expanded nodes
    unsigned int n_pending;
    /// The threads (allocated on first next())
    Runner** runners;
    /// Mutex protecting all data
    mutable Support::Mutex mutex;
    /// Signalled when search is idle
    Support::Event changed;
    /// Statistics
    Search::Statistics stat;
    /// Number of workers holding a task
    unsigned int active;
    /// Whether search is running, being shut down, stopped, or limited
    bool running, shutdown, _stopped, limited;
    /// First exception thrown by a worker
    std::exception_ptr error;

    /// \name Open nodes (called with the mutex held)
    //@{
    /// Whether there are open nodes
    virtual bool open(void) const = 0;
    /// Remove and return the next open node
    virtual Entry select(void) = 0;
    /// Add child \a e, or delete it; \a leased tells whether its parent is still leased
    virtual void submit(const Entry& e, bool leased) = 0;
    /// Called when all work is done, return whether there is new work
    virtual bool advance(void) = 0;
    /// Delete all open nodes
    virtual void clear(void) = 0;
    /// Called with the root \a r when search starts
    virtual void started(const Space& r);
    //@}
    /// Maximal number of children a worker collects before submitting
    static const unsigned int max_batch = 8;
    /// Number of children a worker collects before submitting
    unsigned int batch;
    /// Whether to compute dominance keys
    bool keyed;

    /// Initialize with options \a o
    Frontier(const Options& o);
    /// Start search from root \a s
    void initialize(Space* s, bool clone);
    /// Add pending task \a t
    void push(Task* t);
    /// Stop all workers (must be called by subclass destructors)
    void terminate(void);
    /// Make \a s with bound \a b the incumbent, if it improves
    void publish(Space* s, std::int64_t b);
    /// Start the threads
    void launch(void);
    /// Wake up all workers
    void wake(void);
    /// Whether worker \a id is currently enabled
    bool enabled(unsigned int id);
    /// Run worker \a id
    void work(unsigned int id);
  public:
    /// Destructor
    ~Frontier(void) override;
    /// Return next better solution (nullptr if none or stopped)
    Space* next(void) override;
    /// Return statistics
    Search::Statistics statistics(void) const override;
    /// Whether search has been stopped
    bool stopped(void) const override;
    /// Constrain future solutions to be better than \a s
    void constrain(const Space& s) override;
    /// Reset engine to restart at space \a s
    void reset(Space* s) override;
  };

}}}
#endif
#endif

// STATISTICS: search-par
