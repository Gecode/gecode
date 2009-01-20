/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

namespace Gecode {

  namespace Set {

    /**
     * \defgroup TaskActorSetView Set views
     *
     * Set propagators and branchings compute with set views.
     * Set views provide views on set variable implementations,
     * set constants, and integer variable implementations.
     * \ingroup TaskActorSet
     */

    /**
     * \brief %Set view for set variables
     * \ingroup TaskActorSetView
     */

    class SetView : public VarViewBase<SetVarImp> {
    protected:
      using VarViewBase<SetVarImp>::varimp;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      SetView(void);
      /// Initialize from set variable \a x
      SetView(const SetVar& x);
      /// Initialize from set variable \a x
      SetView(SetVarImp* x);
      /// Initialize from specification
      SetView(Space& home, const Reflection::VarMap& vars,
              Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{

      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
      //@}

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, SetView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}

    };

    /**
     * \brief Print set variable view
     * \relates Gecode::Set::SetView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const SetView& x);

  }

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for SetView.
   * \ingroup TaskActorSetView
   */
  template<>
  class ViewVarImpTraits<Set::SetView> {
  public:
    /// The variable type of a SetView
    typedef Set::SetVarImp VarImp;
  };


  /**
   * \brief Traits class mapping variables to views
   *
   * This class specializes the VarViewTraits for set variables.
   */
  template<>
  class VarViewTraits<SetVar> {
  public:
    /// The variable type of an IntView
    typedef Set::SetView View;
  };

  // Forward declarations for friends
  namespace Set { class ConstantView;  }
  bool same(const Set::ConstantView&, const Set::ConstantView&);
  bool before(const Set::ConstantView&, const Set::ConstantView&);

  namespace Set {

    /**
     * \brief Constant view
     *
     * A constant set view \f$x\f$ for a set \f$s\f$ provides operations such
     * that \f$x\f$ behaves like \f$s\f$.
     * \ingroup TaskActorSetView
     */
    class ConstantView : public ConstViewBase {
      friend class LubRanges<ConstantView>;
      friend class GlbRanges<ConstantView>;
      friend bool Gecode::same(const Gecode::Set::ConstantView&,
                               const Gecode::Set::ConstantView&);
      friend bool Gecode::before(const Gecode::Set::ConstantView&,
                                 const Gecode::Set::ConstantView&);
    private:
      int *ranges;
      unsigned int size;
      unsigned int domSize;
      /// Initialize with constant \a s
      void init(Space& home, const IntSet& s);
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ConstantView(void);
      /// Construct with \a s as the domain
      ConstantView(Space& home, const IntSet& s);
      /// Initialize from specification
      ConstantView(Space& home, const Reflection::VarMap& vars,
                   Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{
      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
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

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, ConstantView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}

    };

    /**
     * \brief Print constant set view
     * \relates Gecode::Set::ConstantView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const ConstantView& x);
  }

  /** \name View comparison
   *  \relates Gecode::Set::ConstantView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Set::ConstantView& x, const Set::ConstantView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Set::ConstantView& x, const Set::ConstantView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for ConstantView.
   * \ingroup TaskActorSetView
   */
  template<>
  class ViewVarImpTraits<Set::ConstantView> {
  public:
    /// The variable type of a ConstantView
    typedef VarImpBase VarImp;
  };

  namespace Set {

    /**
     * \brief Constant view for the empty set
     *
     * A constant set view \f$x\f$ for the empty set provides operations such
     * that \f$x\f$ behaves like the empty set.
     * \ingroup TaskActorSetView
     */

    class EmptyView : public ConstViewBase {
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      EmptyView(void);
      /// Initialize from specification
      EmptyView(Space& home, const Reflection::VarMap& vars,
                Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{

      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
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

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, EmptyView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}

    };

    /**
     * \brief Print empty set view
     * \relates Gecode::Set::EmptyView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const EmptyView& x);

  }

  /** \name View comparison
   *  \relates Gecode::Set::EmptyView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Set::EmptyView& x, const Set::EmptyView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Set::EmptyView& x, const Set::EmptyView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for EmptyView.
   * \ingroup TaskActorSetView
   */
  template<>
  class ViewVarImpTraits<Set::EmptyView> {
  public:
    /// The variable type of an EmptyView
    typedef VarImpBase VarImp;
  };

  namespace Set {


    /**
     * \brief Constant view for the universe
     *
     * A constant set view \f$x\f$ for the universe provides operations such
     * that \f$x\f$ behaves like the universe.
     * \ingroup TaskActorSetView
     */

    class UniverseView : public ConstViewBase {
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      UniverseView(void);
      /// Initialize from specification
      UniverseView(Space& home, const Reflection::VarMap& vars,
                   Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{

      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
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

      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, UniverseView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}

    };

