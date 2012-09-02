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
   * View merit classes
   *
   */

  /**
   * \brief Merit class for mimimum
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class MeritMin : public MeritBase<FloatView> {
  public:
    /// Default constructor
    MeritMin(void);
    /// Constructor for initialization
    MeritMin(Space& home, const VarBranchOptions& vbo);
    /// Return minimum as merit for view \a x at position \a i
    double operator ()(Space& home, FloatView x, int i);
  };

  /**
   * \brief Merit class for maximum
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class MeritMax : public MeritBase<FloatView> {
  public:
    /// Default constructor
    MeritMax(void);
    /// Constructor for initialization
    MeritMax(Space& home, const VarBranchOptions& vbo);
    /// Return maximum as merit for view \a x at position \a i
    double operator ()(Space& home, FloatView x, int i);
  };

  /**
   * \brief Merit class for size
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class MeritSize : public MeritBase<FloatView> {
  public:
    /// Default constructor
    MeritSize(void);
    /// Constructor for initialization
    MeritSize(Space& home, const VarBranchOptions& vbo);
    /// Return size as merit for view \a x at position \a i
    double operator ()(Space& home, FloatView x, int i);
  };

  /**
   * \brief Merit class for size over degree
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class MeritSizeDegree : public MeritBase<FloatView> {
  public:
    /// Default constructor
    MeritSizeDegree(void);
    /// Constructor for initialization
    MeritSizeDegree(Space& home, const VarBranchOptions& vbo);
    /// Return size over degree as merit for view \a x at position \a i
    double operator ()(Space& home, FloatView x, int i);
  };

  /**
   * \brief Merit class for size over afc
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class MeritSizeAfc : public MeritBase<FloatView> {
  public:
    /// Default constructor
    MeritSizeAfc(void);
    /// Constructor for initialization
    MeritSizeAfc(Space& home, const VarBranchOptions& vbo);
    /// Return size over AFC as merit for view \a x at position \a i
    double operator ()(Space& home, FloatView x, int i);
  };

  /**
   * \brief Merit class for size over activity
   *
   * Requires \code #include <gecode/float/branch.hh> \endcode
   * \ingroup FuncFloatSelView
   */
  class MeritSizeActivity : public MeritBase<FloatView> {
    /// Activity information
    Activity activity;
  public:
    /// Default constructor
    MeritSizeActivity(void);
    /// Constructor for initialization
    MeritSizeActivity(Space& home, const VarBranchOptions& vbo);
    /// Return size over activity as merit for view \a x at position \a i
    double operator ()(Space& home, FloatView x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, MeritSizeActivity& msa);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

}}}

#include <gecode/float/branch/select-val.hpp>
#include <gecode/float/branch/select-view.hpp>
#include <gecode/float/branch/post-val.hpp>

#endif

// STATISTICS: float-branch
