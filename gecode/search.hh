/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Kevin Leo <kevin.leo@monash.edu>
 *     Maxim Shishmarev <maxim.shishmarev@monash.edu>
 * 
 *  Copyright:
 *     Kevin Leo, 2017
 *     Christian Schulte, 2002
 *     Maxim Shishmarev, 2017
 *     Guido Tack, 2004
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

#include <initializer_list>

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


namespace Gecode { namespace Search {

  /// %Sequential search engine implementations
  namespace Sequential {}

  /// %Parallel search engine implementations
  namespace Parallel {}

  /// %Meta search engine implementations
  namespace Meta {}

  namespace Meta {

    /// %Sequential meta search engine implementations
    namespace Sequential {}

    /// %Parallel meta search engine implementations
    namespace Parallel {}

  }


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

    /// Default discrepancy limit for LDS
    const unsigned int d_l = 5;

    /// Base for geometric restart sequence
    const double base = 1.5;
    /// Size of a slice in a portfolio and scale factor for restarts(in number of failures)
    const unsigned int slice = 250;

    /// Depth limit for no-good generation during search
    const unsigned int nogoods_limit = 128;

    /// Default port for CPProfiler
    const unsigned int cpprofiler_port = 6565U;
  }

}}

#include <gecode/search/exception.hpp>

namespace Gecode { namespace Search {

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
    /// Number of restarts
    unsigned long int restart;
    /// Number of no-goods posted
    unsigned long int nogood;
    /// Initialize
    Statistics(void);
    /// Reset
    void reset(void);
    /// Return sum with \a s
    Statistics operator +(const Statistics& s);
    /// Increment by statistics \a s
    Statistics& operator +=(const Statistics& s);
  };

}}

#include <gecode/search/statistics.hpp>

namespace Gecode { namespace Search {

  class WrapTraceRecorder;
  class TraceRecorder;
  class EdgeTraceRecorder;

}}

#include <string>
#include <sstream>

namespace Gecode {

