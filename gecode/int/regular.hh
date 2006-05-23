/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
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

#ifndef __GECODE_INT_REGULAR_HH__
#define __GECODE_INT_REGULAR_HH__

#include "gecode/int.hh"

/**
 * \namespace Gecode::Int::Regular
 * \brief %Regular propagators
 */

namespace Gecode { namespace Int { namespace Regular {

  /**
   * \brief Domain-consistent regular propagator
   *
   * The algorithm for the regular propagator is based on:
   *   Gilles Pesant, A Regular Language Membership Constraint
   *   for Finite Sequences of Variables, CP 2004.
   *   Pages 482-495, LNCS 3258, Springer-Verlag, 2004.
   *
   * Requires \code #include "gecode/int/regular.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Dom : public NaryPropagator<View,PC_INT_DOM> {
  protected:
    /// The %DFA describing the language
    DFA dfa;
    /// %LayeredGraph as data structure used for propagation
    class LayeredGraph;
    /// Propagation is performed on a layered graph (used as cache)
    LayeredGraph* lg;
        
    /// Constructor for cloning \a p (use \a shared for \a dfa)
    Dom(Space* home, bool shared, Dom<View>& p);
    /// Constructor for posting
    Dom(Space* home, ViewArray<View>& x, DFA& d);
  public:
    /// Copy propagator during cloning    
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as dynamic PC_LINEAR_HI)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Flush layered graph
    virtual void flush(void);
    /// Returns size of layered graph
    virtual size_t size(void) const;
    /// Delete propagator
    virtual void dispose(Space* home);
    /// Post propagator on views \a x and DFA \a d
    static ExecStatus post(Space* home, ViewArray<View>& x, DFA& d);
  };

}}}

#include "gecode/int/regular/dom.icc"

#endif

// STATISTICS: int-prop

