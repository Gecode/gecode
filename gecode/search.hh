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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_SEARCH_HH__
#define __GECODE_SEARCH_HH__

#include <ctime>

#include "gecode/kernel.hh"

/*
 * Support for DLLs under Windows
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

#define GECODE_SEARCH_EXPORT

#endif

#include "gecode/support/dynamic-stack.hh"

namespace Gecode { 

  /// %Search engines
  namespace Search {

    /**
     * \brief %Search configuration
     *
     * \ingroup TaskIntSearch
     */
    namespace Config {
      /// Create a clone after every \a c_d commits
      const unsigned int c_d = 8;
      /// Create a clone during recomputation if distance is greater than \a a_d
      const unsigned int a_d = 2;
    }

    /**
     * \brief %Search engine statistics
     * \ingroup TaskIntSearch
     */
    class Statistics {
    public:
      /// Number of propagation steps
      unsigned long int propagate;
      /// Number of failed nodes in search tree
      unsigned long int fail;
      /// Number of clones created
      unsigned long int clone;
      /// Number of commit operations performed
      unsigned long int commit;
      /// Peak memory allocated
      size_t memory;
      /// Initialize with all numbers zero
      Statistics(void);
    };
    

    /**
     * \defgroup TaskIntSearchStop Stop-objects for stopping search
     * \ingroup TaskIntSearch
     *
     * Allows to specify various criteria when a search engine should
     * stop exploration. Only exploration but neither recomputation 
     * nor propagation will be interrupted.
     *
     */

    /**
     * \brief Base-class for %Stop-object
     * \ingroup TaskIntSearchStop
     */
    class Stop {
    public:
      /// Default constructor
      Stop(void);
      /// Stop search, if returns true (can be based on statistics \a s)
      virtual bool stop(const Statistics& s) = 0;
      /// Destructor
      GECODE_SEARCH_EXPORT virtual ~Stop(void);
    };

    /**
     * \brief %Stop-object based on memory consumption
     *
     * \ingroup TaskIntSearchStop
     */
    class MemoryStop : public Stop {
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
      GECODE_SEARCH_EXPORT virtual bool stop(const Statistics& s);
    };

    /**
     * \brief %Stop-object based on number of failures
     *
     * The number of failures reported (by the statistics) is the
     * number since the engine started exploration. It is not the
     * number since the last stop!
     * \ingroup TaskIntSearchStop
     */
    class FailStop : public Stop {
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
      GECODE_SEARCH_EXPORT virtual bool stop(const Statistics& s);
    };

    /**
     * \brief %Stop-object based on time
     * \ingroup TaskIntSearchStop
     */
    class TimeStop : public Stop {
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
      GECODE_SEARCH_EXPORT virtual bool stop(const Statistics& s);
    };


    /**
     * \brief %Search engine control including memory information
     */
    class EngineCtrl : public Statistics {
    protected:
      /// %Stop-object to be used
      Stop* st;
      /// Whether engine has been stopped
      bool _stopped;
      /// Memory required for a single space
      size_t mem_space;
      /// Memory for the current space (including memory for caching)
      size_t mem_cur;
      /// Current total memory
      size_t mem_total;
    public:
      /// Initialize with stop-object \a st and space size \a sz
      EngineCtrl(Stop* st, size_t sz);
      /// Reset stop information
      void start(void);
      /// Check whether engine must be stopped (with additional stackspace \a sz)
      bool stop(size_t sz);
      /// Check whether engine has been stopped
      bool stopped(void) const;
      /// New space \a s gets pushed on stack
      void push(const Space* s);
      /// New space \a s and branching description \a d get pushed on stack
      void push(const Space* s, const BranchingDesc* d);
      /// Space \a s gets popped from stack
      void pop(const Space* s);
      /// Space \a s and branching description \a d get popped from stack
      void pop(const Space* s, const BranchingDesc* d);
      /// Space \a s becomes current space (\a s = NULL: current space deleted)
      void current(const Space* s);
      /// Reset statistics for space \a s
      void reset(const Space* s);
    };
    
    /**
     * \brief %Search tree node for recomputation
     *
     */
    class ReCoNode {
    protected:
      /// Space corresponding to this node (might be NULL)
      Space*         _space;
      /// Current alternative 
      unsigned int   _alt;
      /// Last alternative
      unsigned int   _last;
      /// Braching description
      BranchingDesc* _desc;
    public:
      /// Node for space \a s with clone \a c (possibly NULL) and alternatives \a alt
      ReCoNode(Space* s, Space* c, unsigned int alt);
      /// Return space for node
      Space* space(void) const; 
      /// Return number for alternatives
      unsigned int alt(void) const; 
      /// Return branching description
      BranchingDesc* desc(void) const; 
      /// Set space to \a s
      void space(Space* s);
      /// Set number of alternatives to \a a
      void alt(unsigned int a);
      /// Set branching description to \a d
      void desc(BranchingDesc* d);
      /// Test whether current alternative is rightmost
      bool rightmost(void) const;
      /// Movre to next alternative
      void next(void);
      /// Return the rightmost alternative and remove it
      unsigned int share(void);
      /// Free memory for node
      void dispose(void);
    };


    /**
     * \brief Stack of nodes supporting recomputation
     *
     * Maintains the invariant that it contains
     * the path of the node being currently explored. This
     * is required to support recomputation, of course.
     *
     * The stack supports adaptive recomputation controlled
     * by the value of a_d: only if the recomputation
     * distance is at least this large, an additional
     * clone is created.
     *
     */
    class ReCoStack : public Support::DynamicStack<ReCoNode> {
    private:
      /// Adaptive recomputation distance
      const unsigned int a_d;
    public:
      /// Initialize with adaptive recomputation distance \a a_d
      ReCoStack(unsigned int a_d);
      /// Push space \a c (a clone of \a a or NULL) with alternatives \a a
      BranchingDesc* push(Space* s, Space* c, unsigned int a);
      /// Generate path for next node and return whether a next node exists
      bool next(EngineCtrl& s);
      /// Recompute space according to path with copying distance \a d
      GECODE_SEARCH_EXPORT
      Space* recompute(unsigned int& d, 
		       EngineCtrl& s);
      /// Reset stack
      void reset(void);
    };

    /**
     * \brief Depth-first search engine implementation
     *
     */
    class DfsEngine : public EngineCtrl {
    private:
      /// Recomputation stack of nodes
      ReCoStack          ds;
      /// Current space being explored
      Space*             cur;
      /// Copying recomputation distance
      const unsigned int c_d;
      /// Distance until next clone
      unsigned int       d;
    public:
      /**
       * \brief Initialize engine
       * \param c_d minimal recomputation distance
       * \param a_d adaptive recomputation distance
       * \param st %Stop-object
       * \param sz size of one space
       */
      DfsEngine(unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
      /// Initialize engine to start at space \a s
      void init(Space* s);
      /// Reset engine to restart at space \a s
      void reset(Space* s);
      /// %Search for next solution
      Space* explore(void);
      /// Return stack size used by engine
      size_t stacksize(void) const;
      /// Destructor
      ~DfsEngine(void);
    };

    /**
     * \brief Depth-first search engine
     *
     * This class implements depth-first exploration for spaces. In order to
     * use depth-first search on subclasses of Space, additional
     * functionality providing the necessary typecasts is available
     * in Gecode::DFS.
     */
    class GECODE_SEARCH_EXPORT DFS {
    protected:
      /// Engine used for exploration
      DfsEngine e;
    public:
      /**
       * \brief Initialize search engine
       * \param s root node (subclass of Space)
       * \param c_d minimal recomputation distance
       * \param a_d adaptive recomputation distance
       * \param st %Stop-object
       * \param sz size of one space
       */
      DFS(Space* s, unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
      /// Return next solution (NULL, if none exists or search has been stopped)
      Space* next(void);
      /// Return statistics
      Statistics statistics(void) const;
      /// Check whether engine has been stopped
      bool stopped(void) const;
    };

  }

  /**
   * \brief Depth-first search engine
   *
   * This class supports depth-first search for subclasses \a T of
   * Space.
   * \ingroup TaskIntSearch
   */
  template <class T>
  class DFS : public Search::DFS {
  public:
    /**
     * \brief Initialize search engine
     * \param s root node (subclass of Space)
     * \param c_d minimal recomputation distance
     * \param a_d adaptive recomputation distance
     * \param st %Stop-object
     */ 
    DFS(T* s, 
	unsigned int c_d=Search::Config::c_d, 
	unsigned int a_d=Search::Config::a_d,
	Search::Stop* st=NULL);
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
  };

  /**
   * \brief Invoke depth-first search engine
   * \param s root node (subclass \a T of Space)
   * \param c_d minimal recomputation distance
   * \param a_d adaptive recomputation distance
   * \param st %Stop-object
   * \ingroup TaskIntSearch
   */
  template <class T>
  T* dfs(T* s,
	 unsigned int c_d=Search::Config::c_d,
	 unsigned int a_d=Search::Config::a_d,
	 Search::Stop* st=NULL);



  namespace Search {

    /**
     * \brief Probing engine for %LDS
     *
     */
    class ProbeEngine : public EngineCtrl {
    protected:
      /// %Node in the search tree for %LDS
      class ProbeNode {
      private:
	/// %Space of current node
	Space*       _space;
	/// Next alternative to try
	unsigned int _alt;
      public:
	/// Initialize with node \a s and next alternative \a a
	ProbeNode(Space* s, unsigned int a);
	/// Return space
	Space* space(void) const; 
	/// Set space to \a s
	void space(Space* s);
	/// Return next alternative
	unsigned int alt(void) const; 
	/// %Set next alternative
	void alt(unsigned int a);
      };
      /// %Stack storing current path in search tree
      Support::DynamicStack<ProbeNode> ds;
      /// Current space
      Space* cur;
      /// Current discrepancy
      unsigned int d;
    public:
      /// Initialize for spaces of size \a s
      ProbeEngine(Stop* st, size_t s);
      /// Initialize with space \a s and discrepancy \a d
      void init(Space* s, unsigned int d);
      /// Reset with space \a s and discrepancy \a d
      void reset(Space* s, unsigned int d);
      /// Return stack size used by engine
      size_t stacksize(void) const;
      /// Destructor
      ~ProbeEngine(void);
      /// %Search for next solution
      Space* explore(void);
    };

    /**
     * \brief Limited discrepancy search engine
     */
    class GECODE_SEARCH_EXPORT LDS {
    protected:
      Space*       root;        ///< Root node for problem
      unsigned int d_cur;       ///< Current discrepancy
      unsigned int d_max;       ///< Maximal discrepancy
      bool         no_solution; ///< Solution found for current discrepancy
      ProbeEngine  e;           ///< The probe engine
    public:
      /** Initialize engine
       * \param s root node
       * \param d maximal discrepancy
       * \param st %Stop-object
       * \param sz size of space
       */
      LDS(Space* s, unsigned int d, Stop* st, size_t sz);
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
   * \ingroup TaskIntSearch
   */
  template <class T>
  class LDS : public Search::LDS {
  public:
    /** \brief Initialize engine
     * \param s root node (subclass \a T of Space)
     * \param d maximal discrepancy
     * \param st %Stop-object
     */
    LDS(T* s, unsigned int d, Search::Stop* st=NULL);
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
  };

  /**
   * \brief Invoke limited-discrepancy search
   * \param s root node (subclass \a T of Space)
   * \param d maximum number of discrepancies
   * \param st %Stop-object
   * \ingroup TaskIntSearch
   */
  template <class T>
  T* lds(T* s,unsigned int d, Search::Stop* st=NULL);





  /*
   * Best solution search engines
   *
   */

  namespace Search {

    /**
     * \brief Implementation of depth-first branch-and-bound search engines
     */
    class BabEngine : public EngineCtrl {
    private:
      /// Recomputation stack of nodes
      ReCoStack          ds;
      /// Current space being explored
      Space*             cur;
      /// Number of entries not yet constrained to be better
      unsigned int       mark;
      /// Best solution found so far
      Space*             best;
      /// Copying recomputation distance
      const unsigned int c_d;
      /// Distance until next clone
      unsigned int       d;
    public:
      /**
       * \brief Initialize engine
       * \param c_d minimal recomputation distance
       * \param a_d adaptive recomputation distance
       * \param st %Stop-object
       * \param sz size of one space
       */
      BabEngine(unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
      /// Initialize engine to start at space \a s
      void init(Space* s);
      /**
       * \brief %Search for next better solution
       *
       * If \c true is returned, a next better solution has been found.
       * This solution is available from \a s1.
       *
       * If \c false is returned, the engine requires that the space \a s1
       * is constrained to be better by the so-far best solution \a s2.
       *
       */
      GECODE_SEARCH_EXPORT 
      bool explore(Space*& s1, Space*& s2);
      /// Return stack size used by engine
      size_t stacksize(void) const;
      /// Destructor
      ~BabEngine(void);
    };

    /**
     * \brief Depth-first branch-and-bound search engine
     *
     * This class implements depth-first branch-and-bound exploration 
     * for spaces. In order to use it on subclasses of Space, additional
     * functionality providing the necessary typecasts is available 
     * in Gecode::BAB:
     *
     */
    class GECODE_SEARCH_EXPORT BAB {
    protected:
      /// Engine used for exploration
      BabEngine e;
    public:
      /**
       * \brief Initialize engine
       * \param s root node 
       * \param c_d minimal recomputation distance
       * \param a_d adaptive recomputation distance
       * \param st %Stop-object
       * \param sz size of one space
       */
      BAB(Space* s, unsigned int c_d, unsigned int a_d, Stop* st, size_t sz);
      /// Check whether engine has been stopped
      bool stopped(void) const;
      /// Return statistics
      Statistics statistics(void) const;
    };

  }
  
  /**
   * \brief Depth-first branch-and-bound search engine
   * \ingroup TaskIntSearch
   */
  template <class T>
  class BAB : public Search::BAB {
  public:
    /**
     * \brief Initialize engine
     * \param s Root node (subclass \a T of Space). 
     *          Additionally, \a s must implement
     *          a member function \code void constrain(T* t) \endcode
     *          Whenever exploration requires to add a constraint
     *          to the space \a c currently being explored, the engine
     *          executes \c c->constrain(t) where \a t is the so-far 
     *          best solution.
     * \param c_d Minimal recomputation distance
     * \param a_d Adaptive recomputation distance
     * \param st %Stop-object
     */
    BAB(T* s,
	unsigned int c_d=Search::Config::c_d,
	unsigned int a_d=Search::Config::a_d,
	Search::Stop* st=NULL);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
  };

  /**
   * \brief Perform depth-first branch-and-bound search
   * \param s root node (subclass \a T of Space). 
   *          Additionally, \a s must implement
   *          a member function \code void constrain(T* t) \endcode
   *          Whenever exploration requires to add a constraint
   *          to the space \a c currently being explored, the engine
   *          executes \c c->constrain(t) where \a t is the so-far 
   *          best solution.
   * \param c_d minimal recomputation distance
   * \param a_d adaptive recomputation distance
   * \param st %Stop-object
   * \ingroup TaskIntSearch
   */
  template <class T>
  T* bab(T* s,
	 unsigned int c_d=Search::Config::c_d,
	 unsigned int a_d=Search::Config::a_d,
	 Search::Stop* st=NULL);



  /**
   * \brief Depth-first restart best solution search engine
   * \ingroup TaskIntSearch
   */
  template <class T>
  class Restart : public DFS<T> {
  protected:
    /// Root node
    Space* root;
    /// So-far best solution
    Space* best;
  public:
    /**
     * \brief Initialize engine
     * \param s root node (subclass \a T of Space). 
     *          Additionally, \a s must implement
     *          a member function \code void constrain(T* t) \endcode
     *          Whenever exploration requires to add a constraint
     *          to the space \a c currently being explored, the engine
     *          executes \c c->constrain(t) where \a t is the so-far 
     *          best solution.
     * \param c_d minimal recomputation distance
     * \param a_d adaptive recomputation distance
     * \param st %Stop-object
     */
    Restart(T* s, 
	    unsigned int c_d=Search::Config::c_d, 
	    unsigned int a_d=Search::Config::a_d,
	    Search::Stop* st=NULL);
    /// Destructor
    ~Restart(void);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
  };

  /**
   * \brief Perform depth-first restart best solution search
   * \param s root node (subclass \a T of Space). 
   *          Additionally, \a s must implement
   *          a member function \code void constrain(T* t) \endcode
   *          Whenever exploration requires to add a constraint
   *          to the space \a c currently being explored, the engine
   *          executes \c c->constrain(t) where \a t is the so-far 
   *          best solution.
   * \param c_d minimal recomputation distance
   * \param a_d adaptive recomputation distance
   * \param st %Stop-object
   */
  template <class T>
  T* restart(T* s,
	     unsigned int c_d=Search::Config::c_d,
	     unsigned int a_d=Search::Config::a_d,
	     Search::Stop* st=NULL);

}

#include "gecode/search/statistics.icc"
#include "gecode/search/stop.icc"
#include "gecode/search/engine-ctrl.icc"

#include "gecode/search/reco-stack.icc"

#include "gecode/search/dfs.icc"
#include "gecode/search/lds.icc"
#include "gecode/search/bab.icc"
#include "gecode/search/restart.icc"

#endif

// STATISTICS: search-any
