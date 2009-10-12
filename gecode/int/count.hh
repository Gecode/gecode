/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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

#ifndef __GECODE_INT_COUNT_HH__
#define __GECODE_INT_COUNT_HH__

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::Count
 * \brief Counting propagators
 */

namespace Gecode { namespace Int { namespace Count {

  /**
   * Relations for domain consistent counting
   *
   */
  //@{
  /// Test whether \a x and \a y are equal
  template<class VX>
  RelTest holds(VX x, VX y);
  /// Test whether \a x and \a y are equal
  template<class VX>
  RelTest holds(VX x, ConstIntView y);
  /// Post that all views in \a x are equal to \a y
  template<class VX>
  bool post_true(Home home, ViewArray<VX>& x, VX y);
  /// Post that all views in \a x are equal to \a y
  template<class VX>
  bool post_true(Home home, ViewArray<VX>& x, ConstIntView y);
  /// Post that all views in \a x are not equal to \a y
  template<class VX>
  bool post_false(Home home, ViewArray<VX>& x, VX y);
  /// Post that all views in \a x are not equal to \a y
  template<class VX>
  bool post_false(Home home, ViewArray<VX>& x, ConstIntView y);
  //@}

}}}

#include <gecode/int/count/rel.hpp>


namespace Gecode { namespace Int { namespace Count {

  /**
   * \brief Baseclass for count propagators (integer)
   *
   */
  template<class VX, class VY>
  class BaseInt : public Propagator {
  protected:
    /// Views still to count
    ViewArray<VX> x;
    /// Views from x[0] ... x[n_s-1] have subscriptions
    int n_s;
    /// View to compare to
    VY y;
    /// Number of views which are equal and have been eliminated
    int c;
    /// Constructor for cloning \a p
    BaseInt(Space& home, bool share, BaseInt& p);
    /// Constructor for creation
    BaseInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief %Propagator for counting views (equal integer to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY>
  class EqInt : public BaseInt<VX,VY> {
  protected:
    using BaseInt<VX,VY>::x;
    using BaseInt<VX,VY>::n_s;
    using BaseInt<VX,VY>::y;
    using BaseInt<VX,VY>::c;
    /// Constructor for cloning \a p
    EqInt(Space& home, bool share, EqInt& p);
    /// Constructor for creation
    EqInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, int c);
  };

  /**
   * \brief %Propagator for counting views (greater or equal integer to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY>
  class GqInt : public BaseInt<VX,VY> {
  protected:
    using BaseInt<VX,VY>::x;
    using BaseInt<VX,VY>::n_s;
    using BaseInt<VX,VY>::y;
    using BaseInt<VX,VY>::c;
    /// Constructor for cloning \a p
    GqInt(Space& home, bool share, GqInt& p);
    /// Constructor for creation
    GqInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, int c);
  };

  /**
   * \brief %Propagator for counting views (less or equal integer to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY>
  class LqInt : public BaseInt<VX,VY> {
  protected:
    using BaseInt<VX,VY>::x;
    using BaseInt<VX,VY>::n_s;
    using BaseInt<VX,VY>::y;
    using BaseInt<VX,VY>::c;
    /// Constructor for cloning \a p
    LqInt(Space& home, bool share, LqInt& p);
    /// Constructor for creation
    LqInt(Home home, ViewArray<VX>& x, int n_s, VY y, int c);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, int c);
  };

  /**
   * \brief %Propagator for counting views (not equal integer to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY>
  class NqInt : public BinaryPropagator<VX,PC_INT_DOM> {
  protected:
    using BinaryPropagator<VX,PC_INT_DOM>::x0;
    using BinaryPropagator<VX,PC_INT_DOM>::x1;
    /// Views not yet subscribed to
    ViewArray<VX> x;
    /// View to compare with
    VY y;
    /// Righthandside
    int c;
    /// Update subscription
    bool resubscribe(Space& home, VX& z);
    /// Constructor for posting
    NqInt(Home home,  ViewArray<VX>& x, VY y, int c);
    /// Constructor for cloning \a p
    NqInt(Space& home, bool share, NqInt& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\neq c\f$
    static  ExecStatus post(Home home, ViewArray<VX>& x, VY y, int c);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

}}}

#include <gecode/int/count/int.hpp>


namespace Gecode { namespace Int { namespace Count {

  /**
   * \brief Base-class for count propagators (view)
   *
   */
  template<class VX, class VY, class VZ, bool shr>
  class BaseView : public Propagator {
  protected:
    /// Views still to count
    ViewArray<VX> x;
    /// View to compare to
    VY y;
    /// View which yields result of counting
    VZ z;
    /// Number of views which are equal and have been eliminated
    int   c;
    /// Constructor for cloning \a p
    BaseView(Space& home, bool share, BaseView& p);
    /// Constructor for creation
    BaseView(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
  protected:
    /// Count how many views are equal now
    void count(Space& home);
    /// How many views are at least equal
    int atleast(void) const;
    /// How many views are at most equal
    int atmost(void) const;
    /// Test whether there is sharing of \a z with \a x or \a y
    static bool sharing(const ViewArray<VX>& x, const VY& y, const VZ& z);
  };

