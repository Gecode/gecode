/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *     Gabor Szokoli, 2003
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

#ifndef __GECODE_INT_DISTINCT_HH__
#define __GECODE_INT_DISTINCT_HH__

#include "gecode/int.hh"

#include "gecode/int/rel.hh"
#include "gecode/support/sentinel-stack.hh"

/**
 * \namespace Gecode::Int::Distinct
 * \brief %Distinct propagators
 */

namespace Gecode { namespace Int { namespace Distinct {

  /**
   * \brief Naive value distinct propagator
   *
   * Eliminates values of assigned views of type \a View.
   *
   * Requires \code #include "gecode/int/distinct.hh" \endcode
   * \ingroup FuncIntProp
   */

#if defined(DISTINCT_NAIVE_NO_EVENTS)
  template <class View>
  class Val : public NaryPropagator<View,PC_INT_DOM> {
  protected:
    using NaryPropagator<View,PC_INT_DOM>::x;

    /// Constructor for posting
    Val(Space* home, ViewArray<View>& x);
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val<View>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor*     copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for view array \a x
    static ExecStatus post(Space* home, ViewArray<View>& x);
  };
#elif defined(DISTINCT_NAIVE_ADVISOR_BASE) || defined(DISTINCT_NAIVE_ADVISOR_AVOID)
  template <class View>
  class Val : public Propagator {
  protected:
    ViewArray<View> x;
    Council<ViewAdvisor<View> > c;
    /// Constructor for posting
    Val(Space* home, ViewArray<View>& x);
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val<View>& p);
  public:
    virtual ExecStatus 
    advise(Space* home, Advisor& _a, const Delta& d) {
      ViewAdvisor<View>& a = static_cast<ViewAdvisor<View>&>(_a);
#if defined(DISTINCT_NAIVE_ADVISOR_BASE)
      if (View::modevent(d) == ME_INT_VAL)
        return ES_SUBSUMED_NOFIX(a,home);
      return ES_NOFIX;
#endif
#if defined(DISTINCT_NAIVE_ADVISOR_AVOID)
      if (View::modevent(d) == ME_INT_VAL)
        return ES_SUBSUMED_NOFIX(a,home);
      return ES_FIX;
#endif
    }
    /// Cost function (defined as dynamic PC_LINEAR_LO)
    virtual PropCost cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*     copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for view array \a x
    static ExecStatus post(Space* home, ViewArray<View>& x);
  };
#else
  template <class View>
  class Val : public NaryPropagator<View,PC_INT_VAL> {
  protected:
    using NaryPropagator<View,PC_INT_VAL>::x;

    /// Constructor for posting
    Val(Space* home, ViewArray<View>& x);
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val<View>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor*     copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for view array \a x
    static ExecStatus post(Space* home, ViewArray<View>& x);
  };
#endif

  /**
   * \brief Eliminate singletons by naive value propagation
   *
   * This is actually the propagation algorithm for Distinct::Val.
   * It is available as separate function as it is reused for
   * both bounds-consistent and domain-consistent distinct
   * propagators.
   *
   * If \a complete is true, computes fixpoint, otherwise might not
   * compute fixpoint. This can be helpful when used together with
   * bounds or domain propagation to protect from pathological cases
   * which can be handled more efficiently with bounds propagation.
   */
  template <class View, bool complete>
  ExecStatus prop_val(Space* home, ViewArray<View>&);



  /**
   * \brief Bounds-consistent distinct propagator
   *
   * The propagator uses staging: first it uses naive value-based
   * propagation and only then uses bounds-consistent propagation.
   * Due to using naive value-based propagation, the propagator
   * might actually achieve stronger consistency than just
   * bounds-consistency.
   *
   * The algorithm is taken from:
   *        A. Lopez-Ortiz, C.-G. Quimper, J. Tromp, and P. van Beek.
   *        A fast and simple algorithm for bounds consistency of the
   *        alldifferent constraint. IJCAI-2003.
   *
   * This implementation uses the code that is provided by
   * Peter Van Beek:
   *   http://ai.uwaterloo.ca/~vanbeek/software/software.html
   * The code (originally by John Tromp) here has only been slightly modified
   * to fit %Gecode (taking idempotent/non-idempotent propagation into account)
   * and uses a more efficient layout of datastructures (keeping the number
   * of different arrays small).
   *
   * Requires \code #include "gecode/int/distinct.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Bnd : public Propagator {
  protected:
    /// Views on which to perform bounds-propagation
    ViewArray<View> x;
    /// Views on which to perform value-propagation (subset of \c x)
    ViewArray<View> y;
    /// Constructor for posting
    Bnd(Space* home, ViewArray<View>& x);
    /// Constructor for cloning \a p
    Bnd(Space* home, bool share, Bnd<View>& p);
  public:
    /// Post propagator for view array \a x
    static ExecStatus post(Space* home, ViewArray<View>& x);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is dynamic
     * PC_LINEAR_LO. Otherwise it is dynamic PC_LINEAR_HI.
     */
    virtual PropCost cost(void) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Destructor
    virtual size_t dispose(Space* home);
  };

