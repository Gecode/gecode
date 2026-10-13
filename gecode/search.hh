/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Kevin Leo <kevin.leo@monash.edu>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Maxim Shishmarev <maxim.shishmarev@monash.edu>
 *
 *  Copyright:
 *     Kevin Leo, 2017
 *     Christian Schulte, 2002
 *     Mikael Zayenz Lagerkvist, 2026
 *     Maxim Shishmarev, 2017
 *     Guido Tack, 2004
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

#ifndef GECODE_SEARCH_HH
#define GECODE_SEARCH_HH

#include <chrono>
#include <initializer_list>
#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>
#include <utility>

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
    unsigned long long int fail;
    /// Number of nodes expanded
    unsigned long long int node;
    /// Maximum depth of search stack
    unsigned long int depth;
    /// Number of restarts
    unsigned long int restart;
    /// Number of no-goods posted
    unsigned long int nogood;
    /// Number of arrivals or live candidates removed by sound state pruning
    unsigned long long int state_pruned;
    /// Peak number of retained state descriptors
    unsigned long int state_descriptors;
    /// Peak owned descriptor bytes, excluding container/allocator overhead
    std::size_t state_bytes;
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
    /// Connector to connect to running instance of CPProfiler
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
    virtual unsigned long long int operator ()(void) const = 0;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void) = 0;
    /// Destructor
    virtual ~Cutoff(void);
    //@}
    /// \name Predefined cutoff generators
    //@{
    /// Create generator for constant sequence with constant \a s
    static Cutoff*
    constant(unsigned long long int scale=Config::slice);
    /// Create generator for linear sequence scaled by \a scale
    static Cutoff*
    linear(unsigned long long int scale=Config::slice);
    /** Create generator for geometric sequence scaled by
     *  \a scale using base \a base
     */
    static Cutoff*
    geometric(unsigned long long int scale=Config::slice,
              double base=Config::base);
    /// Create generator for luby sequence with scale-factor \a scale
    static Cutoff*
    luby(unsigned long long int scale=Config::slice);
    /** Create generator for random sequence with seed \a seed that
     *  generates values between \a min and \a max with \a n steps
     *  between the extreme values.
     */
    static Cutoff*
    rnd(unsigned int seed,
        unsigned long long int min, unsigned long long int max,
        unsigned long long int n);
    /// Append cutoff values from \a c2 after \a n values from \a c1
    static Cutoff*
    append(Cutoff* c1, unsigned long long int n, Cutoff* c2);
    /// Merge cutoff values from \a c1 with values from \a c2
    static Cutoff*
    merge(Cutoff* c1, Cutoff* c2);
    /// Create generator that repeats \a n times each cutoff value from \a c
    static Cutoff*
    repeat(Cutoff* c, unsigned long long int n);
    //@}
  };

  /**
   * \brief Cutoff generator for constant sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffConstant : public Cutoff {
  protected:
    /// Constant
    unsigned long long int c;
  public:
    /// Constructor
    CutoffConstant(unsigned long long int c);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
  };

  /**
   * \brief Cutoff generator for linear sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffLinear : public Cutoff {
  protected:
    /// Scale factor
    unsigned long long int scale;
    /// Next number in sequence
    unsigned long long int n;
  public:
    /// Constructor
    CutoffLinear(unsigned long long int scale);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
  };

  /**
   * \brief Cutoff generator for the Luby sequence
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT CutoffLuby : public Cutoff {
  protected:
    /// Iteration number
    unsigned long long int i;
    /// Scale factor
    unsigned long long int scale;
    /// Number of pre-computed luby values
    static const unsigned long long int n_start = 63U;
    /// Precomputed luby-values
    static unsigned long int start[n_start];
    /// Compute binary logarithm of \a i
    static unsigned long long int log(unsigned long long int i);
    /// Compute Luby number for step \a i
    static unsigned long long int luby(unsigned long long int i);
  public:
    /// Constructor
    CutoffLuby(unsigned long long int scale);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
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
    CutoffGeometric(unsigned long long int scale, double base);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
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
    unsigned long long int min;
    /// Random values
    unsigned long long int n;
    /// Step size
    unsigned long long int step;
    /// Current value
    unsigned long long int cur;
  public:
    /// Constructor
    CutoffRandom(unsigned int seed,
                 unsigned long long int min, unsigned long long int max,
                 unsigned long long int n);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
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
    unsigned long long int n;
  public:
    /// Constructor
    CutoffAppend(Cutoff* c1, unsigned long long int n, Cutoff* c2);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
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
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
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
    unsigned long long int cutoff;
    // Iteration
    unsigned long long int i;
    // Number of repetitions
    unsigned long long int n;
  public:
    /// Constructor
    CutoffRepeat(Cutoff* c, unsigned long long int n);
    /// Return the current cutoff value
    virtual unsigned long long int operator ()(void) const;
    /// Increment and return the next cutoff value
    virtual unsigned long long int operator ++(void);
    /// Destructor
    virtual ~CutoffRepeat(void);
  };

}}

#include <gecode/search/cutoff.hpp>

namespace Gecode { namespace Search {

  class WorkerControlAccess;

  /**
   * \brief External control for the requested number of search workers
   *
   * A worker control is a copyable handle. Copies share the same request
   * and can be used concurrently. The worker capacity is fixed when the
   * handle is first attached to a search engine.
   * Without thread support, concurrent use and pausing are unsupported:
   * attaching a zero request or calling request(0) raises InvalidWorkerRequest.
   *
   * \ingroup TaskModelSearch
   */
  class GECODE_SEARCH_EXPORT WorkerControl {
  private:
    class State;
    State* state;
    friend class WorkerControlAccess;
  public:
    /// Construct an empty handle
    WorkerControl(void) noexcept;
    /// Construct an engaged handle with initial request \a requested (zero pauses)
    explicit WorkerControl(unsigned int requested);
    /// Copy constructor
    WorkerControl(const WorkerControl& control);
    /// Assignment operator
    WorkerControl& operator =(const WorkerControl& control);
    /// Destructor
    ~WorkerControl(void);
    /// Whether this handle is engaged
    bool engaged(void) const noexcept;
    /// Return the requested number of workers (zero also denotes an empty handle)
    unsigned int requested(void) const noexcept;
    /// Request \a workers workers (zero pauses the engine)
    void request(unsigned int workers);
    /// Return the fixed worker capacity, or zero before attachment
    unsigned int capacity(void) const noexcept;
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
      /**
       * External worker control
       *
       * After option expansion, \a threads is the immutable worker capacity.
       * Requests through this handle never change \a threads.
       */
      WorkerControl worker_control;
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

#include <gecode/search/worker-control.hh>

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
    static Stop* node(unsigned long long int l);
    /// Stop if failure limit \a l has been exceeded
    static Stop* fail(unsigned long long int l);
    /// Stop if time limit \a l (in milliseconds) has been exceeded
    static Stop* time(double l);
    /// Stop if restart limit \a l has been exceeded
    static Stop* restart(unsigned long long int l);
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
    std::atomic<unsigned long long int> l;
  public:
    /// Stop if node limit \a l is exceeded
    NodeStop(unsigned long long int l);
    /// Copy constructor
    NodeStop(const NodeStop& s);
    /// Assignment operator
    NodeStop& operator =(const NodeStop& s);
    /// Return current limit
    unsigned long long int limit(void) const;
    /// Set current limit to \a l nodes
    void limit(unsigned long long int l);
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
    std::atomic<unsigned long long int> l;
  public:
    /// Stop if failure limit \a l is exceeded
    FailStop(unsigned long long int l);
    /// Copy constructor
    FailStop(const FailStop& s);
    /// Assignment operator
    FailStop& operator =(const FailStop& s);
    /// Return current limit
    unsigned long long int limit(void) const;
    /// Set current limit to \a l failures
    void limit(unsigned long long int l);
    /// Return true if failure limit is exceeded
    virtual bool stop(const Statistics& s, const Options& o);
  };

  /**
   * \brief %Stop-object based on time
   * \ingroup TaskModelSearchStop
   */
  class GECODE_SEARCH_EXPORT TimeStop : public Stop {
  protected:
    using clock = std::chrono::steady_clock;
    using clock_rep = clock::time_point::rep;
    /// Clock representation at the start of timing
    std::atomic<clock_rep> t0;
    /// Current limit in milliseconds
    std::atomic<double> l;
  public:
    /// Stop if search exceeds \a l milliseconds (from creation of this object)
    TimeStop(double l);
    /// Copy constructor
    TimeStop(const TimeStop& s);
    /// Assignment operator
    TimeStop& operator =(const TimeStop& s);
    /// Return current limit in milliseconds
    double limit(void) const;
    /// Set current limit to \a l milliseconds
    void limit(double l);
    /// Reset time to zero
    void reset(void);
    /// Return true if time limit is exceeded
    virtual bool stop(const Statistics& s, const Options& o);
  };

  /**
   * \brief %Stop-object based on number of restarts
   * \ingroup TaskModelSearchStop
   */
  class GECODE_SEARCH_EXPORT RestartStop : public Stop {
  protected:
    /// Restart limit
    std::atomic<unsigned long long int> l;
  public:
    /// Stop if restart limit \a l is exceeded
    RestartStop(unsigned long long int l);
    /// Copy constructor
    RestartStop(const RestartStop& s);
    /// Assignment operator
    RestartStop& operator =(const RestartStop& s);
    /// Return current limit
    unsigned long long int limit(void) const;
    /// Set current limit to \a l restarts
    void limit(unsigned long long int l);
    /// Return true if failure limit is exceeded
    virtual bool stop(const Statistics& s, const Options& o);
  };

}}

