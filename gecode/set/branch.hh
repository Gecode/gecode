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
   * View selection classes
   *
   */

  /**
   * \brief View selection class for view with smallest minimum element in lub-glb
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class ByMinMin : public ViewSelBase<SetView> {
  private:
    /// So-far smallest minimum element
    int min;
  public:
    /// Default constructor
    ByMinMin(void);
    /// Constructor for initialization
    ByMinMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with largest minimum element in lub-glb
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class ByMinMax : public ViewSelBase<SetView> {
  private:
    /// So-far largest minimum element
    int min;
  public:
    /// Default constructor
    ByMinMax(void);
    /// Constructor for initialization
    ByMinMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with smallest maximal element in lub-glb
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class ByMaxMin : public ViewSelBase<SetView> {
  private:
    /// So-far smallest maximal element
    int max;
  public:
    /// Default constructor
    ByMaxMin(void);
    /// Constructor for initialization
    ByMaxMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with largest maximal element in lub-glb
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class ByMaxMax : public ViewSelBase<SetView> {
  private:
    /// So-far smallest maximal element
    int max;
  public:
    /// Default constructor
    ByMaxMax(void);
    /// Constructor for initialization
    ByMaxMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with smallest cardinality of lub-glb
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class BySizeMin : public ViewSelBase<SetView> {
  private:
    /// So-far smallest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMin(void);
    /// Constructor for initialization
    BySizeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with largest cardinality of lub-glb
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncSetSelView
   */
  class BySizeMax : public ViewSelBase<SetView> {
  private:
    /// So-far largest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMax(void);
    /// Constructor for initialization
    BySizeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with smallest cardinality of lub-glb 
   * divided by degree.
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeDegreeMin : public ViewSelBase<SetView> {
  protected:
    /// So-far smallest size/degree
    double sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMin(void);
    /// Constructor for initialization
    BySizeDegreeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with largest cardinality of lub-glb 
   * divided by degree.
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeDegreeMax : public ViewSelBase<SetView> {
  protected:
    /// So-far largest size/degree
    double sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMax(void);
    /// Constructor for initialization
    BySizeDegreeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with smallest cardinality of lub-glb 
   * divided by accumulated failure count
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeAfcMin : public ViewSelBase<SetView> {
  protected:
    /// So-far smallest size/afc
    double sizeafc;
  public:
    /// Default constructor
    BySizeAfcMin(void);
    /// Constructor for initialization
    BySizeAfcMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

  /**
   * \brief View selection class for view with largest cardinality of lub-glb 
   * divided by accumulated failure count
   *
   * Requires \code #include <gecode/set/branch.hh> \endcode
   * \ingroup FuncIntSelView
   */
  class BySizeAfcMax : public ViewSelBase<SetView> {
  protected:
    /// So-far largest size/afc
    double sizeafc;
  public:
    /// Default constructor
    BySizeAfcMax(void);
    /// Constructor for initialization
    BySizeAfcMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, SetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, SetView x);
  };

}}}

#include <gecode/set/branch/select-val.hpp>
#include <gecode/set/branch/select-view.hpp>
#include <gecode/set/branch/post-val.hpp>

#endif
// STATISTICS: set-branch

