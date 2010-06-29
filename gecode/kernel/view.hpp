/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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
   * \brief Base-class for constant views
   * \ingroup TaskVarView
   */
  template<class View>
  class ConstView {
  public:
    /// The variable type corresponding to the constant view
    typedef typename View::VarImpType VarImpType;
    /// \name Generic view information
    //@{
    /// Return degree (number of subscribed propagators and advisors)
    unsigned int degree(void) const;
    /// Return accumulated failure count (plus degree)
    double afc(void) const;
    /// Return whether this view is derived from a VarImpView
    static bool varderived(void);
    /// Return dummy variable implementation of view
    VarImpType* varimp(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether view is assigned
    bool assigned(void) const;
    //@}

    /// \name View-dependent propagator support
    //@{
    /// Schedule propagator \a p with modification event \a me
    static void schedule(Space& home, Propagator& p, ModEvent me);
    /// Return modification event for view type in \a med
    static ModEvent me(const ModEventDelta& med);
    /// Translate modification event \a me to modification event delta for view
    static ModEventDelta med(ModEvent me);
    //@}

    /// \name Dependencies
    //@{
    /**
     * \brief Subscribe propagator \a p with propagation condition \a pc to view
     *
     * In case \a schedule is false, the propagator is just subscribed but
     * not scheduled for execution (this must be used when creating
     * subscriptions during propagation).
     */
    void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule=true);
    /// Cancel subscription of propagator \a p with propagation condition \a pc to view
    void cancel(Space& home, Propagator& p, PropCond pc);
    /// Subscribe advisor \a a to view
    void subscribe(Space& home, Advisor& a);
    /// Cancel subscription of advisor \a a
    void cancel(Space& home, Advisor& a);
    //@}

    /// \name Delta information for advisors
    //@{
    /// Return modification event
    static ModEvent modevent(const Delta& d);
    //@}

    /// \name Cloning
    //@{
    /// Update this view to be a clone of view \a y
    void update(Space& home, bool share, ConstView& y);
    //@}
  };



  /**
   * \brief Base-class for variable implementation views
   * \ingroup TaskVarView
   */
  template<class VarImp>
  class VarImpView {
  protected:
    /// Pointer to variable implementation
    VarImp* x;
    /// Default constructor
    VarImpView(void);
    /// Initialize with variable implementation \a y
    VarImpView(VarImp* y);
  public:
    /// The variable implementation type corresponding to the view
    typedef VarImp VarImpType;
    /// \name Generic view information
    //@{
    /// Return whether this view is derived from a VarImpView
    static bool varderived(void);
    /// Return variable implementation of view
    VarImp* varimp(void) const;
    /// Return degree (number of subscribed propagators and advisors)
    unsigned int degree(void) const;
    /// Return accumulated failure count (plus degree)
    double afc(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether view is assigned
    bool assigned(void) const;
    //@}

    /// \name View-dependent propagator support
    //@{
    /// Schedule propagator \a p with modification event \a me
    static void schedule(Space& home, Propagator& p, ModEvent me);
    /// Return modification event for view type in \a med
    static ModEvent me(const ModEventDelta& med);
    /// Translate modification event \a me to modification event delta for view
    static ModEventDelta med(ModEvent me);
    //@}

    /// \name Dependencies
    //@{
    /**
     * \brief Subscribe propagator \a p with propagation condition \a pc to view
     *
     * In case \a schedule is false, the propagator is just subscribed but
     * not scheduled for execution (this must be used when creating
     * subscriptions during propagation).
     */
    void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule=true);
    /// Cancel subscription of propagator \a p with propagation condition \a pc to view
    void cancel(Space& home, Propagator& p, PropCond pc);
    /// Subscribe advisor \a a to view
    void subscribe(Space& home, Advisor& a);
    /// Cancel subscription of advisor \a a
    void cancel(Space& home, Advisor& a);
    //@}

    /// \name Delta information for advisors
    //@{
    /// Return modification event
    static ModEvent modevent(const Delta& d);
    //@}

    /// \name Cloning
    //@{
    /// Update this view to be a clone of view \a y
    void update(Space& home, bool share, VarImpView<VarImp>& y);
    //@}
  };

  /** \name View comparison
   *  \relates VarImpView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  template<class VarImpA, class VarImpB>
  bool same(const VarImpView<VarImpA>& x, const VarImpView<VarImpB>& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  template<class ViewA, class ViewB>
  bool before(const ViewA& x, const ViewB& y);
  //@}


  /**
   * \brief Base-class for derived views
   * \ingroup TaskVarView
   */
  template<class View>
  class DerivedView {
  protected:
    /// View from which this view is derived
    View x;
    /// Default constructor
    DerivedView(void);
    /// Initialize with view \a y
    DerivedView(const View& y);
  public:
    /// The variable implementation type belonging to the \a View
    typedef typename View::VarImpType VarImpType;
    /// \name Generic view information
    //@{
    /// Return whether this view is derived from a VarImpView
    static bool varderived(void);
    /// Return variable implementation of view
    VarImpType* varimp(void) const;
    /// Return view from which this view is derived
    View base(void) const;
    /// Return degree (number of subscribed propagators)
    unsigned int degree(void) const;
    /// Return accumulated failure count (plus degree)
    double afc(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether view is assigned
    bool assigned(void) const;
    //@}

    /// \name View-dependent propagator support
    //@{
    /// Schedule propagator \a p with modification event \a me
    static void schedule(Space& home, Propagator& p, ModEvent me);
    /// Return modification event for view type in \a med
    static ModEvent me(const ModEventDelta& med);
    /// Translate modification event \a me to modification event delta for view
    static ModEventDelta med(ModEvent);
    //@}

    /// \name Dependencies
    //@{
    /**
     * \brief Subscribe propagator \a p with propagation condition \a pc to view
     *
     * In case \a schedule is false, the propagator is just subscribed but
     * not scheduled for execution (this must be used when creating
     * subscriptions during propagation).
     */
    void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule=true);
    /// Cancel subscription of propagator \a p with propagation condition \a pc to view
    void cancel(Space& home, Propagator& p, PropCond pc);
    /// Subscribe advisor \a a to view
    void subscribe(Space& home, Advisor& a);
    /// Cancel subscription of advisor \a a
    void cancel(Space& home, Advisor& a);
    //@}

    /// \name Delta information for advisors
    //@{
    /// Return modification event
    static ModEvent modevent(const Delta& d);
    //@}

    /// \name Cloning
    //@{
    /// Update this view to be a clone of view \a y
    void update(Space& home, bool share, DerivedView<View>& y);
    //@}
  };


  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class ViewA, class ViewB>
  bool shared(const ConstView<ViewA>&, const ConstView<ViewB>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class VarImp, class View>
  bool shared(const VarImpView<VarImp>&, const ConstView<View>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class ViewA, class ViewB>
  bool shared(const DerivedView<ViewA>&, const ConstView<ViewB>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class View, class VarImp>
  bool shared(const ConstView<View>&, const VarImpView<VarImp>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class ViewA, class ViewB>
  bool shared(const ConstView<ViewA>&, const DerivedView<ViewB>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class VarImpA, class VarImpB>
  bool shared(const VarImpView<VarImpA>&, const VarImpView<VarImpB>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class VarImpA, class ViewB>
  bool shared(const VarImpView<VarImpA>&, const DerivedView<ViewB>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class ViewA, class VarImpB>
  bool shared(const DerivedView<ViewA>&, const VarImpView<VarImpB>&);
  /**
   * \brief Test whether views share same variable
   * \ingroup TaskVarView
   */
  template<class ViewA, class ViewB>
  bool shared(const DerivedView<ViewA>&, const DerivedView<ViewB>&);


  /*
   * Constant view: has no variable implementation
   *
   */
  template<class View>
  forceinline unsigned int
  ConstView<View>::degree(void) const {
    return 0;
  }
  template<class View>
  forceinline double
  ConstView<View>::afc(void) const {
    return 0.0;
  }
  template<class View>
  forceinline bool
  ConstView<View>::varderived(void) {
    return false;
  }
  template<class View>
  forceinline typename View::VarImpType*
  ConstView<View>::varimp(void) const {
    return NULL;
  }
  template<class View>
  forceinline bool
  ConstView<View>::assigned(void) const {
    return true;
  }
  template<class View>
  forceinline void
  ConstView<View>::subscribe(Space& home, Propagator& p, PropCond, 
                             bool schedule) {
    if (schedule)
      View::schedule(home,p,ME_GEN_ASSIGNED);
  }
  template<class View>
  forceinline void
  ConstView<View>::cancel(Space&, Propagator&, PropCond) {
  }
  template<class View>
  forceinline void
  ConstView<View>::subscribe(Space&, Advisor&) {
  }
  template<class View>
  forceinline void
  ConstView<View>::cancel(Space&, Advisor&) {
  }
  template<class View>
  forceinline void
  ConstView<View>::schedule(Space& home, Propagator& p, ModEvent me) {
    View::schedule(home,p,me);
  }
  template<class View>
  forceinline ModEvent
  ConstView<View>::me(const ModEventDelta& med) {
    return View::me(med);
  }
  template<class View>
  forceinline ModEventDelta
  ConstView<View>::med(ModEvent me) {
    return View::med(me);
  }
  template<class View>
  forceinline ModEvent
  ConstView<View>::modevent(const Delta& d) {
    return ME_GEN_NONE;
  }
  template<class View>
  forceinline void
  ConstView<View>::update(Space&, bool, ConstView&) {
  }

  /*
   * Variable view: contains a pointer to a variable implementation
   *
   */
  template<class VarImp>
  forceinline
  VarImpView<VarImp>::VarImpView(void)
    : x(NULL) {}
  template<class VarImp>
  forceinline
  VarImpView<VarImp>::VarImpView(VarImp* y)
    : x(y) {}
  template<class VarImp>
  forceinline bool
  VarImpView<VarImp>::varderived(void) {
    return true;
  }
  template<class VarImp>
  forceinline VarImp*
  VarImpView<VarImp>::varimp(void) const {
    return x;
  }
  template<class VarImp>
  forceinline unsigned int
  VarImpView<VarImp>::degree(void) const {
    return x->degree();
  }
  template<class VarImp>
  forceinline double
  VarImpView<VarImp>::afc(void) const {
    return x->afc();
  }
  template<class VarImp>
  forceinline bool
  VarImpView<VarImp>::assigned(void) const {
    return x->assigned();
  }
  template<class VarImp>
  forceinline void
  VarImpView<VarImp>::subscribe(Space& home, Propagator& p, PropCond pc,
                                bool schedule) {
    x->subscribe(home,p,pc,schedule);
  }
  template<class VarImp>
  forceinline void
  VarImpView<VarImp>::cancel(Space& home, Propagator& p, PropCond pc) {
    x->cancel(home,p,pc);
  }
  template<class VarImp>
  forceinline void
  VarImpView<VarImp>::subscribe(Space& home, Advisor& a) {
    x->subscribe(home,a);
  }
  template<class VarImp>
  forceinline void
  VarImpView<VarImp>::cancel(Space& home, Advisor& a) {
    x->cancel(home,a);
  }
  template<class VarImp>
  forceinline void
  VarImpView<VarImp>::schedule(Space& home, Propagator& p, ModEvent me) {
    VarImp::schedule(home,p,me);
  }
  template<class VarImp>
  forceinline ModEvent
  VarImpView<VarImp>::me(const ModEventDelta& med) {
    return VarImp::me(med);
  }
  template<class VarImp>
  forceinline ModEventDelta
  VarImpView<VarImp>::med(ModEvent me) {
    return VarImp::med(me);
  }
  template<class VarImp>
  forceinline ModEvent
  VarImpView<VarImp>::modevent(const Delta& d) {
    return VarImp::modevent(d);
  }
  template<class VarImp>
  forceinline void
  VarImpView<VarImp>::update(Space& home, bool share, VarImpView<VarImp>& y) {
    x = y.x->copy(home,share);
  }

  /*
   * Derived view: contain the base view from which they are derived
   *
   */

  template<class View>
  forceinline
  DerivedView<View>::DerivedView(void) {}

  template<class View>
  forceinline
  DerivedView<View>::DerivedView(const View& y)
    : x(y) {}

  template<class View>
  forceinline bool
  DerivedView<View>::varderived(void) {
    return View::varderived();
  }

  template<class View>
  forceinline typename View::VarImpType*
  DerivedView<View>::varimp(void) const {
    return x.varimp();
  }

  template<class View>
  forceinline View
  DerivedView<View>::base(void) const {
    return x;
  }

  template<class View>
  forceinline unsigned int
  DerivedView<View>::degree(void) const {
    return x.degree();
  }
  template<class View>
  forceinline double
  DerivedView<View>::afc(void) const {
    return x.afc();
  }
  template<class View>
  forceinline bool
  DerivedView<View>::assigned(void) const {
    return x.assigned();
  }

  template<class View>
  forceinline void
  DerivedView<View>::schedule(Space& home, Propagator& p, ModEvent me) {
    return View::schedule(home,p,me);
  }
  template<class View>
  forceinline ModEvent
  DerivedView<View>::me(const ModEventDelta& med) {
    return View::me(med);
  }
  template<class View>
  forceinline ModEventDelta
  DerivedView<View>::med(ModEvent me) {
    return View::med(me);
  }

  template<class View>
  forceinline void
  DerivedView<View>::subscribe(Space& home, Propagator& p, PropCond pc,
                               bool schedule) {
    x.subscribe(home,p,pc,schedule);
  }
  template<class View>
  forceinline void
  DerivedView<View>::cancel(Space& home, Propagator& p, PropCond pc) {
    x.cancel(home,p,pc);
  }
  template<class View>
  forceinline void
  DerivedView<View>::subscribe(Space& home, Advisor& a) {
    x.subscribe(home,a);
  }
  template<class View>
  forceinline void
  DerivedView<View>::cancel(Space& home, Advisor& a) {
    x.cancel(home,a);
  }
  template<class View>
  forceinline ModEvent
  DerivedView<View>::modevent(const Delta& d) {
    return View::modevent(d);
  }
  template<class View>
  forceinline void
  DerivedView<View>::update(Space& home, bool share, DerivedView<View>& y) {
    x.update(home,share,y.x);
  }


  /*
   * Tests whether two views are the same
   *
   */

  /// Test whether two views are the same
  template<class ViewA, class ViewB>
  forceinline bool 
  same(const ConstView<ViewA>&, const ConstView<ViewB>&) {
    return false;
  }
  /// Test whether two views are the same
  template<class VarImp, class View>
  forceinline bool 
  same(const VarImpView<VarImp>&, const ConstView<View>&) {
    return false;
  }
  /// Test whether two views are the same
  template<class ViewA, class ViewB>
  forceinline bool 
  same(const ConstView<ViewA>&, const DerivedView<ViewB>&) {
    return false;
  }
  /// Test whether two views are the same
  template<class VarImpA, class ViewB>
  forceinline bool 
  same(const VarImpView<VarImpA>&, const DerivedView<ViewB>&) {
    return false;
  }
  /// Test whether two views are the same
  template<class ViewA, class VarImpB>
  forceinline bool 
  same(const DerivedView<ViewA>&, const VarImpView<VarImpB>&) {
    return false;
  }
  /// Test whether two views are the same
  template<class VarImp>
  forceinline bool
  same(const VarImpView<VarImp>& x, const VarImpView<VarImp>& y) {
    return x.varimp() == y.varimp();
  }
  /// Test whether two views are the same
  template<class ViewA, class ViewB>
  forceinline bool 
  same(const DerivedView<ViewA>& x, const DerivedView<ViewB>& y) {
    return same(x.base(),y.base());
  }


  /*
   * Tests whether one view is before the other
   *
   */
  template<class ViewA, class ViewB>
  forceinline bool
  before(const ViewA& x, const ViewB& y) {
    return x.varimp() < y.varimp();
  }


  /*
   * Testing whether two views share the same variable
   *
   */

  template<class ViewA, class ViewB>
  forceinline bool
  shared(const ConstView<ViewA>&, const ConstView<ViewB>&) {
    return false;
  }
  template<class VarImp, class View>
  forceinline bool
  shared(const VarImpView<VarImp>&, const ConstView<View>&) {
    return false;
  }
  template<class ViewA, class ViewB>
  forceinline bool
  shared(const DerivedView<ViewA>&, const ConstView<ViewB>&) {
    return false;
  }
  template<class View, class VarImp>
  forceinline bool
  shared(const ConstView<View>&, const VarImpView<VarImp>&) {
    return false;
  }
  template<class ViewA, class ViewB>
  forceinline bool
  shared(const ConstView<ViewA>&, const DerivedView<ViewB>&) {
    return false;
  }
  template<class VarImpA, class VarImpB>
  forceinline bool
  shared(const VarImpView<VarImpA>& x, const VarImpView<VarImpB>& y) {
    return (static_cast<VarImpBase*>(x.varimp()) ==
            static_cast<VarImpBase*>(y.varimp()));
  }
  template<class VarImpA, class ViewB>
  forceinline bool
  shared(const VarImpView<VarImpA>& x, const DerivedView<ViewB>& y) {
    return (ViewB::varderived() &&
            static_cast<VarImpBase*>(x.varimp()) ==
            static_cast<VarImpBase*>(y.varimp()));
  }
  template<class ViewA, class VarImpB>
  forceinline bool
  shared(const DerivedView<ViewA>& x, const VarImpView<VarImpB>& y) {
    return (ViewA::varderived() &&
            static_cast<VarImpBase*>(x.varimp()) ==
            static_cast<VarImpBase*>(y.varimp()));
  }
  template<class ViewA, class ViewB>
  forceinline bool
  shared(const DerivedView<ViewA>& x, const DerivedView<ViewB>& y) {
    return (ViewA::varderived() && ViewB::varderived() &&
            static_cast<VarImpBase*>(x.varimp()) ==
            static_cast<VarImpBase*>(y.varimp()));
  }

}

// STATISTICS: kernel-var