  /// Support for tracing search
  class SearchTracer {
    friend class Search::WrapTraceRecorder;
    friend class Search::TraceRecorder;
    friend class Search::EdgeTraceRecorder;
  public:
    /// Which type of engine
    enum EngineType {
      DFS = 0, ///< Engine is a DFS engine
      BAB = 1, ///< Engine is a BAB engine
      LDS = 2, ///< Engine is a LDS engine
      RBS = 3, ///< Engine is a RBS engine
      PBS = 4, ///< Engine is a PBS engine
      AOE = 5  ///< Unspecified engine (any other engine)
    };
    /// Information about an engine
    class EngineInfo {
    protected:
      /// The engine type
      EngineType _type;
      /// First worker or engine
      unsigned int _fst;
      /// Last worker or engine
      unsigned int _lst;
    public:
      /// Do not initialize
      EngineInfo(void);
      /// Initialize
      EngineInfo(EngineType et, unsigned int fst, unsigned int lst);
      /// \name Engine type information
      //@{
      /// Return engine type
      EngineType type(void) const;
      /// Return whether engine is a meta engine
      bool meta(void) const;
      //@}
      /// \name Information for basic (non-meta) engines
      //@{
      /// Return id of first worker
      unsigned int wfst(void) const;
      /// Return id of last worker plus one
      unsigned int wlst(void) const;
      /// Return number of workers
      unsigned int workers(void) const;
      //@}
      /// \name Information for meta engines
      //@{
      /// Return id of first engine
      unsigned int efst(void) const;
      /// Return id of last engine
      unsigned int elst(void) const;
      /// Return number of engines
      unsigned int engines(void) const;
      //@}
    };
    /// Edge information
    class EdgeInfo {
    protected:
      /// The parent worker id  (edge does not exist if UINT_MAX)
      unsigned int _wid;
      /// The parent node id
      unsigned int _nid;
      /// Number of alternative
      unsigned int _a;
      /// String corresponding to alternative
      std::string _s;
    public:
      /// Initialize
      void init(unsigned int wid, unsigned int nid, unsigned int a);
      /// Initialize
      void init(unsigned int wid, unsigned int nid, unsigned int a,
                const Space& s, const Choice & c);
      /// Invalidate edge information (for stealing)
      void invalidate(void);
      /// Initialize as non existing
      EdgeInfo(void);
      /// Initialize
      EdgeInfo(unsigned int wid, unsigned int nid, unsigned int a);
      /// Test whether edge actually exists
      operator bool(void) const;
      /// Return parent worker id
      unsigned int wid(void) const;
      /// Return parent node id
      unsigned int nid(void) const;
      /// Return number of alternative
      unsigned int alternative(void) const;
      /// Return string for alternative
      std::string string(void) const;
    };
    /// Node type
    enum NodeType {
      SOLVED  = 0, /// A solution node
      FAILED  = 1, /// A failed node
      BRANCH  = 2  /// A branch node
    };
    /// Node information
    class NodeInfo {
    protected:
      /// The node type
      NodeType _nt;
      /// The worker id
      unsigned int _wid;
      /// The node id
      unsigned int _nid;
      /// The corresponding space
      const Space& _s;
      /// The corresponding choice (nullptr if type is not BRANCH)
      const Choice* _c;
    public:
      /// Initialize node info
      NodeInfo(NodeType nt,
               unsigned int wid, unsigned int nid,
               const Space& s, const Choice* c = nullptr);
      /// Return node type
      NodeType type(void) const;
      /// Return worker id
      unsigned int wid(void) const;
      /// Return node id
      unsigned int nid(void) const;
      /// Return corresponding space
      const Space& space(void) const;
      /// Return corresponding choice
      const Choice& choice(void) const;
    };
  private:
    /// Mutex for serialized access
    Support::Mutex m;
    /// Number of pending engine and workers calls
    unsigned int pending;
    /// Number of engines
    unsigned int n_e;
    /// Number of workers
    unsigned int n_w;
    /// Number of active workers (for termination)
    unsigned int n_active;
    /// The engines
    Support::DynamicArray<EngineInfo,Heap> es;
    /// Mapping of workers to engines
    Support::DynamicArray<unsigned int,Heap> w2e;
    /// Register new engine
    void engine(EngineType t, unsigned int n);
    /// Register new worker
    void worker(unsigned int& wid, unsigned int& eid);
    /// Deregister a worker
    void worker(void);
    /// \name Unsynchronized internal calls
    //{@
    /// The engine with id \a eid goes to a next round (restart or next iteration in LDS)
    void _round(unsigned int eid);
    /// The engine skips an edge
    void _skip(const EdgeInfo& ei);
    /// The engine creates a new node with information \a ei and \a ni
    void _node(const EdgeInfo& ei, const NodeInfo& ni);
    //@}
  public:
    /// Initialize
    SearchTracer(void);
    /// \name Engine information
    //@{
    /// Return number of workers
    unsigned int workers(void) const;
    /// Return number of engines
    unsigned int engines(void) const;
    /// Provide access to engine with id \a eid
    const EngineInfo& engine(unsigned int eid) const;
    /// Return the engine id of a worker with id \a wid
    unsigned int eid(unsigned int wid) const;
    //@}
    /// \name Trace event functions
    //@{
    /// The search engine initializes
    virtual void init(void) = 0;
    /// The engine with id \a eid goes to a next round (restart or next iteration in LDS)
    virtual void round(unsigned int eid) = 0;
    /// The engine skips an edge
    virtual void skip(const EdgeInfo& ei) = 0;
    /// The engine creates a new node with information \a ei and \a ni
    virtual void node(const EdgeInfo& ei, const NodeInfo& ni) = 0;
    /// All workers are done
    virtual void done(void) = 0;
    //@}
    /// Delete
    virtual ~SearchTracer(void);
  };

