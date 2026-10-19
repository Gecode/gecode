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
#include <memory>
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
    /// Control shared for the lifetime of a leaf engine
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

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search {

  /**
   * \brief Read-only guidance for scalar integer heuristic search
   *
   * The bound is an optimistic total objective value on a stable Space:
   * a lower bound for minimization, an upper bound for maximization, and
   * the exact objective on a solution. It must agree with the strict
   * improvement restriction in Space::constrain. The engine adds no prefix cost.
   * Values are signed 64-bit integers compared directly; callback arithmetic
   * must stay in range. Engines define how they use the optional ranking
   * (lower first); rank can be arbitrary and never justifies pruning.
   * Guidance is copied into engines and portfolio builders. Callbacks read
   * only the supplied Space and must not post constraints or capture another
   * Space's local state. Captured dependencies must outlive every engine and
   * builder using them. With multiple threads, callbacks can run concurrently;
   * captured shared state and model copy/prepare operations must be thread-safe.
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

  /// Fixed exploration order for A* search
  enum AStarOrder {
    BOUND_FIRST, ///< Optimistic bound, lower rank, then insertion order
    RANK_FIRST   ///< Lower rank, optimistic bound, then insertion order
  };

}}

namespace Gecode {

  /**
   * \brief A* with improving solutions and a fixed exploration order
   *
   * Experimental 6.5.0 preview: build with GECODE_ENABLE_EXPERIMENTAL=ON
   * or --enable-experimental; GECODE_HAS_EXPERIMENTAL reports availability.
   * APIs may change or be removed in future feature releases. The documented
   * correctness contracts still apply.
   *
   * Spaces must implement strict improvement constraints compatible with the
   * guidance's exact scalar integer objective on solutions. A generated solution
   * can precede a better one, so the first result need not be optimal.
   * Continue next() until nullptr with stopped()==false
   * to prove optimality of the best accepted incumbent, including external
   * adoption, or infeasibility if no incumbent was accepted. After external
   * adoption, no new solution does not imply infeasibility. Completeness
   * requires a finite search tree and sound bounds and improvement constraints.
   * Returned solutions belong to the caller, independently of the engine's
   * incumbent copy.
   *
   * The default BOUND_FIRST order compares optimistic bound, lower rank,
   * then insertion order. RANK_FIRST compares lower rank, optimistic bound,
   * then insertion order. Bounds follow the objective direction; ranks are
   * lower-first for either direction. Missing rank falls back to bound order.
   * Both values are cached when a stable branching node enters the frontier.
   * Ordering is fixed for the engine, including native reset; values are not
   * rekeyed. Rank selects pending branching Spaces and never prunes. Generated
   * solutions are published immediately, so neither order guarantees the first
   * result is optimal or has the best rank.
   *
   * The frontier stores clones; c_d and a_d do not control recomputation.
   * There is no hard memory guarantee and no-good extraction is empty.
   * Use BAB as an optimization baseline. Retaining frontier clones can cost
   * much more memory and cloning work than depth-first search. Start with one
   * thread and measure a few thread counts on representative instances.
   * Expensive propagation can benefit from parallel expansion, but frontier
   * coordination, memory traffic, and changes in search order can outweigh it.
   * More threads do not necessarily run faster. Neither AStar nor CABS
   * guarantees a speedup over BAB.
   *
   * Options::threads selects sequential or parallel search. Parallel workers
   * select the best currently queued node; concurrent expansions can change
   * generation and solution order. Exhaustion waits for all expansions.
   * Worker control can pause, grow, or shrink the active worker count while
   * preserving pending work. Tracing is unsupported. After an ordinary stop,
   * raise or reset its limit and call next() to resume. Parallel workers check
   * stops after at most 32 consecutive failed or incumbent-pruned alternatives;
   * worker-count requests are checked after each alternative. Statistics depth
   * counts committed edges. Parallel next() waits for active model operations before
   * returning; failures in workers are rethrown on the caller.
   * Before generating a choice, Brancher::prepare finishes any lazy model
   * preparation on the stable Space.
   * The native Search::Engine::reset interface takes ownership of its root
   * regardless of Options::clone, clearing pending work, incumbent, and
   * statistics while retaining guidance and ordering.
   * If guidance or a model operation throws, destroy the engine
   * before further use.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class AStar : public Search::Base<T> {
  public:
    /**
     * \brief Initialize from space, guidance, and ordinary search options
     *
     * Options::clone=true (the default) leaves the supplied root with the
     * caller. With clone=false, successful construction transfers ownership
     * to the engine. Constructor failure leaves it with the caller.
     */
    AStar(T* s, const Search::HeuristicGuidance& guidance,
          const Search::Options& o=Search::Options::def);
    /// Initialize with an explicit fixed order; ownership follows the constructor above
    AStar(T* s, const Search::HeuristicGuidance& guidance,
          Search::AStarOrder ordering,
          const Search::Options& o=Search::Options::def);
    /// Copy a compatible external solution if its exact bound strictly improves
    void constrain(const Space& incumbent);
    /// Whether engine does best solution search
    static const bool best = true;
  };

  /**
   * \brief Search to completion or a stop and return the best solution found
   *
   * The caller owns the result; nullptr means no solution was found. A stop
   * can leave the result unproven. The helper cannot report whether search
   * stopped; use AStar to inspect completion or resume search.
   * Root ownership follows AStar and Options::clone.
   * \ingroup TaskModelSearch
   */
  template<class T>
  T* astar(T* s, const Search::HeuristicGuidance& guidance,
           const Search::Options& o=Search::Options::def);

  /// Return an A* builder for PBS portfolios, with copied guidance and options
  template<class T>
  SEB astar(const Search::HeuristicGuidance& guidance,
            const Search::Options& o=Search::Options::def);

  /// Search with a fixed order; result, stop, and ownership contracts follow astar above
  template<class T>
  T* astar(T* s, const Search::HeuristicGuidance& guidance,
           Search::AStarOrder ordering,
           const Search::Options& o=Search::Options::def);

  /// Return an A* builder for PBS portfolios, with copied guidance, order, and options
  template<class T>
  SEB astar(const Search::HeuristicGuidance& guidance,
            Search::AStarOrder ordering,
            const Search::Options& o=Search::Options::def);

}

