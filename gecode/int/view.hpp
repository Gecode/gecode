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

#include <iostream>

#if (-17 / 7) == -2
#define GECODE_INT_RND_TWDS_ZERO 1
#else
#define GECODE_INT_RND_TWDS_ZERO 0
#endif

namespace Gecode { namespace Int {

  /**
   * \brief Range iterator for integer views
   *
   * This class provides (by specialization) a range iterator
   * for all integer views.
   *
   * Note that this template class serves only as a specification
   * of the interface of the various specializations.
   *
   * \ingroup TaskActorInt
   */
  template<class View>
  class ViewRanges {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ViewRanges(void);
    /// Initialize with ranges for view \a x
    ViewRanges(const View& x);
    /// Initialize with ranges for view \a x
    void init(const View& x);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a range or done
    bool operator ()(void) const;
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    int min(void) const;
    /// Return largest value of range
    int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    unsigned int width(void) const;
    //@}
  };

  /**
   * \brief Value iterator for integer views
   *
   * This class provides a value iterator for all
   * integer views.
   *
   * \ingroup TaskActorInt
   */
  template<class View>
  class ViewValues : public Iter::Ranges::ToValues<ViewRanges<View> > {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ViewValues(void);
    /// Initialize with values for \a x
    ViewValues(const View& x);
    /// Initialize with values \a x
    void init(const View& x);
    //@}
  };

}}

#include <gecode/int/view/iter.hpp>

namespace Gecode {

  namespace Int {

    /**
     * \defgroup TaskActorIntView Integer views
     *
     * Integer propagators and branchers compute with integer views.
     * Integer views provide views on integer variable implementations,
     * integer constants, and also allow to scale, translate, and negate
     * variables. Additionally, a special Boolean view is provided that
     * offers convenient and efficient operations for Boolean (0/1)
     * views.
     * \ingroup TaskActorInt
     */

