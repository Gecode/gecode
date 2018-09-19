/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

#include <cfloat>

namespace Gecode {

  /**
   * \brief Class for CHB management
   *
   * The idea is taken from: Exponential Recency Weighted Average
   * Branching Heuristic for SAT Solvers, Jia Hui Liang, Vijay Ganesh,
   * Pascal Poupart, Krzysztof Czarnecki, AAAI 2016, pages 3434-3440.
   *
   */
  class CHB : public SharedHandle {
  protected:
    template<class View>
    class Recorder;
    /// View information
    class Info {
    public:
      /// Last failure
      unsigned long long int lf;
      /// Q-score
      double qs;
    };
    /// Object for storing chb information
    class GECODE_VTABLE_EXPORT Storage : public SharedHandle::Object {
    public:
      /// Mutex to synchronize globally shared access
      GECODE_KERNEL_EXPORT static Support::Mutex m;
      /// Number of chb values
      int n;
      /// Number of failures
      unsigned long int nf;
      /// Alpha value
      double alpha;
      /// CHB information
      Info* chb;
      /// Initialize CHB info
      template<class View>
      Storage(Home home, ViewArray<View>& x,
              typename BranchTraits<typename View::VarType>::Merit bm);
      /// Delete object
      GECODE_KERNEL_EXPORT
      ~Storage(void);
      /// Bump failure count and alpha
      void bump(void);
      /// Update chb information at position \a i
      void update(int i, bool failed);
    };
    /// Return object of correct type
    Storage& object(void) const;
    /// Set object to \a o
    void object(Storage& o);
    /// Update chb value at position \a i
    void update(int i);
    /// Acquire mutex
    void acquire(void);
    /// Release mutex
    void release(void);
    /// Bump failure count and alpha
    void bump(void);
    /// Update chb information at position \a i
    void update(int i, bool failed);
  public:
    /// \name Constructors and initialization
    //@{
    /**
     * \brief Construct as not yet intialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized chb storage is init and the assignment operator.
     *
     */
    CHB(void);
    /// Copy constructor
    GECODE_KERNEL_EXPORT
    CHB(const CHB& a);
    /// Assignment operator
    GECODE_KERNEL_EXPORT
    CHB& operator =(const CHB& a);
    /// Initialize for views \a x and Q-score as defined by \a bm
    template<class View>
    CHB(Home home, ViewArray<View>& x,
        typename BranchTraits<typename View::VarType>::Merit bm);
    /// Initialize for views \a x and Q-score as defined by \a bm
    template<class View>
    void init(Home home, ViewArray<View>& x,
              typename BranchTraits<typename View::VarType>::Merit bm);
    /// Default (empty) chb information
    GECODE_KERNEL_EXPORT static const CHB def;
    //@}

    /// Destructor
    GECODE_KERNEL_EXPORT
    ~CHB(void);

    /// \name Information access
    //@{
    /// Return chb value at position \a i
    double operator [](int i) const;
    /// Return number of chb values
    int size(void) const;
    //@}
  };

  /// Propagator for recording chb information
  template<class View>
  class CHB::Recorder : public NaryPropagator<View,PC_GEN_NONE> {
  protected:
    using NaryPropagator<View,PC_GEN_NONE>::x;
    /// Advisor with index and change information
    class Idx : public Advisor {
    protected:
      /// Index and mark information
      int _info;
    public:
      /// Constructor for creation
      Idx(Space& home, Propagator& p, Council<Idx>& c, int i);
      /// Constructor for cloning \a a
      Idx(Space& home, Idx& a);
      /// Mark advisor as modified
      void mark(void);
      /// Mark advisor as unmodified
      void unmark(void);
      /// Whether advisor's view has been marked
      bool marked(void) const;
      /// Get index of view
      int idx(void) const;
    };
    /// Access to chb information
    CHB chb;
    /// The advisor council
    Council<Idx> c;
    /// Constructor for cloning \a p
    Recorder(Space& home, Recorder<View>& p);
  public:
    /// Constructor for creation
    Recorder(Home home, ViewArray<View>& x, CHB& chb);
    /// Copy propagator during cloning
    virtual Propagator* copy(Space& home);
    /// Cost function (record so that propagator runs last)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Give advice to propagator when \a home has failed
    virtual void advise(Space& home, Advisor& a);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
    /// Post chb recorder propagator
    static ExecStatus post(Home home, ViewArray<View>& x, CHB& chb);
  };