#include <gecode/search/stop.hpp>

namespace Gecode { namespace Search {

  /**
   * \brief %Search engine implementation interface
   */
  class GECODE_SEARCH_EXPORT Engine : public HeapAllocated {
  protected:
    /// Control retained for the lifetime of a leaf engine
    WorkerControl worker_control;
    /// Construct a meta engine without worker control
    Engine(void);
    /// Construct a leaf engine and bind worker control to \a capacity
    Engine(const Options& o, unsigned int capacity);
  public:
    /// Return next solution (nullptr, if none exists or search has been stopped)
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
    Base(Engine* e = nullptr);
  public:
    /// Return next solution (nullptr, if none exists or search has been stopped)
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
    /// Disallow assignment operator
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

namespace Gecode { namespace Search {

  /**
   * \brief Read-only guidance for scalar integer heuristic search
   *
   * The bound is an optimistic total objective value on a stable Space:
   * a lower bound for minimization, an upper bound for maximization, and
   * the exact objective on a solution. It must agree with Space::compare
   * and Space::constrain. The engine adds no prefix cost. Engines define how
   * they use the optional lower-first ranking; rank never justifies pruning.
   * Callbacks and their captured
   * dependencies must remain valid for all engine clones and must not post
   * constraints or refer to another Space's local state.
   * \ingroup TaskModelSearch
   */
  class HeuristicGuidance {
  public:
    /// Objective direction
    enum Sense { MINIMIZE, MAXIMIZE };
    /// Evaluate a propagated stable space
    typedef std::function<std::int64_t(const Space&)> Evaluate;
    /// Objective direction
    Sense sense;
    /// Optimistic total objective bound (required)
    Evaluate bound;
    /// Optional ranking (lower first)
    Evaluate rank;
    /// Initialize guidance
    HeuristicGuidance(Sense sense, Evaluate bound, Evaluate rank=Evaluate());
  };

