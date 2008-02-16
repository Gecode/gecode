/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

#ifndef __GECODE_SET_BRANCH_HH__
#define __GECODE_SET_BRANCH_HH__

#include "gecode/set.hh"

/**
 * \namespace Gecode::Set::Branch
 * \brief %Set branchings
 */

namespace Gecode { namespace Set { namespace Branch {

  /*
   * Value selection classes
   *
   */


  /**
   * \brief Class for selecting minimum value
   *
   * All value selection classes require 
   * \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelVal
   */
  class ValMin {
  public:
    /// Return minimum value of view \a x
    int val(const Space* home, SetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, SetView x, int v);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, SetView x, int n) const;
  };

  /**
   * \brief Class for selecting maximum value
   *
   * All value selection classes require 
   * \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelVal
   */
  class ValMax {
  public:
    /// Return maximum value of view \a x
    int val(const Space* home, SetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space* home, unsigned int a, SetView x, int v);
    /// Type of this value selection (for reflection)
    static Support::Symbol type(void);
    /// Specification of a branch (for reflection)
    void branchingSpec(const Space* home,
                       Reflection::VarMap& m, Reflection::BranchingSpec& bs,
                       int alt, SetView x, int n) const;
  };

  /// Create branchings for a given view selection strategy \a SelView
  template <class SelView>
  static void
  create(Space* home, ViewArray<SetView>&, SetValBranch);

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
    ViewSelStatus init(const Space* home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, SetView x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
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
    ViewSelStatus init(const Space* home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, SetView x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
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
    ViewSelStatus init(const Space* home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, SetView x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

  /**
   * \brief View selection class for view with smallest element in lub-glb
   *
   * Requires \code #include "gecode/set/branch.hh" \endcode
   * \ingroup FuncSetSelView
   */
  class ByMinUnknown {
  private:
    /// So-far smallest element
    int minUnknown;
  public:
    /// Intialize with view \a x
    ViewSelStatus init(const Space* home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, SetView x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
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
    ViewSelStatus init(const Space* home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(const Space* home, SetView x);
    /// Type of this view selection (for reflection)
    static Support::Symbol type(void);
  };

}}}

#include "gecode/set/branch/select-val.icc"
#include "gecode/set/branch/select-view.icc"

#endif
// STATISTICS: set-branch

