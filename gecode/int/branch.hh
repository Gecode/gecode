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
  template<class View>
  class ValMin {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef int Val;
    /// Return minimum value of view \a x
    int val(const Space* home, View x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, int n) const;
  };

  /**
   * \brief Class for selecting maximum value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValMed {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef int Val;
    /// Return maximum value of view \a x
    int val(const Space* home, View x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, int n) const;
  };

  /**
   * \brief Class for selecting median value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValMax {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef int Val;
    /// Return median value of view \a x
    int val(const Space* home, View x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, int n) const;
  };

  /**
   * \brief Class for splitting domain (lower half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValSplitMin {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef int Val;
    /// Return minimum value of view \a x
    int val(const Space* home, View x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x >n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, int n) const;
  };

  /**
   * \brief Class for splitting domain (upper half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValSplitMax {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef int Val;
    /// Return minimum value of view \a x
    int val(const Space* home, View x) const;
    /// Tell \f$x>n\f$ (\a a = 0) or \f$x\leq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, int n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, int n) const;
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
  template<class View>
  class ValZeroOne {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef NoValue Val;
    /// Return no value of view \a x
    NoValue val(const Space* home, View x) const;
    /// Tell \f$x=0\f$ (\a a = 0) or \f$x=1\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, NoValue n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, NoValue n) const;
  };

  /**
   * \brief Class for trying one and then zero
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValOneZero {
  public:
    /// View type
    typedef View View;
    /// Value type
    typedef NoValue Val;
    /// Return no value of view \a x
    NoValue val(const Space* home, View x) const;
    /// Tell \f$x=1\f$ (\a a = 0) or \f$x=0\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, View x, NoValue n);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, View x, NoValue n) const;
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
  template<class View>
  class ByNone {
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByMinMin {
  protected:
    /// So-far smallest minimum
    int min;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByMinMax {
  protected:
    /// So-far largest minimum
    int min;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest max
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByMaxMin {
  protected:
    /// So-far smallest maximum
    int max;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest max
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByMaxMax {
  protected:
    /// So-far largest maximum
    int max;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest size
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class BySizeMin {
  protected:
    /// So-far smallest size
    unsigned int size;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest size
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class BySizeMax {
  protected:
    /// So-far largest size
    unsigned int size;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest degree (and smallest size in case of ties)
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByDegreeMin {
  protected:
    /// So-far smallest degree
    unsigned int degree;
    /// So-far smallest size for degree
    unsigned int size;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest degree (and smallest size in case of ties)
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByDegreeMax {
  protected:
    /// So-far largest degree
    unsigned int degree;
    /// So-far smallest size for degree
    unsigned int size;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest degree
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByDegreeMinNoTies {
  protected:
    /// So-far smallest degree
    unsigned int degree;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest degree
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByDegreeMaxNoTies {
  protected:
    /// So-far largest degree
    unsigned int degree;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
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
  template<class View>
  class BySizeDegreeMin {
  protected:
    /// So-far smallest size/degree
    double sizedegree;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest size divided by degree.
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class BySizeDegreeMax {
  protected:
    /// So-far largest size/degree
    double sizedegree;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest min-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByRegretMinMin {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest min-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByRegretMinMax {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest max-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByRegretMaxMin {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with largest max-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByRegretMaxMax {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// View type
    typedef View View;
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
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
