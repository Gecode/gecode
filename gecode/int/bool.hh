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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_INT_BOOL_HH__
#define __GECODE_INT_BOOL_HH__

#include "gecode/int.hh"

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
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
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
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

  /**
   * \brief Boolean equality propagator
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
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
   * \brief Boolean less or equal propagator
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class Lq : public BoolBinary<BV,BV> {
  protected:
    using BoolBinary<BV,BV>::x0;
    using BoolBinary<BV,BV>::x1;
    /// Constructor for posting
    Lq(Space* home, BV b0, BV b1);
    /// Constructor for cloning \a p
    Lq(Space* home, bool share, Lq& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \leq b_1\f$
    static  ExecStatus post(Space* home, BV b0, BV b1);
  };


  /**
   * \brief Boolean less propagator
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class Le {
  public:
    /// Post propagator \f$ b_0 < b_1\f$
    static  ExecStatus post(Space* home, BV b0, BV b1);
  };


  /**
   * \brief Binary Boolean disjunction propagator (true)
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB>
  class BinOrTrue : public BoolBinary<BVA,BVB> {
  protected:
    using BoolBinary<BVA,BVB>::x0;
    using BoolBinary<BVA,BVB>::x1;
    /// Constructor for posting
    BinOrTrue(Space* home, BVA b0, BVB b1);
    /// Constructor for cloning \a p
    BinOrTrue(Space* home, bool share, BinOrTrue& p);
  public:
    /// Constructor for rewriting \a p during cloning
    BinOrTrue(Space* home, bool share, Propagator& p,
              BVA b0, BVB b1);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \lor b_1 = 1 \f$
    static  ExecStatus post(Space* home, BVA b0, BVB b1);
  };

  /**
   * \brief Ternary Boolean disjunction propagator (true)
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class TerOrTrue : public BoolBinary<BV,BV> {
  protected:
    using BoolBinary<BV,BV>::x0;
    using BoolBinary<BV,BV>::x1;
    /// Boolean view without subscription
    BV x2;
    /// Constructor for posting
    TerOrTrue(Space* home, BV b0, BV b1, BV b2);
    /// Constructor for cloning \a p
    TerOrTrue(Space* home, bool share, TerOrTrue& p);
  public:
    /// Constructor for rewriting \a p during cloning
    TerOrTrue(Space* home, bool share, Propagator& p,
              BV b0, BV b1, BV b2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \lor b_1 \lor b_2 = 1 \f$
    static  ExecStatus post(Space* home, BV b0, BV b1, BV b2);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

  /**
   * \brief Quarternary Boolean disjunction propagator (true)
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class QuadOrTrue : public BoolBinary<BV,BV> {
  protected:
    using BoolBinary<BV,BV>::x0;
    using BoolBinary<BV,BV>::x1;
    /// Boolean view without subscription
    BV x2;
    /// Boolean view without subscription
    BV x3;
    /// Constructor for posting
    QuadOrTrue(Space* home, BV b0, BV b1, BV b2, BV b3);
    /// Constructor for cloning \a p
    QuadOrTrue(Space* home, bool share, QuadOrTrue& p);
  public:
    /// Constructor for rewriting \a p during cloning
    QuadOrTrue(Space* home, bool share, Propagator& p,
               BV b0, BV b1, BV b2, BV b3);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \lor b_1 \lor b_2 \lor b_3 = 1 \f$
    static  ExecStatus post(Space* home, BV b0, BV b1, BV b2, BV b3);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

  /**
   * \brief Boolean disjunction propagator
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BVA, class BVB, class BVC>
  class Or : public BoolTernary<BVA,BVB,BVC> {
  protected:
    using BoolTernary<BVA,BVB,BVC>::x0;
    using BoolTernary<BVA,BVB,BVC>::x1;
    using BoolTernary<BVA,BVB,BVC>::x2;
    /// Constructor for posting
    Or(Space* home, BVA b0, BVB b1, BVC b2);
    /// Constructor for cloning \a p
    Or(Space* home, bool share, Or& p);
  public:
    /// Constructor for rewriting \a p during cloning
    Or(Space* home, bool share, Propagator& p, BVA b0, BVB b1, BVC b2);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ b_0 \lor b_1 = b_2 \f$
    static  ExecStatus post(Space* home, BVA b0, BVB b1, BVC b2);
  };

  /**
   * \brief Boolean n-ary disjunction propagator
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class NaryOr : public NaryOnePropagator<BV,PC_BOOL_VAL> {
  protected:
    using NaryOnePropagator<BV,PC_BOOL_VAL>::x;
    using NaryOnePropagator<BV,PC_BOOL_VAL>::y;
    /// Constructor for posting
    NaryOr(Space* home,  ViewArray<BV>& b, BV c);
    /// Constructor for cloning \a p
    NaryOr(Space* home, bool share, NaryOr<BV>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ \bigvee_{i=0}^{|b|-1} b_i = c\f$
    static  ExecStatus post(Space* home, ViewArray<BV>& b, BV c);
  };


  /**
   * \brief Boolean n-ary disjunction propagator (true)
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class NaryOrTrue : public BinaryPropagator<BV,PC_BOOL_VAL> {
  protected:
    using BinaryPropagator<BV,PC_BOOL_VAL>::x0;
    using BinaryPropagator<BV,PC_BOOL_VAL>::x1;
    /// Views not yet subscribed to
    ViewArray<BV> x;
    /// Update subscription
    ExecStatus resubscribe(Space* home, BV& x0, BV x1);
    /// Constructor for posting
    NaryOrTrue(Space* home,  ViewArray<BV>& b);
    /// Constructor for cloning \a p
    NaryOrTrue(Space* home, bool share, NaryOrTrue<BV>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as PC_LINEAR_LO)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ \bigvee_{i=0}^{|b|-1} b_i = 0\f$
    static  ExecStatus post(Space* home, ViewArray<BV>& b);
  };


#if GECODE_USE_ADVISORS
  /**
   * \brief Boolean n-ary disjunction propagator (true)
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
   * \ingroup FuncIntProp
   */
  template<class BV>
  class NaryOrWLTrue : public Propagator {
  protected:
    /// Views not yet subscribed to
    ViewArray<BV> x;
    /// Update subscription
    ExecStatus resubscribe(Space* home, BV& x0, BV x1);
    /// Constructor for posting
    NaryOrWLTrue(Space* home,  ViewArray<BV>& b);
    /// Constructor for cloning \a p
    NaryOrWLTrue(Space* home, bool share, NaryOrWLTrue<BV>& p);
    /// Disposing
    size_t dispose(Space* home);

    // Advisors
    class WLAdvisor : public IntUnaryViewAdvisor<BV, PC_BOOL_ADVISOR> {
      PropagatorPointer pp;
      using IntUnaryViewAdvisor<BV, PC_BOOL_ADVISOR>::x;
    public:
      WLAdvisor(Space* home, Propagator* p, BV v) 
        : IntUnaryViewAdvisor<BV, PC_BOOL_ADVISOR>(home,p,v), pp(p) {
        assert(!x.assigned());
      }
      WLAdvisor(Space* home, Propagator* p, bool share, WLAdvisor& d) 
        : IntUnaryViewAdvisor<BV, PC_BOOL_ADVISOR>(home, p, share, d), 
          pp(p) {}
      Advisor *copy(Space *home, Propagator* p, bool share) {
        return new (home) WLAdvisor(home, p, share, *this); 
      }
      size_t dispose(Space* home) {
        (void) IntUnaryViewAdvisor<BV, PC_BOOL_ADVISOR>::dispose(home);
        return sizeof(*this);
      }
      ExecStatus advise(Space *home, ModEvent me, int lo, int hi);
    };
    //typedef AdvisorCollection<WLAdvisor, 2> AC;
    AdvisorCollection<WLAdvisor, 2> ac;

  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as PC_LINEAR_LO)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator \f$ \bigvee_{i=0}^{|b|-1} b_i = 0\f$
    static  ExecStatus post(Space* home, ViewArray<BV>& b);
  };
#endif

  /**
   * \brief Boolean equivalence propagator
   *
   * Requires \code #include "gecode/int/bool.hh" \endcode
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

#include "gecode/int/bool/base.icc"
#include "gecode/int/bool/eq.icc"
#include "gecode/int/bool/lq.icc"
#include "gecode/int/bool/or.icc"
#include "gecode/int/bool/eqv.icc"

#endif

// STATISTICS: int-prop