  class GECODE_SEARCH_EXPORT StdSearchTracer : public SearchTracer {
  protected:
    /// Output stream to use
    std::ostream& os;
    /// Map engine type to string
    static const char* t2s[EngineType::AOE + 1];
  public:
    /// Initialize  with output stream \a os
    StdSearchTracer(std::ostream& os = std::cerr);
    /// The search engine initializes
    virtual void init(void);
    /// The engine with id \a eid goes to a next round (restart or next iteration in LDS)
    virtual void round(unsigned int eid);
    /// The engine skips an edge
    virtual void skip(const EdgeInfo& ei);
    /// The engine creates a new node with information \a ei and \a ni
    virtual void node(const EdgeInfo& ei, const NodeInfo& ni);
    /// All workers are done
    virtual void done(void);
    /// Delete
    virtual ~StdSearchTracer(void);
    /// Default tracer (printing to std::cerr)
    static StdSearchTracer def;
  };

}

#include <gecode/search/tracer.hpp>
#include <gecode/search/trace-recorder.hpp>

#ifdef GECODE_HAS_CPPROFILER

namespace Gecode {

  /// Code that is specific to the CPProfiler
  namespace CPProfiler {}

}

namespace Gecode { namespace CPProfiler {

  /// The actual connector to the CPProfiler
  class Connector;

}}

namespace Gecode {

  /// Class to record search trace info for CPProfiler
  class GECODE_SEARCH_EXPORT CPProfilerSearchTracer : public SearchTracer {
  public:
    /// Class to send solution information to CPProfiler
    class GECODE_SEARCH_EXPORT GetInfo : public HeapAllocated {
    public:
      /// Initialize
      GetInfo(void);
      /// Return info for a space
      virtual std::string getInfo(const Space& home) const = 0;
      /// Delete
      virtual ~GetInfo(void);
    };
  private:
    /// Connector to connect to running instnace of CPProfiler
    CPProfiler::Connector* connector;
    /// Execution id to be displayed by CPProfiler
    int execution_id;
    /// Name of script being traced
    std::string name;
    /// Number of the current restart
    int restart;
    /// Send solution information to CPProfiler
    const GetInfo* pgi;
  public:
    /// Initialize
    CPProfilerSearchTracer(int eid, std::string name,
                           unsigned int port = Search::Config::cpprofiler_port,
                           const GetInfo* pgi = nullptr);
    /// The search engine initializes
    virtual void init(void);
    /// The engine with id \a eid goes to a next round (restart or next iteration in LDS)
    virtual void round(unsigned int eid);
    /// The engine skips an edge
    virtual void skip(const EdgeInfo& ei);
    /// The engine creates a new node with information \a ei and \a ni
    virtual void node(const EdgeInfo& ei, const NodeInfo& ni);
    /// All workers are done
    virtual void done(void);
    /// Delete
    virtual ~CPProfilerSearchTracer(void);
  };

}

#endif

namespace Gecode { namespace Search {

  /**
   * \brief Base class for cutoff generators for restart-based meta engine
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT Cutoff : public HeapAllocated {
  public:
    /// \name Constructors and member functions
    //@{
    /// Default constructor
    Cutoff(void);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const = 0;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void) = 0;
    /// Destructor
    virtual ~Cutoff(void);
    //@}
    /// \name Predefined cutoff generators
    //@{
    /// Create generator for constant sequence with constant \a s
    static Cutoff*
    constant(unsigned long int scale=Config::slice);
    /// Create generator for linear sequence scaled by \a scale
    static Cutoff*
    linear(unsigned long int scale=Config::slice);
    /** Create generator for geometric sequence scaled by
     *  \a scale using base \a base
     */
    static Cutoff*
    geometric(unsigned long int scale=Config::slice, double base=Config::base);
    /// Create generator for luby sequence with scale-factor \a scale
    static Cutoff*
    luby(unsigned long int scale=Config::slice);
    /** Create generator for random sequence with seed \a seed that
     *  generates values between \a min and \a max with \a n steps
     *  between the extreme values (use 0 for \a n to get step size 1).
     */
    static Cutoff*
    rnd(unsigned int seed,
        unsigned long int min, unsigned long int max,
        unsigned long int n);
    /// Append cutoff values from \a c2 after \a n values from \a c1
    static Cutoff*
    append(Cutoff* c1, unsigned long int n, Cutoff* c2);
    /// Merge cutoff values from \a c1 with values from \a c2
    static Cutoff*
    merge(Cutoff* c1, Cutoff* c2);
    /// Create generator that repeats \a n times each cutoff value from \a c
    static Cutoff*
    repeat(Cutoff* c, unsigned long int n);
    //@}
  };