  /// Fixed exploration order for native best-first search
  enum BestFirstOrder {
    BOUND_FIRST, ///< Optimistic bound, lower rank, then insertion order
    RANK_FIRST   ///< Lower rank, optimistic bound, then insertion order
  };

  /**
   * \brief Optional model-justified exact-state and label pruning
   *
   * Descriptors own values only, never native handles or Spaces. Equal hashes
   * select comparisons, but exact keys must also match. The author guarantees
   * that a key includes every fact affecting residual feasibility and
   * brancher/search progress. Equivalence/dominance must preserve an
   * at-least-as-good completion, including prefix objective cost, and a route
   * to a solution under repeated pruning. A sufficient argument preserves such
   * a completion with no more remaining search transitions. Identical domains
   * and costs at successive brancher stages alone are insufficient: a retained
   * ancestor descriptor could suppress its only route to a goal. Include
   * sufficient stage state in the key or decline extraction; no stage field
   * is required or inferred. Relations are directional, consistent and
   * transitive. An arbitrary heuristic bound does not establish a sound label
   * relation.
   * Callbacks read stable Spaces and follow HeuristicGuidance's lifetime rules.
   * Both callbacks empty disable all extraction and retained descriptor work;
   * supplying only one is invalid. Extraction may decline individual nodes.
   * \ingroup TaskModelSearch
   */
  class StatePruning {
  public:
    /// Owned exact grouping key and cost/resource label
    struct Descriptor {
      std::size_t hash;
      std::vector<std::int64_t> key, label;
      /// Object and vector capacity bytes, excluding container/allocator overhead
      std::size_t storage(void) const {
        return sizeof(Descriptor) +
          (key.capacity()+label.capacity())*sizeof(std::int64_t);
      }
    };
    /// Relation of the first descriptor to the second, with identical keys
    enum Relation { EQUIVALENT, DOMINATES, DOMINATED, INCOMPARABLE };
    /// Extract owned state, or decline when sufficient state is unavailable
    typedef std::function<std::optional<Descriptor>(const Space&)> Extract;
    /// Compare labels of descriptors with exactly equal grouping keys
    typedef std::function<Relation(const Descriptor&,const Descriptor&)> Compare;
    Extract extract;
    Compare compare;
    /// Metadata ceiling; zero is unlimited. Forgetting never removes search work.
    unsigned long int descriptor_limit;
    /// Initialize optional extraction, relation and metadata ceiling
    StatePruning(Extract extract=Extract(), Compare compare=Compare(),
                 unsigned long int descriptor_limit=0);
  };

}}

namespace Gecode {

