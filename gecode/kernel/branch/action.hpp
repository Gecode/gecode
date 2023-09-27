/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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
   * \brief Class for action management
   *
   */
  class Action : public SharedHandle {
  protected:
    template<class View, bool p, bool f>
    class Recorder;
    /// Object for storing action values
    class GECODE_VTABLE_EXPORT Storage : public SharedHandle::Object {
    public:
      /// Mutex to synchronize globally shared access
      GECODE_KERNEL_EXPORT static Support::Mutex m;
      /// Number of action values
      int n;
      /// Inverse decay factor
      double invd;
      /// Action values (more follow)
      double* a;
      /// Initialize action values
      template<class View>
      Storage(Home home, ViewArray<View>& x, double d,
              typename BranchTraits<typename View::VarType>::Merit bm);
      /// Update action value at position \a i
      void update(int i);
      /// Delete object
      GECODE_KERNEL_EXPORT
      ~Storage(void);
    };
    /// Return object of correct type
    Storage& object(void) const;
    /// Set object to \a o
    void object(Storage& o);
    /// Update action value at position \a i
    void update(int i);
    /// Acquire mutex
    void acquire(void);
    /// Release mutex
    void release(void);
  public:
    /// \name Constructors and initialization
    //@{
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized action storage is init and the assignment operator.
     *
     */
    Action(void);
    /// Copy constructor
    GECODE_KERNEL_EXPORT
    Action(const Action& a);
    /// Assignment operator
    GECODE_KERNEL_EXPORT
    Action& operator =(const Action& a);
    /**
     * \brief Initialize for views \a x and decay factor \a d and action as defined by \a bm
     *
     * Count propagation if \a p is true, count failure if \a f is true.
     */
    template<class View>
    Action(Home home, ViewArray<View>& x, double d, bool p, bool f,
           typename BranchTraits<typename View::VarType>::Merit bm);
    /**
     * \brief Initialize for views \a x and decay factor \a d and action as defined by \a bm
     *
     * Count propagation if \a p is true, count failure if \a f is true.
     */
    template<class View>
    void init(Home home, ViewArray<View>& x, double d, bool p, bool f,
              typename BranchTraits<typename View::VarType>::Merit bm);
    /// Default (empty) action information
    GECODE_KERNEL_EXPORT static const Action def;
    //@}

    /// Destructor
    GECODE_KERNEL_EXPORT
    ~Action(void);

    /// \name Information access
    //@{
    /// Return action value at position \a i
    double operator [](int i) const;
    /// Return number of action values
    int size(void) const;
    //@}

    /// \name Decay factor for aging
    //@{
    /// Set decay factor to \a d
    GECODE_KERNEL_EXPORT
    void decay(Space& home, double d);
    /// Return decay factor
    GECODE_KERNEL_EXPORT
    double decay(const Space& home) const;
    //@}
  };

  /// Propagator for recording action information
  template<class View, bool p, bool f>
  class Action::Recorder : public NaryPropagator<View,PC_GEN_NONE> {
  protected:
    using NaryPropagator<View,PC_GEN_NONE>::x;
    /// Advisor with index and change information
    class Idx : public Advisor {
    protected:
      /// Index and mark/failed information
      unsigned int _info;
    public:
      /// Constructor for creation
      Idx(Space& home, Propagator& r, Council<Idx>& c, int i);
      /// Constructor for cloning \a a
      Idx(Space& home, Idx& a);
      /// Mark index as propagated
      void propagate(void);
      /// Mark index as failed
      void fail(void);
      /// Whether index has been propagated
      bool propagated(void) const;
      /// Whether index has been failed
      bool failed(void) const;
      /// Clear any information
      void clear(void);
      /// Get index of view
      int idx(void) const;
    };
    /// Access to action information
    Action a;
    /// The advisor council
    Council<Idx> c;
    /// Constructor for cloning \a r
    Recorder(Space& home, Recorder<View,p,f>& r);
  public:
    /// Constructor for creation
    Recorder(Home home, ViewArray<View>& x, Action& a);
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
    /// Post action recorder propagator
    static ExecStatus post(Home home, ViewArray<View>& x, Action& a);
  };

  /**
   * \brief Print action values enclosed in curly brackets
   * \relates Action
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const Action& a);


  /*
   * Advisor for action recorder
   *
   */
  template<class View, bool p, bool f>
  forceinline
  Action::Recorder<View,p,f>::Idx::Idx(Space& home, Propagator& r,
                                       Council<Idx>& c, int i)
    : Advisor(home,r,c), _info(static_cast<unsigned int>(i) << 2U) {}
  template<class View, bool p, bool f>
  forceinline
  Action::Recorder<View,p,f>::Idx::Idx(Space& home, Idx& a)
    : Advisor(home,a), _info(a._info) {
  }
  template<class View, bool p, bool f>
  forceinline void
  Action::Recorder<View,p,f>::Idx::propagate(void) {
    _info |= 1U;
  }
  template<class View, bool p, bool f>
  forceinline void
  Action::Recorder<View,p,f>::Idx::fail(void) {
    _info |= 2U;
  }
  template<class View, bool p, bool f>
  forceinline bool
  Action::Recorder<View,p,f>::Idx::propagated(void) const {
    return (_info & 1U) != 0;
  }
  template<class View, bool p, bool f>
  forceinline bool
  Action::Recorder<View,p,f>::Idx::failed(void) const {
    return (_info & 2U) != 0;
  }
  template<class View, bool p, bool f>
  forceinline void
  Action::Recorder<View,p,f>::Idx::clear(void) {
    _info &= ~3U;
  }
  template<class View, bool p, bool f>
  forceinline int
  Action::Recorder<View,p,f>::Idx::idx(void) const {
    return static_cast<int>(_info >> 2);
  }


  /*
   * Posting of action recorder propagator
   *
   */
  template<class View, bool p, bool f>
  forceinline
  Action::Recorder<View,p,f>::Recorder(Home home, ViewArray<View>& x,
                                       Action& a0)
    : NaryPropagator<View,PC_GEN_NONE>(home,x), a(a0), c(home) {
    home.notice(*this,AP_DISPOSE);
    for (int i=0; i<x.size(); i++)
      if (!x[i].assigned())
        x[i].subscribe(home,*new (home) Idx(home,*this,c,i), true);
  }

  template<class View, bool p, bool f>
  forceinline ExecStatus
  Action::Recorder<View,p,f>::post(Home home, ViewArray<View>& x, Action& a) {
    (void) new (home) Recorder<View,p,f>(home,x,a);
    return ES_OK;
  }


  /*
   * Action value storage
   *
   */

  template<class View>
  forceinline
  Action::Storage::Storage(Home home, ViewArray<View>& x, double d,
                           typename
                           BranchTraits<typename View::VarType>::Merit bm)
    : n(x.size()), invd(1.0 / d), a(heap.alloc<double>(x.size())) {
    if (bm)
      for (int i=0; i<n; i++) {
        typename View::VarType xi(x[i].varimp());
        a[i] = bm(home,xi,i);
      }
    else
      for (int i=0; i<n; i++)
        a[i] = 1.0;
  }
  forceinline void
  Action::Storage::update(int i) {
    /*
     * The trick to inverse decay is from: An Extensible SAT-solver,
     * Niklas Eén, Niklas Sörensson, SAT 2003.
     */
    assert((i >= 0) && (i < n));
    a[i] = invd * (a[i] + 1.0);
    if (a[i] > Kernel::Config::rescale_limit)
      for (int j=0; j<n; j++)
        a[j] *= Kernel::Config::rescale;
  }


  /*
   * Action
   *
   */

  forceinline Action::Storage&
  Action::object(void) const {
    return static_cast<Action::Storage&>(*SharedHandle::object());
  }

  forceinline void
  Action::object(Action::Storage& o) {
    SharedHandle::object(&o);
  }

  forceinline void
  Action::update(int i) {
    object().update(i);
  }
  forceinline double
  Action::operator [](int i) const {
    assert((i >= 0) && (i < object().n));
    return object().a[i];
  }
  forceinline int
  Action::size(void) const {
    return object().n;
  }
  forceinline void
  Action::acquire(void) {
    object().m.acquire();
  }
  forceinline void
  Action::release(void) {
    object().m.release();
  }


  forceinline
  Action::Action(void) {}

  template<class View>
  forceinline
  Action::Action(Home home, ViewArray<View>& x, double d,
                 bool p, bool f,
                 typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(!*this);
    if (!p && !f)
      return;
    object(*new Storage(home,x,d,bm));
    if (p && f)
      (void) Recorder<View,true,true>::post(home,x,*this);
    else if (p && !f)
      (void) Recorder<View,true,false>::post(home,x,*this);
    else if (!p && f)
      (void) Recorder<View,false,true>::post(home,x,*this);
  }
  template<class View>
  forceinline void
  Action::init(Home home, ViewArray<View>& x, double d,
               bool p, bool f,
               typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(!*this);
    if (!p && !f)
      return;
    object(*new Storage(home,x,d,bm));
    if (p && f)
      (void) Recorder<View,true,true>::post(home,x,*this);
    else if (p && !f)
      (void) Recorder<View,true,false>::post(home,x,*this);
    else if (!p && f)
      (void) Recorder<View,false,true>::post(home,x,*this);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const Action& a) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (a.size() > 0) {
      s << a[0];
      for (int i=1; i<a.size(); i++)
        s << ", " << a[i];
    }
    s << '}';
    return os << s.str();
  }


  /*
   * Propagation for action recorder
   *
   */
  template<class View, bool p, bool f>
  forceinline
  Action::Recorder<View,p,f>::Recorder(Space& home, Recorder<View,p,f>& r)
    : NaryPropagator<View,PC_GEN_NONE>(home,r), a(r.a) {
    c.update(home, r.c);
  }

  template<class View, bool p, bool f>
  Propagator*
  Action::Recorder<View,p,f>::copy(Space& home) {
    return new (home) Recorder<View,p,f>(home, *this);
  }

  template<class View, bool p, bool f>
  inline size_t
  Action::Recorder<View,p,f>::dispose(Space& home) {
    // Delete access to action information
    home.ignore(*this,AP_DISPOSE);
    a.~Action();
    // Cancel remaining advisors
    for (Advisors<Idx> as(c); as(); ++as)
      x[as.advisor().idx()].cancel(home,as.advisor(),true);
    c.dispose(home);
    (void) NaryPropagator<View,PC_GEN_NONE>::dispose(home);
    return sizeof(*this);
  }

  template<class View, bool p, bool f>
  PropCost
  Action::Recorder<View,p,f>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::record();
  }

  template<class View, bool p, bool f>
  void
  Action::Recorder<View,p,f>::reschedule(Space& home) {
    View::schedule(home,*this,ME_GEN_ASSIGNED);
  }

  template<class View, bool p, bool f>
  ExecStatus
  Action::Recorder<View,p,f>::advise(Space& home, Advisor& _a, const Delta&) {
    Idx& a = static_cast<Idx&>(_a);
    if (p) {
      a.propagate();
      return ES_NOFIX;
    } else {
      return x[a.idx()].assigned() ? home.ES_FIX_DISPOSE(c,a) : ES_FIX;
    }
  }

  template<class View, bool p, bool f>
  void
  Action::Recorder<View,p,f>::advise(Space&, Advisor& _a) {
    Idx& a = static_cast<Idx&>(_a);
    if (f)
      a.fail();
  }

  template<class View, bool p, bool f>
  ExecStatus
  Action::Recorder<View,p,f>::propagate(Space& home, const ModEventDelta&) {
    // Lock action information
    a.acquire();
    for (Advisors<Idx> as(c); as(); ++as) {
      int i = as.advisor().idx();
      if (p && as.advisor().propagated()) {
        as.advisor().clear();
        a.update(i);
        if (x[i].assigned())
          as.advisor().dispose(home,c);
      }
      if (f && as.advisor().failed()) {
        as.advisor().clear();
        a.update(i);
        if (x[i].assigned())
          as.advisor().dispose(home,c);
      }
    }
    a.release();
    return c.empty() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


}

// STATISTICS: kernel-branch