  /**
   * \brief Cutoff generator for constant sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffConstant : public Cutoff {
  protected:
    /// Constant
    unsigned long int c;
  public:
    /// Constructor
    CutoffConstant(unsigned long int c);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
  };

  /**
   * \brief Cutoff generator for linear sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffLinear : public Cutoff {
  protected:
    /// Scale factor
    unsigned long int scale;
    /// Next number in sequence
    unsigned long int n;
  public:
    /// Constructor
    CutoffLinear(unsigned long int scale);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
  };

  /**
   * \brief Cutoff generator for the Luby sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffLuby : public Cutoff {
  protected:
    /// Iteration number
    unsigned long int i;
    /// Scale factor
    unsigned long int scale;
    /// Number of pre-computed luby values
    static const unsigned long int n_start = 63U;
    /// Precomputed luby-values
    static unsigned long int start[n_start];
    /// Compute binary logarithm of \a i
    static unsigned long int log(unsigned long int i);
    /// Compute Luby number for step \a i
    static unsigned long int luby(unsigned long int i);
  public:
    /// Constructor
    CutoffLuby(unsigned long int scale);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
  };

  /**
   * \brief Cutoff generator for the geometric sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffGeometric : public Cutoff {
  protected:
    /// Current cutoff value
    double n;
    /// Scale factor
    double scale;
    /// Base
    double base;
  public:
    /// Constructor
    CutoffGeometric(unsigned long int scale, double base);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
  };

  /**
   * \brief Cutoff generator for the random sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffRandom : public Cutoff {
  protected:
    /// Random number generator
    Support::RandomGenerator rnd;
    /// Minimum cutoff value
    unsigned long int min;
    /// Random values
    unsigned long int n;
    /// Step size
    unsigned long int step;
    /// Current value
    unsigned long int cur;
  public:
    /// Constructor
    CutoffRandom(unsigned int seed,
                 unsigned long int min, unsigned long int max,
                 unsigned long int n);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
  };

  /**
   * \brief Cutoff generator appending two cutoff generators
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffAppend : public Cutoff {
  protected:
    /// First cutoff generators
    Cutoff* c1;
    /// Second cutoff generators
    Cutoff* c2;
    /// How many number to take from the first
    unsigned long int n;
  public:
    /// Constructor
    CutoffAppend(Cutoff* c1, unsigned long int n, Cutoff* c2);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
    /// Destructor
    virtual ~CutoffAppend(void);
  };

  /**
   * \brief Cutoff generator merging two cutoff generators
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffMerge : public Cutoff {
  protected:
    /// First cutoff generator
    Cutoff* c1;
    /// Second cutoff generator
    Cutoff* c2;
  public:
    /// Constructor
    CutoffMerge(Cutoff* c1, Cutoff* c2);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
    /// Destructor
    virtual ~CutoffMerge(void);
  };

  /**
   * \brief Cutoff generator that repeats a cutoff from another cutoff generator
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffRepeat : public Cutoff {
  protected:
    /// Actual cutoff generator
    Cutoff* c;
    // Current cutoff
    unsigned int cutoff;
    // Iteration
    unsigned long int i;
    // Number of repetitions
    unsigned long int n;
  public:
    /// Constructor
    CutoffRepeat(Cutoff* c, unsigned long int n);
    /// Return the current cutoff value
    virtual unsigned long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long int operator ++(void);
    /// Destructor
    virtual ~CutoffRepeat(void);
  };

}}

#include <gecode/search/cutoff.hpp>

namespace Gecode { namespace Search {

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
      /// Discrepancy limit (for LDS)
      unsigned int d_l;
      /// Number of assets (engines) in a portfolio
      unsigned int assets;
      /// Size of a slice in a portfolio (in number of failures)
      unsigned int slice;
      /// Depth limit for extraction of no-goods
      unsigned int nogoods_limit;
      /// Stop object for stopping search
      Stop* stop;
      /// Cutoff for restart-based search
      Cutoff* cutoff;
      /// Tracer object for tracing search
      SearchTracer* tracer;
      /// Default options
      GECODE_SEARCH_EXPORT static const Options def;
      /// Initialize with default values
      Options(void);
      /// Expand with real number of threads
      GECODE_SEARCH_EXPORT Options
      expand(void) const;
    };

}}

#include <gecode/search/options.hpp>

namespace Gecode { namespace Search {

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
  class GECODE_SEARCH_EXPORT Stop : public HeapAllocated {
  public:
    /// \name Constructors and member functions
    //@{
    /// Default constructor
    Stop(void);
    /// Stop search, if returns true
    virtual bool stop(const Statistics& s, const Options& o) = 0;
    /// Destructor
    virtual ~Stop(void);
    //@}
    /// \name Predefined stop objects
    //@{
    /// Stop if node limit \a l has been exceeded
    static Stop* node(unsigned long int l);
    /// Stop if failure limit \a l has been exceeded
    static Stop* fail(unsigned long int l);
    /// Stop if time limit \a l (in milliseconds) has been exceeded
    static Stop* time(unsigned long int l);
    //@}
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

}}

#include <gecode/search/stop.hpp>

namespace Gecode { namespace Search {

  /**
   * \brief %Search engine implementation interface
   */
  class GECODE_SEARCH_EXPORT Engine : public HeapAllocated {
  public:
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void) = 0;
    /// Return statistics
    virtual Statistics statistics(void) const = 0;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const = 0;
    /// Constrain future solutions to be better than \a b (raises exception)
    virtual void constrain(const Space& b);
    /// Reset engine to restart at space \a s (does nothing)
    virtual void reset(Space* s);
    /// Return no-goods (the no-goods are empty)
    virtual NoGoods& nogoods(void);
    /// Destructor
    virtual ~Engine(void);
  };

}}