  /**
   * \brief Print chb values enclosed in curly brackets
   * \relates CHB
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const CHB& a);


  /*
   * Advisor for chb recorder
   *
   */
  template<class View>
  forceinline
  CHB::Recorder<View>::Idx::Idx(Space& home, Propagator& p,
                                Council<Idx>& c, int i)
    : Advisor(home,p,c), _info(i << 1) {}
  template<class View>
  forceinline
  CHB::Recorder<View>::Idx::Idx(Space& home, Idx& a)
    : Advisor(home,a), _info(a._info) {
  }
  template<class View>
  forceinline void
  CHB::Recorder<View>::Idx::mark(void) {
    _info |= 1;
  }
  template<class View>
  forceinline bool
  CHB::Recorder<View>::Idx::marked(void) const {
    return (_info & 1) != 0;
  }
  template<class View>
  forceinline void
  CHB::Recorder<View>::Idx::unmark(void) {
    assert(marked());
    _info -= 1;
  }
  template<class View>
  forceinline int
  CHB::Recorder<View>::Idx::idx(void) const {
    return _info >> 1;
  }


  /*
   * Posting of chb recorder propagator
   *
   */
  template<class View>
  forceinline
  CHB::Recorder<View>::Recorder(Home home, ViewArray<View>& x,
                                CHB& chb0)
    : NaryPropagator<View,PC_GEN_NONE>(home,x), chb(chb0), c(home) {
    home.notice(*this,AP_DISPOSE);
    for (int i=0; i<x.size(); i++)
      if (!x[i].assigned())
        x[i].subscribe(home,*new (home) Idx(home,*this,c,i), true);
  }

  template<class View>
  forceinline ExecStatus
  CHB::Recorder<View>::post(Home home, ViewArray<View>& x, CHB& chb) {
    (void) new (home) Recorder<View>(home,x,chb);
    return ES_OK;
  }


  /*
   * CHB value storage
   *
   */
  template<class View>
  forceinline
  CHB::Storage::Storage(Home home, ViewArray<View>& x,
                        typename 
                        BranchTraits<typename View::VarType>::Merit bm)
    : n(x.size()), nf(0U), alpha(Kernel::Config::chb_alpha_init),
      chb(heap.alloc<Info>(x.size())) {
    if (bm) {
      for (int i=0; i<n; i++) {
        typename View::VarType xi(x[i].varimp());
        chb[i].lf = 0U;
        chb[i].qs = bm(home,xi,i);
      }
    } else {
      for (int i=0; i<n; i++) {
        chb[i].lf = 0U;
        chb[i].qs = Kernel::Config::chb_qscore_init;
      }
    }
  }
  forceinline void
  CHB::Storage::bump(void) {
    nf++;
    if (alpha > Kernel::Config::chb_alpha_limit) {
      alpha -= Kernel::Config::chb_alpha_decrement;
    }
  }
  forceinline void
  CHB::Storage::update(int i, bool failed) {
    if (failed) {
      chb[i].lf = nf;
      double reward = 1.0 / (nf - chb[i].lf + 1);
      chb[i].qs = (1.0 - alpha) * chb[i].qs + alpha * reward;
    } else {
      double reward = 0.9 / (nf - chb[i].lf + 1);
      chb[i].qs = (1.0 - alpha) * chb[i].qs + alpha * reward;
    }
  }


  /*
   * CHB
   *
   */

