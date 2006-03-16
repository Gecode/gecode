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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_INT_BOOL_HH__
#define __GECODE_INT_BOOL_HH__

#include "int.hh"

/**
 * \namespace Gecode::Int::Bool
 * \brief Boolean propagators
 */

namespace Gecode { namespace Int { namespace Bool {

  /*
   * Base Classes
   *
   */

  /// Base-class for binary Boolean propagators
  template<class BVA, class BVB>
  class BoolBinary : public Propagator {
  protected:
    BVA x0; ///< Boolean view
    BVB x1; ///< Boolean view
    /// Constructor for posting
    BoolBinary(Space* home, BVA b0, BVB b1);
    /// Constructor for cloning
    BoolBinary(Space* home, bool share, BoolBinary& p);
    /// Constructor for rewriting \a p during cloning
    BoolBinary(Space* home, bool share, Propagator& p,
	       BVA b0, BVB b1);
  public:
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(void) const;
    /// Destructor
    virtual ~BoolBinary(void);
  };

  /// Base-class for ternary Boolean propagators
  template<class BVA, class BVB, class BVC>
  class BoolTernary : public Propagator {
  protected:
    BVA x0; ///< Boolean view
    BVB x1; ///< Boolean view
    BVC x2; ///< Boolean view
    /// Constructor for posting
    BoolTernary(Space* home, BVA b0, BVB b1, BVC b2);
    /// Constructor for cloning
    BoolTernary(Space* home, bool share, BoolTernary& p);
  public:
    /// Constructor for rewriting \a p during cloning
    BoolTernary(Space* home, bool share, Propagator& p,
		BVA b0, BVB b1, BVC b2);
    /// Cost function (defined as PC_BINARY_LO)
    virtual PropCost cost(void) const;
    /// Destructor
    virtual ~BoolTernary(void);
  };

  /**
   * \brief Boolean equality propagator
   *
   * Requires \code #include "int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB>
  class Eq : public BoolBinary<BVA,BVB> {
  protected:
    using BoolBinary<BVA,BVB>::x0;
    using BoolBinary<BVA,BVB>::x1;
    /// Constructor for posting
    Eq(Space* home, BVA b0, BVB b1);
    /// Constructor for cloning \a p
    Eq(Space* home, bool share, Eq& p);
  public:
    /// Constructor for rewriting \a p during cloning
    Eq(Space* home, bool share, Propagator& p,
       BVA b0, BVB b1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 = b_1\f$
    static  ExecStatus post(Space* home, BVA b0, BVB b1);
  };


  /**
   * \brief Boolean conjunction propagator
   *
   * Requires \code #include "int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB, class BVC>
  class And : public BoolTernary<BVA,BVB,BVC> {
  protected:
    using BoolTernary<BVA,BVB,BVC>::x0;
    using BoolTernary<BVA,BVB,BVC>::x1;
    using BoolTernary<BVA,BVB,BVC>::x2;
    /// Constructor for posting
    And(Space* home, BVA b0, BVB b1, BVC b2);
    /// Constructor for cloning \a p
    And(Space* home, bool share, And& p);
  public:
    /// Constructor for rewriting \a p during cloning
    And(Space* home, bool share, Propagator& p,
	BVA b0, BVB b1, BVC b2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \land b_1 = b_2 \f$
    static  ExecStatus post(Space* home, BVA b0, BVB b1, BVC b2);
  };

  /**
   * \brief Boolean n-ary conjunction propagator
   *
   * Requires \code #include "int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class View>
  class NaryAnd : public NaryOnePropagator<View,PC_INT_VAL> {
  protected:
    using NaryOnePropagator<View,PC_INT_VAL>::x;
    using NaryOnePropagator<View,PC_INT_VAL>::y;
    /// Constructor for posting
    NaryAnd(Space* home,  ViewArray<View>& b, View c);
    /// Constructor for cloning \a p
    NaryAnd(Space* home, bool share, NaryAnd<View>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ \bigwedge_{i=0}^{|b|-1} b_i = c\f$
    static  ExecStatus post(Space* home, ViewArray<View>& b, View c);
  };


  /**
   * \brief Boolean equivalence propagator
   *
   * Requires \code #include "int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB, class BVC>
  class Eqv : public BoolTernary<BVA,BVB,BVC> {
  protected:
    using BoolTernary<BVA,BVB,BVC>::x0;
    using BoolTernary<BVA,BVB,BVC>::x1;
    using BoolTernary<BVA,BVB,BVC>::x2;
    /// Constructor for cloning \a p
    Eqv(Space* home, bool share, Eqv& p);
    /// Constructor for posting
    Eqv(Space* home, BVA b0 ,BVB b1, BVC b2);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \Leftrightarrow b_1 = b_2 \f$ (equivalence)
    static  ExecStatus post(Space* home, BVA b0, BVB b1, BVC b2);
  };

}}}

#include "int/bool/base.icc"
#include "int/bool/eq.icc"
#include "int/bool/and.icc"
#include "int/bool/eqv.icc"

#endif

// STATISTICS: int-prop

