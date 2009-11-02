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

#ifndef __GECODE_INT_ARITHMETIC_HH__
#define __GECODE_INT_ARITHMETIC_HH__

#include <gecode/int.hh>

#include <gecode/int/rel.hh>
#include <gecode/int/linear.hh>

/**
 * \namespace Gecode::Int::Arithmetic
 * \brief Numerical (arithmetic) propagators
 */

namespace Gecode { namespace Int { namespace Arithmetic {

  /**
   * \brief Bounds consistent absolute value propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class AbsBnd : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    AbsBnd(Space& home, bool share, AbsBnd& p);
    /// Constructor for posting
    AbsBnd(Home home, View x0, View x1);
  public:

    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is low unary.
     * Otherwise it is low binary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus  propagate(Space& home, const ModEventDelta& med);
    /// Post bounds consistent propagator \f$ |x_0|=x_1\f$
    static  ExecStatus  post(Home home, View x0, View x1);
  };

  /**
   * \brief Domain consistent absolute value propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class AbsDom : public BinaryPropagator<View,PC_INT_DOM> {
  protected:
    using BinaryPropagator<View,PC_INT_DOM>::x0;
    using BinaryPropagator<View,PC_INT_DOM>::x1;

    /// Constructor for cloning \a p
    AbsDom(Space& home, bool share, AbsDom& p);
    /// Constructor for posting
    AbsDom(Home home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is low binary.
     * If in stage for bounds propagation, the cost is
     * low binary. Otherwise it is high binary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus  propagate(Space& home, const ModEventDelta& med);
    /// Post domain consistent propagator \f$ |x_0|=x_1\f$
    static  ExecStatus  post(Home home, View x0, View x1);
  };



  /**
   * \brief Bounds consistent ternary maximum propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class MaxBnd : public TernaryPropagator<View,PC_INT_BND> {
  protected:
    using TernaryPropagator<View,PC_INT_BND>::x0;
    using TernaryPropagator<View,PC_INT_BND>::x1;
    using TernaryPropagator<View,PC_INT_BND>::x2;

    /// Constructor for cloning \a p
    MaxBnd(Space& home, bool share, MaxBnd& p);
    /// Constructor for posting
    MaxBnd(Home home, View x0, View x1, View x2);
  public:
    /// Constructor for rewriting \a p during cloning
    MaxBnd(Space& home, bool share, Propagator& p, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \max\{x_0,x_1\}=x_2\f$
    static  ExecStatus post(Home home, View x0, View x1, View x2);
  };

  /**
   * \brief Bounds consistent n-ary maximum propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class NaryMaxBnd : public NaryOnePropagator<View,PC_INT_BND> {
  protected:
    using NaryOnePropagator<View,PC_INT_BND>::x;
    using NaryOnePropagator<View,PC_INT_BND>::y;

    /// Constructor for cloning \a p
    NaryMaxBnd(Space& home, bool share, NaryMaxBnd& p);
    /// Constructor for posting
    NaryMaxBnd(Home home, ViewArray<View>& x, View y);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \max x=y\f$
    static  ExecStatus post(Home home, ViewArray<View>& x, View y);
  };

  /**
   * \brief Domain consistent ternary maximum propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class MaxDom : public TernaryPropagator<View,PC_INT_DOM> {
  protected:
    using TernaryPropagator<View,PC_INT_DOM>::x0;
    using TernaryPropagator<View,PC_INT_DOM>::x1;
    using TernaryPropagator<View,PC_INT_DOM>::x2;

    /// Constructor for cloning \a p
    MaxDom(Space& home, bool share, MaxDom& p);
    /// Constructor for posting
    MaxDom(Home home, View x0, View x1, View x2);
  public:
    /// Constructor for rewriting \a p during cloning
    MaxDom(Space& home, bool share, Propagator& p, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds propagation, the cost is
     * low ternary. Otherwise it is high ternary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \max\{x_0,x_1\}=x_2\f$
    static  ExecStatus post(Home home, View x0, View x1, View x2);
  };

  /**
   * \brief Domain consistent n-ary maximum propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class NaryMaxDom : public NaryOnePropagator<View,PC_INT_DOM> {
  protected:
    using NaryOnePropagator<View,PC_INT_DOM>::x;
    using NaryOnePropagator<View,PC_INT_DOM>::y;

    /// Constructor for cloning \a p
    NaryMaxDom(Space& home, bool share, NaryMaxDom& p);
    /// Constructor for posting
    NaryMaxDom(Home home, ViewArray<View>& x, View y);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds propagation, the cost is
     * low linear. Otherwise it is high linear.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \max x=y\f$
    static  ExecStatus post(Home home, ViewArray<View>& x, View y);
  };




  /**
   * \brief Bounds consistent positive square propagator
   *
   * This propagator provides multiplication for positive views only.
   */
  template<class VA, class VB>
  class SqrPlusBnd : public MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND> {
  protected:
    using MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>::x0;
    using MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>::x1;
    /// Constructor for posting
    SqrPlusBnd(Home home, VA x0, VB x1);
    /// Constructor for cloning \a p
    SqrPlusBnd(Space& home, bool share, SqrPlusBnd<VA,VB>& p);
  public:
    /// Post propagator \f$x_0\cdot x_0=x_1\f$
    static ExecStatus post(Home home, VA x0, VB x1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Bounds consistent square propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class SqrBnd : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    SqrBnd(Space& home, bool share, SqrBnd<View>& p);
    /// Constructor for posting
    SqrBnd(Home home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$x_0\cdot x_0=x_1\f$
    static ExecStatus post(Home home, View x0, View x1);
  };



  /**
   * \brief Domain consistent positive square propagator
   *
   * This propagator provides multiplication for positive views only.
   */
  template<class VA, class VB>
  class SqrPlusDom : public MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM> {
  protected:
    using MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM>::x0;
    using MixBinaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM>::x1;
    /// Constructor for posting
    SqrPlusDom(Home home, VA x0, VB x1);
    /// Constructor for cloning \a p
    SqrPlusDom(Space& home, bool share, SqrPlusDom<VA,VB>& p);
  public:
    /// Post propagator \f$x_0\cdot x_0=x_1\f$
    static ExecStatus post(Home home, VA x0, VB x1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is low unary.
     * If in stage for bounds propagation, the cost is
     * low binary. Otherwise it is high binary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Domain consistent square propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class SqrDom : public BinaryPropagator<View,PC_INT_DOM> {
  protected:
    using BinaryPropagator<View,PC_INT_DOM>::x0;
    using BinaryPropagator<View,PC_INT_DOM>::x1;

    /// Constructor for cloning \a p
    SqrDom(Space& home, bool share, SqrDom<View>& p);
    /// Constructor for posting
    SqrDom(Home home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is low unary.
     * If in stage for bounds propagation, the cost is
     * low binary. Otherwise it is high binary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Post propagator \f$x_0\cdot x_0=x_1\f$
    static ExecStatus post(Home home, View x0, View x1);
  };



  /**
   * \brief Bounds consistent square root propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class SqrtBnd : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    SqrtBnd(Space& home, bool share, SqrtBnd<View>& p);
    /// Constructor for posting
    SqrtBnd(Home home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$\lfloor\sqrt{x_0}\rfloor=x_1\f$
    static ExecStatus post(Home home, View x0, View x1);
  };

  /**
   * \brief Domain consistent square root propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class SqrtDom : public BinaryPropagator<View,PC_INT_DOM> {
  protected:
    using BinaryPropagator<View,PC_INT_DOM>::x0;
    using BinaryPropagator<View,PC_INT_DOM>::x1;

    /// Constructor for cloning \a p
    SqrtDom(Space& home, bool share, SqrtDom<View>& p);
    /// Constructor for posting
    SqrtDom(Home home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is low unary.
     * If in stage for bounds propagation, the cost is
     * low binary. Otherwise it is high binary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Post propagator \f$\lfloor\sqrt{x_0}\rfloor=x_1\f$
    static ExecStatus post(Home home, View x0, View x1);
  };



  /**
   * \brief Bounds or domain consistent propagator for \f$x_0\times x_1=x_0\f$
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class View, PropCond pc>
  class MultZeroOne : public BinaryPropagator<View,pc> {
  protected:
    using BinaryPropagator<View,pc>::x0;
    using BinaryPropagator<View,pc>::x1;

    /// Constructor for cloning \a p
    MultZeroOne(Space& home, bool share, MultZeroOne<View,pc>& p);
    /// Constructor for posting
    MultZeroOne(Home home, View x0, View x1);
    /// Test whether \a x is equal to \a n
    static RelTest equal(View x, int n);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$x_0\cdot x_1=x_0\f$
    static ExecStatus post(Home home, View x0, View x1);
  };



  /**
   * \brief Bounds consistent positive multiplication propagator
   *
   * This propagator provides multiplication for positive views only.
   */
  template<class Val, class VA, class VB, class VC>
  class MultPlusBnd :
    public MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND> {
  protected:
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x0;
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x1;
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x2;
  public:
    /// Constructor for posting
    MultPlusBnd(Home home, VA x0, VB x1, VC x2);
    /// Constructor for cloning \a p
    MultPlusBnd(Space& home, bool share, MultPlusBnd<Val,VA,VB,VC>& p);
    /// Post propagator \f$x_0\cdot x_1=x_2\f$
    static ExecStatus post(Home home, VA x0, VB x1, VC x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Bounds consistent multiplication propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<class View>
  class MultBnd : public TernaryPropagator<View,PC_INT_BND> {
  protected:
    using TernaryPropagator<View,PC_INT_BND>::x0;
    using TernaryPropagator<View,PC_INT_BND>::x1;
    using TernaryPropagator<View,PC_INT_BND>::x2;

    /// Constructor for cloning \a p
    MultBnd(Space& home, bool share, MultBnd<View>& p);
  public:
    /// Constructor for posting
    MultBnd(Home home, View x0, View x1, View x2);
    /// Post propagator \f$x_0\cdot x_1=x_2\f$
    static  ExecStatus post(Home home, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };



  /**
   * \brief Domain consistent positive multiplication propagator
   *
   * This propagator provides multiplication for positive views only.
   */
  template<class Val, class VA, class VB, class VC>
  class MultPlusDom :
    public MixTernaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM,VC,PC_INT_DOM> {
  protected:
    using MixTernaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM,VC,PC_INT_DOM>::x0;
    using MixTernaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM,VC,PC_INT_DOM>::x1;
    using MixTernaryPropagator<VA,PC_INT_DOM,VB,PC_INT_DOM,VC,PC_INT_DOM>::x2;
  public:
    /// Constructor for posting
    MultPlusDom(Home home, VA x0, VB x1, VC x2);
    /// Constructor for cloning \a p
    MultPlusDom(Space& home, bool share, MultPlusDom<Val,VA,VB,VC>& p);
    /// Post propagator \f$x_0\cdot x_1=x_2\f$
    static ExecStatus post(Home home, VA x0, VB x1, VC x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds propagation, the cost is
     * low ternary. Otherwise it is high ternary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Domain consistent multiplication propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<class View>
  class MultDom : public TernaryPropagator<View,PC_INT_DOM> {
  protected:
    using TernaryPropagator<View,PC_INT_DOM>::x0;
    using TernaryPropagator<View,PC_INT_DOM>::x1;
    using TernaryPropagator<View,PC_INT_DOM>::x2;

    /// Constructor for cloning \a p
    MultDom(Space& home, bool share, MultDom<View>& p);
  public:
    /// Constructor for posting
    MultDom(Home home, View x0, View x1, View x2);
    /// Post propagator \f$x_0\cdot x_1=x_2\f$
    static  ExecStatus post(Home home, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds propagation, the cost is
     * low ternary. Otherwise it is high ternary.
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Bounds consistent positive division propagator
   *
   * This propagator provides division for positive views only.
   */
  template<class Val, class VA, class VB, class VC>
  class DivPlusBnd :
    public MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND> {
  protected:
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x0;
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x1;
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x2;
  public:
    /// Constructor for posting
    DivPlusBnd(Home home, VA x0, VB x1, VC x2);
    /// Constructor for cloning \a p
    DivPlusBnd(Space& home, bool share,
               DivPlusBnd<Val,VA,VB,VC>& p);
    /// Post propagator \f$x_0\mathrm{div} x_1=x_2\f$ (rounding towards 0)
    static ExecStatus post(Home home, VA x0, VB x1, VC x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Bounds consistent division propagator
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<class View>
  class DivBnd : public TernaryPropagator<View,PC_INT_BND> {
  protected:
    using TernaryPropagator<View,PC_INT_BND>::x0;
    using TernaryPropagator<View,PC_INT_BND>::x1;
    using TernaryPropagator<View,PC_INT_BND>::x2;

    /// Constructor for cloning \a p
    DivBnd(Space& home, bool share, DivBnd<View>& p);
  public:
    /// Constructor for posting
    DivBnd(Home home, View x0, View x1, View x2);
    /// Post propagator \f$x_0\mathrm{div} x_1=x_2\f$ (rounding towards 0)
    static  ExecStatus post(Home home, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

  /**
   * \brief Integer division/modulo propagator
   *
   * This propagator implements the relation between divisor and
   * modulo of an integer division.
   *
   * Requires \code #include <gecode/int/arithmetic.hh> \endcode
   *
   * \ingroup FuncIntProp
   */
  template<class View>
  class DivMod : public TernaryPropagator<View,PC_INT_BND> {
  protected:
    using TernaryPropagator<View,PC_INT_BND>::x0;
    using TernaryPropagator<View,PC_INT_BND>::x1;
    using TernaryPropagator<View,PC_INT_BND>::x2;

    /// Constructor for cloning \a p
    DivMod(Space& home, bool share, DivMod<View>& p);
  public:
    /// Constructor for posting
    DivMod(Home home, View x0, View x1, View x2);
    /// Post propagator \f$x_1\neq 0 \land (x_2\neq 0\Rightarrow x_0\times x_2>0) \land \mathrm{abs}(x_2)<\mathrm{abs}(x_1)\f$
    static  ExecStatus post(Home home, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
  };

}}}

#include <gecode/int/arithmetic/abs.hpp>
#include <gecode/int/arithmetic/max.hpp>
#include <gecode/int/arithmetic/sqr.hpp>
#include <gecode/int/arithmetic/sqrt.hpp>
#include <gecode/int/arithmetic/mult.hpp>
#include <gecode/int/arithmetic/divmod.hpp>

#endif

// STATISTICS: int-prop