#include <gecode/search/astar.hpp>

namespace Gecode { namespace Search {

  /// CABS resource limits and optional candidate dominance
  class CABSOptions {
  public:
    /// Maximum permitted width (passes use widths 1, 2, 4, ...); zero is unlimited
    unsigned long int max_width;
    /// Pending-Space quota, including the selected parent; zero is unlimited
    unsigned long int frontier_limit;
    /**
     * \brief Whether the first space subsumes the second; empty disables pruning
     *
     * Inputs are stable, read-only next-layer candidates. Returning true must
     * preserve every improving completion of the second space at no worse total
     * objective, including prefix cost, residual feasibility, and branching
     * progress. A completion using no more transitions is sufficient. The model
     * must establish that repeated replacement is safe, with a consistent,
     * transitive no-worse relation; equality may use the same predicate.
     * CABS does not infer these properties from bounds or ranks.
     *
     * Only live candidates in the same layer and pass justify pruning. A new
     * dominator inherits a removed candidate's beam slot, regardless of rank.
     * The engine copies the callable; captured references must outlive it.
     * The predicate must not mutate either space. Parallel admission serializes
     * calls to this predicate, but guidance and key callbacks may run concurrently
     * with it. Shared captured state must support that use. If it throws, destroy
     * the engine before further use.
     */
    std::function<bool(const Space&,const Space&)> dominates;
    /**
     * \brief Optional key grouping candidates for dominance comparisons
     *
     * Used only when dominates is set. Different keys skip comparison; equal
     * keys still require the dominance predicate, so collisions are safe.
     * Potential dominators must have equal keys. The engine caches the key
     * when a stable branching candidate is generated. An empty callable puts
     * all candidates in one group.
     *
     * The callable reads only its supplied Space and is copied with the options.
     * Captured dependencies must outlive the engine. Parallel workers can call
     * it concurrently with guidance and dominance; shared captured state must
     * support that use. If it throws, destroy the engine before further use.
     */
    std::function<std::size_t(const Space&)> dominance_key;
    /// Initialize limits, with dominance disabled
    CABSOptions(unsigned long int maximum_width=0,
                unsigned long int frontier_nodes=0);
  };

}}

namespace Gecode {