  /**
   * \brief %Propagator for counting views (equal to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY, class VZ, bool shr>
  class EqView : public BaseView<VX,VY,VZ,shr> {
  protected:
    using BaseView<VX,VY,VZ,shr>::x;
    using BaseView<VX,VY,VZ,shr>::z;
    using BaseView<VX,VY,VZ,shr>::c;
    using BaseView<VX,VY,VZ,shr>::y;
    using BaseView<VX,VY,VZ,shr>::count;
    using BaseView<VX,VY,VZ,shr>::atleast;
    using BaseView<VX,VY,VZ,shr>::atmost;

    /// Constructor for cloning \a p
    EqView(Space& home, bool share, EqView& p);
  public:
    /// Constructor for creation
    EqView(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=z+c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

  /**
   * \brief %Propagator for counting views (different from number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY, class VZ, bool shr>
  class NqView : public BaseView<VX,VY,VZ,shr> {
  protected:
    using BaseView<VX,VY,VZ,shr>::x;
    using BaseView<VX,VY,VZ,shr>::z;
    using BaseView<VX,VY,VZ,shr>::c;
    using BaseView<VX,VY,VZ,shr>::y;
    using BaseView<VX,VY,VZ,shr>::count;
    using BaseView<VX,VY,VZ,shr>::atleast;
    using BaseView<VX,VY,VZ,shr>::atmost;

    /// Constructor for cloning \a p
    NqView(Space& home, bool share, NqView& p);
  public:
    /// Constructor for creation
    NqView(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\neq z+c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

  /**
   * \brief %Propagator for counting views (less or equal to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY, class VZ, bool shr>
  class LqView : public BaseView<VX,VY,VZ,shr> {
  protected:
    using BaseView<VX,VY,VZ,shr>::x;
    using BaseView<VX,VY,VZ,shr>::z;
    using BaseView<VX,VY,VZ,shr>::c;
    using BaseView<VX,VY,VZ,shr>::y;
    using BaseView<VX,VY,VZ,shr>::count;
    using BaseView<VX,VY,VZ,shr>::atleast;
    using BaseView<VX,VY,VZ,shr>::atmost;

    /// Constructor for cloning \a p
    LqView(Space& home, bool share, LqView& p);
  public:
    /// Constructor for creation
    LqView(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq z+c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

  /**
   * \brief %Propagator for counting views (greater or equal to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Requires \code #include <gecode/int/count.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY, class VZ, bool shr>
  class GqView : public BaseView<VX,VY,VZ,shr> {
  protected:
    using BaseView<VX,VY,VZ,shr>::x;
    using BaseView<VX,VY,VZ,shr>::z;
    using BaseView<VX,VY,VZ,shr>::c;
    using BaseView<VX,VY,VZ,shr>::y;
    using BaseView<VX,VY,VZ,shr>::count;
    using BaseView<VX,VY,VZ,shr>::atleast;
    using BaseView<VX,VY,VZ,shr>::atmost;

    /// Constructor for cloning \a p
    GqView(Space& home, bool share, GqView& p);
  public:
    /// Constructor for creation
    GqView(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq z+c\f$
    static ExecStatus post(Home home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

}}}

#include <gecode/int/count/view.hpp>

#endif

// STATISTICS: int-prop

