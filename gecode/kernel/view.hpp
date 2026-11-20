/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Samuel Gagnon, 2018
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

namespace Gecode {

    /**
     * \brief Base-class for constant views
     * \ingroup TaskVarView
     */
    template <class View>
    class ConstView {
    public:
        /// The variable implementation type corresponding to the constant view
        using VarImpType = View::VarImpType;
        /// The variable type corresponding to the constant view
        using VarType = View::VarType;
        /// \name Generic view information
        //@{
        /// Return degree (number of subscribed propagators and advisors)
        unsigned int degree() const;
        /// Return accumulated failure count
        double afc() const;
        /// Return whether this view is derived from a VarImpView
        static bool is_variable_derived();
        /// Return dummy variable implementation of view
        VarImpType* varimp() const;
#ifdef GECODE_HAS_CBS
        /// Return dummy id
        unsigned int id() const;
#endif
        //@}

        /// \name Domain tests
        //@{
        /// Test whether view is assigned
        bool assigned() const;
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
        void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule = true);
        /// Cancel subscription of propagator \a p with propagation condition \a pc to view
        void cancel(Space& home, Propagator& p, PropCond pc);
        /// Re-schedule propagator \a p with propagation condition \a pc
        void reschedule(Space& home, Propagator& p, PropCond pc);
        /** \brief Subscribe advisor \a a to view
         *
         * If \a fail is true, run the advisor also on failure. This
         * feature is undocumented.
         */
        void subscribe(Space& home, Advisor& a, bool fail = false);
        /// Cancel subscription of advisor \a a
        void cancel(Space& home, Advisor& a, bool fail = false);
        //@}

        /// \name Delta information for advisors
        //@{
        /// Return modification event
        static ModEvent modification_event(const Delta& d);
        //@}

        /// \name Cloning
        //@{
        /// Update this view to be a clone of view \a y
        void update(Space& home, ConstView& y);
        //@}

