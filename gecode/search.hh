/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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

#ifndef __GECODE_SEARCH_HH__
#define __GECODE_SEARCH_HH__

#include <gecode/kernel.hh>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_SEARCH
#define GECODE_SEARCH_EXPORT __declspec( dllexport )
#else
#define GECODE_SEARCH_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_SEARCH_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_SEARCH_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_SEARCH
#define GECODE_LIBRARY_NAME "Search"
#include <gecode/support/auto-link.hpp>
#endif


namespace Gecode {

  /// %Search engines
  namespace Search {

    /**
     * \brief %Search configuration
     *
     * \ingroup TaskModelSearch
     */
    namespace Config {
      /// Whether engines create a clone when being initialized
      const bool clone = true;
      /// Number of threads to use
      const double threads = 1.0;
      /// Create a clone after every \a c_d commits (commit distance)
      const unsigned int c_d = 8;
      /// Create a clone during recomputation if distance is greater than \a a_d (adaptive distance)
      const unsigned int a_d = 2;

      /// Minimal number of open nodes for stealing
      const unsigned int steal_limit = 3;
      /// Initial delay in milliseconds for all but first worker thread
      const unsigned int initial_delay = 5;
    }

    /**
     * \brief %Search engine statistics
     * \ingroup TaskModelSearch
     */
    class Statistics : public StatusStatistics {
    public:
      /// Number of failed nodes in search tree
      unsigned long int fail;
      /// Number of nodes expanded
      unsigned long int node;
      /// Maximum depth of search stack
      unsigned long int depth;
      /// Peak memory allocated
      size_t memory;
      /// Initialize
      Statistics(void);
      /// Reset
      void reset(void);
      /// Return sum with \a s
      Statistics operator +(const Statistics& s);
      /// Increment by statistics \a s
      Statistics& operator +=(const Statistics& s);
    };

    class Stop;

    /**
     * \brief %Search engine options
     *
     * Defines options for search engines. Not all search engines might
     * honor all option values.
     *
     *  - \a c_d as minimal recomputation distance: this guarantees that
     *    a path between two nodes in the search tree for which copies are
     *    stored has at least length \a c_d. That is, in order to recompute
     *    a node in the search tree, \a c_d recomputation steps are needed.
     *    The minimal recomputation distance yields a guarantee on saving
     *    memory compared to full copying: it stores \a c_d times less nodes
     *    than full copying.
     *  - \a a_d as adaptive recomputation distance: when a node needs to be
     *    recomputed and the path is longer than \a a_d, an intermediate copy
     *    is created (approximately in the middle of the path) to speed up
     *    future recomputation. Note that small values of \a a_d can increase
     *    the memory consumption considerably.
     *
     * Full copying corresponds to a maximal recomputation distance
     * \a c_d of 1.
     *
     * All recomputation performed is based on batch recomputation: batch
     * recomputation performs propagation only once for an entire path
     * used in recomputation.
     *
     * The number of threads to be used is controlled by a double \f$n\f$
     * (assume that \f$m\f$ is the number of processing units available). If
     * \f$1 \leq n\f$, \f$n\f$ threads are chosen (of course with rounding).
     * If \f$n \leq -1\f$, then \f$m + n\f$ threads are 
     * chosen (all but \f$-n\f$ processing units get a thread). If \f$n\f$
     * is zero, \f$m\f$ threads are chosen. If \f$0<n<1\f$,
     * \f$n \times m\f$ threads are chosen. If \f$-1 <n<0\f$, 
     * \f$(1+n)\times m\f$ threads are chosen.
     * 
     * \ingroup TaskModelSearch
     */
    class Options {
    public:
      /// Whether engines create a clone when being initialized
      bool clone;
      /// Number of threads to use
      double threads;
      /// Create a clone after every \a c_d commits (commit distance)
      unsigned int c_d;
      /// Create a clone during recomputation if distance is greater than \a a_d (adaptive distance)
      unsigned int a_d;
      /// Stop object for stopping search
      Stop* stop;
      /// Default options
      GECODE_SEARCH_EXPORT static const Options def;
      /// Initialize with default values
      Options(void);
      /// Expand with real number of threads
      GECODE_SEARCH_EXPORT Options
      expand(void) const;
    };

    /**
     * \defgroup TaskModelSearchStop Stop-objects for stopping search
     * \ingroup TaskModelSearch
     *
     * Allows to specify various criteria when a search engine should
     * stop exploration. Only exploration but neither recomputation
     * nor propagation will be interrupted.
     *
     */

    /**
     * \brief Base-class for %Stop-object
     * \ingroup TaskModelSearchStop
     */
    class GECODE_SEARCH_EXPORT Stop {
    public:
      /// Default constructor
      Stop(void);
      /// Stop search, if returns true
      virtual bool stop(const Statistics& s, const Options& o) = 0;
      /// Destructor
      virtual ~Stop(void);
    };

    /**
     * \brief %Stop-object based on memory consumption
     *
     * \ingroup TaskModelSearchStop
     */
    class GECODE_SEARCH_EXPORT MemoryStop : public Stop {
    protected:
      /// Size limit
      size_t l;
    public:
      /// Stop if memory limit \a l (in bytes) is exceeded
      MemoryStop(size_t l);
      /// Return current limit
      size_t limit(void) const;
      /// Set current limit to \a l (in bytes)
      void limit(size_t l);
      /// Return true if memory limit is exceeded
      virtual bool stop(const Statistics& s, const Options& o);
    };

