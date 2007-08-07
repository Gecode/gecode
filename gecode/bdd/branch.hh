/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#ifndef __GECODE_BDD_BRANCH_HH__
#define __GECODE_BDD_BRANCH_HH__

#include "gecode/bdd.hh"
#include "gecode/iter.hh"

/**
 * \namespace Gecode::Bdd::Branch
 * \brief %Bdd branchings
 */

namespace Gecode { namespace Bdd { namespace Branch {

  /*
   * Value selection classes
   *
   */


  /**
   * \brief Class for selecting minimum value
   *
   * All value selection classes require 
   * \code #include "gecode/bdd/branch.hh" \endcode
   * \ingroup FuncSetSelVal
   */
  template <bool exclude>
  class ValMin {
  public:
    /// Return minimum value of view \a x
    IntSet val(const Space*, BddView x);
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, BddView x, IntSet v);
    /// Type of this value selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief Class for selecting maximum value
   *
   * All value selection classes require 
   * \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelVal
   */
  template <bool exclude>
  class ValMax {
  public:
    /// Return maximum value of view \a x
    IntSet val(const Space*, BddView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, BddView x, IntSet v);
    /// Type of this value selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief Class for selecting minimum value of unknown ranges
   *
   * All value selection classes require 
   * \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelVal
   */
  template <bool exclude>
  class ValMinUnknown {
  public:
    /// Return minimum value of view \a x
    int val(const Space*, BddView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, BddView x, int v);
    /// Type of this value selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief Class for selecting minimum value of unknown ranges
   *
   * All value selection classes require 
   * \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelVal
   */
  template <bool exclude>
  class ValMaxUnknown {
  public:
    /// Return minimum value of view \a x
    int val(const Space*, BddView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, BddView x, int v);
    /// Type of this value selection (for reflection)
    static Reflection::Type* type(void);
  };

  /// Create branchings for a given view selection strategy \a ViewSel
  template <class SelView>
  static void
  create(Space* home, ViewArray<BddView>&, BddBvalSel);

  /*
   * View selection classes
   *
   */

  /**
   * \brief View selection class for first view
   *
   * Requires \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelView
   */
  class ByNone {
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space*, BddView);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space*, BddView);
    /// Type of this view selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief View selection class for view with smallest cardinality of lub-glb
   *
   * Requires \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelView
   */
  class ByMinCard {
  private:
    /// So-far smallest cardinality
    unsigned int minCard;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space*, BddView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space*, BddView x);
    /// Type of this view selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief View selection class for view with greatest cardinality of lub-glb
   *
   * Requires \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelView
   */
  class ByMaxCard {
  private:
    /// So-far greatest cardinality
    unsigned int maxCard;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space*, BddView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space*, BddView x);
    /// Type of this view selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief View selection class for view with smallest element in lub-glb
   *
   * Requires \code #include "gecode/bdd/branch.hh" \endcode
   * \ingroup FuncSetSelView
   */
  class ByMinUnknown {
  private:
    /// So-far smallest element
    int minUnknown;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space*, BddView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space*, BddView x);
    /// Type of this view selection (for reflection)
    static Reflection::Type* type(void);
  };

  /**
   * \brief View selection class for view with greatest element in lub-glb
   *
   * Requires \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelView
   */
  class ByMaxUnknown {
  private:
    /// So-far greatest element
    int maxUnknown;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space*, BddView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space*, BddView x);
    /// Type of this view selection (for reflection)
    static Reflection::Type* type(void);
  };

}}}

#include "gecode/bdd/branch/select-val.icc"
#include "gecode/bdd/branch/select-view.icc"

#endif

// STATISTICS: bdd-branch
