/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *
 *  Copyright:
 *
 *  Last modified:
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

#ifndef __GECODE_FLOAT_BRANCH_HH__
#define __GECODE_FLOAT_BRANCH_HH__

#include <gecode/float.hh>

/**
 * \namespace Gecode::Float::Branch
 * \brief Float branchers
 */

namespace Gecode { namespace Float { namespace Branch {

  /*
   * Value selection classes
   *
   */

  /**
   * \brief Class for splitting domain at mean of smallest and largest element 
   * (lower half first)
   *
   * Requires
   * \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelVal
   */
  template<class View>
  class ValSplitMin : public ValSelBase<View,FloatNum> {
  public:
    /// Default constructor
    ValSplitMin(void);
    /// Constructor for initialization
    ValSplitMin(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    FloatNum val(Space& home, View x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x >n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, FloatNum n);
  };

  /**
   * \brief Class for splitting domain at mean of smallest and largest element
   * (upper half first)
   *
   * Requires
   * \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelVal
   */
  template<class View>
  class ValSplitMax : public ValSelBase<View,FloatNum> {
  public:
    /// Default constructor
    ValSplitMax(void);
    /// Constructor for initialization
    ValSplitMax(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x
    FloatNum val(Space& home, View x) const;
    /// Tell \f$x\geq n\f$ (\a a = 0) or \f$x <n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, FloatNum n);
  };


  /*
   * Variable selection classes
   *
   */

  /**
   * \brief View selection class for view with smallest min
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByMinMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest minimum
    FloatNum min;
  public:
    /// Default constructor
    ByMinMin(void);
    /// Constructor for initialization
    ByMinMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest min
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByMinMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest minimum
    FloatNum min;
  public:
    /// Default constructor
    ByMinMax(void);
    /// Constructor for initialization
    ByMinMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest max
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByMaxMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest maximum
    FloatNum max;
  public:
    /// Default constructor
    ByMaxMin(void);
    /// Constructor for initialization
    ByMaxMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest max
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByMaxMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest maximum
    FloatNum max;
  public:
    /// Default constructor
    ByMaxMax(void);
    /// Constructor for initialization
    ByMaxMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest width
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByWidthMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest width
    FloatNum width;
  public:
    /// Default constructor
    ByWidthMin(void);
    /// Constructor for initialization
    ByWidthMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest width
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByWidthMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest width
    FloatNum width;
  public:
    /// Default constructor
    ByWidthMax(void);
    /// Constructor for initialization
    ByWidthMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest Width divided
   * by degree.
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByWidthDegreeMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest width/degree
    FloatNum widthdegree;
  public:
    /// Default constructor
    ByWidthDegreeMin(void);
    /// Constructor for initialization
    ByWidthDegreeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest width divided
   * by degree.
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByWidthDegreeMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest width/degree
    FloatNum widthdegree;
  public:
    /// Default constructor
    ByWidthDegreeMax(void);
    /// Constructor for initialization
    ByWidthDegreeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest width divided by accumulated failure count
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByWidthAfcMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest width/afc
    FloatNum widthafc;
  public:
    /// Default constructor
    ByWidthAfcMin(void);
    /// Constructor for initialization
    ByWidthAfcMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest width divided by accumulated failure count
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class ByWidthAfcMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest width/afc
    FloatNum widthafc;
  public:
    /// Default constructor
    ByWidthAfcMax(void);
    /// Constructor for initialization
    ByWidthAfcMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

}}}

#include <gecode/float/branch/select-val.hpp>
#include <gecode/float/branch/select-view.hpp>
#include <gecode/float/branch/post-val-float.hpp>

#endif

// STATISTICS: float-branch
