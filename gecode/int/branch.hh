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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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
  class ValMin {
  public:
    /// Return minimum value of view \a x
    int val(const Space* home, IntView x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, IntView x, int n);
  };

  /**
   * \brief Class for selecting maximum value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  class ValMed {
  public:
    /// Return maximum value of view \a x
    int val(const Space* home, IntView x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, IntView x, int n);
  };

  /**
   * \brief Class for selecting median value
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  class ValMax {
  public:
    /// Return median value of view \a x
    int val(const Space* home, IntView x) const;
    /// Tell \f$x=n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, IntView x, int n);
  };

  /**
   * \brief Class for splitting domain (lower half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  class ValSplitMin {
  public:
    /// Return minimum value of view \a x
    int val(const Space* home, IntView x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x >n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, IntView x, int n);
  };

  /**
   * \brief Class for splitting domain (upper half first)
   *
   * Requires
   * \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelVal
   */
  class ValSplitMax {
  public:
    /// Return minimum value of view \a x
    int val(const Space* home, IntView x) const;
    /// Tell \f$x>n\f$ (\a a = 0) or \f$x\leq n\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, IntView x, int n);
  };

  /// Create branchings for a given view selection strategy \a ViewSel
  template <class SelView>
  static void
  create(Space* home, ViewArray<IntView>&, BvalSel);


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
  class ByNone {
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByMinMin {
  protected:
    /// So-far smallest minimum
    int min;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with largest min
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByMinMax {
  protected:
    /// So-far largest minimum
    int min;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest max
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByMaxMin {
  protected:
    /// So-far smallest maximum
    int max;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with largest max
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByMaxMax {
  protected:
    /// So-far largest maximum
    int max;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest size
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeMin {
  protected:
    /// So-far smallest size
    unsigned int size;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with largest size
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeMax {
  protected:
    /// So-far largest size
    unsigned int size;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest degree (and smallest size in case of ties)
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByDegreeMin {
  protected:
    /// So-far smallest degree
    unsigned int degree;
    /// So-far smallest size for degree
    unsigned int size;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with largest degree (and smallest size in case of ties)
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByDegreeMax {
  protected:
    /// So-far largest degree
    unsigned int degree;
    /// So-far smallest size for degree
    unsigned int size;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest min-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMinMin {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with largest min-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMinMax {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with smallest max-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMaxMin {
  protected:
    /// So-far smallest regret
    unsigned int regret;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };

  /**
   * \brief View selection class for view with largest max-regret
   *
   * Requires \code #include "gecode/int/branch.hh" \endcode
   * \ingroup FuncIntSelView
   */
  class ByRegretMaxMax {
  protected:
    /// So-far largest regret
    unsigned int regret;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, IntView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, IntView x);
  };


  /**
   * \brief %Branching descriptions storing position and value
   *
   */
  class AssignDesc : public BranchingDesc {
  protected:
    const int _pos;
    const int _val;
  public:
    /// Initialize description for branching \a b, position \a p and value \a n
    AssignDesc(const Branching* b, const int p, const int n);
    /// Return position in array
    int pos(void) const;
    /// Return value to branch with
    int val(void) const;
    /// Report size occupied
    virtual size_t size(void) const;
  };

  /*
   * Classes for assignment
   *
   */

  /// Assignment (single-alternative branching) base-class
  class Assign : public Branching {
  protected:
    /// Views to assign
    ViewArray<IntView> x;
    /// Next position to be assigned
    mutable int pos;
    /// Constructor for cloning \a b
    Assign(Space* home, bool share, Assign& b);
  public:
    /// Constructor for creation
    Assign(Space* home, ViewArray<IntView>& x);
    /// Perform branching (selects view)
    virtual bool status(const Space* home) const;
    /// Perform commit for branching description \a d and alternative \a a
    virtual ExecStatus commit(Space* home, const BranchingDesc* d,
                              unsigned int a);
  };


  /// Minimum assignment (single-alternative branching)
  class AssignMin : public Assign {
  protected:
    /// Constructor for cloning \a b
    AssignMin(Space* home, bool share, AssignMin& b);
  public:
    /// Constructor for creation
    AssignMin(Space* home, ViewArray<IntView>& x);
    /// Perform cloning
    virtual Actor* copy(Space* home, bool share);
    /// Return branching description (of type AssignDesc)
    virtual const BranchingDesc* description(const Space* home) const;
  };

  /// Median assignment (single-alternative branching)
  class AssignMed : public Assign {
  protected:
    /// Constructor for cloning \a b
    AssignMed(Space* home, bool share, AssignMed& b);
  public:
    /// Constructor for creation
    AssignMed(Space* home, ViewArray<IntView>& x);
    /// Perform cloning
    virtual Actor* copy(Space* home, bool share);
    /// Return branching description (of type AssignDesc)
    virtual const BranchingDesc* description(const Space* home) const;
  };

  /// Maximum assignment (single-alternative branching)
  class AssignMax : public Assign {
  protected:
    /// Constructor for cloning \a b
    AssignMax(Space* home, bool share, AssignMax& b);
  public:
    /// Constructor for creation
    AssignMax(Space* home, ViewArray<IntView>& x);
    /// Perform cloning
    virtual Actor* copy(Space* home, bool share);
    /// Return branching description (of type AssignDesc)
    virtual const BranchingDesc* description(const Space* home) const;
  };

}}}

#include "gecode/int/branch/select-val.icc"
#include "gecode/int/branch/select-view.icc"

#include "gecode/int/branch/assign.icc"

#endif

// STATISTICS: int-branch

