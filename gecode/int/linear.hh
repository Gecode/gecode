/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Tias Guns <tias.guns@cs.kuleuven.be>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *     Tias Guns, 2009
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

#ifndef __GECODE_INT_LINEAR_HH__
#define __GECODE_INT_LINEAR_HH__

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::Linear
 * \brief %Linear propagators
 */

namespace Gecode { namespace Int { namespace Linear {

  /*
   * Binary propagators
   *
   */

  /**
   * \brief Base-class for binary linear propagators
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   */
  template <class Val, class A, class B, PropCond pc>
  class LinBin : public Propagator {
  protected:
    /// View of type \a A
    A x0;
    /// View of type \a B
    B x1;
    /// Value of type \a Val
    Val c;
    /// Constructor for cloning \a p
    LinBin(Space& home, bool share, LinBin& p);
    /// Constructor for rewriting \a p during cloning
    LinBin(Space& home, bool share, Propagator& p, A x0, B x1, Val c);
    /// Constructor for creation
    LinBin(Space& home, A x0, B x1, Val c);
  public:
    /// Cost function (defined as low binary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Base-class for reified binary linear propagators
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   */
  template <class Val, class A, class B, PropCond pc, class Ctrl>
  class ReLinBin : public Propagator {
  protected:
    /// View of type \a A
    A x0;
    /// View of type \a B
    B x1;
    /// Value of type \a Val
    Val c;
    /// Control view for reification
    Ctrl b;
    /// Constructor for cloning \a p
    ReLinBin(Space& home, bool share, ReLinBin& p);
    /// Constructor for creation
    ReLinBin(Space& home, A x0, B x1, Val c, Ctrl b);
  public:
    /// Cost function (defined as low binary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief %Propagator for bounds consistent binary linear equality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B>
  class EqBin : public LinBin<Val,A,B,PC_INT_BND> {
  protected:
    using LinBin<Val,A,B,PC_INT_BND>::x0;
    using LinBin<Val,A,B,PC_INT_BND>::x1;
    using LinBin<Val,A,B,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    EqBin(Space& home, bool share, EqBin& p);
    /// Constructor for creation
    EqBin(Space& home, A x0, B x1, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    EqBin(Space& home, bool share, Propagator& p, A x0, B x1, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$x_0+x_1 = c\f$
    static ExecStatus post(Space& home, A x0, B x1, Val c);
  };

  /**
   * \brief %Propagator for reified bounds consistent binary linear equality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B, class Ctrl>
  class ReEqBin : public ReLinBin<Val,A,B,PC_INT_BND,Ctrl> {
  protected:
    using ReLinBin<Val,A,B,PC_INT_BND,Ctrl>::x0;
    using ReLinBin<Val,A,B,PC_INT_BND,Ctrl>::x1;
    using ReLinBin<Val,A,B,PC_INT_BND,Ctrl>::c;
    using ReLinBin<Val,A,B,PC_INT_BND,Ctrl>::b;

    /// Constructor for cloning \a p
    ReEqBin(Space& home, bool share, ReEqBin& p);
    /// Constructor for creation
    ReEqBin(Space& home,A,B,Val,Ctrl);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$(x_0+x_1 = c)\Leftrightarrow b\f$
    static ExecStatus post(Space& home, A x0, B x1, Val c, Ctrl b);
  };

  /**
   * \brief %Propagator for bounds consistent binary linear disequality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B>
  class NqBin : public LinBin<Val,A,B,PC_INT_VAL> {
  protected:
    using LinBin<Val,A,B,PC_INT_VAL>::x0;
    using LinBin<Val,A,B,PC_INT_VAL>::x1;
    using LinBin<Val,A,B,PC_INT_VAL>::c;

    /// Constructor for cloning \a p
    NqBin(Space& home, bool share, NqBin& p);
    /// Constructor for creation
    NqBin(Space& home, A x0, B x1, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    NqBin(Space& home, bool share, Propagator& p, A x0, B x1, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Cost function (defined as low unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Post propagator for \f$x_0+x_1 \neq c\f$
    static ExecStatus post(Space& home, A x0, B x1, Val c);
  };

  /**
   * \brief %Propagator for bounds consistent binary linear less or equal
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B>
  class LqBin : public LinBin<Val,A,B,PC_INT_BND> {
  protected:
    using LinBin<Val,A,B,PC_INT_BND>::x0;
    using LinBin<Val,A,B,PC_INT_BND>::x1;
    using LinBin<Val,A,B,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    LqBin(Space& home, bool share, LqBin& p);
    /// Constructor for creation
    LqBin(Space& home, A x0, B x1, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    LqBin(Space& home, bool share, Propagator& p, A x0, B x1, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$x_0+x_1 \leq c\f$
    static ExecStatus post(Space& home, A x0, B x1, Val c);
  };

  /**
   * \brief %Propagator for bounds consistent binary linear greater or equal
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B>
  class GqBin : public LinBin<Val,A,B,PC_INT_BND> {
  protected:
    using LinBin<Val,A,B,PC_INT_BND>::x0;
    using LinBin<Val,A,B,PC_INT_BND>::x1;
    using LinBin<Val,A,B,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    GqBin(Space& home, bool share, GqBin& p);
    /// Constructor for creation
    GqBin(Space& home, A x0, B x1, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    GqBin(Space& home, bool share, Propagator& p, A x0, B x1, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$x_0+x_1 \geq c\f$
    static ExecStatus post(Space& home, A x0, B x1, Val c);
  };

  /**
   * \brief %Propagator for reified bounds consistent binary linear less or equal
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A and \a B
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B>
  class ReLqBin : public ReLinBin<Val,A,B,PC_INT_BND,BoolView> {
  protected:
    using ReLinBin<Val,A,B,PC_INT_BND,BoolView>::x0;
    using ReLinBin<Val,A,B,PC_INT_BND,BoolView>::x1;
    using ReLinBin<Val,A,B,PC_INT_BND,BoolView>::c;
    using ReLinBin<Val,A,B,PC_INT_BND,BoolView>::b;

    /// Constructor for cloning \a p
    ReLqBin(Space& home, bool share, ReLqBin& p);
    /// Constructor for creation
    ReLqBin(Space& home, A x0, B x1, Val c, BoolView b);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$(x_0+x_1 \leq c)\Leftrightarrow b\f$
    static ExecStatus post(Space& home, A x0, B x1, Val c, BoolView b);
  };

}}}

#include <gecode/int/linear/int-bin.hpp>

namespace Gecode { namespace Int { namespace Linear {

  /*
   * Ternary propagators
   *
   */

  /**
   * \brief Base-class for ternary linear propagators
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A, \a B,
   * and \a C give the types of the views.
   *
   * The propagation condition \a pc refers to all three views.
   */
  template <class Val, class A, class B, class C, PropCond pc>
  class LinTer : public Propagator {
  protected:
    /// View of type \a A
    A x0;
    /// View of type \a B
    B x1;
    /// View of type \a C
    C x2;
    /// Value of type \a Val
    Val c;
    /// Constructor for cloning \a p
    LinTer(Space& home, bool share, LinTer& p);
    /// Constructor for creation
    LinTer(Space& home, A x0, B x1, C x2, Val c);
    /// Constructor for rewriting \a p during cloning
    LinTer(Space& home, bool share, Propagator& p, A x0, B x1, C x2, Val c);
  public:
    /// Cost function (defined as low ternary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief %Propagator for bounds consistent ternary linear equality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A, \a B,
   * and \a C give the types of the views.
   *
   * The propagation condition \a pc refers to all three views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B, class C>
  class EqTer : public LinTer<Val,A,B,C,PC_INT_BND> {
  protected:
    using LinTer<Val,A,B,C,PC_INT_BND>::x0;
    using LinTer<Val,A,B,C,PC_INT_BND>::x1;
    using LinTer<Val,A,B,C,PC_INT_BND>::x2;
    using LinTer<Val,A,B,C,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    EqTer(Space& home, bool share, EqTer& p);
    /// Constructor for creation
    EqTer(Space& home, A x0, B x1, C x2, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    EqTer(Space& home, bool share, Propagator& p, A x0, B x1, C x2, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$x_0+x_1+x_2 = c\f$
    static ExecStatus post(Space& home, A x0, B x1, C x2, Val c);
  };

  /**
   * \brief %Propagator for bounds consistent ternary linear disquality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A, \a B,
   * and \a C give the types of the views.
   *
   * The propagation condition \a pc refers to all three views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B, class C>
  class NqTer : public LinTer<Val,A,B,C,PC_INT_VAL> {
  protected:
    using LinTer<Val,A,B,C,PC_INT_VAL>::x0;
    using LinTer<Val,A,B,C,PC_INT_VAL>::x1;
    using LinTer<Val,A,B,C,PC_INT_VAL>::x2;
    using LinTer<Val,A,B,C,PC_INT_VAL>::c;

    /// Constructor for cloning \a p
    NqTer(Space& home, bool share, NqTer& p);
    /// Constructor for creation
    NqTer(Space& home, A x0, B x1, C x2, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    NqTer(Space& home, bool share, Propagator& p, A x0, B x1, C x2, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$x_0+x_1+x_2 \neq c\f$
    static ExecStatus post(Space& home, A x0, B x1, C x2, Val c);
  };

  /**
   * \brief %Propagator for bounds consistent ternary linear less or equal
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a A, \a B,
   * and \a C give the types of the views.
   *
   * The propagation condition \a pc refers to all three views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class A, class B, class C>
  class LqTer : public LinTer<Val,A,B,C,PC_INT_BND> {
  protected:
    using LinTer<Val,A,B,C,PC_INT_BND>::x0;
    using LinTer<Val,A,B,C,PC_INT_BND>::x1;
    using LinTer<Val,A,B,C,PC_INT_BND>::x2;
    using LinTer<Val,A,B,C,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    LqTer(Space& home, bool share, LqTer& p);
    /// Constructor for creation
    LqTer(Space& home, A x0, B x1, C x2, Val c);
  public:
    /// Constructor for rewriting \a p during cloning
    LqTer(Space& home, bool share, Propagator& p, A x0, B x1, C x2, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$x_0+x_1+x_2 \leq c\f$
    static ExecStatus post(Space& home, A x0, B x1, C x2, Val c);
  };

}}}

#include <gecode/int/linear/int-ter.hpp>

namespace Gecode { namespace Int { namespace Linear {

  /*
   * n-ary propagators
   *
   */

  /**
   * \brief Base-class for n-ary linear propagators
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. Positive views are of
   * type \a P whereas negative views are of type \a N.
   *
   * The propagation condition \a pc refers to all views.
   */
  template <class Val, class P, class N, PropCond pc>
  class Lin : public Propagator {
  protected:
    /// Array of positive views
    ViewArray<P> x;
    /// Array of negative views
    ViewArray<N> y;
    /// Constant value
    Val c;

    /// Constructor for cloning \a p
    Lin(Space& home, bool share, Lin& p);
    /// Constructor for creation
    Lin(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Base-class for reified n-ary linear propagators
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. Positive views are of
   * type \a P whereas negative views are of type \a N.
   *
   * The propagation condition \a pc refers to all views.
   */
  template <class Val, class P, class N, PropCond pc, class Ctrl>
  class ReLin : public Lin<Val,P,N,pc> {
  protected:
    /// Control view for reification
    Ctrl b;
    /// Constructor for cloning \a p
    ReLin(Space& home, bool share, ReLin& p);
    /// Constructor for creation
    ReLin(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c, Ctrl b);
  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Compute bounds information for positive views
   *
   * \relates Lin
   */
  template <class Val, class View>
  void bounds_p(ModEventDelta med, ViewArray<View>& x,
                Val& c, Val& sl, Val& su);

  /**
   * \brief Compute bounds information for negative views
   *
   * \relates Lin
   */
  template <class Val, class View>
  void bounds_n(ModEventDelta med, ViewArray<View>& y,
                Val& c, Val& sl, Val& su);

  /**
   * \brief %Propagator for bounds consistent n-ary linear equality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a P and \a N
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class P, class N>
  class Eq : public Lin<Val,P,N,PC_INT_BND> {
  protected:
    using Lin<Val,P,N,PC_INT_BND>::x;
    using Lin<Val,P,N,PC_INT_BND>::y;
    using Lin<Val,P,N,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    Eq(Space& home, bool share, Eq& p);
  public:
    /// Constructor for creation
    Eq(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i=c\f$
    static ExecStatus
    post(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
  };

  /**
   * \brief %Propagator for domain consistent n-ary linear equality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a View
   * give the type of the view.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class View>
  class DomEq
    : public Lin<Val,View,View,PC_INT_DOM> {
  protected:
    using Lin<Val,View,View,PC_INT_DOM>::x;
    using Lin<Val,View,View,PC_INT_DOM>::y;
    using Lin<Val,View,View,PC_INT_DOM>::c;

    /// Constructor for cloning \a p
    DomEq(Space& home, bool share, DomEq& p);
  public:
    /// Constructor for creation
    DomEq(Space& home, ViewArray<View>& x, ViewArray<View>& y, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds propagation, the cost is
     * low linear. Otherwise it is high crazy.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i=c\f$
    static ExecStatus
    post(Space& home, ViewArray<View>& x, ViewArray<View>& y, Val c);
  };

  /**
   * \brief %Propagator for reified bounds consistent n-ary linear equality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a P and \a N
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class P, class N, class Ctrl>
  class ReEq : public ReLin<Val,P,N,PC_INT_BND,Ctrl> {
  protected:
    using ReLin<Val,P,N,PC_INT_BND,Ctrl>::x;
    using ReLin<Val,P,N,PC_INT_BND,Ctrl>::y;
    using ReLin<Val,P,N,PC_INT_BND,Ctrl>::c;
    using ReLin<Val,P,N,PC_INT_BND,Ctrl>::b;

    /// Constructor for cloning \a p
    ReEq(Space& home, bool share, ReEq& p);
  public:
    /// Constructor for creation
    ReEq(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c, Ctrl b);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i=c\right)\Leftrightarrow b\f$
    static ExecStatus
    post(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c, Ctrl b);
  };

  /**
   * \brief %Propagator for bounds consistent n-ary linear disequality
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a P and \a N
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class P, class N>
  class Nq : public Lin<Val,P,N,PC_INT_VAL> {
  protected:
    using Lin<Val,P,N,PC_INT_VAL>::x;
    using Lin<Val,P,N,PC_INT_VAL>::y;
    using Lin<Val,P,N,PC_INT_VAL>::c;

    /// Constructor for cloning \a p
    Nq(Space& home, bool share, Nq& p);
  public:
    /// Constructor for creation
    Nq(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i\neq c\f$
    static ExecStatus
    post(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
  };

  /**
   * \brief %Propagator for bounds consistent n-ary linear less or equal
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a P and \a N
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class P, class N>
  class Lq : public Lin<Val,P,N,PC_INT_BND> {
  protected:
    using Lin<Val,P,N,PC_INT_BND>::x;
    using Lin<Val,P,N,PC_INT_BND>::y;
    using Lin<Val,P,N,PC_INT_BND>::c;

    /// Constructor for cloning \a p
    Lq(Space& home, bool share, Lq& p);
  public:
    /// Constructor for creation
    Lq(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i\leq c\f$
    static ExecStatus
    post(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c);
  };

  /**
   * \brief %Propagator for reified bounds consistent n-ary linear less or equal
   *
   * The type \a Val can be either \c double or \c int, defining the
   * numerical precision during propagation. The types \a P and \a N
   * give the types of the views.
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class Val, class P, class N>
  class ReLq : public ReLin<Val,P,N,PC_INT_BND,BoolView> {
  protected:
    using ReLin<Val,P,N,PC_INT_BND,BoolView>::x;
    using ReLin<Val,P,N,PC_INT_BND,BoolView>::y;
    using ReLin<Val,P,N,PC_INT_BND,BoolView>::c;
    using ReLin<Val,P,N,PC_INT_BND,BoolView>::b;

    /// Constructor for cloning \a p
    ReLq(Space& home, bool share, ReLq& p);
  public:
    /// Constructor for creation
    ReLq(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c, BoolView b);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i\leq c\right)\Leftrightarrow b\f$
    static ExecStatus
    post(Space& home, ViewArray<P>& x, ViewArray<N>& y, Val c, BoolView b);
  };

}}}

#include <gecode/int/linear/int-nary.hpp>
#include <gecode/int/linear/int-dom.hpp>

namespace Gecode { namespace Int { namespace Linear {

  /*
   * Boolean linear propagators
   *
   */

  /**
   * \brief Baseclass for integer Boolean sum using dependencies
   *
   */
  template <class VX>
  class MemoryLinBoolInt : public Propagator {
  protected:
    /// Boolean views
    ViewArray<VX> x;
    /// Views from x[0] ... x[n_s-1] have subscriptions
    int n_s;
    /// Righthandside
    int c;
    /// Constructor for cloning \a p
    MemoryLinBoolInt(Space& home, bool share, MemoryLinBoolInt& p);
    /// Constructor for creation
    MemoryLinBoolInt(Space& home, ViewArray<VX>& x, int n_s, int c);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Baseclass for integer Boolean sum using advisors
   *
   */
  template <class VX>
  class SpeedLinBoolInt : public Propagator {
  protected:
    /// Boolean views
    ViewArray<VX> x;
    /// How many views are attached to advisors
    int n_s;
    /// Righthandside
    int c;
    /// Council for managing advisors
    Council<ViewAdvisor<VX> > co;
    /// Constructor for cloning \a p
    SpeedLinBoolInt(Space& home, bool share, SpeedLinBoolInt& p);
    /// Constructor for creation
    SpeedLinBoolInt(Space& home, ViewArray<VX>& x, int n_s, int c);
  public:
    /// Cost function (defined as high unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief %Propagator for integer equal to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class VX>
  class EqBoolInt {
  public:
    /// Threshold of whether to prefer speed or memory
    static const int threshold = 32;
    /// Propagator using less memory but with linear runtime
    class Memory : public MemoryLinBoolInt<VX> {
    protected:
      using MemoryLinBoolInt<VX>::x;
      using MemoryLinBoolInt<VX>::n_s;
      using MemoryLinBoolInt<VX>::c;
    public:
      /// Constructor for cloning \a p
      Memory(Space& home, bool share, Memory& p);
      /// Constructor for creation
      Memory(Space& home, ViewArray<VX>& x, int c);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
      /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i = c\f$
      static ExecStatus post(Space& home, ViewArray<VX>& x, int c);
    };
    /// Propagator using more memory but with constant runtime
    class Speed : public SpeedLinBoolInt<VX> {
    protected:
      using SpeedLinBoolInt<VX>::x;
      using SpeedLinBoolInt<VX>::n_s;
      using SpeedLinBoolInt<VX>::c;
      using SpeedLinBoolInt<VX>::co;
    public:
      /// Constructor for cloning \a p
      Speed(Space& home, bool share, Speed& p);
      /// Constructor for creation
      Speed(Space& home, ViewArray<VX>& x, int c);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Give advice to propagator
      virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    };
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i = c\f$
    static ExecStatus post(Space& home, ViewArray<VX>& x, int c);
  };

  /**
   * \brief %Propagator for integer less or equal to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class VX>
  class GqBoolInt {
  public:
    /// Threshold of whether to prefer speed or memory
    static const int threshold = 32;
    /// Propagator using less memory but with linear runtime
    class Memory : public MemoryLinBoolInt<VX> {
    protected:
      using MemoryLinBoolInt<VX>::x;
      using MemoryLinBoolInt<VX>::n_s;
      using MemoryLinBoolInt<VX>::c;
    public:
      /// Constructor for cloning \a p
      Memory(Space& home, bool share, Memory& p);
      /// Constructor for creation
      Memory(Space& home, ViewArray<VX>& x, int c);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    };
    /// Propagator using more memory but with constant runtime
    class Speed : public SpeedLinBoolInt<VX> {
    protected:
      using SpeedLinBoolInt<VX>::x;
      using SpeedLinBoolInt<VX>::n_s;
      using SpeedLinBoolInt<VX>::c;
      using SpeedLinBoolInt<VX>::co;
    public:
      /// Constructor for cloning \a p
      Speed(Space& home, bool share, Speed& p);
      /// Constructor for creation
      Speed(Space& home, ViewArray<VX>& x, int c);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Give advice to propagator
      virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    };
  public:
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i \geq c\f$
    static ExecStatus post(Space& home, ViewArray<VX>& x, int c);
  };

  /**
   * \brief %Propagator for integer disequal to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX>
  class NqBoolInt : public BinaryPropagator<VX,PC_INT_VAL> {
  protected:
    using BinaryPropagator<VX,PC_INT_VAL>::x0;
    using BinaryPropagator<VX,PC_INT_VAL>::x1;
    /// Views not yet subscribed to
    ViewArray<VX> x;
    /// Righthandside
    int c;
    /// Update subscription
    bool resubscribe(Space& home, VX& y);
    /// Constructor for posting
    NqBoolInt(Space& home,  ViewArray<VX>& b, int c);
    /// Constructor for cloning \a p
    NqBoolInt(Space& home, bool share, NqBoolInt<VX>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i \neq c\f$
    static  ExecStatus post(Space& home, ViewArray<VX>& b, int c);
  };


  /*
   * Reified boolean linear propagators
   *
   */

  /**
   * \brief Baseclass for reified integer Boolean sum using dependencies
   *
   */
  template <class VX, class VB>
  class MemoryReLinBoolInt : public MemoryLinBoolInt<VX> {
  protected:
    using MemoryLinBoolInt<VX>::x;
    using MemoryLinBoolInt<VX>::n_s;
    using MemoryLinBoolInt<VX>::c;
    /// Boolean view
    VB b;
    /// Constructor for cloning \a p
    MemoryReLinBoolInt(Space& home, bool share, MemoryReLinBoolInt& p);
    /// Constructor for creation
    MemoryReLinBoolInt(Space& home, ViewArray<VX>& x, int n_s, int c, VB b);
  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Baseclass for reified integer Boolean sum using advisors
   *
   */
  template <class VX, class VB>
  class SpeedReLinBoolInt : public Propagator {
  protected:
    /// How many Boolean views (all advised)
    int n;
    /// Righthandside
    int c;
    /// Council for managing advisors
    Council<ViewAdvisor<VX> > co;
    /// Control variable
    VB b;
    /// Constructor for cloning \a p
    SpeedReLinBoolInt(Space& home, bool share, SpeedReLinBoolInt& p);
    /// Constructor for creation
    SpeedReLinBoolInt(Space& home, ViewArray<VX>& x, int c, VB b);
  public:
    /// Cost function (defined as high unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };




  /**
   * \brief %Propagator for reified integer less or equal to Boolean sum (cardinality)
   *
   * Requires \code #include "gecode/int/linear.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VX, class VB>
  class ReGqBoolInt {
  public:
    /// Threshold of whether to prefer speed or memory
    static const int threshold = 32;
    /// Propagator using less memory but with linear runtime
    class Memory : public MemoryReLinBoolInt<VX,VB> {
    protected:
      using MemoryReLinBoolInt<VX,VB>::x;
      using MemoryReLinBoolInt<VX,VB>::n_s;
      using MemoryReLinBoolInt<VX,VB>::c;
      using MemoryReLinBoolInt<VX,VB>::b;
    public:
      /// Constructor for cloning \a p
      Memory(Space& home, bool share, Memory& p);
      /// Constructor for creation
      Memory(Space& home, ViewArray<VX>& x, int c, VB b);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
      /// Rewrite when reif is false, BoolView
      ExecStatus rewrite_inverse(Space& home, ViewArray<BoolView>& x, int c);
      /// Rewrite when reif is false, NegBoolView
      ExecStatus rewrite_inverse(Space& home, ViewArray<NegBoolView>& x, int c);
    };
    /// Propagator using more memory but with constant runtime
    class Speed : public SpeedReLinBoolInt<VX,VB> {
    protected:
      using SpeedReLinBoolInt<VX,VB>::n;
      using SpeedReLinBoolInt<VX,VB>::c;
      using SpeedReLinBoolInt<VX,VB>::co;
      using SpeedReLinBoolInt<VX,VB>::b;
    public:
      /// Constructor for cloning \a p
      Speed(Space& home, bool share, Speed& p);
      /// Constructor for creation
      Speed(Space& home, ViewArray<VX>& x, int c, VB b);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Give advice to propagator
      virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
      /// Rewrite when reif is false, BoolView
      ExecStatus rewrite_inverse(Space& home, ViewArray<BoolView>& x, int c);
      /// Rewrite when reif is false, NegBoolView
      ExecStatus rewrite_inverse(Space& home, ViewArray<NegBoolView>& x, int c);
    };
  public:
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i \geq c <=> b\f$
    static ExecStatus post(Space& home, ViewArray<VX>& x, int c, VB b);
  };

  /**
   * \brief %Propagator for reified integer equal to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class VX, class VB>
  class ReEqBoolInt {
  public:
    /// Threshold of whether to prefer speed or memory
    static const int threshold = 32;
    /// Propagator using less memory but with linear runtime
    class Memory : public MemoryReLinBoolInt<VX,VB> {
    protected:
      using MemoryReLinBoolInt<VX,VB>::x;
      using MemoryReLinBoolInt<VX,VB>::n_s;
      using MemoryReLinBoolInt<VX,VB>::c;
      using MemoryReLinBoolInt<VX,VB>::b;
    public:
      /// Constructor for cloning \a p
      Memory(Space& home, bool share, Memory& p);
      /// Constructor for creation
      Memory(Space& home, ViewArray<VX>& x, int c, VB b);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    };
    /// Propagator using more memory but with constant runtime
    class Speed : public SpeedReLinBoolInt<VX,VB> {
    protected:
      using SpeedReLinBoolInt<VX,VB>::n;
      using SpeedReLinBoolInt<VX,VB>::c;
      using SpeedReLinBoolInt<VX,VB>::co;
      using SpeedReLinBoolInt<VX,VB>::b;
    public:
      /// Constructor for cloning \a p
      Speed(Space& home, bool share, Speed& p);
      /// Constructor for creation
      Speed(Space& home, ViewArray<VX>& x, int c, VB b);
      /// Create copy during cloning
      virtual Actor* copy(Space& home, bool share);
      /// Give advice to propagator
      virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
      /// Perform propagation
      virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    };
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i = c\f$
    static ExecStatus post(Space& home, ViewArray<VX>& x, int c, VB b);
  };

}}}

#include <gecode/int/linear/bool-int.hpp>

namespace Gecode { namespace Int { namespace Linear {

  /**
   * \brief Base-class for Boolean linear propagators
   *
   */
  template <class XV, class YV>
  class LinBoolView : public Propagator {
  protected:
    /// Boolean views
    ViewArray<XV> x;
    /// View to compare number of assigned Boolean views to
    YV y;
    /// Righthandside (constant part from Boolean views assigned to 1)
    int c;
    /// Constructor for cloning \a p
    LinBoolView(Space& home, bool share, LinBoolView& p);
    /// Constructor for creation
    LinBoolView(Space& home, ViewArray<XV>& x, YV y, int c);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


  /**
   * \brief Base-class for reified Boolean linear propagators
   *
   */
  template <class XV, class YV, class BV>
  class ReLinBoolView : public LinBoolView<XV,YV> {
  protected:
    using LinBoolView<XV,YV>::x;
    using LinBoolView<XV,YV>::y;
    using LinBoolView<XV,YV>::c;
    /// Control variable
    BV b;
    /// Constructor for cloning \a p
    ReLinBoolView(Space& home, bool share, ReLinBoolView& p);
    /// Constructor for creation
    ReLinBoolView(Space& home, ViewArray<XV>& x, YV y, int c, BV b);
  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


  /**
   * \brief %Propagator for equality to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class XV, class YV>
  class EqBoolView : public LinBoolView<XV,YV> {
  protected:
    using LinBoolView<XV,YV>::x;
    using LinBoolView<XV,YV>::y;
    using LinBoolView<XV,YV>::c;

    /// Constructor for cloning \a p
    EqBoolView(Space& home, bool share, EqBoolView& p);
    /// Constructor for creation
    EqBoolView(Space& home, ViewArray<XV>& x, YV y, int c);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i = y+c\f$
    static ExecStatus post(Space& home, ViewArray<XV>& x, YV y, int c);
  };

  /**
   * \brief %Propagator for disequality to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class XV, class YV>
  class NqBoolView : public LinBoolView<XV,YV> {
  protected:
    using LinBoolView<XV,YV>::x;
    using LinBoolView<XV,YV>::y;
    using LinBoolView<XV,YV>::c;

    /// Constructor for cloning \a p
    NqBoolView(Space& home, bool share, NqBoolView& p);
    /// Constructor for creation
    NqBoolView(Space& home, ViewArray<XV>& x, YV y, int c);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i \neq y+c\f$
    static ExecStatus post(Space& home, ViewArray<XV>& x, YV y, int c);
  };

  /**
   * \brief %Propagator for greater or equal to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class XV, class YV>
  class GqBoolView : public LinBoolView<XV,YV> {
  protected:
    using LinBoolView<XV,YV>::x;
    using LinBoolView<XV,YV>::y;
    using LinBoolView<XV,YV>::c;

    /// Constructor for cloning \a p
    GqBoolView(Space& home, bool share, GqBoolView& p);
    /// Constructor for creation
    GqBoolView(Space& home, ViewArray<XV>& x, YV y, int c);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i \geq y+c\f$
    static ExecStatus post(Space& home, ViewArray<XV>& x, YV y, int c);
  };

  /**
   * \brief %Propagator for reified equality to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class XV, class YV, class BV>
  class ReEqBoolView : public ReLinBoolView<XV,YV,BV> {
  protected:
    using ReLinBoolView<XV,YV,BV>::x;
    using ReLinBoolView<XV,YV,BV>::y;
    using ReLinBoolView<XV,YV,BV>::c;
    using ReLinBoolView<XV,YV,BV>::b;

    /// Constructor for cloning \a p
    ReEqBoolView(Space& home, bool share, ReEqBoolView& p);
    /// Constructor for creation
    ReEqBoolView(Space& home, ViewArray<XV>& x, YV y, int c, BV b);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i = y+c\right)\Leftrightarrow b\f$
    static ExecStatus post(Space& home, ViewArray<XV>& x, YV y, int c, BV b);
  };

  /**
   * \brief %Propagator for reified greater or equal to Boolean sum (cardinality)
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class XV, class YV, class BV>
  class ReGqBoolView : public ReLinBoolView<XV,YV,BV> {
  protected:
    using ReLinBoolView<XV,YV,BV>::x;
    using ReLinBoolView<XV,YV,BV>::y;
    using ReLinBoolView<XV,YV,BV>::c;
    using ReLinBoolView<XV,YV,BV>::b;

    /// Constructor for cloning \a p
    ReGqBoolView(Space& home, bool share, ReGqBoolView& p);
    /// Constructor for creation
    ReGqBoolView(Space& home, ViewArray<XV>& x, YV y, int c, BV b);
  public:
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i \geq y+c\right)\Leftrightarrow b\f$
    static ExecStatus post(Space& home, ViewArray<XV>& x, YV y, int c, BV b);
  };

}}}

#include <gecode/int/linear/bool-view.hpp>

namespace Gecode { namespace Int { namespace Linear {

  /// Coefficient and Boolean view
  class ScaleBool {
  public:
    /// Integer coefficient
    int      a;
    /// Boolean view
    BoolView x;
  };

  /// Array of scale Boolean views
  class ScaleBoolArray {
  private:
    /// First entry in array
    ScaleBool* _fst;
    /// One after last entry in array
    ScaleBool* _lst;
  public:
    /// Default constructor
    ScaleBoolArray(void);
    /// Create array with \a n elements
    ScaleBoolArray(Space& home, int n);
    /// Subscribe propagator \a p
    void subscribe(Space& home, Propagator& p);
    /// Cancel propagator \a p
    void cancel(Space& home, Propagator& p);
    /// Update \a sba during copying
    void update(Space& home, bool share, ScaleBoolArray& sba);
    /// Return pointer to first element
    ScaleBool* fst(void) const;
    /// Return pointer after last element
    ScaleBool* lst(void) const;
    /// Set pointer to first element
    void fst(ScaleBool* f);
    /// Set pointer after last element
    void lst(ScaleBool* l);
    /// Test whether array is empty
    bool empty(void) const;
    /// Return number of elements
    int size(void) const;
  private:
    /// For sorting array in decreasing order of coefficients
    class ScaleDec {
    public:
      bool
      operator ()(const ScaleBool& x, const ScaleBool& y);
    };
  public:
    /// Sort array in decreasing order of coefficients
    void sort(void);
  };


  /// Empty array of scale Boolean views
  class EmptyScaleBoolArray {
  public:
    /// Default constructor
    EmptyScaleBoolArray(void);
    /// Create array with \a n elements
    EmptyScaleBoolArray(Space& home, int n);
    /// Subscribe propagator \a p
    void subscribe(Space& home, Propagator& p);
    /// Cancel propagator \a p
    void cancel(Space& home, Propagator& p);
    /// Update \a sba during copying
    void update(Space& home, bool share, EmptyScaleBoolArray& esba);
    /// Return pointer to first element
    ScaleBool* fst(void) const;
    /// Return pointer after last element
    ScaleBool* lst(void) const;
    /// Set pointer to first element
    void fst(ScaleBool* f);
    /// Set pointer after last element
    void lst(ScaleBool* l);
    /// Test whether array is empty
    bool empty(void) const;
    /// Return number of elements
    int size(void) const;
    /// Sort array in decreasing order of coefficients
    void sort(void);
  };


  /**
   * \brief Base class for linear Boolean constraints with coefficients
   *
   */
  template <class SBAP, class SBAN, class VX, PropCond pcx>
  class LinBoolScale : public Propagator {
  protected:
    /// Positive Boolean views with coefficients on left-hand side
    SBAP p;
    /// Negative Boolean views with coefficients on left-hand side
    SBAN n;
    /// Integer view on right-hand side
    VX   x;
    /// Integer constant on right-hand side
    int  c;
  public:
    /// Constructor for creation
    LinBoolScale(Space& home, SBAP& p, SBAN& n, VX x, int c);
    /// Constructor for cloning \a pr
    LinBoolScale(Space& home, bool share, Propagator& pr,
                 SBAP& p, SBAN& n, VX x, int c);
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief %Propagator for equality to Boolean sum with coefficients
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class SBAP, class SBAN, class VX>
  class EqBoolScale : public LinBoolScale<SBAP,SBAN,VX,PC_INT_BND> {
  protected:
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::p;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::n;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::x;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::c;
  public:
    /// Constructor for creation
    EqBoolScale(Space& home, SBAP& p, SBAN& n, VX x, int c);
    /// Constructor for cloning \a pr
    EqBoolScale(Space& home, bool share, Propagator& pr,
                SBAP& p, SBAN& n, VX x, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator
    static ExecStatus post(Space& home, SBAP& p, SBAN& n, VX x, int c);
  };

  /**
   * \brief %Propagator for inequality to Boolean sum with coefficients
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class SBAP, class SBAN, class VX>
  class LqBoolScale : public LinBoolScale<SBAP,SBAN,VX,PC_INT_BND> {
  protected:
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::p;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::n;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::x;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_BND>::c;
  public:
    /// Constructor for creation
    LqBoolScale(Space& home, SBAP& p, SBAN& n, VX x, int c);
    /// Constructor for cloning \a pr
    LqBoolScale(Space& home, bool share, Propagator& pr,
                SBAP& p, SBAN& n, VX x, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator
    static ExecStatus post(Space& home, SBAP& p, SBAN& n, VX x, int c);
  };

  /**
   * \brief %Propagator for disequality to Boolean sum with coefficients
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  template <class SBAP, class SBAN, class VX>
  class NqBoolScale : public LinBoolScale<SBAP,SBAN,VX,PC_INT_VAL> {
  protected:
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_VAL>::p;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_VAL>::n;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_VAL>::x;
    using LinBoolScale<SBAP,SBAN,VX,PC_INT_VAL>::c;
  public:
    /// Constructor for creation
    NqBoolScale(Space& home, SBAP& p, SBAN& n, VX x, int c);
    /// Constructor for cloning \a pr
    NqBoolScale(Space& home, bool share, Propagator& pr,
                SBAP& p, SBAN& n, VX x, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator
    static ExecStatus post(Space& home, SBAP& p, SBAN& n, VX x, int c);
  };

}}}

#include <gecode/int/linear/bool-scale.hpp>

namespace Gecode { namespace Int { namespace Linear {

  /**
   * \brief Class for describing linear term \f$a\cdot x\f$
   *
   */
  template<class View>
  class Term {
  public:
    /// Coefficient
    int a;
    /// View
    View x;
  };

  /** \brief Estimate lower and upper bounds
   *
   * Estimates the boundaries for a linear expression
   * \f$\sum_{i=0}^{n-1}t_i + c\f$. If the boundaries exceed
   * the limits as defined in Limits::Int, these boundaries
   * are returned.
   *
   * \param t array of linear terms
   * \param n size of array
   * \param c constant
   * \param l lower bound
   * \param u upper bound
   *
   */
  template<class View>
  void estimate(Term<View>* t, int n, int c,
                int& l, int& u);

  /** \brief Normalize linear integer constraints
   *
   * \param t array of linear terms
   * \param n size of array
   * \param t_p array of linear terms over integers with positive coefficients
   * \param n_p number of postive terms
   * \param t_n array of linear terms over integers with negative coefficients
   * \param n_n number of negative terms
   *
   * Replaces all negative coefficients by positive coefficients.
   *
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *
   * Returns true, if all coefficients are unit coefficients
   */
  template<class View>
  bool normalize(Term<View>* t, int &n,
                 Term<View>* &t_p, int &n_p,
                 Term<View>* &t_n, int &n_n);


  /**
   * \brief Post propagator for linear constraint over integers
   * \param t array of linear terms over integers
   * \param n size of array
   * \param r type of relation
   * \param c result of linear constraint
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume linear terms for the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for doubles as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  GECODE_INT_EXPORT void
  post(Space& home, Term<IntView>* t, int n, IntRelType r, int c,
       IntConLevel=ICL_DEF);

  /**
   * \brief Post reified propagator for linear constraint
   * \param t array of linear terms
   * \param n size of array
   * \param r type of relation
   * \param c result of linear constraint
   * \param b Boolean control view
   *
   * All variants for linear constraints share the following properties:
   *  - Only bounds consistency is supported.
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume linear terms for the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for doubles as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  GECODE_INT_EXPORT void
  post(Space& home, Term<IntView>* t, int n, IntRelType r, int c, BoolView b,
       IntConLevel=ICL_DEF);

  /**
   * \brief Post propagator for linear constraint over Booleans
   * \param t array of linear terms over Booleans
   * \param n size of array
   * \param r type of relation
   * \param c result of linear constraint
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume linear terms for the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for integers as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  GECODE_INT_EXPORT void
  post(Space& home, Term<BoolView>* t, int n, IntRelType r, int c,
       IntConLevel=ICL_DEF);

  /**
   * \brief Post propagator for reified linear constraint over Booleans
   * \param t array of linear terms over Booleans
   * \param n size of array
   * \param r type of relation
   * \param c result of linear constraint
   * \param b Boolean control varaible
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume linear terms for the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for integers as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  GECODE_INT_EXPORT void
  post(Space& home, Term<BoolView>* t, int n, IntRelType r, int c, BoolView b,
       IntConLevel=ICL_DEF);

  /**
   * \brief Post propagator for linear constraint over Booleans
   * \param t array of linear terms over Booleans
   * \param n size of array
   * \param r type of relation
   * \param y variable right hand side of linear constraint
   * \param c constant right hand side of linear constraint
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume linear terms for the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for integers as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  GECODE_INT_EXPORT void
  post(Space& home, Term<BoolView>* t, int n, IntRelType r, IntView y, int c=0,
       IntConLevel=ICL_DEF);

  /**
   * \brief Post propagator for reified linear constraint over Booleans
   * \param t array of linear terms over Booleans
   * \param n size of array
   * \param r type of relation
   * \param y variable right hand side of linear constraint
   * \param b Boolean control variable
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume linear terms for the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for integers as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   *
   * Requires \code #include <gecode/int/linear.hh> \endcode
   * \ingroup FuncIntProp
   */
  GECODE_INT_EXPORT void
  post(Space& home, Term<BoolView>* t, int n, IntRelType r, IntView y,
       BoolView b, IntConLevel=ICL_DEF);

}}}

#include <gecode/int/linear/post.hpp>

#endif

// STATISTICS: int-prop
