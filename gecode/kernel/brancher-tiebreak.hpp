/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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
   * \defgroup TaskBranchTieBreak Generic view tie-breaking for brancher based on view and value selection
   *
   * \ingroup TaskBranchViewVal
   */
  //@{
  /**
   * \brief View selection class for static tie breaking
   */
  template<class A, class B>
  class ViewSelTieBreakStatic {
  protected:
    /// First selection class
    A a;
    /// Second selection class
    B b;
  public:
    /// View type
    typedef typename A::View View;
    /// View selection choice
    class Choice {
    public:
      /// First choice
      typename A::Choice a;
      /// Second choice
      typename B::Choice b;
      /// Constructor
      Choice(const typename A::Choice& a, const typename B::Choice& b);
      /// Report size occupied
      size_t size(void) const;
      /// Archive into \a e
      void archive(Archive& e) const;
    };
    /// Default constructor
    ViewSelTieBreakStatic(void);
    /// Constructor for initialization
    ViewSelTieBreakStatic(Space& home, A& a, B& b);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, typename A::View x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, typename A::View x);
    /// Return choice
    Choice choice(Space& home);
    /// Return choice
    Choice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home, const Choice& c, unsigned int a);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelTieBreakStatic& vs);
    /// Delete view selection
    void dispose(Space& home);
  };

  /**
   * \brief Virtualized choice baseclass
   */
  class ChoiceVirtualBase {
  public:
    /// Create copy
    virtual ChoiceVirtualBase* copy(void) const = 0;
    /// Report size required
    virtual size_t size(void) const = 0;
    /// Destructor
    GECODE_KERNEL_EXPORT virtual ~ChoiceVirtualBase(void);
    /// Archive into \a e
    virtual void archive(Archive& e) const = 0;
    /// \name Memory management
    //@{
    /// Allocate memory
    static void* operator new(size_t s);
    /// Delete memory
    static void  operator delete(void*);
    //@}
  };

  /**
   * \brief Virtualized view selection base class
   */
  template<class View>
  class ViewSelVirtualBase {
  public:
    /// Intialize with view \a x
    virtual ViewSelStatus init(Space& home, View x) = 0;
    /// Possibly select better view \a x
    virtual ViewSelStatus select(Space& home, View x) = 0;
    /// Return choice
    virtual ChoiceVirtualBase* choice(Space& home) = 0;
    /// Return choice
    virtual ChoiceVirtualBase* choice(const Space& home, Archive& e) = 0;
    /// Commit to choice
    virtual void commit(Space& home, const ChoiceVirtualBase* c, 
                        unsigned int a) = 0;
    /// Create copy
    virtual ViewSelVirtualBase<View>* copy(Space& home, bool share) = 0;
    /// Delete view selection and return its size
    virtual size_t dispose(Space& home) = 0;
    /// \name Memory management
    //@{
    /// Allocate memory from space
    static void* operator new(size_t s, Space& home);
    /// No-op for exceptions
    static void  operator delete(void* p, Space& home);
    /// Needed for exceptions
    static void  operator delete(void*);
    //@}
  };

  /**
   * \brief Virtualized choice
   */
  template<class Choice>
  class ChoiceVirtual : public ChoiceVirtualBase {
  public:
    /// Static choice object
    Choice choice;
    /// Constructor for initialization
    ChoiceVirtual(const Choice& c);
    /// Create copy
    virtual ChoiceVirtualBase* copy(void) const;
    /// Report size required
    virtual size_t size(void) const;
    /// Destructor
    virtual ~ChoiceVirtual(void);
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };

  /**
   * \brief Virtualized view selection
   */
  template<class ViewSel>
  class ViewSelVirtual : public ViewSelVirtualBase<typename ViewSel::View> {
  protected:
    /// Static view selection object
    ViewSel viewsel;
  public:
    /// Constructor for initialization
    ViewSelVirtual(Space& home, const VarBranchOptions& vbo);
    /// Constructor for cloning \a vsv
    ViewSelVirtual(Space& home, bool share, ViewSelVirtual& vsv);
    /// Intialize with view \a x
    virtual ViewSelStatus init(Space& home, typename ViewSel::View x);
    /// Possibly select better view \a x
    virtual ViewSelStatus select(Space& home, typename ViewSel::View x);
    /// Return choice
    virtual ChoiceVirtualBase* choice(Space& home);
    /// Return choice
    virtual ChoiceVirtualBase* choice(const Space& home, Archive& e);
    /// Commit to choice
    virtual void commit(Space& home, const ChoiceVirtualBase* d,
                        unsigned int a);
    /// Create copy during cloning
    virtual ViewSelVirtualBase<typename ViewSel::View>*
    copy(Space& home, bool share);
    /// Delete view selection and returns its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief View selection class for dynamic tie breaking
   */
  template<class _View>
  class ViewSelTieBreakDynamic {
  protected:
    /// Number of tie breakers
    int n;
    /// Tie breakers
    ViewSelVirtualBase<_View>** tb;
  public:
    /// View type
    typedef _View View;
    /// %Choice for tie breakers
    class Choice {
    public:
      /// Number of choices
      int n;
      /// Choices
      ChoiceVirtualBase** c;
      /// Constructor
      Choice(Space& home, ViewSelVirtualBase<_View>** tb, int n0);
      /// Constructor
      Choice(const Space& home, Archive& e,
             ViewSelVirtualBase<_View>** tb, int n0);
      /// Copy constructor
      Choice(const Choice& ce);
      /// Assignment operator
      const Choice& operator =(const Choice& ce);
      /// Perform commit
      void commit(Space& home, unsigned int a,
                  ViewSelVirtualBase<_View>** tb)  const;
      /// Report size occupied
      size_t size(void) const;
      /// Destructor
      ~Choice(void);
      /// Archive into \a e
      void archive(Archive& e) const;
    };
    /// Default constructor
    ViewSelTieBreakDynamic(void);
    /// Constructor for initialization
    ViewSelTieBreakDynamic(Space& home, ViewSelVirtualBase<_View>** vsv,
                           int n);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, _View x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, _View x);
    /// Return choice
    Choice choice(Space& home);
    /// Return choice
    Choice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home,
                const typename ViewSelTieBreakDynamic<_View>::Choice& c,
                unsigned int a);
    /// Updating during cloning
    void update(Space& home, bool share, ViewSelTieBreakDynamic& vs);
    /// Delete view selection
    void dispose(Space& home);
  };
  //@}


  // Select variable with static tie breaking
  template<class A, class B>
  forceinline
  ViewSelTieBreakStatic<A,B>::Choice::Choice(const typename A::Choice& a0,
                                             const typename B::Choice& b0)
    : a(a0), b(b0) {}
  template<class A, class B>
  forceinline size_t
  ViewSelTieBreakStatic<A,B>::Choice::size(void) const {
    return a.size() + b.size();
  }
  template<class A, class B>
  forceinline void
  ViewSelTieBreakStatic<A,B>::Choice::archive(Archive& e) const {
    a.archive(e);
    b.archive(e);
  }

  template<class A, class B>
  forceinline
  ViewSelTieBreakStatic<A,B>::ViewSelTieBreakStatic(void) {}
  template<class A, class B>
  forceinline
  ViewSelTieBreakStatic<A,B>::
  ViewSelTieBreakStatic(Space&, A& a0, B& b0)
    : a(a0), b(b0) {}
  template<class A, class B>
  forceinline ViewSelStatus
  ViewSelTieBreakStatic<A,B>::init(Space& home, typename A::View x) {
    ViewSelStatus s = a.init(home,x);
    return (b.init(home,x) != VSS_BEST) ? VSS_BETTER : s;
  }
  template<class A, class B>
  forceinline ViewSelStatus
  ViewSelTieBreakStatic<A,B>::select(Space& home, typename A::View x) {
    switch (a.select(home,x)) {
    case VSS_BEST:
      return (b.init(home,x) != VSS_BEST) ? VSS_BETTER : VSS_BEST;
    case VSS_BETTER:
      (void) b.init(home,x);
      return VSS_BETTER;
    case VSS_WORSE:
      return VSS_WORSE;
    case VSS_TIE:
      switch (b.select(home,x)) {
      case VSS_BEST:   return VSS_BETTER;
      case VSS_BETTER: return VSS_BETTER;
      case VSS_TIE:    return VSS_TIE;
      case VSS_WORSE:  return VSS_WORSE;
      default: GECODE_NEVER;
      }
    default: GECODE_NEVER;
      return VSS_WORSE;
    }
  }
  template<class A, class B>
  inline typename ViewSelTieBreakStatic<A,B>::Choice
  ViewSelTieBreakStatic<A,B>::choice(Space& home) {
    typename ViewSelTieBreakStatic<A,B>::Choice c(a.choice(home),
                                                  b.choice(home));
    return c;
  }
  template<class A, class B>
  inline typename ViewSelTieBreakStatic<A,B>::Choice
  ViewSelTieBreakStatic<A,B>::choice(const Space& home, Archive& e) {
    typename ViewSelTieBreakStatic<A,B>::Choice c(a.choice(home,e),
                                                  b.choice(home,e));
    return c;
  }
  template<class A, class B>
  forceinline void
  ViewSelTieBreakStatic<A,B>::commit(Space& home, const Choice& c,
                                     unsigned int al) {
    a.commit(home, c.a, al);
    b.commit(home, c.b, al);
  }
  template<class A, class B>
  forceinline void
  ViewSelTieBreakStatic<A,B>::update(Space& home, bool share,
                                     ViewSelTieBreakStatic<A,B>& vstb) {
    a.update(home,share,vstb.a);
    b.update(home,share,vstb.b);
  }
  template<class A, class B>
  forceinline void
  ViewSelTieBreakStatic<A,B>::dispose(Space& home) {
    a.dispose(home);
    b.dispose(home);
  }


  // Virtualized view selection
  template<class View>
  forceinline void
  ViewSelVirtualBase<View>::operator delete(void*) {}
  template<class View>
  forceinline void
  ViewSelVirtualBase<View>::operator delete(void*, Space&) {}
  template<class View>
  forceinline void*
  ViewSelVirtualBase<View>::operator new(size_t s, Space& home) {
    return home.ralloc(s);
  }

  // Virtualized choice
  forceinline void
  ChoiceVirtualBase::operator delete(void* p) {
    heap.rfree(p);
  }
  forceinline void*
  ChoiceVirtualBase::operator new(size_t s) {
    return heap.ralloc(s);
  }
  forceinline
  ChoiceVirtualBase::~ChoiceVirtualBase(void) {
  }


  template<class Choice>
  forceinline
  ChoiceVirtual<Choice>::ChoiceVirtual(const Choice& c)
    : choice(c) {}
  template<class Choice>
  forceinline ChoiceVirtualBase*
  ChoiceVirtual<Choice>::copy(void) const {
    return new ChoiceVirtual<Choice>(choice);
  }
  template<class Choice>
  forceinline size_t
  ChoiceVirtual<Choice>::size(void) const {
    return sizeof(ChoiceVirtual<Choice>);
  }
  template<class Choice>
  ChoiceVirtual<Choice>::~ChoiceVirtual(void) {}
  template<class Choice> void
  ChoiceVirtual<Choice>::archive(Archive& e) const {
    choice.archive(e);
  }

  template<class ViewSel>
  forceinline
  ViewSelVirtual<ViewSel>::ViewSelVirtual(Space& home,
                                          const VarBranchOptions& vbo)
    : viewsel(home,vbo) {}
  template<class ViewSel>
  forceinline
  ViewSelVirtual<ViewSel>::ViewSelVirtual(Space& home, bool share,
                                          ViewSelVirtual<ViewSel>& vsv) {
    viewsel.update(home,share,vsv.viewsel);
  }
  template<class ViewSel>
  ViewSelStatus
  ViewSelVirtual<ViewSel>::init(Space& home, typename ViewSel::View x) {
    return viewsel.init(home,x);
  }
  template<class ViewSel>
  ViewSelStatus
  ViewSelVirtual<ViewSel>::select(Space& home, typename ViewSel::View x) {
    return viewsel.select(home,x);
  }
  template<class ViewSel>
  ChoiceVirtualBase*
  ViewSelVirtual<ViewSel>::choice(Space& home) {
    return new ChoiceVirtual<typename ViewSel::Choice>(viewsel.choice(home));
  }
  template<class ViewSel>
  ChoiceVirtualBase*
  ViewSelVirtual<ViewSel>::choice(const Space& home, Archive& e) {
    return new ChoiceVirtual<typename ViewSel::Choice>(viewsel.choice(home,e));
  }
  template<class ViewSel>
  void
  ViewSelVirtual<ViewSel>::commit(Space& home, const ChoiceVirtualBase* _c,
                                  unsigned int a) {
    const ChoiceVirtual<typename ViewSel::Choice>* c =
      static_cast<const ChoiceVirtual<typename ViewSel::Choice>*>(_c);
    viewsel.commit(home, c->choice, a);
  }
  template<class ViewSel>
  ViewSelVirtualBase<typename ViewSel::View>*
  ViewSelVirtual<ViewSel>::copy(Space& home, bool share) {
    return new (home) ViewSelVirtual<ViewSel>(home,share,*this);
  }
  template<class ViewSel>
  size_t
  ViewSelVirtual<ViewSel>::dispose(Space& home) {
    viewsel.dispose(home);
    return sizeof(ViewSelVirtual<ViewSel>);
  }


  // Choice for dynamic tie breaking
  template<class View>
  forceinline
  ViewSelTieBreakDynamic<View>::Choice::Choice
  (Space& home, ViewSelVirtualBase<View>** tb, int n0)
    : n(n0), c(heap.alloc<ChoiceVirtualBase*>(n)) {
    for (int i=n; i--; )
      c[i] = tb[i]->choice(home);
  }
  template<class View>
  forceinline
  ViewSelTieBreakDynamic<View>::Choice::Choice
  (const Space& home, Archive& e, ViewSelVirtualBase<View>** tb,
   int n0)
    : n(n0), c(heap.alloc<ChoiceVirtualBase*>(n)) {
    for (int i=n; i--; )
      c[i] = tb[i]->choice(home,e);
  }
  template<class View>
  forceinline
  ViewSelTieBreakDynamic<View>::Choice::Choice(const Choice& ce)
    : n(ce.n), c(heap.alloc<ChoiceVirtualBase*>(n)) {
    for (int i=n; i--; )
      c[i] = ce.c[i]->copy();
  }
  template<class View>
  forceinline const typename ViewSelTieBreakDynamic<View>::Choice&
  ViewSelTieBreakDynamic<View>::Choice::operator =(const Choice& ce) {
    if (&ce != this) {
      assert(ce.n == n);
      for (int i=n; i--; ) {
        delete c[i]; c[i] = ce.c[i]->copy();
      }
    }
    return *this;
  }
  template<class View>
  forceinline void
  ViewSelTieBreakDynamic<View>::Choice::commit
  (Space& home, unsigned int a, ViewSelVirtualBase<View>** tb)  const {
    for (int i=n; i--; )
      tb[i]->commit(home, c[i], a);
  }
  template<class View>
  forceinline size_t
  ViewSelTieBreakDynamic<View>::Choice::size(void) const {
    size_t s = (sizeof(typename ViewSelTieBreakDynamic<View>::Choice) +
                n * sizeof(ChoiceVirtualBase*));
    for (int i=n; i--; )
      s += c[i]->size();
    return s;
  }
  template<class View>
  forceinline
  ViewSelTieBreakDynamic<View>::Choice::~Choice(void) {
    for (int i=n; i--; )
      delete c[i];
    heap.free(c,n);
  }
  template<class View>
  forceinline void
  ViewSelTieBreakDynamic<View>::Choice::archive(Archive& e) const 
  {
    for (int i=0; i<n; i++)
      c[i]->archive(e);
  }


  // Select variable with dynamic tie breaking
  template<class View>
  forceinline
  ViewSelTieBreakDynamic<View>::ViewSelTieBreakDynamic(void) {}
  template<class View>
  forceinline
  ViewSelTieBreakDynamic<View>::
  ViewSelTieBreakDynamic(Space& home, ViewSelVirtualBase<View>** vsv, int n0)
    : n(n0), tb(home.alloc<ViewSelVirtualBase<View>*>(n)) {
    for (int i=0; i<n; i++)
      tb[i]=vsv[i];
    assert(n > 0);
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelTieBreakDynamic<View>::init(Space& home, View x) {
    ViewSelStatus s = VSS_BEST;
    for (int i=0; i<n; i++)
      if (tb[i]->init(home,x) != VSS_BEST)
        s = VSS_BETTER;
    return s;
  }
  template<class View>
  forceinline ViewSelStatus
  ViewSelTieBreakDynamic<View>::select(Space& home, View x) {
    switch (tb[0]->select(home,x)) {
    case VSS_BEST:
      {
        ViewSelStatus s = VSS_BEST;
        for (int i=1; i<n; i++)
          if (tb[i]->init(home,x) != VSS_BEST)
            s = VSS_BETTER;
        return s;
      }
    case VSS_BETTER:
      for (int i=1; i<n; i++)
        (void) tb[i]->init(home,x);
      return VSS_BETTER;
    case VSS_WORSE:
      return VSS_WORSE;
    case VSS_TIE:
      for (int i=1; i<n; i++)
        switch (tb[i]->select(home,x)) {
        case VSS_BEST: case VSS_BETTER:
          for (int j=i+1; j<n; j++)
            (void) tb[j]->init(home,x);
          return VSS_BETTER;
        case VSS_TIE:    break;
        case VSS_WORSE:  return VSS_WORSE;
        default: GECODE_NEVER;
        }
      return VSS_TIE;
    default: GECODE_NEVER;
      return VSS_WORSE;
    }
  }
  template<class _View>
  inline typename ViewSelTieBreakDynamic<_View>::Choice
  ViewSelTieBreakDynamic<_View>::choice(Space& home) {
    Choice c(home,tb,n);
    return c;
  }
  template<class _View>
  inline typename ViewSelTieBreakDynamic<_View>::Choice
  ViewSelTieBreakDynamic<_View>::choice(const Space& home, Archive& e) {
    Choice c(home,e,tb,n);
    return c;
  }
  template<class _View>
  forceinline void
  ViewSelTieBreakDynamic<_View>::commit
  (Space& home, const typename ViewSelTieBreakDynamic<_View>::Choice& c,
   unsigned int a) {
    c.commit(home,a,tb);
  }
  template<class _View>
  forceinline void
  ViewSelTieBreakDynamic<_View>::update(Space& home, bool share,
                                        ViewSelTieBreakDynamic<_View>& vstb) {
    n = vstb.n;
    tb = home.alloc<ViewSelVirtualBase<View>*>(n);
    for (int i=0; i<n; i++)
      tb[i] = vstb.tb[i]->copy(home,share);
  }
  template<class _View>
  forceinline void
  ViewSelTieBreakDynamic<_View>::dispose(Space& home) {
    for (int i=0; i<n; i++)
      home.rfree(tb[i],tb[i]->dispose(home));
    home.free<ViewSelVirtualBase<_View>*>(tb,n);
  }

}

// STATISTICS: kernel-branch