    /**
     * \brief Print universe set view
     * \relates Gecode::Set::UniverseView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const UniverseView& x);

  }

  /** \name View comparison
   *  \relates Gecode::Set::UniverseView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Set::UniverseView& x, const Set::UniverseView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Set::UniverseView& x, const Set::UniverseView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for UniverseView.
   * \ingroup TaskActorSetView
   */
  template<>
  class ViewVarImpTraits<Set::UniverseView> {
  public:
    /// The variable type of a UniverseView
    typedef VarImpBase VarImp;
  };

  namespace Set {



    /**
     * \brief Singleton set view
     *
     * A singleton set view \f$s\f$ for an integer view \f$x\f$ provides
     * operations such that \f$s\f$ behaves like the singleton set \f$\{x\}\f$.
     * \ingroup TaskActorSetView
     */

    class SingletonView :
      public DerivedViewBase<Gecode::Int::IntView> {
    protected:
      using DerivedViewBase<Gecode::Int::IntView>::view;

      /// Convert set variable PropCond \a pc to a PropCond for integer variables
      static PropCond pc_settoint(PropCond pc);
      /// Convert integer variable ModEvent \a me to a ModEvent for set variables
      static ModEvent me_inttoset(ModEvent me);
      /// Convert set variable ModEvent \a me to a ModEvent for integer variables
      static ModEvent me_settoint(ModEvent me);

    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      SingletonView(void);
      /// Initialize with integer view \a x
      SingletonView(Gecode::Int::IntView& x);
      /// Initialize from specification
      SingletonView(Space& home, const Reflection::VarMap& vars,
                    Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{

      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
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


      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, SingletonView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}

    };

    /**
     * \brief Print singleton set view
     * \relates Gecode::Set::SingletonView
     */
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const SingletonView& x);

  }

  /** \name View comparison
   *  \relates Gecode::Set::SingletonView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  bool same(const Set::SingletonView& x, const Set::SingletonView& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  bool before(const Set::SingletonView& x, const Set::SingletonView& y);
  //@}

  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for SingletonView.
   * \ingroup TaskActorSetView
   */
  template<>
  class ViewVarImpTraits<Set::SingletonView> {
  public:
    /// The variable type of a SingletonView
    typedef Int::IntVarImp VarImp;
  };

  namespace Set {


    /**
     * \brief Complement set view
     *
     * A complement set view \f$s\f$ for a set view \f$t\f$ provides
     * operations such that \f$s\f$ behaves like the complement of \f$\{t\}\f$.
     * The complement is defined in terms of the set universe.
     * \ingroup TaskActorSetView
     */

    template <class View>
    class ComplementView
      : public DerivedViewBase<View> {
    protected:
      using DerivedViewBase<View>::view;

    public:
      /// Negate the propagation condition \a pc
      static PropCond pc_negateset(PropCond pc);
      /// Negate the modification event \a me
      static ModEvent me_negateset(ModEvent me);

      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ComplementView(void);
      /// Initialize with set view x
      ComplementView(View& x);
      /// Initialize from specification
      ComplementView(Space& home, const Reflection::VarMap& vars,
                     Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{

      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
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


      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, ComplementView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}

    };

    /**
     * \brief Print complement set view
     * \relates Gecode::Set::ComplementView
     */
    template<class Char, class Traits, class View>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os,
                const ComplementView<View>& x);
  }

  /** \name View comparison
   *  \relates Gecode::Set::ComplementView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  template <class View>
  bool same(const Set::ComplementView<View>& x,
            const Set::ComplementView<View>& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  template <class View>
  bool before(const Set::ComplementView<View>& x,
              const Set::ComplementView<View>& y);
  //@}


  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for ComplementView.
   * \ingroup TaskActorSetView
   */
  template <class View>
  class ViewVarImpTraits<Set::ComplementView<View> > {
  public:
    /// The variable type of a ComplementView
    typedef typename ViewVarImpTraits<View>::VarImp VarImp;
  };


  namespace Set {


    /**
     * \brief Offset set view
     *
     * An offset set view \f$s\f$ for a set view \f$t\f$ provides
     * operations such that \f$s\f$ behaves like \f$\{i+k\ |\ i\in t\}\f$.
     * \ingroup TaskActorSetView
     */

    template <class View>
    class OffsetSetView : public DerivedViewBase<View> {
    protected:
      using DerivedViewBase<View>::view;
      /// The offset
      int k;
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      OffsetSetView(void);
      /// Initialize with set view \a x and offset \a k0
      OffsetSetView(View& x, int k0);
      /// Initialize from specification
      OffsetSetView(Space& home, const Reflection::VarMap& vars,
                    Reflection::Arg* arg);
      //@}

