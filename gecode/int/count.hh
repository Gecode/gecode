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

#ifndef __GECODE_INT_COUNT_HH__
#define __GECODE_INT_COUNT_HH__

#include "gecode/int.hh"

/**
 * \namespace Gecode::Int::Count
 * \brief Counting propagators
 */

namespace Gecode { namespace Int { namespace Count {

  /**
   * \brief Base-class for count propagators
   *
   */
  template <class VX, class VY, class VZ, class Rel, bool shr>
  class Base : public Propagator {
  protected:
    /// Views still to count
    ViewArray<VX> x;
    /// View to compare to
    VY y;
    /// View which yields result of counting
    VZ z;
    /// Number of views which are equal and have been eliminated
    int   c;
    /// Equality relation used in counting
    Rel   r;
    /// Constructor for cloning \a p
    Base(Space* home, bool shr, Base& p);
    /// Constructor for creation
    Base(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Cost function (defined as dynamic PC_LINEAR_LO)
    virtual PropCost cost(void) const;
  protected:
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
   * Can be used with RelEqBnd (bounds-consistent) or RelEqDom
   * (domain-consistent) as type for \a Rel.
   *
   * Requires \code #include "gecode/int/count.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VX, class VY, class VZ, class Rel, bool shr>
  class Eq : public Base<VX,VY,VZ,Rel,shr> {
  protected:
    using Base<VX,VY,VZ,Rel,shr>::x;
    using Base<VX,VY,VZ,Rel,shr>::z;
    using Base<VX,VY,VZ,Rel,shr>::c;
    using Base<VX,VY,VZ,Rel,shr>::r;
    using Base<VX,VY,VZ,Rel,shr>::y;
    using Base<VX,VY,VZ,Rel,shr>::atleast;
    using Base<VX,VY,VZ,Rel,shr>::atmost;

    /// Constructor for cloning \a p
    Eq(Space* home, bool shr, Eq& p);
  public:
    /// Constructor for creation
    Eq(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space* home, bool shr);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}+c=z\f$
    static ExecStatus post(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

  /**
   * \brief %Propagator for counting views (different from number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Can be used with RelEqBnd (bounds-consistent) or RelEqDom
   * (domain-consistent) as type for \a Rel.
   *
   * Requires \code #include "gecode/int/count.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VX, class VY, class VZ, class Rel, bool shr>
  class Nq : public Base<VX,VY,VZ,Rel,shr> {
  protected:
    using Base<VX,VY,VZ,Rel,shr>::x;
    using Base<VX,VY,VZ,Rel,shr>::z;
    using Base<VX,VY,VZ,Rel,shr>::c;
    using Base<VX,VY,VZ,Rel,shr>::r;
    using Base<VX,VY,VZ,Rel,shr>::y;
    using Base<VX,VY,VZ,Rel,shr>::atleast;
    using Base<VX,VY,VZ,Rel,shr>::atmost;

    /// Constructor for cloning \a p
    Nq(Space* home, bool shr, Nq& p);
  public:
    /// Constructor for creation
    Nq(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space* home, bool shr);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}+c\neq z\f$
    static ExecStatus post(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

  /**
   * \brief %Propagator for counting views (less or equal to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Can be used with RelEqBnd (bounds-consistent) or RelEqDom
   * (domain-consistent) as type for \a Rel.
   *
   * Requires \code #include "gecode/int/count.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VX, class VY, class VZ, class Rel, bool shr>
  class Lq : public Base<VX,VY,VZ,Rel,shr> {
  protected:
    using Base<VX,VY,VZ,Rel,shr>::x;
    using Base<VX,VY,VZ,Rel,shr>::z;
    using Base<VX,VY,VZ,Rel,shr>::c;
    using Base<VX,VY,VZ,Rel,shr>::r;
    using Base<VX,VY,VZ,Rel,shr>::y;
    using Base<VX,VY,VZ,Rel,shr>::atleast;
    using Base<VX,VY,VZ,Rel,shr>::atmost;

    /// Constructor for cloning \a p
    Lq(Space* home, bool shr, Lq& p);
  public:
    /// Constructor for creation
    Lq(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space* home, bool shr);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}+c\leq z\f$
    static ExecStatus post(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

  /**
   * \brief %Propagator for counting views (greater or equal to number of equal views)
   *
   * Not all combinations of views are possible. The types \a VX
   * and \a VY must be either equal, or \a VY must be ConstIntView.
   *
   * Can be used with RelEqBnd (bounds-consistent) or RelEqDom
   * (domain-consistent) as type for \a Rel.
   *
   * Requires \code #include "gecode/int/count.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VX, class VY, class VZ, class Rel, bool shr>
  class Gq : public Base<VX,VY,VZ,Rel,shr> {
  protected:
    using Base<VX,VY,VZ,Rel,shr>::x;
    using Base<VX,VY,VZ,Rel,shr>::z;
    using Base<VX,VY,VZ,Rel,shr>::c;
    using Base<VX,VY,VZ,Rel,shr>::r;
    using Base<VX,VY,VZ,Rel,shr>::y;
    using Base<VX,VY,VZ,Rel,shr>::atleast;
    using Base<VX,VY,VZ,Rel,shr>::atmost;

    /// Constructor for cloning \a p
    Gq(Space* home, bool shr, Gq& p);
  public:
    /// Constructor for creation
    Gq(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
    /// Create copy during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}+c\geq z\f$
    static ExecStatus post(Space* home, ViewArray<VX>& x, VY y, VZ z, int c);
  };

}}}

#include "gecode/int/count/count.icc"

namespace Gecode { namespace Int { namespace Count {

  /**
   * \brief Relation for bounds-consistent counting
   *
   */
  template <class VX>
  class RelEqBnd {
  public:
    /// Propagation condition (PC_INT_BND)
    PropCond cond(void) const;
    /// Test whether \a x and \a y are equal
    RelTest holds(VX,VX);
    /// Test whether \a x and \a y are equal
    RelTest holds(VX x, ConstIntView y);
    /// Post that all views in \a x are equal to \a y
    ExecStatus post_true(Space* home, ViewArray<VX>& x, VX y);
    /// Post that all views in \a x are equal to \a y
    ExecStatus post_true(Space* home, ViewArray<VX>& x, ConstIntView y);
    /// Post that all views in \a x are not equal to \a y
    ExecStatus post_false(Space* home, ViewArray<VX>& x, VX y);
    /// Post that all views in \a x are not equal to \a y
    ExecStatus post_false(Space* home, ViewArray<VX>& x, ConstIntView y);
  };

  /**
   * \brief Relation for domain-consistent counting
   *
   */
  template <class VX>
  class RelEqDom {
  public:
    /// Propagation condition (PC_INT_DOM)
    PropCond cond(void) const;
    /// Test whether \a x and \a y are equal
    RelTest holds(VX x, VX y);
    /// Test whether \a x and \a y are equal
    RelTest holds(VX x, ConstIntView y);
    /// Post that all views in \a x are equal to \a y
    ExecStatus post_true(Space* home, ViewArray<VX>& x, VX y);
    /// Post that all views in \a x are equal to \a y
    ExecStatus post_true(Space* home, ViewArray<VX>& x, ConstIntView y);
    /// Post that all views in \a x are not equal to \a y
    ExecStatus post_false(Space* home, ViewArray<VX>& x, VX y);
    /// Post that all views in \a x are not equal to \a y
    ExecStatus post_false(Space* home, ViewArray<VX>& x, ConstIntView y);
  };

}}}

#include "gecode/int/count/rel.icc"

#endif

// STATISTICS: int-prop

