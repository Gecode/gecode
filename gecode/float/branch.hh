/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Vincent Barichard, 2012
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
    /// Return median value of view \a x
    FloatNum val(Space& home, View x) const;
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x > n\f$ (\a a = 1)
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
    /// Return median value of view \a x
    FloatNum val(Space& home, View x) const;
    /// Tell \f$x\geq n\f$ (\a a = 0) or \f$x < n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, View x, FloatNum n);
  };

  /**
   * \brief Class for splitting domain at mean of smallest and largest element
   * (randomly select upper or lower part first)
   *
   * Requires
   * \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelVal
   */
  template<class _View>
  class ValSplitRnd {
  protected:
    /// The random number generator
    ArchivedRandomGenerator r;
  public:
    /// View type
    typedef _View View;
    /// Value type
    typedef std::pair<FloatNum, bool> Val;
    /// Choice type
    typedef ArchivedRandomGenerator Choice;
    /// Number of alternatives
    static const unsigned int alternatives = 2;
    /// Default constructor
    ValSplitRnd(void);
    /// Constructor for initialization
    ValSplitRnd(Space& home, const ValBranchOptions& vbo);
    /// Return minimum value of view \a x at position \a i
    Val val(Space& home, _View x);
    /// Tell \f$x\leq n\f$ (\a a = 0) or \f$x > n\f$ (\a a = 1)
    ModEvent tell(Space& home, unsigned int a, _View x, Val rn);
    /// Return choice
    Choice choice(Space& home);
    /// Return choice
    Choice choice(const Space& home, Archive& e);
    /// Commit to choice
    void commit(Space& home, const Choice& c, unsigned a);
    /// Updating during cloning
    void update(Space& home, bool share, ValSplitRnd& vs);
    /// Delete value selection
    void dispose(Space& home);
  };

  /// Class for assigning median value of lower part of the interval
  template<class View>
  class AssignValMin : public ValSplitMin<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMin(void);
    /// Constructor for initialization
    AssignValMin(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning median value of upper part of the interval
  template<class View>
  class AssignValMax : public ValSplitMax<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValMax(void);
    /// Constructor for initialization
    AssignValMax(Space& home, const ValBranchOptions& vbo);
  };

  /// Class for assigning median value of a randomly chosen part of the interval
  template<class View>
  class AssignValRnd : public ValSplitRnd<View> {
  public:
    /// Number of alternatives
    static const unsigned int alternatives = 1;
    /// Default constructor
    AssignValRnd(void);
    /// Constructor for initialization
    AssignValRnd(Space& home, const ValBranchOptions& vbo);
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
   * \brief View selection class for view with smallest size
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest size
    FloatNum size;
  public:
    /// Default constructor
    BySizeMin(void);
    /// Constructor for initialization
    BySizeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest size
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest size
    FloatNum size;
  public:
    /// Default constructor
    BySizeMax(void);
    /// Constructor for initialization
    BySizeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest Size divided
   * by degree.
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeDegreeMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest size/degree
    FloatNum sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMin(void);
    /// Constructor for initialization
    BySizeDegreeMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest size divided
   * by degree.
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeDegreeMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest size/degree
    FloatNum sizedegree;
  public:
    /// Default constructor
    BySizeDegreeMax(void);
    /// Constructor for initialization
    BySizeDegreeMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest size divided by accumulated failure count
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeAfcMin : public ViewSelBase<FloatView> {
  protected:
    /// So-far smallest size/afc
    FloatNum sizeafc;
  public:
    /// Default constructor
    BySizeAfcMin(void);
    /// Constructor for initialization
    BySizeAfcMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with largest size divided by accumulated failure count
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeAfcMax : public ViewSelBase<FloatView> {
  protected:
    /// So-far largest size/afc
    FloatNum sizeafc;
  public:
    /// Default constructor
    BySizeAfcMax(void);
    /// Constructor for initialization
    BySizeAfcMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \i
    ViewSelStatus init(Space& home, FloatView x, int i);
    /// Possibly select better view \a x at position \i
    ViewSelStatus select(Space& home, FloatView x, int i);
  };

  /**
   * \brief View selection class for view with smallest size divided by activity
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeActivityMin : public ViewSelBase<FloatView> {
  protected:
    /// Activity information
    Activity activity;
    /// So-far smallest size/activity
    double sizeact;
  public:
    /// Default constructor
    BySizeActivityMin(void);
    /// Constructor for initialization
    BySizeActivityMin(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, BySizeActivityMin& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief View selection class for view with largest size divided by activity
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class BySizeActivityMax : public ViewSelBase<FloatView> {
  protected:
    /// Activity information
    Activity activity;
    /// So-far largest size/activity
    double sizeact;
  public:
    /// Default constructor
    BySizeActivityMax(void);
    /// Constructor for initialization
    BySizeActivityMax(Space& home, const VarBranchOptions& vbo);
    /// Intialize with view \a x at position \a i
    ViewSelStatus init(Space& home, View x, int i);
    /// Possibly select better view \a x at position \a i
    ViewSelStatus select(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, BySizeActivityMax& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

}}}

#include <gecode/float/branch/select-val.hpp>
#include <gecode/float/branch/select-view.hpp>
#include <gecode/float/branch/post-val-float.hpp>

#endif

// STATISTICS: float-branch
