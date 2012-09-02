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

#include <gecode/set.hh>

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
   * \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelVal
   */
  template<bool inc>
  class ValMin : public ValSelBase<SetView,int> {
  public:
    /// Default constructor
    ValMin(void);
    /// Constructor for initialization
    ValMin(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, SetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, SetView x, int v);
  };

  /**
   * \brief Class for selecting median value (rounding downwards)
   *
   * All value selection classes require
   * \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelVal
   */
  template<bool inc>
  class ValMed : public ValSelBase<SetView,int> {
  public:
    /// Default constructor
    ValMed(void);
    /// Constructor for initialization
    ValMed(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, SetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, SetView x, int v);
  };

  /**
   * \brief Class for selecting maximum value
   *
   * All value selection classes require
   * \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelVal
   */
  template<bool inc>
  class ValMax : public ValSelBase<SetView,int> {
  public:
    /// Default constructor
    ValMax(void);
    /// Constructor for initialization
    ValMax(Space& home, const ValBranchOptions& vbo);
    /// Return maximum value of view \a x
    int val(Space& home, SetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, SetView x, int v);
  };

  /**
   * \brief Class for random value selection
   *
   * Requires
   * \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncIntSelVal
   */
  template<bool inc>
  class ValRnd {
  protected:
    /// Random number generator
    ArchivedRandomGenerator r;
  public:
    /// View type
    typedef SetView View;
    /// Value type
    typedef int Val;
    /// Choice
    typedef ArchivedRandomGenerator Choice;
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// Default constructor
    ValRnd(void);
    /// Constructor for initialization
    ValRnd(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space& home, SetView x);
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x\neq n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, SetView x, int n);
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

  /// Class for assigning minimum value
  template<bool inc>
  class AssignValMin : public ValMin<inc> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMin(void);
    /// Constructor for initialization
    AssignValMin(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning median value (rounding downwards)
  template<bool inc>
  class AssignValMed : public ValMed<inc> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMed(void);
    /// Constructor for initialization
    AssignValMed(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning maximum value
  template<bool inc>
  class AssignValMax : public ValMax<inc> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMax(void);
    /// Constructor for initialization
    AssignValMax(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning random value
  template<bool inc>
  class AssignValRnd : public ValRnd<inc> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValRnd(void);
    /// Constructor for initialization
    AssignValRnd(Space& home, const ValBranchOptions& vbo);
  };


  /*
   * View merit classes
   *
   */

  /**
   * \brief Merit class for mimimum
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class MeritMin : public MeritBase<SetView> {
  public:
    /// Default constructor
    MeritMin(void);
    /// Constructor for initialization
    MeritMin(Space& home, const VarBranchOptions& vbo);
    /// Return minimum as merit for view \a x at position \a i
    double operator ()(Space& home, SetView x, int i);
  };

  /**
   * \brief Merit class for maximum
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class MeritMax : public MeritBase<SetView> {
  public:
    /// Default constructor
    MeritMax(void);
    /// Constructor for initialization
    MeritMax(Space& home, const VarBranchOptions& vbo);
    /// Return maximum as merit for view \a x at position \a i
    double operator ()(Space& home, SetView x, int i);
  };

  /**
   * \brief Merit class for size
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class MeritSize : public MeritBase<SetView> {
  public:
    /// Default constructor
    MeritSize(void);
    /// Constructor for initialization
    MeritSize(Space& home, const VarBranchOptions& vbo);
    /// Return size as merit for view \a x at position \a i
    double operator ()(Space& home, SetView x, int i);
  };

  /**
   * \brief Merit class for size over degree
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class MeritSizeDegree : public MeritBase<SetView> {
  public:
    /// Default constructor
    MeritSizeDegree(void);
    /// Constructor for initialization
    MeritSizeDegree(Space& home, const VarBranchOptions& vbo);
    /// Return size over degree as merit for view \a x at position \a i
    double operator ()(Space& home, SetView x, int i);
  };

  /**
   * \brief Merit class for size over afc
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class MeritSizeAfc : public MeritBase<SetView> {
  public:
    /// Default constructor
    MeritSizeAfc(void);
    /// Constructor for initialization
    MeritSizeAfc(Space& home, const VarBranchOptions& vbo);
    /// Return size over AFC as merit for view \a x at position \a i
    double operator ()(Space& home, SetView x, int i);
  };

  /**
   * \brief Merit class for size over activity
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class MeritSizeActivity : public MeritBase<SetView> {
    /// Activity information
    Activity activity;
  public:
    /// Default constructor
    MeritSizeActivity(void);
    /// Constructor for initialization
    MeritSizeActivity(Space& home, const VarBranchOptions& vbo);
    /// Return size over activity as merit for view \a x at position \a i
    double operator ()(Space& home, SetView x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, MeritSizeActivity& msa);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

}}}

#include <gecode/set/branch/select-val.hpp>
#include <gecode/set/branch/select-view.hpp>
#include <gecode/set/branch/post-val.hpp>

#endif
// STATISTICS: set-branch