#include <gecode/search/engine.hpp>

namespace Gecode { namespace Search {

  /// Base-class for search engines
  template<class T>
  class Base : public HeapAllocated {
    template<class, class>
    friend Engine* build(Space*, const Options&);
    template<class, template<class> class>
    friend Engine* build(Space*, const Options&);
  protected:
    /// The actual search engine
    Engine* e;
    /// Constructor
    Base(Engine* e = NULL);
  public:
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual T* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Destructor
    virtual ~Base(void);
  private:
    /// Disallow copy constructor
    Base(const Base&);
    /// Disallow assigment operator
    Base& operator =(const Base&);
  };

}}

#include <gecode/search/base.hpp>

namespace Gecode { namespace Search {

  /// Build an engine of type \a E for a script \a T
  template<class T, class E>
  Engine* build(Space* s, const Options& opt);
  /// Build a parametric engine of type \a E for a script \a T
  template<class T, template<class> class E>
  Engine* build(Space* s, const Options& opt);

  /// A class for building search engines
  class GECODE_SEARCH_EXPORT Builder : public HeapAllocated {
  protected:
    /// Stored and already expanded options
    Options opt;
    /// Whether engine to be built is a best solution search engine
    const bool b;
  public:
    /// Initialize with options \a opt and \a best solution search support
    Builder(const Options& opt, bool best);
    /// Provide access to options
    Options& options(void);
    /// Provide access to options
    const Options& options(void) const;
    /// Whether engine is a best solution search engine
    bool best(void) const;
    /// Build an engine according to stored options for \a s
    virtual Engine* operator() (Space* s) const = 0;
    /// Destructor
    virtual ~Builder(void);
  };

}}

