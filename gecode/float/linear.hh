/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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

#ifndef __GECODE_FLOAT_LINEAR_HH__
#define __GECODE_FLOAT_LINEAR_HH__

#include <gecode/int.hh>
#include <gecode/float.hh>

/**
 * \namespace Gecode::Float::Linear
 * \brief %Linear propagators
 */

namespace Gecode { namespace Float { namespace Linear {

  /**
   * \brief Base-class for n-ary linear propagators
   *
   * Positive views are of type \a P whereas negative views are of type \a N.
   */
  template<class P, class N, PropCond pc>
  class Lin : public Propagator {
  protected:
    /// Array of positive views
    ViewArray<P> x;
    /// Array of negative views
    ViewArray<N> y;
    /// Constant value
    FloatVal c;

    /// Constructor for cloning \a p
    Lin(Space& home, bool share, Lin<P,N,pc>& p);
    /// Constructor for creation
    Lin(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
  public:
    /// Cost function (defined as low linear)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Base-class for reified n-ary linear propagators
   *
   * The propagation condition \a pc refers to all views.
   */
  template<class P, class N, PropCond pc, class Ctrl>
  class ReLin : public Lin<P,N,pc> {
  protected:
    /// Control view for reification
    Ctrl b;
    /// Constructor for cloning \a p
    ReLin(Space& home, bool share, ReLin& p);
    /// Constructor for creation
    ReLin(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b);
  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Compute bounds information for positive views
   *
   * \relates Lin
   */
  template<class View>
  void bounds_p(ModEventDelta med, ViewArray<View>& x,
                FloatVal& c, FloatNum& sl, FloatNum& su);

  /**
   * \brief Compute bounds information for negative views
   *
   * \relates Lin
   */
  template<class View>
  void bounds_n(ModEventDelta med, ViewArray<View>& y,
                FloatVal& c, FloatNum& sl, FloatNum& su);

  /**
   * \brief %Propagator for bounds consistent n-ary linear equality
   *
   * Positive views are of type \a P whereas negative views are of type \a N.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  template<class P, class N>
  class Eq : public Lin<P,N,PC_FLOAT_BND> {
  protected:
    using Lin<P,N,PC_FLOAT_BND>::x;
    using Lin<P,N,PC_FLOAT_BND>::y;
    using Lin<P,N,PC_FLOAT_BND>::c;

    /// Constructor for cloning \a p
    Eq(Space& home, bool share, Eq& p);
  public:
    /// Constructor for creation
    Eq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i=c\f$
    static ExecStatus
    post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
  };

  /**
   * \brief %Propagator for reified bounds consistent n-ary linear equality
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  template<class P, class N, class Ctrl, ReifyMode rm>
  class ReEq : public ReLin<P,N,PC_FLOAT_BND,Ctrl> {
  protected:
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::x;
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::y;
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::c;
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::b;

    /// Constructor for cloning \a p
    ReEq(Space& home, bool share, ReEq& p);
  public:
    /// Constructor for creation
    ReEq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i=c\right)\equiv \operatorname{rm}(b)\f$
    static ExecStatus
    post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b);
  };
  /**
   * \brief %Propagator for bounds consistent n-ary linear disequality
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  template<class P, class N>
  class Nq : public Lin<P,N,PC_FLOAT_VAL> {
  protected:
    using Lin<P,N,PC_FLOAT_VAL>::x;
    using Lin<P,N,PC_FLOAT_VAL>::y;
    using Lin<P,N,PC_FLOAT_VAL>::c;

    /// Constructor for cloning \a p
    Nq(Space& home, bool share, Nq& p);
  public:
    /// Constructor for creation
    Nq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i\neq c\f$
    static ExecStatus
    post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
  };


  /**
   * \brief %Propagator for bounds consistent n-ary linear less or equal
   *
   * Positive views are of type \a P whereas negative views are of type \a N.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  template<class P, class N>
  class Lq : public Lin<P,N,PC_FLOAT_BND> {
  protected:
    using Lin<P,N,PC_FLOAT_BND>::x;
    using Lin<P,N,PC_FLOAT_BND>::y;
    using Lin<P,N,PC_FLOAT_BND>::c;

    /// Constructor for cloning \a p
    Lq(Space& home, bool share, Lq& p);
  public:
    /// Constructor for creation
    Lq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i\leq c\f$
    static ExecStatus
    post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c);
  };

  /**
   * \brief %Propagator for reified bounds consistent n-ary linear less or equal
   *
   * The propagation condition \a pc refers to both views.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  template<class P, class N, class Ctrl, ReifyMode rm>
  class ReLq : public ReLin<P,N,PC_FLOAT_BND,Ctrl> {
  protected:
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::x;
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::y;
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::c;
    using ReLin<P,N,PC_FLOAT_BND,Ctrl>::b;

    /// Constructor for cloning \a p
    ReLq(Space& home, bool share, ReLq& p);
  public:
    /// Constructor for creation
    ReLq(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b);
    /// Create copy during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i-\sum_{i=0}^{|y|-1}y_i\leq c\right)\equiv \operatorname{rm}(b)\f$
    static ExecStatus
    post(Home home, ViewArray<P>& x, ViewArray<N>& y, FloatVal c, Ctrl b);
  };
}}}

#include <gecode/float/linear/nary.hpp>

namespace Gecode { namespace Float { namespace Linear {

  /**
   * \brief Class for describing linear term \f$a\cdot x\f$
   *
   */
  class Term {
  public:
    /// Coefficient
    FloatVal a;
    /// View
    FloatView x;
  };

  /** \brief Estimate lower and upper bounds
   *
   * Estimates the boundaries for a linear expression
   * \f$\sum_{i=0}^{n-1}t_i + c\f$. If the boundaries exceed
   * the limits as defined in Limits::Float, these boundaries
   * are returned.
   *
   * \param t array of linear terms
   * \param n size of array
   * \param c constant
   * \param l lower bound
   * \param u upper bound
   *
   */
  GECODE_FLOAT_EXPORT void 
  estimate(Term* t, int n, FloatVal c, FloatNum& l, FloatNum& u);

  /** \brief Normalize linear constraints
   *
   * \param t array of linear terms
   * \param n size of array
   * \param t_p array of linear terms with positive coefficients
   * \param n_p number of postive terms
   * \param t_n array of linear terms with negative coefficients
   * \param n_n number of negative terms
   *
   * Replaces all negative coefficients by positive coefficients.
   *
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *
   * Returns true, if all coefficients are unit coefficients
   */
  GECODE_FLOAT_EXPORT bool 
  normalize(Term* t, int &n,
            Term* &t_p, int &n_p, Term* &t_n, int &n_n);

  /**
   * \brief Post propagator for linear constraint over floats
   * \param home current space
   * \param t array of linear terms over floats
   * \param n size of array
   * \param frt type of relation
   * \param c result of linear constraint
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  GECODE_FLOAT_EXPORT void
  post(Home home, Term* t, int n, FloatRelType frt, FloatVal c);

  /**
   * \brief Post propagator for reified linear constraint over floats
   * \param home current space
   * \param t array of linear terms over Booleans
   * \param n size of array
   * \param frt type of relation
   * \param c result of linear constraint
   * \param r reification specification
   * \param b false for negation of Reify mode
   *
   * All variants for linear constraints share the following properties:
   *  - Variables occuring multiply in the term array are replaced
   *    by a single occurence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *
   * Requires \code #include <gecode/float/linear.hh> \endcode
   * \ingroup FuncFloatProp
   */
  GECODE_FLOAT_EXPORT void
  post(Home home, Term* t, int n, FloatRelType frt, FloatVal c, Reify r, bool b = true);

}}}

#endif

// STATISTICS: float-prop