        /// \name Ordering
        //@{
        /// Whether this view comes before view \a y (arbitrary order)
        bool operator<(const ConstView& y) const;
        //@}
    };

    /**
     * \brief Base-class for variable implementation views
     * \ingroup TaskVarView
     */
    template <class Var>
    class VarImpView {
    public:
        /// The variable type corresponding to the view
        using VarType = Var;
        /// The variable implementation type corresponding to the view
        using VarImpType = Var::VarImpType;

    protected:
        /// Pointer to variable implementation
        VarImpType* x;
        /// Default constructor
        VarImpView();
        /// Initialize with variable implementation \a y
        VarImpView(VarImpType* y);
        /// Set variable implementation to \a y
        void varimp(VarImpType* y);

    public:
        /// \name Generic view information
        //@{
        /// Return whether this view is derived from a VarImpView
        static bool is_variable_derived();
        /// Return variable implementation of view
        VarImpType* varimp() const;
        /// Return degree (number of subscribed propagators and advisors)
        unsigned int degree() const;
        /// Return accumulated failure count
        double afc() const;
#ifdef GECODE_HAS_CBS
        /// Return id of implementation of view
        unsigned int id() const;
#endif
        //@}

        /// \name Domain tests
        //@{
        /// Test whether view is assigned
        bool assigned() const;
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
        void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule = true);
        /// Cancel subscription of propagator \a p with propagation condition \a pc to view
        void cancel(Space& home, Propagator& p, PropCond pc);
        /// Re-schedule propagator \a p with propagation condition \a pc
        void reschedule(Space& home, Propagator& p, PropCond pc);
        /** \brief Subscribe advisor \a a to view
         *
         * If \a fail is true, run the advisor also on failure. This
         * feature is undocumented.
         */
        void subscribe(Space& home, Advisor& a, bool fail = false);
        /// Cancel subscription of advisor \a a
        void cancel(Space& home, Advisor& a, bool fail = false);
        //@}

        /// \name Delta information for advisors
        //@{
        /// Return modification event
        static ModEvent modification_event(const Delta& d);
        //@}

        /// \name Cloning
        //@{
        /// Update this view to be a clone of view \a y
        void update(Space& home, VarImpView<Var>& y);
        //@}

        /// \name Ordering
        //@{
        /// Whether this view comes before view \a y (arbitrary order)
        bool operator<(const VarImpView& y) const;
        //@}
    };

    /**
     * \brief Base-class for derived views
     * \ingroup TaskVarView
     */
    template <class View>
    class DerivedView {
    public:
        /// The variable implementation type belonging to the \a View
        using VarImpType = View::VarImpType;
        /// The variable type belonging to the \a View
        using VarType = View::VarType;

    protected:
        /// View from which this view is derived
        View x;
        /// Default constructor
        DerivedView() = default;

    public:
        /// Initialize with view \a y
        DerivedView(const View& y);
        /// \name Generic view information
        //@{
        /// Return whether this view is derived from a VarImpView
        static bool is_variable_derived();
        /// Return variable implementation of view
        VarImpType* varimp() const;
        /// Return view from which this view is derived
        View base() const;
        /// Return degree (number of subscribed propagators)
        unsigned int degree() const;
        /// Return accumulated failure count
        double afc() const;
#ifdef GECODE_HAS_CBS
        /// Return id of implementation of view
        unsigned int id() const;
#endif
        //@}

        /// \name Domain tests
        //@{
        /// Test whether view is assigned
        bool assigned() const;
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
        void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule = true);
        /// Cancel subscription of propagator \a p with propagation condition \a pc to view
        void cancel(Space& home, Propagator& p, PropCond pc);
        /// Re-schedule propagator \a p with propagation condition \a pc
        void reschedule(Space& home, Propagator& p, PropCond pc);
        /** \brief Subscribe advisor \a a to view
         *
         * If \a fail is true, run the advisor also on failure. This
         * feature is undocumented.
         */
        void subscribe(Space& home, Advisor& a, bool fail = false);
        /// Cancel subscription of advisor \a a
        void cancel(Space& home, Advisor& a, bool fail = false);
        //@}

        /// \name Delta information for advisors
        //@{
        /// Return modification event
        static ModEvent modification_event(const Delta& d);
        //@}

        /// \name Cloning
        //@{
        /// Update this view to be a clone of view \a y
        void update(Space& home, DerivedView<View>& y);
        //@}

        /// \name Ordering
        //@{
        /// Whether this view comes before view \a y (arbitrary order)
        bool operator<(const DerivedView<View>& y) const;
        //@}
    };

    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class ViewA, class ViewB>
    bool shared(const ConstView<ViewA>&, const ConstView<ViewB>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class Var, class View>
    bool shared(const VarImpView<Var>&, const ConstView<View>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class ViewA, class ViewB>
    bool shared(const DerivedView<ViewA>&, const ConstView<ViewB>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class View, class Var>
    bool shared(const ConstView<View>&, const VarImpView<Var>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class ViewA, class ViewB>
    bool shared(const ConstView<ViewA>&, const DerivedView<ViewB>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class VarA, class VarB>
    bool shared(const VarImpView<VarA>&, const VarImpView<VarB>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class Var, class View>
    bool shared(const VarImpView<Var>&, const DerivedView<View>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class View, class Var>
    bool shared(const DerivedView<View>&, const VarImpView<Var>&);
    /**
     * \brief Test whether views share same variable
     * \ingroup TaskVarView
     */
    template <class ViewA, class ViewB>
    bool shared(const DerivedView<ViewA>&, const DerivedView<ViewB>&);

    /*
     * Constant view: has no variable implementation
     *
     */
    template <class View>
    forceinline unsigned int
    ConstView<View>::degree() const {
        return 0;
    }
    template <class View>
    forceinline double
    ConstView<View>::afc() const {
        return 0.0;
    }
    template <class View>
    forceinline bool
    ConstView<View>::is_variable_derived() {
        return false;
    }
    template <class View>
    forceinline View::VarImpType*
    ConstView<View>::varimp() const {
        return nullptr;
    }
#ifdef GECODE_HAS_CBS
    template <class View>
    forceinline unsigned int
    ConstView<View>::id() const {
        return 0;
    }
#endif
    template <class View>
    forceinline bool
    ConstView<View>::assigned() const {
        return true;
    }
    template <class View>
    forceinline void
    ConstView<View>::subscribe(Space& home, Propagator& p, PropCond, bool schedule) {
        if (schedule)
            View::schedule(home, p, ME_GEN_ASSIGNED);
    }
    template <class View>
    forceinline void
    ConstView<View>::cancel(Space&, Propagator&, PropCond) {}
    template <class View>
    forceinline void
    ConstView<View>::reschedule(Space& home, Propagator& p, PropCond) {
        View::schedule(home, p, ME_GEN_ASSIGNED);
    }
    template <class View>
    forceinline void
    ConstView<View>::subscribe(Space&, Advisor&, bool) {}
    template <class View>
    forceinline void
    ConstView<View>::cancel(Space&, Advisor&, bool) {}
    template <class View>
    forceinline void
    ConstView<View>::schedule(Space& home, Propagator& p, ModEvent me) {
        View::schedule(home, p, me);
    }
    template <class View>
    forceinline ModEvent
    ConstView<View>::me(const ModEventDelta& med) {
        return View::me(med);
    }
    template <class View>
    forceinline ModEventDelta
    ConstView<View>::med(ModEvent me) {
        return View::med(me);
    }
    template <class View>
    forceinline ModEvent
    ConstView<View>::modification_event(const Delta& d) {
        (void)d;
        return ME_GEN_NONE;
    }
    template <class View>
    forceinline void
    ConstView<View>::update(Space&, ConstView<View>&) {}
    template <class View>
    forceinline bool
    ConstView<View>::operator<(const ConstView<View>&) const {
        return true;
    }

    /*
     * Variable view: contains a pointer to a variable implementation
     *
     */
    template <class Var>
    forceinline
    VarImpView<Var>::VarImpView()
        : x(nullptr) {}
    template <class Var>
    forceinline
    VarImpView<Var>::VarImpView(VarImpType* y)
        : x(y) {}
    template <class Var>
    forceinline void
    VarImpView<Var>::varimp(VarImpType* y) {
        x = y;
    }
    template <class Var>
    forceinline bool
    VarImpView<Var>::is_variable_derived() {
        return true;
    }
    template <class Var>
    forceinline Var::VarImpType*
    VarImpView<Var>::varimp() const {
        return x;
    }
    template <class Var>
    forceinline unsigned int
    VarImpView<Var>::degree() const {
        return x->degree();
    }
    template <class Var>
    forceinline double
    VarImpView<Var>::afc() const {
        return x->afc();
    }
#ifdef GECODE_HAS_CBS
    template <class Var>
    forceinline unsigned int
    VarImpView<Var>::id() const {
        return x->id();
    }
#endif
    template <class Var>
    forceinline bool
    VarImpView<Var>::assigned() const {
        return x->assigned();
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::subscribe(Space& home, Propagator& p, PropCond pc, bool schedule) {
        x->subscribe(home, p, pc, schedule);
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::cancel(Space& home, Propagator& p, PropCond pc) {
        x->cancel(home, p, pc);
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::reschedule(Space& home, Propagator& p, PropCond pc) {
        x->reschedule(home, p, pc);
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::subscribe(Space& home, Advisor& a, bool fail) {
        x->subscribe(home, a, fail);
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::cancel(Space& home, Advisor& a, bool fail) {
        x->cancel(home, a, fail);
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::schedule(Space& home, Propagator& p, ModEvent me) {
        VarImpType::schedule(home, p, me);
    }
    template <class Var>
    forceinline ModEvent
    VarImpView<Var>::me(const ModEventDelta& med) {
        return VarImpType::me(med);
    }
    template <class Var>
    forceinline ModEventDelta
    VarImpView<Var>::med(ModEvent me) {
        return VarImpType::med(me);
    }
    template <class Var>
    forceinline ModEvent
    VarImpView<Var>::modification_event(const Delta& d) {
        return VarImpType::modification_event(d);
    }
    template <class Var>
    forceinline void
    VarImpView<Var>::update(Space& home, VarImpView<Var>& y) {
        x = y.x->copy(home);
    }
    template <class Var>
    forceinline bool
    VarImpView<Var>::operator<(const VarImpView<Var>& y) const {
        return this->varimp() < y.varimp();
    }

    /*
     * Derived view: contain the base view from which they are derived
     *
     */

    template <class View>
    forceinline
    DerivedView<View>::DerivedView(const View& y)
        : x(y) {}

    template <class View>
    forceinline bool
    DerivedView<View>::is_variable_derived() {
        return View::is_variable_derived();
    }

    template <class View>
    forceinline View::VarImpType*
    DerivedView<View>::varimp() const {
        return x.varimp();
    }

    template <class View>
    forceinline View
    DerivedView<View>::base() const {
        return x;
    }

    template <class View>
    forceinline unsigned int
    DerivedView<View>::degree() const {
        return x.degree();
    }
    template <class View>
    forceinline double
    DerivedView<View>::afc() const {
        return x.afc();
    }
#ifdef GECODE_HAS_CBS
    template <class View>
    forceinline unsigned int
    DerivedView<View>::id() const {
        return x.id();
    }
#endif
    template <class View>
    forceinline bool
    DerivedView<View>::assigned() const {
        return x.assigned();
    }

    template <class View>
    forceinline void
    DerivedView<View>::schedule(Space& home, Propagator& p, ModEvent me) {
        return View::schedule(home, p, me);
    }
    template <class View>
    forceinline ModEvent
    DerivedView<View>::me(const ModEventDelta& med) {
        return View::me(med);
    }
    template <class View>
    forceinline ModEventDelta
    DerivedView<View>::med(ModEvent me) {
        return View::med(me);
    }

    template <class View>
    forceinline void
    DerivedView<View>::subscribe(Space& home, Propagator& p, PropCond pc, bool schedule) {
        x.subscribe(home, p, pc, schedule);
    }
    template <class View>
    forceinline void
    DerivedView<View>::cancel(Space& home, Propagator& p, PropCond pc) {
        x.cancel(home, p, pc);
    }
    template <class View>
    forceinline void
    DerivedView<View>::reschedule(Space& home, Propagator& p, PropCond pc) {
        x.reschedule(home, p, pc);
    }
    template <class View>
    forceinline void
    DerivedView<View>::subscribe(Space& home, Advisor& a, bool fail) {
        x.subscribe(home, a, fail);
    }
    template <class View>
    forceinline void
    DerivedView<View>::cancel(Space& home, Advisor& a, bool fail) {
        x.cancel(home, a, fail);
    }
    template <class View>
    forceinline ModEvent
    DerivedView<View>::modification_event(const Delta& d) {
        return View::modification_event(d);
    }
    template <class View>
    forceinline void
    DerivedView<View>::update(Space& home, DerivedView<View>& y) {
        x.update(home, y.x);
    }
    template <class View>
    forceinline bool
    DerivedView<View>::operator<(const DerivedView<View>& y) const {
        return base() < y.base();
    }

    /*
     * Tests whether two views are the same
     *
     */

    /// Test whether two views are the same
    template <class ViewA, class ViewB>
    forceinline bool
    operator==(const ConstView<ViewA>&, const ConstView<ViewB>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class Var, class View>
    forceinline bool
    operator==(const ConstView<View>&, const VarImpView<Var>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class ViewA, class ViewB>
    forceinline bool
    operator==(const ConstView<ViewA>&, const DerivedView<ViewB>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class Var, class View>
    forceinline bool
    operator==(const VarImpView<Var>&, const ConstView<View>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class Var, class View>
    forceinline bool
    operator==(const VarImpView<Var>&, const DerivedView<View>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class ViewX, class ViewY>
    forceinline bool
    operator==(const DerivedView<ViewX>&, const ConstView<ViewY>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class View, class Var>
    forceinline bool
    operator==(const DerivedView<View>&, const VarImpView<Var>&) {
        return false;
    }
    /// Test whether two views are not the same
    template <class ViewX, class ViewY>
    forceinline bool
    operator==(const DerivedView<ViewX>&, const DerivedView<ViewY>&) {
        return false;
    }
    /// Test whether two views are the same
    template <class VarX, class VarY>
    forceinline bool
    operator==(const VarImpView<VarX>& x, const VarImpView<VarY>& y) {
        return x.varimp() == y.varimp();
    }

    /// Test whether two views are not the same
    template <class ViewA, class ViewB>
    forceinline bool
    operator!=(const ConstView<ViewA>&, const ConstView<ViewB>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class Var, class View>
    forceinline bool
    operator!=(const ConstView<View>&, const VarImpView<Var>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class ViewA, class ViewB>
    forceinline bool
    operator!=(const ConstView<ViewA>&, const DerivedView<ViewB>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class Var, class View>
    forceinline bool
    operator!=(const VarImpView<Var>&, const ConstView<View>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class Var, class View>
    forceinline bool
    operator!=(const VarImpView<Var>&, const DerivedView<View>&) {
        return true;
    }
    /// Test whether two views are the same
    template <class ViewX, class ViewY>
    forceinline bool
    operator!=(const DerivedView<ViewX>&, const ConstView<ViewY>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class View, class Var>
    forceinline bool
    operator!=(const DerivedView<View>&, const VarImpView<Var>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class ViewX, class ViewY>
    forceinline bool
    operator!=(const DerivedView<ViewX>&, const DerivedView<ViewY>&) {
        return true;
    }
    /// Test whether two views are not the same
    template <class VarX, class VarY>
    forceinline bool
    operator!=(const VarImpView<VarX>& x, const VarImpView<VarY>& y) {
        return x.varimp() != y.varimp();
    }

    /*
     * Testing whether two views share the same variable
     *
     */

    template <class ViewA, class ViewB>
    forceinline bool
    shared(const ConstView<ViewA>&, const ConstView<ViewB>&) {
        return false;
    }
    template <class Var, class View>
    forceinline bool
    shared(const VarImpView<Var>&, const ConstView<View>&) {
        return false;
    }
    template <class ViewA, class ViewB>
    forceinline bool
    shared(const DerivedView<ViewA>&, const ConstView<ViewB>&) {
        return false;
    }
    template <class View, class Var>
    forceinline bool
    shared(const ConstView<View>&, const VarImpView<Var>&) {
        return false;
    }
    template <class ViewA, class ViewB>
    forceinline bool
    shared(const ConstView<ViewA>&, const DerivedView<ViewB>&) {
        return false;
    }
    template <class VarA, class VarB>
    forceinline bool
    shared(const VarImpView<VarA>& x, const VarImpView<VarB>& y) {
        return (static_cast<VarImpBase*>(x.varimp()) == static_cast<VarImpBase*>(y.varimp()));
    }
    template <class Var, class View>
    forceinline bool
    shared(const VarImpView<Var>& x, const DerivedView<View>& y) {
        return (View::is_variable_derived() &&
                static_cast<VarImpBase*>(x.varimp()) == static_cast<VarImpBase*>(y.varimp()));
    }
    template <class View, class Var>
    forceinline bool
    shared(const DerivedView<View>& x, const VarImpView<Var>& y) {
        return (View::is_variable_derived() &&
                static_cast<VarImpBase*>(x.varimp()) == static_cast<VarImpBase*>(y.varimp()));
    }
    template <class ViewA, class ViewB>
    forceinline bool
    shared(const DerivedView<ViewA>& x, const DerivedView<ViewB>& y) {
        return (ViewA::is_variable_derived() && ViewB::is_variable_derived() &&
                static_cast<VarImpBase*>(x.varimp()) == static_cast<VarImpBase*>(y.varimp()));
    }

} // namespace Gecode

// STATISTICS: kernel-var