#include <gecode/search/build.hpp>

namespace Gecode {

  /// Type for a search engine builder
  typedef Search::Builder* SEB;

}

#include <gecode/search/traits.hpp>

namespace Gecode {

  /// Passing search engine builder arguments
  class SEBs : public ArgArray<SEB> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    SEBs(void);
    /// Allocate array with \a n elements
    explicit SEBs(int n);
    /// Allocate array and copy elements from \a x
    SEBs(const std::vector<SEB>& x);
    /// Allocate array with and initialize with \a x
    SEBs(std::initializer_list<SEB> x);
    /// Allocate array and copy elements from \a first to \a last
    template<class InputIterator>
    SEBs(InputIterator first, InputIterator last);
    /// Initialize from primitive argument array \a a (copy elements)
    SEBs(const ArgArray<SEB>& a);
    //@}
  };

}

#include <gecode/search/sebs.hpp>

namespace Gecode {

  /**
   * \brief Depth-first search engine
   *
   * This class supports depth-first search for subclasses \a T of
   * Space.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class DFS : public Search::Base<T> {
  public:
    /// Initialize search engine for space \a s with options \a o
    DFS(T* s, const Search::Options& o=Search::Options::def);
    /// Whether engine does best solution search
    static const bool best = false;
  };

  /// Invoke depth-first search engine for subclass \a T of space \a s with options \a o
  template<class T>
  T* dfs(T* s, const Search::Options& o=Search::Options::def);

  /// Return a depth-first search engine builder
  template<class T>
  SEB dfs(const Search::Options& o=Search::Options::def);

}

#include <gecode/search/dfs.hpp>

namespace Gecode {

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
  class BAB : public Search::Base<T> {
  public:
    /// Initialize engine for space \a s and options \a o
    BAB(T* s, const Search::Options& o=Search::Options::def);
    /// Whether engine does best solution search
    static const bool best = true;
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

  /// Return a depth-first branch-and-bound search engine builder
  template<class T>
  SEB bab(const Search::Options& o=Search::Options::def);

}

#include <gecode/search/bab.hpp>

namespace Gecode {

  /**
   * \brief Limited discrepancy search engine
   * \ingroup TaskModelSearch
   */
  template<class T>
  class LDS : public Search::Base<T> {
  public:
    /// Initialize engine for space \a s and options \a o
    LDS(T* s, const Search::Options& o=Search::Options::def);
    /// Whether engine does best solution search
    static const bool best = false;
  };

  /**
   * \brief Invoke limited-discrepancy search for \a s as root node and options\a o
   * \ingroup TaskModelSearch
   */
  template<class T>
  T* lds(T* s, const Search::Options& o=Search::Options::def);

  /// Return a limited discrepancy search engine builder
  template<class T>
  SEB lds(const Search::Options& o=Search::Options::def);

}

#include <gecode/search/lds.hpp>

namespace Gecode {

  /**
   * \brief Meta-engine performing restart-based search
   *
   * The engine uses the Cutoff sequence supplied in the options \a o to
   * periodically restart the search of engine \a E.
   *
   * The class \a T can implement member functions
   * \code virtual bool master(const MetaInfo& mi) \endcode
   * and
   * \code virtual bool slave(const MetaInfo& mi) \endcode
   *
   * Whenever exploration restarts or a solution is found, the
   * engine executes the functions on the master and slave
   * space. For more details, consult "Modeling and Programming
   * with Gecode".
   *
   * \ingroup TaskModelSearch
   */
  template<class T, template<class> class E = DFS>
  class RBS : public Search::Base<T> {
    using Search::Base<T>::e;
  public:
    /// Initialize engine for space \a s and options \a o
    RBS(T* s, const Search::Options& o);
    /// Whether engine does best solution search
    static const bool best = E<T>::best;
  };

