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
   * The propagator is not capable of multiple occurences of the same
   * view, see gecode/regular.cc for the necessary preprocessing.
   *
   * Requires \code #include "gecode/int/regular.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Dom : public Propagator {
  protected:
    /// Advisors for views (by position in array)
    class Index : public Advisor {
    public:
      /// The position of the view in the view array
      int i;
      /// Create index advisor
      Index(Space* home, Propagator* p, Council<Index>& c, int i);
      /// Clone index advisor \a a
      Index(Space* home, bool share, Index& a);
      /// Dispose advisor
      void dispose(Space* home, Council<Index>& c);
    };
    /// The advisor council
    Council<Index> c;
    /// The views
    ViewArray<View> x;
    /// The %DFA describing the language
    DFA dfa;
    /// The start state for graph construction
    int start;
    /// The layers of the graph
    Layer* layers;
    /// The states used in the graph
    State* states;
    /// Whether propagator is currently propagating
    bool propagating;
    
    /// Test whether layered graph has already been constructed
    bool constructed(void) const;
    /// Eliminate assigned prefix, return how many layers removed
    void eliminate(void);
    /// Construct layered graph
    ExecStatus construct(Space* home);
    /// Prune incrementally
    ExecStatus prune(Space* home);

    /// Constructor for cloning \a p
    Dom(Space* home, bool share, Dom<View>& p);
  public:
    /// Constructor for posting
    Dom(Space* home, ViewArray<View>& x, DFA& d);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as dynamic PC_LINEAR_HI)
    virtual PropCost cost(void) const;
    /// Give advice to propagator
    virtual ExecStatus advise(Space* home, Advisor* a, const Delta* d);
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