  forceinline CHB::Storage&
  CHB::object(void) const {
    return static_cast<CHB::Storage&>(*SharedHandle::object());
  }

  forceinline void
  CHB::object(CHB::Storage& o) {
    SharedHandle::object(&o);
  }

  forceinline double
  CHB::operator [](int i) const {
    assert((i >= 0) && (i < object().n));
    return object().chb[i].qs;
  }
  forceinline int
  CHB::size(void) const {
    return object().n;
  }
  forceinline void
  CHB::acquire(void) {
    object().m.acquire();
  }
  forceinline void
  CHB::release(void) {
    object().m.release();
  }
  forceinline void
  CHB::bump(void) {
    object().bump();
  }
  forceinline void
  CHB::update(int i, bool failed) {
    object().update(i,failed);
  }

  forceinline
  CHB::CHB(void) {}

  template<class View>
  forceinline
  CHB::CHB(Home home, ViewArray<View>& x,
           typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(!*this);
    object(*new Storage(home,x,bm));
    (void) Recorder<View>::post(home,x,*this);
  }
  template<class View>
  forceinline void
  CHB::init(Home home, ViewArray<View>& x,
            typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(!*this);
    object(*new Storage(home,x,bm));
    (void) Recorder<View>::post(home,x,*this);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const CHB& chb) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (chb.size() > 0) {
      s << chb[0];
      for (int i=1; i<chb.size(); i++)
        s << ", " << chb[i];
    }
    s << '}';
    return os << s.str();
  }


  /*
   * Propagation for chb recorder
   *
   */
  template<class View>
  forceinline
  CHB::Recorder<View>::Recorder(Space& home, Recorder<View>& p)
    : NaryPropagator<View,PC_GEN_NONE>(home,p), chb(p.chb) {
    c.update(home, p.c);
  }

  template<class View>
  Propagator*
  CHB::Recorder<View>::copy(Space& home) {
    return new (home) Recorder<View>(home, *this);
  }

  template<class View>
  inline size_t
  CHB::Recorder<View>::dispose(Space& home) {
    // Delete access to chb information
    home.ignore(*this,AP_DISPOSE);
    chb.~CHB();
    // Cancel remaining advisors
    for (Advisors<Idx> as(c); as(); ++as)
      x[as.advisor().idx()].cancel(home,as.advisor(),true);
    c.dispose(home);
    (void) NaryPropagator<View,PC_GEN_NONE>::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  PropCost
  CHB::Recorder<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::record();
  }

  template<class View>
  void
  CHB::Recorder<View>::reschedule(Space& home) {
    View::schedule(home,*this,ME_GEN_ASSIGNED);
  }

  template<class View>
  ExecStatus
  CHB::Recorder<View>::advise(Space&, Advisor& a, const Delta&) {
    static_cast<Idx&>(a).mark();
    return ES_NOFIX;
  }

  template<class View>
  void
  CHB::Recorder<View>::advise(Space&, Advisor& a) {
    static_cast<Idx&>(a).mark();
  }

  template<class View>
  ExecStatus
  CHB::Recorder<View>::propagate(Space& home, const ModEventDelta&) {
    // Lock chb information
    chb.acquire();
    if (home.failed()) {
      chb.bump();
      for (Advisors<Idx> as(c); as(); ++as) {
        int i = as.advisor().idx();
        if (as.advisor().marked()) {
          as.advisor().unmark();
          chb.update(i,true);
          if (x[i].assigned())
            as.advisor().dispose(home,c);
        }
      }
    } else {
      for (Advisors<Idx> as(c); as(); ++as) {
        int i = as.advisor().idx();
        if (as.advisor().marked()) {
          as.advisor().unmark();
          chb.update(i,false);
          if (x[i].assigned())
            as.advisor().dispose(home,c);
        }
      }
    }
    chb.release();
    return c.empty() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


}

// STATISTICS: kernel-branch
