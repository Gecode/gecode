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
  class ValMin : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValMin(void);
    /// Constructor for initialization
    ValMin(Space* home, const ValBranchOptions& vbo);
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
   * \brief Class for selecting median value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValMed : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValMed(void);
    /// Constructor for initialization
    ValMed(Space* home, const ValBranchOptions& vbo);
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
   * \brief Class for random value selection
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class _View>
  class ValRnd {
  protected:
    /// Random number generator
    Support::RandomGenerator r;
  public:
    /// View type
    typedef _View View;
    /// Value type
    typedef int Val;
    /// Description type
    typedef Support::RandomGenerator Desc;
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// Default constructor
    ValRnd(void);
    /// Constructor for initialization
    ValRnd(Space* home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(const Space* home, _View x);
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, _View x, int n);
    /// Return description
    Support::RandomGenerator description(const Space* home) const;
    /// Commit to description
    void commit(Space* home, const Support::RandomGenerator& d, unsigned a);
    /// Updating during cloning
    void update(Space* home, bool share, ValRnd& vs);
    /// Delete value selection
    void dispose(Space* home);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, _View x, int n) const;
  };

  /**
   * \brief Class for splitting domain (lower half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  template<class View>
  class ValSplitMin : public ValSelBase<View,int> {
  public:
    /// Default constructor
    ValSplitMin(void);
    /// Constructor for initialization
    ValSplitMin(Space* home, const ValBranchOptions& vbo);
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
  class ValZeroOne : public ValSelBase<View,NoValue> {
  public:
    /// Default constructor
    ValZeroOne(void);
    /// Constructor for initialization
    ValZeroOne(Space* home, const ValBranchOptions& vbo);
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
   * \brief Branching by view and values selection
   *
   * For an explanation of \a ViewSel see the class Gecode::ViewBranching.
   */
  template <class ViewSel, class View>
  class ViewValuesBranching : public ViewBranching<ViewSel> {
  protected:
    using ViewBranching<ViewSel>::x;
    /// Constructor for cloning \a b
    ViewValuesBranching(Space* home, bool share, ViewValuesBranching& b);
  public:
    /// Constructor for creation
    ViewValuesBranching(Space* home, ViewArray<typename ViewSel::View>& x,
                        ViewSel& vi_s);
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


  /// Class for assigning minimum value
  template<class View>
  class AssignValMin : public ValMin<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMin(void);
    /// Constructor for initialization
    AssignValMin(Space* home, const ValBranchOptions& vbo);
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
    AssignValMed(Space* home, const ValBranchOptions& vbo);
  };

  /// Class for assigning zero
  template<class View>
  class AssignValZeroOne : public ValZeroOne<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValZeroOne(void);
    /// Constructor for initialization
    AssignValZeroOne(Space* home, const ValBranchOptions& vbo);
  };


  /*
   * Variable selection classes
   *
   */

  /**
   * \brief View selection class for view with smallest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  template<class View>
  class ByMinMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest minimum
    int min;
  public:
    /// Default constructor
    ByMinMin(void);
    /// Constructor for initialization
    ByMinMin(Space* home, const VarBranchOptions& vbo);    
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
  class ByMinMax : public ViewSelBase<View> {
  protected:
    /// So-far largest minimum
    int min;
  public:
    /// Default constructor
    ByMinMax(void);
    /// Constructor for initialization
    ByMinMax(Space* home, const VarBranchOptions& vbo);    
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
  class ByMaxMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest maximum
    int max;
  public:
    /// Default constructor
    ByMaxMin(void);
    /// Constructor for initialization
    ByMaxMin(Space* home, const VarBranchOptions& vbo);    
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
  class ByMaxMax : public ViewSelBase<View> {
  protected:
    /// So-far largest maximum
    int max;
  public:
    /// Default constructor
    ByMaxMax(void);
    /// Constructor for initialization
    ByMaxMax(Space* home, const VarBranchOptions& vbo);    
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
  class BySizeMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMin(void);
    /// Constructor for initialization
    BySizeMin(Space* home, const VarBranchOptions& vbo);    
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
  class BySizeMax : public ViewSelBase<View> {
  protected:
    /// So-far largest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMax(void);
    /// Constructor for initialization
    BySizeMax(Space* home, const VarBranchOptions& vbo);    
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
  class BySizeDegreeMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest size/degree
    double sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMin(void);
    /// Constructor for initialization
    BySizeDegreeMin(Space* home, const VarBranchOptions& vbo);    
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
  class BySizeDegreeMax : public ViewSelBase<View> {
  protected:
    /// So-far largest size/degree
    double sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMax(void);
    /// Constructor for initialization
    BySizeDegreeMax(Space* home, const VarBranchOptions& vbo);    
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
  class ByRegretMinMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMinMin(void);
    /// Constructor for initialization
    ByRegretMinMin(Space* home, const VarBranchOptions& vbo);    
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
  class ByRegretMinMax : public ViewSelBase<View> {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMinMax(void);
    /// Constructor for initialization
    ByRegretMinMax(Space* home, const VarBranchOptions& vbo);    
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
  class ByRegretMaxMin : public ViewSelBase<View> {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMaxMin(void);
    /// Constructor for initialization
    ByRegretMaxMin(Space* home, const VarBranchOptions& vbo);    
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
  class ByRegretMaxMax : public ViewSelBase<View> {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// Default constructor
    ByRegretMaxMax(void);
    /// Constructor for initialization
    ByRegretMaxMax(Space* home, const VarBranchOptions& vbo);    
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, View x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, View x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  GECODE_INT_EXPORT void
  post(Space* home, ViewArray<IntView>& x,
       const TieBreakVarBranch<IntVarBranch>& vars, IntValBranch vals,
       const TieBreakVarBranchOptions& o_vars,
       const ValBranchOptions& o_vals);

  GECODE_INT_EXPORT void
  post(Space* home, ViewArray<BoolView>& x,
       const TieBreakVarBranch<IntVarBranch>& vars, IntValBranch vals,
       const TieBreakVarBranchOptions& o_vars,
       const ValBranchOptions& o_vals);

}}}

#include "gecode/int/branch/select-val.icc"
#include "gecode/int/branch/select-values.icc"
#include "gecode/int/branch/select-view.icc"
#include "gecode/int/branch/post-val-int.icc"
#include "gecode/int/branch/post-val-bool.icc"

#endif

// STATISTICS: int-branch
