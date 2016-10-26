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

  /**
   * \brief Which events to trace
   * \ingroup TaskTrace
   */
  enum TraceEvent {
    TE_INIT  = 1 << 0, ///< Trace init events
    TE_PRUNE = 1 << 1, ///< Trace prune events
    TE_FIX   = 1 << 2, ///< Trace fixpoint events
    TE_DONE  = 1 << 3  ///< Trace done events
  };

  /**
   * \brief Propagator for recording trace information
   * \ingroup TaskTrace
   */
  template<class View>
  class TraceRecorder : public Propagator {
  public:
    /// The corresponding duplicate view type
    typedef typename TraceTraits<View>::TraceView TraceView;
    /// The corresponding trace delta type
    typedef typename TraceTraits<View>::TraceDelta TraceDelta;
    /// The corresponding slack value type
    typedef typename TraceTraits<View>::SlackValue SlackValue;
    /// Collection of slack values
    class Slack {
      template<class ViewForTraceRecorder> friend class TraceRecorder;
    protected:
      /// The initial slack value
      SlackValue i;
      /// Slack value at previous event (fixpoint or init)
      SlackValue p;
      /// Current slack value
      SlackValue c;
    public:
      /// Return initial slack value
      SlackValue initial(void) const;
      /// Return previous slack value
      SlackValue previous(void) const;
      /// Return current slack value
      SlackValue current(void) const;
    };
  protected:
    /// Advisor with index information
    class Idx : public Advisor {
    protected:
      /// Index information
      int _idx;
    public:
      /// Constructor for creation
      Idx(Space& home, Propagator& p, Council<Idx>& c, int i);
      /// Constructor for cloning \a a
      Idx(Space& home, bool share, Idx& a);
      /// Get index of view
      int idx(void) const;
    };
    /// Duplicate views (old information)
    ViewArray<TraceView> o;
    /// Original views (new information)
    ViewArray<View> n;
    /// The advisor council
    Council<Idx> c;
    /// The trace filter
    TraceFilter tf;
    /// Which events to trace
    int te;
    /// The actual tracer
    Tracer<View>& t;
    /// Slack information
    Slack s;
    /// Constructor for cloning \a p
    TraceRecorder(Space& home, bool share, TraceRecorder& p);
  public:
    /// Constructor for creation
    TraceRecorder(Home home, ViewArray<View>& x,
                  TraceFilter tf, int te, Tracer<View>& t);
    /// Copy propagator during cloning
    virtual Propagator* copy(Space& home, bool share);
    /// Cost function (record so that propagator runs last)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
    /// Post activity recorder propagator
    static ExecStatus post(Home home, ViewArray<View>& x,
                           TraceFilter tf, int te, Tracer<View>& t);
    /// \name Trace information
    //@{
    /// Return variable being traced at position \a i
    const typename View::VarType operator [](int i) const;
    /// Return number of variables being traced
    int size(void) const;
    /// Provide access to slack information
    const Slack& slack(void) const;
    //@}
  };

  /*
   * Functions for trace support
   *
   */
  template<class View>
  forceinline const typename View::VarType
  TraceRecorder<View>::operator [](int i) const {
    const typename View::VarType x(n[i].varimp());
    return x;
  }
  template<class View>
  forceinline int
  TraceRecorder<View>::size(void) const {
    return n.size();
  }
  template<class View>
  forceinline const typename TraceRecorder<View>::Slack&
  TraceRecorder<View>::slack(void) const {
    return s;
  }


  /*
   * Functions for access to slack
   *
   */
  template<class View>
  forceinline typename TraceRecorder<View>::SlackValue
  TraceRecorder<View>::Slack::initial(void) const {
    return i;
  }
  template<class View>
  forceinline typename TraceRecorder<View>::SlackValue
  TraceRecorder<View>::Slack::previous(void) const {
    return p;
  }
  template<class View>
  forceinline typename TraceRecorder<View>::SlackValue
  TraceRecorder<View>::Slack::current(void) const {
    return c;
  }


  /*
   * Advisor for tracer
   *
   */

  template<class View>
  forceinline
  TraceRecorder<View>::Idx::Idx(Space& home, Propagator& p,
                                 Council<Idx>& c, int i)
    : Advisor(home,p,c), _idx(i) {}
  template<class View>
  forceinline
  TraceRecorder<View>::Idx::Idx(Space& home, bool share, Idx& a)
    : Advisor(home,share,a), _idx(a._idx) {
  }
  template<class View>
  forceinline int
  TraceRecorder<View>::Idx::idx(void) const {
    return _idx;
  }


  /*
   * Posting of tracer propagator
   *
   */
  template<class View>
  forceinline
  TraceRecorder<View>::TraceRecorder(Home home, ViewArray<View>& x,
                                     TraceFilter tf0, int te0,
                                     Tracer<View>& t0)
    : Propagator(home), o(home,x.size()), n(x), c(home),
      tf(tf0), te(te0), t(t0) {
    home.notice(*this, AP_DISPOSE);
    for (int i=n.size(); i--; ) {
      o[i] = TraceView(home,n[i]);
      if (!n[i].assigned())
        n[i].subscribe(home,*new (home) Idx(home,*this,c,i));
    }
    View::schedule(home,*this,ME_GEN_ASSIGNED);
    s.i = TraceView::slack(n[n.size()-1]);
    for (int i=n.size()-1; i--; )
      s.i += TraceView::slack(n[i]);
    s.p = s.i;
    if ((te & TE_INIT) != 0)
      t._init(home,*this);
  }


  template<class View>
  forceinline ExecStatus
  TraceRecorder<View>::post(Home home, ViewArray<View>& x,
                            TraceFilter tf, int te, Tracer<View>& t) {
    if (x.size() > 0)
      (void) new (home) TraceRecorder(home,x,tf,te,t);
    return ES_OK;
  }


  /*
   * Propagation for trace collector
   *
   */
  template<class View>
  forceinline
  TraceRecorder<View>::TraceRecorder(Space& home, bool share,
                                     TraceRecorder& p)
    : Propagator(home,share,p), te(p.te), t(p.t), s(p.s) {
    o.update(home, share, p.o);
    n.update(home, share, p.n);
    c.update(home, share, p.c);
    tf.update(home, share, p.tf);
  }

  template<class View>
  Propagator*
  TraceRecorder<View>::copy(Space& home, bool share) {
    return new (home) TraceRecorder(home, share, *this);
  }

  template<class View>
  inline size_t
  TraceRecorder<View>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    tf.~TraceFilter();
    // Cancel remaining advisors
    for (Advisors<Idx> as(c); as(); ++as)
      n[as.advisor().idx()].cancel(home,as.advisor());
    c.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  PropCost
  TraceRecorder<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::record();
  }

  template<class View>
  void
  TraceRecorder<View>::reschedule(Space& home) {
    View::schedule(home,*this,ME_GEN_ASSIGNED);
  }

  template<class View>
  ExecStatus
  TraceRecorder<View>::advise(Space& home, Advisor& _a, const Delta& d) {
    Idx& a = static_cast<Idx&>(_a);
    int i = a.idx();
    if (((te & TE_PRUNE) != 0) && !disabled() && tf(a(home)) ) {
      TraceDelta td(o[i],n[i],d);
      t._prune(home,*this,a(home),i,td);
    }
    o[i].prune(home,n[i],d);
    if (n[a.idx()].assigned())
      a.dispose(home,c);
    return ES_NOFIX;
  }

  template<class View>
  ExecStatus
  TraceRecorder<View>::propagate(Space& home, const ModEventDelta&) {
    s.c = TraceView::slack(n[n.size()-1]);
    for (int i=n.size()-1; i--; )
      s.c += TraceView::slack(n[i]);
    if ((te & TE_FIX) != 0)
      t._fix(home,*this);
    s.p = s.c;
    if (c.empty()) {
      if ((te & TE_DONE) != 0)
        t._done(home,*this);
      return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

}

// STATISTICS: kernel-other
