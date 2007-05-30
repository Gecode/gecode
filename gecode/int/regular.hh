/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

  class Layer;
  class State;

  /**
   * \brief Domain-consistent regular propagator
   *
   * The algorithm for the regular propagator is based on:
   *   Gilles Pesant, A Regular Language Membership Constraint
   *   for Finite Sequences of Variables, CP 2004.
   *   Pages 482-495, LNCS 3258, Springer-Verlag, 2004.
   *
   * If \a shared is true, the propagator is capable of multiple
   * occurences of the same view.
   *
   * Requires \code #include "gecode/int/regular.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, bool shared>
  class Dom : public Propagator {
  protected:
    /// The views
    ViewArray<View> x;
    /// The layers of the graph
    Layer* layers;
    /// The states used in the graph
    State* states;
    /// The %DFA describing the language
    DFA dfa;
    /// The start state for graph construction
    int start;

    /// Test whether layered graph has already been constructed
    bool constructed(void) const;
    /// Eliminate assigned prefix
    void eliminate(void);
    /// Construct layered graph
    ExecStatus construct(Space* home);
    /// Prune incrementally
    ExecStatus prune(Space* home);
    /// Tell back modifications
    ExecStatus tell(Space* home);

    /// Constructor for cloning \a p (use \a shared for \a dfa)
    Dom(Space* home, bool share, Dom<View,shared>& p);
  public:
    /// Constructor for posting
    Dom(Space* home, ViewArray<View>& x, DFA& d);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as dynamic PC_LINEAR_HI)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Post propagator on views \a x and DFA \a d
    static ExecStatus post(Space* home, ViewArray<View>& x, DFA& d);
  };

}}}

#include "gecode/int/regular/dom.icc"

#endif

// STATISTICS: int-prop