  /**
   * \brief Perform restart-based search
   *
   * The engine uses the Cutoff sequence supplied in the options \a o to
   * periodically restart the search of engine \a E.
   *
   * The class \a T can implement member functions
   * \code virtual bool master(const MetaInfo& mi) \endcode
   * and
   * \code virtual bool slave(const MetaInfo& mi) \endcode
   *
   * Whenever exploration restarts or a solution is found, the
   * engine executes the functions on the master and slave
   * space. For more details, consult "Modeling and Programming
   * with Gecode".
   *
   * \ingroup TaskModelSearch
   */
  template<class T, template<class> class E>
  T* rbs(T* s, const Search::Options& o);

  /// Return a restart search engine builder
  template<class T, template<class> class E>
  SEB rbs(const Search::Options& o);

}

#include <gecode/search/rbs.hpp>

namespace Gecode { namespace Search { namespace Meta {

  /// Build a sequential engine
  template<class T, template<class> class E>
  Engine* sequential(T* master, const Search::Statistics& stat, Options& opt);

  /// Build a sequential engine
  template<class T, template<class> class E>
  Engine* sequential(T* master, SEBs& sebs,
                     const Search::Statistics& stat, Options& opt, bool best);

#ifdef GECODE_HAS_THREADS

  /// Build a parallel engine
  template<class T, template<class> class E>
  Engine* parallel(T* master, const Search::Statistics& stat, Options& opt);

  /// Build a parallel engine
  template<class T, template<class> class E>
  Engine* parallel(T* master, SEBs& sebs,
                   const Search::Statistics& stat, Options& opt, bool best);

#endif

}}}

namespace Gecode {

  /**
   * \brief Meta engine using a portfolio of search engines
   *
   * The engine will run a portfolio with a number of assets as defined
   * by the options \a o. The engine supports parallel execution of
   * assets by using the number of threads as defined by the options.
   *
   * The class \a T can implement member functions
   * \code virtual bool master(const MetaInfo& mi) \endcode
   * and
   * \code virtual bool slave(const MetaInfo& mi) \endcode
   *
   * When the assets are created, these functions are executed.
   * For more details, consult "Modeling and Programming with Gecode".
   *
   * \ingroup TaskModelSearch
   */
  template<class T, template<class> class E = DFS>
  class PBS : public Search::Base<T> {
    using Search::Base<T>::e;
  protected:
    /// The actual build function
    void build(T* s, SEBs& sebs, const Search::Options& o);
  public:
    /// Initialize with engines running copies of \a s with options \a o
    PBS(T* s, const Search::Options& o=Search::Options::def);
    /// Initialize with engine builders \a sebs
    PBS(T* s, SEBs& sebs, const Search::Options& o=Search::Options::def);
    /// Whether engine does best solution search
    static const bool best = E<T>::best;
  };

  /**
   * \brief Run a portfolio of search engines
   *
   * The engine will run a portfolio with a number of assets as defined
   * by the options \a o. The engine supports parallel execution of
   * assets by using the number of threads as defined by the options.
   *
   * The class \a T can implement member functions
   * \code virtual bool master(const MetaInfo& mi) \endcode
   * and
   * \code virtual bool slave(const MetaInfo& mi) \endcode
   *
   * When the assets are created, these functions are executed.
   * For more details, consult "Modeling and Programming with Gecode".
   *
   * \ingroup TaskModelSearch
   */
  template<class T, template<class> class E>
  T* pbs(T* s, const Search::Options& o=Search::Options::def);

  /// Return a portfolio search engine builder
  template<class T>
  SEB pbs(const Search::Options& o=Search::Options::def);

}

#include <gecode/search/pbs.hpp>

#endif

// STATISTICS: search-other
