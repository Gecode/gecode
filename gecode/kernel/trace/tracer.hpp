/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

namespace Gecode {

  /// Class to provide synchronization
  class TracerBase : public HeapAllocated {
  protected:
    /// Mutex to provide synchronization
    GECODE_KERNEL_EXPORT
    static Support::Mutex m;
  };

  template<class View> class ViewTraceRecorder;

  /**
   * \brief Tracer that process view trace information
   * \ingroup TaskTrace
   */
  template<class View>
  class ViewTracer : public TracerBase {
    template<class ViewForTraceRecorder> friend class ViewTraceRecorder;
  private:
    /**
     * \brief Init function synchronization
     *
     * Just calls the actual init function protected by a mutex.
     *
     */
    void _init(const Space& home, const ViewTraceRecorder<View>& t);
    /**
     * \brief Prune function synchronization
     *
     * Just calls the actual prune function protected by a mutex.
     *
     */
    void _prune(const Space& home, const ViewTraceRecorder<View>& t,
                const ViewTraceInfo& vti,
                int i, typename TraceTraits<View>::TraceDelta& d);
    /**
     * \brief Fail function synchronization
     *
     * Just calls the actual fail function protected by a mutex.
     *
     */
    void _fail(const Space& home, const ViewTraceRecorder<View>& t);
    /**
     * \brief Fixpoint function synchronization
     *
     * Just calls the actual fixpoint function protected by a mutex.
     */
    void _fix(const Space& home, const ViewTraceRecorder<View>& t);
    /**
     * \brief Done function synchronization
     *
     * Just calls the actual done function protected by a mutex.
     */
    void _done(const Space& home, const ViewTraceRecorder<View>& t);
  public:
    /// Constructor
    ViewTracer(void);
    /**
     * \brief Init function
     *
     * The init function is called when the trace collector has
     * been initialized.
     */
    virtual void init(const Space& home,
                      const ViewTraceRecorder<View>& t) = 0;
    /**
     * \brief Prune function
     *
     * The variable at position \a i has been modified where
     * the modification is described by \a d. Additional
     * information about how the variable has been changed is
     * provided by the trace collector \a t and execution
     * information \a ei.
     */
    virtual void prune(const Space& home,
                       const ViewTraceRecorder<View>& t,
                       const ViewTraceInfo& vti,
                       int i, typename TraceTraits<View>::TraceDelta& d) = 0;
    /**
     * \brief Fail function
     *
     * The fail function is called whenever \a home containing the
     * trace collector \a t has been failed.
     */
    virtual void fail(const Space& home,
                      const ViewTraceRecorder<View>& t) = 0;
    /**
     * \brief Fixpoint function
     *
     * The fixpoint function is called whenever \a home containing
     * the trace collector \a t reaches a fixpoint (and fixpoint
     * tracing is enabled).
     */
    virtual void fix(const Space& home,
                     const ViewTraceRecorder<View>& t) = 0;
    /**
     * \brief Done function
     *
     * The done function is called whenever the trace collector \a t
     * is done and will terminate.
     */
    virtual void done(const Space& home,
                      const ViewTraceRecorder<View>& t) = 0;
    /// Destructor
    virtual ~ViewTracer(void);
  };




