/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#ifndef __GECODE_CPLTSET_BRANCH_HH__
#define __GECODE_CPLTSET_BRANCH_HH__

#include <gecode/cpltset.hh>

/**
 * \namespace Gecode::CpltSet::Branch
 * \brief %CpltSet branchings
 */

namespace Gecode { namespace CpltSet { namespace Branch {

  /*
   * Value selection classes
   *
   */

  /**
   * \brief Class for selecting minimum value of unknown ranges
   *
   * All value selection classes require
   * \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelVal
   */
  template <bool inc>
  class ValMin : public ValSelBase<CpltSetView,int> {
  public:
    /// Default constructor
    ValMin(void);
    /// Constructor for initialization
    ValMin(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space&, CpltSetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, CpltSetView x, int v);
  };

  /**
   * \brief Class for selecting maximum value of unknown ranges
   *
   * All value selection classes require
   * \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelVal
   */
  template <bool inc>
  class ValMax : public ValSelBase<CpltSetView,int> {
  public:
    /// Default constructor
    ValMax(void);
    /// Constructor for initialization
    ValMax(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    int val(Space&, CpltSetView x) const;
    /// Tell \f$v\in x\f$ (\a a = 0) or \f$v\notin x\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, CpltSetView x, int v);
  };

  /*
   * View selection classes
   *
   */

  /**
   * \brief View selection class for view with smallest minimum element in lub-glb
   *
   * Requires \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelView
   */
  class ByMinMin : public ViewSelBase<CpltSetView> {
  private:
    /// So-far smallest minimum element
    int min;
  public:
    /// Default constructor
    ByMinMin(void);
    /// Constructor for initialization
    ByMinMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, CpltSetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, CpltSetView x);
  };

  /**
   * \brief View selection class for view with largest minimum element in lub-glb
   *
   * Requires \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelView
   */
  class ByMinMax : public ViewSelBase<CpltSetView> {
  private:
    /// So-far largest minimum element
    int min;
  public:
    /// Default constructor
    ByMinMax(void);
    /// Constructor for initialization
    ByMinMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, CpltSetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, CpltSetView x);
  };

  /**
   * \brief View selection class for view with smallest maximal element in lub-glb
   *
   * Requires \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelView
   */
  class ByMaxMin : public ViewSelBase<CpltSetView> {
  private:
    /// So-far smallest maximal element
    int max;
  public:
    /// Default constructor
    ByMaxMin(void);
    /// Constructor for initialization
    ByMaxMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, CpltSetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, CpltSetView x);
  };

  /**
   * \brief View selection class for view with largest maximal element in lub-glb
   *
   * Requires \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelView
   */
  class ByMaxMax : public ViewSelBase<CpltSetView> {
  private:
    /// So-far smallest maximal element
    int max;
  public:
    /// Default constructor
    ByMaxMax(void);
    /// Constructor for initialization
    ByMaxMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, CpltSetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, CpltSetView x);
  };

  /**
   * \brief View selection class for view with smallest cardinality of lub-glb
   *
   * Requires \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelView
   */
  class BySizeMin : public ViewSelBase<CpltSetView> {
  private:
    /// So-far smallest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMin(void);
    /// Constructor for initialization
    BySizeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, CpltSetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, CpltSetView x);
  };

  /**
   * \brief View selection class for view with largest cardinality of lub-glb
   *
   * Requires \code #include <gecode/cpltset/branch.hh> \endcode
   * \ingroup FuncCpltSetSelView
   */
  class BySizeMax : public ViewSelBase<CpltSetView> {
  private:
    /// So-far largest size
    unsigned int size;
  public:
    /// Default constructor
    BySizeMax(void);
    /// Constructor for initialization
    BySizeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x
    ViewSelStatus init(Space& home, CpltSetView x);
    /// Possibly select better view \a x
    ViewSelStatus select(Space& home, CpltSetView x);
  };

}}}

#include <gecode/cpltset/branch/select-val.hpp>
#include <gecode/cpltset/branch/select-view.hpp>
#include <gecode/cpltset/branch/post-val.hpp>

#endif

// STATISTICS: cpltset-branch