    /**
     * \brief Integer view for integer variables
     * \ingroup TaskActorIntView
     */
    class IntView : public VarViewBase<IntVarImp> {
    protected:
      using VarViewBase<IntVarImp>::varimp;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      IntView(void);
      /// Initialize from integer variable \a x
      IntView(const IntVar& x);
      /// Initialize from integer variable \a x
      IntView(IntVarImp* x);
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return median of domain (greatest element not greater than the median)
      int med(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;

      /// Return size (cardinality) of domain
      unsigned int size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;

      /// Test whether \a n is contained in domain
      bool in(int n) const;
      /// Test whether \a n is contained in domain
      bool in(double n) const;
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n);
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n);

      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n);

      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n);

      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n);

      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n);
      //@}

      /**
       * \name Domain update by iterator
       *
       * Views can be both updated by range and value iterators.
       * Value iterators do not need to be strict in that the same value
       * is allowed to occur more than once in the iterated sequence.
       *
       * The argument \a depends must be true, if the iterator
       * passed as argument depends on the view on which the operation
       * is invoked. In this case, the view is only updated after the
       * iterator has been consumed. Otherwise, the domain might be updated
       * concurrently while following the iterator.
       *
       */
      //@{
      /// Replace domain by ranges described by \a i
      template<class I>
      ModEvent narrow_r(Space& home, I& i, bool depends=true);
      /// Intersect domain with ranges described by \a i
      template<class I>
      ModEvent inter_r(Space& home, I& i, bool depends=true);
      /// Remove from domain the ranges described by \a i
      template<class I>
      ModEvent minus_r(Space& home, I& i, bool depends=true);
      /// Replace domain by values described by \a i
      template<class I>
      ModEvent narrow_v(Space& home, I& i, bool depends=true);
      /// Intersect domain with values described by \a i
      template<class I>
      ModEvent inter_v(Space& home, I& i, bool depends=true);
      /// Remove from domain the values described by \a i
      template<class I>
      ModEvent minus_v(Space& home, I& i, bool depends=true);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      //@}

      /// \name View-dependent propagator support
      //@{
      /// Translate modification event \a me to modification event delta for view
      static ModEventDelta med(ModEvent me);
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, IntView& x);
      //@}
    };

    /**
     * \brief Print integer variable view
     * \relates Gecode::Int::IntView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const IntView& x);

  }

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for IntView
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::IntView> {
  public:
    /// The variable type of an IntView
    typedef Int::IntVarImp VarImp;
  };

  /**
   * \brief Traits class mapping variables to views
   *
   * This class specializes the VarViewTraits for integer variables.
   */
  template<>
  class VarViewTraits<IntVar> {
  public:
    /// The variable type of an IntView
    typedef Int::IntView View;
  };

  namespace Int {

    /**
     * \brief Minus integer view
     *
     * A minus integer view \f$m\f$ for an integer view \f$x\f$ provides
     * operations such that \f$m\f$ behaves as \f$-x\f$.
     * \ingroup TaskActorIntView
     */
    class MinusView : public DerivedViewBase<IntView> {
    protected:
      using DerivedViewBase<IntView>::view;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      MinusView(void);
      /// Initialize with integer view \a x
      MinusView(const IntView& x);
      /// Initialize with integer view \a x
      void init(const IntView& x);
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return median of domain
      int med(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;

      /// Return size (cardinality) of domain
      unsigned int size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;

      /// Test whether \a n is contained in domain
      bool in(int n) const;
      /// Test whether \a n is contained in domain
      bool in(double n) const;
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n);
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n);
      //@}

      /**
       * \name Domain update by iterator
       *
       * Views can be both updated by range and value iterators.
       * Value iterators do not need to be strict in that the same value
       * is allowed to occur more than once in the iterated sequence.
       *
       * The argument \a depends must be true, if the iterator
       * passed as argument depends on the view on which the operation
       * is invoked. In this case, the view is only updated after the
       * iterator has been consumed. Otherwise, the domain might be updated
       * concurrently while following the iterator.
       *
       */
      //@{
      /// Replace domain by ranges described by \a i
      template<class I>
      ModEvent narrow_r(Space& home, I& i, bool depends=true);
      /// Intersect domain with ranges described by \a i
      template<class I>
      ModEvent inter_r(Space& home, I& i, bool depends=true);
      /// Remove from domain the ranges described by \a i
      template<class I>
      ModEvent minus_r(Space& home, I& i, bool depends=true);
      /// Replace domain by values described by \a i
      template<class I>
      ModEvent narrow_v(Space& home, I& i, bool depends=true);
      /// Intersect domain with values described by \a i
      template<class I>
      ModEvent inter_v(Space& home, I& i, bool depends=true);
      /// Remove from domain the values described by \a i
      template<class I>
      ModEvent minus_v(Space& home, I& i, bool depends=true);
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
       * In case \a process is false, the propagator is just subscribed but
       * not processed for execution (this must be used when creating
       * subscriptions during propagation).
       */
      void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
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
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, MinusView& x);
      //@}
    };

    /**
     * \brief Print integer minus view
     * \relates Gecode::Int::MinusView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const MinusView& x);

  }

  /** \name View comparison
   *  \relates Gecode::Int::MinusView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Int::MinusView& x, const Int::MinusView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Int::MinusView& x, const Int::MinusView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for MinusView.
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::MinusView> {
  public:
    /// The variable type of a MinusView
    typedef Int::IntVarImp VarImp;
  };


  namespace Int {

    /**
     * \brief Offset integer view
     *
     * An offset integer view \f$o\f$ for an integer view \f$x\f$ and
     * an integer \f$c\f$ provides operations such that \f$o\f$
     * behaves as \f$x+c\f$.
     * \ingroup TaskActorIntView
     */
    class OffsetView : public DerivedViewBase<IntView> {
    protected:
      /// Offset
      int c;
      using DerivedViewBase<IntView>::view;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      OffsetView(void);
      /// Initialize with integer view \a x and offset \a c
      OffsetView(const IntView& x, int c);
      /// Initialize with integer view \a x and offset \a c
      void init(const IntView& x, int c);
      /// Return offset
      int offset(void) const;
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return median of domain (greatest element not greater than the median)
      int med(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;

      /// Return size (cardinality) of domain
      unsigned int size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;

      /// Test whether \a n is contained in domain
      bool in(int n) const;
      /// Test whether \a n is contained in domain
      bool in(double n) const;
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n);
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n);
      //@}

      /**
       * \name Domain update by iterator
       *
       * Views can be both updated by range and value iterators.
       * Value iterators do not need to be strict in that the same value
       * is allowed to occur more than once in the iterated sequence.
       *
       * The argument \a depends must be true, if the iterator
       * passed as argument depends on the view on which the operation
       * is invoked. In this case, the view is only updated after the
       * iterator has been consumed. Otherwise, the domain might be updated
       * concurrently while following the iterator.
       *
       */
      //@{
      /// Replace domain by ranges described by \a i
      template<class I>
      ModEvent narrow_r(Space& home, I& i, bool depends=true);
      /// Intersect domain with ranges described by \a i
      template<class I>
      ModEvent inter_r(Space& home, I& i, bool depends=true);
      /// Remove from domain the ranges described by \a i
      template<class I>
      ModEvent minus_r(Space& home, I& i, bool depends=true);
      /// Replace domain by values described by \a i
      template<class I>
      ModEvent narrow_v(Space& home, I& i, bool depends=true);
      /// Intersect domain with values described by \a i
      template<class I>
      ModEvent inter_v(Space& home, I& i, bool depends=true);
      /// Remove from domain the values described by \a i
      template<class I>
      ModEvent minus_v(Space& home, I& i, bool depends=true);
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
       * In case \a process is false, the propagator is just subscribed but
       * not processed for execution (this must be used when creating
       * subscriptions during propagation).
       */
      void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
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
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, OffsetView& x);
      //@}
    };

    /**
     * \brief Print integer offset view
     * \relates Gecode::Int::OffsetView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const OffsetView& x);

  }

  /** \name View comparison
   *  \relates Gecode::Int::OffsetView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Int::OffsetView& x, const Int::OffsetView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Int::OffsetView& x, const Int::OffsetView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for OffsetView.
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::OffsetView> {
  public:
    /// The variable type of an OffsetView
    typedef Int::IntVarImp VarImp;
  };



  namespace Int {

    /**
     * \brief Scale integer view (template)
     *
     * A scale integer view \f$s\f$ for an integer view \f$x\f$ and
     * a non-negative integer \f$a\f$ provides operations such that \f$s\f$
     * behaves as \f$a\cdot x\f$.
     *
     * The precision of a scale integer view is defined by the value types
     * \a Val and \a UnsVal. \a Val can be either \c int or \c double where
     * \a UnsVal must be the unsigned variant of \a Val (that is, if \a Val
     * is \c int, then \a UnsVal must be \c unsigned \c int; if \a Val is
     * \c double, then \a UnsVal must be \c double as well). The range which is
     * allowed for the two types is defined by the values in
     * Gecode::Limits.
     *
     * Note that scale integer views currently do not provide operations
     * for updating domains by range iterators.
     *
     * The template is not to be used directly (as it is very clumsy). Use
     * the following instead:
     *  - IntScaleView for scale views with integer precision
     *  - DoubleScaleView for scale views with double precision
     *
     * \ingroup TaskActorIntView
     */
    template<class Val, class UnsVal>
    class ScaleView : public DerivedViewBase<IntView> {
    protected:
      /// Scale factor
      int a;
      using DerivedViewBase<IntView>::view;

      /// \name Support functions for division
      //@{
      /// Return \f$\lfloor x/a\rfloor\f$
      int floor_div(double x) const;
      /// Return \f$\lceil x/a\rceil\f$
      int ceil_div(double x) const;
      /// Return \f$x/a\f$ and set \a exact to true if \a x is multiple of \a a
      int exact_div(double x, bool& exact) const;
#if GECODE_INT_RND_TWDS_ZERO
      /// Return \f$\lfloor x/a\rfloor\f$
      int floor_div(int) const;
      /// Return \f$\lceil x/a\rceil\f$
      int ceil_div(int) const;
      /// Return \f$x/a\f$ and set \a exact to true if \a x is multiple of \a a
      int exact_div(int, bool&) const;
#endif
      //@}

    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ScaleView(void);
      /// Initialize as \f$b\cdot y\f$
      ScaleView(int b, const IntView& y);
      /// Initialize as \f$b\cdot y\f$
      void init(int b, const IntView& y);
      /// Return scale factor of scale view
      int scale(void) const;
      //@}


      /// \name Value access
      //@{
      /// Return minimum of domain
      Val min(void) const;
      /// Return maximum of domain
      Val max(void) const;
      /// Return median of domain (greatest element not greater than the median)
      Val med(void) const;
      /// Return assigned value (only if assigned)
      Val val(void) const;

      /// Return size (cardinality) of domain
      UnsVal size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      UnsVal width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      UnsVal regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      UnsVal regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a n is contained in domain
      bool in(Val n) const;
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, Val n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, Val n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, Val n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, Val n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, Val n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, Val n);
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
       * In case \a process is false, the propagator is just subscribed but
       * not processed for execution (this must be used when creating
       * subscriptions during propagation).
       */
      void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
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
      /// Return minimum value just pruned
      Val min(const Delta& d) const;
      /// Return maximum value just pruned
      Val max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, ScaleView<Val,UnsVal>& x);
      //@}
    };

    /**
     * \brief Integer-precision integer scale view
     * \ingroup TaskActorIntView
     */
    typedef ScaleView<int,unsigned int> IntScaleView;

    /**
     * \brief Double-precision integer scale view
     * \ingroup TaskActorIntView
     */
    typedef ScaleView<double,double> DoubleScaleView;

    /**
     * \brief Print integer-precision integer scale view
     * \relates Gecode::Int::ScaleView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const IntScaleView& x);

    /**
     * \brief Print double-precision integer scale view
     * \relates Gecode::Int::ScaleView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const DoubleScaleView& x);

  }

  /** \name View comparison
   *  \relates Gecode::Int::ScaleView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  template<class Val, class UnsVal>
  bool same(const Int::ScaleView<Val,UnsVal>& x,
            const Int::ScaleView<Val,UnsVal>& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  template<class Val, class UnsVal>
  bool before(const Int::ScaleView<Val,UnsVal>& x,
              const Int::ScaleView<Val,UnsVal>& y);
  //@}


  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for ScaleView.
   * \ingroup TaskActorIntView
   */
  template<class Val, class UnsVal>
  class ViewVarImpTraits<Int::ScaleView<Val,UnsVal> > {
  public:
    /// The variable type of a ScaleView
    typedef Int::IntVarImp VarImp;
  };



  namespace Int {

    /**
     * \brief Constant integer view
     *
     * A constant integer view \f$x\f$ for an integer \f$c\f$ provides
     * operations such that \f$x\f$ behaves as a view assigned to \f$c\f$.
     * \ingroup TaskActorIntView
     */
    class ConstIntView : public ConstViewBase {
    protected:
      int x;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ConstIntView(void);
      /// Initialize with integer value \a n
      ConstIntView(int n);
      /// Initialize with integer value \a n
      void init(int n);
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return median of domain (greatest element not greater than the median)
      int med(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;

      /// Return size (cardinality) of domain
      unsigned int size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;

      /// Test whether \a n is contained in domain
      bool in(int n) const;
      /// Test whether \a n is contained in domain
      bool in(double n) const;
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n);
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n);
      //@}

      /**
       * \name Domain update by iterator
       *
       * Views can be both updated by range and value iterators.
       * Value iterators do not need to be strict in that the same value
       * is allowed to occur more than once in the iterated sequence.
       *
       * The argument \a depends must be true, if the iterator
       * passed as argument depends on the view on which the operation
       * is invoked. In this case, the view is only updated after the
       * iterator has been consumed. Otherwise, the domain might be updated
       * concurrently while following the iterator.
       *
       */
      //@{
      /// Replace domain by ranges described by \a i
      template<class I>
      ModEvent narrow_r(Space& home, I& i, bool depends=true);
      /// Intersect domain with ranges described by \a i
      template<class I>
      ModEvent inter_r(Space& home, I& i, bool depends=true);
      /// Remove from domain the ranges described by \a i
      template<class I>
      ModEvent minus_r(Space& home, I& i, bool depends=true);
      /// Replace domain by values described by \a i
      template<class I>
      ModEvent narrow_v(Space& home, I& i, bool depends=true);
      /// Intersect domain with values described by \a i
      template<class I>
      ModEvent inter_v(Space& home, I& i, bool depends=true);
      /// Remove from domain the values described by \a i
      template<class I>
      ModEvent minus_v(Space& home, I& i, bool depends=true);
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
       * In case \a process is false, the propagator is just subscribed but
       * not processed for execution (this must be used when creating
       * subscriptions during propagation).
       */
      void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
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
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, ConstIntView& x);
      //@}
    };

    /**
     * \brief Print integer constant integer view
     * \relates Gecode::Int::ConstIntView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const ConstIntView& x);

  }

  /**
   * \name View comparison
   * \relates Gecode::Int::ConstIntView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Int::ConstIntView& x, const Int::ConstIntView& y);
  /// Test whether view \a x is before \a y (arbitrary order)
  bool before(const Int::ConstIntView& x, const Int::ConstIntView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for ConstIntView.
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::ConstIntView> {
  public:
    /// The variable type of a ConstIntView
    typedef VarImpBase VarImp;
  };

  namespace Int {

    /**
     * \brief Zero integer view
     *
     * A zero integer view \f$x\f$ for provides
     * operations such that \f$x\f$ behaves as a view assigned to \f$0\f$.
     * \ingroup TaskActorIntView
     */
    class ZeroIntView : public ConstViewBase {
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ZeroIntView(void);
      /// Initialize
      void init(void);
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return median of domain (greatest element not greater than the median)
      int med(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;

      /// Return size (cardinality) of domain
      unsigned int size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;

      /// Test whether \a n is contained in domain
      bool in(int n) const;
      /// Test whether \a n is contained in domain
      bool in(double n) const;
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n);
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n);
      //@}

      /**
       * \name Domain update by iterator
       *
       * Views can be both updated by range and value iterators.
       * Value iterators do not need to be strict in that the same value
       * is allowed to occur more than once in the iterated sequence.
       *
       * The argument \a depends must be true, if the iterator
       * passed as argument depends on the view on which the operation
       * is invoked. In this case, the view is only updated after the
       * iterator has been consumed. Otherwise, the domain might be updated
       * concurrently while following the iterator.
       *
       */
      //@{
      /// Replace domain by ranges described by \a i
      template<class I>
      ModEvent narrow_r(Space& home, I& i, bool depends=true);
      /// Intersect domain with ranges described by \a i
      template<class I>
      ModEvent inter_r(Space& home, I& i, bool depends=true);
      /// Remove from domain the ranges described by \a i
      template<class I>
      ModEvent minus_r(Space& home, I& i, bool depends=true);
      /// Replace domain by values described by \a i
      template<class I>
      ModEvent narrow_v(Space& home, I& i, bool depends=true);
      /// Intersect domain with values described by \a i
      template<class I>
      ModEvent inter_v(Space& home, I& i, bool depends=true);
      /// Remove from domain the values described by \a i
      template<class I>
      ModEvent minus_v(Space& home, I& i, bool depends=true);
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
       * In case \a process is false, the propagator is just subscribed but
       * not processed for execution (this must be used when creating
       * subscriptions during propagation).
       */
      void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
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
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, ZeroIntView& x);
      //@}
    };

    /**
     * \brief Print integer zero view
     * \relates Gecode::Int::ScaleView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const ZeroIntView& x);

  }

  /**
   * \name View comparison
   * \relates Gecode::Int::ZeroIntView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Int::ZeroIntView& x, const Int::ZeroIntView& y);
  /// Test whether view \a x is before \a y (arbitrary order)
  bool before(const Int::ZeroIntView& x, const Int::ZeroIntView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for ZeroIntView.
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::ZeroIntView> {
  public:
    /// The variable type of a ZeroIntView
    typedef VarImpBase VarImp;
  };

  namespace Int {

    /**
     * \brief Boolean view for Boolean variables
     *
     * Provides convenient and efficient operations for Boolean views.
     * \ingroup TaskActorIntView
     */
    class BoolView : public VarViewBase<BoolVarImp> {
    protected:
      using VarViewBase<BoolVarImp>::varimp;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      BoolView(void);
      /// Initialize from Boolean variable \a x
      BoolView(const BoolVar& x);
      /// Initialize from Boolean variable \a x
      BoolView(BoolVarImp* x);
      //@}

      /// \name Domain status access
      //@{
      /// How many bits does the status have
      static const int BITS = BoolVarImp::BITS;
      /// Status of domain assigned to zero
      static const BoolStatus ZERO = BoolVarImp::ZERO;
      /// Status of domain assigned to one
      static const BoolStatus ONE  = BoolVarImp::ONE;
      /// Status of domain not yet assigned
      static const BoolStatus NONE = BoolVarImp::NONE;
      /// Return current domain status
      BoolStatus status(void) const;
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return median of domain (greatest element not greater than the median)
      int med(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;

      /// Return size (cardinality) of domain
      unsigned int size(void) const;
      /// Return width of domain (distance between maximum and minimum)
      unsigned int width(void) const;
      /// Return regret of domain minimum (distance to next larger value)
      unsigned int regret_min(void) const;
      /// Return regret of domain maximum (distance to next smaller value)
      unsigned int regret_max(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether domain is a range
      bool range(void) const;
      /// Test whether view is assigned
      bool assigned(void) const;

      /// Test whether \a n is contained in domain
      bool in(int n) const;
      /// Test whether \a n is contained in domain
      bool in(double n) const;
      //@}

      /// \name Boolean domain tests
      //@{
      /// Test whether view is assigned to be zero
      bool zero(void) const;
      /// Test whether view is assigned to be one
      bool one(void) const;
      /// Test whether view is not yet assigned
      bool none(void) const;
      //@}

      /// \name Boolean assignment operations
      //@{
      /// Try to assign view to one
      ModEvent one(Space& home);
      /// Try to assign view to zero
      ModEvent zero(Space& home);
      /// Assign not yet assigned view to one
      ModEvent one_none(Space& home);
      /// Assign not yet assigned view to zero
      ModEvent zero_none(Space& home);
      //@}

      /// \name Domain update by value
      //@{
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, int n);
      /// Restrict domain values to be less or equal than \a n
      ModEvent lq(Space& home, double n);

      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, int n);
      /// Restrict domain values to be less than \a n
      ModEvent le(Space& home, double n);

      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, int n);
      /// Restrict domain values to be greater or equal than \a n
      ModEvent gq(Space& home, double n);

      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, int n);
      /// Restrict domain values to be greater than \a n
      ModEvent gr(Space& home, double n);

      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, int n);
      /// Restrict domain values to be different from \a n
      ModEvent nq(Space& home, double n);

      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, int n);
      /// Restrict domain values to be equal to \a n
      ModEvent eq(Space& home, double n);
      //@}

      /**
       * \name Domain update by iterator
       *
       * Views can be both updated by range and value iterators.
       * Value iterators do not need to be strict in that the same value
       * is allowed to occur more than once in the iterated sequence.
       *
       * The argument \a depends must be true, if the iterator
       * passed as argument depends on the view on which the operation
       * is invoked. In this case, the view is only updated after the
       * iterator has been consumed. Otherwise, the domain might be updated
       * concurrently while following the iterator.
       *
       */
      //@{
      /// Replace domain by ranges described by \a i
      template<class I>
      ModEvent narrow_r(Space& home, I& i, bool depends=true);
      /// Intersect domain with ranges described by \a i
      template<class I>
      ModEvent inter_r(Space& home, I& i, bool depends=true);
      /// Remove from domain the ranges described by \a i
      template<class I>
      ModEvent minus_r(Space& home, I& i, bool depends=true);
      /// Replace domain by values described by \a i
      template<class I>
      ModEvent narrow_v(Space& home, I& i, bool depends=true);
      /// Intersect domain with values described by \a i
      template<class I>
      ModEvent inter_v(Space& home, I& i, bool depends=true);
      /// Remove from domain the values described by \a i
      template<class I>
      ModEvent minus_v(Space& home, I& i, bool depends=true);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      /// Test whether a view has been assigned to zero
      static bool zero(const Delta& d);
      /// Test whether a view has been assigned to one
      static bool one(const Delta& d);
      //@}

      /// \name View-dependent propagator support
      //@{
      /// Translate modification event \a me to modification event delta for view
      static ModEventDelta med(ModEvent me);
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, BoolView& x);
      //@}
    };

    /**
     * \brief Print Boolean view
     * \relates Gecode::Int::BoolView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const BoolView& x);

  }

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for BoolView
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::BoolView> {
  public:
    /// The variable type of a BoolView
    typedef Int::BoolVarImp VarImp;
  };

  /**
   * \brief Traits class mapping variables to views
   *
   * This class specializes the VarViewTraits for Boolean variables.
   */
  template<>
  class VarViewTraits<BoolVar> {
  public:
    /// The variable type of an IntView
    typedef Int::BoolView View;
  };

  namespace Int {

    /**
     * \brief Negated Boolean view
     *
     * A negated Boolean view \f$n\f$ for a Boolean view \f$b\f$
     * provides operations such that \f$n\f$
     * behaves as \f$\neg b\f$.
     * \ingroup TaskActorIntView
     */
    class NegBoolView : public DerivedViewBase<BoolView> {
    protected:
      using DerivedViewBase<BoolView>::view;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      NegBoolView(void);
      /// Initialize with Boolean view \a b
      NegBoolView(const BoolView& b);
      /// Initialize with Boolean view \a b
      void init(const BoolView& b);
      //@}

      /// \name Domain status access
      //@{
      /// How many bits does the status have
      static const int BITS = BoolView::BITS;
      /// Status of domain assigned to zero
      static const BoolStatus ZERO = BoolView::ONE;
      /// Status of domain assigned to one
      static const BoolStatus ONE  = BoolView::ZERO;
      /// Status of domain not yet assigned
      static const BoolStatus NONE = BoolView::NONE;
      /// Return current domain status
      BoolStatus status(void) const;
      //@}

      /// \name Boolean domain tests
      //@{
      /// Test whether view is assigned to be zero
      bool zero(void) const;
      /// Test whether view is assigned to be one
      bool one(void) const;
      /// Test whether view is not yet assigned
      bool none(void) const;
      //@}

      /// \name Boolean assignment operations
      //@{
      /// Try to assign view to one
      ModEvent one(Space& home);
      /// Try to assign view to zero
      ModEvent zero(Space& home);
      /// Assign not yet assigned view to one
      ModEvent one_none(Space& home);
      /// Assign not yet assigned view to zero
      ModEvent zero_none(Space& home);
      //@}

      /// \name Value access
      //@{
      /// Return minimum of domain
      int min(void) const;
      /// Return maximum of domain
      int max(void) const;
      /// Return assigned value (only if assigned)
      int val(void) const;
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
       * In case \a process is false, the propagator is just subscribed but
       * not processed for execution (this must be used when creating
       * subscriptions during propagation).
       */
      void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
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
      /// Return minimum value just pruned
      int min(const Delta& d) const;
      /// Return maximum value just pruned
      int max(const Delta& d) const;
      /// Test whether arbitrary values got pruned
      bool any(const Delta& d) const;
      /// Test whether a view has been assigned to zero
      static bool zero(const Delta& d);
      /// Test whether a view has been assigned to one
      static bool one(const Delta& d);
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, NegBoolView& x);
      //@}
    };

    /**
     * \brief Print negated Boolean view
     * \relates Gecode::Int::NegBoolView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const NegBoolView& x);


  }

  /** \name View comparison
   *  \relates Gecode::Int::NegBoolView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Int::NegBoolView& x, const Int::NegBoolView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Int::NegBoolView& x, const Int::NegBoolView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for NegBoolView.
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewVarImpTraits<Int::NegBoolView> {
  public:
    /// The variable type of a NegBoolView
    typedef Int::BoolVarImp VarImp;
  };

}

#include <gecode/int/var/int.hpp>
#include <gecode/int/var/bool.hpp>

#include <gecode/int/view/int.hpp>

#include <gecode/int/view/constint.hpp>
#include <gecode/int/view/zero.hpp>
#include <gecode/int/view/minus.hpp>
#include <gecode/int/view/offset.hpp>
#include <gecode/int/view/scale.hpp>

#include <gecode/int/view/bool.hpp>

#include <gecode/int/view/neg-bool.hpp>

#include <gecode/int/view/print.hpp>
#include <gecode/int/var/print.hpp>

namespace Gecode {

  namespace Int {

    /**
     * \defgroup TaskActorIntTest Testing relations between integer views
     * \ingroup TaskActorInt
     */

    //@{
    /// Result of testing relation
    enum RelTest {
      RT_FALSE = 0, ///< Relation does not hold
      RT_MAYBE = 1, ///< Relation may hold or not
      RT_TRUE  = 2  ///< Relation does hold
    };

    /// Test whether views \a x and \a y are equal (use bounds information)
    template<class View> RelTest rtest_eq_bnd(View x, View y);
    /// Test whether views \a x and \a y are equal (use full domain information)
    template<class View> RelTest rtest_eq_dom(View x, View y);
    /// Test whether view \a x and integer \a n are equal (use bounds information)
    template<class View> RelTest rtest_eq_bnd(View x, int n);
    /// Test whether view \a x and integer \a n are equal (use full domain information)
    template<class View> RelTest rtest_eq_dom(View x, int n);

    /// Test whether views \a x and \a y are different (use bounds information)
    template<class View> RelTest rtest_nq_bnd(View x, View y);
    /// Test whether views \a x and \a y are different (use full domain information)
    template<class View> RelTest rtest_nq_dom(View x, View y);
    /// Test whether view \a x and integer \a n are different (use bounds information)
    template<class View> RelTest rtest_nq_bnd(View x, int n);
    /// Test whether view \a x and integer \a n are different (use full domain information)
    template<class View> RelTest rtest_nq_dom(View x, int n);

    /// Test whether view \a x is less or equal than view \a y
    template<class View> RelTest rtest_lq(View x, View y);
    /// Test whether view \a x is less or equal than integer \a n
    template<class View> RelTest rtest_lq(View x, int n);

    /// Test whether view \a x is less than view \a y
    template<class View> RelTest rtest_le(View x, View y);
    /// Test whether view \a x is less than integer \a n
    template<class View> RelTest rtest_le(View x, int n);

    /// Test whether view \a x is greater or equal than view \a y
    template<class View> RelTest rtest_gq(View x, View y);
    /// Test whether view \a x is greater or equal than integer \a n
    template<class View> RelTest rtest_gq(View x, int n);

    /// Test whether view \a x is greater than view \a y
    template<class View> RelTest rtest_gr(View x, View y);
    /// Test whether view \a x is greater than integer \a n
    template<class View> RelTest rtest_gr(View x, int n);
    //@}


    /**
     * \brief Boolean tests
     *
     */
    enum BoolTest {
      BT_NONE, ///< No sharing
      BT_SAME, ///< Same variable
      BT_COMP  ///< Same variable but complement
    };

    /**
     * \name Test sharing between Boolean and negated Boolean views
     * \relates BoolView NegBoolView
     */
    //@{
    /// Test whether views \a b0 and \a b1 are the same
    BoolTest bool_test(const BoolView& b0, const BoolView& b1);
    /// Test whether views \a b0 and \a b1 are complementary
    BoolTest bool_test(const BoolView& b0, const NegBoolView& b1);
    /// Test whether views \a b0 and \a b1 are complementary
    BoolTest bool_test(const NegBoolView& b0, const BoolView& b1);
    /// Test whether views \a b0 and \a b1 are the same
    BoolTest bool_test(const NegBoolView& b0, const NegBoolView& b1);
    //@}

  }

}

#include <gecode/int/view/rel-test.hpp>
#include <gecode/int/view/bool-test.hpp>

// STATISTICS: int-var
