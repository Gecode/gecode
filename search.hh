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

#include "./kernel.hh"

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

#include "support/dynamic-stack.hh"

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
     * \brief %Full search engine statistics including memory information
     */
    class FullStatistics : public Statistics {
    public:
      /// Memory required for a single space
      size_t mem_space;
      /// Memory for the current space (including memory for caching)
      size_t mem_cur;
      /// Current total memory
      size_t mem_total;
      /// Initialize with space size \a sz
      FullStatistics(size_t sz);
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
      bool next(FullStatistics& s);
      /// Recompute space according to path with copying distance \a d
      Space* recompute(unsigned int& d, 
		       FullStatistics& s);
      /// Reset stack
      void reset(void);
    };

    /**
     * \brief Depth-first search engine implementation
     *
     */
    class DfsEngine : public FullStatistics {
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
       * \param sz size of one space
       */
      DfsEngine(unsigned int c_d, unsigned int a_d, size_t sz);
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
       * \param sz size of one space
       */
      DFS(Space* s, unsigned int c_d, unsigned int a_d, size_t sz);
      /// Return next solution (NULL, if none exists)
      Space* next(void);
      /// Return statistics
      Statistics statistics(void) const;
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
     */ 
    DFS(T* s, 
	unsigned int c_d=Search::Config::c_d, 
	unsigned int a_d=Search::Config::a_d);
    /// Return next solution (NULL, if none exists)
    T* next(void);
  };

  /**
   * \brief Invoke depth-first search engine
   * \param s root node (subclass \a T of Space)
   * \param c_d minimal recomputation distance
   * \param a_d adaptive recomputation distance
   * \ingroup TaskIntSearch
   */
  template <class T>
  T* dfs(T* s,
	 unsigned int c_d=Search::Config::c_d,
	 unsigned int a_d=Search::Config::a_d);



  namespace Search {

    /**
     * \brief Probing engine for %LDS
     *
     */
    class ProbeEngine : public FullStatistics {
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
      ProbeEngine(size_t s);
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
       * \param sz size of space
       */
      LDS(Space* s, unsigned int d, size_t sz);
      /// Return next solution (NULL, if none exists)
      Space* next(void);
      /// Return statistics
      Statistics statistics(void) const;
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
     */
    LDS(T* s, unsigned int d);
    /// Return next solution (NULL, if none exists)
    T* next(void);
  };

  /**
   * \brief Invoke limited-discrepancy search
   * \param s root node (subclass \a T of Space)
   * \param d maximum number of discrepancies
   * \ingroup TaskIntSearch
   */
  template <class T>
  T* lds(T* s,unsigned int d);





  /*
   * Best solution search engines
   *
   */

  namespace Search {

    /**
     * \brief Implementation of depth-first branch-and-bound search engines
     */
    class BabEngine : public FullStatistics {
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
       * \param sz size of one space
       */
      BabEngine(unsigned int c_d, unsigned int a_d, size_t sz);
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
       * \param sz size of one space
       */
      BAB(Space* s, unsigned int c_d, unsigned int a_d, size_t sz);
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
     */
    BAB(T* s,
	unsigned int c_d=Search::Config::c_d,
	unsigned int a_d=Search::Config::a_d);
    /// Return next better solution (NULL, if none exists)
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
   * \ingroup TaskIntSearch
   */
  template <class T>
  T* bab(T* s,
	 unsigned int c_d=Search::Config::c_d,
	 unsigned int a_d=Search::Config::a_d);



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
     */
    Restart(T* s, 
	    unsigned int c_d=Search::Config::c_d, 
	    unsigned int a_d=Search::Config::a_d);
    /// Destructor
    ~Restart(void);
    /// Return next better solution (NULL, if none exists)
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
   */
  template <class T>
  T* restart(T* s,
	     unsigned int c_d=Search::Config::c_d,
	     unsigned int a_d=Search::Config::a_d);

}

#include "search/statistics.icc"

#include "search/reco-stack.icc"

#include "search/dfs.icc"
#include "search/lds.icc"
#include "search/bab.icc"
#include "search/restart.icc"

#endif

// STATISTICS: search-any