  /**
   * \brief Tracer
   * \ingroup TaskTrace
   */
  class Tracer : public TracerBase {
    friend class Space;
    friend class PostInfo;
  private:
    /**
     * \brief Propagate function synchronization
     *
     * Just calls the actual propagate function protected by a mutex.
     *
     */
    void _propagate(const Space& home, const PropagateTraceInfo& pti);
    /**
     * \brief Commit function synchronization
     *
     * Just calls the actual commit function protected by a mutex.
     *
     */
    void _commit(const Space& home, const CommitTraceInfo& cti);
    /**
     * \brief Post function synchronization
     *
     * Just calls the actual post function protected by a mutex.
     *
     */
    void _post(const Space& home, const PostTraceInfo& pti);
  public:
    /// Constructor
    Tracer(void);
    /**
     * \brief Propagate function
     *
     * The propagate function is called when a propagator has been
     * executed.
     */
    virtual void propagate(const Space& home,
                           const PropagateTraceInfo& pti) = 0;
    /**
     * \brief Commit function
     *
     * The commit function is called when a brancher has executed
     * a commit operation.
     */
    virtual void commit(const Space& home,
                        const CommitTraceInfo& cti) = 0;
    /**
     * \brief Post function
     *
     * The post function is called when an attempt to post a propagator
     * has been executed.
     */
    virtual void post(const Space& home,
                      const PostTraceInfo& pti) = 0;
    /// Destructor
    virtual ~Tracer(void);
  };


  /**
   * \brief Default tracer
   * \ingroup TaskTrace
   */
  class  GECODE_KERNEL_EXPORT StdTracer : public Tracer {
  protected:
    /// Output stream to use
    std::ostream& os;
  public:
    /// Initialize with output stream \a os
    StdTracer(std::ostream& os = std::cerr);
    /**
     * \brief Propagate function
     *
     * The propagate function is called when a propagator has been
     * executed.
     */
    virtual void propagate(const Space& home,
                           const PropagateTraceInfo& pti);
    /**
     * \brief Commit function
     *
     * The commit function is called when a brancher has executed
     * a commit operation.
     */
    virtual void commit(const Space& home,
                        const CommitTraceInfo& cti);
    /**
     * \brief Post function
     *
     * The post function is called when an attempt to post a propagator
     * has been executed.
     */
    virtual void post(const Space& home,
                      const PostTraceInfo& pti);
    /// Default tracer (printing to std::cerr)
    static StdTracer def;
  };


  /*
   * View tracer
   */

  template<class View>
  forceinline
  ViewTracer<View>::ViewTracer(void) {
  }

  template<class View>
  forceinline void
  ViewTracer<View>::_init(const Space& home,
                          const ViewTraceRecorder<View>& t) {
    Support::Lock l(m);
    init(home,t);
  }
  template<class View>
  forceinline void
  ViewTracer<View>::_prune(const Space& home,
                           const ViewTraceRecorder<View>& t,
                           const ViewTraceInfo& vti,
                           int i, typename TraceTraits<View>::TraceDelta& d) {
    Support::Lock l(m);
    prune(home,t,vti,i,d);
  }
  template<class View>
  forceinline void
  ViewTracer<View>::_fail(const Space& home,
                          const ViewTraceRecorder<View>& t) {
    Support::Lock l(m);
    fail(home,t);
  }
  template<class View>
  forceinline void
  ViewTracer<View>::_fix(const Space& home,
                         const ViewTraceRecorder<View>& t) {
    Support::Lock l(m);
    fix(home,t);
  }
  template<class View>
  forceinline void
  ViewTracer<View>::_done(const Space& home,
                          const ViewTraceRecorder<View>& t) {
    Support::Lock l(m);
    done(home,t);
  }

  template<class View>
  forceinline
  ViewTracer<View>::~ViewTracer(void) {
  }


  /*
   * Tracer
   */

  forceinline
  Tracer::Tracer(void) {
  }

  forceinline void
  Tracer::_propagate(const Space& home,
                     const PropagateTraceInfo& pti) {
    Support::Lock l(m);
    propagate(home,pti);
  }
  forceinline void
  Tracer::_commit(const Space& home,
                  const CommitTraceInfo& cti) {
    Support::Lock l(m);
    commit(home,cti);
  }
  forceinline void
  Tracer::_post(const Space& home,
                const PostTraceInfo& pti) {
    Support::Lock l(m);
    post(home,pti);
  }

  forceinline
  Tracer::~Tracer(void) {
  }

}

// STATISTICS: kernel-trace
