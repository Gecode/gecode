/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *     Gabor Szokoli, 2003
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

#ifndef __GECODE_INT_REL_HH__
#define __GECODE_INT_REL_HH__

#include "int.hh"

#include "iter.hh"

/**
 * \namespace Gecode::Int::Rel
 * \brief Simple relation propagators
 */

namespace Gecode { namespace Int { namespace Rel {

  /*
   * Equality propagators
   *
   */

  /**
   * \brief Binary domain-consistent equality propagator
   *
   * Uses staging by first performing bounds propagation and only
   * then domain propagation.
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class EqDom : public BinaryPropagator<View,PC_INT_DOM> {
  protected:
    using BinaryPropagator<View,PC_INT_DOM>::x0;
    using BinaryPropagator<View,PC_INT_DOM>::x1;

    /// Constructor for cloning \a p
    EqDom(Space* home, bool share, EqDom<View>& p);
    /// Constructor for posting
    EqDom(Space* home, View x0, View x1);
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
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post domain-consistent propagator \f$ x_0 = x_1\f$
    static  ExecStatus post(Space* home, View x0, View x1);
  };

  /**
   * \brief Binary bounds-consistent equality propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class EqBnd : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    EqBnd(Space* home, bool share, EqBnd<View>& p);
    /// Constructor for posting
    EqBnd(Space* home, View x0, View x1);
  public:
    /// Constructor for rewriting \a p during cloning
    EqBnd(Space* home, bool share, Propagator& p, View x0, View x1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post bounds-consistent propagator \f$ x_0 = x_1\f$
    static  ExecStatus post(Space* home, View x0, View x1);
  };

  /**
   * \brief n-ary domain-consistent equality propagator
   *
   * Uses staging by first performing bounds propagation and only
   * then domain propagation.
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class NaryEqDom : public NaryPropagator<View,PC_INT_DOM> {
  protected:
    using NaryPropagator<View,PC_INT_DOM>::x;

    /// Constructor for cloning \a p
    NaryEqDom(Space* home, bool share, NaryEqDom<View>& p);
    /// Constructor for posting
    NaryEqDom(Space* home, ViewArray<View>&);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is PC_UNARY_LO. 
     * If in stage for bounds propagation, the cost is dynamic
     * PC_LINEAR_LO. Otherwise it is dynamic PC_LINEAR_HI.
     */
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post domain-consistent propagator \f$ x_0 = x_1=\ldots =x_{|x|-1}\f$
    static  ExecStatus post(Space* home, ViewArray<View>& x);
  };

  /**
   * \brief n-ary bounds-consistent equality propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class NaryEqBnd : public NaryPropagator<View,PC_INT_BND> {
  protected:
    using NaryPropagator<View,PC_INT_BND>::x;

    /// Constructor for cloning \a p
    NaryEqBnd(Space* home, bool share, NaryEqBnd<View>& p);
    /// Constructor for posting
    NaryEqBnd(Space* home, ViewArray<View>&);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * If a view has been assigned, the cost is PC_UNARY_LO. 
     * Otherwise it is dynamic PC_LINEAR_HI.
     */
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post bounds-consistent propagator \f$ x_0 = x_1=\ldots =x_{|x|-1}\f$
    static  ExecStatus post(Space* home, ViewArray<View>& x);
  };

  /**
   * \brief Reified binary domain-consistent equality propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, class CtrlView>
  class ReEqDom : public ReBinaryPropagator<View,PC_INT_DOM,CtrlView> {
  protected:
    using ReBinaryPropagator<View,PC_INT_DOM,CtrlView>::x0;
    using ReBinaryPropagator<View,PC_INT_DOM,CtrlView>::x1;
    using ReBinaryPropagator<View,PC_INT_DOM,CtrlView>::b;

    /// Constructor for cloning \a p
    ReEqDom(Space* home, bool share, ReEqDom& p);
    /// Constructor for posting
    ReEqDom(Space* home, View x0, View x1, CtrlView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post domain-consistent propagator \f$ (x_0 = x_1)\Leftrightarrow b\f$
    static  ExecStatus post(Space* home, View x0, View x1, CtrlView b);
  };

  /**
   * \brief Reified binary bounds-consistent equality propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, class CtrlView>
  class ReEqBnd : public ReBinaryPropagator<View,PC_INT_BND,CtrlView> {
  protected:
    using ReBinaryPropagator<View,PC_INT_BND,CtrlView>::x0;
    using ReBinaryPropagator<View,PC_INT_BND,CtrlView>::x1;
    using ReBinaryPropagator<View,PC_INT_BND,CtrlView>::b;

    /// Constructor for cloning \a p
    ReEqBnd(Space* home, bool share, ReEqBnd& p);
    /// Constructor for posting
    ReEqBnd(Space* home, View x0, View x1, CtrlView b);
  public:
    /// Copy propagator during cloning
    virtual Actor*     copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post bounds-consistent propagator \f$ (x_0 = x_1)\Leftrightarrow b\f$
    static  ExecStatus post(Space* home, View x0, View x1, CtrlView b);
  };

  /**
   * \brief Reified domain-consistent equality with integer propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, class CtrlView>
  class ReEqDomInt : public ReUnaryPropagator<View,PC_INT_DOM,CtrlView> {
  protected:
    using ReUnaryPropagator<View,PC_INT_DOM,CtrlView>::x0;
    using ReUnaryPropagator<View,PC_INT_DOM,CtrlView>::b;

    /// Integer constant to check
    int c;
    /// Constructor for cloning \a p
    ReEqDomInt(Space* home, bool share, ReEqDomInt& p);
    /// Constructor for posting
    ReEqDomInt(Space* home, View x, int c, CtrlView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post domain-consistent propagator \f$ (x = c)\Leftrightarrow b\f$
    static  ExecStatus post(Space* home, View x, int c, CtrlView b);
  };

  /**
   * \brief Reified bounds-consistent equality with integer propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, class CtrlView>
  class ReEqBndInt : public ReUnaryPropagator<View,PC_INT_BND,CtrlView> {
  protected:
    using ReUnaryPropagator<View,PC_INT_BND,CtrlView>::x0;
    using ReUnaryPropagator<View,PC_INT_BND,CtrlView>::b;

    /// Integer constant to check
    int c;
    /// Constructor for cloning \a p
    ReEqBndInt(Space* home, bool share, ReEqBndInt& p);
    /// Constructor for posting
    ReEqBndInt(Space* home, View x, int c, CtrlView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post bounds-consistent propagator \f$ (x = c)\Leftrightarrow b\f$
    static  ExecStatus post(Space* home, View x, int c, CtrlView b);
  };




  /*
   * Disequality propagators
   *
   */

