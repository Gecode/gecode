/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#ifndef __GECODE_SET_SELECT_HH__
#define __GECODE_SET_SELECT_HH__

#include "gecode/set.hh"

#include "gecode/set/select/idxarray.hh"
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"

namespace Gecode { namespace Set { namespace Select {

  /**
   * \namespace Gecode::Set::Select
   * \brief %Set selection propagators
   */

  /**
   * \brief %Propagator for selected intersection
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class SView, class RView>
  class SelectIntersection :
    public Propagator {
  protected:
    IntSet universe;
    SView x0;
    IdxViewArray<SView> iv;
    RView x1;

    /// Constructor for cloning \a p
    SelectIntersection(Space* home, bool share,SelectIntersection& p);
    /// Constructor for posting
    SelectIntersection(Space* home,SView,IdxViewArray<SView>&,RView,
                       const IntSet& universe);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /** Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y] \f$ using \a u as universe
     *
     * If \a y is empty, \a z will be constrained to be the given universe
     * \a u (as an empty intersection is the universe).
     */
    static  ExecStatus  post(Space* home,SView z,IdxViewArray<SView>& x,
                             RView y, const IntSet& u);
  };

  /**
   * \brief %Propagator for selected union
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class SView, class RView>
  class SelectUnion :
    public Propagator {
  protected:
    SView x0;
    IdxViewArray<SView> iv;
    RView x1;

    /// Constructor for cloning \a p
    SelectUnion(Space* home, bool share,SelectUnion& p);
    /// Constructor for posting
    SelectUnion(Space* home,SView,IdxViewArray<SView>&,RView);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /** Post propagator for \f$ z=\bigcup\langle x_0,\dots,x_{n-1}\rangle[y] \f$
     *
     * If \a y is empty, \a z will be constrained to be empty
     * (as an empty union is the empty set).
     */
    static  ExecStatus  post(Space* home,SView z,IdxViewArray<SView>& x,
                             RView y);
  };

  /**
   * \brief %Propagator for selected disjointness
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  class SelectDisjoint :
    public Propagator {
  protected:
    IdxViewArray<SetView> iv;
    SetView x1;

    /// Constructor for cloning \a p
    SelectDisjoint(Space* home, bool share,SelectDisjoint& p);
    /// Constructor for posting
    SelectDisjoint(Space* home,IdxViewArray<SetView>&,SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home,bool);
    GECODE_SET_EXPORT virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    GECODE_SET_EXPORT virtual size_t dispose(Space* home);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus  propagate(Space* home);
    /// Post propagator for \f$ \parallel\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
    static  ExecStatus  post(Space* home,IdxViewArray<SetView>& x,SetView y);
  };

}}}

#include "gecode/set/select/inter.icc"
#include "gecode/set/select/union.icc"
#include "gecode/set/select/disjoint.icc"

#endif

// STATISTICS: set-prop