  /**
   * \brief Complete anytime beam search with widening restarts
   *
   * Experimental 6.5.0 preview: build with GECODE_ENABLE_EXPERIMENTAL=ON
   * or --enable-experimental; GECODE_HAS_EXPERIMENTAL reports availability.
   * APIs may change or be removed in future feature releases. The documented
   * correctness contracts still apply.
   *
   * Each native Choice commit advances one layer. Width starts at one and
   * doubles after a completed pass if discarded bounds still permit improvement
   * or no incumbent exists. A pass can finish without widening when every
   * discarded bound excludes strict improvement of the accepted incumbent.
   * Finite search trees are required for widening completeness.
   * Lower rank selects the beam; without rank, the optimistic total bound
   * selects it. Equal priorities use generation order. Admissible bounds and
   * the optional CABSOptions::dominates predicate justify pruning and completion.
   *
   * Returned solutions belong to the caller, independently of the engine's
   * incumbent copy, and strictly improve it. Continue next() until nullptr with
   * stopped()==false to prove
   * optimality of the best accepted incumbent, including external adoption,
   * or infeasibility if none was accepted. A shallow goal can precede a deeper
   * improvement; the first result need not be optimal. No new solution after
   * external adoption does not imply infeasibility. Publishing a solution
   * preserves unfinished passes.
   * After an ordinary stop, raise or reset its limit and call next() to resume.
   * Worker control also preserves continuation, including a zero-worker pause.
   * Width/frontier
   * limits and width overflow report stopped() until native Engine::reset.
   * A complete pass can prove its result at the maximum width; a maximum of
   * three allows passes at widths one and two. Native reset takes ownership
   * of its root regardless of Options::clone and clears the incumbent, pending
   * work, intrinsic stops, and statistics while retaining guidance and CABS options.
   *
   * The frontier quota includes current-layer entries, next-layer candidates,
   * and selected parents. In parallel search, each unfinished expansion reserves
   * a parent and an unsubmitted successor at admission; there is at most one
   * temporary successor per worker. The quota excludes the restart root,
   * incumbent copies, and returned solutions. It is checked when adding a beam
   * slot, without a byte or process-memory guarantee. Clones store the search;
   * c_d/a_d do not enable replay and no-good extraction is empty.
   * The threading tradeoffs described for AStar also apply here.
   * Options::threads selects sequential or parallel expansion of layer parents.
   * All workers share one next-layer beam and dominance admission. A layer or
   * pass advances only after all current work has been submitted. Parallel
   * generation order and returned improvements can differ from sequential search.
   * Stop and worker-count checks follow AStar. next() waits for active model
   * operations before returning and rethrows worker failures on the caller.
   * Tracing is unsupported. Guidance and strict
   * improvement constraints obey HeuristicGuidance's scalar integer contract.
   * Brancher preparation follows AStar. Statistics depth counts committed
   * edges; restarts count widening passes.
   * If guidance, dominance, a key callback, or a model operation throws,
   * destroy the engine before further use.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class CABS : public Search::Base<T> {
  public:
    /**
     * \brief Initialize from space, guidance, ordinary options, and CABS options
     *
     * Options::clone=true (the default) leaves the supplied root with the
     * caller. With clone=false, successful construction transfers ownership
     * to the engine. Constructor failure leaves it with the caller.
     */
    CABS(T* s, const Search::HeuristicGuidance& guidance,
         const Search::Options& o=Search::Options::def,
         const Search::CABSOptions& limits=Search::CABSOptions());
    /// Copy a compatible external solution if its exact bound strictly improves
    void constrain(const Space& incumbent);
    /// Whether engine does best solution search
    static const bool best = true;
  };

  /**
   * \brief Search to completion or a stop and return the best solution found
   *
   * The caller owns the result; nullptr means no solution was found. Ordinary
   * stops or beam limits can leave the result unproven. The helper cannot
   * report whether search stopped; use CABS to inspect completion or resume
   * search. Root ownership follows CABS and Options::clone.
   * \ingroup TaskModelSearch
   */
  template<class T>
  T* cabs(T* s, const Search::HeuristicGuidance& guidance,
          const Search::Options& o=Search::Options::def,
          const Search::CABSOptions& limits=Search::CABSOptions());

  /// Return a CABS builder for PBS portfolios, with copied guidance and CABS options
  template<class T>
  SEB cabs(const Search::HeuristicGuidance& guidance,
           const Search::Options& o=Search::Options::def,
           const Search::CABSOptions& limits=Search::CABSOptions());

}

#include <gecode/search/cabs.hpp>

#endif

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
     * In parallel PBS, the outer thread count limits how many builders run,
     * in array order; each builder's options set its own engine's thread count.
     *
     * With experimental search enabled, BAB, AStar, and CABS builders can share
     * improving solutions in one portfolio. The model must install branching
     * in slave() or override master() to retain it: Space::master removes
     * branchers for portfolios by default. For compatible model and guidance:
     * \code
     * Search::Options asset;
     * asset.threads = 1;
     * SEBs assets{bab<Model>(asset), astar<Model>(guidance,asset),
     *             cabs<Model>(guidance,asset)};
     * Search::Options portfolio;
     * portfolio.threads = 3;
     * PBS<Model,BAB> engine(model,assets,portfolio);
     * \endcode
     * These settings admit all three assets with one thread each. Asset
     * selection and any policy for changing worker counts belong to the caller.
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