  /**
   * \brief Binary disequality propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View>
  class Nq : public BinaryPropagator<View,PC_INT_VAL> {
  protected:
    using BinaryPropagator<View,PC_INT_VAL>::x0;
    using BinaryPropagator<View,PC_INT_VAL>::x1;

    /// Constructor for cloning \a p
    Nq(Space* home, bool share, Nq<View>& p);
    /// Constructor for posting
    Nq(Space* home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$x_0=x_1\f$
    static  ExecStatus post(Space* home, View x0, View x1);
  };

  /*
   * Order propagators
   *
   */

  /**
   * \brief Less or equal propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View>
  class Lq : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    Lq(Space* home, bool share, Lq& p);
    /// Constructor for posting
    Lq(Space* home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor*     copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$x_0 \leq x_1\f$
    static  ExecStatus post(Space* home, View x0, View x1);
  };

  /**
   * \brief Less propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View>
  class Le : public BinaryPropagator<View,PC_INT_BND> {
  protected:
    using BinaryPropagator<View,PC_INT_BND>::x0;
    using BinaryPropagator<View,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    Le(Space* home, bool share, Le& p);
    /// Constructor for posting
    Le(Space* home, View x0, View x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$x_0 \le x_1\f$
    static  ExecStatus post(Space* home, View x0, View x1);
  };



  /*
   * Reified order propagators
   *
   */

  /**
   * \brief Reified less or equal propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View, class CtrlView>
  class ReLq : public ReBinaryPropagator<View,PC_INT_BND,CtrlView> {
  protected:
    using ReBinaryPropagator<View,PC_INT_BND,CtrlView>::x0;
    using ReBinaryPropagator<View,PC_INT_BND,CtrlView>::x1;
    using ReBinaryPropagator<View,PC_INT_BND,CtrlView>::b;

    /// Constructor for cloning \a p
    ReLq(Space* home, bool share, ReLq& p);
    /// Constructor for posting
    ReLq(Space* home, View x0, View x1, CtrlView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ (x_0 \leq x_1)\Leftrightarrow b\f$
    static  ExecStatus post(Space* home, View x0, View x1, CtrlView b);
  };

  /**
   * \brief Reified less or equal with integer propagator
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View, class CtrlView>
  class ReLqInt : public ReUnaryPropagator<View,PC_INT_BND,CtrlView> {
  protected:
    using ReUnaryPropagator<View,PC_INT_BND,CtrlView>::x0;
    using ReUnaryPropagator<View,PC_INT_BND,CtrlView>::b;

    /// Integer constant to check
    int c;
    /// Constructor for cloning \a p
    ReLqInt(Space* home, bool share, ReLqInt& p);
    /// Constructor for posting
    ReLqInt(Space* home, View x, int c, CtrlView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ (x \leq c)\Leftrightarrow b\f$
    static  ExecStatus post(Space* home, View x, int c, CtrlView b);
   };





  /**
   * \brief Lexical ordering propagator
   *
   * The propagator uses the algorithm (and also the automaton)
   * from:
   *   Mats Carlsson, Nicolas Beldiceanu, Revisiting the
   *   Lexicographic Ordering Constraint. SICS Technical
   *   Report T2002:17, SICS, Sweden, 2002.
   *
   * It deviates in the following two main aspects:
   *  - Assigned variables are eagerly eliminated
   *    This yields the same incremental behaviour with
   *    respect to states 1 and 2 of the automaton.
   *    With respect to the values of \a q and \a r in the report:
   *     - \a q is always 0 after elimination
   *     - \a r is always 1 after elimination
   *
   *  - It is not incremental with respect to states 3 and 4
   *    as no propagation event information is available
   *
   * Requires \code #include "int/rel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Lex : public NaryPropagator<ViewTuple<View,2>,PC_INT_BND> {
  protected:
    using NaryPropagator<ViewTuple<View,2>,PC_INT_BND>::x;

    /// Determines whether propagator is strict or not
    bool strict;
    /// Constructor for cloning \a p
    Lex(Space* home, bool share, Lex& p);
    /// Constructor for posting
    Lex(Space* home, ViewArray<ViewTuple<View,2> >& xy, bool strict);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for lexical order on \a xy
    static  ExecStatus post(Space* home, 
			    ViewArray<ViewTuple<View,2> >& xy, bool strict);
  };

}}}

#include "int/rel/eq.icc"
#include "int/rel/nq.icc"
#include "int/rel/lq-le.icc"
#include "int/rel/lex.icc"

#endif


// STATISTICS: int-prop

