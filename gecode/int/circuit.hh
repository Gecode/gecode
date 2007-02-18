/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#ifndef __GECODE_INT_CIRCUIT_HH__
#define __GECODE_INT_CIRCUIT_HH__

#include "gecode/int.hh"
#include "gecode/int/distinct.hh"

/**
 * \namespace Gecode::Int::Circuit
 * \brief %Circuit propagators
 */

namespace Gecode { namespace Int { namespace Circuit {

  /**
   * \brief Simple circuit propagator
   *
   * Propagates domain-consistent distinct and checks that
   * the induced variable value graph has a single connected
   * component.
   *
   * Requires \code #include "gecode/int/circuit.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class SsccInfo;
  template <class View>
  class Simple : public NaryPropagator<View,PC_INT_DOM> {
 protected:
    using NaryPropagator<View,PC_INT_DOM>::x;
    /// Array for performing value propagation for distinct
    ViewArray<View> y;
    /// Propagation controller for propagating distinct
    Distinct::DomCtrl<View> dc;
    /// Constructor for cloning \a p
    Simple(Space* home, bool share, Simple& p);
    /// Constructor for posting
    Simple(Space* home, ViewArray<View>& x);
    bool
    Simple<View>::sscc(SsccInfo<View>* si, int &cnt, int i) const;
    /// Check whether the view value graph is strongly connected
    bool connected(void) const;
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is dynamic
     * PC_LINEAR_HI. Otherwise it is dynamic PC_QUADRATIC_LO.
     */
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for circuit on \a x
    static  ExecStatus post(Space* home, ViewArray<View>& x);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

}}}

#include "gecode/int/circuit/simple.icc"

#endif

// STATISTICS: int-prop
