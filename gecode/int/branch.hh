/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#ifndef __GECODE_INT_BRANCH_HH__
#define __GECODE_INT_BRANCH_HH__

#include "gecode/int.hh"

/**
 * \namespace Gecode::Int::Branch
 * \brief Integer branchings
 */

namespace Gecode { namespace Int { namespace Branch {

  /*
   * Value selection classes
   *
   */

  /**
   * \brief Class for selecting minimum value
   *
   * All value selection classes require
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValMin {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef int Val;
    /// Return minimum value of view \a x
    int val(const Space* home, V x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, int n) const;
  };

  /**
   * \brief Class for selecting maximum value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValMed {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef int Val;
    /// Return maximum value of view \a x
    int val(const Space* home, V x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, int n) const;
  };

  /**
   * \brief Class for selecting median value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValMax {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef int Val;
    /// Return median value of view \a x
    int val(const Space* home, V x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, int n) const;
  };

  /**
   * \brief Class for splitting domain (lower half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValSplitMin {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef int Val;
    /// Return minimum value of view \a x
    int val(const Space* home, V x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x >n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, int n) const;
  };

  /**
   * \brief Class for splitting domain (upper half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValSplitMax {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef int Val;
    /// Return minimum value of view \a x
    int val(const Space* home, V x) const;
    /// Tell \f$x>n\f$ (\a a = 0) or \f$x\leq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, int n) const;
  };

  /// Create branchings for a given view selection strategy \a SelView
  template<class View, class SelView>
  static void
  create(Space* home, ViewArray<View>&, IntValBranch);

  /// For Boolean branchings not needing a value
  class NoValue {
  public:
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Cast to integer (for reflection)
    operator int(void);
  };

  /**
   * \brief Class for trying zero and then one
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValZeroOne {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef NoValue Val;
    /// Return no value of view \a x
    NoValue val(const Space* home, V x) const;
    /// Tell \f$x=0\f$ (\a a = 0) or \f$x=1\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, NoValue n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, NoValue n) const;
  };

  /**
   * \brief Class for trying one and then zero
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class V>
  class ValOneZero {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// View type
    typedef V View;
    /// Value type
    typedef NoValue Val;
    /// Return no value of view \a x
    NoValue val(const Space* home, V x) const;
    /// Tell \f$x=1\f$ (\a a = 0) or \f$x=0\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, V x, NoValue n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, V x, NoValue n) const;
  };


  /**
   * \brief Branching by view and values selection
   *
   * Implements view-based branching for an array of views (of type
   * \a ViewSel::View) and the full set of values. The behaviour is 
   * defined by the class \a ViewSel (which view is selected for branching).
   *
   * The class \a ViewSel must implement two member functions:
   *  - Gecode::ViewSelStatus init(const Space* \a home, \a View \a x)
   *    Initializes view selection with the \a View \a x. If \a x
   *    is known to be a best one, VSS_BEST should be returned.
   *    Otherwise, VSS_BETTER should be returned.
   *  - Gecode::ViewSelStatus select(const Space* \a home, \a View \a x)
   *    If \a x is better than the previously selected view,
   *    return VSS_BETTER. If it is a best view, return VSS_BEST. If it 
   *    is worse, return VSS_WORSE. If it is equally good, return VSS_TIE.
   *
   */
  template <class ViewSel>
  class ViewValuesBranching : public ViewBranching<ViewSel> {
  protected:
    using ViewBranching<ViewSel>::x;
    /// Constructor for cloning \a b
    ViewValuesBranching(Space* home, bool share, ViewValuesBranching& b);
  public:
    /// Constructor for creation
    ViewValuesBranching(Space* home, ViewArray<typename ViewSel::View>& x);
    /// Return branching description (of type PosValuesDesc)
    virtual const BranchingDesc* description(const Space* home) const;
    /// Perform commit for branching description \a d and alternative \a a
    virtual ExecStatus commit(Space* home, const BranchingDesc* d,
                              unsigned int a);
    /// Return specification for this branching given a variable map \a m
    virtual Reflection::ActorSpec spec(const Space* home,
                                       Reflection::VarMap& m) const;
    /// Return specification for a branch
    virtual Reflection::BranchingSpec
    branchingSpec(const Space* home, 
                  Reflection::VarMap& m,
                  const BranchingDesc* d) const;
    /// Actor type identifier of this branching
    static Support::Symbol ati(void);
    /// Post branching according to specification
    static void post(Space* home, Reflection::VarMap& m,
                     const Reflection::ActorSpec& spec);
    /// Perform cloning
    virtual Actor* copy(Space* home, bool share);
  };