  /**
   * \brief Perform bounds-consistent distinct propagation
   *
   * This is actually the propagation algorithm for Distinct::Bnd.
   * It is available as separate function as it is reused for
   * both bounds-consistent and domain-consistent distinct
   * propagators.
   */
  template <class View>
  ExecStatus prop_bnd(Space* home, ViewArray<View>& x, int m);

  template <class View> class ViewNode;
  template <class View> class ValNode;

  /**
   * \brief Propagation controller for domain-consistent distinct
   *
   * The propagation controller provides convenient access to
   * performing incremental domain-consistent distinct propagation
   * so that the routines can be reused easily.
   *
   * Requires \code #include "gecode/int/distinct.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class DomCtrl {
  protected:
    /// View-value graph for propagation
    class ViewValGraph {
    protected:
      /// Array of view nodes
      ViewNode<View>** view; 
      /// Number of view nodes
      int n_view;
      /// Array of value nodes
      ValNode<View>** val;
      /// Number of value nodes
      int n_val;
      /// Marking counter
      unsigned int count;
    public:
      /// Construct graph as not yet initialized
      ViewValGraph(void);
      /// Test whether graph has been initialized
      bool initialized(void) const;
      /// Initialize graph
      ExecStatus init(Space* home, int n, View* x);
      /// Mark edges in graph
      void mark(void);
      /// Prune unmarked edges
      bool tell(Space* home);
      /// Purge graph if necessary
      void purge(void);
      /// Synchronize graph with new view domains
      bool sync(void);
    protected:
      /// Stack used during matching
      typedef Support::SentinelStack<ViewNode<View>*> MatchStack;
      /// Find a matching for node \a x
      bool match(MatchStack& m, ViewNode<View>* x);
    };
    /// Propagation is performed on a view-value graph
    ViewValGraph vvg;
  public:
    /// Initialize with non-initialized view-value graph
    DomCtrl(void);
    /// Check whether a view-value graph is available
    bool available(void);
    /// Initialize view-value graph for views \a x
    ExecStatus init(Space* home, int n, View* x);
    /// Synchronize available view-value graph
    ExecStatus sync(void);
    /// Perform propagation and return true if a view gets assigned
    bool propagate(Space* home);
  };

  /**
   * \brief Domain-consistent distinct propagator
   *
   * The propagator uses staging: first it uses naive value-based
   * propagation and only then uses domain-consistent propagation.
   *
   * The algorithm is taken from:
   *    Jean-Charles Régin, A filtering algorithm for constraints
   *    of difference in CSPs, Proceedings of the Twelfth National
   *    Conference on Artificial Intelligence, pages 362--367.
   *    Seattle, WA, USA, 1994.
   *
   * Requires \code #include "gecode/int/distinct.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Dom : public NaryPropagator<View,PC_INT_DOM> {
  protected:
    using NaryPropagator<View,PC_INT_DOM>::x;

    /// Propagation controller
    DomCtrl<View> dc;
    /// Constructor for cloning \a p
    Dom(Space* home, bool share, Dom<View>& p);
    /// Constructor for posting
    Dom(Space* home, ViewArray<View>& x);
  public:
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is dynamic
     * PC_LINEAR_LO. Otherwise it is dynamic PC_CUBIC_LO.
     */
    virtual PropCost cost(void) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Post propagator for views \a x
    static  ExecStatus post(Space* home, ViewArray<View>& x);
  };

  /**
   * \brief Ternary domain-consistent distinct propagator
   *
   * Requires \code #include "gecode/int/distinct.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class TerDom : public TernaryPropagator<View,PC_INT_DOM> {
  protected:
    using TernaryPropagator<View,PC_INT_DOM>::x0;
    using TernaryPropagator<View,PC_INT_DOM>::x1;
    using TernaryPropagator<View,PC_INT_DOM>::x2;

    /// Constructor for cloning \a p
    TerDom(Space* home, bool share, TerDom<View>& p);
    /// Constructor for posting
    TerDom(Space* home, View x0, View x1, View x2);
  public:
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Post propagator for views \a x
    static  ExecStatus post(Space* home, View x0, View x1, View x2);
  };

}}}

#include "gecode/int/distinct/val.icc"
#include "gecode/int/distinct/bnd.icc"
#include "gecode/int/distinct/ter-dom.icc"
#include "gecode/int/distinct/dom.icc"

#endif

// STATISTICS: int-prop

