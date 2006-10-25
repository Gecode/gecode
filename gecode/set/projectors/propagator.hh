/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#ifndef __GECODE_GENERATOR_PROJECOTRS_HH
#define __GECODE_GENERATOR_PROJECTORS_HH

#include "gecode/set/projectors.hh"

namespace Gecode { namespace Set { namespace Projection {

  /**
   * \brief Nary projection propagator
   */
  template <bool negated>
  class NaryProjection : public Propagator {
  protected:
    /// Array of views
    ViewArray<SetView> x;
    /// Array of PropConds
    Support::SharedArray<PropCond> pc;
    /// The projector set to propagate
    ProjectorSet ps;
    /// Constructor for cloning \a p
    NaryProjection(Space* home, bool share, NaryProjection& p);
    /// Constructor for creation
    NaryProjection(Space* home, ViewArray<SetView>& x, ProjectorSet& ps);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, ViewArray<SetView>& x,
                            ProjectorSet& ps);
  };

  /**
   * \brief Reified Nary projection propagator
   */
  class ReNaryProjection : public Propagator {
  protected:
    /// Array of views
    ViewArray<SetView> x;
    /// Boolean control view
    Gecode::Int::BoolView b;
    /// The projector set to propagate
    ProjectorSet ps;
    /// Constructor for cloning \a p
    ReNaryProjection(Space* home, bool share, ReNaryProjection& p);
    /// Constructor for creation
    ReNaryProjection(Space* home,ViewArray<SetView>& x,
                     Gecode::Int::BoolView,
                     ProjectorSet& ps);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Delete propagator
    GECODE_SET_EXPORT virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor* copy(Space* home,bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space* home);
    GECODE_SET_EXPORT static  ExecStatus post(Space* home,
                                              ViewArray<SetView>& x,
                                              Gecode::Int::BoolView b,
                                              ProjectorSet& ps);
  };

  /**
   * \brief Nary cardinality projection propagator
   */
  class CardProjection : public Propagator {
  protected:
    /// Array of views
    ViewArray<SetView> x;
    /// Integer view for cardinality
    Gecode::Int::IntView i;
    /// Array of PropConds
    Support::SharedArray<PropCond> pc;
    /// The projector to propagate
    Projector proj;
    /// Constructor for cloning \a p
    CardProjection(Space* home, bool share, CardProjection& p);
    /// Constructor for creation
    CardProjection(Space* home, ViewArray<SetView>& x,
                   Gecode::Int::IntView i, Projector& ps);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, ViewArray<SetView>& x,
                            Gecode::Int::IntView i, Projector& p);
  };
  
}}}

#include "gecode/set/projectors/propagator/nary.icc"
#include "gecode/set/projectors/propagator/re-nary.icc"
#include "gecode/set/projectors/propagator/card.icc"
  
#endif
