/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <cfloat>

namespace Gecode {

  /**
   * \brief Class for action management
   *
   */
  class Action {
  protected:
    template<class View>
    class Recorder;
    /// Object for storing action values
    class Storage : public HeapAllocated {
    public:
      /// Mutex to synchronize globally shared access
      Support::Mutex m;
      /// How many references exist for this object
      unsigned int use_cnt;
      /// Number of action values
      int n;
      /// Action values
      double* a;
      /// Inverse decay factor
      double invd;
      /// Allocate action values
      template<class View>
      Storage(Home home, ViewArray<View>& x, double d,
              typename BranchTraits<typename View::VarType>::Merit bm);
      /// Delete object
      ~Storage(void);
      /// Update action value at position \a i
      void update(int i);
    };

    /// Pointer to storage object
    Storage* storage;
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
     * \brief Construct as not yet intialized
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
    /// Initialize for views \a x and decay factor \a d and action as defined by \a bm
    template<class View>
    Action(Home home, ViewArray<View>& x, double d,
           typename BranchTraits<typename View::VarType>::Merit bm);
    /// Initialize for views \a x and decay factor \a d and action as defined by \a bm
    template<class View>
    void init(Home home, ViewArray<View>& x, double d,
              typename BranchTraits<typename View::VarType>::Merit bm);
    /// Test whether already initialized
    operator bool(void) const;
    /// Default (empty) action information
    GECODE_KERNEL_EXPORT static const Action def;
    //@}

    /// \name Update and delete action information
    //@{
    /// Updating during cloning
    GECODE_KERNEL_EXPORT
    void update(Space& home, bool share, Action& a);
    /// Destructor
    GECODE_KERNEL_EXPORT
    ~Action(void);
    //@}

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
  template<class View>
  class Action::Recorder : public NaryPropagator<View,PC_GEN_NONE> {
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
      Idx(Space& home, bool share, Idx& a);
      /// Mark advisor as modified
      void mark(void);
      /// Mark advisor as unmodified
      void unmark(void);
      /// Whether advisor's view has been marked
      bool marked(void) const;
      /// Get index of view
      int idx(void) const;
    };
    /// Access to action information
    Action a;
    /// The advisor council
    Council<Idx> c;
    /// Constructor for cloning \a p
    Recorder(Space& home, bool share, Recorder<View>& p);
  public:
    /// Constructor for creation
    Recorder(Home home, ViewArray<View>& x, Action& a);
    /// Copy propagator during cloning
    virtual Propagator* copy(Space& home, bool share);
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
  template<class View>
  forceinline
  Action::Recorder<View>::Idx::Idx(Space& home, Propagator& p,
                                   Council<Idx>& c, int i)
    : Advisor(home,p,c), _info(i << 1) {}
  template<class View>
  forceinline
  Action::Recorder<View>::Idx::Idx(Space& home, bool share, Idx& a)
    : Advisor(home,share,a), _info(a._info) {
  }
  template<class View>
  forceinline void
  Action::Recorder<View>::Idx::mark(void) {
    _info |= 1;
  }
  template<class View>
  forceinline bool
  Action::Recorder<View>::Idx::marked(void) const {
    return (_info & 1) != 0;
  }
  template<class View>
  forceinline void
  Action::Recorder<View>::Idx::unmark(void) {
    assert(marked());
    _info -= 1;
  }
  template<class View>
  forceinline int
  Action::Recorder<View>::Idx::idx(void) const {
    return _info >> 1;
  }


  /*
   * Posting of action recorder propagator
   *
   */
  template<class View>
  forceinline
  Action::Recorder<View>::Recorder(Home home, ViewArray<View>& x,
                                   Action& a0)
    : NaryPropagator<View,PC_GEN_NONE>(home,x), a(a0), c(home) {
    home.notice(*this,AP_DISPOSE);
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        x[i].subscribe(home,*new (home) Idx(home,*this,c,i), true);
  }

  template<class View>
  forceinline ExecStatus
  Action::Recorder<View>::post(Home home, ViewArray<View>& x, Action& a) {
    (void) new (home) Recorder<View>(home,x,a);
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
    : use_cnt(1), n(x.size()), a(heap.alloc<double>(x.size())),
      invd(1.0 / d) {
    if (bm)
      for (int i=n; i--; ) {
        typename View::VarType xi(x[i].varimp());
        a[i] = bm(home,xi,i);
      }
    else
      for (int i=n; i--; )
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
      for (int j=n; j--; )
        a[j] *= Kernel::Config::rescale;
  }
  forceinline
  Action::Storage::~Storage(void) {
    heap.free<double>(a,n);
  }


  /*
   * Action
   *
   */

  forceinline void
  Action::update(int i) {
    storage->update(i);
  }
  forceinline double
  Action::operator [](int i) const {
    assert((i >= 0) && (i < storage->n));
    return storage->a[i];
  }
  forceinline int
  Action::size(void) const {
    return storage->n;
  }
  forceinline void
  Action::acquire(void) {
    storage->m.acquire();
  }
  forceinline void
  Action::release(void) {
    storage->m.release();
  }


  forceinline
  Action::Action(void) : storage(NULL) {}

  forceinline
  Action::operator bool(void) const {
    return storage != NULL;
  }

  template<class View>
  forceinline
  Action::Action(Home home, ViewArray<View>& x, double d,
                 typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(storage == NULL);
    storage = new Storage(home,x,d,bm);
    (void) Recorder<View>::post(home,x,*this);
  }
  template<class View>
  forceinline void
  Action::init(Home home, ViewArray<View>& x, double d,
               typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(storage == NULL);
    storage = new Storage(home,x,d,bm);
    (void) Recorder<View>::post(home,x,*this);
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
  template<class View>
  forceinline
  Action::Recorder<View>::Recorder(Space& home, bool share, Recorder<View>& p)
    : NaryPropagator<View,PC_GEN_NONE>(home,share,p) {
    a.update(home, share, p.a);
    c.update(home, share, p.c);
  }

  template<class View>
  Propagator*
  Action::Recorder<View>::copy(Space& home, bool share) {
    return new (home) Recorder<View>(home, share, *this);
  }

  template<class View>
  inline size_t
  Action::Recorder<View>::dispose(Space& home) {
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

  template<class View>
  PropCost
  Action::Recorder<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::record();
  }

  template<class View>
  void
  Action::Recorder<View>::reschedule(Space& home) {
    View::schedule(home,*this,ME_GEN_ASSIGNED);
  }

  template<class View>
  ExecStatus
  Action::Recorder<View>::advise(Space&, Advisor& a, const Delta&) {
    static_cast<Idx&>(a).mark();
    return ES_NOFIX;
  }

  template<class View>
  void
  Action::Recorder<View>::advise(Space&, Advisor& a) {
    static_cast<Idx&>(a).mark();
  }

  template<class View>
  ExecStatus
  Action::Recorder<View>::propagate(Space& home, const ModEventDelta&) {
    // Lock action information
    a.acquire();
    for (Advisors<Idx> as(c); as(); ++as) {
      int i = as.advisor().idx();
      if (as.advisor().marked()) {
        as.advisor().unmark();
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
