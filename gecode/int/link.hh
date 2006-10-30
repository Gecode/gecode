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

#ifndef __GECODE_INT_LINK_HH__
#define __GECODE_INT_LINK_HH__

#include "gecode/int.hh"

/**
 * \namespace Gecode::Int::Link
 * \brief Propagators for linking Boolean and integer variables
 */

namespace Gecode { namespace Int { namespace Link {

  /**
   * \brief Link propagator
   *
   * Requires \code #include "gecode/int/link.hh" \endcode
   * \ingroup FuncIntProp
   */
  class Link :
    public MixBinaryPropagator<IntView,PC_INT_VAL,BoolView,PC_INT_VAL> {
  private:
    using MixBinaryPropagator<IntView,PC_INT_VAL,BoolView,PC_INT_VAL>::x0;
    using MixBinaryPropagator<IntView,PC_INT_VAL,BoolView,PC_INT_VAL>::x1;

    /// Constructor for cloning \a p
    Link(Space* home, bool share, Link& p);
    /// Constructor for posting
    Link(Space* home, IntView x0, BoolView x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ x_0 = x_1\f$
    static  ExecStatus post(Space* home, IntView x0, BoolView x1);
  };

}}}

#endif


// STATISTICS: int-prop