      /// \name Value access
      //@{
      /// Return minimum cardinality
      unsigned int cardMin(void) const;
      /// Return maximum cardinality
      unsigned int cardMax(void) const;
      /// Return minimum of the least upper bound
      int lubMin(void) const;
      /// Return maximum of the least upper bound
      int lubMax(void) const;
      /// Return n-th smallest element of the least upper bound
      int lubMinN(int n) const;
      /// Return minimum of the greatest lower bound
      int glbMin(void) const;
      /// Return maximum of the greatest lower bound
      int glbMax(void) const;

      /// Return the number of elements in the greatest lower bound
      unsigned int glbSize(void) const;
      /// Return the number of elements in the least upper bound
      unsigned int lubSize(void) const;
      /// Return the number of unknown elements
      unsigned int unknownSize(void) const;
      /// Return the offset
      int offset(void) const;
      //@}

      /// \name Domain tests
      //@{
      /// Test whether view is assigned
      bool assigned(void) const;
      /// Test whether \a i is in the greatest lower bound
      bool contains(int i) const;
      /// Test whether \a i is not in the least upper bound
      bool notContains(int i) const;
      //@}


      /// \name Domain update by value
      //@{
      /// Restrict cardinality to be greater than or equal to \a m
      ModEvent cardMin(Space& home, unsigned int m);
      /// Restrict cardinality to be less than or equal to \a m
      ModEvent cardMax(Space& home, unsigned int m);
      /**
       * \brief Update greatest lower bound to include all elements
       * between and including \a i and \a j
       */
      ModEvent include(Space& home,int i,int j);
      /**
       * \brief Restrict least upper bound to not contain all elements
       * between and including \a i and \a j
       */
      ModEvent exclude(Space& home,int i,int j);
      /// Update greatest lower bound to contain \a i
      ModEvent include(Space& home,int i);
      /// Restrict least upper bound to not contain \a i
      ModEvent exclude(Space& home,int i);
      /**
       * \brief Update least upper bound to contain at most all elements
       * between and including \a i and \a j
       */
      ModEvent intersect(Space& home,int i,int j);
      /// Update least upper bound to contain at most the element \a i
      ModEvent intersect(Space& home,int i);
      //@}

      /// \name Domain update by range iterator
      //@{

      /// Remove range sequence described by \a i from least upper bound
      template <class I> ModEvent excludeI(Space& home, I& i);
      /// Include range sequence described by \a i in greatest lower bound
      template <class I> ModEvent includeI(Space& home, I& i);
      /// Intersect least upper bound with range sequence described by \a i
      template <class I> ModEvent intersectI(Space& home, I& iter);
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


      /// \name Cloning
      //@{
      /// Update this view to be a clone of view \a x
      void update(Space& home, bool share, OffsetSetView& x);
      //@}

      /// \name Reflection
      //@{
      /// Return specification for this view, using variable map \a m
      Reflection::Arg* spec(const Space& home, Reflection::VarMap& m) const;
      static Support::Symbol type(void);
      //@}

      /// \name Delta information for advisors
      //@{
      /// Return modification event
      static ModEvent modevent(const Delta& d);
      /// Return minimum value just pruned from glb
      int glbMin(const Delta& d) const;
      /// Return maximum value just pruned from glb
      int glbMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from glb
      bool glbAny(const Delta& d) const;
      /// Return minimum value just pruned from lub
      int lubMin(const Delta& d) const;
      /// Return maximum value just pruned from lub
      int lubMax(const Delta& d) const;
      /// Test whether arbitrary values got pruned from lub
      bool lubAny(const Delta& d) const;
      //@}
    };

    /**
     * \brief Print offset set view
     * \relates Gecode::Set::OffsetSetView
     */
    template<class Char, class Traits, class View>
    std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os,
                const OffsetSetView<View>& x);

  }

  /** \name View comparison
   *  \relates Gecode::Set::ComplementView
   */
  //@{
  /// Test whether views \a x and \a y are the same
  template <class View>
  bool same(const Set::OffsetSetView<View>& x,
            const Set::OffsetSetView<View>& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  template <class View>
  bool before(const Set::OffsetSetView<View>& x,
              const Set::OffsetSetView<View>& y);
  //@}


  /**
   * \brief Traits class for views and variable implementations
   *
   * This class specializes the ViewVarImpTraits for ComplementView.
   * \ingroup TaskActorSetView
   */
  template <class View>
  class ViewVarImpTraits<Set::OffsetSetView<View> > {
  public:
    /// The variable type of a ComplementView
    typedef typename ViewVarImpTraits<View>::VarImp VarImp;
  };


}

#include <gecode/set/var/set.hpp>

#include <gecode/set/view/set.hpp>

#include <gecode/set/view/const.hpp>
#include <gecode/set/view/singleton.hpp>
#include <gecode/set/view/complement.hpp>
#include <gecode/set/view/offset.hpp>

#include <gecode/set/view/print.hpp>
#include <gecode/set/var/print.hpp>

// STATISTICS: set-var
