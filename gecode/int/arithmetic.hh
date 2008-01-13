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

#include "gecode/int.hh"

#include "gecode/int/rel.hh"
#include "gecode/int/linear.hh"

/**
 * \namespace Gecode::Int::Arithmetic
 * \brief Numerical (arithmetic) propagators
 */

namespace Gecode { namespace Int { namespace Arithmetic {

  /**
   * \brief Bounds-consistent absolute value propagator
   *
   * Requires \code #include "gecode/int/arithmetic.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class AbsBnd : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    AbsBnd(Space* home, bool share, AbsBnd& p);
    /// Constructor for posting
    AbsBnd(Space* home, View x0, View x1);
  public:

    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is PC_UNARY_LO.
     * Otherwise it is PC_BINARY_LO.
     */
    virtual PropCost cost(ModEventDelta med) const;
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home, ModEventDelta med);
    /// Post bounds-consistent propagator \f$ |x_0|=x_1\f$
    static  ExecStatus  post(Space* home, View x0, View x1);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

  /**
   * \brief Perform bounds-consistent absolute value propagation
   *
   * This is actually the propagation algorithm for AbsBnd.
   * It is available as separate function as it is reused for
   * domain-consistent distinct propagators.
   */
  template <class View>
  ExecStatus prop_bnd(Space* home, Propagator* p, ViewArray<View>&);

  /**
   * \brief Domain-consistent absolute value propagator
   *
   * Requires \code #include "gecode/int/arithmetic.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class AbsDom : public BinaryPropagator<View,PC_INT_DOM> {
  protected:
    using BinaryPropagator<View,PC_INT_DOM>::x0;
    using BinaryPropagator<View,PC_INT_DOM>::x1;

    /// Constructor for cloning \a p
    AbsDom(Space* home, bool share, AbsDom& p);
    /// Constructor for posting
    AbsDom(Space* home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is PC_UNARY_LO.
     * If in stage for bounds propagation, the cost is
     * PC_BINARY_LO. Otherwise it is PC_BINARY_HI.
     */
    virtual PropCost cost(ModEventDelta med) const;
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home, ModEventDelta med);
    /// Post domain-consistent propagator \f$ |x_0|=x_1\f$
    static  ExecStatus  post(Space* home, View x0, View x1);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

  /**
   * \brief Bounds-consistent ternary maximum propagator
   *
   * Requires \code #include "gecode/int/arithmetic.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Max : public TernaryPropagator<View,PC_INT_BND> {
  protected:
    using TernaryPropagator<View,PC_INT_BND>::x0;
    using TernaryPropagator<View,PC_INT_BND>::x1;
    using TernaryPropagator<View,PC_INT_BND>::x2;

    /// Constructor for cloning \a p
    Max(Space* home, bool share, Max& p);
    /// Constructor for posting
    Max(Space* home, View x0, View x1, View x2);
  public:
    /// Constructor for rewriting \a p during cloning
    Max(Space* home, bool share, Propagator& p, View x0, View x1, View x2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ \max\{x_0,x_1\}=x_2\f$
    static  ExecStatus post(Space* home, View x0, View x1, View x2);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

  /**
   * \brief Bounds-consistent n-ary maximum propagator
   *
   * Requires \code #include "gecode/int/arithmetic.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class NaryMax : public NaryOnePropagator<View,PC_INT_BND> {
  protected:
    using NaryOnePropagator<View,PC_INT_BND>::x;
    using NaryOnePropagator<View,PC_INT_BND>::y;

    /// Constructor for cloning \a p
    NaryMax(Space* home, bool share, NaryMax& p);
    /// Constructor for posting
    NaryMax(Space* home, ViewArray<View>& x, View y);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ \max x=y\f$
    static  ExecStatus post(Space* home, ViewArray<View>& x, View y);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };




  /**
   * \brief Bounds-consistent positive square propagator
   *
   * This propagator provides multiplication for positive views only.
   */
  template <class VA, class VB>
  class SquarePlus : public MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND> {
  protected:
    using MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>::x0;
    using MixBinaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND>::x1;
    /// Constructor for posting
    SquarePlus(Space* home, VA x0, VB x1);
    /// Constructor for cloning \a p
    SquarePlus(Space* home, bool share, SquarePlus<VA,VB>& p);
  public:
    /// Post propagator \f$x_0\cdot x_0=x_1\f$
    static ExecStatus post(Space* home, VA x0, VB x1);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

  /**
   * \brief Bounds-consistent square propagator
   *
   * Requires \code #include "gecode/int/arithmetic.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Square : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    Square(Space* home, bool share, Square<View>& p);
    /// Constructor for posting
    Square(Space* home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Cost function (defined as PC_BINARY_HI)
    virtual PropCost cost(ModEventDelta med) const;
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Post propagator \f$x_0\cdot x_0=x_1\f$
    static  ExecStatus post(Space* home, View x0, View x1);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

  /**
   * \brief Bounds-consistent positive multiplication propagator
   *
   * This propagator provides multiplication for positive views only.
   */
  template <class Val, class VA, class VB, class VC>
  class MultPlus : 
    public MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND> {
  protected:
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x0;
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x1;
    using MixTernaryPropagator<VA,PC_INT_BND,VB,PC_INT_BND,VC,PC_INT_BND>::x2;
  public:
    /// Constructor for posting
    MultPlus(Space* home, VA x0, VB x1, VC x2);
    /// Constructor for cloning \a p
    MultPlus(Space* home, bool share, MultPlus<Val,VA,VB,VC>& p);
    /// Post propagator \f$x_0\cdot x_1=x_2\f$
    static ExecStatus post(Space* home, VA x0, VB x1, VC x2);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

  /**
   * \brief Bounds-consistent multiplication propagator
   *
   * Requires \code #include "gecode/int/arithmetic.hh" \endcode
   *
   * \ingroup FuncIntProp
   */
  template <class View>
  class Mult : public TernaryPropagator<View,PC_INT_BND> {
  protected:
    using TernaryPropagator<View,PC_INT_BND>::x0;
    using TernaryPropagator<View,PC_INT_BND>::x1;
    using TernaryPropagator<View,PC_INT_BND>::x2;

    /// Constructor for cloning \a p
    Mult(Space* home, bool share, Mult<View>& p);
  public:
    /// Constructor for posting
    Mult(Space* home, View x0, View x1, View x2);
    /// Post propagator \f$x_0\cdot x_1=x_2\f$
    static  ExecStatus post(Space* home, View x0, View x1, View x2);
    /// Post propagator for specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Cost function (defined as PC_TERNARY_HI)
    virtual PropCost cost(ModEventDelta med) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
  };

}}}

#include "gecode/int/arithmetic/abs.icc"
#include "gecode/int/arithmetic/max.icc"
#include "gecode/int/arithmetic/mult.icc"

#endif

// STATISTICS: int-prop

