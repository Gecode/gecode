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
    /// Return minimum value of view \a x at position \a i
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
    /// Return maximum value of view \a x at position \a i
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
    /// Return minimum value of view \a x at position \a i
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
    /// Return minimum value of view \a x at position \a i
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
    /// Return minimum value of view \a x at position \a i
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
    /// Return no value of view \a x at position \a i
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
   * View merit classes
   *
   */

  /**
   * \brief Merit class for mimimum
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritMin : public MeritBase {
  public:
    /// Default constructor
    MeritMin(void);
    /// Constructor for initialization
    MeritMin(Space& home, const VarBranchOptions& vbo);
    /// Return minimum as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

  /**
   * \brief Merit class for maximum
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritMax : public MeritBase {
  public:
    /// Default constructor
    MeritMax(void);
    /// Constructor for initialization
    MeritMax(Space& home, const VarBranchOptions& vbo);
    /// Return maximum as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

  /**
   * \brief Merit class for size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritSize : public MeritBase {
  public:
    /// Default constructor
    MeritSize(void);
    /// Constructor for initialization
    MeritSize(Space& home, const VarBranchOptions& vbo);
    /// Return size as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

  /**
   * \brief Merit class for size over degree
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritSizeDegree : public MeritBase {
  public:
    /// Default constructor
    MeritSizeDegree(void);
    /// Constructor for initialization
    MeritSizeDegree(Space& home, const VarBranchOptions& vbo);
    /// Return size over degree as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

  /**
   * \brief Merit class for size over afc
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritSizeAfc : public MeritBase {
  public:
    /// Default constructor
    MeritSizeAfc(void);
    /// Constructor for initialization
    MeritSizeAfc(Space& home, const VarBranchOptions& vbo);
    /// Return size over AFC as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

  /**
   * \brief Merit class for size over activity
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritSizeActivity : public MeritBase {
    /// Activity information
    Activity activity;
  public:
    /// Default constructor
    MeritSizeActivity(void);
    /// Constructor for initialization
    MeritSizeActivity(Space& home, const VarBranchOptions& vbo);
    /// Return size over activity as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, MeritSizeActivity& msa);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for minimum regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritRegretMin : public MeritBase {
  public:
    /// Default constructor
    MeritRegretMin(void);
    /// Constructor for initialization
    MeritRegretMin(Space& home, const VarBranchOptions& vbo);
    /// Return minimum regret as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

  /**
   * \brief Merit class for maximum regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class MeritRegretMax : public MeritBase {
  public:
    /// Default constructor
    MeritRegretMax(void);
    /// Constructor for initialization
    MeritRegretMax(Space& home, const VarBranchOptions& vbo);
    /// Return maximum regret as merit for view \a x at position \a i
    double operator ()(Space& home, IntView x, int i);
  };

}}}

#include <gecode/int/branch/select-val.hpp>
#include <gecode/int/branch/select-values.hpp>
#include <gecode/int/branch/select-view.hpp>
#include <gecode/int/branch/post-val-int.hpp>
#include <gecode/int/branch/post-val-bool.hpp>

#endif

// STATISTICS: int-branch