  /**
   * \brief Sequential bound-ordered A* with improving solution publication
   *
   * Spaces must implement compatible scalar integer comparison and strict
   * improvement constraints. A generated solution can precede a better one;
   * continue next() until nullptr with stopped()==false to prove optimality of
   * the best accepted incumbent, including external adoption, or infeasibility
   * if no incumbent was accepted. Returned solutions belong to the caller.
   *
   * Equal bounds use lower rank, then insertion order.
   * The frontier stores clones; c_d and a_d do not control recomputation.
   * There is no hard memory guarantee and no-good extraction is empty.
   * Only one expanded thread and no tracer are supported. Capacity-one
   * worker control and ordinary resumable stops are supported.
   * If guidance, state pruning, or comparison throws, destroy the engine before further use.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class AStar : public Search::Base<T> {
  public:
    /// Initialize from space, guidance, and ordinary search options
    AStar(T* s, const Search::HeuristicGuidance& guidance,
          const Search::Options& o=Search::Options::def,
          const Search::StatePruning& pruning=Search::StatePruning());
    /// Adopt a strictly improving compatible external incumbent
    void constrain(const Space& incumbent);
    /// Whether engine does best solution search
    static const bool best = true;
  };

  /// Return an A* engine builder with copied guidance
  template<class T>
  SEB astar(const Search::HeuristicGuidance& guidance,
            const Search::Options& o=Search::Options::def,
            const Search::StatePruning& pruning=Search::StatePruning());

  /**
   * \brief Return a native best-first builder with copied guidance and fixed order
   *
   * Uses the same engine and contracts as AStar. BOUND_FIRST preserves AStar
   * ordering; RANK_FIRST permits arbitrary, inadmissible lower-first ranking.
   * Missing rank falls back to bound order. Ties use insertion order. Values
   * are cached for each admitted stable node; reset preserves the order.
   * Rank selects pending branching Spaces only and never prunes. Generated
   * solutions are published immediately, so the first need not be optimal.
   * Natural finite-tree exhaustion with stopped()==false proves closure under
   * the bound, comparison, and optional sound StatePruning contracts.
   * \ingroup TaskModelSearch
   */
  template<class T>
  SEB bestfirst(const Search::HeuristicGuidance& guidance,
                Search::BestFirstOrder ordering,
                const Search::Options& o=Search::Options::def,
                const Search::StatePruning& pruning=Search::StatePruning());

}

#include <gecode/search/astar.hpp>

namespace Gecode { namespace Search {