    /**
     * \brief %Stop-object based on number of nodes
     *
     * The number of nodes reported (by the statistics) is the
     * number since the engine started exploration. It is not the
     * number since the last stop!
     * \ingroup TaskModelSearchStop
     */
    class GECODE_SEARCH_EXPORT NodeStop : public Stop {
    protected:
      /// Node limit
      unsigned long int l;
    public:
      /// Stop if node limit \a l is exceeded
      NodeStop(unsigned long int l);
      /// Return current limit
      unsigned long int limit(void) const;
      /// Set current limit to \a l nodes
      void limit(unsigned long int l);
      /// Return true if node limit is exceeded
      virtual bool stop(const Statistics& s, const Options& o);
    };

    /**
     * \brief %Stop-object based on number of failures
     *
     * The number of failures reported (by the statistics) is the
     * number since the engine started exploration. It is not the
     * number since the last stop!
     * \ingroup TaskModelSearchStop
     */
    class GECODE_SEARCH_EXPORT FailStop : public Stop {
    protected:
      /// Failure limit
      unsigned long int l;
    public:
      /// Stop if failure limit \a l is exceeded
      FailStop(unsigned long int l);
      /// Return current limit
      unsigned long int limit(void) const;
      /// Set current limit to \a l failures
      void limit(unsigned long int l);
      /// Return true if failure limit is exceeded
      virtual bool stop(const Statistics& s, const Options& o);
    };

    /**
     * \brief %Stop-object based on time
     * \ingroup TaskModelSearchStop
     */
    class GECODE_SEARCH_EXPORT TimeStop : public Stop {
    protected:
      /// Time when execution should stop
      Support::Timer t;
      /// Current limit in milliseconds
      unsigned long int l;
    public:
      /// Stop if search exceeds \a l milliseconds (from creation of this object)
      TimeStop(unsigned long int l);
      /// Return current limit in milliseconds
      unsigned long int limit(void) const;
      /// Set current limit to \a l milliseconds
      void limit(unsigned long int l);
      /// Reset time to zero
      void reset(void);
      /// Return true if time limit is exceeded
      virtual bool stop(const Statistics& s, const Options& o);
    };


    /**
     * \brief %Search engine interface
     */
    class Engine {
    public:
      /// Return next solution (NULL, if none exists or search has been stopped)
      virtual Space* next(void) = 0;
      /// Return statistics
      virtual Search::Statistics statistics(void) const = 0;
      /// Check whether engine has been stopped
      virtual bool stopped(void) const = 0;
      /// Destructor
      virtual ~Engine(void) {}
    };

    /// %Sequential search engine implementations
    namespace Sequential {}

    /// %Parallel search engine implementations
    namespace Parallel {}

  }

}

#include <gecode/search/statistics.hpp>
#include <gecode/search/stop.hpp>
#include <gecode/search/options.hpp>

namespace Gecode {

  /**
   * \brief Depth-first search engine
   *
   * This class supports depth-first search for subclasses \a T of
   * Space.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class DFS {
  private:
    /// The actual search engine
    Search::Engine* e;
  public:
    /// Initialize search engine for space \a s with options \a o
    DFS(T* s, const Search::Options& o=Search::Options::def);
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// Destructor
    ~DFS(void);
  };

  /// Invoke depth-first search engine for subclass \a T of space \a s with options \a o
  template<class T>
  T* dfs(T* s, const Search::Options& o=Search::Options::def);



  /**
   * \brief Depth-first branch-and-bound search engine
   *
   * Additionally, \a s must implement a member function
   * \code virtual void constrain(const T& t) \endcode
   * Whenever exploration requires to add a constraint
   * to the space \a c currently being explored, the engine
   * executes \c c.constrain(t) where \a t is the so-far
   * best solution.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class BAB {
  private:
    /// The actual search engine
    Search::Engine* e;
  public:
    /// Initialize engine for space \a s and options \a o
    BAB(T* s, const Search::Options& o=Search::Options::def);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// Destructor
    ~BAB(void);
  };

  /**
   * \brief Perform depth-first branch-and-bound search for subclass \a T of space \a s and options \a o
   *
   * Additionally, \a s must implement a member function
   * \code virtual void constrain(const T& t) \endcode
   * Whenever exploration requires to add a constraint
   * to the space \a c currently being explored, the engine
   * executes \c c.constrain(t) where \a t is the so-far
   * best solution.
   *
   * \ingroup TaskModelSearch
   */
  template<class T>
  T* bab(T* s, const Search::Options& o=Search::Options::def);



  /**
   * \brief Depth-first restart best solution search engine
   *
   * Additionally, \a s must implement a member function
   * \code virtual void constrain(T& t) \endcode
   * Whenever exploration requires to add a constraint
   * to the space \a c currently being explored, the engine
   * executes \c c.constrain(t) where \a t is the so-far
   * best solution.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class Restart {
  private:
    /// The actual search engine
    Search::Engine* e;
  public:
    /// Initialize engine for space \a s and options \a o
    Restart(T* s, const Search::Options& o=Search::Options::def);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// Destructor
    ~Restart(void);
  };

  /**
   * \brief Perform depth-first restart best solution search for subclass \a T of space \a s and options \a o
   *
   * Additionally, \a s must implement a member function
   * \code virtual void constrain(T& t) \endcode
   * Whenever exploration requires to add a constraint
   * to the space \a c currently being explored, the engine
   * executes \c c.constrain(t) where \a t is the so-far
   * best solution.
   * \ingroup TaskModelSearch
   */
  template<class T>
  T* restart(T* s, const Search::Options& o=Search::Options::def);

}

#include <gecode/search/dfs.hpp>
#include <gecode/search/bab.hpp>
#include <gecode/search/restart.hpp>

#endif

// STATISTICS: search-other
