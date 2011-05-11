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

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::Branch
 * \brief Integer branchers
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
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValMin : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValMin(void);
    /// Constructor for initialization
    ValMin(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, View x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, int n);
  };

  /**
   * \brief Class for selecting median value
   *
   * Requires
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValMed : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValMed(void);
    /// Constructor for initialization
    ValMed(Space& home, const ValBranchOptions& vbo);
    /// Return maximum value of view \a x
    int val(Space& home, View x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, int n);
  };

  /**
   * \brief Class for random value selection
   *
   * Requires
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<class _View>
  class ValRnd {
  protected:
    /// The random number generator
    ArchivedRandomGenerator r;
  public:
    /// View type
    typedef _View View;
    /// Value type
    typedef int Val;
    /// Choice type
    typedef ArchivedRandomGenerator Choice;
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// Default constructor
    ValRnd(void);
    /// Constructor for initialization
    ValRnd(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, _View x);
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, _View x, int n);
    /// Return choice
    Choice choice(Space& home);
    /// Return choice
    Choice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home, const Choice& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ValRnd& vs);
    /// Delete value selection
    void dispose(Space& home);
  };

  /**
   * \brief Class for splitting domain at mean of smallest and largest element 
   * (lower half first)
   *
   * Requires
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValSplitMin : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValSplitMin(void);
    /// Constructor for initialization
    ValSplitMin(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, View x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x >n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, int n);
  };


  /**
   * \brief Class for trying the first range of a domain first or splitting domain at mean of smallest and largest element 
   * (lower half first)
   *
   * Requires
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValRangeMin : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValRangeMin(void);
    /// Constructor for initialization
    ValRangeMin(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, View x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x >n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, int n);
  };


  /// For Boolean branchers not needing a value
  class NoValue {};

}}}

namespace Gecode {
  forceinline Archive&
  operator >>(Archive& e, Gecode::Int::Branch::NoValue&) {
    return e;
  }
  forceinline Archive&
  operator <<(Archive& e, const Gecode::Int::Branch::NoValue&) {
    return e;
  }
}

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \brief Class for trying zero and then one
   *
   * Requires
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValZeroOne : public ValSelBase<View,NoValue> {
  public:
    /// Default constructor
    ValZeroOne(void);
    /// Constructor for initialization
    ValZeroOne(Space& home, const ValBranchOptions& vbo);
    /// Return no value of view \a x
    NoValue val(Space& home, View x) const;
    /// Tell \f$x=0\f$ (\a a = 0) or \f$x=1\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, NoValue n);
  };


  /**
   * \brief %Brancher by view and values selection
   *
   */
  template<class ViewSel, class View>
  class ViewValuesBrancher : public ViewBrancher<ViewSel> {
  protected:
    using ViewBrancher<ViewSel>::x;
    using ViewBrancher<ViewSel>::viewsel;
    /// Constructor for cloning \a b
    ViewValuesBrancher(Space& home, bool share, ViewValuesBrancher& b);
    /// Constructor for creation
    ViewValuesBrancher(Home home, ViewArray<typename ViewSel::View>& x,
                       ViewSel& vi_s, BranchFilter bf);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a a
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int a);
    /// Perform cloning
    virtual Actor* copy(Space& home, bool share);
    /// Constructor for creation
    static void post(Home home, ViewArray<typename ViewSel::View>& x,
                     ViewSel& vi_s, BranchFilter bf=NULL);
  };


  /// Class for assigning minimum value
  template<class View>
  class AssignValMin : public ValMin<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMin(void);
    /// Constructor for initialization
    AssignValMin(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning maximum value
  template<class View>
  class AssignValMed : public ValMed<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMed(void);
    /// Constructor for initialization
    AssignValMed(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning random value
  template<class View>
  class AssignValRnd : public ValRnd<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValRnd(void);
    /// Constructor for initialization
    AssignValRnd(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning zero
  template<class View>
  class AssignValZero : public ValZeroOne<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValZero(void);
    /// Constructor for initialization
    AssignValZero(Space& home, const ValBranchOptions& vbo);
  };

  /*
   * Variable selection classes
   *
   */

  /**
   * \brief View selection class for view with smallest min
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByMinMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest minimum
    int min;
  public:
    /// Default constructor
    ByMinMin(void);
    /// Constructor for initialization
    ByMinMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest min
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByMinMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest minimum
    int min;
  public:
    /// Default constructor
    ByMinMax(void);
    /// Constructor for initialization
    ByMinMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest max
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByMaxMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest maximum
    int max;
  public:
    /// Default constructor
    ByMaxMin(void);
    /// Constructor for initialization
    ByMaxMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest max
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByMaxMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest maximum
    int max;
  public:
    /// Default constructor
    ByMaxMax(void);
    /// Constructor for initialization
    ByMaxMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMin(void);
    /// Constructor for initialization
    BySizeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMax(void);
    /// Constructor for initialization
    BySizeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest size divided
   * by degree.
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeDegreeMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest size/degree
    double sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMin(void);
    /// Constructor for initialization
    BySizeDegreeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest size divided
   * by degree.
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeDegreeMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest size/degree
    double sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMax(void);
    /// Constructor for initialization
    BySizeDegreeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest size divided by accumulated failure count
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeAfcMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest size/afc
    double sizeafc;
  public:
    /// Default constructor
    BySizeAfcMin(void);
    /// Constructor for initialization
    BySizeAfcMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest size divided by accumulated failure count
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeAfcMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest size/afc
    double sizeafc;
  public:
    /// Default constructor
    BySizeAfcMax(void);
    /// Constructor for initialization
    BySizeAfcMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest min-regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMinMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMinMin(void);
    /// Constructor for initialization
    ByRegretMinMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest min-regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMinMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMinMax(void);
    /// Constructor for initialization
    ByRegretMinMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest max-regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMaxMin : public ViewSelBase<IntView> {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMaxMin(void);
    /// Constructor for initialization
    ByRegretMaxMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

  /**
   * \brief View selection class for view with largest max-regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMaxMax : public ViewSelBase<IntView> {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMaxMax(void);
    /// Constructor for initialization
    ByRegretMaxMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, IntView x);
  };

}}}

#include <gecode/int/branch/select-val.hpp>
#include <gecode/int/branch/select-values.hpp>
#include <gecode/int/branch/select-view.hpp>
#include <gecode/int/branch/post-val-int.hpp>
#include <gecode/int/branch/post-val-bool.hpp>

#endif

// STATISTICS: int-branch