  /// Intrinsic CABS resource limits; zero means unlimited
  class CABSOptions {
  public:
    /// Maximum permitted width (passes use widths 1, 2, 4, ...)
    unsigned long int max_width;
    /// Quota of retained pending search Spaces, including the selected parent
    unsigned long int frontier_limit;
    /// Initialize width and frontier-node limits
    CABSOptions(unsigned long int maximum_width=0,
                unsigned long int frontier_nodes=0);
  };

}}

namespace Gecode {

  /**
   * \brief Sequential complete anytime beam search with widening restarts
   *
   * Each native Choice commit advances one layer. Width starts at one and
   * doubles after a completed pass if discarded bounds still permit improvement
   * or no incumbent exists. A pass can finish without widening when every
   * discarded bound excludes strict improvement of the accepted incumbent.
   * Lower rank selects the beam; without rank, the optimistic total bound
   * selects it. Equal priorities use generation order. Only admissible bounds
   * justify pruning and completion.
   *
   * Returned solutions belong to the caller and strictly improve the retained
   * incumbent. Continue next() until nullptr with stopped()==false to prove
   * optimality of the best accepted incumbent, including external adoption,
   * or infeasibility if none was accepted. Publication preserves unfinished
   * passes.
   * Ordinary stops and capacity-one worker control preserve continuation.
   * Width/frontier limits and width overflow are incomplete stops latched
   * until reset through the native Engine interface.
   *
   * The frontier quota includes current-layer entries, next-layer candidates,
   * and the selected parent. It excludes the restart root, incumbent, returned
   * solutions, temporary successor, and brancher trial Spaces. It is a node
   * quota, without a byte or process-memory guarantee. Clones store the search;
   * c_d/a_d do not enable replay and no-good extraction is empty.
   * One expanded thread and no tracer are supported. Guidance, comparison,
   * and constraints obey the same scalar integer contract as AStar.
   * If guidance, state pruning, or comparison throws, destroy the engine before further use.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class CABS : public Search::Base<T> {
  public:
    /// Initialize from space, guidance, ordinary options, and beam limits
    CABS(T* s, const Search::HeuristicGuidance& guidance,
         const Search::Options& o=Search::Options::def,
         const Search::CABSOptions& limits=Search::CABSOptions(),
         const Search::StatePruning& pruning=Search::StatePruning());
    /// Adopt a strictly improving compatible external incumbent
    void constrain(const Space& incumbent);
    /// Whether engine does best solution search
    static const bool best = true;
  };

  /// Return a CABS engine builder with copied guidance and beam limits
  template<class T>
  SEB cabs(const Search::HeuristicGuidance& guidance,
           const Search::Options& o=Search::Options::def,
           const Search::CABSOptions& limits=Search::CABSOptions(),
           const Search::StatePruning& pruning=Search::StatePruning());

}

#include <gecode/search/cabs.hpp>

namespace Gecode { namespace Search {

  /// Expansion quotas and retained-frontier limit for sequential column search
  class ColumnOptions {
  public:
    /// Positive quota of selected parents per depth in the first cycle
    unsigned long int initial_quota;
    /// Added after each full depth cycle; zero selects fixed-quota CBFS
    unsigned long int quota_growth;
    /// Retained pending Spaces including the unfinished parent; zero is unlimited
    unsigned long int frontier_limit;
    ColumnOptions(unsigned long int initial=1, unsigned long int growth=1,
                  unsigned long int frontier_nodes=0);
  };

}}

namespace Gecode {

  /**
   * \brief Sequential progressive column search with fixed-quota CBFS mode
   *
   * Each cycle expands up to the quota from successive depth queues, including
   * deeper successors generated in that cycle. Quota growth saturates safely.
   * Deferred work remains queued. Lower-first rank orders each queue when
   * supplied; otherwise the optimistic objective bound orders it. Only sound
   * bounds and optional state dominance prune work.
   *
   * Publication, ordinary stops and capacity-one worker control preserve the
   * full cycle and original Choice cursor. Returned solutions belong to the
   * caller and strictly improve the incumbent. Continue next() to nullptr with
   * stopped()==false to prove optimality of the best accepted incumbent,
   * including external adoption, or infeasibility if none was accepted.
   * A frontier-cap loss is incomplete and stays stopped until Engine::reset.
   *
   * The cap counts queued Spaces and the unfinished parent, excluding incumbent,
   * returned solutions and temporary successors/trials. It is a node limit,
   * without a byte or process-memory guarantee. Clones retain search work;
   * c_d/a_d do not enable replay and no-good extraction is empty. One expanded
   * thread and no tracer are supported. Guidance and comparison obey AStar's
   * scalar integer contract. Destroy the engine if a callback throws.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class Column : public Search::Base<T> {
  public:
    Column(T* s, const Search::HeuristicGuidance& guidance,
           const Search::Options& o=Search::Options::def,
           const Search::ColumnOptions& limits=Search::ColumnOptions(),
           const Search::StatePruning& pruning=Search::StatePruning());
    /// Adopt a strictly improving compatible external incumbent
    void constrain(const Space& incumbent);
    static const bool best = true;
  };

  /// Return a column-search builder with copied guidance, quotas and pruning
  template<class T>
  SEB column(const Search::HeuristicGuidance& guidance,
             const Search::Options& o=Search::Options::def,
             const Search::ColumnOptions& limits=Search::ColumnOptions(),
             const Search::StatePruning& pruning=Search::StatePruning());

}

#include <gecode/search/column.hpp>

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
   * An external worker control in \a o is supported only for a single
   * homogeneous asset. Multiple controlled assets require explicit engine
   * builders, each with its own control and immutable worker capacity.
   * PBS does not allocate workers or adjust those controls.
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
    /**
     * Initialize with engine builders \a sebs
     *
     * The outer options must not contain a worker control. Each builder can
     * instead supply a distinct control for its underlying engine.
     */
    PBS(T* s, SEBs& sebs, const Search::Options& o=Search::Options::def);
    /// Constrain future portfolio solutions to be better than \a b
    void constrain(const T& b);
    /// Whether engine does best solution search
    static const bool best = E<T>::best;
  };

  /**
   * \brief Run a portfolio of search engines
   *
   * The engine will run a portfolio with a number of assets as defined
   * by the options \a o. The engine supports parallel execution of
   * assets by using the number of threads as defined by the options.
   * An external worker control is supported only when \a o selects one
   * asset. PBS does not implement worker-allocation policy.
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
