/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/int/bool.hh
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
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

#ifndef __GECODE_INT_QBOOL_HH__
#define __GECODE_INT_QBOOL_HH__

#include <gecode/int.hh>
#include <quacode/qcsp.hh>

/**
 * \namespace Gecode::Int::Bool
 * \brief Quantified Boolean propagators
 */

namespace Gecode { namespace Int { namespace Bool {

  /**
   * \brief Quantified Boolean equality propagator
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB>
  class QEq : public BoolBinary<BVA,BVB> {
  protected:
    using BoolBinary<BVA,BVB>::x0;
    using BoolBinary<BVA,BVB>::x1;
    /// Quantifier for x0
    TQuantifier q0;
    /// Rank for x0
    int r0;
    /// Quantifier for x1
    TQuantifier q1;
    /// Rank for x1
    int r1;
    /// Constructor for posting
    QEq(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
    /// Constructor for cloning \a p
    QEq(Space& home, bool share, QEq& p);
  public:
    /// Constructor for rewriting \a p during cloning
    QEq(Space& home, bool share, Propagator& p,
        BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ V/E x_0 = V/E x_1\f$
    static  ExecStatus post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
    static  ExecStatus post(Home home, QBoolVar x0, QBoolVar x1);
  };

  /**
   * \brief Quantified Boolean xor propagator
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB>
  class QXor : public BoolBinary<BVA,BVB> {
  protected:
    using BoolBinary<BVA,BVB>::x0;
    using BoolBinary<BVA,BVB>::x1;
    /// Quantifier for x0
    TQuantifier q0;
    /// Rank for x0
    int r0;
    /// Quantifier for x1
    TQuantifier q1;
    /// Rank for x1
    int r1;
    /// Constructor for posting
    QXor(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
    /// Constructor for cloning \a p
    QXor(Space& home, bool share, QXor& p);
  public:
    /// Constructor for rewriting \a p during cloning
    QXor(Space& home, bool share, Propagator& p,
         BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ V/E x_0 <> V/E x_1\f$
    static  ExecStatus post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
    static  ExecStatus post(Home home, QBoolVar x0, QBoolVar x1);
  };


  /**
   * \brief Quantified Binary Boolean disjunction propagator (true)
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB>
  class QBinOrTrue : public BoolBinary<BVA,BVB> {
  protected:
    using BoolBinary<BVA,BVB>::x0;
    using BoolBinary<BVA,BVB>::x1;
    /// Quantifier for x0
    TQuantifier q0;
    /// Rank for x0
    int r0;
    /// Quantifier for x1
    TQuantifier q1;
    /// Rank for x1
    int r1;
    /// Constructor for posting
    QBinOrTrue(Home home,
               BVA b0, TQuantifier _q0, int _r0,
               BVB b1, TQuantifier _q1, int _r1);
    /// Constructor for cloning \a p
    QBinOrTrue(Space& home, bool share, QBinOrTrue& p);
  public:
    /// Constructor for rewriting \a p during cloning
    QBinOrTrue(Space& home, bool share, Propagator& p,
               BVA b0, TQuantifier _q0, int _r0,
               BVB b1, TQuantifier _q1, int _r1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ V/E b_0 \lor V/E b_1 = 1 \f$
    static  ExecStatus post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1);
  };

  /**
   * \brief Quantified Boolean disjunction propagator
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB, class BVC>
  class QOr : public BoolTernary<BVA,BVB,BVC> {
  protected:
    using BoolTernary<BVA,BVB,BVC>::x0;
    using BoolTernary<BVA,BVB,BVC>::x1;
    using BoolTernary<BVA,BVB,BVC>::x2;
    /// Quantifier for x0
    TQuantifier q0;
    /// Rank for x0
    int r0;
    /// Quantifier for x1
    TQuantifier q1;
    /// Rank for x1
    int r1;
    /// Constructor for posting
    QOr(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
    /// Constructor for cloning \a p
    QOr(Space& home, bool share, QOr& p);
  public:
    /// Constructor for rewriting \a p during cloning
    QOr(Space& home, bool share, Propagator& p, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ V/E b_0 \lor V/E b_1 = b_2 \f$
    static  ExecStatus post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
  };

  /**
   * \brief Quantified boolean n-ary disjunction propagator
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX,class VY>
  class QNaryOr
    : public MixNaryOnePropagator<VX,PC_BOOL_NONE,VY,PC_BOOL_VAL> {
  protected:
    using MixNaryOnePropagator<VX,PC_BOOL_NONE,VY,PC_BOOL_VAL>::x;
    using MixNaryOnePropagator<VX,PC_BOOL_NONE,VY,PC_BOOL_VAL>::y;
    /// Views quantifiers
    TQuantifier* q;
    /// Views ranks
    int* r;
    /// The number of views assigned to zero in \a x
    int n_zero;
    /// The advisor council
    Council<Advisor> c;
    /// Constructor for posting
    QNaryOr(Home home,  ViewArray<VX>& x, const QuantArgs& q, const IntArgs& r, VY y);
    /// Constructor for cloning \a p
    QNaryOr(Space& home, bool share, QNaryOr<VX,VY>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Cost function (defined as low unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \bigvee_{i=0}^{|x|-1} V/E x_i = y\f$
    static  ExecStatus post(Home home, ViewArray<VX>& x, QuantArgs q, IntArgs r, VY y);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


  /**
   * \brief Quantified boolean n-ary disjunction propagator (true)
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class QNaryOrTrue : public BinaryPropagator<BV,PC_BOOL_VAL> {
  protected:
    using BinaryPropagator<BV,PC_BOOL_VAL>::x0;
    using BinaryPropagator<BV,PC_BOOL_VAL>::x1;
    /// Views not yet subscribed to
    ViewArray<BV> x;
    /// Current view quantifier for x0
    TQuantifier qx0;
    /// Current view quantifier for x1
    TQuantifier qx1;
    /// Views quantifiers not yet subscribed to
    TQuantifier* qx;
    /// Current view rank for x0
    int rx0;
    /// Current view rank for x1
    int rx1;
    /// Views ranks not yet subscribed to
    int* rx;
    /// Update subscription
    ExecStatus resubscribe(Space& home, BV& x0, TQuantifier& _qx0, int& _rx0, BV x1, TQuantifier _qx1, int _rx1);
    /// Constructor for posting
    QNaryOrTrue(Home home, ViewArray<BV>& x, const QuantArgs& q, const IntArgs& r);
    /// Constructor for cloning \a p
    QNaryOrTrue(Space& home, bool share, QNaryOrTrue<BV>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Cost function (defined as low unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \bigvee_{i=0}^{|b|-1} V/E b_i = 0\f$
    static  ExecStatus post(Home home, ViewArray<BV>& b, QuantArgs q, IntArgs r);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


  /**
   * \brief Boolean equivalence propagator for quantified variables
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB, class BVC>
  class QEqv : public BoolTernary<BVA,BVB,BVC> {
  protected:
    using BoolTernary<BVA,BVB,BVC>::x0;
    using BoolTernary<BVA,BVB,BVC>::x1;
    using BoolTernary<BVA,BVB,BVC>::x2;
    /// Quantifier for x0
    TQuantifier q0;
    /// Rank for x0
    int r0;
    /// Quantifier for x1
    TQuantifier q1;
    /// Rank for x1
    int r1;
    /// Constructor for cloning \a p
    QEqv(Space& home, bool share, QEqv& p);
    /// Constructor for posting
    QEqv(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ V/E b_0 \Leftrightarrow V/E b_1 = b_2 \f$ (equivalence)
    static  ExecStatus post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
    static  ExecStatus post(Home home, QBoolVar b0, QBoolVar b1, BVC b2);
  };

  /**
   * \brief Boolean xor propagator for quantified variables
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB, class BVC>
  class QXorv : public BoolTernary<BVA,BVB,BVC> {
  protected:
    using BoolTernary<BVA,BVB,BVC>::x0;
    using BoolTernary<BVA,BVB,BVC>::x1;
    using BoolTernary<BVA,BVB,BVC>::x2;
    /// Quantifier for x0
    TQuantifier q0;
    /// Rank for x0
    int r0;
    /// Quantifier for x1
    TQuantifier q1;
    /// Rank for x1
    int r1;
    /// Constructor for cloning \a p
    QXorv(Space& home, bool share, QXorv& p);
    /// Constructor for posting
    QXorv(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ V/E b_0 <> V/E b_1 = b_2 \f$ (equivalence)
    static  ExecStatus post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2);
    static  ExecStatus post(Home home, QBoolVar b0, QBoolVar b1, BVC b2);
  };

  /**
   * \brief Quantified Boolean clause propagator (disjunctive)
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY>
  class QClause : public Propagator {
  protected:
    /// Positive views
    ViewArray<VX> x;
    /// Positive views quantifiers
    TQuantifier* qx;
    /// Positive views ranks
    int* rx;
    /// Positive views (origin from negative variables)
    ViewArray<VY> y;
    /// Positive views quantifiers (origin from negative variables)
    TQuantifier* qy;
    /// Positive ranks (origin from negative variables)
    int* ry;
    /// Result
    VX z;
    /// The number of views assigned to zero in \a x and \a y
    int n_zero;
    /// %Advisors for views (tagged whether for \a x or \a y)
    class Tagged : public Advisor {
    public:
      /// Whether advises a view for x or y
      const bool x;
      /// Create tagged advisor
      Tagged(Space& home, Propagator& p, Council<Tagged>& c, bool x);
      /// Clone tagged advisor \a a
      Tagged(Space& home, bool share, Tagged& a);
    };
    /// The advisor council
    Council<Tagged> c;
    /// Cancel subscriptions
    void cancel(Space& home);
    /// Constructor for posting
    QClause(Home home, ViewArray<VX>& x, const QuantArgs& qx, const IntArgs& rx, ViewArray<VY>& y, const QuantArgs& qy,  const IntArgs& ry, VX z);
    /// Constructor for cloning \a p
    QClause(Space& home, bool share, QClause<VX,VY>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Cost function (defined as low unary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \bigvee_{i=0}^{|x|-1} V/E x_i \vee \bigvee_{i=0}^{|x|-1} V/E y_i = z\f$
    static  ExecStatus post(Home home, ViewArray<VX>& x, QuantArgs qx, IntArgs rx, ViewArray<VY>& y, QuantArgs qy, IntArgs ry,
                            VX z);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Quantified Boolean clause propagator (disjunctive, true)
   *
   * Requires \code #include <gecode/int/bool.hh> \endcode
   * \ingroup FuncIntProp
   */
  template<class VX, class VY>
  class QClauseTrue
    : public MixBinaryPropagator<VX,PC_BOOL_VAL,VY,PC_BOOL_VAL> {
  protected:
    using MixBinaryPropagator<VX,PC_BOOL_VAL,VY,PC_BOOL_VAL>::x0;
    using MixBinaryPropagator<VX,PC_BOOL_VAL,VY,PC_BOOL_VAL>::x1;
    /// Views not yet subscribed to
    ViewArray<VX> x;
    /// Current view quantifier
    TQuantifier c_qx;
    /// Views quantifiers not yet subscribed to
    TQuantifier* qx;
    /// Current view rank
    int c_rx;
    /// Views ranks not yet subscribed to
    int* rx;
    /// Views not yet subscribed to (origin from negative variables)
    ViewArray<VY> y;
    /// Current view quantifier (origin from negative variables)
    TQuantifier c_qy;
    /// Views quantifiers not yet subscribed to (origin from negative variables)
    TQuantifier* qy;
    /// Current view rank (origin from negative variables)
    int c_ry;
    /// Views ranks not yet subscribed to (origin from negative variables)
    int* ry;
    /// Constructor for posting
    QClauseTrue(Home home, ViewArray<VX>& x, const QuantArgs& qx, const IntArgs& rx, ViewArray<VY>& y, const QuantArgs& qy,  const IntArgs& ry);
    /// Constructor for cloning \a p
    QClauseTrue(Space& home, bool share, QClauseTrue<VX,VY>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Cost function (defined as low binary)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Post propagator \f$ \bigvee_{i=0}^{|x|-1} V/E x_i \vee \bigvee_{i=0}^{|y|-1} V/E y_i = 1\f$
    static  ExecStatus post(Home home, ViewArray<VX>& x, QuantArgs qx, IntArgs rx, ViewArray<VY>& y, QuantArgs qy, IntArgs ry);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


}}}

#include <quacode/qint/qbool/eq.hpp>
#include <quacode/qint/qbool/xor.hpp>
#include <quacode/qint/qbool/or.hpp>
#include <quacode/qint/qbool/eqv.hpp>
#include <quacode/qint/qbool/xorv.hpp>
#include <quacode/qint/qbool/clause.hpp>

#endif

// STATISTICS: int-prop
