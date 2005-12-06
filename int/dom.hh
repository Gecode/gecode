/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#ifndef __GECODE_INT_DOM_HH__
#define __GECODE_INT_DOM_HH__

#include "int.hh"
#include "int/rel.hh"
#include "iter.hh"

/**
 * \namespace Gecode::Int::Dom
 * \brief Domain propagators
 */

namespace Gecode { namespace Int { namespace Dom {

  /**
   * \brief Reified range dom-propagator
   *
   * Requires \code #include "int/dom.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class ReRange : public ReUnaryPropagator<View,PC_INT_BND,BoolView> {
  protected:
    using ReUnaryPropagator<View,PC_INT_BND,BoolView>::x0;
    using ReUnaryPropagator<View,PC_INT_BND,BoolView>::b;
    /// Minimum of range
    int min; 
    /// Maximum of range
    int max;
    /// Constructor for cloning \a p
    ReRange(Space* home, bool share, ReRange& p);
    /// Constructor for creation
    ReRange(Space* home, View x, int min, int max, BoolView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ (l\leq x \leq m) \Leftrightarrow b\f$
    static ExecStatus post(Space* home, View x, int min, int max, BoolView b);
  };

  /**
   * \brief Reified domain dom-propagator
   *
   * Requires \code #include "int/dom.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class ReIntSet : public ReUnaryPropagator<View,PC_INT_DOM,BoolView> {
  protected:
    using ReUnaryPropagator<View,PC_INT_DOM,BoolView>::x0;
    using ReUnaryPropagator<View,PC_INT_DOM,BoolView>::b;

    /// %Domain
    IntSet is;
    /// Constructor for cloning \a p
    ReIntSet(Space* home, bool share, ReIntSet& p);
    /// Constructor for creation
    ReIntSet(Space* home, View x, const IntSet& s, BoolView b);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ (x \in d) \Leftrightarrow b\f$
    static ExecStatus post(Space* home, View x, const IntSet& s, BoolView b);
  };

}}}

#include "int/dom/range.icc"
#include "int/dom/spec.icc"

#endif

// STATISTICS: int-prop

