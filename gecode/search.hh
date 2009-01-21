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

#include <ctime>

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
      /// Create a clone after every \a c_d commits
      const unsigned int c_d = 8;
      /// Create a clone during recomputation if distance is greater than \a a_d
      const unsigned int a_d = 2;
      /// Default discrepancy limit for LDS
      const unsigned int d = 5;
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
      /// Stop search, if returns true (can be based on statistics \a s)
      virtual bool stop(const Statistics& s) = 0;
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
      virtual bool stop(const Statistics& s);
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
      /// Set current limit to \a l bodes
      void limit(unsigned long int l);
      /// Return true if node limit is exceeded
      virtual bool stop(const Statistics& s);
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
      virtual bool stop(const Statistics& s);
    };

    /**
     * \brief %Stop-object based on time
     * \ingroup TaskModelSearchStop
     */
    class GECODE_SEARCH_EXPORT TimeStop : public Stop {
    protected:
      /// Clock when execution should stop
      clock_t s;
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
      virtual bool stop(const Statistics& s);
    };

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
     */
    class Options {
    public:
      /// Create a clone after every \a c_d commits (for DFS and BAB)
      unsigned int c_d;
      /// Create a clone during recomputation if distance is greater than \a a_d (for DFS and BAB)
      unsigned int a_d;
      /// Discrepancy (for LDS)
      unsigned int d;
      /// Stop object for stopping search
      Stop* stop;
      /// Default options
      GECODE_SEARCH_EXPORT static const Options def;
      /// Initialize with default values
      Options(void);
    };

  }
}

#include <gecode/search/statistics.hpp>
#include <gecode/search/stop.hpp>
#include <gecode/search/options.hpp>

#include <gecode/search/ctrl.hpp>
#include <gecode/search/reco-stack.hpp>

#include <gecode/search/engine.hpp>

#include <gecode/search/dfs-engine.hpp>
#include <gecode/search/bab-engine.hpp>
#include <gecode/search/restart-engine.hpp>

namespace Gecode {

  /**
   * \brief Depth-first search engine
   *
   * This class supports depth-first search for subclasses \a T of
   * Space.
   * \ingroup TaskModelSearch
   */
  template <class T>
  class DFS {
  private:
    /// The actual search engine
    Search::DFS e;
  public:
    /// Initialize search engine for space \a s with options \a o
    DFS(T* s, const Search::Options& o=Search::Options::def);
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
  };

  /// Invoke depth-first search engine for subclass \a T of space \a s with options \a o
  template <class T>
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
  template <class T>
  class BAB {
  private:
    /// The actual search engine
    Search::BAB e;
  public:
    /// Initialize engine for space \a s and options \a o
    BAB(T* s, const Search::Options& o=Search::Options::def);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
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
  template <class T>
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
  template <class T>
  class Restart {
  private:
    /// The actual search engine
    Search::Restart e;
  public:
    /// Initialize engine for space \a s and options \a o
    Restart(T* s, const Search::Options& o=Search::Options::def);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
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
  template <class T>
  T* restart(T* s, const Search::Options& o=Search::Options::def);

}

namespace Gecode {

  namespace Search {

    /**
     * \brief Limited discrepancy search engine
     */
    class GECODE_SEARCH_EXPORT LDS {
    protected:
      /// Search options
      Options opt;
      ProbeEngine  e;           ///< The probe engine
      Space*       root;        ///< Root node for problem
      unsigned int d_cur;       ///< Current discrepancy
      bool         no_solution; ///< Solution found for current discrepancy
    public:
      /// Initialize for space \a s (of size \a sz) with options \a o
      LDS(Space* s, const Options& o, size_t sz);
      /// Return next solution (NULL, if none exists or search has been stopped)
      Space* next(void);
      /// Return statistics
      Statistics statistics(void) const;
      /// Check whether engine has been stopped
      bool stopped(void) const;
      /// Destructor
      ~LDS(void);
    };

  }

  /**
   * \brief Limited discrepancy search engine
   * \ingroup TaskModelSearch
   */
  template <class T>
  class LDS : public Search::LDS {
  public:
    /// Initialize engine with \a s as root node and options \a o
    LDS(T* s, const Search::Options& o=Search::Options::def);
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
  };

  /**
   * \brief Invoke limited-discrepancy search for \a s as root node and options\a o
   * \ingroup TaskModelSearch
   */
  template <class T>
  T* lds(T* s, const Search::Options& o=Search::Options::def);

}

#include <gecode/search/dfs.hpp>
#include <gecode/search/bab.hpp>
#include <gecode/search/restart.hpp>
#include <gecode/search/lds.hpp>

#endif

// STATISTICS: search-any
