/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

namespace Gecode {

  /// Class to provide synchronization
  class TracerBase : public HeapAllocated {
  public:
    /// Which events to trace
    enum TraceEvent {
      INIT     = 1 << 0, ///< Trace init events
      PRUNE    = 1 << 1, ///< Trace prune events
      FIXPOINT = 1 << 2, ///< Trace fixpoint events
      DONE     = 1 << 3  ///< Trace done events
    };
  protected:
    /// Mutex to provide synchronization
    GECODE_KERNEL_EXPORT
    static Support::Mutex m;
    /// Flags that determine which events to trace
    const int events;
    /// Initialize with flags \a f
    TracerBase(int f);
  };

  template<class View> class TraceRecorder;

  /**
   * \brief Tracer that process trace information
   * \ingroup TaskTrace
   */
  template<class View>
  class Tracer : public TracerBase {
    template<class View> friend class TraceRecorder;
  private:
    /**
     * \brief Init function synchronization
     *
     * Just calls the actual init function protected by a mutex.
     *
     */
    void _init(const Space& home, const TraceRecorder<View>& t);
    /**
     * \brief Prune function synchronization
     *
     * Just calls the actual prune function protected by a mutex.
     *
     */
    void _prune(const Space& home, const TraceRecorder<View>& t,
                const ExecInfo& ei,
                int i, typename TraceTraits<View>::TraceDelta& d);
    /**
     * \brief Fixpoint function synchronization
     *
     * Just calls the actual fixpoint function protected by a mutex.
     */
    void _fixpoint(const Space& home, const TraceRecorder<View>& t);
    /**
     * \brief Done function synchronization
     *
     * Just calls the actual done function protected by a mutex.
     */
    void _done(const Space& home, const TraceRecorder<View>& t);
  public:
    /// Constructor where is a disjunction of trace events
    Tracer(int e = INIT | PRUNE | FIXPOINT | DONE);
    /**
     * \brief Init function
     *
     * The init function is called when the trace collector has
     * been initialized.
     */
    virtual void init(const Space& home,
                      const TraceRecorder<View>& t) = 0;
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
                       const TraceRecorder<View>& t,
                       const ExecInfo& ei,
                       int i, typename TraceTraits<View>::TraceDelta& d) = 0;
    /**
     * \brief Fixpoint function
     *
     * The fixpoint function is called whenever \a home containing
     * the trace collector \a t reaches a fixpoint (and fixpoint
     * tracing is enabled).
     */
    virtual void fixpoint(const Space& home,
                          const TraceRecorder<View>& t) = 0;
    /**
     * \brief Done function
     *
     * The done function is called whenever the trace collector \a t
     * is done and will terminate.
     */
    virtual void done(const Space& home,
                      const TraceRecorder<View>& t) = 0;
    /// Destructor
    virtual ~Tracer(void);
  };


  forceinline
  TracerBase::TracerBase(int f) : events(f) {}

  template<class View>
  forceinline
  Tracer<View>::Tracer(int e) : TracerBase(e) {}

  template<class View>
  forceinline void
  Tracer<View>::_init(const Space& home, const TraceRecorder<View>& t) {
    if (events & INIT) {
      m.acquire();
      init(home,t);
      m.release();
    }
  }
  template<class View>
  forceinline void
  Tracer<View>::_prune(const Space& home, const TraceRecorder<View>& t,
                       const ExecInfo& ei,
                       int i, typename TraceTraits<View>::TraceDelta& d) {
    if (events & PRUNE) {
      m.acquire();
      prune(home,t,ei,i,d);
      m.release();
    }
  }
  template<class View>
  forceinline void
  Tracer<View>::_fixpoint(const Space& home, const TraceRecorder<View>& t) {
    if (events & FIXPOINT) {
      m.acquire();
      fixpoint(home,t);
      m.release();
    }
  }
  template<class View>
  forceinline void
  Tracer<View>::_done(const Space& home, const TraceRecorder<View>& t) {
    if (events & DONE) {
      m.acquire();
      done(home,t);
      m.release();
    }
  }
  template<class View>
  forceinline
  Tracer<View>::~Tracer(void) {}

}

// STATISTICS: kernel-other