  /**
   * \brief %Branching descriptions storing position and values
   *
   * The maximal number of alternatives is defined by \a alt.
   */
  class PosValuesDesc : public PosDesc {
  private:
    /// Information about position and minimum
    class PosMin {
    public:
      /// Start position of range
      unsigned int pos;
      /// Minmum of range
      int min;
    };
    /// Number of ranges
    unsigned int n;
    /// Values to assign to
    PosMin* pm;
  public:
    /// Initialize description for branching \a b, position \a p, and value \a n
    PosValuesDesc(const Branching* b, const Pos& p, IntView x);
    /// Return value to branch with
    int val(unsigned int a) const;
    /// Report size occupied
    virtual size_t size(void) const;
  };

  
  /// Class for assigning minimum value
  template<class V>
  class AssignValMin : public ValMin<V> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
  };

  /// Class for assigning maximum value
  template<class V>
  class AssignValMed : public ValMed<V> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
  };

  /// Class for assigning median value
  template<class V>
  class AssignValMax : public ValMax<V> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
  };
  /// Class for assigning zero
  template<class V>
  class AssignValZeroOne : public ValZeroOne<V> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
  };

  /// Class for assigning one
  template<class V>
  class AssignValOneZero : public ValOneZero<V> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
  };


  /*
   * Variable selection classes
   *
   */

  /**
   * \brief View selection class for first variable
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByNone {
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByMinMin {
  protected:
    /// So-far smallest minimum
    int min;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByMinMax {
  protected:
    /// So-far largest minimum
    int min;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest max
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByMaxMin {
  protected:
    /// So-far smallest maximum
    int max;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest max
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByMaxMax {
  protected:
    /// So-far largest maximum
    int max;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest size
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class BySizeMin {
  protected:
    /// So-far smallest size
    unsigned int size;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest size
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class BySizeMax {
  protected:
    /// So-far largest size
    unsigned int size;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest degree (and smallest size in case of ties)
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByDegreeMin {
  protected:
    /// So-far smallest degree
    unsigned int degree;
    /// So-far smallest size for degree
    unsigned int size;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest degree (and smallest size in case of ties)
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByDegreeMax {
  protected:
    /// So-far largest degree
    unsigned int degree;
    /// So-far smallest size for degree
    unsigned int size;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest degree
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByDegreeMinNoTies {
  protected:
    /// So-far smallest degree
    unsigned int degree;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest degree
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByDegreeMaxNoTies {
  protected:
    /// So-far largest degree
    unsigned int degree;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest size divided
   * by degree.
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class BySizeDegreeMin {
  protected:
    /// So-far smallest size/degree
    double sizedegree;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest size divided by degree.
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class BySizeDegreeMax {
  protected:
    /// So-far largest size/degree
    double sizedegree;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest min-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByRegretMinMin {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest min-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByRegretMinMax {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest max-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByRegretMaxMin {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest max-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class V>
  class ByRegretMaxMax {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// View type
    typedef V View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, V x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, V x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  template <template <class,class> class ViewValBranching>
  void
  createBranch(Space* home, const IntVarArgs& x,
               IntVarBranch vars, IntValBranch vals);

  template <template <class,class> class ViewValBranching>
  void
  createBranch(Space* home, const BoolVarArgs& x, 
               IntVarBranch vars, IntValBranch vals);

}}}

#include "gecode/int/branch/select-val.icc"
#include "gecode/int/branch/select-view.icc"
#include "gecode/int/branch/create-branch.icc"

#endif

// STATISTICS: int-branch
